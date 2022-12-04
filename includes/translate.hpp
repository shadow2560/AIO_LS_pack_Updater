#ifndef _TRANSLATE_HPP_
#define _TRANSLATE_HPP_

#include <map>

struct translation_map_comparator {
	bool operator()(const char* left, const char* right) const {
		return ((left != nullptr) && (right != nullptr) && (strcmp(left, right) < 0));
	}
};

typedef std::map<const char*, char*, translation_map_comparator> translation_map;

#define INIT_LNG(X) translation_map X = {\
	{"lng_yes", (char*) "Oui"}, \
	{"lng_no", (char*) "Non"}, \
	{"lng_unknown_0", (char*) "inconnu"}, \
	{"lng_unknown_1", (char*) "inconnue"}, \
	{"lng_files", (char*) "fichiers"}, \
	{"lng_partition", (char*) "partition"}, \
	{"lng_usable", (char*) "utilisable"}, \
	{"lng_not_usable", (char*) "inutilisable"}, \
	{"lng_success_reboot_in_five_seconds", (char*) "Finis!\n\nRedemarrage automatique dans 5 secondes"}, \
	{"lng_error_reboot_to_payload", (char*) "Payload non trouve."}, \
	{"lng_error_not_enough_space_on_sd", (char*) "Erreur, pas assez d'espace sur la SD."}, \
	{"lng_error_copy_file", (char*) "Erreur de copie de fichier."}, \
\
	{"lng_title", (char*) "AIO_LS_pack_Updater v%s - par Shadow256"}, \
	{"lng_title_beta", (char*) "AIO_LS_pack_Updater v%s - par Shadow256 - BETA MODE"}, \
	{"lng_a_menu", (char*) "Appuyez sur (A) pour selectionner une option"}, \
	{"lng_x_menu", (char*) "Appuyez sur (X) pour afficher diverses informations"}, \
	{"lng_y_menu", (char*) "Appuyez sur (Y) pour enregistrer diverses informations dans un fichier"}, \
	{"lng_minus_menu", (char*) "Appuyez sur (-) pour passer en mode beta"}, \
	{"lng_minus_menu_beta", (char*) "Appuyez sur (-) pour passer en mode stable"}, \
	{"lng_plus_menu", (char*) "Appuyez sur (+) pour quitter l'application"}, \
	{"lng_update_app_menu", (char*) "= Mise a jour de l'application"}, \
	{"lng_update_pack_menu", (char*) "= Mise a jour du pack"}, \
	{"lng_update_firmware_menu", (char*) "= Mise a jour du firmware"}, \
	{"lng_update_hbmenu_menu", (char*) "= Installation de l'icone du Homebrew Menu (recommande si vous le l'avez pas)"}, \
	{"lng_set_90dns_menu", (char*) "= Application de la protection DNS sur tous les reseaux Wifi deja configures"}, \
	{"lng_protect_console_menu", (char*) "= Application de configurations pour  proteger au mieux la console lancee sous Atmosphere (Atmosphere 0.18.1 minimum requis)"}, \
