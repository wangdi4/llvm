======================================================
Intel product IR Versioning and Backward Compatibility
======================================================

.. contents::
   :local:

Introduction
============

This document describes the design for versioning of LLVM bitcode files
produced by Intel compiler products and how that version information will
be used to provide backward compatibility if the bitcode format changes in
a way that can't be handled by the LLVM auto-upgrade process.


Goals and Objectives
====================

LLVM bitcode modules may be embedded in fat binaries for user applications
that have been created using the Intel DPC++ compiler or the user may have
created object files using the Intel compiler in LTO mode that are stored as
LLVM bitcode files. We must support using such applications and object files
produced with older (but recent) versions of Intel compilers with newer
versions of the Intel compiler and new DPC++ runtimes shipped with the compiler.
Therefore, we need a mechanism that will allow us to read bitcode files that
were produced by recent Intel compiler releases even if the bitcode
representation changes in a way that the default bitcode reader cannot handle.


Bitcode Identification
======================

Since LLVM 4.0, LLVM bitcode files have contained an identification block that
is intended to allow the bitcode reader to detect incompatible bitcode files.
This is represented as a block within the bitcode with a block ID of
IDENTIFICATION_BLOCK_ID. The block contains two records -- a producer string
(IDENTIFICATION_CODE_STRING) and an epoch number (IDENTIFICATION_CODE_EPOCH).

The open source LLVM implementation uses a producer string of the form
'LLVMX.Y.Zgit' where 'X.Y.Z' is the LLVM version number. The epoch has been
zero since this feature was introduced. As long as bitcode files produced by
LLVM 4.0 and newer can be auto-upgraded, the epoch will remain zero. When a
change is introduced that breaks backward compatibility, the epoch will be
incremented.

The open source implementation reports an error and fails to read any bitcode
file with an epoch that does not match the current epoch. The producer string
is only used for error reporting, though it can be read by the llvm-bcanalyzer
tool.

For general information about the LLVM bitcode format, see
`LLVM Bitcode File Format <https://llvm.org/docs/BitCodeFormat.html>`_.

The initial Intel(R) oneAPI DPC++ compiler gold release used the default LLVM
identification block and so bitcode produced by that release will be
indistinguishable from open source LLVM bitcode files.

Starting with oneAPI update 1 (2021.2.0), Intel compiler products will use
a producer string based on a variable (DPCPP_BITCODE_PRODUCER_STR) defined in
the llvm/CMakeLists.txt file. This string will closely reflect the product
name string (DPCPP_PRODUCT_NAME), but it must be reformatted to meet the
constraints of the char6 format used by LLVM bitcode files. See
`6-bit character encoding <https://llvm.org/docs/BitCodeFormat.html#bit-characters>`_.

The epoch number used by Intel compilers will be defined using the current
LLVM epoch number but with an Intel-specific "sub-epoch" defined in the high
byte. This is an arbitrary convention that allows us to track the Intel epoch
and the LLVM epoch separately.

There is no way to guarantee that other vendors will not use the same epoch
we have chosen. This is unavoidable. The producer string will allow useful
error messages to be produced, but we will not support reading bitcode files
produced by other vendors.

Reading Bitcode Files from the Current Epoch
============================================

Intel compilers will support reading bitcode files that are identified using
either the current Intel epoch or the current LLVM epoch. Some tests from
the LLVM open source repository use binary bitcode files, so we must allow
the current LLVM epoch. However, we may choose not to officially support
reading bitcode files produced by non-Intel compilers.

Bitcode files identified with the current Intel epoch will be handled by the
standard BitcodeReader.

Reading Bitcode Files from the Earlier Epochs
=============================================

When it becomes neccessary to break compatibility with older bitcode formats
in a way that requires updating the Intel epoch, we will also create a
specialized bitcode reader class (derived from legacy source code) that is
capable of reading bitcode files from older epochs and translating them to
the most current in-memory IR representation. The details of this Handling
will need to be determined when such incompatibility is introduced.

Error Handling of Bitcode Files from Newer Epochs
=================================================

If the bitcode reader is asked to read a bitcode file which contains an
epoch number that cannot be identified as either the current LLVM epoch or
a known and supported Intel epoch, the bitcode reader will report an error,
showing the user the producer string and epoch number for the bitcode file
as well as the current Intel producer string and epoch number.

Test Strategy
=============
TBD
