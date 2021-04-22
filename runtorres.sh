#!/bin/sh
# This script will run 'test_torres' on all .raw files, generating
# one folder with five .dat files, for each .raw file
#

counter=0
for infile in ./torres/raw/*
do
	echo "Processando arquivo: " ${infile}

	rm -f input.raw output.raw again.dat fgain.dat aind.dat find.dat lsf.dat lsf_index.dat again_ind.dat fgain_ind.dat
	cp ${infile} ./difone.raw

	./test_torres difone.raw > /dev/null

	base=`basename ${infile} '.raw'`
	mkdir ./torres/dat/${base}
	cp again.dat fgain.dat aind.dat find.dat lsf.dat lsf_index.dat again_ind.dat fgain_ind.dat ./torres/dat/${base}

	rm -f input.raw output.raw again.dat fgain.dat aind.dat find.dat lsf.dat lsf_index.dat again_ind.dat fgain_ind.dat
	

	counter=$((${counter} + 1))
	echo ""
done

echo "Numero de arquivos processados: " ${counter}
