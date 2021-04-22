#!/bin/sh

for f in `find . -name *.raw`
do
	sox -r 8000 -c 1 -w -s ${f} `dirname ${f}`/`basename ${f} raw`wav
done
