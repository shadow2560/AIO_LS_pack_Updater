#ifndef _TRANSLATE_HPP_
#define _TRANSLATE_HPP_

#include <map>
#include <string>

struct translation_map_comparator {
	bool operator()(const char* left, const char* right) const {
		return ((left != nullptr) && (right != nullptr) && (strcmp(left, right) < 0));
	}
};

typedef std::map<const char*, std::string, translation_map_comparator> translation_map;

extern translation_map language_vars;

#define INIT_LNG(X) translation_map X = {\
	{"lng_yes", (std::string) "Oui"}, \
	{"lng_no", (std::string) "Non"}, \
	{"lng_unknown_0", (std::string) "inconnu"}, \
	{"lng_unknown_1", (std::string) "inconnue"}, \
	{"lng_files", (std::string) "fichiers"}, \
	{"lng_partition", (std::string) "partition"}, \
	{"lng_usable", (std::string) "utilisable"}, \
	{"lng_not_usable", (std::string) "inutilisable"}, \
	{"lng_success_reboot_in_five_seconds", (std::string) "Finis!\n\nRedemarrage automatique dans 5 secondes"}, \
	{"lng_error_reboot_to_payload", (std::string) "Payload non trouve."}, \
	{"lng_error_not_enough_space_on_sd", (std::string) "Erreur, pas assez d'espace sur la SD."}, \
	{"lng_error_copy_file", (std::string) "Erreur de copie de fichier."}, \
	{"lng_calculate_sha256_of_downloaded_file", (std::string) "Verification du sha256 du fichier telecharge..."}, \
	{"lng_press_b_to_go_back", (std::string) "Appuyer sur (B) pour revenir au menu precedent."}, \
	{"lng_press_any_key_to_continue", (std::string) "Appuyer sur une touche pour continuer."}, \
	{"lng_display_changelog_choice", (std::string) "Afficher le changelog (synchronisera l'heure de la console)"}, \
	{"lng_error_changelog_display_applet", (std::string) "Affichage du changelog impossible car le homebrew  est en mode applet. Pour utiliser cette fonction vous ne devez pas lancer le homebrew via l'album."}, \
	{"lng_error_changelog_display_no_adress", (std::string) "Affichage du changelog impossible car aucune adresse indiquee dans la configuration du homebrew."}, \
	{"lng_error_no_internet_connection_for_function", (std::string) "Aucune connexion a internet, fonction indisponible."}, \
\
	{"lng_title", (std::string) "%s v%s - par %s"}, \
	{"lng_title_beta", (std::string) "%s v%s - par %s - BETA MODE"}, \
	{"lng_ls_menu", (std::string) "Appuyez sur (LS) pour afficher le changelog du pack (synchronisera l'heure de la console)"}, \
	{"lng_rs_menu", (std::string) "Appuyez sur (RS) pour afficher les controles du homebrew"}, \
	{"lng_move_menu", (std::string) "Appuyez sur haut ou bas sur les sticks ou la croix pour vous deplacer dans les menus"}, \
	{"lng_a_menu", (std::string) "Appuyez sur (A) pour selectionner une option"}, \
	{"lng_x_menu", (std::string) "Appuyez sur (X) pour afficher diverses informations"}, \
	{"lng_y_menu", (std::string) "Appuyez sur (Y) pour enregistrer diverses informations dans un fichier"}, \
	{"lng_minus_menu", (std::string) "Appuyez sur (-) pour passer en mode beta"}, \
	{"lng_minus_menu_beta", (std::string) "Appuyez sur (-) pour passer en mode stable"}, \
	{"lng_minus_menu_switch", (std::string) "Appuyez sur (-) pour passer entre le mode stable et le mode beta"}, \
	{"lng_l_r_menu", (std::string) "Appuyez sur (L+R) pour redemarrer la console"}, \
	{"lng_zl_zr_menu", (std::string) "Appuyez sur (ZL+ZR) pour redemarrer la console en utilisant le payload de nettoyage de l'app (a ne faire que si vous savez se que vous faite)"}, \
	{"lng_l_zl_menu", (std::string) "Appuyez sur (L+ZL) pour desactiver l'autoboot sur une configuration de Hekate"}, \
	{"lng_r_zr_menu", (std::string) "Appuyez sur (R+ZR) pour activer l'autoboot sur une configuration de Hekate"}, \
	{"lng_l_zr_menu", (std::string) "Appuyez sur (L+ZR) pour desactiver l'auto-configuration du homebrew ou si la configuration ne donne pas acces aux autres fonctions du homebrew appuyer sur (A) au lancement du homebrew lors de la demande"}, \
	{"lng_r_zl_menu", (std::string) "Appuyez sur (R+ZL) pour configurer et activer l'auto-configuration du homebrew"}, \
	{"lng_plus_menu", (std::string) "Appuyez sur (+) pour quitter l'application"}, \
	{"lng_update_app_menu", (std::string) "= Mise a jour de l'application"}, \
	{"lng_update_pack_menu", (std::string) "= Mise a jour du pack"}, \
	{"lng_update_firmware_menu", (std::string) "= Mise a jour du firmware"}, \
	{"lng_update_hbmenu_menu", (std::string) "= Installation de l'icone du Homebrew Menu (recommandee si vous ne l'avez pas)"}, \
	{"lng_set_90dns_menu", (std::string) "= Application de la protection DNS sur tous les reseaux Wifi deja configures"}, \
	{"lng_protect_console_menu", (std::string) "= Application de configurations pour  proteger au mieux la console lancee sous Atmosphere (Atmosphere 0.18.1 minimum requis)"}, \
	{"lng_install_app_fwd_menu", (std::string) "= Installation de l'icone de cet homebrew"}, \
	{"lng_reset_parental_control_menu", (std::string) "= Reinitialisation du controle parental"}, \
	{"lng_sync_time_menu", (std::string) "= Synchroniser l'heure de la console avec internet"}, \
	{"lng_reset_menu", (std::string) "= Reinitialisation du systeme"}, \
