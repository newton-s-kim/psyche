#!/bin/bash

PROGRAMME=./build/sciche/psyche
GPROF=gprof
GPROF2DOT=gprof2dot

SCRIPT=$1
GMONOUT=${SCRIPT}.gmon
DOTOUT=${SCRIPT}.png

. ~/mypy/bin/activate

$PROGRAMME $SCRIPT
$GPROF $PROGRAMME gmon.out > $GMONOUT
echo gmonout: $GMONOUT
cat $GMONOUT | $GPROF2DOT | dot -Tpng -o $DOTOUT
echo dotout: $DOTOUT

deactivate
