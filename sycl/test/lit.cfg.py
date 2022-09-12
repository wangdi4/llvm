# -*- Python -*-

import os
import platform
import re
import subprocess
import tempfile
from distutils.spawn import find_executable

import lit.formats
import lit.util

from lit.llvm import llvm_config

# Configuration file for the 'lit' test runner.

# name: The name of this test suite.
config.name = 'SYCL'

# testFormat: The test format to use to interpret tests.
#
# For now we require '&&' between commands, until they get globally killed and
# the test runner updated.
config.test_format = lit.formats.ShTest()

# suffixes: A list of file extensions to treat as test files.
config.suffixes = ['.c', '.cpp', '.dump'] #add .spv. Currently not clear what to do with those

# feature tests are considered not so lightweight, so, they are excluded by default
config.excludes = ['Inputs', 'feature-tests']

# test_source_root: The root path where tests are located.
config.test_source_root = os.path.dirname(__file__)

# test_exec_root: The root path where tests should be run.
config.test_exec_root = os.path.join(config.sycl_obj_root, 'test')

# INTEL_CUSTOMIZATION
def getAdditionalFlags():
    flags = config.sycl_clang_extra_flags.split(' ')
    # Propagate --gcc-toolchain if we are overriding system installed gcc.
    if 'ICS_GCCBIN' in os.environ:
        flags += ['--gcc-toolchain='
            + os.path.normpath(os.path.join(os.environ['ICS_GCCBIN'], ".." ) ) ]
    # Add flags according to used host device backend
    backend = os.getenv('DPCPP_HOST_BACKEND', 'serial').lower()
    if backend is not None:
        if backend == 'openmp':
            flags += ['-DDPCPP_HOST_DEVICE_OPENMP=1', '-fopenmp']
        elif backend == 'openmp-perf':
            flags += ['-DDPCPP_HOST_DEVICE_PERF_NATIVE=1', '-fiopenmp', \
                      '-O3']
        elif backend == 'serial':
            pass
        else:
            print("Unknown host backend: " + backend)

    return flags
# end INTEL_CUSTOMIZATION

# Propagate some variables from the host environment.
llvm_config.with_system_environment(['PATH', 'OCL_ICD_FILENAMES', 'SYCL_DEVICE_ALLOWLIST', 'SYCL_CONFIG_FILE_NAME'])
llvm_config.with_system_environment(['TC_WRAPPER_PATH']) # INTEL_CUSTOMIZATION

# Propagate extra environment variables
if config.extra_environment:
    lit_config.note("Extra environment variables")
    for env_pair in config.extra_environment.split(','):
        [var,val]=env_pair.split("=")
        if val:
           llvm_config.with_environment(var,val)
           lit_config.note("\t"+var+"="+val)
        else:
           lit_config.note("\tUnset "+var)
           llvm_config.with_environment(var,"")

# Configure LD_LIBRARY_PATH or corresponding os-specific alternatives
# Add 'libcxx' feature to filter out all SYCL abi tests when SYCL runtime
# is built with llvm libcxx. This feature is added for Linux only since MSVC
# CL compiler doesn't support to use llvm libcxx instead of MSVC STL.
if platform.system() == "Linux":
    config.available_features.add('linux')
    if config.sycl_use_libcxx == "ON":
        config.available_features.add('libcxx')
    llvm_config.with_system_environment('LD_LIBRARY_PATH')
    llvm_config.with_environment('LD_LIBRARY_PATH', config.sycl_libs_dir, append_path=True)

elif platform.system() == "Windows":
    config.available_features.add('windows')
    llvm_config.with_system_environment('LIB')
    llvm_config.with_environment('LIB', config.sycl_libs_dir, append_path=True)

elif platform.system() == "Darwin":
    # FIXME: surely there is a more elegant way to instantiate the Xcode directories.
    llvm_config.with_system_environment('CPATH')
    llvm_config.with_environment('CPATH', "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/include/c++/v1", append_path=True)
    llvm_config.with_environment('CPATH', "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk/usr/include/", append_path=True)
    llvm_config.with_environment('DYLD_LIBRARY_PATH', config.sycl_libs_dir)

llvm_config.with_environment('PATH', config.sycl_tools_dir, append_path=True)

