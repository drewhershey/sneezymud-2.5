#!/bin/bash

ulimit -n 256

DIR=/mud/home/sneezy/sun
LIB=$DIR/lib
CRASH=$DIR/crash
TINYNEXT=$LIB/tinyworld.next
TINYOLD=$LIB/tinyworld.old
AREAS=$LIB/areas

DATE=$(date "+%m%d%y.%H%M")
theLine=$(ps -x | egrep 'dmserver 7900|dbx |mv ' | grep -v grep | wc -l)

if [ "$theLine" -eq 0 ]; then
    if [ -f "$DIR/dmserver.next" ]; then 
        mv "$DIR/dmserver" "$DIR/dmserver.old"
        mv "$DIR/dmserver.next" "$DIR/dmserver"
    fi

    if [ -f "$LIB/core" ]; then
        mv "$LIB/core" "$CRASH/core.$DATE"
    fi

    rm -f log

    if [ -f "$DIR/log.err" ]; then
        mv "$DIR/log.err" "$CRASH/log$DATE"
    fi

    for FILE in "$TINYNEXT"/*; do
        BASE=$(basename "$FILE")
        mv "$LIB/$BASE" "$TINYOLD/$BASE"
        mv "$FILE" "$LIB/$BASE"
    done

    for FILE in "${AREAS}.next"/*; do
        BASE=$(basename "$FILE")
        mv "$AREAS/$BASE" "$AREAS/${BASE}.bkp"
        mv "$FILE" "$AREAS/$BASE"
    done

    ccn_7900 &

    #if [ -f LogWatcher.pid ]; then
    #    kill $(cat LogWatcher.pid)
    #fi
    #./LogWatcher &
fi

sleep 15

exec rebooter
