@echo off
chcp 65001 >nul
cd aiosu-forwarder
make clean & make -j%number_of_processors%
copy .\aiosu-forwarder.nro ..\romfs\nro\aiosu-forwarder.nro
cd ..\amssu-rcm
make clean & make -j%number_of_processors%
copy .\output\ams_rcm.bin ..\romfs\payload\ams_rcm.bin
cd ..
make clean & make -j%number_of_processors%
pause