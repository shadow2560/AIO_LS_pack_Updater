#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <switch.h>

#include "translate.h"
#include "ini.h"

// define a structure for holding the values in "language" section of the ini file.
typedef struct{
// Simple word and global messages
	const char *lng_yes;
	const char *lng_no;
	const char *lng_unknown_0;
	const char *lng_unknown_1;
	const char *lng_files;
	const char *lng_partition;
	const char *lng_usable;
	const char *lng_not_usable;
	const char *lng_success_reboot_in_five_seconds;
	const char *lng_error_reboot_to_payload;
	const char *lng_error_not_enough_space_on_sd;

// Main menu
	const char *lng_title;
	const char *lng_title_beta;
	const char *lng_a_menu;
	const char *lng_x_menu;
	const char *lng_y_menu;
	const char *lng_minus_menu;
	const char *lng_minus_menu_beta;
	const char *lng_plus_menu;
	const char *lng_update_app_menu;
	const char *lng_update_pack_menu;
	const char *lng_update_firmware_menu;
	const char *lng_update_hbmenu_menu;
	const char *lng_set_90dns_menu;
	const char *lng_protect_console_menu;

// Infos display and record
	const char *lng_infos_begin;
	const char *lng_infos_is_applet;
	const char *lng_infos_is_not_applet;
	const char *lng_infos_pack_version;
	const char *lng_infos_last_pack_version;
	const char *lng_infos_console_id;
	const char *lng_infos_serial_incognito;
	const char *lng_infos_serial;
	const char *lng_infos_sysnand;
	const char *lng_infos_emunand;
	const char *lng_infos_console_model;
	const char *lng_infos_fusee_gelee_patch;
	const char *lng_infos_actual_firmware_version;
	const char *lng_infos_actual_atmosphere_version;
	const char *lng_infos_official_charge;
	const char *lng_infos_usb_charge;
	const char *lng_infos_no_charge;
	const char *lng_record_infos_log_not_open_error;
	const char *lng_record_infos_success;

// hbmenu installation
	const char *lng_hbmenu_install_begin;
	const char *lng_hbmenu_install_uninstall_begin;
	const char *lng_hbmenu_install_uninstall_error;
	const char *lng_hbmenu_install_uninstall_success;
	const char *lng_hbmenu_install_success;
	const char *lng_hbmenu_install_error;

// Battery charge needed errors
	const char *lng_battery_error_10;
	const char *lng_battery_error_20;
	const char *lng_battery_error_30;

// Questions Yes or No
	const char *lng_ask_update_firmware;
	const char *lng_ask_clean_theme;
	const char *lng_ask_clean_logos;
	const char *lng_ask_hbmenu_install;
	const char *lng_ask_validate_choices;

// Install firmware messages
	const char *lng_install_firmware_end_error;
	const char *lng_install_firmware_end_success;
	const char *lng_install_firmware_error_folder_not_found;
	const char *lng_install_firmware_folder_location;

// Daybreak-cli messages
	const char *lng_db_pause;
	const char *lng_db_atmosphere_not_found_error;
	const char *lng_db_atmosphere_outdated_error;
	const char *lng_db_nso_launch_error;
	const char *lng_db_update_not_found_error;
	const char *lng_db_get_update_infos_error;
	const char *lng_db_update_infos_fw_version;
	const char *lng_db_update_infos_exfat_support;
	const char *lng_db_update_infos_exfat_no_support;
	const char *lng_db_update_infos_fw_version_variations;
	const char *lng_db_update_validation_error;
	const char *lng_db_update_validation_success;
	const char *lng_db_update_validation_exfat_error;
	const char *lng_db_update_validation_missing_content;
	const char *lng_db_update_validation_content_error;
	const char *lng_db_install_process_setup_error;
	const char *lng_db_install_process_setup_success;
	const char *lng_db_install_process_request_preparation_error;
	const char *lng_db_install_process_preparing;
	const char *lng_db_install_process_get_preparation_result_error;
	const char *lng_db_install_process_preparation_error;
	const char *lng_db_install_process_preparation_result_error;
	const char *lng_db_install_process_preparation_success;
	const char *lng_db_install_process_applying_update;
	const char *lng_db_install_process_get_progress_error;
	const char *lng_db_install_process_applying_update_error;
	const char *lng_db_install_process_success;
	const char *lng_db_install_process_reset_to_factory_error;
	const char *lng_db_install_process_reset_to_factory_for_refurbishment_error;
	const char *lng_db_install_process_reset_to_factory_success;
	const char *lng_db_title;
	const char *lng_db_init_failed_error;
	const char *lng_db_get_hardware_type_error;
	const char *lng_db_get_rcm_bug_patch_error;
	const char *lng_db_get_emummc_status_error;
	const char *lng_db_patched_model_warning;
	const char *lng_db_update_validation;
	const char *lng_db_update_validation_failed_error;
	const char *lng_db_ask_exfat_driver_install;
	const char *lng_db_ask_fat32_if_exfat_not_supported;
	const char *lng_db_update_canceled;
	const char *lng_db_exfat_forced_but_not_supported_error;
	const char *lng_db_force_unsupported_firmware_enabled;
	const char *lng_db_ask_force_unsupported_firmware_install;
	const char *lng_db_firmware_not_supported_and_force_install_disabled;
	const char *lng_db_reset_to_factory_forced;
	const char *lng_db_ask_reset_to_factory;
	const char *lng_db_install_update_begin;

// install pack messages
	const char *lng_install_pack_recap_begin;
	const char *lng_install_pack_recap_firmware_install;
	const char *lng_install_pack_recap_not_install_firmware;
	const char *lng_install_pack_recap_clean_theme;
	const char *lng_install_pack_recap_not_clean_theme;
	const char *lng_install_pack_recap_clean_logos;
	const char *lng_install_pack_recap_not_clean_logos;
	const char *lng_install_pack_recap_install_hbmenu;
	const char *lng_install_pack_recap_not_install_hbmenu;
	const char *lng_install_pack_download_pack_error;
	const char *lng_install_pack_folder_create;
	const char *lng_install_pack_extract_file_protected;
	const char *lng_install_pack_extract_file;
	const char *lng_install_pack_file_write_error;

// Clean logos, themes and files messages
	const char *lng_clean_logos_begin;
	const char *lng_clean_theme_begin;
	const char *lng_clean_sd_begin;
	const char *lng_clean_sd_clean_unusful_files;
	const char *lng_clean_sd_finish;

// Install app messages
	const char *lng_install_app_download_app_error;

// 90DNS install messages
	const char *lng_dns_end_install_error;
	const char *lng_dns_install_set_init_error;
	const char *lng_dns_install_setsys_init_error;
	const char *lng_dns_install_get_region_code_error;
	const char *lng_dns_install_display_region_code;
	const char *lng_dns_install_use_us_dns;
	const char *lng_dns_install_use_eu_dns;
	const char *lng_dns_install_use_unknown_dns;
	const char *lng_dns_install_aplying;
	const char *lng_dns_install_get_wifi_networks_error;
	const char *lng_dns_install_networs_count;
	const char *lng_dns_install_wifi_network_config_error;
	const char *lng_dns_install_success;

// Protect console messages
	const char *lng_protect_console_begin;
	const char *lng_protect_console_error;

// download messages
	const char *lng_dl_progress_0;
	const char *lng_dl_progress_1;
	const char *lng_dl_progress_with_bar_0;
	const char *lng_dl_progress_with_bar_1;
	const char *lng_dl_begin;
	const char *lng_dl_file_write_error;
	const char *lng_dl_success;
	const char *lng_dl_dl_error;
	const char *lng_dl_open_temp_file_error;
	const char *lng_dl_curl_init_error;
} language_section;

// define a structure for holding all of the config of the ini file.
typedef struct
{
	language_section s1;
} translation;

