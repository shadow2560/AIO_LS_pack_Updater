@echo off
chcp 65001 >nul
cd aiosu-forwarder
make clean & make
copy .\aiosu-forwarder.nro ..\romfs\nro\aiosu-forwarder.nro
cd ..\amssu-rcm
make clean & make
copy .\output\ams_rcm.bin ..\romfs\payload\ams_rcm.bin
copy .\output\ams_rcm.bin ..\Daybreak-cli\romfs\payload\ams_rcm.bin
cd ..\Daybreak-cli
make clean & make
copy .\Daybreak-cli.nro ..\romfs\nro\Daybreak-cli.nro
cd ..
make clean & make
pause