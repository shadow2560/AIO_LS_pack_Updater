/*
 * Copyright (c) Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <switch.h>
#include <string.h>
#include "ams_su.h"
#include "service_guard.h"
#include "main_util.h"

static Service g_amssuSrv;
static TransferMemory g_tmem;
static void *g_ownedTransferMemoryBuffer;
static bool g_transferMemoryDetached;
static bool g_setupRequestSent;
static bool g_setupBufferWasOwned;
static bool g_setupAccepted;

NX_GENERATE_SERVICE_GUARD(amssu);

static void AmssuDetachTransferMemory(void) {
	/*
	 * Do not free the source buffer here. After a successful SetupUpdate,
	 * ams:su owns a mapped reference to this TransferMemory and may continue
	 * using it while the asynchronous preparation/apply task is alive.
	 * Closing our local handle does not release the service's reference.
	 * Unlike Daybreak's standalone process lifecycle, this application keeps
	 * running after DaybreakAppExit(), and ams:su exposes no public command to
	 * destroy its update task or unmap the transferred memory. Freeing the
	 * buffer merely because the local Daybreak session ended could therefore
	 * invalidate memory still used by the service. Keep the pointer and only
	 * release it later if svcQueryMemory confirms that it is RW again.
	 */
	if (g_tmem.src_addr != NULL)
		g_ownedTransferMemoryBuffer = g_tmem.src_addr;
	tmemCloseHandle(&g_tmem);
	memset(&g_tmem, 0, sizeof(g_tmem));
	g_tmem.handle = INVALID_HANDLE;
}

Result _amssuInitialize(void) {
	memset(&g_tmem, 0, sizeof(g_tmem));
	g_tmem.handle = INVALID_HANDLE;
	if (!g_setupAccepted) {
		g_transferMemoryDetached = false;
		g_setupRequestSent = false;
		g_setupBufferWasOwned = false;
	}
	return smGetService(&g_amssuSrv, "ams:su");
}

void _amssuCleanup(void) {
	serviceClose(&g_amssuSrv);
	AmssuDetachTransferMemory();
	g_transferMemoryDetached = true;
}

void amssuReleaseTransferMemoryBuffer(void) {
	if (!g_transferMemoryDetached || g_ownedTransferMemoryBuffer == NULL)
		return;

	/* A one-shot permission check is intentional: never wait here because
	 * ams:su may retain the mapping until process/system teardown. */
	MemoryInfo memoryInfo = {0};
	u32 pageInfo = 0;
	Result rc = svcQueryMemory(&memoryInfo, &pageInfo, (u64)g_ownedTransferMemoryBuffer);
	if (R_FAILED(rc))
		return;

	u64 bufferAddress = (u64)g_ownedTransferMemoryBuffer;
	if (bufferAddress < memoryInfo.addr || bufferAddress - memoryInfo.addr >= memoryInfo.size)
		return;
	if ((memoryInfo.perm & Perm_Rw) != Perm_Rw)
		return;

	TransferMemory ownedMemory = {0};
	ownedMemory.handle = INVALID_HANDLE;
	ownedMemory.src_addr = g_ownedTransferMemoryBuffer;
	rc = tmemClose(&ownedMemory);
	if (R_SUCCEEDED(rc))
		g_ownedTransferMemoryBuffer = NULL;
}

bool amssuCanRetrySetup(void) {
	if (g_setupAccepted)
		return false;
	if (!g_setupRequestSent)
		return true;
	if (!g_setupBufferWasOwned)
		return false;
	amssuReleaseTransferMemoryBuffer();
	if (g_ownedTransferMemoryBuffer != NULL)
		return false;
	g_setupRequestSent = false;
	g_setupBufferWasOwned = false;
	return true;
}

Service *amssuGetServiceSession(void) {
    return &g_amssuSrv;
}

Result amssuGetUpdateInformation(AmsSuUpdateInformation *out, const char *path) {
    char send_path[FS_MAX_PATH] = {0};
    strncpy(send_path, path, FS_MAX_PATH-1);
    send_path[FS_MAX_PATH-1] = 0;

    return serviceDispatchOut(&g_amssuSrv, 0, *out,
        .buffer_attrs = { SfBufferAttr_In | SfBufferAttr_HipcPointer | SfBufferAttr_FixedSize },
        .buffers      = { { send_path, FS_MAX_PATH } },
    );
}

