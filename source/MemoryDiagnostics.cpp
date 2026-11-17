#include <switch.h>
#include <malloc.h>
#include "MemoryDiagnostics.h"
#include "main_util.h"

#if MEMORY_DIAGNOSTICS
static void MemoryDiagnosticsLogMemoryValue(const char *name, Result result, u64 value) {
	if (R_SUCCEEDED(result)) {
		debug_log_write("  %s: %llu bytes (%llu.%02llu MiB)\n",
			name,
			(unsigned long long)value,
			(unsigned long long)(value / 1048576ULL),
			(unsigned long long)(((value % 1048576ULL) * 100ULL) / 1048576ULL));
	} else {
		debug_log_write("  %s: indisponible (rc=0x%08X)\n", name, (unsigned int)result);
	}
}

void MemoryDiagnosticsLogAllocator(const char *stage) {
	/* Take the snapshot before logging, because the logger may allocate memory. */
	const struct mallinfo allocatorInfo = mallinfo();
	const bool valid = allocatorInfo.arena >= 0 &&
		allocatorInfo.uordblks >= 0 &&
		allocatorInfo.fordblks >= 0 &&
		allocatorInfo.uordblks <= allocatorInfo.arena &&
		allocatorInfo.fordblks <= allocatorInfo.arena &&
		(u64)allocatorInfo.uordblks + (u64)allocatorInfo.fordblks <= (u64)allocatorInfo.arena;

	debug_log_write("MemoryDiagnostics allocator: %s\n", stage);
	if (valid) {
		debug_log_write("  newlib arena=%d, allocated=%d, free=%d bytes\n",
			allocatorInfo.arena, allocatorInfo.uordblks, allocatorInfo.fordblks);
	} else {
		debug_log_write("  newlib mallinfo invalide (arena=%d, allocated=%d, free=%d)\n",
			allocatorInfo.arena, allocatorInfo.uordblks, allocatorInfo.fordblks);
	}
}