\
	{"lng_infos_begin", (std::string) "Informations:"}, \
	{"lng_infos_is_applet", (std::string) "Homebrew en mode applet, utilisation non recommandee."}, \
	{"lng_infos_is_not_applet", (std::string) "Le homebrew peut utiliser toute la RAM, utilisation recommandee."}, \
	{"lng_infos_pack_version", (std::string) "Version actuelle du pack : %s"}, \
	{"lng_infos_last_pack_version", (std::string) "Derniere version du pack : %s"}, \
	{"lng_infos_console_id", (std::string) "ID de la console : %li"}, \
	{"lng_infos_serial_incognito", (std::string) "Numero de serie de la console  indetectable, Incognito est actif."}, \
	{"lng_infos_serial", (std::string) "Numero de serie de la console : %s"}, \
	{"lng_infos_sysnand", (std::string) "Type de systeme : %s"}, \
	{"lng_infos_emunand", (std::string) "Type de systeme : %s via %s"}, \
	{"lng_infos_console_model", (std::string) "Modele de la console : %s"}, \
	{"lng_infos_fusee_gelee_patch", (std::string) "Etat de l'exploit Fusee Gelee : %s"}, \
	{"lng_infos_actual_firmware_version", (std::string) "Version actuelle du firmware : %s"}, \
	{"lng_infos_actual_atmosphere_version", (std::string) "Version actuelle d'Atmosphere : %s"}, \
	{"lng_infos_90dns_status_ok", (std::string) "Protection 90DNS: Active, la console n'a pas acces aux serveurs de Nintendo."}, \
	{"lng_infos_90dns_status_no_conection", (std::string) "Protection 90DNS: Aucune connexion au reseau, impossible de tester."}, \
	{"lng_infos_90dns_status_not_protected", (std::string) "Protection 90DNS: Danger, un ou plusieurs serveurs de Nintendo ne sont pas bloques, fort risque de bannissement de la console."}, \
	{"lng_infos_90dns_status_error", (std::string) "Protection 90DNS: Erreur,  verification impossible."}, \
	{"lng_infos_official_charge", (std::string) "Charge active, charge batterie : %d%%, type de chargeur : officiel"}, \
	{"lng_infos_usb_charge", (std::string) "Charge active, charge batterie : %d%%, type de chargeur : USB ou non officiel"}, \
	{"lng_infos_no_charge", (std::string) "Charge inactive, charge batterie : %d%%"}, \
	{"lng_infos_hekate_autoboot_enabled", (std::string) "Autoboot de Hekate : actif"}, \
	{"lng_infos_hekate_autoboot_disabled", (std::string) "Autoboot de Hekate : inactif"}, \
	{"lng_record_infos_log_not_open_error", (std::string) "Le fichier \"switch/AIO_LS_pack_Updater/console_infos.log\" n'a pas pu etre enregistre."}, \
	{"lng_record_infos_success", (std::string) "Le fichier contenant les informations de la console ont ete enregistrees dans le fichier \"switch/AIO_LS_pack_Updater/console_infos.log\"."}, \
