#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <switch.h>

#include "translate.hpp"
#include "main_util.h"
#include "ini.h"

translation_map language_vars;

const char* translation_vars_list[] =
{
	"lng_yes",
	"lng_no",
	"lng_unknown_0",
	"lng_unknown_1",
	"lng_files",
	"lng_partition",
	"lng_usable",
	"lng_not_usable",
	"lng_success_reboot_in_five_seconds",
	"lng_error_reboot_to_payload",
	"lng_error_not_enough_space_on_sd",
	"lng_error_copy_file",
	"lng_calculate_sha256_of_downloaded_file",
	"lng_press_b_to_go_back",
	"lng_press_any_key_to_continue",
	"lng_display_changelog_choice",
	"lng_error_changelog_display_applet",
	"lng_error_changelog_display_no_adress",
	"lng_error_no_internet_connection_for_function",

	"lng_title",
	"lng_title_beta",
	"lng_ls_menu",
	"lng_rs_menu",
	"lng_move_menu",
	"lng_a_menu",
	"lng_x_menu",
	"lng_y_menu",
	"lng_minus_menu",
	"lng_minus_menu_beta",
	"lng_minus_menu_switch",
	"lng_l_r_menu",
	"lng_zl_zr_menu",
	"lng_l_zl_menu",
	"lng_r_zr_menu",
	"lng_l_zr_menu",
	"lng_r_zl_menu",
	"lng_plus_menu",
	"lng_update_app_menu",
	"lng_update_pack_menu",
	"lng_update_firmware_menu",
	"lng_update_hbmenu_menu",
	"lng_set_90dns_menu",
	"lng_protect_console_menu",
	"lng_install_app_fwd_menu",
	"lng_reset_parental_control_menu",
	"lng_sync_time_menu",
	"lng_reset_menu",

	"lng_infos_begin",
	"lng_infos_is_applet",
	"lng_infos_is_not_applet",
	"lng_infos_pack_version",
	"lng_infos_last_pack_version",
	"lng_infos_console_id",
	"lng_infos_serial_incognito",
	"lng_infos_serial",
	"lng_infos_sysnand",
	"lng_infos_emunand",
	"lng_infos_console_model",
	"lng_infos_fusee_gelee_patch",
	"lng_infos_actual_firmware_version",
	"lng_infos_actual_atmosphere_version",
	"lng_infos_90dns_status_ok",
	"lng_infos_90dns_status_no_conection",
	"lng_infos_90dns_status_not_protected",
	"lng_infos_90dns_status_error",
	"lng_infos_official_charge",
	"lng_infos_usb_charge",
	"lng_infos_no_charge",
	"lng_infos_hekate_autoboot_enabled",
	"lng_infos_hekate_autoboot_disabled",
	"lng_record_infos_log_not_open_error",
	"lng_record_infos_success",

"lng_hekate_config_choice_menu_title",
"lng_hekate_config_choice_no_configs_found",
	"lng_no_hekate_autoboot_config_set_error",
	"lng_hekate_autoboot_config_canceled",
	"lng_hekate_param_already_set",
	"lng_hekate_error_config_file_read",
	"lng_hekate_error_temp_file_write",
	"lng_hekate_replace_config_error",
	"lng_hekate_autoboot_enabled_success",
	"lng_hekate_autoboot_disabled_success",

	"lng_configuring_app_autoconfig",
	"lng_autoconfig_recap_begin",
	"lng_autoconfig_recap_full_app_control",
	"lng_autoconfig_recap_not_full_app_control",
	"lng_autoconfig_recap_beta_launch",
	"lng_autoconfig_recap_not_not_beta_launch",
	"lng_autoconfig_set_error",
	"lng_autoconfig_set_success",
	"lng_autoconfig_disabled",

	"lng_hbmenu_install_begin",
	"lng_hbmenu_install_uninstall_begin",
	"lng_hbmenu_install_uninstall_error",
	"lng_hbmenu_install_uninstall_success",
	"lng_hbmenu_install_success",
	"lng_hbmenu_install_error",

"lng_app_fwd_install_begin",
	"lng_app_fwd_install_success",
	"lng_app_fwd_install_error",

	"lng_reset_parental_begin",
	"lng_reset_parental_app_error",
	"lng_reset_parental_app_success",
	"lng_reset_parental_error",
	"lng_reset_parental_success",

	"lng_battery_error_10",
	"lng_battery_error_20",
	"lng_battery_error_30",

	"lng_ask_disable_autoconfig",
	"lng_ask_autoconfig_full_control",
	"lng_ask_autoconfig_beta_launch",
	"lng_ask_sd_exfat",
	"lng_ask_update_firmware",
	"lng_ask_update_firmware_only",
	"lng_ask_update_app",
	"lng_ask_app_need_update",
	"lng_pack_have_update",
	"lng_pack_have_no_update",
	"lng_ask_pack_have_update",
	"lng_ask_clean_theme",
	"lng_ask_agressive_clean",
	"lng_ask_clean_modules",
	"lng_ask_keep_files",
	"lng_ask_hekate_autoboot",
	"lng_ask_clean_logos",
	"lng_ask_hbmenu_install",
	"lng_ask_app_fwd_install",
	"lng_ask_validate_choices",
	"lng_ask_validate_choices_for_parental_reset",
	"lng_ask_validate_choices_for_sync_time",
	"lng_ask_validate_choices_for_reset",
	"lng_ask_force_error_with_sha256",

	"lng_install_firmware_end_error",
	"lng_install_firmware_end_success",
	"lng_install_firmware_error_folder_not_found",
	"lng_install_firmware_folder_location",

	"lng_db_pause",
	"lng_db_atmosphere_not_found_error",
	"lng_db_atmosphere_outdated_error",
	"lng_db_nso_launch_error",
	"lng_db_update_not_found_error",
	"lng_db_get_update_infos_error",
	"lng_db_update_infos_fw_version",
	"lng_db_update_infos_exfat_support",
	"lng_db_update_infos_exfat_no_support",
	"lng_db_update_infos_fw_version_variations",
	"lng_db_update_validation_error",
	"lng_db_update_validation_success",
	"lng_db_update_validation_exfat_error",
	"lng_db_update_validation_missing_content",
	"lng_db_update_validation_content_error",
	"lng_db_install_process_setup_error",
	"lng_db_install_process_setup_success",
	"lng_db_install_process_request_preparation_error",
	"lng_db_install_process_preparing",
	"lng_db_install_process_get_preparation_result_error",
	"lng_db_install_process_preparation_error",
	"lng_db_install_process_preparation_result_error",
	"lng_db_install_process_preparation_success",
	"lng_db_install_process_applying_update",
	"lng_db_install_process_get_progress_error",
	"lng_db_install_process_applying_update_error",
	"lng_db_install_process_success",
	"lng_db_install_process_reset_to_factory_error",
	"lng_db_install_process_reset_to_factory_for_refurbishment_error",
	"lng_db_install_process_reset_to_factory_success",
	"lng_db_title",
	"lng_db_init_failed_error",
	"lng_db_get_hardware_type_error",
	"lng_db_get_rcm_bug_patch_error",
	"lng_db_get_emummc_status_error",
	"lng_db_patched_model_warning",
	"lng_db_update_validation",
	"lng_db_update_validation_failed_error",
	"lng_db_ask_special_dg",
	"lng_db_ask_exfat_driver_install",
	"lng_db_ask_fat32_if_exfat_not_supported",
	"lng_db_update_canceled",
	"lng_db_exfat_forced_but_not_supported_error",
	"lng_db_force_unsupported_firmware_enabled",
	"lng_db_ask_force_unsupported_firmware_install",
	"lng_db_firmware_not_supported_and_force_install_disabled",
	"lng_db_reset_to_factory_forced",
	"lng_db_ask_reset_to_factory",
	"lng_db_install_update_begin",

	"lng_install_pack_recap_begin",
	"lng_install_pack_recap_firmware_install",
	"lng_install_pack_recap_not_install_firmware",
	"lng_install_pack_recap_clean_theme",
	"lng_install_pack_recap_not_clean_theme",
	"lng_install_pack_recap_agressive_clean",
	"lng_install_pack_recap_not_agressive_clean",
	"lng_install_pack_recap_clean_modules",
	"lng_install_pack_recap_not_clean_modules",
	"lng_install_pack_recap_keep_files",
	"lng_install_pack_recap_not_keep_files",
	"lng_install_pack_recap_enable_hekate_autoboot",
	"lng_install_pack_recap_not_enable_hekate_autoboot",
	"lng_install_pack_recap_clean_logos",
	"lng_install_pack_recap_not_clean_logos",
	"lng_install_pack_recap_install_hbmenu",
	"lng_install_pack_recap_not_install_hbmenu",
	"lng_install_pack_recap_install_app_fwd",
	"lng_install_pack_recap_not_install_app_fwd",
	"lng_install_pack_download_pack_error",
	"lng_install_custom_files_pack_download_error",
	"lng_install_pack_file_skip",
	"lng_install_pack_folder_create",
	"lng_install_pack_same_files",
	"lng_install_pack_extract_file_protected",
	"lng_install_pack_extract_file",
	"lng_install_pack_file_write__retrying_error",
	"lng_install_pack_file_write_error",
	"lng_installing_pack_custom_files",
	"lng_install_pack_configuring_hekate_autoboot",

	"lng_clean_logos_begin",
	"lng_clean_theme_begin",
	"lng_agressive_clean_begin",
	"lng_clean_modules_begin",
	"lng_clean_sd_begin",
	"lng_clean_sd_clean_unusful_files",
	"lng_clean_sd_finish",

	"lng_install_app_download_app_error",

	"lng_dns_end_install_error",
	"lng_dns_install_set_init_error",
	"lng_dns_install_setsys_init_error",
	"lng_dns_install_get_region_code_error",
	"lng_dns_install_display_region_code",
	"lng_dns_install_use_us_dns",
	"lng_dns_install_use_eu_dns",
	"lng_dns_install_use_unknown_dns",
	"lng_dns_install_aplying",
	"lng_dns_install_get_wifi_networks_error",
	"lng_dns_install_networs_count",
	"lng_dns_install_wifi_network_config_error",
	"lng_dns_install_success",

	"lng_protect_console_begin",
	"lng_protect_console_error",

"lng_sync_time_begin",
"lng_sync_time_error",
"lng_sync_time_success",

	"lng_dl_progress_0",
	"lng_dl_progress_1",
	"lng_dl_progress_with_bar_0",
	"lng_dl_progress_with_bar_1",
	"lng_dl_begin",
	"lng_dl_file_write_error",
	"lng_dl_success",
	"lng_dl_dl_error",
	"lng_dl_open_temp_file_error",
	"lng_dl_curl_init_error"
};