\
	{"lng_infos_begin", (char*) "Informations:"}, \
	{"lng_infos_is_applet", (char*) "Homebrew en mode applet, utilisation non recommandee."}, \
	{"lng_infos_is_not_applet", (char*) "Le homebrew peut utiliser toute la RAM, utilisation recommandee."}, \
	{"lng_infos_pack_version", (char*) "Version actuelle du pack : %s"}, \
	{"lng_infos_last_pack_version", (char*) "Derniere version du pack : %s"}, \
	{"lng_infos_console_id", (char*) "ID de la console : %li"}, \
	{"lng_infos_serial_incognito", (char*) "Numero de serie de la console  indetectable, Incognito est actif."}, \
	{"lng_infos_serial", (char*) "Numero de serie de la console : %s"}, \
	{"lng_infos_sysnand", (char*) "Type de systeme : %s"}, \
	{"lng_infos_emunand", (char*) "Type de systeme : %s via %s"}, \
	{"lng_infos_console_model", (char*) "Modele de la console : %s"}, \
	{"lng_infos_fusee_gelee_patch", (char*) "Etat de l'exploit Fusee Gelee : %s"}, \
	{"lng_infos_actual_firmware_version", (char*) "Version actuelle du firmware : %s"}, \
	{"lng_infos_actual_atmosphere_version", (char*) "Version actuelle d'Atmosphere : %s"}, \
	{"lng_infos_90dns_status_ok", (char*) "Protection 90DNS: Active, la console n'a pas acces aux serveurs de Nintendo."}, \
	{"lng_infos_90dns_status_no_conection", (char*) "Protection 90DNS: Aucune connexion au reseau, impossible de tester."}, \
	{"lng_infos_90dns_status_not_protected", (char*) "Protection 90DNS: Danger, un ou plusieurs serveurs de Nintendo ne sont pas bloques, fort risque de bannissement de la console."}, \
	{"lng_infos_90dns_status_error", (char*) "Protection 90DNS: Erreur,  verification impossible."}, \
	{"lng_infos_official_charge", (char*) "Charge active, charge batterie : %d%%, type de chargeur : officiel"}, \
	{"lng_infos_usb_charge", (char*) "Charge active, charge batterie : %d%%, type de chargeur : USB ou non officiel"}, \
	{"lng_infos_no_charge", (char*) "Charge inactive, charge batterie : %d%%"}, \
	{"lng_record_infos_log_not_open_error", (char*) "Le fichier \"switch/AIO_LS_pack_Updater/console_infos.log\" n'a pas pu etre enregistre."}, \
	{"lng_record_infos_success", (char*) "Le fichier contenant les informations de la console ont ete enregistrees dans le fichier \"switch/AIO_LS_pack_Updater/console_infos.log\"."}, \
\
	{"lng_hbmenu_install_begin", (char*) "Installation du forwarder du Homebrew Menu en cours..."}, \
	{"lng_hbmenu_install_uninstall_begin", (char*) "Ancienne version du forwarder trouvee, desinstallation de celle-ci..."}, \
	{"lng_hbmenu_install_uninstall_error", (char*) "Erreur durant la desinstallation de l'ancienne version du forwarder, anulation de l'installation."}, \
	{"lng_hbmenu_install_uninstall_success", (char*) "Desinstallation de l'ancienne version du forwarder effectuee."}, \
	{"lng_hbmenu_install_success", (char*) "Installation du forwarder du Homebrew Menu effectuee."}, \
	{"lng_hbmenu_install_error", (char*) "Une erreur s'est produite durant l'installation du forwarder du Homebrew Menu.\nVos sig_patches ne sont peut-etre pas a jour, l'espace restant sur votre stockage est peut-etre insufisant ou vous avez peut-etre de la corruption de donnees sur votre SD ou votre nand/emunand."}, \
\
	{"lng_battery_error_10", (char*) "Impossible d'effectuer cette action dans les conditions actuelles, vous devez attendre d'etre au-dessus de 10%% de batterie."}, \
	{"lng_battery_error_20", (char*) "Impossible d'effectuer cette action dans les conditions actuelles, vous devez attendre d'etre au-dessus de 20%% de batterie."}, \
	{"lng_battery_error_30", (char*) "Impossible d'effectuer cette action dans les conditions actuelles, vous devez attendre d'etre au-dessus de 30%% de batterie."}, \
\
	{"lng_ask_update_firmware", (char*) "Souhaitez-vous vraiment mettre a jour le firmware (si oui les fichiers du theme seront aussi nettoyes)?"}, \
	{"lng_ask_clean_theme", (char*) "Souhaitez-vous nettoyer les fichiers du theme, utile si mise a jour du firmware par la suite?"}, \
	{"lng_ask_agressive_clean", (char*) "Souhaitez-vous faire un nettoyage agressif, utile si mise a jour du firmware si vous utilisez des modules non inclus dans le pack ou si vous venez d'un autre pack?"}, \
	{"lng_ask_clean_logos", (char*) "Souhaitez-vous retirer les logos?"}, \
	{"lng_ask_hbmenu_install", (char*) "Souhaitez-vous installer l'icone du Homebrew Menu permettant de lancer les homebrews en profitant de toute la RAM de la console (vivement recommande si vous ne l'avez pas)?"}, \
	{"lng_ask_validate_choices", (char*) "Souhaitez-vous vraiment continuer?"}, \