static int translation_handler(void* config, const char * section, const char * name, const char * value)
{
	// config instance for filling in the values.
	translation* pconfig = (translation*)config;

	// define a macro for checking Sections and keys under the sections.
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	// FILE *test = fopen("/switch/AIO_LS_pack_Updater/test.log", "a");
	// fprintf(test, "%s\n%s\n\n", name, value);
	// fill the values in config struct for Section 1.
	if(MATCH("language", "lng_yes")){
		if (value != 0) {
			pconfig->s1.lng_yes = strdup(value);
		} else {
			pconfig->s1.lng_yes = "";
		}
	} else if(MATCH("language", "lng_no")){
		if (value != 0) {
			pconfig->s1.lng_no = strdup(value);
		} else {
			pconfig->s1.lng_no = "";
		}
	} else if(MATCH("language", "lng_unknown_0")){
		if (value != 0) {
			pconfig->s1.lng_unknown_0 = strdup(value);
		} else {
			pconfig->s1.lng_unknown_0 = "";
		}
	} else if(MATCH("language", "lng_unknown_1")){
		if (value != 0) {
			pconfig->s1.lng_unknown_1 = strdup(value);
		} else {
			pconfig->s1.lng_unknown_1 = "";
		}
	} else if(MATCH("language", "lng_files")){
		if (value != 0) {
			pconfig->s1.lng_files = strdup(value);
		} else {
			pconfig->s1.lng_files = "";
		}
	} else if(MATCH("language", "lng_partition")){
		if (value != 0) {
			pconfig->s1.lng_partition = strdup(value);
		} else {
			pconfig->s1.lng_partition = "";
		}
	} else if(MATCH("language", "lng_usable")){
		if (value != 0) {
			pconfig->s1.lng_usable = strdup(value);
		} else {
			pconfig->s1.lng_usable = "";
		}
	} else if(MATCH("language", "lng_not_usable")){
		if (value != 0) {
			pconfig->s1.lng_not_usable = strdup(value);
		} else {
			pconfig->s1.lng_not_usable = "";
		}
	} else if(MATCH("language", "lng_success_reboot_in_five_seconds")){
		if (value != 0) {
			pconfig->s1.lng_success_reboot_in_five_seconds = strdup(value);
		} else {
			pconfig->s1.lng_success_reboot_in_five_seconds = "";
		}
	} else if(MATCH("language", "lng_error_reboot_to_payload")){
		if (value != 0) {
			pconfig->s1.lng_error_reboot_to_payload = strdup(value);
		} else {
			pconfig->s1.lng_error_reboot_to_payload = "";
		}
	} else if(MATCH("language", "lng_error_not_enough_space_on_sd")){
		if (value != 0) {
			pconfig->s1.lng_error_not_enough_space_on_sd = strdup(value);
		} else {
			pconfig->s1.lng_error_not_enough_space_on_sd = "";
		}

	} else if(MATCH("language", "lng_title")){
		if (value != 0) {
			pconfig->s1.lng_title = strdup(value);
		} else {
			pconfig->s1.lng_title = "";
		}
	} else 	if(MATCH("language", "lng_title_beta")){
		if (value != 0) {
			pconfig->s1.lng_title_beta = strdup(value);
		} else {
			pconfig->s1.lng_title_beta = "";
		}
	} else 	if(MATCH("language", "lng_a_menu")){
		if (value != 0) {
			pconfig->s1.lng_a_menu = strdup(value);
		} else {
			pconfig->s1.lng_a_menu = "";
		}
	} else 	if(MATCH("language", "lng_x_menu")){
		if (value != 0) {
			pconfig->s1.lng_x_menu = strdup(value);
		} else {
			pconfig->s1.lng_x_menu = "";
		}
	} else 	if(MATCH("language", "lng_y_menu")){
		if (value != 0) {
			pconfig->s1.lng_y_menu = strdup(value);
		} else {
			pconfig->s1.lng_y_menu = "";
		}
	} else 	if(MATCH("language", "lng_minus_menu")){
		if (value != 0) {
			pconfig->s1.lng_minus_menu = strdup(value);
		} else {
			pconfig->s1.lng_minus_menu = "";
		}
	} else 	if(MATCH("language", "lng_minus_menu_beta")){
		if (value != 0) {
			pconfig->s1.lng_minus_menu_beta = strdup(value);
		} else {
			pconfig->s1.lng_minus_menu_beta = "";
		}
	} else 	if(MATCH("language", "lng_plus_menu")){
		if (value != 0) {
			pconfig->s1.lng_plus_menu = strdup(value);
		} else {
			pconfig->s1.lng_plus_menu = "";
		}
	} else 	if(MATCH("language", "lng_update_app_menu")){
		if (value != 0) {
			pconfig->s1.lng_update_app_menu = strdup(value);
		} else {
			pconfig->s1.lng_update_app_menu = "";
		}
	} else 	if(MATCH("language", "lng_update_pack_menu")){
		if (value != 0) {
			pconfig->s1.lng_update_pack_menu = strdup(value);
		} else {
			pconfig->s1.lng_update_pack_menu = "";
		}
	} else 	if(MATCH("language", "lng_update_firmware_menu")){
		if (value != 0) {
			pconfig->s1.lng_update_firmware_menu = strdup(value);
		} else {
			pconfig->s1.lng_update_firmware_menu = "";
		}
	} else 	if(MATCH("language", "lng_update_hbmenu_menu")){
		if (value != 0) {
			pconfig->s1.lng_update_hbmenu_menu = strdup(value);
		} else {
			pconfig->s1.lng_update_hbmenu_menu = "";
		}
	} else 	if(MATCH("language", "lng_set_90dns_menu")){
		if (value != 0) {
			pconfig->s1.lng_set_90dns_menu = strdup(value);
		} else {
			pconfig->s1.lng_set_90dns_menu = "";
		}
	} else 	if(MATCH("language", "lng_protect_console_menu")){
		if (value != 0) {
			pconfig->s1.lng_protect_console_menu = strdup(value);
		} else {
			pconfig->s1.lng_protect_console_menu = "";
		}

	} else 	if(MATCH("language", "lng_infos_begin")){
		if (value != 0) {
			pconfig->s1.lng_infos_begin = strdup(value);
		} else {
			pconfig->s1.lng_infos_begin = "";
		}
	} else 	if(MATCH("language", "lng_infos_is_applet")){
		if (value != 0) {
			pconfig->s1.lng_infos_is_applet = strdup(value);
		} else {
			pconfig->s1.lng_infos_is_applet = "";
		}
	} else 	if(MATCH("language", "lng_infos_is_not_applet")){
		if (value != 0) {
			pconfig->s1.lng_infos_is_not_applet = strdup(value);
		} else {
			pconfig->s1.lng_infos_is_not_applet = "";
		}
	} else 	if(MATCH("language", "lng_infos_pack_version")){
		if (value != 0) {
			pconfig->s1.lng_infos_pack_version = strdup(value);
		} else {
			pconfig->s1.lng_infos_pack_version = "";
		}
	} else 	if(MATCH("language", "lng_infos_last_pack_version")){
		if (value != 0) {
			pconfig->s1.lng_infos_last_pack_version = strdup(value);
		} else {
			pconfig->s1.lng_infos_last_pack_version = "";
		}
	} else 	if(MATCH("language", "lng_infos_console_id")){
		if (value != 0) {
			pconfig->s1.lng_infos_console_id = strdup(value);
		} else {
			pconfig->s1.lng_infos_console_id = "";
		}
	} else 	if(MATCH("language", "lng_infos_serial_incognito")){
		if (value != 0) {
			pconfig->s1.lng_infos_serial_incognito = strdup(value);
		} else {
			pconfig->s1.lng_infos_serial_incognito = "";
		}
	} else 	if(MATCH("language", "lng_infos_serial")){
		if (value != 0) {
			pconfig->s1.lng_infos_serial = strdup(value);
		} else {
			pconfig->s1.lng_infos_serial = "";
		}
	} else 	if(MATCH("language", "lng_infos_sysnand")){
		if (value != 0) {
			pconfig->s1.lng_infos_sysnand = strdup(value);
		} else {
			pconfig->s1.lng_infos_sysnand = "";
		}
	} else 	if(MATCH("language", "lng_infos_emunand")){
		if (value != 0) {
			pconfig->s1.lng_infos_emunand = strdup(value);
		} else {
			pconfig->s1.lng_infos_emunand = "";
		}
	} else 	if(MATCH("language", "lng_infos_console_model")){
		if (value != 0) {
			pconfig->s1.lng_infos_console_model = strdup(value);
		} else {
			pconfig->s1.lng_infos_console_model = "";
		}
	} else 	if(MATCH("language", "lng_infos_fusee_gelee_patch")){
		if (value != 0) {
			pconfig->s1.lng_infos_fusee_gelee_patch = strdup(value);
		} else {
			pconfig->s1.lng_infos_fusee_gelee_patch = "";
		}
	} else 	if(MATCH("language", "lng_infos_actual_firmware_version")){
		if (value != 0) {
			pconfig->s1.lng_infos_actual_firmware_version = strdup(value);
		} else {
			pconfig->s1.lng_infos_actual_firmware_version = "";
		}
	} else 	if(MATCH("language", "lng_infos_actual_atmosphere_version")){
		if (value != 0) {
			pconfig->s1.lng_infos_actual_atmosphere_version = strdup(value);
		} else {
			pconfig->s1.lng_infos_actual_atmosphere_version = "";
		}
	} else 	if(MATCH("language", "lng_infos_official_charge")){
		if (value != 0) {
			pconfig->s1.lng_infos_official_charge = strdup(value);
		} else {
			pconfig->s1.lng_infos_official_charge = "";
		}
	} else 	if(MATCH("language", "lng_infos_usb_charge")){
		if (value != 0) {
			pconfig->s1.lng_infos_usb_charge = strdup(value);
		} else {
			pconfig->s1.lng_infos_usb_charge = "";
		}
	} else 	if(MATCH("language", "lng_infos_no_charge")){
		if (value != 0) {
			pconfig->s1.lng_infos_no_charge = strdup(value);
		} else {
			pconfig->s1.lng_infos_no_charge = "";
		}
	} else 	if(MATCH("language", "lng_record_infos_log_not_open_error")){
		if (value != 0) {
			pconfig->s1.lng_record_infos_log_not_open_error = strdup(value);
		} else {
			pconfig->s1.lng_record_infos_log_not_open_error = "";
		}
	} else 	if(MATCH("language", "lng_record_infos_success")){
		if (value != 0) {
			pconfig->s1.lng_record_infos_success = strdup(value);
		} else {
			pconfig->s1.lng_record_infos_success = "";
		}

	} else 	if(MATCH("language", "lng_hbmenu_install_begin")){
		if (value != 0) {
			pconfig->s1.lng_hbmenu_install_begin = strdup(value);
		} else {
			pconfig->s1.lng_hbmenu_install_begin = "";
		}
	} else 	if(MATCH("language", "lng_hbmenu_install_uninstall_begin")){
		if (value != 0) {
			pconfig->s1.lng_hbmenu_install_uninstall_begin = strdup(value);
		} else {
			pconfig->s1.lng_hbmenu_install_uninstall_begin = "";
		}
	} else 	if(MATCH("language", "lng_hbmenu_install_uninstall_error")){
		if (value != 0) {
			pconfig->s1.lng_hbmenu_install_uninstall_error = strdup(value);
		} else {
			pconfig->s1.lng_hbmenu_install_uninstall_error = "";
		}
	} else 	if(MATCH("language", "lng_hbmenu_install_uninstall_success")){
		if (value != 0) {
			pconfig->s1.lng_hbmenu_install_uninstall_success = strdup(value);
		} else {
			pconfig->s1.lng_hbmenu_install_uninstall_success = "";
		}
	} else 	if(MATCH("language", "lng_hbmenu_install_success")){
		if (value != 0) {
			pconfig->s1.lng_hbmenu_install_success = strdup(value);
		} else {
			pconfig->s1.lng_hbmenu_install_success = "";
		}
	} else 	if(MATCH("language", "lng_hbmenu_install_error")){
		if (value != 0) {
			pconfig->s1.lng_hbmenu_install_error = strdup(value);
		} else {
			pconfig->s1.lng_hbmenu_install_error = "";
		}

	} else 	if(MATCH("language", "lng_battery_error_10")){
		if (value != 0) {
			pconfig->s1.lng_battery_error_10 = strdup(value);
		} else {
			pconfig->s1.lng_battery_error_10 = "";
		}
	} else 	if(MATCH("language", "lng_battery_error_20")){
		if (value != 0) {
			pconfig->s1.lng_battery_error_20 = strdup(value);
		} else {
			pconfig->s1.lng_battery_error_20 = "";
		}
	} else 	if(MATCH("language", "lng_battery_error_30")){
		if (value != 0) {
			pconfig->s1.lng_battery_error_30 = strdup(value);
		} else {
			pconfig->s1.lng_battery_error_30 = "";
		}

	} else 	if(MATCH("language", "lng_ask_update_firmware")){
		if (value != 0) {
			pconfig->s1.lng_ask_update_firmware = strdup(value);
		} else {
			pconfig->s1.lng_ask_update_firmware = "";
		}
	} else 	if(MATCH("language", "lng_ask_clean_theme")){
		if (value != 0) {
			pconfig->s1.lng_ask_clean_theme = strdup(value);
		} else {
			pconfig->s1.lng_ask_clean_theme = "";
		}
	} else 	if(MATCH("language", "lng_ask_clean_logos")){
		if (value != 0) {
			pconfig->s1.lng_ask_clean_logos = strdup(value);
		} else {
			pconfig->s1.lng_ask_clean_logos = "";
		}
	} else 	if(MATCH("language", "lng_ask_hbmenu_install")){
		if (value != 0) {
			pconfig->s1.lng_ask_hbmenu_install = strdup(value);
		} else {
			pconfig->s1.lng_ask_hbmenu_install = "";
		}
	} else 	if(MATCH("language", "lng_ask_validate_choices")){
		if (value != 0) {
			pconfig->s1.lng_ask_validate_choices = strdup(value);
		} else {
			pconfig->s1.lng_ask_validate_choices = "";
		}

	} else 	if(MATCH("language", "lng_install_firmware_end_error")){
		if (value != 0) {
			pconfig->s1.lng_install_firmware_end_error = strdup(value);
		} else {
			pconfig->s1.lng_install_firmware_end_error = "";
		}
	} else 	if(MATCH("language", "lng_install_firmware_end_success")){
		if (value != 0) {
			pconfig->s1.lng_install_firmware_end_success = strdup(value);
		} else {
			pconfig->s1.lng_install_firmware_end_success = "";
		}
	} else 	if(MATCH("language", "lng_install_firmware_error_folder_not_found")){
		if (value != 0) {
			pconfig->s1.lng_install_firmware_error_folder_not_found = strdup(value);
		} else {
			pconfig->s1.lng_install_firmware_error_folder_not_found = "";
		}
	} else 	if(MATCH("language", "lng_install_firmware_folder_location")){
		if (value != 0) {
			pconfig->s1.lng_install_firmware_folder_location = strdup(value);
		} else {
			pconfig->s1.lng_install_firmware_folder_location = "";
		}

	} else 	if(MATCH("language", "lng_db_pause")){
		if (value != 0) {
			pconfig->s1.lng_db_pause = strdup(value);
		} else {
			pconfig->s1.lng_db_pause = "";
		}
	} else 	if(MATCH("language", "lng_db_atmosphere_not_found_error")){
		if (value != 0) {
			pconfig->s1.lng_db_atmosphere_not_found_error = strdup(value);
		} else {
			pconfig->s1.lng_db_atmosphere_not_found_error = "";
		}
	} else 	if(MATCH("language", "lng_db_atmosphere_outdated_error")){
		if (value != 0) {
			pconfig->s1.lng_db_atmosphere_outdated_error = strdup(value);
		} else {
			pconfig->s1.lng_db_atmosphere_outdated_error = "";
		}
	} else 	if(MATCH("language", "lng_db_nso_launch_error")){
		if (value != 0) {
			pconfig->s1.lng_db_nso_launch_error = strdup(value);
		} else {
			pconfig->s1.lng_db_nso_launch_error = "";
		}
	} else 	if(MATCH("language", "lng_db_update_not_found_error")){
		if (value != 0) {
			pconfig->s1.lng_db_update_not_found_error = strdup(value);
		} else {
			pconfig->s1.lng_db_update_not_found_error = "";
		}
	} else 	if(MATCH("language", "lng_db_get_update_infos_error")){
		if (value != 0) {
			pconfig->s1.lng_db_get_update_infos_error = strdup(value);
		} else {
			pconfig->s1.lng_db_get_update_infos_error = "";
		}
	} else 	if(MATCH("language", "lng_db_update_infos_fw_version")){
		if (value != 0) {
			pconfig->s1.lng_db_update_infos_fw_version = strdup(value);
		} else {
			pconfig->s1.lng_db_update_infos_fw_version = "";
		}
	} else 	if(MATCH("language", "lng_db_update_infos_exfat_support")){
		if (value != 0) {
			pconfig->s1.lng_db_update_infos_exfat_support = strdup(value);
		} else {
			pconfig->s1.lng_db_update_infos_exfat_support = "";
		}
	} else 	if(MATCH("language", "lng_db_update_infos_exfat_no_support")){
		if (value != 0) {
			pconfig->s1.lng_db_update_infos_exfat_no_support = strdup(value);
		} else {
			pconfig->s1.lng_db_update_infos_exfat_no_support = "";
		}
	} else 	if(MATCH("language", "lng_db_update_infos_fw_version_variations")){
		if (value != 0) {
			pconfig->s1.lng_db_update_infos_fw_version_variations = strdup(value);
		} else {
			pconfig->s1.lng_db_update_infos_fw_version_variations = "";
		}
	} else 	if(MATCH("language", "lng_db_update_validation_error")){
		if (value != 0) {
			pconfig->s1.lng_db_update_validation_error = strdup(value);
		} else {
			pconfig->s1.lng_db_update_validation_error = "";
		}
	} else 	if(MATCH("language", "lng_db_update_validation_success")){
		if (value != 0) {
			pconfig->s1.lng_db_update_validation_success = strdup(value);
		} else {
			pconfig->s1.lng_db_update_validation_success = "";
		}
	} else 	if(MATCH("language", "lng_db_update_validation_exfat_error")){
		if (value != 0) {
			pconfig->s1.lng_db_update_validation_exfat_error = strdup(value);
		} else {
			pconfig->s1.lng_db_update_validation_exfat_error = "";
		}
	} else 	if(MATCH("language", "lng_db_update_validation_missing_content")){
		if (value != 0) {
			pconfig->s1.lng_db_update_validation_missing_content = strdup(value);
		} else {
			pconfig->s1.lng_db_update_validation_missing_content = "";
		}
	} else 	if(MATCH("language", "lng_db_update_validation_content_error")){
		if (value != 0) {
			pconfig->s1.lng_db_update_validation_content_error = strdup(value);
		} else {
			pconfig->s1.lng_db_update_validation_content_error = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_setup_error")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_setup_error = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_setup_error = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_setup_success")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_setup_success = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_setup_success = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_request_preparation_error")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_request_preparation_error = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_request_preparation_error = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_preparing")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_preparing = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_preparing = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_get_preparation_result_error")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_get_preparation_result_error = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_get_preparation_result_error = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_preparation_error")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_preparation_error = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_preparation_error = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_preparation_result_error")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_preparation_result_error = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_preparation_result_error = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_preparation_success")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_preparation_success = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_preparation_success = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_applying_update")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_applying_update = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_applying_update = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_get_progress_error")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_get_progress_error = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_get_progress_error = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_applying_update_error")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_applying_update_error = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_applying_update_error = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_success")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_success = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_success = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_reset_to_factory_error")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_reset_to_factory_error = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_reset_to_factory_error = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_reset_to_factory_for_refurbishment_error")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_reset_to_factory_for_refurbishment_error = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_reset_to_factory_for_refurbishment_error = "";
		}
	} else 	if(MATCH("language", "lng_db_install_process_reset_to_factory_success")){
		if (value != 0) {
			pconfig->s1.lng_db_install_process_reset_to_factory_success = strdup(value);
		} else {
			pconfig->s1.lng_db_install_process_reset_to_factory_success = "";
		}
	} else 	if(MATCH("language", "lng_db_title")){
		if (value != 0) {
			pconfig->s1.lng_db_title = strdup(value);
		} else {
			pconfig->s1.lng_db_title = "";
		}
	} else 	if(MATCH("language", "lng_db_init_failed_error")){
		if (value != 0) {
			pconfig->s1.lng_db_init_failed_error = strdup(value);
		} else {
			pconfig->s1.lng_db_init_failed_error = "";
		}
	} else 	if(MATCH("language", "lng_db_get_hardware_type_error")){
		if (value != 0) {
			pconfig->s1.lng_db_get_hardware_type_error = strdup(value);
		} else {
			pconfig->s1.lng_db_get_hardware_type_error = "";
		}
	} else 	if(MATCH("language", "lng_db_get_rcm_bug_patch_error")){
		if (value != 0) {
			pconfig->s1.lng_db_get_rcm_bug_patch_error = strdup(value);
		} else {
			pconfig->s1.lng_db_get_rcm_bug_patch_error = "";
		}
	} else 	if(MATCH("language", "lng_db_get_emummc_status_error")){
		if (value != 0) {
			pconfig->s1.lng_db_get_emummc_status_error = strdup(value);
		} else {
			pconfig->s1.lng_db_get_emummc_status_error = "";
		}
	} else 	if(MATCH("language", "lng_db_patched_model_warning")){
		if (value != 0) {
			pconfig->s1.lng_db_patched_model_warning = strdup(value);
		} else {
			pconfig->s1.lng_db_patched_model_warning = "";
		}
	} else 	if(MATCH("language", "lng_db_update_validation")){
		if (value != 0) {
			pconfig->s1.lng_db_update_validation = strdup(value);
		} else {
			pconfig->s1.lng_db_update_validation = "";
		}
	} else 	if(MATCH("language", "lng_db_update_validation_failed_error")){
		if (value != 0) {
			pconfig->s1.lng_db_update_validation_failed_error = strdup(value);
		} else {
			pconfig->s1.lng_db_update_validation_failed_error = "";
		}
	} else 	if(MATCH("language", "lng_db_ask_exfat_driver_install")){
		if (value != 0) {
			pconfig->s1.lng_db_ask_exfat_driver_install = strdup(value);
		} else {
			pconfig->s1.lng_db_ask_exfat_driver_install = "";
		}
	} else 	if(MATCH("language", "lng_db_ask_fat32_if_exfat_not_supported")){
		if (value != 0) {
			pconfig->s1.lng_db_ask_fat32_if_exfat_not_supported = strdup(value);
		} else {
			pconfig->s1.lng_db_ask_fat32_if_exfat_not_supported = "";
		}
	} else 	if(MATCH("language", "lng_db_update_canceled")){
		if (value != 0) {
			pconfig->s1.lng_db_update_canceled = strdup(value);
		} else {
			pconfig->s1.lng_db_update_canceled = "";
		}
	} else 	if(MATCH("language", "lng_db_exfat_forced_but_not_supported_error")){
		if (value != 0) {
			pconfig->s1.lng_db_exfat_forced_but_not_supported_error = strdup(value);
		} else {
			pconfig->s1.lng_db_exfat_forced_but_not_supported_error = "";
		}
	} else 	if(MATCH("language", "lng_db_force_unsupported_firmware_enabled")){
		if (value != 0) {
			pconfig->s1.lng_db_force_unsupported_firmware_enabled = strdup(value);
		} else {
			pconfig->s1.lng_db_force_unsupported_firmware_enabled = "";
		}
	} else 	if(MATCH("language", "lng_db_ask_force_unsupported_firmware_install")){
		if (value != 0) {
			pconfig->s1.lng_db_ask_force_unsupported_firmware_install = strdup(value);
		} else {
			pconfig->s1.lng_db_ask_force_unsupported_firmware_install = "";
		}
	} else 	if(MATCH("language", "lng_db_firmware_not_supported_and_force_install_disabled")){
		if (value != 0) {
			pconfig->s1.lng_db_firmware_not_supported_and_force_install_disabled = strdup(value);
		} else {
			pconfig->s1.lng_db_firmware_not_supported_and_force_install_disabled = "";
		}
	} else 	if(MATCH("language", "lng_db_reset_to_factory_forced")){
		if (value != 0) {
			pconfig->s1.lng_db_reset_to_factory_forced = strdup(value);
		} else {
			pconfig->s1.lng_db_reset_to_factory_forced = "";
		}
	} else 	if(MATCH("language", "lng_db_ask_reset_to_factory")){
		if (value != 0) {
			pconfig->s1.lng_db_ask_reset_to_factory = strdup(value);
		} else {
			pconfig->s1.lng_db_ask_reset_to_factory = "";
		}
	} else 	if(MATCH("language", "lng_db_install_update_begin")){
		if (value != 0) {
			pconfig->s1.lng_db_install_update_begin = strdup(value);
		} else {
			pconfig->s1.lng_db_install_update_begin = "";
		}

	} else 	if(MATCH("language", "lng_install_pack_recap_begin")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_recap_begin = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_recap_begin = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_recap_firmware_install")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_recap_firmware_install = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_recap_firmware_install = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_recap_not_install_firmware")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_recap_not_install_firmware = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_recap_not_install_firmware = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_recap_clean_theme")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_recap_clean_theme = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_recap_clean_theme = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_recap_not_clean_theme")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_recap_not_clean_theme = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_recap_not_clean_theme = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_recap_clean_logos")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_recap_clean_logos = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_recap_clean_logos = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_recap_not_clean_logos")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_recap_not_clean_logos = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_recap_not_clean_logos = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_recap_install_hbmenu")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_recap_install_hbmenu = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_recap_install_hbmenu = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_recap_not_install_hbmenu")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_recap_not_install_hbmenu = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_recap_not_install_hbmenu = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_download_pack_error")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_download_pack_error = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_download_pack_error = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_folder_create")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_folder_create = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_folder_create = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_extract_file_protected")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_extract_file_protected = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_extract_file_protected = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_extract_file")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_extract_file = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_extract_file = "";
		}
	} else 	if(MATCH("language", "lng_install_pack_file_write_error")){
		if (value != 0) {
			pconfig->s1.lng_install_pack_file_write_error = strdup(value);
		} else {
			pconfig->s1.lng_install_pack_file_write_error = "";
		}

	} else 	if(MATCH("language", "lng_clean_logos_begin")){
		if (value != 0) {
			pconfig->s1.lng_clean_logos_begin = strdup(value);
		} else {
			pconfig->s1.lng_clean_logos_begin = "";
		}
	} else 	if(MATCH("language", "lng_clean_theme_begin")){
		if (value != 0) {
			pconfig->s1.lng_clean_theme_begin = strdup(value);
		} else {
			pconfig->s1.lng_clean_theme_begin = "";
		}
	} else 	if(MATCH("language", "lng_clean_sd_begin")){
		if (value != 0) {
			pconfig->s1.lng_clean_sd_begin = strdup(value);
		} else {
			pconfig->s1.lng_clean_sd_begin = "";
		}
	} else 	if(MATCH("language", "lng_clean_sd_clean_unusful_files")){
		if (value != 0) {
			pconfig->s1.lng_clean_sd_clean_unusful_files = strdup(value);
		} else {
			pconfig->s1.lng_clean_sd_clean_unusful_files = "";
		}
	} else 	if(MATCH("language", "lng_clean_sd_finish")){
		if (value != 0) {
			pconfig->s1.lng_clean_sd_finish = strdup(value);
		} else {
			pconfig->s1.lng_clean_sd_finish = "";
		}

	} else 	if(MATCH("language", "lng_install_app_download_app_error")){
		if (value != 0) {
			pconfig->s1.lng_install_app_download_app_error = strdup(value);
		} else {
			pconfig->s1.lng_install_app_download_app_error = "";
		}

	} else 	if(MATCH("language", "lng_dns_end_install_error")){
		if (value != 0) {
			pconfig->s1.lng_dns_end_install_error = strdup(value);
		} else {
			pconfig->s1.lng_dns_end_install_error = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_set_init_error")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_set_init_error = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_set_init_error = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_setsys_init_error")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_setsys_init_error = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_setsys_init_error = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_get_region_code_error")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_get_region_code_error = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_get_region_code_error = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_display_region_code")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_display_region_code = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_display_region_code = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_use_us_dns")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_use_us_dns = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_use_us_dns = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_use_eu_dns")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_use_eu_dns = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_use_eu_dns = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_use_unknown_dns")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_use_unknown_dns = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_use_unknown_dns = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_aplying")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_aplying = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_aplying = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_get_wifi_networks_error")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_get_wifi_networks_error = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_get_wifi_networks_error = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_networs_count")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_networs_count = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_networs_count = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_wifi_network_config_error")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_wifi_network_config_error = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_wifi_network_config_error = "";
		}
	} else 	if(MATCH("language", "lng_dns_install_success")){
		if (value != 0) {
			pconfig->s1.lng_dns_install_success = strdup(value);
		} else {
			pconfig->s1.lng_dns_install_success = "";
		}

	} else 	if(MATCH("language", "lng_protect_console_begin")){
		if (value != 0) {
			pconfig->s1.lng_protect_console_begin = strdup(value);
		} else {
			pconfig->s1.lng_protect_console_begin = "";
		}
	} else 	if(MATCH("language", "lng_protect_console_error")){
		if (value != 0) {
			pconfig->s1.lng_protect_console_error = strdup(value);
		} else {
			pconfig->s1.lng_protect_console_error = "";
		}

	} else 	if(MATCH("language", "lng_dl_progress_0")){
		if (value != 0) {
			pconfig->s1.lng_dl_progress_0 = strdup(value);
		} else {
			pconfig->s1.lng_dl_progress_0 = "";
		}
	} else 	if(MATCH("language", "lng_dl_progress_1")){
		if (value != 0) {
			pconfig->s1.lng_dl_progress_1 = strdup(value);
		} else {
			pconfig->s1.lng_dl_progress_1 = "";
		}
	} else 	if(MATCH("language", "lng_dl_progress_with_bar_0")){
		if (value != 0) {
			pconfig->s1.lng_dl_progress_with_bar_0 = strdup(value);
		} else {
			pconfig->s1.lng_dl_progress_with_bar_0 = "";
		}
	} else 	if(MATCH("language", "lng_dl_progress_with_bar_1")){
		if (value != 0) {
			pconfig->s1.lng_dl_progress_with_bar_1 = strdup(value);
		} else {
			pconfig->s1.lng_dl_progress_with_bar_1 = "";
		}
	} else 	if(MATCH("language", "lng_dl_begin")){
		if (value != 0) {
			pconfig->s1.lng_dl_begin = strdup(value);
		} else {
			pconfig->s1.lng_dl_begin = "";
		}
	} else 	if(MATCH("language", "lng_dl_file_write_error")){
		if (value != 0) {
			pconfig->s1.lng_dl_file_write_error = strdup(value);
		} else {
			pconfig->s1.lng_dl_file_write_error = "";
		}
	} else 	if(MATCH("language", "lng_dl_success")){
		if (value != 0) {
			pconfig->s1.lng_dl_success = strdup(value);
		} else {
			pconfig->s1.lng_dl_success = "";
		}
	} else 	if(MATCH("language", "lng_dl_dl_error")){
		if (value != 0) {
			pconfig->s1.lng_dl_dl_error = strdup(value);
		} else {
			pconfig->s1.lng_dl_dl_error = "";
		}
	} else 	if(MATCH("language", "lng_dl_open_temp_file_error")){
		if (value != 0) {
			pconfig->s1.lng_dl_open_temp_file_error = strdup(value);
		} else {
			pconfig->s1.lng_dl_open_temp_file_error = "";
		}
	} else 	if(MATCH("language", "lng_dl_curl_init_error")){
		if (value != 0) {
			pconfig->s1.lng_dl_curl_init_error = strdup(value);
		} else {
			pconfig->s1.lng_dl_curl_init_error = "";
		}

	}else{
		// fprintf(test, "Error\n");
		// fclose(test);
		return 0;
	}
	// fprintf(test, "Pass\n");
	// fclose(test);
	return 1;
}

