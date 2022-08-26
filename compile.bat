@echo off
chcp 65001 >nul
cd aiosu-forwarder
make clean & make
copy .\aiosu-forwarder.nro ..\romfs\nro\aiosu-forwarder.nro
cd ..\amssu-rcm
make clean & make
copy .\output\ams_rcm.bin ..\romfs\payload\ams_rcm.bin
cd ..
make clean & make
pause