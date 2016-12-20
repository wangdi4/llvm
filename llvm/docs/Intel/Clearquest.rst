=================================
LLVM Specific Clearquest Policies
=================================

.. contents::
   :local:

The Intel LLVM development process documentation is currently under review. If
anything looks wrong to you, please contact `David Kreitzer
<mailto:david.l.kreitzer@intel.com>`_.

Introduction
============

The LLVM projects are governed by the same Clearquest usage rules that govern
other Intel Compiler projects. These rules are documented on the process pages.
However, there are a few details covered in this document that only apply to the
LLVM projects.

LLVM projects are identified in Clearquest by
"Found in Project = Compiler-LLVM". The Compiler-LLVM project applies to both
xmain and llorg.

Project Versions
================

Just as we have different project versions for the mainline and each product
branch of icc, we have different project versions for the mainline and each
product branch of xmain. As of this writing, there is a Compiler-LLVM 2016.0
project version for the 2016 MCU product branch. And there is a Compiler-LLVM
2017.0 project version for the xmain mainline.

Compiler-LLVM llvm.org is a special project version that represents open source
trunk. We ultimately do not have full control over whether a particular fix will
be committed to llvm.org, but we use Clearquest to capture our intent.

Use these project versions to
set the "Impacted Project Version(s)", "Targeted Project Version(s)", and
"Verified in Project Version(s)" fields.