\
	{"lng_install_firmware_end_error", (char*) "Une erreur s'est produite durant l'installation du firmware."}, \
	{"lng_install_firmware_end_success", (char*) "Mise a jour du firmware effectuee."}, \
	{"lng_install_firmware_error_folder_not_found", (char*) "Le repertoire du firmware pour mettre a jour la console n'a pas pu etre ouvert, la mise a jour du firmware ne sera pas faite."}, \
	{"lng_install_firmware_folder_location", (char*) "Le dossier contenant le firmware est le dossier \"%s\"."}, \
\
	{"lng_db_pause", (char*) "Appuyez sur \"A\" pour continuer..."}, \
	{"lng_db_atmosphere_not_found_error", (char*) "Atmosphere non trouve, Daybreak requiere Atmosphere. %d"}, \
	{"lng_db_atmosphere_outdated_error", (char*) "Version d'Atmosphere depassee, Daybreak requiere Atmosphere 0.14.0 ou superieure. %d"}, \
	{"lng_db_nso_launch_error", (char*) "Environment non supporte, Lancez Daybreak via le Homebrew menu. %d"}, \
	{"lng_db_update_not_found_error", (char*) "Aucune mise a jour trouvee dans le dossier, assurez-vous que vos ncas sont nommes correctement!\nResult: 0x%08x"}, \
	{"lng_db_get_update_infos_error", (char*) "Echec de recuperation des informations de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_update_infos_fw_version", (char*) "- Version: %d.%d.%d"}, \
	{"lng_db_update_infos_exfat_support", (char*) "- exFAT: Supporte"}, \
	{"lng_db_update_infos_exfat_no_support", (char*) "- exFAT: Non supporte"}, \
	{"lng_db_update_infos_fw_version_variations", (char*) "- Firmware variations: %d"}, \
	{"lng_db_update_validation_error", (char*) "Echec de la validation de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_update_validation_success", (char*) "La mise a jour est valide!"}, \
	{"lng_db_update_validation_exfat_error", (char*) "Echec de la validation de l'exFAT.\nResult: 0x%08x"}, \
	{"lng_db_update_validation_missing_content", (char*) "Contenu manquant:\n- Program id: %016lx\n- Version: %d.%d.%d"}, \
	{"lng_db_update_validation_content_error", (char*) "Echec de la validation \nResult: 0x%08x"}, \
	{"lng_db_install_process_setup_error", (char*) "Echec durant la mise en place de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_setup_success", (char*) "Mise a jour parametree."}, \
	{"lng_db_install_process_request_preparation_error", (char*) "Echec de la requete de preparation de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_preparing", (char*) "Preparation de la mise a jour..."}, \
	{"lng_db_install_process_get_preparation_result_error", (char*) "Echec de la recuperation du resultat de la preparation de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_preparation_error", (char*) "Echec de la preparation de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_preparation_result_error", (char*) "Echec de la verification pour savoir si la mise a jour a bien ete preparee.\nResult: 0x%08x"}, \
	{"lng_db_install_process_preparation_success", (char*) "Preparation de la mise a jour effectuee."}, \
	{"lng_db_install_process_applying_update", (char*) "Application de la mise a jour..."}, \
	{"lng_db_install_process_get_progress_error", (char*) "Echec de la recuperation de la progression de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_applying_update_error", (char*) "Echec de l'application de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_success", (char*) "Application de la mise a jour effectuee."}, \
	{"lng_db_install_process_reset_to_factory_error", (char*) "Echec de la reinitialisation d'usine de la console.\nResult: 0x%08x"}, \
	{"lng_db_install_process_reset_to_factory_for_refurbishment_error", (char*) "Echec de la reinitialisation d'usine pour la redistribution de la console.\nResult: 0x%08x"}, \
	{"lng_db_install_process_reset_to_factory_success", (char*) "Reinitialisation d'usine effectuee. %d"}, \
	{"lng_db_title", (char*) "Daybreak-cli %s par shadow256, largement basee sur Daybreak  d'Atmosphere"}, \
	{"lng_db_init_failed_error", (char*) "Erreur durant le processus d'initialisation."}, \
	{"lng_db_get_hardware_type_error", (char*) "Echec de la recuperation du type de materiel. %d"}, \
	{"lng_db_get_rcm_bug_patch_error", (char*) "Echec de la recuperation du statut du bug RCM. %d"}, \
	{"lng_db_get_emummc_status_error", (char*) "Echec de la recuperation du status de l'emuMMC. %d"}, \
	{"lng_db_patched_model_warning", (char*) "Attention: Modele patche detecte, Vous pourriez bruler des Efuses ou rendre la console  inutilisable."}, \
	{"lng_db_update_validation", (char*) "Validation de la mise a jour, cela peut prendre du temps..."}, \
	{"lng_db_update_validation_failed_error", (char*) "Echec de la validation de la mise a jour."}, \
	{"lng_db_ask_exfat_driver_install", (char*) "Souhaitez-vous installer le driver EXFAT (recommande)?"}, \
	{"lng_db_ask_fat32_if_exfat_not_supported", (char*) "Ce firmware  ne supporte pas l'EXFAT, souhaitez-vous n'utiliser que le FAT32?"}, \
	{"lng_db_update_canceled", (char*) "Mise a jour annulee."}, \
	{"lng_db_exfat_forced_but_not_supported_error", (char*) "Erreur: Le firmware EXFAT est manquant ou corrompu."}, \
	{"lng_db_force_unsupported_firmware_enabled", (char*) "Forcage de la mise a jour sur un firmware non supportee active."}, \
	{"lng_db_ask_force_unsupported_firmware_install", (char*) "Ce firmware est trop recent et n'est pas encore supporte.\nVoulez-vous l'installer malgre tout?"}, \
	{"lng_db_firmware_not_supported_and_force_install_disabled", (char*) "Erreur: Ce firmware est trop recent et n'est pas force a l'installation, la mise a jour est annulee."}, \
	{"lng_db_reset_to_factory_forced", (char*) "Reinitialisation d'usine activee"}, \
	{"lng_db_ask_reset_to_factory", (char*) "Souhaitez-vous reinitialiser la console d'usine?"}, \
	{"lng_db_install_update_begin", (char*) "Debut de la mise en place de la mise a jour..."}, \
