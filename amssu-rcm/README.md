# LockSmith-RCM

## Warnings

This payload can dod a lot of things that can dammage your console, I'm not responsable for loss of datas, brick  or even hardware destruction by using this payload.

Please be sure to have the console's battery charged, especialy if you use the unbrick functions witch can take time and corrupt your storage if brutaly interrupted.

Don't remove the SD during execution of the payload, if you want to change SD shutdown the console, replace the SD ans launch the payload.

Copy Hekate's minerva module in "bootloader/sys/libsys_minerva.bso", already in place if you have installed Hekate. If you don't do it the payload will be slower than if you do it.

## Functionnalities
This payload, largely based on [Hekate](https://github.com/CTCaer/hekate), it BDK, [TegraExplorer](https://github.com/suchmememanyskill/TegraExplorer) and Lockpick-RCM, can do a lot of things:
* Work on most configs than before for each project based on Lockpick-RCM included (all witch work on or with the nand)
* Work on emunand (Atmosphere's one in "emummc/emummc.ini" or those configure in Hekate's configs with the var "emupath") or sysnand
* Launch almost functions when flags files are founded, in this case the menu will not display and the reboot will be done on "payload.bin", "bootloader/update.bin" or "atmosphere/reboot_to_payload.bin". At the end a log will be displayed to show up what has been done and the log file will be saved to "LockSmith-RCM/log.txt" on the SD. Flag files are removed if the function has been executed.
* Grey out options that can't be used in your config (Mariko consoles can't reboot to RCM, build PRODINFO from donor can't be donne if files are missing, etc...)
* Register a screenshot at the end of each function if launched without flag file (99999 screenshots for each function)
* Load the file "sd:/LockSmith-RCM/prod.keys" to set bis keys slots (decrypt nands), usful to work on a nand that is not from the console (this will grey out some options like generating PRODINFO, dump keys, show Efuses infos). If the file is present it will load this keys by default so be careful if you use flags files cause they will use these keys. If error when reading the file (some bis keys miss) or if no nandd can be read via these keys this will fallback to the console's keys. If payload launched without flag file you can switch to console keys or file keys at any time.
* Dump keys
* Build and flash a PRODINFO, from donor or from scratch, based on [ProdinfoGen](https://github.com/CaramelDunes/prodinfo_gen). If you choose to flash it you must backup your PRODINFO first if you want to restore it later, the payload will not do it.
* Fix downgrade from firmware 21.0.0+ to a lower firmware, based on [DowngradeFixer](https://github.com/sthetix/DowngradeFixer) but largely modified
* Remove parental control
* Wip nand
* Flash an EmmcHacGen package placed in "sd:/cdj_package_files", with or without wip.
* Remove ERPT save (dangerous)
* Synchronize joycons between nands (dangerous)
* Display efuses check and diagnostic, largely based on [FuseCheck](https://github.com/sthetix/FuseCheck) but with internal DB, no suport for external DB and no display Efuses table
* Reboot to a payload
* Reboot to OFW with bypass fuses or not
* Reboot to RCM
* Poweroff the console
* Reboot the payload itself
* AIO_LS_pack_Updater can use a special file flag to update some files during pack update, in this case the menu will not display and the reboot will be done on "payload.bin", "bootloader/update.bin" or "atmosphere/reboot_to_payload.bin", this is based on amssu-rcm. If using this flag no log will be recorded and reboot will be done automaticaly without user interaction.

## Flags files
These files can be placed in "sd:/LockSmith-RCM" folder to launch a specific functions. You can place any flag you want, all functions will be executed then the payload will reboot to a payload (or will reboot the console on Mariko models). Files should only be named and placed correctly and can be empty.

List of files and associated functions:
| filename | function launched |
|----------|-------------------|
| fix_dg_sysnand | Fix downgrade from firmware 21.0.0+ to a lower firmware on sysnand |
| fix_dg_emunand | Fix downgrade from firmware 21.0.0+ to a lower firmware on emunand |
| wip_sysnand | Wip sysnand |
| wip_emunand | Wip emunand |
| rm_parental_control_sysnand | Remove parental control on sysnand |
| rm_parental_control_emunand | Remove parental control on emunand |
| unbrick_sysnand | Flash a generated EmmcHacGen package placed in "cdj_package_files" on the SD to the sysnand |
| unbrick_emunand | Flash a generated EmmcHacGen package placed in "cdj_package_files" on the SD to the emunand (could take some time to flash) |
| unbrick_and_wip_sysnand | Flash a generated EmmcHacGen package placed in "cdj_package_files" on the SD to the sysnand and wip the sysnand (don't mix with "unbrick_sysnand", it will double the work time for nothing) |
| unbrick_and_wip_emunand | Flash a generated EmmcHacGen package placed in "cdj_package_files" on the SD to the emunand and wip the emunand (don't mix with "unbrick_emunand", it will double the work time for nothing) (could take some time to flash) |
| rm_erpt_sysnand | Remove ERPT save on sysnand, do it only if someone told you to do so |
| rm_erpt_emunand | Remove ERPT save on emunand, do it only if someone told you to do so |
| sync_joycons_sysnand | Synchronize joycons configs from sysnand to emunand, dangerous, use it if you know what you're doing |
| sync_joycons_emunand | Synchronize joycons configs from emunand to sysnand, normaly not needed, dangerous, use it if you know what you're doing |
| prodinfogen_flash_scratch_sysnand | Build and flash a PRODINFO from scratch on sysnand, dont't do it on an other nand than the console's one |
| prodinfogen_flash_scratch_emunand | Build and flash a PRODINFO from scratch on emunand, dont't do it on an other nand than the console's one |
| prodinfogen_flash_donor_sysnand | Build and flash a PRODINFO from donor on sysnand (need a decrypted donor PRODINFO and eventualy a donor keys file placed in the "switch" folder of the SD, like in ProdinfoGen payload), dont't do it on an other nand than the console's one |
| prodinfogen_flash_donor_emunand | Build and flash a PRODINFO from donor on emunand (need a decrypted donor PRODINFO and eventualy a donor keys file placed in the "switch" folder of the SD, like in ProdinfoGen payload), dont't do it on an other nand than the console's one |
| dump_keys_sysnand | Dump keys from sysnand, if nand is not the console's one it will only dump console's keys |
| dump_keys_emunand | Dump keys from emunand, don't mix up with "dump_keys_sysnand" or else the sysnand dump will be erased, if nand is not the console's one it will only dump console's keys |
| dump_amiibo_keys | Dump amiibo keys of the console |

## To do:
* Optimize or factorize some functions or elements, like sd_mount() calls.
* Remove all gfx_printf() without params and use gfx_puts() witch is more appropriate in this case.
* Restore BCPKG partitions only from EmmcHacGen package, useful to restore EXFAT driver.
* Dump/restore PRODINFO and eventualy PRODINFOF, BOOT0 and BOOT1 partitions (to/from "LockSmith-RCM/backups/[emmc_id]" folder).
* Dumping/restoring functions must also work on splited files partition (not realy needed for now cause no functions actualy need it)
* Choose the emunand to work on by parsing also the Hekate's configs and config the emunand vars accordingly. Done but not tested.
* Test Hekate's emunand configs, if we have the same "emupath" for X emunands we must keep only one
* Improve the size of the payload, we are on the limits with the compressed one. I've tried to rewrite the ProdinfoGen part to use nx_emmc_cal0_t struct but the payload is bigger with these modifications, I have thinked that using this same struct already used in the payload could reduce the size but it's not the case so I'll keep the old version (keeping the rewrited version in the ".bak" files in "source/prodinfogen" folder just in case I want to re-work on that later, I haven't even tested the modifications so it even may not work for now).
* Add the possibility to work on a forced emunand config if a file is defined with some values (will be usful for AIO_LS_pack_Updater work). Seems to be difficult to obtain the info of start sector for partition emunand when HOS is launched so this may not be done.
* If "LOG_MAX_ENTRIES" is defined to a value of 300 we have corrupted memory, using a value of 250 or 1024 seems to work but it's a strange behaviour, shoud be investigated. May be a problem of a too large BSS, may be a problem with the flag "-os" used during compilation, normaly Hekate use "-o2" flag but if "-o2" is used on this payload it has a too big size with compressed version. Seems to be a too big BSS during the tests, now based on malloc.
* Fix a potential memory corruption in the main function, for now if we move the content of the function "init_payload()" in the main function the payload reboot functions are broken, maybe this could be fixed by updating the BDK or change some init value on the main function (update like Hekate do) or in the Makefile (also update it like Hekate's one). This has not been re-tested since BDK update, should be investigated, probably a too big BSS during the tests.
* Make extensive tests on Fatfs copy, for example with a buffer of 8 KB it could cause some problems if a folder contains a lot of files in some conditions that I'm not understand (make an unbrick with wip on emunand, launch the CFW, shut down before making the first config and re-launch the unbrick with wip process, with a 8 KB buffer this will copy only 127 files in the "Contents/registered" folder). With actual buffers this seems to not appen but this should be investigated; the problem could happened if cache is enabled during bis mount even with the actual buffer so disabling cache with bis seems to be the solution.

## Update for new keys

Update ""/source/hos/hos.h", "/source/keys/crypto.h" and "/source/keys/key_sources.inl"  with the  new keys fromm [this file from Atmosphere](https://github.com/Atmosphere-NX/Atmosphere/blob/master/exosphere/program/source/boot/secmon_boot_key_data.s).

For "mariko_master_kek_sources_dev", "mariko_master_kek_sources" and "master_kek_sources" in "/source/keys/key_sources.inl" you must use the keys on [this file from Atmosphere](https://github.com/Atmosphere-NX/Atmosphere/blob/master/fusee/program/source/fusee_key_derivation.cpp) (updated when keys change so if you want an old key you must download the sources of Atmosphere released for this firmware):

If the payload freeze during keys dump try to modify the file "/source/keys/keys.c" (search the line starting with "u32 text_buffer_size = MAX(_titlekey_count * sizeof(titlekey_text_buffer_t) + 1, " in function "static void _save_keys_to_sd(key_storage_t *keys, titlekey_buffer_t *titlekey_buffer, bool is_dev) {") and increase the size for the second param of max() function (now it's SZ_32K, before was SZ_16K).

## Build

Follow the guide located [here](https://devkitpro.org/wiki/Getting_Started) to install and configure all the tools necessary for the build process. You need to install these package via "pacman" or "dkp-pacman" to build:
` pacman -Syuu
` pacman -S switch-dev devkitARM devkitarm-rules

Then clone andd build the project:
```
git clone https://github.com/shadow2560/LockSmith-RCM.git
cd LockSmith-RCM
make