\
	{"lng_hekate_config_choice_menu_title", (std::string) "Choix de la  configuration lancee  par  l'autoboot de Hekate :"}, \
	{"lng_hekate_config_choice_no_configs_found", (std::string) "Aucune configuration trouvee."}, \
	{"lng_no_hekate_autoboot_config_set_error", (std::string) "Parametre Parametre autoboot de Hekate non defini ou fichiers de configurations non existants."}, \
	{"lng_hekate_autoboot_config_canceled", (std::string) "Configuration de l'autoboot de Hekate annulee."}, \
	{"lng_hekate_param_already_set", (std::string) "Parametre autoboot de Hekate deja defini ainsi."}, \
	{"lng_hekate_error_config_file_read", (std::string) "Erreur de lecture du fichier de configuration de Hekate."}, \
	{"lng_hekate_error_temp_file_write", (std::string) "Erreur d'ecriture du fichier temporaire de configuration de Hekate."}, \
	{"lng_hekate_replace_config_error", (std::string) "Erreur durant le remplacement du fichier de configuration de Hekate."}, \
	{"lng_hekate_autoboot_enabled_success", (std::string) "Autoboot de Hekate actif."}, \
	{"lng_hekate_autoboot_disabled_success", (std::string) "Autoboot de Hekate inactif."}, \
\
	{"lng_configuring_app_autoconfig", (std::string) "Configuration de l'auto-configuration du homebrew."}, \
	{"lng_autoconfig_recap_begin", (std::string) "Recapitulatif des options choisies pour l'auto-configuration du homebrew :"}, \
	{"lng_autoconfig_recap_full_app_control", (std::string) "Acces aux autres fonctions du homebrew actif."}, \
	{"lng_autoconfig_recap_not_full_app_control", (std::string) "Acces aux autres fonctions du homebrew inactif."}, \
	{"lng_autoconfig_recap_beta_launch", (std::string) "Lancement en mode beta au demarrage du homebrew actif."}, \
	{"lng_autoconfig_recap_not_not_beta_launch", (std::string) "Lancement en mode beta au demarrage du homebrew inactif."}, \
	{"lng_autoconfig_set_error", (std::string) "Erreur durant la configuration de l'auto-configuration du homebrew."}, \
	{"lng_autoconfig_set_success", (std::string) "La configuration de l'auto-configuration du homebrew est active, le homebrew va se fermer dans cinq secondes."}, \
	{"lng_autoconfig_disabled", (std::string) "L'auto-configuration du homebrew est inactive."}, \
\
	{"lng_hbmenu_install_begin", (std::string) "Installation du forwarder du Homebrew Menu en cours..."}, \
	{"lng_hbmenu_install_uninstall_begin", (std::string) "Ancienne version du forwarder trouvee, desinstallation de celle-ci..."}, \
	{"lng_hbmenu_install_uninstall_error", (std::string) "Erreur durant la desinstallation de l'ancienne version du forwarder, anulation de l'installation."}, \
	{"lng_hbmenu_install_uninstall_success", (std::string) "Desinstallation de l'ancienne version du forwarder effectuee."}, \
	{"lng_hbmenu_install_success", (std::string) "Installation du forwarder du Homebrew Menu effectuee."}, \
	{"lng_hbmenu_install_error", (std::string) "Une erreur s'est produite durant l'installation du forwarder du Homebrew Menu.\nVos sig_patches ne sont peut-etre pas a jour, l'espace restant sur votre stockage est peut-etre insufisant ou vous avez peut-etre de la corruption de donnees sur votre SD ou votre nand/emunand."}, \