\
	{"lng_install_pack_recap_begin", (char*) "Recapitulatif des options choisies :"}, \
	{"lng_install_pack_recap_firmware_install", (char*) "Mise a jour du firmware, incluant le nettoyage du theme de la console s'il y en a un."}, \
	{"lng_install_pack_recap_not_install_firmware", (char*) "Le firmware ne sera pas mis a jour."}, \
	{"lng_install_pack_recap_clean_theme", (char*) "Nettoyage du theme de la console s'il y en a un."}, \
	{"lng_install_pack_recap_not_clean_theme", (char*) "Le theme de la console, s'il y en a un, ne sera pas supprime."}, \
	{"lng_install_pack_recap_agressive_clean", (char*) "Nettoyage agrressif actif."}, \
	{"lng_install_pack_recap_not_agressive_clean", (char*) "Nettoyage agressif non actif."}, \
	{"lng_install_pack_recap_clean_logos", (char*) "Les logos du pack seront supprimes."}, \
	{"lng_install_pack_recap_not_clean_logos", (char*) "Les logos du pack ne seront pas supprimes."}, \
	{"lng_install_pack_recap_install_hbmenu", (char*) "L'icone du Homebrew Menu sera mis en place."}, \
	{"lng_install_pack_recap_not_install_hbmenu", (char*) "L'icone du Homebrew Menu ne sera pas mis en place."}, \
	{"lng_install_pack_download_pack_error", (char*) "Une erreure est survenue lors du telechargement du pack."}, \
	{"lng_install_pack_folder_create", (char*) "Creation du repertoir: %s"}, \
	{"lng_install_pack_extract_file_protected", (char*) "Extraction de: %-5s"}, \
	{"lng_install_pack_extract_file", (char*) "Extraction de: %s"}, \
	{"lng_install_pack_file_write_error", (char*) "Erreur d'ecriture du fichier \"%s\", verifiez l'espace libre sur votre SD."}, \