void MemoryDiagnosticsLogMemory(const char *stage) {
	u64 processTotal = 0;
	u64 processUsed = 0;
	u64 systemResourceTotal = 0;
	u64 systemResourceUsed = 0;
	u64 nonSystemTotal = 0;
	u64 nonSystemUsed = 0;
	u64 heapAddress = 0;
	u64 heapSize = 0;
	u64 resourceLimitRaw = 0;
	s64 resourceCurrent = -1;
	s64 resourceLimit = -1;
	s64 resourcePeak = -1;

	const Result processTotalRc = svcGetInfo(&processTotal, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0);
	const Result processUsedRc = svcGetInfo(&processUsed, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0);
	const Result systemResourceTotalRc = svcGetInfo(&systemResourceTotal, InfoType_SystemResourceSizeTotal, CUR_PROCESS_HANDLE, 0);
	const Result systemResourceUsedRc = svcGetInfo(&systemResourceUsed, InfoType_SystemResourceSizeUsed, CUR_PROCESS_HANDLE, 0);
	const Result nonSystemTotalRc = svcGetInfo(&nonSystemTotal, InfoType_TotalNonSystemMemorySize, CUR_PROCESS_HANDLE, 0);
	const Result nonSystemUsedRc = svcGetInfo(&nonSystemUsed, InfoType_UsedNonSystemMemorySize, CUR_PROCESS_HANDLE, 0);
	const Result heapAddressRc = svcGetInfo(&heapAddress, InfoType_HeapRegionAddress, CUR_PROCESS_HANDLE, 0);
	const Result heapSizeRc = svcGetInfo(&heapSize, InfoType_HeapRegionSize, CUR_PROCESS_HANDLE, 0);
	const Result resourceHandleRc = svcGetInfo(&resourceLimitRaw, InfoType_ResourceLimit, CUR_PROCESS_HANDLE, 0);
	Result resourceCurrentRc = MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
	Result resourceLimitRc = MAKERESULT(Module_Libnx, LibnxError_NotInitialized);
	Result resourcePeakRc = MAKERESULT(Module_Libnx, LibnxError_NotInitialized);

	static const char *physicalTotalLabels[] = {
		"system DRAM pool total (Application)",
		"system DRAM pool total (Applet)",
		"system DRAM pool total (System)",
		"system DRAM pool total (SystemUnsafe)"
	};
	static const char *physicalUsedLabels[] = {
		"system DRAM pool used (Application)",
		"system DRAM pool used (Applet)",
		"system DRAM pool used (System)",
		"system DRAM pool used (SystemUnsafe)"
	};
	u64 physicalTotal[4] = {};
	u64 physicalUsed[4] = {};
	Result physicalTotalRc[4] = {};
	Result physicalUsedRc[4] = {};
	u64 physicalTotalSum = 0;
	u64 physicalUsedSum = 0;
	bool physicalTotalsComplete = true;
	bool physicalUsedComplete = true;
	const bool physicalInfoAvailable = envIsSyscallHinted(0x6F);

	if (physicalInfoAvailable) {
		for (u64 category = 0; category < 4; ++category) {
			physicalTotalRc[category] = svcGetSystemInfo(
				&physicalTotal[category],
				SystemInfoType_TotalPhysicalMemorySize,
				INVALID_HANDLE,
				category);
			physicalUsedRc[category] = svcGetSystemInfo(
				&physicalUsed[category],
				SystemInfoType_UsedPhysicalMemorySize,
				INVALID_HANDLE,
				category);

			if (R_SUCCEEDED(physicalTotalRc[category]))
				physicalTotalSum += physicalTotal[category];
			else
				physicalTotalsComplete = false;

			if (R_SUCCEEDED(physicalUsedRc[category])) {
				if (R_SUCCEEDED(physicalTotalRc[category]) && physicalUsed[category] > physicalTotal[category]) {
					physicalUsedComplete = false;
				} else {
					physicalUsedSum += physicalUsed[category];
				}
			} else {
				physicalUsedComplete = false;
			}
		}
	} else {
		physicalTotalsComplete = false;
		physicalUsedComplete = false;
	}

	if (R_SUCCEEDED(resourceHandleRc)) {
		const Handle resourceHandle = (Handle)resourceLimitRaw;
		if (envIsSyscallHinted(0x31))
			resourceCurrentRc = svcGetResourceLimitCurrentValue(&resourceCurrent, resourceHandle, LimitableResource_Memory);
		if (envIsSyscallHinted(0x30))
			resourceLimitRc = svcGetResourceLimitLimitValue(&resourceLimit, resourceHandle, LimitableResource_Memory);
		if (envIsSyscallHinted(0x37))
			resourcePeakRc = svcGetResourceLimitPeakValue(&resourcePeak, resourceHandle, LimitableResource_Memory);
	}

	/* Snapshot the C allocator before writing diagnostic lines to the log. */
	const struct mallinfo allocatorInfo = mallinfo();
	const bool allocatorInfoValid = allocatorInfo.arena >= 0 &&
		allocatorInfo.uordblks >= 0 &&
		allocatorInfo.fordblks >= 0 &&
		allocatorInfo.uordblks <= allocatorInfo.arena &&
		allocatorInfo.fordblks <= allocatorInfo.arena &&
		(u64)allocatorInfo.uordblks + (u64)allocatorInfo.fordblks <= (u64)allocatorInfo.arena;
	static bool hasPreviousProcessUsed = false;
	static bool hasPreviousPhysicalUsed = false;
	static bool hasPreviousAllocatorSample = false;
	static u64 previousProcessUsed = 0;
	static u64 previousPhysicalUsed = 0;
	static u64 previousAllocatorArena = 0;
	static u64 previousAllocatorUsed = 0;
	static u64 previousAllocatorFree = 0;

	debug_log_write("MemoryDiagnostics: %s\n", stage);
	MemoryDiagnosticsLogMemoryValue("kernel process-memory accounting total (InfoType_TotalMemorySize)", processTotalRc, processTotal);
	MemoryDiagnosticsLogMemoryValue("kernel process-memory accounting used (InfoType_UsedMemorySize)", processUsedRc, processUsed);
	MemoryDiagnosticsLogMemoryValue("kernel process system-resource accounting total", systemResourceTotalRc, systemResourceTotal);
	MemoryDiagnosticsLogMemoryValue("kernel process system-resource accounting used", systemResourceUsedRc, systemResourceUsed);
	MemoryDiagnosticsLogMemoryValue("kernel process non-system accounting total", nonSystemTotalRc, nonSystemTotal);
	MemoryDiagnosticsLogMemoryValue("kernel process non-system accounting used", nonSystemUsedRc, nonSystemUsed);
	debug_log_write("  note: InfoType_UsedMemorySize includes code, main-thread stack and the heap allocated to the process; it is not the live malloc/new usage\n");
	if (R_SUCCEEDED(processTotalRc) && R_SUCCEEDED(processUsedRc) && processUsed > processTotal)
		debug_log_write("  WARNING: process used exceeds process available\n");
	else if (R_SUCCEEDED(processTotalRc) && R_SUCCEEDED(processUsedRc))
		MemoryDiagnosticsLogMemoryValue("kernel-reported process memory remaining (total minus used)", 0, processTotal - processUsed);
	if (R_SUCCEEDED(processUsedRc) && envHasHeapOverride()) {
		const u64 heapOverrideSize = envGetHeapOverrideSize();
		if (processUsed >= heapOverrideSize)
			MemoryDiagnosticsLogMemoryValue("process memory outside loader heap (code and main-thread stack)", 0, processUsed - heapOverrideSize);
		else
			debug_log_write("  WARNING: loader heap size exceeds kernel process-memory used counter\n");
	}
	if (R_SUCCEEDED(systemResourceTotalRc) && R_SUCCEEDED(systemResourceUsedRc) && systemResourceUsed > systemResourceTotal)
		debug_log_write("  WARNING: system-resource used exceeds system-resource total\n");
	if (R_SUCCEEDED(nonSystemTotalRc) && R_SUCCEEDED(nonSystemUsedRc) && nonSystemUsed > nonSystemTotal)
		debug_log_write("  WARNING: non-system used exceeds non-system total\n");

	if (physicalInfoAvailable) {
		for (int category = 0; category < 4; ++category) {
			MemoryDiagnosticsLogMemoryValue(physicalTotalLabels[category], physicalTotalRc[category], physicalTotal[category]);
			MemoryDiagnosticsLogMemoryValue(physicalUsedLabels[category], physicalUsedRc[category], physicalUsed[category]);
			if (R_SUCCEEDED(physicalTotalRc[category]) && R_SUCCEEDED(physicalUsedRc[category]) && physicalUsed[category] > physicalTotal[category])
				debug_log_write("  WARNING: physical used exceeds physical total for category %d\n", category);
		}
		if (physicalTotalsComplete)
			MemoryDiagnosticsLogMemoryValue("system DRAM pool capacity (sum of four categories)", 0, physicalTotalSum);
		else
			debug_log_write("  system DRAM pool capacity: incomplete, sum omitted\n");
		if (physicalUsedComplete) {
			MemoryDiagnosticsLogMemoryValue("system-wide DRAM pool usage (sum of four categories)", 0, physicalUsedSum);
			if (hasPreviousPhysicalUsed) {
				if (physicalUsedSum >= previousPhysicalUsed)
					debug_log_write("  delta system-wide DRAM pool usage: +%llu bytes\n", (unsigned long long)(physicalUsedSum - previousPhysicalUsed));
				else
					debug_log_write("  delta system-wide DRAM pool usage: -%llu bytes\n", (unsigned long long)(previousPhysicalUsed - physicalUsedSum));
			}
			previousPhysicalUsed = physicalUsedSum;
			hasPreviousPhysicalUsed = true;
		} else
			debug_log_write("  system-wide DRAM pool usage: incomplete, sum omitted\n");
		debug_log_write("  note: DRAM pool usage is system-wide kernel accounting, not this process's malloc usage\n");
	} else {
		debug_log_write("  physical memory: indisponible (SVC 0x6F non annoncé)\n");
	}

	if (R_SUCCEEDED(heapAddressRc))
		debug_log_write("  heap virtual base: 0x%llX\n", (unsigned long long)heapAddress);
	else
		debug_log_write("  heap virtual base: indisponible (rc=0x%08X)\n", (unsigned int)heapAddressRc);
	MemoryDiagnosticsLogMemoryValue("reserved virtual heap address-region size (not allocated RAM)", heapSizeRc, heapSize);
	debug_log_write("  loader-provided heap override: enabled=%d address=%p\n",
		envHasHeapOverride() ? 1 : 0,
		envHasHeapOverride() ? envGetHeapOverrideAddr() : NULL);
	if (envHasHeapOverride())
		MemoryDiagnosticsLogMemoryValue("loader-provided heap size (allocated to process, not live malloc usage)", 0, envGetHeapOverrideSize());

	if (allocatorInfoValid) {
		MemoryDiagnosticsLogMemoryValue("newlib allocator arena currently expanded (mallinfo arena)", 0, (u64)allocatorInfo.arena);
		MemoryDiagnosticsLogMemoryValue("newlib allocator live allocated blocks (mallinfo uordblks)", 0, (u64)allocatorInfo.uordblks);
		MemoryDiagnosticsLogMemoryValue("newlib allocator free blocks (mallinfo fordblks)", 0, (u64)allocatorInfo.fordblks);
	} else {
		debug_log_write("  newlib allocator: invalid mallinfo counters (negative, overflowed or inconsistent; arena=%d, allocated=%d, free=%d)\n",
			allocatorInfo.arena,
			allocatorInfo.uordblks,
			allocatorInfo.fordblks);
	}


	if (R_SUCCEEDED(processUsedRc)) {
		if (hasPreviousProcessUsed) {
			if (processUsed >= previousProcessUsed)
				debug_log_write("  delta kernel process-memory accounting used: +%llu bytes\n", (unsigned long long)(processUsed - previousProcessUsed));
			else
				debug_log_write("  delta kernel process-memory accounting used: -%llu bytes\n", (unsigned long long)(previousProcessUsed - processUsed));
		}
		previousProcessUsed = processUsed;
		hasPreviousProcessUsed = true;
	}

	if (allocatorInfoValid && hasPreviousAllocatorSample) {
		if ((u64)allocatorInfo.arena >= previousAllocatorArena)
			debug_log_write("  delta newlib allocator arena: +%llu bytes\n", (unsigned long long)((u64)allocatorInfo.arena - previousAllocatorArena));
		else
			debug_log_write("  delta newlib allocator arena: -%llu bytes\n", (unsigned long long)(previousAllocatorArena - (u64)allocatorInfo.arena));
		if ((u64)allocatorInfo.uordblks >= previousAllocatorUsed)
			debug_log_write("  delta newlib allocated blocks: +%llu bytes\n", (unsigned long long)((u64)allocatorInfo.uordblks - previousAllocatorUsed));
		else
			debug_log_write("  delta newlib allocated blocks: -%llu bytes\n", (unsigned long long)(previousAllocatorUsed - (u64)allocatorInfo.uordblks));
		if ((u64)allocatorInfo.fordblks >= previousAllocatorFree)
			debug_log_write("  delta newlib free blocks: +%llu bytes\n", (unsigned long long)((u64)allocatorInfo.fordblks - previousAllocatorFree));
		else
			debug_log_write("  delta newlib free blocks: -%llu bytes\n", (unsigned long long)(previousAllocatorFree - (u64)allocatorInfo.fordblks));
	}
	if (allocatorInfoValid) {
		previousAllocatorArena = (u64)allocatorInfo.arena;
		previousAllocatorUsed = (u64)allocatorInfo.uordblks;
		previousAllocatorFree = (u64)allocatorInfo.fordblks;
		hasPreviousAllocatorSample = true;
	}

	debug_log_write("  resource handle rc=0x%08X raw=0x%llX\n",
		(unsigned int)resourceHandleRc, (unsigned long long)resourceLimitRaw);
	MemoryDiagnosticsLogMemoryValue("resource-limit memory current", resourceCurrentRc, (u64)resourceCurrent);
	MemoryDiagnosticsLogMemoryValue("resource-limit memory limit", resourceLimitRc, (u64)resourceLimit);
	MemoryDiagnosticsLogMemoryValue("resource-limit memory peak", resourcePeakRc, (u64)resourcePeak);
}
#endif