\
	{"lng_app_fwd_install_begin", (std::string) "Installation du forwarder de cet homebrew en cours..."}, \
	{"lng_app_fwd_install_success", (std::string) "Installation du forwarder de cet homebrew effectuee."}, \
	{"lng_app_fwd_install_error", (std::string) "Une erreur s'est produite durant l'installation du forwarder de cet homebrew.\nVos sig_patches ne sont peut-etre pas a jour, l'espace restant sur votre stockage est peut-etre insufisant ou vous avez peut-etre de la corruption de donnees sur votre SD ou votre nand/emunand."}, \
\
	{"lng_reset_parental_begin", (std::string) "Reinitialisation du controle parental en cours..."}, \
	{"lng_reset_parental_app_error", (std::string) "Reinitialisation de l'association avec l'application du controle parental echouee. %d"}, \
	{"lng_reset_parental_app_success", (std::string) "Reinitialisation de l'association avec l'application du controle parental reussie."}, \
	{"lng_reset_parental_error", (std::string) "Reinitialisation du controle parental echouee. %d"}, \
	{"lng_reset_parental_success", (std::string) "Reinitialisation de l'association avec l'application du controle parental reussie."}, \
\
	{"lng_battery_error_10", (std::string) "Impossible d'effectuer cette action dans les conditions actuelles, vous devez attendre d'etre au-dessus de 10%% de batterie."}, \
	{"lng_battery_error_20", (std::string) "Impossible d'effectuer cette action dans les conditions actuelles, vous devez attendre d'etre au-dessus de 20%% de batterie."}, \
	{"lng_battery_error_30", (std::string) "Impossible d'effectuer cette action dans les conditions actuelles, vous devez attendre d'etre au-dessus de 30%% de batterie."}, \
\
	{"lng_ask_disable_autoconfig", (std::string) "Appuyez sur le bouton [A] pour desactiver l'auto-configuration du homebrew ou attendez %.0f secondes..."}, \
	{"lng_ask_autoconfig_full_control", (std::string) "Souhaitez-vous activer l'acces aux autres fonctions du homebrew lorsque l'auto-configuration sera active (si non il faudra appuyer sur le bouton [A] au lancement du homebrew lors de la demande pour pouvoir desactiver l'auto-configuration du homebrew)?"}, \
	{"lng_ask_autoconfig_beta_launch", (std::string) "Souhaitez-vous activer  le lancement en mode beta au demarrage lorsque l'auto-configuration sera active?"}, \
	{"lng_ask_sd_exfat", (std::string) "Attention, cette fonction ne doit pas etre utilisee sur une SD en EXFAT, si votre SD est dans ce format veuillez faire une installation du pack manuellement via un PC (regarder sur votre SD, il est possible qu'un dossier de tutoriels contienne les informations appropriees).\n\nLa SD est-elle en EXFAT, si oui l'installation du pack sera annulee?"}, \
	{"lng_ask_update_firmware", (std::string) "Souhaitez-vous mettre a jour le firmware (si oui les fichiers du theme seront aussi nettoyes)?"}, \
	{"lng_ask_update_firmware_only", (std::string) "Souhaitez-vous seulement mettre a jour le firmware (si oui les fichiers du theme seront aussi nettoyes)?"}, \
	{"lng_ask_update_app", (std::string) "Nouvelle version du homebrew trouvee, souhaitez-vous le mettre a jour?"}, \
	{"lng_ask_app_need_update", (std::string) "Nouvelle version du homebrew trouvee, vous devez le mettre a jour avant de pouvoir effectuer cette action.\n\nSouhaitez-vous mettre a jour le homebrew?"}, \
	{"lng_pack_have_update", (std::string) "Nouvelle version du pack trouvee, la version du pack sur la console est la version %s et la derniere version est la version %s."}, \
	{"lng_pack_have_no_update", (std::string) "Aucune nouvelle version du pack trouvee, la version du pack sur la console est la version %s et la derniere version est la version %s."}, \
	{"lng_ask_pack_have_update", (std::string) "Souhaitez-vous mettre a jour le pack?"}, \
	{"lng_ask_clean_theme", (std::string) "Souhaitez-vous nettoyer les fichiers du theme, utile si mise a jour du firmware par la suite?"}, \
	{"lng_ask_agressive_clean", (std::string) "Souhaitez-vous faire un nettoyage agressif, utile si mise a jour du firmware ou si vous venez d'un autre pack?"}, \
	{"lng_ask_clean_modules", (std::string) "Souhaitez-vous faire un nettoyage des modules, utile si mise a jour du firmware ou si vous venez d'un autre pack?"}, \
	{"lng_ask_keep_files", (std::string) "Souhaitez-vous activer la conservation de certains fichiers (non recommande en general)?"}, \
	{"lng_ask_hekate_autoboot", (std::string) "Souhaitez-vous activer l'autoboot sur une configuration de Hekate (le choix de la configuration lancee sera a faire durant l'installation du pack et sera obligatoire; pour demarrer sous Hekate avec l'autoboot actif il faut maintenir \"vol-\" au lancement de Hekate (avec certaines puces maintenir le bouton juste apres le glitch))? Si non l'autoboot de Hekate sera desactive."}, \
	{"lng_ask_clean_logos", (std::string) "Souhaitez-vous retirer les logos?"}, \
	{"lng_ask_hbmenu_install", (std::string) "Souhaitez-vous installer l'icone du Homebrew Menu permettant de lancer les homebrews en profitant de toute la RAM de la console (vivement recommande si vous ne l'avez pas)?"}, \
	{"lng_ask_app_fwd_install", (std::string) "Souhaitez-vous installer l'icone du forwarder de cet homebrew?"}, \
	{"lng_ask_validate_choices", (std::string) "Souhaitez-vous vraiment continuer?"}, \
	{"lng_ask_validate_choices_for_parental_reset", (std::string) "Ceci va reinitialiser le controle parental.\n\nSouhaitez-vous vraiment continuer?"}, \
	{"lng_ask_validate_choices_for_sync_time", (std::string) "Souhaitez-vous vraiment synchroniser l'heure de la console via internet?"}, \
	{"lng_ask_validate_choices_for_reset", (std::string) "Ceci va reinitialiser le systeme, toutes les donnees liees a celui-ci seront supprimees.\n\nSouhaitez-vous vraiment continuer?"}, \
	{"lng_ask_force_error_with_sha256", (std::string) "Souhaitez-vous quand même continuer malgre l'erreur de la verification du sha256?"}, \
