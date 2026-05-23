#!/bin/bash

DIRS="psyche byte-code"
EXTS="h cc hh hpp cpp c"

find . -name "*~" -exec rm {} \;
find . -name ".*~" -exec rm {} \;

for dir in $DIRS
do
	for ext in $EXTS
	do
		find $dir -name "*.$ext" -exec clang-format -i {} \;
	done
done

find . -name *.py -exec black {} \;