# INTEL_CUSTOMIZATION
# Tests assume that the host device is always available
llvm_config.with_environment('SYCL_ENABLE_HOST_DEVICE', "1")
# end INTEL_CUSTOMIZATION

config.substitutions.append( ('%threads_lib', config.sycl_threads_lib) )
config.substitutions.append( ('%sycl_libs_dir',  config.sycl_libs_dir ) )
config.substitutions.append( ('%sycl_include',  config.sycl_include ) )
config.substitutions.append( ('%sycl_source_dir', config.sycl_source_dir) )
config.substitutions.append( ('%opencl_libs_dir',  config.opencl_libs_dir) )
config.substitutions.append( ('%opencl_include_dir',  config.opencl_include_dir) )
config.substitutions.append( ('%cuda_toolkit_include',  config.cuda_toolkit_include) )
config.substitutions.append( ('%sycl_tools_src_dir',  config.sycl_tools_src_dir ) )
config.substitutions.append( ('%llvm_build_lib_dir',  config.llvm_build_lib_dir ) )
config.substitutions.append( ('%llvm_build_bin_dir',  config.llvm_build_bin_dir ) )

# INTEL_CUSTOMIZATION
llvm_config.add_intel_features()
# end INTEL_CUSTOMIZATION

config.substitutions.append( ('%sycl_include',  config.sycl_include ) )
config.substitutions.append( ('%opencl_libs_dir',  config.opencl_libs_dir) )
config.substitutions.append( ('%fsycl-host-only', '-std=c++17 -Xclang -fsycl-is-host -isystem %s -isystem %s -isystem %s' % (config.sycl_include, config.opencl_include_dir, config.sycl_include + '/sycl/') ) )

llvm_config.add_tool_substitutions(['llvm-spirv'], [config.sycl_tools_dir])

triple=lit_config.params.get('SYCL_TRIPLE', 'spir64-unknown-unknown')
lit_config.note("Triple: {}".format(triple))
config.substitutions.append( ('%sycl_triple',  triple ) )
if triple == 'nvptx64-nvidia-cuda-sycldevice':
    config.available_features.add('cuda')

# INTEL_CUSTOMIZATION
additional_flags = getAdditionalFlags() + config.sycl_clang_extra_flags.split(' ')
# end INTEL_CUSTOMIZATION

if config.cuda_be == "ON":
    config.available_features.add('cuda_be')

if config.hip_be == "ON":
    config.available_features.add('hip_be')

if config.esimd_emulator_be == "ON":
    config.available_features.add('esimd_emulator_be')

if triple == 'nvptx64-nvidia-cuda':
    config.available_features.add('cuda')

if triple == 'amdgcn-amd-amdhsa':
    config.available_features.add('hip_amd')
    # For AMD the specific GPU has to be specified with --offload-arch
    if not any([f.startswith('--offload-arch') for f in additional_flags]):
        # If the offload arch wasn't specified in SYCL_CLANG_EXTRA_FLAGS,
        # hardcode it to gfx906, this is fine because only compiler tests
        additional_flags += ['-Xsycl-target-backend=amdgcn-amd-amdhsa',
                            '--offload-arch=gfx906']

llvm_config.use_clang(additional_flags=additional_flags)

# INTEL_CUSTOMIZATION
# Needed for disable some test in case of use of particular linker
# Check if the default linker is set in the ICS options
ics_setopts = os.environ.get("ICS_SETOPTS")
if ics_setopts:
    # The ICS options command is case insensitive. The string stored
    # in the environment variable could have any letter casing.
    ics_setopts = ics_setopts.lower()
    # Check if lld is the default linker
    if ics_setopts == 'ld=lld':
        config.available_features.add('default_linker_lld')
    # Check if gold is the default linker
    elif ics_setopts == 'ld=gold':
        config.available_features.add('default_linker_gold')
    # Check if bfd is the default linker
    elif ics_setopts == 'ld=bfd':
        config.available_features.add('default_linker_bfd')
# end INTEL_CUSTOMIZATION

# Set timeout for test = 10 mins
try:
    import psutil
    lit_config.maxIndividualTestTime = 600
except ImportError:
    pass
