#! /bin/bash

inputfile=$1
outputfile=$2

convert \
	-adaptive-blur 5 \
	-fuzz 5000 \
	-fill "rgb(130,51,67)" \
	-opaque "rgb(130,51,67)" \
	-opaque "rgb(146,114,117)" \
	-opaque "rgb(180,101,114)" \
	-fill "rgb(27,120,115)" \
	-opaque "rgb(27,120,115)" \
	-opaque "rgb(20,95,85)" \
	-opaque "rgb(32,105,95)" \
	-fill "rgb(40,40,123)" \
	-opaque "rgb(16,47,88)" \
	-opaque "rgb(112,137,168)" \
	-opaque "rgb(85,105,132)" \
	-opaque "rgb(37,73,123)" \
	-fill "#000000" \
	-opaque "rgb(20,20,20)" \
	-opaque "rgb(40,40,40)" \
	-opaque "rgb(60,60,60)" \
	-opaque "rgb(80,80,80)" \
	-opaque "rgb(100,100,100)" \
	-fill "#ffffff" \
	-opaque "rgb(150,150,150)" \
	-opaque "rgb(155,155,155)" \
	-opaque "rgb(160,160,160)" \
	-opaque "rgb(165,165,165)" \
	-opaque "rgb(170,170,170)" \
	-opaque "rgb(175,175,175)" \
	-opaque "rgb(180,180,180)" \
	-gamma 2 \
	-posterize 8 \
	-modulate 100,190,100 \
	-fill "rgb(219,79,192)" \
	-opaque "rgb(205,123,135)" \
	-opaque "rgb(219,79,192)" \
	$inputfile $outputfile
