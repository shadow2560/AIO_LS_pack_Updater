@echo off
chcp 65001 >nul
cd aiosu-forwarder
make clean
cd ..\amssu-rcm
make clean
cd ..\Daybreak-cli
make clean
cd ..
make clean
pause