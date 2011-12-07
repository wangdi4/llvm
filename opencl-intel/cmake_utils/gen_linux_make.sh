#!/bin/bash

function usage
{
cat <<- _EOT_
Build Unix Makefiles for OpenCL

Usage: $0 [+cnf] [--eclipse] [debug|release] [gcc|intel] [--icc_ver <ver>][-32] [--meego] [--help] <working_dir>

    +cnf           - include conformance tests into build - ONLY FOR INITIAL SETTINGS!
                               use 'make edit_cache' to modify later!

    --eclipse | -e      - create eclipse project

    --esrc              - create eclipse project + sources project (for SVN use)

    [debug,release]     - create debug or release Makefiles version. Default - release

    [gnu,intel]         - use GCC or Intel compiler. Default - gnu.

    +java                - build Java JARs

    --icc_ver           - ICC version (default 11.1)

    --meego | -m        - target Meego, not Linux

    -32 | --32          - build 32 bit version

    --atom              - cross-compile for ATOM CPU

    --help              - show this help message and exit.

    working_dir         - working directory. Default - "build" dir in parallel to "src"


Makefiles are created in the directory working_dir/$${target}$${compiler}$${trgtBit}bit parallel to top level src directory
where: target=Debug/Release ; trgtBit=32/64 ; compiler=GNU/Intel
Compilation output during build is copied to the directory bin/Debug or bin/Release
parallel to the top level src directory

_EOT_
}

# set default values:
generator="Unix Makefiles"
eclipse_extra_args=""
incl_cnf=OFF
target=Release
icc_ver=11.1
meego=OFF
incl_java=OFF

compiler=gnu
compiler_nice_name=GNU
use_intc=OFF

gen_dir=`dirname $0`
if echo ${gen_dir} | grep '^/' > /dev/null
then
	toolchain_dir="${gen_dir}"
else
	toolchain_dir="${PWD}/${gen_dir}"
fi

working_dir=${toolchain_dir}/../../build
src_dir=${toolchain_dir}/../../src

toolchain_file="${toolchain_dir}/Linux-GNU.cmake"
trgtOS=Linux
trgtBit=64
trgt32=OFF

trgt_cpu=x86

while [ "$1" != "" ]; do
    case $1 in
        +cnf )                  incl_cnf=ON
                                ;;
        --esrc )                eclipse_extra_args="${eclipse_extra_args} -D ECLIPSE_CDT4_GENERATE_SOURCE_PROJECT=ON"
                                generator="Eclipse CDT4 - Unix Makefiles"
                                eclipse_extra_args="${eclipse_extra_args} -D CMAKE_BUILD_TYPE=Debug"
                                ;;
        -e | --eclipse )        generator="Eclipse CDT4 - Unix Makefiles"
                                eclipse_extra_args="${eclipse_extra_args} -D CMAKE_BUILD_TYPE=Debug"
                                ;;
        debug )                 target=Debug
                                ;;
        release)                target=Release
                                ;;
        intel )                 compiler=intel
                                compiler_nice_name=Intel
                                use_intc=ON
                                toolchain_file="${toolchain_dir}/Linux-Intel.cmake"
                                ;;
        gnu )                   compiler=gnu
                                compiler_nice_name=GNU
                                use_intc=OFF
                                toolchain_file="${toolchain_dir}/Linux-GNU.cmake"
                                ;;
        -h | --help )           usage
                                exit 1
                                ;;
        --icc_ver )             shift
                                icc_ver=$1
                                ;;
        -m | --meego )          meego=ON
                                ;;
        -32 | --32 )            trgt32=ON
                                trgtBit=32
                                ;;
        -64 | --64 )            trgt32=OFF
                                trgtBit=64
                                ;;
        -a | --atom )           trgt_cpu=Atom
                                ;;
        +java )                 incl_java=ON
                                ;;
        * )                     working_dir=$1
                                ;;
    esac
    shift
done

if test $compiler = intel; then
  if test $trgt32 = ON; then
      ICC=`find /opt/intel/Compiler/$icc_ver -wholename *32/icc`
  else
      ICC=`find /opt/intel/Compiler/$icc_ver -wholename *64/icc`
  fi
  ICC_PATH=`dirname $ICC`
  PATH=$ICC_PATH:$PATH
fi


echo
echo
echo New PATH = $PATH
echo
echo

#echo ICC path = $ICC_PATH
#echo PATH = $PATH

CROSS_COMPILATION_OPTIONS=

if test $meego = ON; then
    echo
    echo CMake configured for Meego OS
    echo

    CROSS_COMPILATION_OPTIONS="${CROSS_COMPILATION_OPTIONS} -D OCL_MEEGO=ON"
fi


if test $trgt_cpu = Atom; then
    echo
    echo CMake configured for Atom CPU
    echo
fi

full_target_name=${compiler_nice_name}${trgtBit}${target}

build_directory=${working_dir}/build/${trgtOS}${trgtBit}/${target}

install_prefix=${working_dir}/install/${trgtOS}${trgtBit}/${target}

echo
echo Going to generate project at ${build_directory}
echo Going to install project at ${install_prefix}
echo
mkdir -p ${build_directory}
pushd ${build_directory}
echo
echo Starting to work at: `pwd -P`
echo

conformance_list="test_api test_atomics test_basic test_buffers test_commonfns test_compiler computeinfo contractions test_conversions test_events test_geometrics test_gl test_half test_headers test_cl_h test_cl_platform_h test_cl_gl_h test_opencl_h test_cl_copy_images test_cl_get_info test_cl_read_write_images test_kernel_image_methods test_image_streams test_integer_ops bruteforce test_multiples test_profiling test_relationals test_select test_thread_dimensions test_vecalign test_vecstep"

cmake -G "$generator" \
    ${eclipse_extra_args} \
    -D OCL_CMAKE_INCLUDE_DIRECTORIES="${toolchain_dir}" \
    -D OCL_TOOLCHAIN_FILE="${toolchain_file}" \
    -D CMAKE_BUILD_TYPE=$target \
    -D CMAKE_PROJECT_NAME=${full_target_name} \
    -D CMAKE_MODULE_PATH="${toolchain_dir}" \
    -D CONFORMANCE_LIST="$conformance_list" \
    -D INCLUDE_CONFORMANCE_TESTS:BOOL=$incl_cnf \
    -D OCL_BUILD32:BOOL=$trgt32 \
    -D TARGET_CPU=$trgt_cpu \
    -D INTEL_COMPILER=$use_intc \
    -D BUILD_JAVA:BOOL=$incl_java \
    -D CMAKE_INSTALL_PREFIX:PATH=${install_prefix} \
    ${CROSS_COMPILATION_OPTIONS} \
    ${src_dir}

# extra debug flags for cmake:
#     --debug-trycompile --debug-output --trace -D CMAKE_VERBOSE_MAKEFILE=true \

if test $? = 0; then
    echo .
    echo . Make files have been generated for the $target target
    echo . Please run make from the directory `pwd`
    echo .
fi

popd
