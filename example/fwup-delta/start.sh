#!/bin/sh

RENODE_EXE_PATH=/Applications/Renode.app/Contents/MacOS/bin/Renode.exe

mono64 $RENODE_EXE_PATH renode-config.resc --port 4444 --disable-xwt
