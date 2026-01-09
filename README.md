## AIO_LS_pack_Updater 🖥️

AIO_LS_pack_Updater is an homebrew for Nintendo Switch which update your cfw with the pack of [shadow2560](https://github.com/shadow2560/) by default but this can also be configured with a "AIO_LS_pack_Updater.ini" file placed in the "switch/AIO_LS_pack_Updater".

The app can also auto-update itself, aply some protection for Atmosphere to be able to connect to internet with your CFW, get some usful informations about the console, install a firmware and install the HBMenu forwarder witch include the last version of the HBLoader.

## Beta mode

This mode could be useful to test the new features of the pack and the app but this also could brake some functions and could be not backward compatible with the stable version so use it if you know what you're doing. Beta mode has his own config file if you want to customize some settings, see the "config" folder of the repository.

## Agressive clean function

This function can be used when updating the pack or the firmware, if you come from an other pack. The folders witch are deleted by this function could be configured with the file "switch/AIO_LS_pack_Updater/pack_install_folders_to_delete.txt" for pack install and "switch/AIO_LS_pack_Updater/fw_install_folders_to_delete.txt" for firmware install and the files could be configured with the file "switch/AIO_LS_pack_Updater/pack_install_files_to_delete.txt" for pack install and "switch/AIO_LS_pack_Updater/pack_install_files_to_delete.txt" for firmware install, see the "config" folder of the repository, be careful with these files cause they can brake all the SD if they're badly configured. If you use one of these custom files the pack or firmware agressive clean (depend on what function you use) integrated into the homebrew will be disabled.

## Modules clean function

This function can be used when updating the pack or the firmware, if you come from an other pack or if you use modules not included in the pack. This function will delete all Atmosphere's modules in the "atmosphere/contents" folder of the SD and some extra folders if some specific modules are founded. To find the modules, the function look into the folders contained in "atmosphere/contents" and search the file "exefs.nsp" in them, if the file is founded the module is deleted. With the file switch/AIO_LS_pack_Updater/modules_to_keep.txt" (see the "configs" folder of this repository for an example, this file should contain one module ID  by line) you can filter modules that you don't want to delete.

## Files keeping function

This function can be used during pack update. It is used to keep some files/folders during process update, for example to keep some configuration files. The files/folders can be configured with the "switch/AIO_LS_pack_Updater/files_to_keep.txt" (see the "configs" folder of this repository for an example), this file should contain one file or folder by line and the path of the file/folder to keep is indicated from the SD root; note also that one line of the file could be used to define more than one folder/file to delete cause only the first letters of the path indicated is compared during the program (for example a line containing the path "atmosphere/conf" will protect any files/folders starting with this path). Also note that if the file doesn't exist on the SD it will be copied even if it is listed in the config file, this function is only used to protect existing files on the SD.

This function is not used during agressive clean or modules clean, the agressive clean and modules clean will delete any files/folders configured for it without taking care of this function's settings. In short, agressive clean and modules clean take priority over this function.

## Autoconfig

This function install automaticaly the pack and could be activated if you place the file "autoconfig.ini" in the folder "switch/AIO_LS_pack_Updater". All the configs should be set correctly, if it's not the case the autoconfig will not be enabled; you can find an example of this file in the "config" folder of this repository.

If you choose to use all functions of the app only the questions during pack installation will be configured automaticaly if you choose to install the pack and the app will switch in Beta mode if you choose to enable the param associed, else the installation sumary is displayed and you can choose to install the pack or not (if not the app will exit on Switch's home screen).

## Translate the project

You can translate the project but for now there are some limitations:
* You can't use special chars or accents
* Only supported language of Nintendo could be used, see [this page](https://switchbrew.org/wiki/Settings_services#LanguageCode) for more infos.

You can see the french language file for example, it is located in "romfs/lng/fr.ini".

Here are some Rules to translate correctly the app:
* Name your file according to Nintendo's language code, with the ".ini" extension and place it to the "romfs/lng" folder of the project. Note that if you want to test the file without recompiling the project you can place the file to the "switch/AIO_LS_pack_Updater" folder and name it "translate.ini", if you do that this file has the priority on those placed in the "romfs/lng" folder.
* Include [language] text on top of the file, this is needed to start the translation structure.
* You can start a line with a ; to comment the line
* Don't touch the % char and the chars witch follow it until the next space, this is used to display some variables of the app.
* Don't use more lines than "26" for the main menu or you could brake the display of it.
* Don't use accents or special chars.
* Include every variables and don't add some new ones or the translation file will not work.
* If you get an error while using a translation file it could be caused by a too long string for a variable.

## How to build 🏗️

To build the project you have to download devkitPro, install devkitPro and the main Switch's libs and follow these commands (no need "sudo" for windows):

``sudo pacman -S switch-curl switch-zlib switch-zziplib switch-libzstd switch-mbedtls``  
``git clone https://github.com/shadow2560/AIO_LS_pack_Updater.git``  
``cd AIO_LS_pack_Updater``  
``cd amssu-rcm``
``make``
``mv ./output/ams_rcm.bin ../romfs/payload/ams_rcm.bin``
``cd ../aiosu-forwarder ``
``make``
``mv ./aiosu-forwarder.nro ../romfs/nro/aiosu-forwarder.nro``
``cd ..``
``make``  
 
## Issues 🚩 

* Can't use accents or special chars when printing to the screen, don't use them in translations.
* Don't use special chars like accents in your files' name, included with files contained in your pack zip  (old issue, must be realy re-tested).

##  Todo list

### Things that I don't know how to do

* Display correctly accents or special chars when printing to the screen.
* Detect if SD is in EXFAT or FAT32.
* Detect emunand type using Atmosphere's detection and not using a file(s) analyse witch is unreliable.
* Install NSP(s) (forwarders or homebrews) contained in a configured folder (TitleID of the NSP should be detected to uninstall the Title on the console before installing it or eventuals orpheans files should be cleaned after installation).

### Improvements that could be made

* Verify the  consoleSelect(), consoleClear() and consoleUpdate() usage, there is probably no need for it sometimes.
* Improve or add error handling, for example when using malloc() or in the zip function when using Minizip functions.
* Improve configs files of the app datas storage, could be improved using globals Configuration structures to store the datas and witch be cleaned  when  closing the app  or  switching app mode  (second solution could be better for memory used by the app because only one global Configuration structure could  be  used against two for the first solution witch need to store the config for Normal Mode and an other for the Beta Mode).
* Improve storage of the datas for emunand config and Hekate's global config values  using a  global structure (one for the emunand and one for Hekate's global configs).
* Rewrite translation  method without  using  vectors to  have a pur C code for it, using structures could probably be a solution but it's a massive work to do.
* Reorganize code logic, massive work to do but lots of improvement can be done.
* Fix bugs, there are probably always some.
* Fix typo, ameliorate messages and maybe add some messages in translations.

## Credits 📜 

- Thanks all the contributors of the [Libnx project](https://github.com/switchbrew/libnx) and the [Homebrew Loader](https://github.com/switchbrew/nx-hbloader)
- Thanks [PoloNX](https://github.com/PoloNX) for [AtmoPackUpdater](https://github.com/PoloNX/AtmoPackUpdater) witch is the project from where mine is forked and for [this fork of the Libnx with console special chars support](https://github.com/PoloNX/libnx) witch is also used in this project
- Thanks [benhoyt](https://github.com/benhoyt/) for [Inih](https://github.com/benhoyt/inih)
- Thanks [arnavyc](https://github.com/arnavyc/) for [getdelim](https://github.com/arnavyc/getdelim)
- Thanks [SciresM](https://github.com/SciresM) for [Reboot_to_payload](https://github.com/Atmosphere-NX/Atmosphere/tree/master/troposphere/reboot_to_payload) and [Daybreak](https://github.com/Atmosphere-NX/Atmosphere/tree/master/troposphere/daybreak) and probably for others things in Atmosphere
- Thanks [sthetix](https://github.com/sthetix) for [DowngradeFixer](https://github.com/sthetix/DowngradeFixer)
- Thanks [Team Neptune](https://github.com/Team-Neptune]) for your [rcm payload](https://github.com/Team-Neptune/DeepSea-Updater/tree/master/rcm)
- Thanks [HamletDuFromage](https://github.com/HamletDuFromage) for your help and for some code in [AIO_Switch_Updater](https://github.com/HamletDuFromage/AIO-switch-updater) and also for some other codes taken from your projects (get fw version, get Atmosphere version, etc...).
- Thanks [ITotalJustice](https://github.com/ITotalJustice) for [sigpatch-updater](https://github.com/ITotalJustice/sigpatch-updater/) and [sys-patch](https://github.com/ITotalJustice/sys-patch/)
- Thanks [suchmememanyskill](https://github.com/suchmememanyskill) for [90dns-setter](https://github.com/suchmememanyskill/switch-90dns-setter/) and [90dns-tester](https://github.com/suchmememanyskill/switch-90dns-tester/
- Thanks [dezem](https://github.com/dezem) for [Atmoxl-titel-installer](https://github.com/dezem/AtmoXL-Titel-Installer)
- Thank [XorTroll](https://github.com/XorTroll) for [Goldleaf](https://github.com/XorTroll/Goldleaf)
- Thank [ITotalJustice](https://github.com/ITotalJustice) for [Reset-Parental-Controls-NX](https://github.com/ITotalJustice/Reset-Parental-Controls-NX)
- Thanks The-4n for the [Homebrew Menu forwarders](https://gbatemp.net/threads/homebrew-menu-loader-legal-nsp.518433/) that I have recompiled with the last version of the HB Loader with his tools
- Thanks [nlohmann](https://github.com/nlohmann) for [this json library](https://github.com/nlohmann/json)
- Thanks [crc-32](https://github.com/crc-32) for [BrowseNX](https://github.com/crc-32/BrowseNX)
- Thanks [nedex](https://github.com/nedex) for [QuickNTP](https://github.com/nedex/QuickNTP)
- Thanks to Dotmhd for his auto-configuration function suggestion and for the forwarder creation.
- Thanks [shadow2560](https://github.com/shadow2560/) for his [pack](https://github.com/shadow2560/switch_AIO_LS_pack)
- And thanks to the others that I may have forgotten.
