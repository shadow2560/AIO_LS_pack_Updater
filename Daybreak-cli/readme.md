# Daybreak-cli

A daybreak like but in console mode and params passing.

## How to use:

This homebrew is designed to be launched by a forwarder or an other homebrew. Params are:

* The first param is the path of the firmware folder. This param is the only one that is needed.
* The second param is a bool witch active or not the factory reset.
* The third param is a bool witch active or not the EXFAT driver installation.
* The fourth param is a bool witch active or not the installation of a firmware not supported by Atmosphere.

Note: Bool param must be passed as strings, for example to launch it via a homebrew :
envSetNextLoad("/switch/AIO_LS_pack_Updater/Daybreak-cli.nro", "\"/switch/AIO_LS_pack_Updater/Daybreak-cli.nro\" \"my_firmware_folder_path\" \"false\" \"true\" \"false\"");

This command launch the homebrew for installing the firmware located at "my_firmware_folder_path" with no factory reset, EXFAT driver installation and doesn't force install if the firmware is not known to be supported by Atmosphere.

If you want to have interactive questions by the program the second, third and fourth params could be defined to "NULL".

Bool param should be defined to "true" or "false", be careful with the case.

Note: The amssu-rcm payload witch is used to clean the app should be adapted to your app's paths.