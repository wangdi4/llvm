#!/bin/sh

LIB_NAME=$1
shift
OBJS=$*

ar rcs $LIB_NAME $OBJS
