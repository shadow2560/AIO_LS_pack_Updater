## AIO_LS_pack_Updater 🖥️

AIO_LS_pack_Updater is an homebrew for Nintendo Switch which update your cfw with the pack of [shadow2560](https://github.com/shadow2560/)

## How to build 🏗️

To build the project you have to download devkitPro, install devkitPro and follow these commands :

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

<ul>
<li>If you update the application two times without exiting totaly HBMenu the third time no download will work but if you relaunch the download directly after the error it will work properly.</li>
</ul>

## Credits 📜 

- Thanks [PoloNX](https://github.com/PoloNX) for [AtmoPackUpdater](https://github.com/PoloNX/AtmoPackUpdater) witch is the project from where mine is forked
- Thanks [benhoyt](https://github.com/benhoyt/) for [Inih](https://github.com/benhoyt/inih)
- Thanks [SciresM](https://github.com/SciresM) for [reboot_to_payload](https://github.com/Atmosphere-NX/Atmosphere/tree/master/troposphere/reboot_to_payload)
- Thanks [Team Neptune](https://github.com/Team-Neptune]) for your [rcm payload](https://github.com/Team-Neptune/DeepSea-Updater/tree/master/rcm)
- Thanks [HamletDuFromage](https://github.com/HamletDuFromage) for your help and for some code in [AIO_Switch_Updater](https://github.com/HamletDuFromage/AIO-switch-updater).
- Thanks [ITotalJustice](https://github.com/ITotalJustice) for [sigpatch-updater](https://github.com/ITotalJustice/sigpatch-updater/)
- Thanks [suchmememanyskill](https://github.com/suchmememanyskill) for [90dns-setter](https://github.com/suchmememanyskill/switch-90dns-setter/)
- Thanks [shadow2560](https://github.com/shadow2560/) for his [pack](https://github.com/shadow2560/switch_AIO_LS_pack)
- And thanks to the others that I may have forgotten.
