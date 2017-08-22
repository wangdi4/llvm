=================================
Open Source Development Processes
=================================

.. contents::
   :local:

The Intel LLVM development process documentation is currently under review. If
anything looks wrong to you, please contact `David Kreitzer
<mailto:david.l.kreitzer@intel.com>`_.

Introduction
============

Open source development is governed primarily by the community. The community
development policies are documented :doc:`here <../DeveloperPolicy>`. This
document describes our processes for code development in llvm.org over and above
what the community requires. There is a separate
:doc:`document <XmainProcesses>` describing our processes for development on
xmain, the ICL-specific LLVM/clang development branch.

.. warning:: Before starting any piece of llorg development work, you must
             obtain approval to open source the work by following the process
             described in our
             :doc:`Open Source Guidelines <OpenSourceGuidelines>`.

.. _ipscan:

Developing Using llorg
======================

llorg is an ics project that uses an SC mirror of the llvm.org sources as the
underlying repository. The repository is updated hourly, so it is nearly in sync
with llvm.org.

It is highly recommended but not required that you do your open source
development using llorg. The biggest advantage of llorg is that it provides a
uniform development environment across ICL. It also enables you to use other ICL
tools like alloy that do not work on manually constructed llvm.org workspaces.

IP Leak Testing
===============

Before sharing code externally, either by publishing changes in Phabricator or
by committing changes to llvm.org, you are required to run the ipscan tool to
test for IP leaks. Our team routinely works with Intel Restricted Secret
technology, and the ipscan tool provides a safeguard against accidentally
leaking this technology publicly, which could cause material harm to Intel.

Typical usage of the ipscan tool looks like this.

.. code-block:: bash

  ipscan

.. code-block:: bash

  Finding files to scan ...

  File(s) to be scanned:

   ipldt_results/scan_files/llvm/lib/Target/X86/X86ISelLowering.cpp.diff

  Doing scan ...

  Results are in: /export/iusers/dlkreitz/xmain_ws/ipldt_results

      == No problems detected in scanned files ==

  Done

.. note:: What is the process for getting approval for false positive failures
          reported by the ipscan tool?

Code Reviews
============

Prior to submitting your changes for external code review, make sure you have
run the ipscan tool as documented :ref:`here <ipscan>`. Also consider conducting
an internal code review. We do not currently have specific internal code review
requirements, so use your best judgment. It is prudent to at least conduct a
design level review for all but the most trivial changes. If you do choose to
conduct an internal code review, `Code Collaborator
<https://ir-codecollab.intel.com/ui>`_ is the tool to use.

External code reviews should follow the community Phabricator code review
procedures documented :doc:`here <../Phabricator>`.

Testing Procedures
==================

Read the Quality section of the
:doc:`LLVM Developer Policy <../DeveloperPolicy>` document to understand
the community's expectations regarding pre-commit testing and how to handle
problems discovered post-commit.

We don't have any specific testing requirements beyond what the community
requires, but that might change in the future. In the meantime, it is strongly
recommended that you run the following level of alloy testing at a minimum for
all but the most trivial changes.

.. code-block:: bash

    alloy run -file llorg_checkin -file zperf_checkin_llorg -ref_comp ws -notify

This will run LIT testing and some basic Intel Compiler test suites on various
platforms. And it will run some performance regression testing. Note that it is
only possible to run this alloy testing on an llorg ics workspace. Running alloy
on a manually constructed llvm.org workspace is not supported.

The community does not generally allow you to knowingly introduce stability
regressions. But occasionally, tests in our internal suites will regress due
to test case errors exposed by your changes. It is acceptable to submit a
bug report against the test suite and proceed with the commit.

Performance regressions may be allowed in some cases. It is impossible to
write a set of rules that covers all scenarios, but here are a few reasons why
a performance regression may be allowed. If in doubt, contact the `performance
architect <mailto:zia.ansari@intel.com>`_.

#. The regression is the result of a necessary correctness fix.

#. The magnitude of the regression is small and offset by larger gains.

#. The regression is caused by a second order effect of an otherwise good code
   generation difference. Code alignment differences are a common culprit.

Commit Procedures
=================

Read the Commit messages section of the
:doc:`LLVM Developer Policy <../DeveloperPolicy>` document to understand the
community guidelines for the format of commit messages. Please follow them.

There are no specific rules on how to do the actual commit, but ics provides
a feature for committing to llvm.org directly from an llorg workspace. That is
a convenience that enables you to develop, test, and commit from a single
workspace.

Any expected stability regressions should be reported to `LLVM QA
<mailto:ICL_LLVM_QA@intel.com>`_.

Any expected performance regressions should be reported to `LLVM PTA
<mailto:ICL_LLVM_Performance@intel.com>`_.