\
	{"lng_install_firmware_end_error", (std::string) "Une erreur s'est produite durant l'installation du firmware."}, \
	{"lng_install_firmware_end_success", (std::string) "Mise a jour du firmware effectuee."}, \
	{"lng_install_firmware_error_folder_not_found", (std::string) "Le repertoire du firmware pour mettre a jour la console n'a pas pu etre ouvert, la mise a jour du firmware ne sera pas faite."}, \
	{"lng_install_firmware_folder_location", (std::string) "Le dossier contenant le firmware est le dossier \"%s\"."}, \
\
	{"lng_db_pause", (std::string) "Appuyez sur \"A\" pour continuer..."}, \
	{"lng_db_atmosphere_not_found_error", (std::string) "Atmosphere non trouve, Daybreak requiere Atmosphere. %d"}, \
	{"lng_db_atmosphere_outdated_error", (std::string) "Version d'Atmosphere depassee, Daybreak requiere Atmosphere 0.14.0 ou superieure. %d"}, \
	{"lng_db_nso_launch_error", (std::string) "Environment non supporte, Lancez Daybreak via le Homebrew menu. %d"}, \
	{"lng_db_update_not_found_error", (std::string) "Aucune mise a jour trouvee dans le dossier, assurez-vous que vos ncas sont nommes correctement!\nResult: 0x%08x"}, \
	{"lng_db_get_update_infos_error", (std::string) "Echec de recuperation des informations de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_update_infos_fw_version", (std::string) "- Version: %d.%d.%d"}, \
	{"lng_db_update_infos_exfat_support", (std::string) "- exFAT: Supporte"}, \
	{"lng_db_update_infos_exfat_no_support", (std::string) "- exFAT: Non supporte"}, \
	{"lng_db_update_infos_fw_version_variations", (std::string) "- Firmware variations: %d"}, \
	{"lng_db_update_validation_error", (std::string) "Echec de la validation de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_update_validation_success", (std::string) "La mise a jour est valide!"}, \
	{"lng_db_update_validation_exfat_error", (std::string) "Echec de la validation de l'exFAT.\nResult: 0x%08x"}, \
	{"lng_db_update_validation_missing_content", (std::string) "Contenu manquant:\n- Program id: %016lx\n- Version: %d.%d.%d"}, \
	{"lng_db_update_validation_content_error", (std::string) "Echec de la validation \nResult: 0x%08x"}, \
	{"lng_db_install_process_setup_error", (std::string) "Echec durant la mise en place de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_setup_success", (std::string) "Mise a jour parametree."}, \
	{"lng_db_install_process_request_preparation_error", (std::string) "Echec de la requete de preparation de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_preparing", (std::string) "Preparation de la mise a jour..."}, \
	{"lng_db_install_process_get_preparation_result_error", (std::string) "Echec de la recuperation du resultat de la preparation de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_preparation_error", (std::string) "Echec de la preparation de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_preparation_result_error", (std::string) "Echec de la verification pour savoir si la mise a jour a bien ete preparee.\nResult: 0x%08x"}, \
	{"lng_db_install_process_preparation_success", (std::string) "Preparation de la mise a jour effectuee."}, \
	{"lng_db_install_process_applying_update", (std::string) "Application de la mise a jour..."}, \
	{"lng_db_install_process_get_progress_error", (std::string) "Echec de la recuperation de la progression de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_applying_update_error", (std::string) "Echec de l'application de la mise a jour.\nResult: 0x%08x"}, \
	{"lng_db_install_process_success", (std::string) "Application de la mise a jour effectuee."}, \
	{"lng_db_install_process_reset_to_factory_error", (std::string) "Echec de la reinitialisation d'usine de la console.\nResult: 0x%08x"}, \
	{"lng_db_install_process_reset_to_factory_for_refurbishment_error", (std::string) "Echec de la reinitialisation d'usine pour la redistribution de la console.\nResult: 0x%08x"}, \
	{"lng_db_install_process_reset_to_factory_success", (std::string) "Reinitialisation d'usine effectuee. %d"}, \
	{"lng_db_title", (std::string) "Daybreak-cli %s par shadow256, largement basee sur Daybreak  d'Atmosphere"}, \
	{"lng_db_init_failed_error", (std::string) "Erreur durant le processus d'initialisation."}, \
	{"lng_db_get_hardware_type_error", (std::string) "Echec de la recuperation du type de materiel. %d"}, \
	{"lng_db_get_rcm_bug_patch_error", (std::string) "Echec de la recuperation du statut du bug RCM. %d"}, \
	{"lng_db_get_emummc_status_error", (std::string) "Echec de la recuperation du status de l'emuMMC. %d"}, \
	{"lng_db_patched_model_warning", (std::string) "Attention: Modele patche detecte, Vous pourriez bruler des Efuses ou rendre la console  inutilisable."}, \
	{"lng_db_update_validation", (std::string) "Validation de la mise a jour, cela peut prendre du temps..."}, \
	{"lng_db_update_validation_failed_error", (std::string) "Echec de la validation de la mise a jour."}, \
	{"lng_db_ask_exfat_driver_install", (std::string) "Souhaitez-vous installer le driver EXFAT (recommande)?"}, \
	{"lng_db_ask_fat32_if_exfat_not_supported", (std::string) "Ce firmware  ne supporte pas l'EXFAT, souhaitez-vous n'utiliser que le FAT32?"}, \
	{"lng_db_update_canceled", (std::string) "Mise a jour annulee."}, \
	{"lng_db_exfat_forced_but_not_supported_error", (std::string) "Erreur: Le firmware EXFAT est manquant ou corrompu."}, \
	{"lng_db_force_unsupported_firmware_enabled", (std::string) "Forcage de la mise a jour sur un firmware non supportee active."}, \
	{"lng_db_ask_force_unsupported_firmware_install", (std::string) "Ce firmware est trop recent et n'est pas encore supporte.\nVoulez-vous l'installer malgre tout?"}, \
	{"lng_db_firmware_not_supported_and_force_install_disabled", (std::string) "Erreur: Ce firmware est trop recent et n'est pas force a l'installation, la mise a jour est annulee."}, \
	{"lng_db_reset_to_factory_forced", (std::string) "Reinitialisation d'usine activee"}, \
	{"lng_db_ask_reset_to_factory", (std::string) "Souhaitez-vous reinitialiser la console d'usine?"}, \
	{"lng_db_install_update_begin", (std::string) "Debut de la mise en place de la mise a jour..."}, \
