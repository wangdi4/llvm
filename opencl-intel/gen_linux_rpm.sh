#!/bin/sh

cd `dirname $0`
export CUR_DIR=`pwd`

perl -v | grep "5.10">/dev/null
if [ $? != 0 ]
	then
		echo "ActivePerl v5.10.* is not currently installed and it's required for rpms generations. Please install it from yast."
		exit 1
fi

cd $CUR_DIR/../BuildSystem

perl -c run.pl

if [ $? != 0]
	then
		echo "Please install XML Simple v2.18 from '\\ger\ec\proj\ha\ptl\MobileTV\Installations\XML-Simple-2.18' or from '\\nntavc101xwb1.ccr.corp.intel.com\AVC.QA_OpenCL_resources\OpenCL Tools\Linux\XML-Simple-2.18'"
		exit 1
fi

if [ $? != 0 ]
	then
		echo "Directory '$CUR_DIR/../BuildSystem' doesn't exist, please sync it from svn."
		exit 1
fi

if [ "$1" == "debug" ]
	then
		./build.sh -bt opencl11_create_rpm -p -napz -c -glp binaries_target[Debug]
	else
		./build.sh -bt opencl11_create_rpm -p -napz -c
fi

if [ $? != 0 ]
	then
		echo "Please see more details in '$CUR_DIR/../BuildSystem/logs/Summary.log'."
		exit 1
fi