/*
 * Copyright (c) 2022 shchmue
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

#include "cal0_read.h"

#include <gfx_utils.h>
#include <sec/se.h>
#include <sec/se_t210.h>
#include "../storage/emummc.h"
#include <storage/emmc.h>
#include <utils/util.h>

#include "../tools.h"
#include "../gfx/messages.h"

u16 crc16_calc(const u8 *buf, u32 len)
{
	const u8 *p, *q;
	u16 crc = 0x55aa;

	static u16 table[16] = {
		0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
		0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
	};

	q = buf + len;
	for (p = buf; p < q; p++)
	{
		u8 oct = *p;
		crc = (crc >> 4) ^ table[crc & 0xf] ^ table[(oct >> 0) & 0xf];
		crc = (crc >> 4) ^ table[crc & 0xf] ^ table[(oct >> 4) & 0xf];
	}

	return crc;
}


#include <stdlib.h> // posix_memalign (si dispo)
bool cal0_read(u32 tweak_ks, u32 crypt_ks, void *read_buffer) {
	nx_emmc_cal0_t *cal0 = (nx_emmc_cal0_t *)read_buffer;

	// Check if CAL0 was already read into this buffer
	if (cal0->magic == MAGIC_CAL0) {
		return true;
	}

	u32 sector = NX_EMMC_CALIBRATION_OFFSET / EMMC_BLOCKSIZE;

	if (!emummc_storage_read(sector, NX_EMMC_CALIBRATION_SIZE / EMMC_BLOCKSIZE, read_buffer)) {
		log_printf(LOG_ERR, LOG_MSG_ERROR_PRODINFO_READ);
		return false;
	}

	se_aes_xts_crypt_old(tweak_ks, crypt_ks, DECRYPT, 0, read_buffer, read_buffer, XTS_CLUSTER_SIZE, NX_EMMC_CALIBRATION_SIZE / XTS_CLUSTER_SIZE);

	if (cal0->magic != MAGIC_CAL0) {
		log_printf(LOG_ERR, LOG_MSG_ERROR_PRODINFO_MAGIC_READ);
		return false;
	}

	return true;
}

static inline u16 swap16(u16 v)
{
	return (v >> 8) | (v << 8);
}

static void debug_dump_hex(const char *label, const u8 *data, u32 len)
{
	debug_log_write("%s (%d bytes):\n", label, len);

	for (u32 i = 0; i < len; i++)
	{
		debug_log_write("%02X", data[i]);
		if ((i & 0x0F) == 0x0F)
			debug_log_write("\n");
	}

	debug_log_write("\n");
}

bool cal0_get_ssl_rsa_key(const nx_emmc_cal0_t *cal0, const void **out_key, u32 *out_key_size, const void **out_iv, u32 *out_generation) {
	const u32 ext_key_size = sizeof(cal0->ext_ssl_key_iv) + sizeof(cal0->ext_ssl_key);
	const u32 ext_key_crc_size = ext_key_size + sizeof(cal0->ext_ssl_key_ver) + sizeof(cal0->crc16_pad39);
	const u32 key_size = sizeof(cal0->ssl_key_iv) + sizeof(cal0->ssl_key);
	const u32 key_crc_size = key_size + sizeof(cal0->crc16_pad18);

// debug_log_write("cal0_get_ssl_rsa_key: \next_ssl_key_crc: 0x%04X\next_ssl_key_crc_calc: 0x%04X\nssl_key_crc: 0x%04X\nssl_key_crc_calc: 0x%04X\n\n", (u32)cal0->ext_ssl_key_crc, (u32)crc16_calc(cal0->ext_ssl_key_iv, ext_key_crc_size), (u32)cal0->ssl_key_crc, (u32)crc16_calc(cal0->ssl_key_iv, key_crc_size));
// debug_dump_hex("ext_ssl_key", cal0->ext_ssl_key_iv, sizeof(cal0->ext_ssl_key_iv) + sizeof(cal0->ext_ssl_key));
// debug_dump_hex("ssl_key", cal0->ssl_key_iv, sizeof(cal0->ssl_key_iv) + sizeof(cal0->ssl_key));

	if (cal0->ext_ssl_key_crc == crc16_calc(cal0->ext_ssl_key_iv, ext_key_crc_size)) {
		*out_key = cal0->ext_ssl_key;
		*out_key_size = ext_key_size;
		*out_iv = cal0->ext_ssl_key_iv;
		// Settings sysmodule manually zeroes this out below cal version 9
		*out_generation = cal0->version <= 8 ? 0 : cal0->ext_ssl_key_ver;
	} else if (cal0->ssl_key_crc == crc16_calc(cal0->ssl_key_iv, key_crc_size)) {
		*out_key = cal0->ssl_key;
		*out_key_size = key_size;
		*out_iv = cal0->ssl_key_iv;
		*out_generation = 0;
	} else {
		log_printf(LOG_ERR, LOG_MSG_KEYS_DUMP_ERROR_CRC_SSL_KEY);
		return false;
	}
	return true;
}


bool cal0_get_eticket_rsa_key(const nx_emmc_cal0_t *cal0, const void **out_key, u32 *out_key_size, const void **out_iv, u32 *out_generation) {
	const u32 ext_key_size = sizeof(cal0->ext_ecc_rsa2048_eticket_key_iv) + sizeof(cal0->ext_ecc_rsa2048_eticket_key);
	const u32 ext_key_crc_size = ext_key_size + sizeof(cal0->ext_ecc_rsa2048_eticket_key_ver) + sizeof(cal0->crc16_pad38);
	const u32 key_size = sizeof(cal0->rsa2048_eticket_key_iv) + sizeof(cal0->rsa2048_eticket_key);
	const u32 key_crc_size = key_size + sizeof(cal0->crc16_pad21);

// debug_log_write("cal0_get_eticket_rsa_key: \next_ecc_rsa2048_eticket_key_crc: %04X\next_ecc_rsa2048_eticket_key_crc: %04X\nrsa2048_eticket_key_crc: %04X\nrsa2048_eticket_key_crc_calc: %04X\n\n", swap16(cal0->ext_ecc_rsa2048_eticket_key_crc), swap16(crc16_calc(cal0->ext_ecc_rsa2048_eticket_key_iv, ext_key_crc_size)), swap16(cal0->rsa2048_eticket_key_crc), swap16(crc16_calc(cal0->rsa2048_eticket_key_iv, key_crc_size)));
// debug_dump_hex("ext_ecc_rsa2048_eticket_key", cal0->ext_ecc_rsa2048_eticket_key_iv, sizeof(cal0->ext_ecc_rsa2048_eticket_key_iv) + sizeof(cal0->ext_ecc_rsa2048_eticket_key));
// debug_dump_hex("rsa2048_eticket_key", cal0->rsa2048_eticket_key_iv, sizeof(cal0->rsa2048_eticket_key_iv) + sizeof(cal0->rsa2048_eticket_key));

	if (cal0->ext_ecc_rsa2048_eticket_key_crc == crc16_calc(cal0->ext_ecc_rsa2048_eticket_key_iv, ext_key_crc_size)) {
		*out_key = cal0->ext_ecc_rsa2048_eticket_key;
		*out_key_size = ext_key_size;
		*out_iv = cal0->ext_ecc_rsa2048_eticket_key_iv;
		// Settings sysmodule manually zeroes this out below cal version 9
		*out_generation = cal0->version <= 8 ? 0 : cal0->ext_ecc_rsa2048_eticket_key_ver;
	} else if (cal0->rsa2048_eticket_key_crc == crc16_calc(cal0->rsa2048_eticket_key_iv, key_crc_size)) {
		*out_key = cal0->rsa2048_eticket_key;
		*out_key_size = key_size;
		*out_iv = cal0->rsa2048_eticket_key_iv;
		*out_generation = 0;
	} else {
		log_printf(LOG_ERR, LOG_MSG_KEYS_DUMP_ERROR_CRC_DEVICE_KEY);
		return false;
	}
	return true;
}