\
	{"lng_install_pack_recap_begin", (std::string) "Recapitulatif des options choisies :"}, \
	{"lng_install_pack_recap_firmware_install", (std::string) "Mise a jour du firmware, incluant le nettoyage du theme de la console s'il y en a un."}, \
	{"lng_install_pack_recap_not_install_firmware", (std::string) "Le firmware ne sera pas mis a jour."}, \
	{"lng_install_pack_recap_clean_theme", (std::string) "Nettoyage du theme de la console s'il y en a un."}, \
	{"lng_install_pack_recap_not_clean_theme", (std::string) "Le theme de la console, s'il y en a un, ne sera pas supprime."}, \
	{"lng_install_pack_recap_agressive_clean", (std::string) "Nettoyage agrressif actif."}, \
	{"lng_install_pack_recap_not_agressive_clean", (std::string) "Nettoyage agressif non actif."}, \
	{"lng_install_pack_recap_clean_modules", (std::string) "Nettoyage des modules actif."}, \
	{"lng_install_pack_recap_not_clean_modules", (std::string) "Nettoyage des modules non actif."}, \
	{"lng_install_pack_recap_keep_files", (std::string) "Conservation de fichiers active."}, \
	{"lng_install_pack_recap_not_keep_files", (std::string) "Conservation de fichiers non active."}, \
	{"lng_install_pack_recap_enable_hekate_autoboot", (std::string) "Autoboot  sur une configuration de Hekate actif."}, \
	{"lng_install_pack_recap_not_enable_hekate_autoboot", (std::string) "Autoboot  sur une configuration de Hekate non  actif."}, \
	{"lng_install_pack_recap_clean_logos", (std::string) "Les logos du pack seront supprimes."}, \
	{"lng_install_pack_recap_not_clean_logos", (std::string) "Les logos du pack ne seront pas supprimes."}, \
	{"lng_install_pack_recap_install_hbmenu", (std::string) "L'icone du Homebrew Menu sera mis en place."}, \
	{"lng_install_pack_recap_not_install_hbmenu", (std::string) "L'icone du Homebrew Menu ne sera pas mis en place."}, \
	{"lng_install_pack_recap_install_app_fwd", (std::string) "L'icone du forwarder de cet homebrew sera mis en place."}, \
	{"lng_install_pack_recap_not_install_app_fwd", (std::string) "L'icone du forwarder de cet homebrew ne sera pas mis en place."}, \
	{"lng_install_pack_download_pack_error", (std::string) "Une erreure est survenue lors du telechargement du pack."}, \
	{"lng_install_custom_files_pack_download_error", (std::string) "Une erreure est survenue lors du telechargement du fichier zip complementaire au pack."}, \
	{"lng_install_pack_file_skip", (std::string) "Saut de la copie du fichier ou dossier \"%s\" via la protection de fichiers"}, \
	{"lng_install_pack_folder_create", (std::string) "Creation du repertoir: %s"}, \
	{"lng_install_pack_same_files", (std::string) "Fichier \"%s\" identique dans le zip et sur la SD, copie du fichier non necessaire."}, \
	{"lng_install_pack_extract_file_protected", (std::string) "Extraction de: %-5s"}, \
	{"lng_install_pack_extract_file", (std::string) "Extraction de: %s"}, \
	{"lng_install_pack_file_write__retrying_error", (std::string) "Erreur d'ecriture du fichier \"%s\", tentative d'une nouvelle copie..."}, \
	{"lng_install_pack_file_write_error", (std::string) "Erreur d'ecriture du fichier \"%s\", verifiez l'espace libre sur votre SD."}, \
	{"lng_installing_pack_custom_files", (std::string) "Installation de fichiers complementaires au pack..."}, \
	{"lng_install_pack_configuring_hekate_autoboot", (std::string) "Configuration de l'autoboot de Hekate..."}, \
