#!/bin/sh
# Archiving of the objects to static library
# All variables in the form of CMake vars will be replaced by CMake:
#     ICC_MIC_AR - mic archiver path
# Invocation parameters:
#	  $1     - library name
#     others - objects to archive


LIBRARY_NAME=$1
shift
OBJECTS_LIST=$*

@ICC_MIC_AR@ rcs $LIBRARY_NAME $OBJECTS_LIST
