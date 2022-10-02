#!/bin/sh

if [ $# -ne 2 ] || [ $1 != 'scan' -a $1 != 'parse' ]
then
   echo "usage: $0 scan|parse DEFS_FILE"
   exit 1
fi

PROG=./$1.sh

if [ ! -e $PROG ]
then
    echo "run from directory containing $PROG"
    exit 1
fi

if [ ! -e $2 ]
then
    echo "$2 not found"
    exit 1
fi

GOLD=`dirname $2`/`basename $2 .defs`.$1.json
if [ ! -e $GOLD ]
then
    echo "file $GOLD containing expected test output not found"
    exit 1
fi

OUTDIR=/tmp/$USER
mkdir -p $OUTDIR

OUT=$OUTDIR/`basename $2 .defs`.json
GOLD_OUT=$OUTDIR/`basename $GOLD`

$PROG < $2 | json_pp > $OUT
if [ $? -ne 0 ]
then
   echo "$PROG returned non-zero status"
   exit 1
fi

json_pp < $GOLD >$GOLD_OUT

if cmp $GOLD_OUT $OUT
then
    rm $GOLD_OUT $OUT
else
    echo "test failed: expected in $GOLD_OUT; result in $OUT"
    exit 1
fi
   

    