\
	{"lng_clean_logos_begin", (std::string) "Suppression des logos..."}, \
	{"lng_clean_theme_begin", (std::string) "Suppression d'un eventuel theme..."}, \
	{"lng_agressive_clean_begin", (std::string) "Nettoyage agressif..."}, \
	{"lng_clean_modules_begin", (std::string) "Nettoyage des modules..."}, \
	{"lng_clean_sd_begin", (std::string) "Nettoyage de la SD:"}, \
	{"lng_clean_sd_clean_unusful_files", (std::string) "Nettoyage des fichiers inutiles..."}, \
	{"lng_clean_sd_finish", (std::string) "Nettoyage de la SD termine."}, \
\
	{"lng_install_app_download_app_error", (std::string) "Une erreure est survenue lors du telechargement de l'application."}, \
\
	{"lng_dns_end_install_error", (std::string) "Une erreur s'est produite durant l'application des parametres DNS."}, \
	{"lng_dns_install_set_init_error", (std::string) "Echec de l'initialisation de set! Err %x"}, \
	{"lng_dns_install_setsys_init_error", (std::string) "Echec de l'initialisation de setsys! Err %x"}, \
	{"lng_dns_install_get_region_code_error", (std::string) "Echec de la recuperation de la region! Err %x"}, \
	{"lng_dns_install_display_region_code", (std::string) "Region %s detectee"}, \
	{"lng_dns_install_use_us_dns", (std::string) "Le DNS americain sera utilise en tant que DNS primaire"}, \
	{"lng_dns_install_use_eu_dns", (std::string) "Le DNS europeen sera utilise en tant que DNS primaire"}, \
	{"lng_dns_install_use_unknown_dns", (std::string) "Region inconnue? Le DNS americain sera utilise en tant que DNS primaire"}, \
	{"lng_dns_install_aplying", (std::string) "Application de 90dns sur les reseaux wifi..."}, \
	{"lng_dns_install_get_wifi_networks_error", (std::string) "Echec de la recuperation des reseaux wifi! Err %x"}, \
	{"lng_dns_install_networs_count", (std::string) "Reseau wifi trouve: %d"}, \
	{"lng_dns_install_wifi_network_config_error", (std::string) "Echec de la configuration du reseau wifi! Err %x"}, \
	{"lng_dns_install_success", (std::string) "Configuration de 90DNS terminee!\nLa console sera redemarree pour appliquer les changements"}, \