lng set_translation_strings() {
	translation config;
	INIT_LNG(language_temp);
	FILE *test_ini;
	char file_path[FS_MAX_PATH];
	test_ini = fopen("/switch/AIO_LS_pack_Updater/translate.ini", "r");
	if (test_ini != NULL) {
		fclose(test_ini);
		strcpy(file_path, "/switch/AIO_LS_pack_Updater/translate.ini");
	} else {
		setInitialize();
		u64 language_code = 0;
		setGetSystemLanguage(&language_code);
		setExit();
		strcpy(file_path, "romfs:/lng/");
		strcat(strcat(file_path, (char *) &language_code), ".ini");
		test_ini = fopen(file_path, "r");
		if (test_ini != NULL) {
			fclose(test_ini);
		} else {
			strcpy(file_path, "romfs:/lng/en_US.ini");
			test_ini = fopen(file_path, "r");
			if (test_ini != NULL) {
				fclose(test_ini);
			} else {
				return language_temp;
			}
		}
	}
	// parse the .ini file
	if (ini_parse(file_path, translation_handler, &config) == 0) {
		if (strcmp(config.s1.lng_yes, "") != 0) {
			strcpy(language_temp.lng_yes, config.s1.lng_yes);
			free((void*)config.s1.lng_yes);
		}
		if (strcmp(config.s1.lng_no, "") != 0) {
			strcpy(language_temp.lng_no, config.s1.lng_no);
			free((void*)config.s1.lng_no);
		}
		if (strcmp(config.s1.lng_unknown_0, "") != 0) {
			strcpy(language_temp.lng_unknown_0, config.s1.lng_unknown_0);
			free((void*)config.s1.lng_unknown_0);
		}
		if (strcmp(config.s1.lng_unknown_1, "") != 0) {
			strcpy(language_temp.lng_unknown_1, config.s1.lng_unknown_1);
			free((void*)config.s1.lng_unknown_1);
		}
		if (strcmp(config.s1.lng_files, "") != 0) {
			strcpy(language_temp.lng_files, config.s1.lng_files);
			free((void*)config.s1.lng_files);
		}
		if (strcmp(config.s1.lng_partition, "") != 0) {
			strcpy(language_temp.lng_partition, config.s1.lng_partition);
			free((void*)config.s1.lng_partition);
		}
		if (strcmp(config.s1.lng_usable, "") != 0) {
			strcpy(language_temp.lng_usable, config.s1.lng_usable);
			free((void*)config.s1.lng_usable);
		}
		if (strcmp(config.s1.lng_not_usable, "") != 0) {
			strcpy(language_temp.lng_not_usable, config.s1.lng_not_usable);
			free((void*)config.s1.lng_not_usable);
		}
		if (strcmp(config.s1.lng_success_reboot_in_five_seconds, "") != 0) {
			strcpy(language_temp.lng_success_reboot_in_five_seconds, config.s1.lng_success_reboot_in_five_seconds);
			free((void*)config.s1.lng_success_reboot_in_five_seconds);
		}
		if (strcmp(config.s1.lng_error_reboot_to_payload, "") != 0) {
			strcpy(language_temp.lng_error_reboot_to_payload, config.s1.lng_error_reboot_to_payload);
			free((void*)config.s1.lng_error_reboot_to_payload);
		}
		if (strcmp(config.s1.lng_error_not_enough_space_on_sd, "") != 0) {
			strcpy(language_temp.lng_error_not_enough_space_on_sd, config.s1.lng_error_not_enough_space_on_sd);
			free((void*)config.s1.lng_error_not_enough_space_on_sd);
		}

		if (strcmp(config.s1.lng_title, "") != 0) {
			strcpy(language_temp.lng_title, config.s1.lng_title);
			free((void*)config.s1.lng_title);
		}
		if (strcmp(config.s1.lng_title_beta, "") != 0) {
			strcpy(language_temp.lng_title_beta, config.s1.lng_title_beta);
			free((void*)config.s1.lng_title_beta);
		}

		if (strcmp(config.s1.lng_a_menu, "") != 0) {
			strcpy(language_temp.lng_a_menu, config.s1.lng_a_menu);
			free((void*)config.s1.lng_a_menu);
		}
		if (strcmp(config.s1.lng_x_menu, "") != 0) {
			strcpy(language_temp.lng_x_menu, config.s1.lng_x_menu);
			free((void*)config.s1.lng_x_menu);
		}
		if (strcmp(config.s1.lng_y_menu, "") != 0) {
			strcpy(language_temp.lng_y_menu, config.s1.lng_y_menu);
			free((void*)config.s1.lng_y_menu);
		}
		if (strcmp(config.s1.lng_minus_menu, "") != 0) {
			strcpy(language_temp.lng_minus_menu, config.s1.lng_minus_menu);
			free((void*)config.s1.lng_minus_menu);
		}
		if (strcmp(config.s1.lng_minus_menu_beta, "") != 0) {
			strcpy(language_temp.lng_minus_menu_beta, config.s1.lng_minus_menu_beta);
			free((void*)config.s1.lng_minus_menu_beta);
		}
		if (strcmp(config.s1.lng_plus_menu, "") != 0) {
			strcpy(language_temp.lng_plus_menu, config.s1.lng_plus_menu);
			free((void*)config.s1.lng_plus_menu);
		}
		if (strcmp(config.s1.lng_update_app_menu, "") != 0) {
			strcpy(language_temp.lng_update_app_menu, config.s1.lng_update_app_menu);
			free((void*)config.s1.lng_update_app_menu);
		}
		if (strcmp(config.s1.lng_update_pack_menu, "") != 0) {
			strcpy(language_temp.lng_update_pack_menu, config.s1.lng_update_pack_menu);
			free((void*)config.s1.lng_update_pack_menu);
		}
		if (strcmp(config.s1.lng_update_firmware_menu, "") != 0) {
			strcpy(language_temp.lng_update_firmware_menu, config.s1.lng_update_firmware_menu);
			free((void*)config.s1.lng_update_firmware_menu);
		}
		if (strcmp(config.s1.lng_update_hbmenu_menu, "") != 0) {
			strcpy(language_temp.lng_update_hbmenu_menu, config.s1.lng_update_hbmenu_menu);
			free((void*)config.s1.lng_update_hbmenu_menu);
		}
		if (strcmp(config.s1.lng_set_90dns_menu, "") != 0) {
			strcpy(language_temp.lng_set_90dns_menu, config.s1.lng_set_90dns_menu);
			free((void*)config.s1.lng_set_90dns_menu);
		}
		if (strcmp(config.s1.lng_protect_console_menu, "") != 0) {
			strcpy(language_temp.lng_protect_console_menu, config.s1.lng_protect_console_menu);
			free((void*)config.s1.lng_protect_console_menu);
		}

		if (strcmp(config.s1.lng_infos_begin, "") != 0) {
			strcpy(language_temp.lng_infos_begin, config.s1.lng_infos_begin);
			free((void*)config.s1.lng_infos_begin);
		}
		if (strcmp(config.s1.lng_infos_is_applet, "") != 0) {
			strcpy(language_temp.lng_infos_is_applet, config.s1.lng_infos_is_applet);
			free((void*)config.s1.lng_infos_is_applet);
		}
		if (strcmp(config.s1.lng_infos_is_not_applet, "") != 0) {
			strcpy(language_temp.lng_infos_is_not_applet, config.s1.lng_infos_is_not_applet);
			free((void*)config.s1.lng_infos_is_not_applet);
		}
		if (strcmp(config.s1.lng_infos_pack_version, "") != 0) {
			strcpy(language_temp.lng_infos_pack_version, config.s1.lng_infos_pack_version);
			free((void*)config.s1.lng_infos_pack_version);
		}
		if (strcmp(config.s1.lng_infos_last_pack_version, "") != 0) {
			strcpy(language_temp.lng_infos_last_pack_version, config.s1.lng_infos_last_pack_version);
			free((void*)config.s1.lng_infos_last_pack_version);
		}
		if (strcmp(config.s1.lng_infos_console_id, "") != 0) {
			strcpy(language_temp.lng_infos_console_id, config.s1.lng_infos_console_id);
			free((void*)config.s1.lng_infos_console_id);
		}
		if (strcmp(config.s1.lng_infos_serial_incognito, "") != 0) {
			strcpy(language_temp.lng_infos_serial_incognito, config.s1.lng_infos_serial_incognito);
			free((void*)config.s1.lng_infos_serial_incognito);
		}
		if (strcmp(config.s1.lng_infos_serial, "") != 0) {
			strcpy(language_temp.lng_infos_serial, config.s1.lng_infos_serial);
			free((void*)config.s1.lng_infos_serial);
		}
		if (strcmp(config.s1.lng_infos_sysnand, "") != 0) {
			strcpy(language_temp.lng_infos_sysnand, config.s1.lng_infos_sysnand);
			free((void*)config.s1.lng_infos_sysnand);
		}
		if (strcmp(config.s1.lng_infos_emunand, "") != 0) {
			strcpy(language_temp.lng_infos_emunand, config.s1.lng_infos_emunand);
			free((void*)config.s1.lng_infos_emunand);
		}
		if (strcmp(config.s1.lng_infos_console_model, "") != 0) {
			strcpy(language_temp.lng_infos_console_model, config.s1.lng_infos_console_model);
			free((void*)config.s1.lng_infos_console_model);
		}
		if (strcmp(config.s1.lng_infos_fusee_gelee_patch, "") != 0) {
			strcpy(language_temp.lng_infos_fusee_gelee_patch, config.s1.lng_infos_fusee_gelee_patch);
			free((void*)config.s1.lng_infos_fusee_gelee_patch);
		}
		if (strcmp(config.s1.lng_infos_actual_firmware_version, "") != 0) {
			strcpy(language_temp.lng_infos_actual_firmware_version, config.s1.lng_infos_actual_firmware_version);
			free((void*)config.s1.lng_infos_actual_firmware_version);
		}
		if (strcmp(config.s1.lng_infos_actual_atmosphere_version, "") != 0) {
			strcpy(language_temp.lng_infos_actual_atmosphere_version, config.s1.lng_infos_actual_atmosphere_version);
			free((void*)config.s1.lng_infos_actual_atmosphere_version);
		}
		if (strcmp(config.s1.lng_infos_official_charge, "") != 0) {
			strcpy(language_temp.lng_infos_official_charge, config.s1.lng_infos_official_charge);
			free((void*)config.s1.lng_infos_official_charge);
		}
		if (strcmp(config.s1.lng_infos_usb_charge, "") != 0) {
			strcpy(language_temp.lng_infos_usb_charge, config.s1.lng_infos_usb_charge);
			free((void*)config.s1.lng_infos_usb_charge);
		}
		if (strcmp(config.s1.lng_infos_no_charge, "") != 0) {
			strcpy(language_temp.lng_infos_no_charge, config.s1.lng_infos_no_charge);
			free((void*)config.s1.lng_infos_no_charge);
		}
		if (strcmp(config.s1.lng_record_infos_log_not_open_error, "") != 0) {
			strcpy(language_temp.lng_record_infos_log_not_open_error, config.s1.lng_record_infos_log_not_open_error);
			free((void*)config.s1.lng_record_infos_log_not_open_error);
		}
		if (strcmp(config.s1.lng_record_infos_success, "") != 0) {
			strcpy(language_temp.lng_record_infos_success, config.s1.lng_record_infos_success);
			free((void*)config.s1.lng_record_infos_success);
		}

		if (strcmp(config.s1.lng_hbmenu_install_begin, "") != 0) {
			strcpy(language_temp.lng_hbmenu_install_begin, config.s1.lng_hbmenu_install_begin);
			free((void*)config.s1.lng_hbmenu_install_begin);
		}
		if (strcmp(config.s1.lng_hbmenu_install_uninstall_begin, "") != 0) {
			strcpy(language_temp.lng_hbmenu_install_uninstall_begin, config.s1.lng_hbmenu_install_uninstall_begin);
			free((void*)config.s1.lng_hbmenu_install_uninstall_begin);
		}
		if (strcmp(config.s1.lng_hbmenu_install_uninstall_error, "") != 0) {
			strcpy(language_temp.lng_hbmenu_install_uninstall_error, config.s1.lng_hbmenu_install_uninstall_error);
			free((void*)config.s1.lng_hbmenu_install_uninstall_error);
		}
		if (strcmp(config.s1.lng_hbmenu_install_uninstall_success, "") != 0) {
			strcpy(language_temp.lng_hbmenu_install_uninstall_success, config.s1.lng_hbmenu_install_uninstall_success);
			free((void*)config.s1.lng_hbmenu_install_uninstall_success);
		}
		if (strcmp(config.s1.lng_hbmenu_install_success, "") != 0) {
			strcpy(language_temp.lng_hbmenu_install_success, config.s1.lng_hbmenu_install_success);
			free((void*)config.s1.lng_hbmenu_install_success);
		}
		if (strcmp(config.s1.lng_hbmenu_install_error, "") != 0) {
			strcpy(language_temp.lng_hbmenu_install_error, config.s1.lng_hbmenu_install_error);
			free((void*)config.s1.lng_hbmenu_install_error);
		}

		if (strcmp(config.s1.lng_battery_error_10, "") != 0) {
			strcpy(language_temp.lng_battery_error_10, config.s1.lng_battery_error_10);
			free((void*)config.s1.lng_battery_error_10);
		}
		if (strcmp(config.s1.lng_battery_error_20, "") != 0) {
			strcpy(language_temp.lng_battery_error_20, config.s1.lng_battery_error_20);
			free((void*)config.s1.lng_battery_error_20);
		}
		if (strcmp(config.s1.lng_battery_error_30, "") != 0) {
			strcpy(language_temp.lng_battery_error_30, config.s1.lng_battery_error_30);
			free((void*)config.s1.lng_battery_error_30);
		}

		if (strcmp(config.s1.lng_ask_update_firmware, "") != 0) {
			strcpy(language_temp.lng_ask_update_firmware, config.s1.lng_ask_update_firmware);
			free((void*)config.s1.lng_ask_update_firmware);
		}
		if (strcmp(config.s1.lng_ask_clean_theme, "") != 0) {
			strcpy(language_temp.lng_ask_clean_theme, config.s1.lng_ask_clean_theme);
			free((void*)config.s1.lng_ask_clean_theme);
		}
		if (strcmp(config.s1.lng_ask_clean_logos, "") != 0) {
			strcpy(language_temp.lng_ask_clean_logos, config.s1.lng_ask_clean_logos);
			free((void*)config.s1.lng_ask_clean_logos);
		}
		if (strcmp(config.s1.lng_ask_hbmenu_install, "") != 0) {
			strcpy(language_temp.lng_ask_hbmenu_install, config.s1.lng_ask_hbmenu_install);
			free((void*)config.s1.lng_ask_hbmenu_install);
		}
		if (strcmp(config.s1.lng_ask_validate_choices, "") != 0) {
			strcpy(language_temp.lng_ask_validate_choices, config.s1.lng_ask_validate_choices);
			free((void*)config.s1.lng_ask_validate_choices);
		}

		if (strcmp(config.s1.lng_install_firmware_end_error, "") != 0) {
			strcpy(language_temp.lng_install_firmware_end_error, config.s1.lng_install_firmware_end_error);
			free((void*)config.s1.lng_install_firmware_end_error);
		}
		if (strcmp(config.s1.lng_install_firmware_end_success, "") != 0) {
			strcpy(language_temp.lng_install_firmware_end_success, config.s1.lng_install_firmware_end_success);
			free((void*)config.s1.lng_install_firmware_end_success);
		}
		if (strcmp(config.s1.lng_install_firmware_error_folder_not_found, "") != 0) {
			strcpy(language_temp.lng_install_firmware_error_folder_not_found, config.s1.lng_install_firmware_error_folder_not_found);
			free((void*)config.s1.lng_install_firmware_error_folder_not_found);
		}
		if (strcmp(config.s1.lng_install_firmware_folder_location, "") != 0) {
			strcpy(language_temp.lng_install_firmware_folder_location, config.s1.lng_install_firmware_folder_location);
			free((void*)config.s1.lng_install_firmware_folder_location);
		}

		if (strcmp(config.s1.lng_db_pause, "") != 0) {
			strcpy(language_temp.lng_db_pause, config.s1.lng_db_pause);
			free((void*)config.s1.lng_db_pause);
		}
		if (strcmp(config.s1.lng_db_atmosphere_not_found_error, "") != 0) {
			strcpy(language_temp.lng_db_atmosphere_not_found_error, config.s1.lng_db_atmosphere_not_found_error);
			free((void*)config.s1.lng_db_atmosphere_not_found_error);
		}
		if (strcmp(config.s1.lng_db_atmosphere_outdated_error, "") != 0) {
			strcpy(language_temp.lng_db_atmosphere_outdated_error, config.s1.lng_db_atmosphere_outdated_error);
			free((void*)config.s1.lng_db_atmosphere_outdated_error);
		}
		if (strcmp(config.s1.lng_db_nso_launch_error, "") != 0) {
			strcpy(language_temp.lng_db_nso_launch_error, config.s1.lng_db_nso_launch_error);
			free((void*)config.s1.lng_db_nso_launch_error);
		}
		if (strcmp(config.s1.lng_db_update_not_found_error, "") != 0) {
			strcpy(language_temp.lng_db_update_not_found_error, config.s1.lng_db_update_not_found_error);
			free((void*)config.s1.lng_db_update_not_found_error);
		}
		if (strcmp(config.s1.lng_db_get_update_infos_error, "") != 0) {
			strcpy(language_temp.lng_db_get_update_infos_error, config.s1.lng_db_get_update_infos_error);
			free((void*)config.s1.lng_db_get_update_infos_error);
		}
		if (strcmp(config.s1.lng_db_update_infos_fw_version, "") != 0) {
			strcpy(language_temp.lng_db_update_infos_fw_version, config.s1.lng_db_update_infos_fw_version);
			free((void*)config.s1.lng_db_update_infos_fw_version);
		}
		if (strcmp(config.s1.lng_db_update_infos_exfat_support, "") != 0) {
			strcpy(language_temp.lng_db_update_infos_exfat_support, config.s1.lng_db_update_infos_exfat_support);
			free((void*)config.s1.lng_db_update_infos_exfat_support);
		}
		if (strcmp(config.s1.lng_db_update_infos_exfat_no_support, "") != 0) {
			strcpy(language_temp.lng_db_update_infos_exfat_no_support, config.s1.lng_db_update_infos_exfat_no_support);
			free((void*)config.s1.lng_db_update_infos_exfat_no_support);
		}
		if (strcmp(config.s1.lng_db_update_infos_fw_version_variations, "") != 0) {
			strcpy(language_temp.lng_db_update_infos_fw_version_variations, config.s1.lng_db_update_infos_fw_version_variations);
			free((void*)config.s1.lng_db_update_infos_fw_version_variations);
		}
		if (strcmp(config.s1.lng_db_update_validation_error, "") != 0) {
			strcpy(language_temp.lng_db_update_validation_error, config.s1.lng_db_update_validation_error);
			free((void*)config.s1.lng_db_update_validation_error);
		}
		if (strcmp(config.s1.lng_db_update_validation_success, "") != 0) {
			strcpy(language_temp.lng_db_update_validation_success, config.s1.lng_db_update_validation_success);
			free((void*)config.s1.lng_db_update_validation_success);
		}
		if (strcmp(config.s1.lng_db_update_validation_exfat_error, "") != 0) {
			strcpy(language_temp.lng_db_update_validation_exfat_error, config.s1.lng_db_update_validation_exfat_error);
			free((void*)config.s1.lng_db_update_validation_exfat_error);
		}
		if (strcmp(config.s1.lng_db_update_validation_missing_content, "") != 0) {
			strcpy(language_temp.lng_db_update_validation_missing_content, config.s1.lng_db_update_validation_missing_content);
			free((void*)config.s1.lng_db_update_validation_missing_content);
		}
		if (strcmp(config.s1.lng_db_update_validation_content_error, "") != 0) {
			strcpy(language_temp.lng_db_update_validation_content_error, config.s1.lng_db_update_validation_content_error);
			free((void*)config.s1.lng_db_update_validation_content_error);
		}
		if (strcmp(config.s1.lng_db_install_process_setup_error, "") != 0) {
			strcpy(language_temp.lng_db_install_process_setup_error, config.s1.lng_db_install_process_setup_error);
			free((void*)config.s1.lng_db_install_process_setup_error);
		}
		if (strcmp(config.s1.lng_db_install_process_setup_success, "") != 0) {
			strcpy(language_temp.lng_db_install_process_setup_success, config.s1.lng_db_install_process_setup_success);
			free((void*)config.s1.lng_db_install_process_setup_success);
		}
		if (strcmp(config.s1.lng_db_install_process_request_preparation_error, "") != 0) {
			strcpy(language_temp.lng_db_install_process_request_preparation_error, config.s1.lng_db_install_process_request_preparation_error);
			free((void*)config.s1.lng_db_install_process_request_preparation_error);
		}
		if (strcmp(config.s1.lng_db_install_process_preparing, "") != 0) {
			strcpy(language_temp.lng_db_install_process_preparing, config.s1.lng_db_install_process_preparing);
			free((void*)config.s1.lng_db_install_process_preparing);
		}
		if (strcmp(config.s1.lng_db_install_process_get_preparation_result_error, "") != 0) {
			strcpy(language_temp.lng_db_install_process_get_preparation_result_error, config.s1.lng_db_install_process_get_preparation_result_error);
			free((void*)config.s1.lng_db_install_process_get_preparation_result_error);
		}
		if (strcmp(config.s1.lng_db_install_process_preparation_error, "") != 0) {
			strcpy(language_temp.lng_db_install_process_preparation_error, config.s1.lng_db_install_process_preparation_error);
			free((void*)config.s1.lng_db_install_process_preparation_error);
		}
		if (strcmp(config.s1.lng_db_install_process_preparation_result_error, "") != 0) {
			strcpy(language_temp.lng_db_install_process_preparation_result_error, config.s1.lng_db_install_process_preparation_result_error);
			free((void*)config.s1.lng_db_install_process_preparation_result_error);
		}
		if (strcmp(config.s1.lng_db_install_process_preparation_success, "") != 0) {
			strcpy(language_temp.lng_db_install_process_preparation_success, config.s1.lng_db_install_process_preparation_success);
			free((void*)config.s1.lng_db_install_process_preparation_success);
		}
		if (strcmp(config.s1.lng_db_install_process_applying_update, "") != 0) {
			strcpy(language_temp.lng_db_install_process_applying_update, config.s1.lng_db_install_process_applying_update);
			free((void*)config.s1.lng_db_install_process_applying_update);
		}
		if (strcmp(config.s1.lng_db_install_process_get_progress_error, "") != 0) {
			strcpy(language_temp.lng_db_install_process_get_progress_error, config.s1.lng_db_install_process_get_progress_error);
			free((void*)config.s1.lng_db_install_process_get_progress_error);
		}
		if (strcmp(config.s1.lng_db_install_process_applying_update_error, "") != 0) {
			strcpy(language_temp.lng_db_install_process_applying_update_error, config.s1.lng_db_install_process_applying_update_error);
			free((void*)config.s1.lng_db_install_process_applying_update_error);
		}
		if (strcmp(config.s1.lng_db_install_process_success, "") != 0) {
			strcpy(language_temp.lng_db_install_process_success, config.s1.lng_db_install_process_success);
			free((void*)config.s1.lng_db_install_process_success);
		}
		if (strcmp(config.s1.lng_db_install_process_reset_to_factory_error, "") != 0) {
			strcpy(language_temp.lng_db_install_process_reset_to_factory_error, config.s1.lng_db_install_process_reset_to_factory_error);
			free((void*)config.s1.lng_db_install_process_reset_to_factory_error);
		}
		if (strcmp(config.s1.lng_db_install_process_reset_to_factory_for_refurbishment_error, "") != 0) {
			strcpy(language_temp.lng_db_install_process_reset_to_factory_for_refurbishment_error, config.s1.lng_db_install_process_reset_to_factory_for_refurbishment_error);
			free((void*)config.s1.lng_db_install_process_reset_to_factory_for_refurbishment_error);
		}
		if (strcmp(config.s1.lng_db_install_process_reset_to_factory_success, "") != 0) {
			strcpy(language_temp.lng_db_install_process_reset_to_factory_success, config.s1.lng_db_install_process_reset_to_factory_success);
			free((void*)config.s1.lng_db_install_process_reset_to_factory_success);
		}
		if (strcmp(config.s1.lng_db_title, "") != 0) {
			strcpy(language_temp.lng_db_title, config.s1.lng_db_title);
			free((void*)config.s1.lng_db_title);
		}
		if (strcmp(config.s1.lng_db_init_failed_error, "") != 0) {
			strcpy(language_temp.lng_db_init_failed_error, config.s1.lng_db_init_failed_error);
			free((void*)config.s1.lng_db_init_failed_error);
		}
		if (strcmp(config.s1.lng_db_get_hardware_type_error, "") != 0) {
			strcpy(language_temp.lng_db_get_hardware_type_error, config.s1.lng_db_get_hardware_type_error);
			free((void*)config.s1.lng_db_get_hardware_type_error);
		}
		if (strcmp(config.s1.lng_db_get_rcm_bug_patch_error, "") != 0) {
			strcpy(language_temp.lng_db_get_rcm_bug_patch_error, config.s1.lng_db_get_rcm_bug_patch_error);
			free((void*)config.s1.lng_db_get_rcm_bug_patch_error);
		}
		if (strcmp(config.s1.lng_db_get_emummc_status_error, "") != 0) {
			strcpy(language_temp.lng_db_get_emummc_status_error, config.s1.lng_db_get_emummc_status_error);
			free((void*)config.s1.lng_db_get_emummc_status_error);
		}
		if (strcmp(config.s1.lng_db_patched_model_warning, "") != 0) {
			strcpy(language_temp.lng_db_patched_model_warning, config.s1.lng_db_patched_model_warning);
			free((void*)config.s1.lng_db_patched_model_warning);
		}
		if (strcmp(config.s1.lng_db_update_validation, "") != 0) {
			strcpy(language_temp.lng_db_update_validation, config.s1.lng_db_update_validation);
			free((void*)config.s1.lng_db_update_validation);
		}
		if (strcmp(config.s1.lng_db_update_validation_failed_error, "") != 0) {
			strcpy(language_temp.lng_db_update_validation_failed_error, config.s1.lng_db_update_validation_failed_error);
			free((void*)config.s1.lng_db_update_validation_failed_error);
		}
		if (strcmp(config.s1.lng_db_ask_exfat_driver_install, "") != 0) {
			strcpy(language_temp.lng_db_ask_exfat_driver_install, config.s1.lng_db_ask_exfat_driver_install);
			free((void*)config.s1.lng_db_ask_exfat_driver_install);
		}
		if (strcmp(config.s1.lng_db_ask_fat32_if_exfat_not_supported, "") != 0) {
			strcpy(language_temp.lng_db_ask_fat32_if_exfat_not_supported, config.s1.lng_db_ask_fat32_if_exfat_not_supported);
			free((void*)config.s1.lng_db_ask_fat32_if_exfat_not_supported);
		}
		if (strcmp(config.s1.lng_db_update_canceled, "") != 0) {
			strcpy(language_temp.lng_db_update_canceled, config.s1.lng_db_update_canceled);
			free((void*)config.s1.lng_db_update_canceled);
		}
		if (strcmp(config.s1.lng_db_exfat_forced_but_not_supported_error, "") != 0) {
			strcpy(language_temp.lng_db_exfat_forced_but_not_supported_error, config.s1.lng_db_exfat_forced_but_not_supported_error);
			free((void*)config.s1.lng_db_exfat_forced_but_not_supported_error);
		}
		if (strcmp(config.s1.lng_db_force_unsupported_firmware_enabled, "") != 0) {
			strcpy(language_temp.lng_db_force_unsupported_firmware_enabled, config.s1.lng_db_force_unsupported_firmware_enabled);
			free((void*)config.s1.lng_db_force_unsupported_firmware_enabled);
		}
		if (strcmp(config.s1.lng_db_ask_force_unsupported_firmware_install, "") != 0) {
			strcpy(language_temp.lng_db_ask_force_unsupported_firmware_install, config.s1.lng_db_ask_force_unsupported_firmware_install);
			free((void*)config.s1.lng_db_ask_force_unsupported_firmware_install);
		}
		if (strcmp(config.s1.lng_db_firmware_not_supported_and_force_install_disabled, "") != 0) {
			strcpy(language_temp.lng_db_firmware_not_supported_and_force_install_disabled, config.s1.lng_db_firmware_not_supported_and_force_install_disabled);
			free((void*)config.s1.lng_db_firmware_not_supported_and_force_install_disabled);
		}
		if (strcmp(config.s1.lng_db_reset_to_factory_forced, "") != 0) {
			strcpy(language_temp.lng_db_reset_to_factory_forced, config.s1.lng_db_reset_to_factory_forced);
			free((void*)config.s1.lng_db_reset_to_factory_forced);
		}
		if (strcmp(config.s1.lng_db_ask_reset_to_factory, "") != 0) {
			strcpy(language_temp.lng_db_ask_reset_to_factory, config.s1.lng_db_ask_reset_to_factory);
			free((void*)config.s1.lng_db_ask_reset_to_factory);
		}
		if (strcmp(config.s1.lng_db_install_update_begin, "") != 0) {
			strcpy(language_temp.lng_db_install_update_begin, config.s1.lng_db_install_update_begin);
			free((void*)config.s1.lng_db_install_update_begin);
		}

		if (strcmp(config.s1.lng_install_pack_recap_begin, "") != 0) {
			strcpy(language_temp.lng_install_pack_recap_begin, config.s1.lng_install_pack_recap_begin);
			free((void*)config.s1.lng_install_pack_recap_begin);
		}
		if (strcmp(config.s1.lng_install_pack_recap_firmware_install, "") != 0) {
			strcpy(language_temp.lng_install_pack_recap_firmware_install, config.s1.lng_install_pack_recap_firmware_install);
			free((void*)config.s1.lng_install_pack_recap_firmware_install);
		}
		if (strcmp(config.s1.lng_install_pack_recap_not_install_firmware, "") != 0) {
			strcpy(language_temp.lng_install_pack_recap_not_install_firmware, config.s1.lng_install_pack_recap_not_install_firmware);
			free((void*)config.s1.lng_install_pack_recap_not_install_firmware);
		}
		if (strcmp(config.s1.lng_install_pack_recap_clean_theme, "") != 0) {
			strcpy(language_temp.lng_install_pack_recap_clean_theme, config.s1.lng_install_pack_recap_clean_theme);
			free((void*)config.s1.lng_install_pack_recap_clean_theme);
		}
		if (strcmp(config.s1.lng_install_pack_recap_not_clean_theme, "") != 0) {
			strcpy(language_temp.lng_install_pack_recap_not_clean_theme, config.s1.lng_install_pack_recap_not_clean_theme);
			free((void*)config.s1.lng_install_pack_recap_not_clean_theme);
		}
		if (strcmp(config.s1.lng_install_pack_recap_clean_logos, "") != 0) {
			strcpy(language_temp.lng_install_pack_recap_clean_logos, config.s1.lng_install_pack_recap_clean_logos);
			free((void*)config.s1.lng_install_pack_recap_clean_logos);
		}
		if (strcmp(config.s1.lng_install_pack_recap_not_clean_logos, "") != 0) {
			strcpy(language_temp.lng_install_pack_recap_not_clean_logos, config.s1.lng_install_pack_recap_not_clean_logos);
			free((void*)config.s1.lng_install_pack_recap_not_clean_logos);
		}
		if (strcmp(config.s1.lng_install_pack_recap_install_hbmenu, "") != 0) {
			strcpy(language_temp.lng_install_pack_recap_install_hbmenu, config.s1.lng_install_pack_recap_install_hbmenu);
			free((void*)config.s1.lng_install_pack_recap_install_hbmenu);
		}
		if (strcmp(config.s1.lng_install_pack_recap_not_install_hbmenu, "") != 0) {
			strcpy(language_temp.lng_install_pack_recap_not_install_hbmenu, config.s1.lng_install_pack_recap_not_install_hbmenu);
			free((void*)config.s1.lng_install_pack_recap_not_install_hbmenu);
		}
		if (strcmp(config.s1.lng_install_pack_download_pack_error, "") != 0) {
			strcpy(language_temp.lng_install_pack_download_pack_error, config.s1.lng_install_pack_download_pack_error);
			free((void*)config.s1.lng_install_pack_download_pack_error);
		}
		if (strcmp(config.s1.lng_install_pack_folder_create, "") != 0) {
			strcpy(language_temp.lng_install_pack_folder_create, config.s1.lng_install_pack_folder_create);
			free((void*)config.s1.lng_install_pack_folder_create);
		}
		if (strcmp(config.s1.lng_install_pack_extract_file_protected, "") != 0) {
			strcpy(language_temp.lng_install_pack_extract_file_protected, config.s1.lng_install_pack_extract_file_protected);
			free((void*)config.s1.lng_install_pack_extract_file_protected);
		}
		if (strcmp(config.s1.lng_install_pack_extract_file, "") != 0) {
			strcpy(language_temp.lng_install_pack_extract_file, config.s1.lng_install_pack_extract_file);
			free((void*)config.s1.lng_install_pack_extract_file);
		}
		if (strcmp(config.s1.lng_install_pack_file_write_error, "") != 0) {
			strcpy(language_temp.lng_install_pack_file_write_error, config.s1.lng_install_pack_file_write_error);
			free((void*)config.s1.lng_install_pack_file_write_error);
		}

		if (strcmp(config.s1.lng_clean_logos_begin, "") != 0) {
			strcpy(language_temp.lng_clean_logos_begin, config.s1.lng_clean_logos_begin);
			free((void*)config.s1.lng_clean_logos_begin);
		}
		if (strcmp(config.s1.lng_clean_theme_begin, "") != 0) {
			strcpy(language_temp.lng_clean_theme_begin, config.s1.lng_clean_theme_begin);
			free((void*)config.s1.lng_clean_theme_begin);
		}
		if (strcmp(config.s1.lng_clean_sd_begin, "") != 0) {
			strcpy(language_temp.lng_clean_sd_begin, config.s1.lng_clean_sd_begin);
			free((void*)config.s1.lng_clean_sd_begin);
		}
		if (strcmp(config.s1.lng_clean_sd_clean_unusful_files, "") != 0) {
			strcpy(language_temp.lng_clean_sd_clean_unusful_files, config.s1.lng_clean_sd_clean_unusful_files);
			free((void*)config.s1.lng_clean_sd_clean_unusful_files);
		}
		if (strcmp(config.s1.lng_clean_sd_finish, "") != 0) {
			strcpy(language_temp.lng_clean_sd_finish, config.s1.lng_clean_sd_finish);
			free((void*)config.s1.lng_clean_sd_finish);
		}

		if (strcmp(config.s1.lng_install_app_download_app_error, "") != 0) {
			strcpy(language_temp.lng_install_app_download_app_error, config.s1.lng_install_app_download_app_error);
			free((void*)config.s1.lng_install_app_download_app_error);
		}

		if (strcmp(config.s1.lng_dns_end_install_error, "") != 0) {
			strcpy(language_temp.lng_dns_end_install_error, config.s1.lng_dns_end_install_error);
			free((void*)config.s1.lng_dns_end_install_error);
		}
		if (strcmp(config.s1.lng_dns_install_set_init_error, "") != 0) {
			strcpy(language_temp.lng_dns_install_set_init_error, config.s1.lng_dns_install_set_init_error);
			free((void*)config.s1.lng_dns_install_set_init_error);
		}
		if (strcmp(config.s1.lng_dns_install_setsys_init_error, "") != 0) {
			strcpy(language_temp.lng_dns_install_setsys_init_error, config.s1.lng_dns_install_setsys_init_error);
			free((void*)config.s1.lng_dns_install_setsys_init_error);
		}
		if (strcmp(config.s1.lng_dns_install_get_region_code_error, "") != 0) {
			strcpy(language_temp.lng_dns_install_get_region_code_error, config.s1.lng_dns_install_get_region_code_error);
			free((void*)config.s1.lng_dns_install_get_region_code_error);
		}
		if (strcmp(config.s1.lng_dns_install_display_region_code, "") != 0) {
			strcpy(language_temp.lng_dns_install_display_region_code, config.s1.lng_dns_install_display_region_code);
			free((void*)config.s1.lng_dns_install_display_region_code);
		}
		if (strcmp(config.s1.lng_dns_install_use_us_dns, "") != 0) {
			strcpy(language_temp.lng_dns_install_use_us_dns, config.s1.lng_dns_install_use_us_dns);
			free((void*)config.s1.lng_dns_install_use_us_dns);
		}
		if (strcmp(config.s1.lng_dns_install_use_eu_dns, "") != 0) {
			strcpy(language_temp.lng_dns_install_use_eu_dns, config.s1.lng_dns_install_use_eu_dns);
			free((void*)config.s1.lng_dns_install_use_eu_dns);
		}
		if (strcmp(config.s1.lng_dns_install_use_unknown_dns, "") != 0) {
			strcpy(language_temp.lng_dns_install_use_unknown_dns, config.s1.lng_dns_install_use_unknown_dns);
			free((void*)config.s1.lng_dns_install_use_unknown_dns);
		}
		if (strcmp(config.s1.lng_dns_install_aplying, "") != 0) {
			strcpy(language_temp.lng_dns_install_aplying, config.s1.lng_dns_install_aplying);
			free((void*)config.s1.lng_dns_install_aplying);
		}
		if (strcmp(config.s1.lng_dns_install_get_wifi_networks_error, "") != 0) {
			strcpy(language_temp.lng_dns_install_get_wifi_networks_error, config.s1.lng_dns_install_get_wifi_networks_error);
			free((void*)config.s1.lng_dns_install_get_wifi_networks_error);
		}
		if (strcmp(config.s1.lng_dns_install_networs_count, "") != 0) {
			strcpy(language_temp.lng_dns_install_networs_count, config.s1.lng_dns_install_networs_count);
			free((void*)config.s1.lng_dns_install_networs_count);
		}
		if (strcmp(config.s1.lng_dns_install_wifi_network_config_error, "") != 0) {
			strcpy(language_temp.lng_dns_install_wifi_network_config_error, config.s1.lng_dns_install_wifi_network_config_error);
			free((void*)config.s1.lng_dns_install_wifi_network_config_error);
		}
		if (strcmp(config.s1.lng_dns_install_success, "") != 0) {
			strcpy(language_temp.lng_dns_install_success, config.s1.lng_dns_install_success);
			free((void*)config.s1.lng_dns_install_success);
		}

		if (strcmp(config.s1.lng_protect_console_begin, "") != 0) {
			strcpy(language_temp.lng_protect_console_begin, config.s1.lng_protect_console_begin);
			free((void*)config.s1.lng_protect_console_begin);
		}
		if (strcmp(config.s1.lng_protect_console_error, "") != 0) {
			strcpy(language_temp.lng_protect_console_error, config.s1.lng_protect_console_error);
			free((void*)config.s1.lng_protect_console_error);
		}

		if (strcmp(config.s1.lng_dl_progress_0, "") != 0) {
			strcpy(language_temp.lng_dl_progress_0, config.s1.lng_dl_progress_0);
			free((void*)config.s1.lng_dl_progress_0);
		}
		if (strcmp(config.s1.lng_dl_progress_1, "") != 0) {
			strcpy(language_temp.lng_dl_progress_1, config.s1.lng_dl_progress_1);
			free((void*)config.s1.lng_dl_progress_1);
		}
		if (strcmp(config.s1.lng_dl_progress_with_bar_0, "") != 0) {
			strcpy(language_temp.lng_dl_progress_with_bar_0, config.s1.lng_dl_progress_with_bar_0);
			free((void*)config.s1.lng_dl_progress_with_bar_0);
		}
		if (strcmp(config.s1.lng_dl_progress_with_bar_1, "") != 0) {
			strcpy(language_temp.lng_dl_progress_with_bar_1, config.s1.lng_dl_progress_with_bar_1);
			free((void*)config.s1.lng_dl_progress_with_bar_1);
		}
		if (strcmp(config.s1.lng_dl_begin, "") != 0) {
			strcpy(language_temp.lng_dl_begin, config.s1.lng_dl_begin);
			free((void*)config.s1.lng_dl_begin);
		}
		if (strcmp(config.s1.lng_dl_file_write_error, "") != 0) {
			strcpy(language_temp.lng_dl_file_write_error, config.s1.lng_dl_file_write_error);
			free((void*)config.s1.lng_dl_file_write_error);
		}
		if (strcmp(config.s1.lng_dl_success, "") != 0) {
			strcpy(language_temp.lng_dl_success, config.s1.lng_dl_success);
			free((void*)config.s1.lng_dl_success);
		}
		if (strcmp(config.s1.lng_dl_dl_error, "") != 0) {
			strcpy(language_temp.lng_dl_dl_error, config.s1.lng_dl_dl_error);
			free((void*)config.s1.lng_dl_dl_error);
		}
		if (strcmp(config.s1.lng_dl_open_temp_file_error, "") != 0) {
			strcpy(language_temp.lng_dl_open_temp_file_error, config.s1.lng_dl_open_temp_file_error);
			free((void*)config.s1.lng_dl_open_temp_file_error);
		}
		if (strcmp(config.s1.lng_dl_curl_init_error, "") != 0) {
			strcpy(language_temp.lng_dl_curl_init_error, config.s1.lng_dl_curl_init_error);
			free((void*)config.s1.lng_dl_curl_init_error);
		}

	}
	return language_temp;
}