static int translation_handler(void* config, const char * section, const char * name, const char * value, int lineno)
{
	// config instance for filling in the values.
	translation_map* pconfig = (translation_map*)config;

	// define a macro for checking Sections and keys under the sections.
	#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
	// FILE *test = fopen("/switch/AIO_LS_pack_Updater/test.log", "a");
	// fprintf(test, "%s\n%s\n", name, value);
	// fprintf(test, "%ld\n", sizeof(translation_vars_list)/sizeof(translation_vars_list[0]));
	// fill the values in config struct for Section 1.
	for (size_t i=0; i < sizeof(translation_vars_list)/sizeof(translation_vars_list[0]); i++) {
		if(MATCH("language", translation_vars_list[i])){
			if (value != 0) {
				pconfig->insert({translation_vars_list[i], (std::string) value});
			} else {
				pconfig->insert({translation_vars_list[i], (std::string) ""});
			}
		// fprintf(test, "Pass\n\n");
		// fclose(test);
			return 1;
		}
	}
	// fprintf(test, "Error\n\n");
	// fclose(test);
	debug_log_write("Clé inconnue: [%s] %s=%s (ligne %d)\n", section, name, value ? value : "(null)", lineno);
	return 1;
}

translation_map set_translation_strings() {
	translation_map config;
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
		char lang_str[0x40] = {0};
		memcpy(lang_str, &language_code, sizeof(language_code));
		snprintf(file_path, sizeof(file_path), "romfs:/lng/%s.ini", lang_str);
		// strcpy(file_path, "romfs:/lng/");
		// strcat(strcat(file_path, (char *) &language_code), ".ini");
		test_ini = fopen(file_path, "r");
		if (test_ini != NULL) {
			fclose(test_ini);
		} else {
			strcpy(file_path, "romfs:/lng/en_US.ini");
			test_ini = fopen(file_path, "r");
			if (test_ini != NULL) {
				fclose(test_ini);
			} else {
				debug_log_write("Langage par défaut utilisé.\n");
				return language_temp;
			}
		}
	}
	debug_log_write("Fichier de langage: %s\n", file_path);
	// int i = 0;
	// parse the .ini file
	if (ini_parse(file_path, translation_handler, &config) == 0) {
		for (size_t i = 0; i < sizeof(translation_vars_list)/sizeof(translation_vars_list[0]); i++) {
			auto it = config.find(translation_vars_list[i]);
			if (it != config.end()) {
				language_temp[translation_vars_list[i]] = it->second;
			}
		}
		// if (strcmp(config[translation_vars_list[i]].c_str(), "") != 0) {
			// language_temp.erase(translation_vars_list[i]);
/*
			int j=0;
			while (config[translation_vars_list[i]][j] != '\0') {
				j++;
			}
			language_temp[translation_vars_list[i]] = new char[j+1];
			if (language_temp[translation_vars_list[i]] != nullptr) {
			delete[] language_temp[translation_vars_list[i]];
			language_temp[translation_vars_list[i]] = nullptr;
			}
			language_temp[translation_vars_list[i]] = new char[strlen(config[translation_vars_list[i]])+1];
			strcpy(language_temp[translation_vars_list[i]], config[translation_vars_list[i]]);
			if (config[translation_vars_list[i]] != nullptr) {
				free((char*)config[translation_vars_list[i]]);
				config[translation_vars_list[i]] = nullptr;
			}
			*/
			// language_temp[translation_vars_list[i]] = config[translation_vars_list[i]];
		// }
		// i++;
	}
	return language_temp;
}