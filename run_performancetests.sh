#!/bin/bash

#avoid locale problems
export LANG=

set -eu

for p in bc timelimit perf ; do
    if ! which $p >/dev/null ; then
	echo please install $p
	exit 1
    fi
done

exe=$(pwd)/shootout
cpuname=$(grep "model name" /proc/cpuinfo  |head -n1 |cut -f2 -d: |sed -e 's/^[ \t]*//' -e 's/[ \t]*$//')
compiler=$($exe --compiler)
results=res.txt
#cat /dev/null >$results
for prog in $($exe --list) ; do
    skipthisprogram=false
    for size in 25 26 27 28 29 30 31 ; do
	if $skipthisprogram ; then
	    continue;
	fi
	echo running $prog with size $size
	#echo -e -n "$cpuname\t$compiler\t$prog\t$size\t" >>$results
	true /usr/bin/time -f "$cpuname\t$compiler\t$prog\t$size\t%e" \
		      --output  $results --append \
		      timeout 10s $exe $prog $((2**$size))

	perf stat -x ';' -o tmp $exe $prog $((2**$size))
	es=$?
	if [ $es -ne 0 ] ; then
	    skipthisprogram=true
	    continue
	fi
	elapsedms=$(grep ";task-clock:u;" <tmp |cut -f1 -d';')
	elapsed=$(echo $elapsedms*0.001 |bc )
	cycles=$(grep ";cycles:u;" <tmp |cut -f1 -d';')
	instructions=$(grep ";instructions:u;" <tmp |cut -f1 -d';')
	branches=$(grep ";branches:u;" <tmp |cut -f1 -d';'|head -n1)
	branchmisses=$(grep ";branch-misses:u;" <tmp |cut -f1 -d';')
	echo -e "$cpuname\t$compiler\t$prog\t$size\t$elapsed\t$cycles\t$instructions\t$branches\t$branchmisses" >>$results
	if [ $(echo "$elapsed > 20" |bc) -eq 1 ] ; then
	    skipthisprogram=true
	    continue
	fi
    done
done

