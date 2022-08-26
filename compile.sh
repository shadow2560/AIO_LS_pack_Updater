cd aiosu-forwarder
make clean && make
cp ./aiosu-forwarder.nro ../romfs/nro/aiosu-forwarder.nro
cd ../amssu-rcm
make clean && make
cp ./output/ams_rcm.bin ../romfs/payload/ams_rcm.bin
cd ..
make clean && make