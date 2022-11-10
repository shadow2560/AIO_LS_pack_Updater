## AIO_LS_pack_Updater 🖥️

AIO_LS_pack_Updater is an homebrew for Nintendo Switch which update your cfw with the pack of [shadow2560](https://github.com/shadow2560/) by default but this can also be configured with a "AIO_LS_pack_Updater.ini" file placed in the "switch/AIO_LS_pack_Updater".

The app can also auto-update itself, aply some protection for Atmosphere to be able to connect to internet with your CFW, get some usful informations about the console, install a firmware and install the HBMenu forwarder witch include the last version of the HBLoader.

## Beta mode

This mode could be useful to test the new features of the pack and the app but this also could brake some functions and could be not backward compatible with the stable version so use it if you know what you're doing. Beta mode has his own config file if you want to customize some settings, see the "config" folder of the repository.

## Modules folders deletion

This function can be used when updating the pack or the firmware. The folders witch are deleted by this function could be configured with the file "switch/AIO_LS_pack_Updater/modules_folders_to_delete.txt", see the "config" folder of the repository, be careful with this file cause it can brake all the SD if it's badly configured.

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

To build the project you have to download devkitPro, install devkitPro and the main Switch's libs and follow these commands :

``sudo pacman -S switch-curl switch-zlib``  
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

* Can't use accents or special chars when printing to the screen.
* Don't use special chars like accents in your files' name, included with files contained in your pack zip.
* Can't detect if SD is in EXFAT or FAT32

## Credits 📜 

- Thanks all the contributors of the [Libnx project](https://github.com/switchbrew/libnx) and the [Homebrew Loader](https://github.com/switchbrew/nx-hbloader)
- Thanks [PoloNX](https://github.com/PoloNX) for [AtmoPackUpdater](https://github.com/PoloNX/AtmoPackUpdater) witch is the project from where mine is forked
- Thanks [benhoyt](https://github.com/benhoyt/) for [Inih](https://github.com/benhoyt/inih)
- Thanks [SciresM](https://github.com/SciresM) for [Reboot_to_payload](https://github.com/Atmosphere-NX/Atmosphere/tree/master/troposphere/reboot_to_payload) and [Daybreak](https://github.com/Atmosphere-NX/Atmosphere/tree/master/troposphere/daybreak) and probably for others things in Atmosphere
- Thanks [Team Neptune](https://github.com/Team-Neptune]) for your [rcm payload](https://github.com/Team-Neptune/DeepSea-Updater/tree/master/rcm)
- Thanks [HamletDuFromage](https://github.com/HamletDuFromage) for your help and for some code in [AIO_Switch_Updater](https://github.com/HamletDuFromage/AIO-switch-updater) and also for some other codes taken from your projects (get fw version, get Atmosphere version, etc...).
- Thanks [ITotalJustice](https://github.com/ITotalJustice) for [sigpatch-updater](https://github.com/ITotalJustice/sigpatch-updater/)
- Thanks [suchmememanyskill](https://github.com/suchmememanyskill) for [90dns-setter](https://github.com/suchmememanyskill/switch-90dns-setter/)
- Thanks [dezem](https://github.com/dezem) for [Atmoxl-titel-installer](https://github.com/dezem/AtmoXL-Titel-Installer)
- Thank [XorTroll](https://github.com/XorTroll) for [Goldleaf](https://github.com/XorTroll/Goldleaf)
- Thanks The-4n for the [Homebrew Menu forwarders](https://gbatemp.net/threads/homebrew-menu-loader-legal-nsp.518433/) that I have recompiled with the last version of the HB Loader with his tools
- Thanks [shadow2560](https://github.com/shadow2560/) for his [pack](https://github.com/shadow2560/switch_AIO_LS_pack)
- And thanks to the others that I may have forgotten.
