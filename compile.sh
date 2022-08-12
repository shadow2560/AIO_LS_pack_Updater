cd aiosu-forwarder
make clean && make
cp ./aiosu-forwarder.nro ../romfs/nro/aiosu-forwarder.nro
cd ../amssu-rcm
make clean && make
cp ./output/ams_rcm.bin ../romfs/payload/ams_rcm.bin
cp ./output/ams_rcm.bin ../Daybreak-cli/romfs/payload/ams_rcm.bin
cd ../Daybreak-cli
make clean && make
cp ./Daybreak-cli.nro ../romfs/nro/Daybreak-cli.nro
cd ..
make clean && make