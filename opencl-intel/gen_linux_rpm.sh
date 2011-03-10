#!/bin/sh

@echo off

cd `dirname $0`
export CUR_DIR=`pwd`

perl -v | grep "5.10">nul
if [ $? -neq 0 ]
	then
		echo ActivePerl v5.10.1 is not currently installed and it's required for msi's generations. Please install it from "\\ger\ec\proj\ha\ptl\MobileTV\Installations\ActivePerl" or from "\\nntavc101xwb1.ccr.corp.intel.com\AVC.QA_OpenCL_resources\OpenCL Tools\ActivePerl". 
		exit 1
fi

cd $CUR_DIR%/../BuildSystem
if [ $? -neq 0 ]
	then
		echo Directory "$CUR_DIR/../BuildSystem" doesn't exist, please sync it from svn.
		exit 1
fi

if [ "$1" -eq "debug" ]
	then
		call ./build.sh -bt opencl11_create_rpm -p -napz -c -glp binaries_target[Debug]
	else
		call ./build.sh -bt opencl11_create_rpm -p -napz -c
fi

if [ $? -neq 0 ]
	then
		echo Please see more details in "$CUR_DIR/../BuildSystem/logs/Summary.log".
		exit 1
fi