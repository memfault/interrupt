#!/bin/sh

RENODE_EXE_PATH=~/code/renode/output/bin/Release/Renode.exe

mono64 $RENODE_EXE_PATH renode-config.resc
