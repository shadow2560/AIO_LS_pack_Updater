/*
 * Copyright (c) 2020 CaramelDunes
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

#include "build_prodinfo.h"

#include "../config.h"
#include <display/di.h>
#include <gfx_utils.h>
#include "../gfx/tui.h"
#include "../hos/hos.h"
#include <libs/fatfs/ff.h>
#include <libs/nx_savedata/save.h>
#include <mem/heap.h>
#include <mem/minerva.h>
#include <mem/sdram.h>
#include <sec/se.h>
#include <sec/se_t210.h>
#include <sec/tsec.h>
#include <soc/fuse.h>
#include <mem/smmu.h>
#include <soc/t210.h>
#include <soc/timer.h>
#include <storage/emmc.h>
#include "../storage/nx_emmc_bis.h"
#include <storage/sd.h>
#include <storage/sdmmc.h>
#include <utils/btn.h>
#include <utils/list.h>
#include <utils/sprintf.h>
#include <utils/util.h>

#include <string.h>

#include "../keys/keys.h"
#include "cal_blocks.h"
#include "cal0.h"
#include "crc16.h"
#include "donor_keys.h"
#include "../keys/crypto.h"
#include "../keys/key_sources.inl"

#include "../tools.h"
#include "../gfx/messages.h"

typedef struct imported_parts {
	u8 gamecard_certificate[0x420];
	u8 extended_gamecard_key[0x140];
} imported_parts_t;

extern bool clear_sector_cache;
extern bool lock_sector_cache;
extern u32 secindex;
extern volatile nyx_storage_t *nyx_str;

// Not static so that keys.c:dump_keys can use it.
// static u32 color_idx = 0;
static u32 start_time, end_time;

// static const char msg_writing[] = "%kWriting %s\n";

static void _build_cal0(u8* prodinfo_buffer, u32 prodinfo_size, u8 master_key_0[16], u64 device_id_int)
{
	char device_id_as_string[0x11] = {0};
	device_id_string(device_id_as_string);

	gfx_printf("Your device id: %s\n", device_id_as_string);
	gfx_printf("key generation: %d\n\n", fuse_read_odm_keygen_rev());

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "header");
	// gfx_printf("%kWriting header\n", colors[(color_idx++) % 6]);
	write_header(prodinfo_buffer);

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "config id");
	// gfx_printf("%kWriting config id\n", colors[(color_idx++) % 6]);
	write_config_id(prodinfo_buffer);

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "Wlan country codes");
	// gfx_printf("%kWriting Wlan country codes\n", colors[(color_idx++) % 6]);
	write_wlan_country_codes(prodinfo_buffer);

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "MAC addresses");
	// gfx_printf("%kWriting MAC addresses\n", colors[(color_idx++) % 6]);
	write_mac_addresses(prodinfo_buffer, device_id_int);

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "sensors calibration data");
	// gfx_printf("%kWriting sensors calibration data\n", colors[(color_idx++) % 6]);
	write_sensors_offset_scale(prodinfo_buffer);

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "blank serial number");
	// gfx_printf("%kWriting blank serial number\n", colors[(color_idx++) % 6]);
	write_serial_number(prodinfo_buffer);

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "random number");
	// gfx_printf("%kWriting random number\n", colors[(color_idx++) % 6]);
	write_random_number(prodinfo_buffer, device_id_int);

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "battery lot");
	// gfx_printf("%kWriting battery lot\n", colors[(color_idx++) % 6]);
	write_battery_lot(prodinfo_buffer);

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "speaker calibration data");
	// gfx_printf("%kWriting speaker calibration data\n", colors[(color_idx++) % 6]);
	write_speaker_calibration_value(prodinfo_buffer);

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "your very own console colors\n");
	// gfx_printf("%kWriting your very own console colors\n\n", colors[(color_idx++) % 6]);
	write_console_colors(prodinfo_buffer, device_id_int);

	u32 display_id = __builtin_bswap32(nyx_str->info.panel_id);
	write_short_values(prodinfo_buffer, display_id);

	// Certificates
	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "device id strings");
	// gfx_printf("%kWriting device id strings\n", colors[(color_idx++) % 6]);
	// On boot, the device id from fuses is compared to the one from certificates.
	// Until 14.0.0 it uses EccB233DeviceCertificate.
	write_device_id_string_at_offset(prodinfo_buffer, device_id_as_string, OFFSET_OF_BLOCK(EccB233DeviceCertificate) + 0xC4);
	// Starting from 14.0.0, it uses Rsa2048ETicketCertificate
	write_device_id_string_at_offset(prodinfo_buffer, device_id_as_string, OFFSET_OF_BLOCK(Rsa2048ETicketCertificate) + 0xC4);

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "empty SSL certificate\n");
	// gfx_printf("%kWriting empty SSL certificate\n\n", colors[(color_idx++) % 6]);
	write_ssl_certificate(prodinfo_buffer);

	// GCM blocks
	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "extended keys");
	// gfx_printf("%kWriting extended keys\n", colors[(color_idx++) % 6]);
	encrypt_extended_device_key(prodinfo_buffer, prodinfo_buffer + OFFSET_OF_BLOCK(ExtendedEccB233DeviceKey) + 0x10, device_id_int, master_key_0);
	encrypt_extended_eticket_key(prodinfo_buffer, prodinfo_buffer + OFFSET_OF_BLOCK(ExtendedRsa2048ETicketKey) + 0x10, device_id_int, master_key_0);

	// gfx_printf(msg_writing, colors[(color_idx++) % 6], "checksums");
	// gfx_printf("\n%kWriting checksums\n", colors[(color_idx++) % 6]);
	write_all_crc(prodinfo_buffer, prodinfo_size);
	write_all_sha256(prodinfo_buffer);

	write_body_checksum(prodinfo_buffer);

	if (!valid_own_prodinfo(prodinfo_buffer, prodinfo_size, master_key_0)) {
		log_printf(LOG_WARN, LOG_MSG_PRODINFOGEN_WARN_CREATE);
		gfx_printf("%k", COLOR_WHITE);
	}
}

static void _save_prodinfo_to_sd(u8* prodinfo_buffer, u32 prodinfo_size, bool is_from_donor) {
	if (!sd_mount()) {
		log_printf(LOG_ERR, LOG_MSG_ERR_SD_MOUNT);
		return;
	}

	f_mkdir("sd:/switch");
	const char* prodinfo_path = "sd:/switch/generated_prodinfo_from_scratch.bin";

	if (is_from_donor) {
		prodinfo_path = "sd:/switch/generated_prodinfo_from_donor.bin";
	}

	FILINFO fno;
	if (sd_save_to_file(prodinfo_buffer, prodinfo_size, prodinfo_path) == FR_OK && f_stat(prodinfo_path, &fno) == FR_OK) {
		log_printf(LOG_OK, LOG_MSG_PRODINFOGEN_SAVE_FILE_SUCCESS, (u32)fno.fsize, prodinfo_path);
	} else {
		log_printf(LOG_ERR, LOG_MSG_PRODINFOGEN_SAVE_FILE_ERROR);
	}
}

static void _master_key_from_key_generation(u8 donor_prodinfo_version, u8 key_generation, const read_keyset_t *donor_keyset, const key_storage_t* keyset, u8 output[SE_KEY_128_SIZE])
{
	u8 temp[SE_KEY_128_SIZE] = {0};
	memcpy(output, keyset->master_key[0], SE_KEY_128_SIZE);

	if (donor_prodinfo_version >= 9)
	{
		u8 offset_key_generation = key_generation - 3;

		if (offset_key_generation >= 0 && offset_key_generation < 0x8 && key_exists(keyset->master_key[offset_key_generation]))
		{
			se_aes_key_set(9, donor_keyset->device_key_4x, SE_KEY_128_SIZE);
			se_aes_crypt_block_ecb(9, DECRYPT, temp, device_master_key_source_sources[offset_key_generation]);

			se_aes_key_set(12, keyset->master_key[0], SE_KEY_128_SIZE);
			se_aes_unwrap_key(12, 12, device_master_kek_sources[offset_key_generation]);

			se_aes_crypt_block_ecb(12, DECRYPT, output, temp);
		}
	}
}

static inline u32 _read_le_u32(const void *buffer, u32 offset)
{
	return (*(u8 *)(buffer + offset + 0)) |
		   (*(u8 *)(buffer + offset + 1) << 0x08) |
		   (*(u8 *)(buffer + offset + 2) << 0x10) |
		   (*(u8 *)(buffer + offset + 3) << 0x18);
}

static bool _read_donor_prodinfo(imported_parts_t* output, const char* donor_prodinfo_filename, const read_keyset_t* donor_keyset, const key_storage_t* keyset) {
	FILINFO fno;
	if (f_stat(donor_prodinfo_filename, &fno)) {
		log_printf(LOG_ERR, LOG_MSG_PRODINFOGEN_ERR_FIND_DONOR_BIN);
		return false;
	}
	else if (fno.fsize < MINIMUM_PRODINFO_SIZE) {
		log_printf(LOG_ERR, LOG_MSG_PRODINFOGEN_ERR_TOO_SMALL, fno.fsize, MINIMUM_PRODINFO_SIZE);
		return false;
	}
	else if (fno.fsize > MAXIMUM_PRODINFO_SIZE) {
		log_printf(LOG_ERR, LOG_MSG_PRODINFOGEN_ERR_TOO_BIG, fno.fsize, MAXIMUM_PRODINFO_SIZE);
		return false;
	}

	// Read donor prodinfo.
	u32 donor_prodinfo_size = 0;
	u8 *donor_prodinfo_buffer = sd_file_read(donor_prodinfo_filename, &donor_prodinfo_size);

	if (!valid_prodinfo_checksums(donor_prodinfo_buffer, donor_prodinfo_size)) {
		log_printf(LOG_ERR, LOG_MSG_PRODINFOGEN_ERR_INVALID);
		return false;
	}

	u32 donor_prodinfo_version = _read_le_u32(donor_prodinfo_buffer, 0x4);
	char donor_prodinfo_device_id[0x11] = {0};
	memcpy(donor_prodinfo_device_id, donor_prodinfo_buffer + OFFSET_OF_BLOCK(EccB233DeviceCertificate) + 0xC6, 0x10);

	gfx_printf("Donor PRODINFO looks valid\n version = %d\n device id = %s\n\n", donor_prodinfo_version, donor_prodinfo_device_id);

	if (donor_prodinfo_version >= 9 && !key_exists(donor_keyset->device_key_4x)) {
		log_printf(LOG_WARN, LOG_MSG_PRODINFOGEN_WARN_KEY4X);
	}

	// GameCardCertificate
	memcpy(output->gamecard_certificate, donor_prodinfo_buffer + OFFSET_OF_BLOCK(GameCardCertificate), SIZE_OF_BLOCK(GameCardCertificate));

	// ExtendedGameCardKey
	u8 key_generation = *(donor_prodinfo_buffer + OFFSET_OF_BLOCK(ExtendedGameCardKey) + SIZE_OF_BLOCK(ExtendedGameCardKey) - 0x10) - 1;
	u8 personalized_master_key[SE_KEY_128_SIZE] = {0};
	_master_key_from_key_generation(donor_prodinfo_version, key_generation, donor_keyset, keyset, personalized_master_key);

	if (!decrypt_extended_gamecard_key(donor_prodinfo_buffer, output->extended_gamecard_key, personalized_master_key)) {
		log_printf(LOG_WARN, LOG_MSG_PRODINFOGEN_WARN_KEY_EGC);
	}

	return true;
}

void build_prodinfo(const char* optional_donor_filename, bool end_with_key_press) {
	minerva_change_freq(FREQ_1600);

	display_backlight_brightness(h_cfg.backlight, 1000);
	gfx_clear_grey(0x1B);
	gfx_con_setpos(0, 0);

	log_printf(LOG_INFO, LOG_MSG_PRODINFOGEN_BEGIN);
	gfx_printf("\n\n");

	// color_idx = 0;

	start_time = get_tmr_us();

	gfx_printf("Dumping keys...\n");
	key_storage_t keyset = {0};
	dump_keys_prodinfogen(&keyset);

	if (get_crc_16(keyset.master_key[0], 0x10) != 0x801B) {
		log_printf(LOG_ERR, LOG_MSG_PRODINFOGEN_ERR_MASTER_KEY);
		gfx_printf("\n");
	} else {
		u32 prodinfo_size = MAXIMUM_PRODINFO_SIZE;
		u8 *prodinfo_buffer = calloc(prodinfo_size, 1);

		u64 device_id_int = fuse_get_device_id();

		// First nibble is always 6 but second one ranges from 0 to 3.
		device_id_int |= 0x6000000000000000ULL;

		bool is_from_donor = optional_donor_filename != NULL;

		if (is_from_donor) {
			read_keyset_t donor_keyset = {0};
			bool read_keys_result = read_keys(&donor_keyset, "sd:/switch/donor.keys");
			if (!read_keys_result) {
				log_printf(LOG_ERR, LOG_MSG_PRODINFOGEN_ERR_READ_DONOR_KEYS);
			}

			imported_parts_t imported_parts = {0};
			bool import_result = _read_donor_prodinfo(&imported_parts, optional_donor_filename, &donor_keyset, &keyset);

			if (!import_result) {
				gfx_printf("\n");
				log_printf(LOG_WARN, LOG_MSG_PRODINFOGEN_WARN_FALLBACK_DONOR_TO_SCRATCH);
				memset(&imported_parts, 0, sizeof(imported_parts_t));
			} else {
				gfx_printf("Importing GameCard certificate\n");
				memcpy(prodinfo_buffer + OFFSET_OF_BLOCK(GameCardCertificate), imported_parts.gamecard_certificate, SIZE_OF_BLOCK(GameCardCertificate));
								
				gfx_printf("Importing extended GameCard key\n\n");
				memcpy(prodinfo_buffer + OFFSET_OF_BLOCK(ExtendedGameCardKey) + 0x10, imported_parts.extended_gamecard_key, SIZE_OF_BLOCK(ExtendedGameCardKey));
			
				// Don't encrypt it when building from scratch, as it will prevent the console from booting.
				encrypt_extended_gamecard_key(prodinfo_buffer, prodinfo_buffer + OFFSET_OF_BLOCK(ExtendedGameCardKey) + 0x10, device_id_int, keyset.master_key[0]);
			}
		}

		_build_cal0(prodinfo_buffer, prodinfo_size, keyset.master_key[0], device_id_int);

		gfx_printf("\n");
		log_printf(LOG_INFO, LOG_MSG_PRODINFOGEN_WRITING_FILE);
		_save_prodinfo_to_sd(prodinfo_buffer, prodinfo_size, is_from_donor);

		free(prodinfo_buffer); 
	}

	end_time = get_tmr_us();
	gfx_printf("\n");
	log_printf(LOG_OK, LOG_MSG_PRODINFOGEN_WRITING_FILE, end_time - start_time);
	gfx_printf("\n");


	minerva_change_freq(FREQ_800);
	if (end_with_key_press) {
		save_screenshot_and_go_back("PRODINFO_build");
	}
}