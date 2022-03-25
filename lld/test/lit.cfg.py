# -*- Python -*-

import os
import platform
import re
import subprocess
import locale

import lit.formats
import lit.util

from lit.llvm import llvm_config

# Configuration file for the 'lit' test runner.

# name: The name of this test suite.
config.name = 'lld'

# testFormat: The test format to use to interpret tests.
#
# For now we require '&&' between commands, until they get globally killed and the test runner updated.
config.test_format = lit.formats.ShTest(not llvm_config.use_lit_shell)

# suffixes: A list of file extensions to treat as test files.
config.suffixes = ['.ll', '.s', '.test', '.yaml', '.objtxt']

# excludes: A list of directories to exclude from the testsuite. The 'Inputs'
# subdirectories contain auxiliary inputs for various tests in their parent
# directories.
config.excludes = ['Inputs']
# INTEL_CUSTOMIZATION
# Exclude tests for disabled functionality.
config.excludes.extend(['darwin', 'mach-o', 'MachO', 'MinGW'])
# end INTEL_CUSTOMIZATION

# test_source_root: The root path where tests are located.
config.test_source_root = os.path.dirname(__file__)

config.test_exec_root = os.path.join(config.lld_obj_root, 'test')

llvm_config.use_default_substitutions()
llvm_config.use_lld()

tool_patterns = [
    'llc', 'llvm-as', 'llvm-mc', 'llvm-nm', 'llvm-objdump', 'llvm-otool', 'llvm-pdbutil',
    'llvm-dwarfdump', 'llvm-readelf', 'llvm-readobj', 'obj2yaml', 'yaml2obj',
    'opt', 'llvm-dis']
# INTEL_CUSTOMIZATION
tool_patterns.append('clang-cl')
# Note: 'clang' was added in the tools instead of 'clang++' because there is
# an issue parsing special symbols. The parsing process converts 'clang++'
# as 'clang+++++++'. This issue produces a missing command failure.
tool_patterns.append('clang')
# end INTEL_CUSTOMIZATION

llvm_config.add_tool_substitutions(tool_patterns)

# LLD tests tend to be flaky on NetBSD, so add some retries.
# We don't do this on other platforms because it's slower.
if platform.system() in ['NetBSD']:
    config.test_retry_attempts = 2

# When running under valgrind, we mangle '-vg' onto the end of the triple so we
# can check it with XFAIL and XTARGET.
if lit_config.useValgrind:
    config.target_triple += '-vg'

# Running on ELF based *nix
if platform.system() in ['FreeBSD', 'NetBSD', 'Linux']:
    config.available_features.add('system-linker-elf')

# Set if host-cxxabi's demangler can handle target's symbols.
if platform.system() not in ['Windows']:
    config.available_features.add('demangler')

# INTEL_CUSTOMIZATION
# Add 'intel_opencl' feature based on ICS_WSVARIANT value.
ics_wsvariant = os.environ.get("ICS_WSVARIANT")
if ics_wsvariant and ics_wsvariant.startswith('xmainocl'):
    config.available_features.add("intel_opencl")

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

intel_devirt_options = ''
intel_mllvm = ''
# INTEL_FEATURE_SW_DTRANS
# Special options that we want to pass to the test cases for Intel only

if config.include_intel_extra_opts:
  # Turn off multiversioning for whole program devirtualization
  intel_devirt_options = '-wholeprogramdevirt-multiversion=false'

  # Substitution for -mllvm
  intel_mllvm = '-mllvm'

# end INTEL_FEATURE_SW_DTRANS
config.substitutions.append(('%intel_devirt_options', intel_devirt_options))
config.substitutions.append(('%intel_mllvm', intel_mllvm))
# end INTEL_CUSTOMIZATION

llvm_config.feature_config(
    [('--targets-built', {'AArch64': 'aarch64',
                          'AMDGPU': 'amdgpu',
                          'ARM': 'arm',
                          'AVR': 'avr',
                          'Hexagon': 'hexagon',
                          'Mips': 'mips',
                          'MSP430': 'msp430',
                          'PowerPC': 'ppc',
                          'RISCV': 'riscv',
                          'Sparc': 'sparc',
                          'WebAssembly': 'wasm',
                          'X86': 'x86'})
     ])

# INTEL_CUSTOMIZATION
# Include the TC_WRAPPER_PATH environment variable if it is available
llvm_config.with_system_environment(['TC_WRAPPER_PATH'])
# end INTEL_CUSTOMIZATION

# Set a fake constant version so that we get consistent output.
config.environment['LLD_VERSION'] = 'LLD 1.0'

# LLD_IN_TEST determines how many times `main` is run inside each process, which
# lets us test that it's cleaning up after itself and resetting global state
# correctly (which is important for usage as a library).
run_lld_main_twice = lit_config.params.get('RUN_LLD_MAIN_TWICE', False)
if not run_lld_main_twice:
    config.environment['LLD_IN_TEST'] = '1'
else:
    config.environment['LLD_IN_TEST'] = '2'
    # Many ELF tests fail in this mode.
    config.excludes.append('ELF')
    # Some old Mach-O backend tests fail, and it's due for removal anyway.
    config.excludes.append('mach-o')
    # Some new Mach-O backend tests fail; give them a way to mark themselves
    # unsupported in this mode.
    config.available_features.add('main-run-twice')

# INTEL_CUSTOMIZATION
config.environment['INTEL_LLD_IN_TEST'] = '1'
# end INTEL_CUSTOMIZATION

# Indirectly check if the mt.exe Microsoft utility exists by searching for
# cvtres, which always accompanies it.  Alternatively, check if we can use
# libxml2 to merge manifests.
if (lit.util.which('cvtres', config.environment['PATH']) or
        config.have_libxml2):
    config.available_features.add('manifest_tool')

if config.have_libxar:
    config.available_features.add('xar')

if config.have_libxml2:
    config.available_features.add('libxml2')

if config.have_dia_sdk:
    config.available_features.add("diasdk")

if config.sizeof_void_p == 8:
    config.available_features.add("llvm-64-bits")

if config.has_plugins:
    config.available_features.add('plugins')

if config.build_examples:
    config.available_features.add('examples')

if config.linked_bye_extension:
    config.substitutions.append(('%loadbye', ''))
    config.substitutions.append(('%loadnewpmbye', ''))
else:
    config.substitutions.append(('%loadbye',
                                 '-load={}/Bye{}'.format(config.llvm_shlib_dir,
                                                         config.llvm_shlib_ext)))
    config.substitutions.append(('%loadnewpmbye',
                                 '-load-pass-plugin={}/Bye{}'
                                 .format(config.llvm_shlib_dir,
                                         config.llvm_shlib_ext)))

tar_executable = lit.util.which('tar', config.environment['PATH'])
if tar_executable:
    env = os.environ
    env['LANG'] = 'C'
    tar_version = subprocess.Popen(
        [tar_executable, '--version'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        env=env)
    sout, _ = tar_version.communicate()
    if 'GNU tar' in sout.decode():
        config.available_features.add('gnutar')

# ELF tests expect the default target for ld.lld to be ELF.
if config.ld_lld_default_mingw:
    config.excludes.append('ELF')
