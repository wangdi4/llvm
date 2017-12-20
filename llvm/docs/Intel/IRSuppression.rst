==========================
LLVM IR Output Suppression
==========================

.. contents::
   :local:

Introduction
============

This document describes the strategies used to suppress the output of LLVM IR
in Intel products based on the xmain branch and the reasoning behind the choices
that were made.  It covers the reasons that we are suppressing LLVM IR output,
the categories of threats we intend to protect against, the details of the
implementation and the rationale behind not choosing to implement additional
measures.

Goals and Objectives
====================

The overall objective behind suppressing LLVM IR output is to protect Intel's
intellectual property and maintain the competitive advantage provided by our
products.  We want to ensure that the products will not unnecessarily expose
information that can be used to discover the proprietary techniques and
algorithms used in our products, and we want to prevent our products from being
incorporated into tool chains used to support architectures other than those
for which the product is intended.


Types of Threats
================

The primary concern is exposing output from our products in any format that
could be used to incorporate our proprietary optimizations into a tool chain
that would produce executable code for an architecture that our product does
not intend to support.  Specifically, we want to avoid writing LLVM IR in any
format that could be consumed by third party tools after we have applied
proprietary optimizations and analyses.  Because the link-time optimization
feature (LTO) depends on LLVM IR bitcode files as input we cannot simply remove
all facilities for writing LLVM IR, but we can avoid running our most sensitive
optimizations before bitcode files are written.

A secondary concern is exposing intermediate information that could be used to
understand the techniques and algorithms we are using in our optimizations.
This includes phase-by-phase changes to the IR, Intel-specific data such as HIR,
and phase ordering information.

Implementation
==============

We are taking the following steps to protect against the threats described
above:

 - Schedule the passes we wish to protect so that they are never run prior to
   LTO when LTO is being used
 - Remove support for command line options that cause IR to be written (except
   for LTO) from release builds.
 - Exclude code that prints pass manager debug information from release builds
 - Exclude code that prints HIR and VPO debug information from release builds
 - Exclude code that prints LLVM IR in text format from release builds
 - Add an "Intel proprietary" module flag that will be set when sensitive
   optimizations have been run and act as a sentinel against attempts to write
   the IR to a file when the flag is set in release builds.

These changes achieve our IP protection goals as far as possible by removing
the facilities from our product that would expose us to threats.  The intention
is that there will be no means directly exposed through the user interface
(command line) of our product to trigger the emission of IR that has been
optimized using any of the passes that we wish to protect.  These changes are
also intended to make it difficult to modify or manipulate our products in a
way that would cause them to expose proprietary information.

The purpose of the "Intel proprietary" module flag is to provide a mechanism
that will alert us during product development and testing if we have
accidentally scheduled proprietary passes in a way that enables these passes to
be run before the IR is written to a bitcode file for LTO.  While this flag
will also serve as a minor obstacle to hackers who might try to modify our
executables to write bitcode files at times that we do not intend, this is not
the true purpose of the flag.  We recognize that it would be easily defeated by
hackers in that scenario.

The INTEL_PRODUCT_RELEASE preprocessor macro is used to cause conditional
compilation of code that should only be used in release builds (such as the
code that checks the "Intel proprietary" module flag) and to exclude code
that should not be present in release builds (such as the code that writes
LLVM IR debug output).  The ICS build system will define the
INTEL_PRODUCT_RELEASE preprocessor macro for "release" build types, but not
for "prod" or "debug" build types or other variants thereof.

Limitations imposed
===================

The introduction of the "Intel proprietary" module flag places restrictions on
when the passes that set this flag can be run.  These passes cannot
be run in any phase that may result in LLVM bitcode being written to a file.
For traditional compilers this means that these passes cannot be scheduled
during the normal optimization phase when LTO is being used.  For other
products, such as OpenCL-based compiler products, other scenarios would need
to be restricted and in the worst case passes that set the "Intel proprietary"
module flag would need to be excluded entirely.

Limiting which passes can be scheduled at various phases presents a potential
barrier to ideal optimization.  The trade-offs between the risk of exposing
the product to unintended uses and the limitations on the performance of
optimized code must be considered on a case-by-case basis to determine if
the performance limitations justify a change to the implementation.

Remaining vulnerabilities
=========================

Because LTO requires intermediate bitcode files it is possible for these files
to be used in a toolchain that targets architectures that we do not intend to
support.  Moving all Intel-specific optimizations and analyses into the link
phase would compromise our ability to optimize code.  Our most sensitive
optimizations are deferred to the link phase when LTO is used, but there will
still be significant Intel-specific passes run prior to writing the intermediate
bitcode file, including early inlining and enhanced alias analysis.

Although the command line options and internal facilities to write LLVM IR at
intermediate phases during compilation are being removed, it is still possible
for a skilled developer to access this information in various ways.  For
instance, because the LLVM data structures we use are publicly documented, a
person running our compiler under a debugger could write a debugger extension
to print these data structures or even have the debugger call the function to
emit a bitcode file once the pointers to the data were found.

Finally, the code which uses the "Intel proprietary" module flag to disable
bitcode output is extremely trivial.  A person could easily modify our
product binaries to skip this check.  This is judged to be acceptable because
the pass scheduling prevents us from even reaching this check after sensitive
optimizations are applied.  It is still theoretically possible for an
advanced hacker to somehow change the routine scheduling to expose these
optimizations.

The above vulnerabilities have been judged to be acceptable risks when weighed
against the cost of implementing protection against them.

Rejected Alternatives
=====================

During design discussions, several alternative strategies were considered and
dismissed.  They are recorded here in order to document the reasoning behind
the decision not to use these strategies.

One such option considered was re-ordering the fields in common LLVM data
structures in Intel's internal code repositories.  This would effectively
prevent binary compatibility of any bitcode files we produced with any
non-Intel tools produced based on LLVM.  It would also make it more difficult
to expose our intermediate IR using debugger extensions.

We decided against re-ordering the data structures because the additional
maintenance burden (for example, due to merge conflicts between public
sources and the Intel repository) does not justify the benefit of this
approach.  Although re-ordering data fields would eliminate simple binary
compatibility, a determined programmer could discover the change and
render it useless with a simple translation layer.

Another possibility consider was to encrypt the bitcode files produced by
our products.  We considered everything from applying a simple bitmask
transformation to more extensive levels of true encryption.  We decided
against encypting the bitcode files because simple levels of encryption
would be too easily defeated, and the cost of implementing more secure
encryption was judged to outweigh the benefits.

