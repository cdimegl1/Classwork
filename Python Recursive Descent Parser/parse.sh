#!/bin/sh

SCRIPT_DIR=`dirname $0`
LANG_DIR=${XLANG:-js}

#brittle assumption: LANG_DIR contains only single executable 
PROG=`find ${SCRIPT_DIR}/${LANG_DIR} -type f -executable`

${PROG} parse