\
	{"lng_protect_console_begin", (std::string) "Application des configurations de protection..."}, \
	{"lng_protect_console_error", (std::string) "Une erreur s'est produite durant l'application des parametres pour proteger la console, verifiez l'espace restant sur votre SD."}, \
\
	{"lng_sync_time_begin", (std::string) "Synchronisation de l'heure..."}, \
	{"lng_sync_time_error", (std::string) "Une erreur s'est produite durant la synchronisation de l'heure avec internet."}, \
	{"lng_sync_time_success", (std::string) "Synchronisation de l'heure effectuee."}, \
\
	{"lng_dl_progress_0", (std::string) "* Telechargement: %.2fMO a %.2f MO/s *"}, \
	{"lng_dl_progress_1", (std::string) "* Telechargement: %.2fMO *"}, \
	{"lng_dl_progress_with_bar_0", (std::string) "Telechargement: %.2fMO sur %.2fMO a %.2f MO/s"}, \
	{"lng_dl_progress_with_bar_1", (std::string) "Telechargement: %.2fMO sur %.2fMO"}, \
	{"lng_dl_begin", (std::string) "Telechargement de\n%s\nVeuillez patienter..."}, \
	{"lng_dl_file_write_error", (std::string) "Erreur d'ecriture du fichier telecharge, verifiez l'espace libre sur la SD."}, \
	{"lng_dl_success", (std::string) "Telechargement complete."}, \
	{"lng_dl_dl_error", (std::string) "Erreur Durant le telechargement, verifiez votre connexion internet ainsi que l'espace restant sur votre SD puis tentez de relancer le telechargement."}, \
	{"lng_dl_open_temp_file_error", (std::string) "Erreur d'ouverture du fichier temporaire, tentez de relancer le telechargement."}, \
	{"lng_dl_curl_init_error", (std::string) "Erreur, l'initialisation de curl a echouee, tentez de relancer le telechargement."} \
}

translation_map set_translation_strings();

#endif