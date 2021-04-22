#!/bin/sh

for f in *.raw
do
	sox -r 8000 -c 1 -w -s ${f} `basename ${f} raw`wav
done