\
	{"lng_clean_logos_begin", (char*) "Suppression des logos..."}, \
	{"lng_clean_theme_begin", (char*) "Suppression d'un eventuel theme..."}, \
	{"lng_agressive_clean_begin", (char*) "Nettoyage agressif..."}, \
	{"lng_clean_sd_begin", (char*) "Nettoyage de la SD:"}, \
	{"lng_clean_sd_clean_unusful_files", (char*) "Nettoyage des fichiers inutiles..."}, \
	{"lng_clean_sd_finish", (char*) "Nettoyage de la SD termine."}, \
\
	{"lng_install_app_download_app_error", (char*) "Une erreure est survenue lors du telechargement de l'application."}, \
\
	{"lng_dns_end_install_error", (char*) "Une erreur s'est produite durant l'application des parametres DNS."}, \
	{"lng_dns_install_set_init_error", (char*) "Echec de l'initialisation de set! Err %x"}, \
	{"lng_dns_install_setsys_init_error", (char*) "Echec de l'initialisation de setsys! Err %x"}, \
	{"lng_dns_install_get_region_code_error", (char*) "Echec de la recuperation de la region! Err %x"}, \
	{"lng_dns_install_display_region_code", (char*) "Region %s detectee"}, \
	{"lng_dns_install_use_us_dns", (char*) "Le DNS americain sera utilise en tant que DNS primaire"}, \
	{"lng_dns_install_use_eu_dns", (char*) "Le DNS europeen sera utilise en tant que DNS primaire"}, \
	{"lng_dns_install_use_unknown_dns", (char*) "Region inconnue? Le DNS americain sera utilise en tant que DNS primaire"}, \
	{"lng_dns_install_aplying", (char*) "Application de 90dns sur les reseaux wifi..."}, \
	{"lng_dns_install_get_wifi_networks_error", (char*) "Echec de la recuperation des reseaux wifi! Err %x"}, \
	{"lng_dns_install_networs_count", (char*) "Reseau wifi trouve: %d"}, \
	{"lng_dns_install_wifi_network_config_error", (char*) "Echec de la configuration du reseau wifi! Err %x"}, \
	{"lng_dns_install_success", (char*) "Configuration de 90DNS terminee!\nLa console sera redemarree pour appliquer les changements"}, \
\
	{"lng_protect_console_begin", (char*) "Application des configurations de protection..."}, \
	{"lng_protect_console_error", (char*) "Une erreur s'est produite durant l'application des parametres pour proteger la console, verifiez l'espace restant sur votre SD."}, \
\
	{"lng_dl_progress_0", (char*) "* Telechargement: %.2fMB a %.2f MB/s *"}, \
	{"lng_dl_progress_1", (char*) "* Telechargement: %.2fMB *"}, \
	{"lng_dl_progress_with_bar_0", (char*) "Telechargement: %.2fMB sur %.2fMB a %.2f MB/s"}, \
	{"lng_dl_progress_with_bar_1", (char*) "Telechargement: %.2fMB sur %.2fMB"}, \
	{"lng_dl_begin", (char*) "Telechargement de\n%s\nVeuillez patienter..."}, \
	{"lng_dl_file_write_error", (char*) "Erreur d'ecriture du fichier telecharge, verifiez l'espace libre sur la SD."}, \
	{"lng_dl_success", (char*) "Telechargement complete."}, \
	{"lng_dl_dl_error", (char*) "Erreur Durant le telechargement, verifiez votre connexion internet ainsi que l'espace restant sur votre SD puis tentez de relancer le telechargement."}, \
	{"lng_dl_open_temp_file_error", (char*) "Erreur d'ouverture du fichier temporaire, tentez de relancer le telechargement."}, \
	{"lng_dl_curl_init_error", (char*) "Erreur, l'initialisation de curl a echouee, tentez de relancer le telechargement."} \
}

translation_map set_translation_strings();

#endif