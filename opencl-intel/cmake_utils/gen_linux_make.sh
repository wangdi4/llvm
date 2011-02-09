#! /bin/sh

#
# Build Unix Makefiles for OpenCL
#
# Usage:
#    gen_linux_make [+cnf] [-e] [debug|release] [gcc|intel] [working_dir]
#
#  +cnf           - include conformance tests into build - ONLY FOR INITIAL SETTINGS!
#                                use 'make edit_cache' to modify later!
#  -e             - create eclipse project
#  debug|release  - create debug or release Makefiles version. Default - release
#  gcc|intel      - use GCC or Intel compiler. Default - GCC.
#  working_dir    - working directory. Default - "build" dir in parallel to "src"
#
#  Makefiles are created in the directory working_dir/Debug|Release parallel to top level src directory
#  Compilation output during build is copied to the directory bin/Debug or bin/Release
#  parallel to the top level src directory
#

my_dir=`dirname $0`
top_dir=`dirname $my_dir/../../../`

cd $top_dir
top_dir=`pwd`
generator="Unix Makefiles"

incl_cnf=OFF
if test x$1 = x+cnf; then
	incl_cnf=ON
	shift
fi

if test x$1 = x-e; then
	generator="Eclipse CDT4 - Unix Makefiles"
	shift
fi

if test x$1 = xdebug; then
	target=Debug
else
	target=Release
fi

use_gcc=1
if test x$2 = xintel; then
	use_gcc=0
fi

working_dir=build
if test x$3 != x; then
	working_dir=$3
fi

top_work_dir=$working_dir/$target

mkdir -p $top_work_dir
cd $top_work_dir

conformance_list="test_api test_atomics test_basic test_buffers test_commonfns test_compiler computeinfo contractions test_conversions test_events test_geometrics test_gl test_half test_headers test_cl_h test_cl_platform_h test_cl_gl_h test_opencl_h test_cl_copy_images test_cl_get_info test_cl_read_write_images test_kernel_image_methods test_image_streams test_integer_ops bruteforce test_multiples test_profiling test_relationals test_select test_thread_dimensions test_vecalign test_vecstep" 

cmake -G "$generator" -D CMAKE_BUILD_TYPE=$target -D CONFORMANCE_LIST="$conformance_list" -D INCLUDE_CONFORMANCE_TESTS:BOOL=$incl_cnf $top_dir/src 

if test $? = 0; then
	echo .
	echo . Make files have been generated for the $target target
	echo . Please run make from the directory `pwd`
	echo .
fi