Result amssuValidateUpdate(AmsSuUpdateValidationInfo *out, const char *path) {
    char send_path[FS_MAX_PATH] = {0};
    strncpy(send_path, path, FS_MAX_PATH-1);
    send_path[FS_MAX_PATH-1] = 0;

    return serviceDispatchOut(&g_amssuSrv, 1, *out,
        .buffer_attrs = { SfBufferAttr_In | SfBufferAttr_HipcPointer | SfBufferAttr_FixedSize },
        .buffers      = { { send_path, FS_MAX_PATH } },
    );
}

Result amssuSetupUpdate(void *buffer, size_t size, const char *path, bool exfat) {
    Result rc = 0;
	g_transferMemoryDetached = false;
	g_setupBufferWasOwned = buffer == NULL;

    if (buffer == NULL) {
        rc = tmemCreate(&g_tmem, size, Perm_None);
    } else {
        rc = tmemCreateFromMemory(&g_tmem, buffer, size, Perm_None);
    }
    if (R_FAILED(rc)) return rc;

    char send_path[FS_MAX_PATH] = {0};
    strncpy(send_path, path, FS_MAX_PATH-1);
    send_path[FS_MAX_PATH-1] = 0;

    const struct {
        u8 exfat;
        u64 size;
    } in = { exfat, g_tmem.size };

	g_setupRequestSent = true;
    rc = serviceDispatchIn(&g_amssuSrv, 2, in,
        .in_num_handles = 1,
        .in_handles = { g_tmem.handle },
        .buffer_attrs = { SfBufferAttr_In | SfBufferAttr_HipcPointer | SfBufferAttr_FixedSize },
        .buffers      = { { send_path, FS_MAX_PATH } },
    );
    if (R_FAILED((rc))) {
		AmssuDetachTransferMemory();
	} else {
		g_setupAccepted = true;
    }

    return rc;
}

Result amssuSetupUpdateWithVariation(void *buffer, size_t size, const char *path, bool exfat, u32 variation) {
    Result rc = 0;
	g_transferMemoryDetached = false;

    if (buffer == NULL) {
        rc = tmemCreate(&g_tmem, size, Perm_None);
    } else {
        rc = tmemCreateFromMemory(&g_tmem, buffer, size, Perm_None);
    }
    if (R_FAILED(rc)) return rc;

    char send_path[FS_MAX_PATH] = {0};
    strncpy(send_path, path, FS_MAX_PATH-1);
    send_path[FS_MAX_PATH-1] = 0;

    const struct {
        u8 exfat;
        u32 variation;
        u64 size;
    } in = { exfat, variation, g_tmem.size };

    rc = serviceDispatchIn(&g_amssuSrv, 3, in,
        .in_num_handles = 1,
        .in_handles = { g_tmem.handle },
        .buffer_attrs = { SfBufferAttr_In | SfBufferAttr_HipcPointer | SfBufferAttr_FixedSize },
        .buffers      = { { send_path, FS_MAX_PATH } },
    );
    if (R_FAILED((rc))) {
		AmssuDetachTransferMemory();
    }

    return rc;
}

Result amssuRequestPrepareUpdate(AsyncResult *a) {
    memset(a, 0, sizeof(*a));

    Handle event = INVALID_HANDLE;
    Result rc = serviceDispatch(&g_amssuSrv, 4,
        .out_num_objects = 1,
        .out_objects = &a->s,
        .out_handle_attrs = { SfOutHandleAttr_HipcCopy },
        .out_handles = &event,
    );

    if (R_SUCCEEDED(rc))
        eventLoadRemote(&a->event, event, false);

    return rc;
}

Result amssuGetPrepareUpdateProgress(NsSystemUpdateProgress *out) {
    return serviceDispatchOut(&g_amssuSrv, 5, *out);
}

Result amssuHasPreparedUpdate(bool *out) {
    u8 outval = 0;
    Result rc = serviceDispatchOut(&g_amssuSrv, 6, outval);
    if (R_SUCCEEDED(rc)) {
        if (out) *out = outval & 1;
    }
    return rc;
}

Result amssuApplyPreparedUpdate() {
    return serviceDispatch(&g_amssuSrv, 7);
}