#!/bin/bash

PROGRAMME=./build/pcsa
GPROF=gprof
GPROF2DOT=gprof2dot

SCRIPT=$1
GMONOUT=${SCRIPT}.gmon
DOTOUT=${SCRIPT}.png

COLDSTART=false
if [ ! -e "~/mypy" ]
then
	COLDSTART=true
fi

if $COLDSTART; then
	python3 -m venv ~/mypy
fi
. ~/mypy/bin/activate
if $COLDSTART; then
	pip3 install gprof2dot
fi

rm -f gmon.out
$PROGRAMME $SCRIPT
$GPROF $PROGRAMME gmon.out > $GMONOUT
echo gmonout: $GMONOUT
cat $GMONOUT | $GPROF2DOT | dot -Tpng -o $DOTOUT
echo dotout: $DOTOUT

deactivate
