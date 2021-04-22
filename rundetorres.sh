#!/bin/sh
# This script will run 'test_detorres' on all .raw files, generating
# one folder with five .dat files, for each .raw file
#

counter=0
rm -f torres.raw
touch torres.raw

echo "Processando difone: 0103"
cp ./torres/dat/0103/{again.dat,fgain.dat,aind.dat,find.dat,lsf.dat} .
./test_detorres difone.raw 49 > /dev/null
cat difone.raw >> torres.raw

echo "Processando difone: 0334"
cp ./torres/dat/0334/{again.dat,fgain.dat,aind.dat,find.dat,lsf.dat} .
./test_detorres difone.raw 86 > /dev/null
cat difone.raw >> torres.raw

echo "Processando difone: 3405"
cp ./torres/dat/3405/{again.dat,fgain.dat,aind.dat,find.dat,lsf.dat} .
./test_detorres difone.raw 46 > /dev/null
cat difone.raw >> torres.raw

echo "Processando difone: 0522"
cp ./torres/dat/0522/{again.dat,fgain.dat,aind.dat,find.dat,lsf.dat} .
./test_detorres difone.raw 37 > /dev/null
cat difone.raw >> torres.raw

echo "Processando difone: 2225"
cp ./torres/dat/2225/{again.dat,fgain.dat,aind.dat,find.dat,lsf.dat} .
./test_detorres difone.raw 88 > /dev/null
cat difone.raw >> torres.raw

echo "Processando difone: 2504"
cp ./torres/dat/2504/{again.dat,fgain.dat,aind.dat,find.dat,lsf.dat} .
./test_detorres difone.raw 127 > /dev/null
cat difone.raw >> torres.raw

echo "Processando difone: 0428"
cp ./torres/dat/0428/{again.dat,fgain.dat,aind.dat,find.dat,lsf.dat} .
./test_detorres difone.raw 13 > /dev/null
cat difone.raw >> torres.raw

echo "Processando difone: 2805"
cp ./torres/dat/2805/{again.dat,fgain.dat,aind.dat,find.dat,lsf.dat} .
./test_detorres difone.raw 102 > /dev/null
cat difone.raw >> torres.raw

echo "Processando difone: 0529"
cp ./torres/dat/0529/{again.dat,fgain.dat,aind.dat,find.dat,lsf.dat} .
./test_detorres difone.raw 105 > /dev/null
cat difone.raw >> torres.raw

echo "Processando difone: 2912"
cp ./torres/dat/2912/{again.dat,fgain.dat,aind.dat,find.dat,lsf.dat} .
./test_detorres difone.raw 120 > /dev/null
cat difone.raw >> torres.raw

echo "Processando difone: 1201"
cp ./torres/dat/1201/{again.dat,fgain.dat,aind.dat,find.dat,lsf.dat} .
./test_detorres difone.raw 33 > /dev/null
cat difone.raw >> torres.raw

rm -f again.dat fgain.dat aind.dat find.dat lsf.dat difone.raw
sox -w -s -r 8000 -c 1 torres.raw torres.wav
rm -f torres.raw
