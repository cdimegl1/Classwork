#!/bin/sh

SCRIPT_DIR=`dirname $0`
TESTS=${SCRIPT_DIR}/tests

stat=0

for f in $TESTS/*.scan.json
do
    input=`dirname $f`/`basename $f .scan.json`.defs
    echo -n "scan `basename $input` ... "
    $SCRIPT_DIR/do-test.sh scan $input
    if [ $? -ne 0 ]
    then
	stat=1
	echo FAIL
    else
	echo ok
    fi
done

if [ $# -eq 1 ]
then
    exit $stat
fi
    
for f in $TESTS/*.parse.json
do
    input=`dirname $f`/`basename $f .parse.json`.defs
    echo -n "parse `basename $input` ... "
    $SCRIPT_DIR/do-test.sh parse $input
    if [ $? -ne 0 ]
    then
	stat=1
	echo FAIL
    else
	echo ok
    fi
done

for f in $TESTS/*-err.defs
do
    echo -n "exit status `basename $f` ... "
    $SCRIPT_DIR/parse.sh $1 <$f 2>/dev/null
    if [ $? -eq 0 ]
    then
	echo "FAIL: success exit for `basename $f`; expected failure"
	stat=1
    else
	echo "ok: error exit as expected"
    fi
    
done

exit $stat
