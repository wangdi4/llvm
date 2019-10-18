===========================
Xmain Development Processes
===========================

.. contents::
   :local:

Introduction
============

This document describes our processes for code development in xmain. There is a
separate :doc:`document <OpenSourceProcesses>` describing our processes for open
source LLVM/clang code development.

The xmain project stays in sync with open source via our
:doc:`pulldown process <PulldownProcess>`.
xmain typically stays within several months of open source trunk. Consequently,
changes that are committed directly to open source usually do **not** need to
be ported to xmain by the developer. The exception is when the changes are
urgently needed in xmain to meet a deadline or to unblock other xmain
development. In these exceptional cases, developers should follow all the
xmain development processes, including the use of
:ref:`Intel-specific code markups <xmain-markups>`.

Developing on a Component Branch
================================

Individual components are free to define their own processes for committing to
the component branch. Those processes may be different from the xmain processes
documented here, but the xmain processes are enforced when the component branch
is promoted to xmain trunk.

Component groups are free to create additional pages here describing their
component-specific processes.

.. _xmain_gatekeeping:

Xmain Gatekeeping
=================

All commits to the xmain trunk must be approved by an xmain gatekeeper, whose
job it is to watch for potential conflicts and ensure that all processes are
followed. A separate page describes the
:doc:`duties of the xmain gatekeeper <XmainGatekeeperDuties>`.

Before requesting gatekeeper review, please go through the following checklist
to ensure that your changes are ready for submission. Gatekeepers will also use
this checklist when evaluating change sets.

#. Make sure your changes are consistent with our
   :doc:`Open Source Decision Making Process <OpenSourceGuidelines>`. We prefer
   to upstream changes where possible to avoid having to maintain unnecessary
   customizations in xmain.
#. Make sure all changes to community files are
   :ref:`properly marked <xmain-markups>` as Intel customizations.
#. Make sure all changes related to non-public (secret) features are
   :ref:`properly marked <secret-feature-guards>` as Intel features.
#. Make sure behavioral changes in the compiler are adequately covered by
   :ref:`unit tests <unit-testing>`.
#. Conduct a :ref:`code review <code-reviews>` using Gerrit and get +1 approval
   from someone who has taken the time to thoroughly understand your changes.
#. Run an appropriate amount of
   :ref:`pre-commit testing <testing-requirements>`, and attach the test
   results to the Gerrit review. We recommend that you use the integrated
   :doc:`alloy testing feature of Gerrit <XmainAlloyGerrit>` to do this.
   Alternatively, please copy the contents of gerrit.log into the Gerrit review
   as described :ref:`here <gerrit-alloy-fallback>`. Any expected failures
   must be captured in JIRA, and the Gerrit review must contain a clear
   explanation for why the change should be approved for xmain in spite of the
   failures.

The next step depends on whether you are submitting an individual change set or
a branch promotion.

For individual change sets, the gatekeeping process is integrated into
`Gerrit <https://git-amr-2.devtools.intel.com/gerrit>`_. Simply add
``xmain gatekeeper`` as you would a normal code reviewer. The current
gatekeepers will be notified and will review your change set for xmain
readiness. Gatekeepers may ask questions, request additional changes, request
additional testing, etc. Once satisfied, a gatekeeper will give +2 and
then you may submit your changes to xmain. Changes might need to be rebased
before submission. If your changes rebase cleanly, you still have permission to
submit your changes, but you will need to reinstate the code review +2 if
Gerrit cleared it. If there are conflicts during rebase, you will need to
resolve them and upload a new change set for further review. The amount of
testing of the new change set is at your discretion but requires agreement
from the gatekeeper.

For branch promotions, gatekeeping is done primarily via
`Gerrit <https://git-amr-2.devtools.intel.com/gerrit>`_ and `repo upload`,
using the following process:

- Multiple-repository promotions must be tested locally using alloy,
  whereas single-repository promotions may still use alloy testing
  integrated to `Gerrit <https://git-amr-2.devtools.intel.com/gerrit>`_.

- The promoter may set the code review +1 or ask one or more team members
  to inspect the promotion changes and set code review +1.

- The actual push of the promotion changes must be done using `ics merge -push`,
  because `Gerrit <https://git-amr-2.devtools.intel.com/gerrit>`_ does not
  support submitting promotion changes.

- The created `Gerrit <https://git-amr-2.devtools.intel.com/gerrit>`_ review
  must be abandoned after `ics merge -push` is done.

..
    The following paragraph provides a link that automatically opens up an email
    with the xmain checkin request form. It is not very human-readable, because
    spaces and other special characters are replaced by hex directives, e.g.
    %20. We should change this if there is a more human-readable form that
    achieves the same functionality.

Whenever possible, perform branch promotions using
`Gerrit <https://git-amr-2.devtools.intel.com/gerrit>`_ to notify
``xmain gatekeeper``.  If for some reason it is not possible, then
gatekeeping must be done via email. When a developer is ready
to commit a change, the `xmain checkin request form
<mailto:icl.xmain.gatekeeper@intel.com?
subject=xmain%20checkin%20request%20(Edit%20this%20description%20and%20date%20
(01/01/2018)&
body=1.%20Describe%20the%20new%20features%20or%20changes.%20Include%20Jira%23
%20where%20applicable.%0D%0A%0D%0A%0D%0A%0D%0A
2.%20Please%20explain%20why%20this%20change%20set%20should%20not%20be%20
upstreamed%20to%20LLVM%20open%20source.%0D%0A%0D%0A%0D%0A%0D%0A
3.%20Please%20list%20all%20modified,%20added%20or%20deleted%20files%20and%20
directories.%0D%0A%0D%0A%0D%0A%0D%0A
4.%20Was%20every%20change%20in%20this%20change-set%20code%20reviewed%3F%20If%20
this%20is%20anything%20other%20than%20a%20single%20component%20promotion%20
checkin%20request,%20please%20list%20the%20code%20reviewers.%0D%0A%0D%0A%0D%0A
%0D%0A
5.%20Does%20every%20change%20in%20the%20LLVM/Clang%20portions%20of%20the%20
source%20tree%20have%20corresponding%20changes%20that%20provide%20unit%20
testing%20coverage%3F%20Are%20any%20of%20the%20newly%20added%20unit%20tests%20
currently%20failing%3F%0D%0A%0D%0A%0D%0A%0D%0A
6.%20What%20testing%20was%20done%20(list%20the%20exact%20command
%20used%20to%20run%20alloy)%3F%20Please%20explain%20anything%20in%20the%20
fail.log%20or%20problem.log%20files,%20and%20why%20the%20checkin%20should%20
be%20allowed%20with%20these%20failures.%20For%20every%20new%20or%20flaky%20
failure%20in%20fail.log,%20a%20JR%20must%20be%20filed%20if%20one%20does%20not
%20already%20exist,%20and%20the%20JR%20number%20provided.%20Was%20any%20
testing%20done%20in%20addition%20to%20alloy%3F%0D%0A%0D%0A%0D%0A%0D%0A
Please%20attach%20the%20following%20files%20from%20your%20alloy%20run,%20if%20
applicable%3A%20status.log,%20fail.log,%20problem.log,%20and%20
zperf%5Frt%5Frpt.log.%0D%0Axmain%20checkin%20questionnaire%20version%204>`_
should be filled out and mailed to the ICL Xmain Gatekeeper.

.. _xmain-markups:

Marking Intel-specific Code in Xmain
====================================

**Every** Intel-specific change to Xmain needs to be marked as such in the
source. This makes it easier for the people merging with the community sources
to do the right thing. There are several acceptable ways to mark Intel-specific
code changes. When choosing a method for marking your code, the most important
consideration is clarity & readability.

For the reader's convenience, the comprehensive list of supported markups
is in the :ref:`Intel code markup references table <supported-markups>`.

- For multi-line additions, the preferred method is to enclose the
  Intel-specific code like this.

.. code-block:: c++

  #if INTEL_CUSTOMIZATION
    // Optimize the size of ICmp and eliminate unnecessary instructions.
    if (Instruction *R = OptimizeICmpInstSize(I, Op0, Op1))
      return R;
  #endif // INTEL_CUSTOMIZATION

- Multi-line modifications may be marked in the same way. There is no need to
  retain the original unmodified community code as that would usually degrade
  clarity. As such, ``#if INTEL_CUSTOMIZATION`` directives should rarely, if
  ever, have an accompanying ``#else``. Here is an example of proper usage.

.. We cannot format this block as c++ due to the diff markers.
.. code-block:: text

  -  // If there is a trivial two-entry PHI node in this basic block, and we can
  -  // eliminate it, do so now.
  -  if (PHINode *PN = dyn_cast<PHINode>(BB->begin()))
  -    if (PN->getNumIncomingValues() == 2)
  -      Changed |= FoldTwoEntryPHINode(PN, TTI, DL);
  +#if INTEL_CUSTOMIZATION
  +  // If there is a PHI node in this basic block, and we can
  +  // eliminate some of its entries, do so now.
  +  if (PHINode *PN = dyn_cast<PHINode>(BB->begin())) {
  +    // FoldPHIEntries is an Intel customized generalized version of the LLVM
  +    // open source routine called FoldTwoEntryPHINode(that folds a two-entry
  +    // phinode into "select") which is capable of handling any number
  +    // of phi entries. It iteratively transforms each conditional into
  +    // "select". Any changes (one such change could be regarding cost model)
  +    // made by the LLVM community to FoldTwoEntryPHINode will need to be
  +    // incorporated to this routine (FoldPHIEntries).
  +    // To keep xmain as clean as possible we got rid of the FoldTwoEntryPHINode,
  +    // therefore, there might be conflicts during code merge. If resolving
  +    // conflicts becomes too cumbersome, we can try something different.
  +    Changed |= FoldPHIEntries(PN, TTI, DL);
  +  }
  +#endif

- Use `# INTEL_CUSTOMIZATION`, `# end INTEL_CUSTOMIZATION` pair for files
  that are not run through the preprocessor, and support `#` as a comment mark,
  e.g. CMakeLists.txt and other CMake files.

- Use `// INTEL_CUSTOMIZATION`, `// end INTEL_CUSTOMIZATION` pair for files
  that are not run through the preprocessor, and support `//` as a comment mark.
  The tablegen files (.td) used to fall into this category, but they do support
  preprocessing directives now.

- Use `; INTEL_CUSTOMIZATION`, `; end INTEL_CUSTOMIZATION` pair for files
  that are not run through the preprocessor, and support `;` as a comment mark.

- Use `\.\. INTEL_CUSTOMIZATION`, `\.\. end INTEL_CUSTOMIZATION` pair
  for documentation files (see the
  :ref:`Intel code markup references table <supported-markups>`
  for more detail).

- For small additions or modifications, it is often clearer to add a comment at
  the end of **each** modified line like this.

.. code-block:: c++

  Inliner::Inliner(char &ID, bool InsertLifetime)
      : CallGraphSCCPass(ID), InsertLifetime(InsertLifetime), // INTEL
        Report(IntelInlineReportLevel) {}                     // INTEL

- Pure deletions should be excluded with an explanatory comment like this.

.. code-block:: c++

  #if !INTEL_CUSTOMIZATION
      // This code isn't needed with the Intel customizations, because we always
      // run the SSAUpdater to resolve cross-BB references.
      // Remap the value if necessary.
      if (Instruction *Inst = dyn_cast<Instruction>(IV)) {
        DenseMap<Instruction*, Value*>::iterator I = ValueMap.find(Inst);
        if (I != ValueMap.end())
          IV = I->second;
      }
  #endif // !INTEL_CUSTOMIZATION

- For Intel-added files, the fully qualified file name should contain ``Intel``
  or ``intel``. You do not need to put any special markups in the sources,
  as these files can be thought of as having implicit INTEL_CUSTOMIZATION
  begin/end markers enclosing them.
  Intel-added files should be headed by an Intel copyright
  notice, not by the typical LLVM one. The following is a sample that you can
  adapt by changing the filename, file description, and copyright dates
  appropriately.

.. code-block:: c++

  //==--- Intel_Directives.cpp - Table of directives and clauses -*- C++ -*---==//
  //
  // Copyright (C) 2015-2017 Intel Corporation. All rights reserved.
  //
  // The information and source code contained herein is the exclusive property
  // of Intel Corporation and may not be disclosed, examined or reproduced in
  // whole or in part without explicit written authorization from the company.
  //
  // ===--------------------------------------------------------------------=== //

- For code which should be excluded from final release
  builds but included in 'prod' builds during development (such as IR
  printing capabilities), you should use the `INTEL_INTERNAL_BUILD`
  markup symbol (this includes both the files that are run through
  the preprocessor and the files that are not).  This macro symbol will be
  **undefined** for the `release` builds when ics usage is set to qa mode
  (using `ics set usage qa`).  For example:

.. parsed-literal::

  void MyClass::print(raw_ostream &OS) const {
  #if INTEL_INTERNAL_BUILD
    // Print the IR for MyClass to OS.
    OS << MyClass.A << "\n";
  #endif // INTEL_INTERNAL_BUILD
  }

..

  `INTEL_INTERNAL_BUILD` **must not** be used in release package's files
  (e.g. compiler header files).  Any non-release changes in such files
  must correspond to some feature and be appropriately
  :ref:`guarded <secret-feature-guards>`.

Marking Intel Code Intended for External Sharing in Xmain
=========================================================

``What are COLLAB changes``
---------------------------

This section describes the mechanism used to mark Intel code that is
intended for external sharing and collaboration, and likely open-sourced
at some point in the future.
We refer to such Intel changes as `COLLAB changes`, mark them
with INTEL_COLLAB instead of INTEL_CUSTOMIZATION, and apply different
rules from the latter.

Marking COLLAB changes as such facilitates automating the extraction of
patches that include only the COLLAB changes and exclude **all** other
Intel changes not so marked, such as the INTEL_CUSTOMIZATION changes.
Thus, Intel can collaborate with the community by periodically providing
patches with Intel-added features long before the actual open-sourcing
of these features takes place.

``Marking COLLAB changes in C/C++ source``
------------------------------------------

The smallest unit of a COLLAB change is a line. A change is either an
`addition` of line(s), or a `deletion` of line(s).
A `modification` is just addition plus deletion.
The examples below show how to mark additions, deletions, and modifications
in C/C++ source code with the INTEL_COLLAB marker.
The part enclosed between a pair of INTEL_COLLAB begin/end markers is
referred to as a `COLLAB region`.

Assume that this is existing community code:

.. code-block:: c++

  void foo() {
    bar(123);
    bar(789);
  }

- How to mark a COLLAB addition:
  Enclose the line(s) being added between a pair of
  INTEL_COLLAB begin/end markers.
  No code outside of this COLLAB region is changed by this addition.
  Below is an example showing how to add a call to new_call():

.. code-block:: c++

  void foo() {
    bar(123);
  #if INTEL_COLLAB
    new_call();
  #endif // INTEL_COLLAB
    bar(789);
  }

- How to mark a COLLAB deletion:
  Enclose the line(s) being deleted between the #else and the #endif
  of the COLLAB region. No change may be made to the line(s) being deleted.
  No code outside of this COLLAB region is changed by this deletion.
  Document the reason of the deletion, as shown in the example below
  where the call to bar(789) is deleted:

.. code-block:: c++

  void foo() {
    bar(123);
  #if INTEL_COLLAB
    // Removed call to bar(789) because ...
  #else // INTEL_COLLAB
    bar(789);
  #endif // INTEL_COLLAB
  }

- A modification is an addition combined with a deletion in the same
  COLLAB region. The example below changes bar(123) to bar(123, x)

.. code-block:: c++

  void foo() {
  #if INTEL_COLLAB
    bar(123, x);
  #else // INTEL_COLLAB
    bar(123);
  #endif // INTEL_COLLAB
    bar(789);
  }


``Marking COLLAB changes in other file types``
----------------------------------------------

The COLLAB changes also occur in files other than C/C++ source files.
Shown below are the markers for other file types currently supported.
For these file types we have not found a need to support deletion,
so the "else" part is not defined and is not supported by the
patch-extracting tool.

- CMakeLists.txt

.. code-block:: cmake

   # INTEL_COLLAB
    ...
   # end INTEL_COLLAB

- CMakeLists.txt (alternate form that is also supported)

.. code-block:: cmake

   if (INTEL_COLLAB)
    ...
   endif (INTEL_COLLAB)

- LLVMBuild.txt

.. code-block:: text

   ; INTEL_COLLAB
    ...
   ; end INTEL_COLLAB

- TableGen (.td) files

.. code-block:: text

   // INTEL_COLLAB
    ...
   // end INTEL_COLLAB


``Marking an entire Intel-added file as COLLAB``
------------------------------------------------

If an Intel-added file is meant for external sharing then
all of its content must be marked as a COLLAB region; i.e.,
its first and last lines must be INTEL_COLLAB begin/end markers.
Note that an Intel-added file either has no INTEL_COLLAB markers,
or has the entire content enclosed between such markers.
This is true even if only part of the Intel-added file
is meant for external sharing; the next sub-section shows how to
exclude code inside a COLLAB region from being shared externally.

LLVM files often use an emacs file-type marker in a comment in their
first line, so when an INTEL_COLLAB marker becomes the first
line in such files, it must coexist with the emacs marker:

- C++ include (.h) files

.. code-block:: c++

   #if INTEL_COLLAB // -*- C++ -*-
    ...
   #endif // INTEL_COLLAB

- LLVMBuild.txt

.. code-block:: text

   ; INTEL_COLLAB   -*- Conf -*-
    ...
   ; end INTEL_COLLAB

- TableGen (.td) files

.. code-block:: text

   // INTEL_COLLAB  -*- tablegen -*-
    ...
   // end INTEL_COLLAB

..

  TableGen does support preprocessing, but the upstream version
  only supports '#ifdef' and does not support '#if'.  As long as
  our rules allow only '#if' and does not allow '#ifdef', we have to
  insert INTEL_COLLAB comments instead of real preprocessing
  directives.  Otherwise, external users of the INTEL_COLLAB
  code would have problems with '#if' usage.


``Excluding code from sharing within a COLLAB change``
------------------------------------------------------

It is not allowed to nest a an INTEL_COLLAB region inside another
INTEL_COLLAB or INTEL_CUSTOMIZATION region.

However, we allow nesting of INTEL_CUSTOMIZATION inside a COLLAB change
to exclude portions of code from being shared externally. This is useful
to mark portions of proprietary logic within a COLLAB region so that
the proprietary logic is excluded from the COLLAB patch.
In the example below, the calls to Intel_code_to_share() will appear in
the COLLAB patch, but it will not include the call to Intel_prorietary_foo():

.. code-block:: c++

  #if INTEL_COLLAB
  ...
  void Intel_func_to_share() {
    Intel_code_to_share();
    ...
    #if INTEL_CUSTOMIZATION
      Intel_prorietary_foo();
    #endif // INTEL_CUSTOMIZATION
    ...
    Intel_code_to_share();
  }
  ...
  #endif // INTEL_COLLAB

The example below uses an #else in the INTEL_CUSTOMIZATION region to switch
between two versions of a function foo(), one proprietary and one for sharing.
Under xmain, Intel_func_to_share() calls Intel_prorietary_version_of_foo().
But in the COLLAB patch, Intel_func_to_share() calls
Intel_shareable_version_of_foo() instead.

.. code-block:: c++

  #if INTEL_COLLAB
  ...
  void Intel_func_to_share() {
    Intel_code_to_share();
    ...
    #if INTEL_CUSTOMIZATION
      Intel_prorietary_version_of_foo();
    #else
      Intel_shareable_version_of_foo();
    #endif // INTEL_CUSTOMIZATION
    ...
    Intel_code_to_share();
  }
  ...
  #endif // INTEL_COLLAB

The example below is similar, but the parent function is community code.
For xmain, we want the parent function to call
Intel_prorietary_version_of_foo(), but for the COLLAB patch we want it
to call Intel_shareable_version_of_foo():

.. code-block:: c++

  void existing_community_function() {
    some_community_code();
    #if INTEL_COLLAB
      #if INTEL_CUSTOMIZATION
        Intel_prorietary_version_of_foo();
      #else
        Intel_shareable_version_of_foo();
      #endif // INTEL_CUSTOMIZATION
    #endif // INTEL_COLLAB
    some_more_community_code();
  }

``When a COLLAB change is open-sourced``
----------------------------------------

When a COLLAB change is promoted to llvm.org, then it is considered
community code and no longer Intel code, so we must remove its INTEL_COLLAB
markers from xmain.

.. _secret-feature-guards:

Guarding Intel Secret Features in Xmain
=======================================

Source code changes added to support some non-public software or hardware
feature are called `secret`, and the feature itself is called
a `secret feature`.

As long as ICL team may share its source code contributions to LLVM with
other Intel teams, and not all of these teams have access to all
`secret features`, the following development practices must be applied
to guarantee that the information about a `secret feature` is not shared
with someone who does not have the business need to know.

Currently, most ICL software contributions, such as the HIR vectorizer and
the loop optimizer, although being Intel Top Secret in terms of IP
classification, may be shared with Intel groups outside of ICL and so
are not considered `secret` for the purposes of this process.
At the same time, any IP that was not created by ICL and that has some
compiler support (e.g. new ISA support) is considered `secret`
and the corresponding compiler changes must be guarded as described
in this section.

These development practices are tightly tied to the processes run inside ICL,
e.g. :ref:`xmain-shared repository <xmain-shared-process>` and
:ref:`release builds <release-compiler-build>`, so it is
critical to follow every rule, especially while the verification and
enforcement tools/processes are not in place.

.. _secret-commits:

Commits of changes for a secret feature
---------------------------------------

**Any** git commit message containing explicit information about a secret
feature must use the following guards to keep the message secret:

.. parsed-literal::

  // INTEL_FEATURE\_ISA_AVX512VL
  This commit is to fix JIRA #777 with ISA_AVX512VL.
  // end INTEL_FEATURE\_ISA_AVX512VL

A more neutral message may be used for the same commit without guarding
the message itself.

.. parsed-literal::

  This commit is to fix JIRA #777.

If your commit message contains information about multiple `secret` features,
you must guard each reference separately with the corresponding feature name:

.. parsed-literal::

  // INTEL_FEATURE\_ISA_AVX512VL
  This commit is to fix JIRA #777 with ISA_AVX512VL.
  // end INTEL_FEATURE\_ISA_AVX512VL
  // INTEL_FEATURE\_ISA_AVX512DQ
  This commit is to fix JIRA #777 with ISA_AVX512DQ.
  // end INTEL_FEATURE\_ISA_AVX512DQ

Marking Intel secret code in files
----------------------------------

All changes exposing any information about a `secret` feature must be guarded.
The guards described in this section must accompany the `INTEL_CUSTOMIZATION`
:ref:`guards <xmain-markups>`.

- Files that are run through the preprocessor must use usual syntax:

.. parsed-literal::

  #if INTEL_CUSTOMIZATION
  #if INTEL_FEATURE\_ISA_AVX512VL
  // AVX512VL specific code.
  #endif // INTEL_FEATURE\_ISA_AVX512VL
  #endif // INTEL_CUSTOMIZATION

..

.. note:: The compiler must build with and without any of INTEL_FEATURE\_XXX
          defined.  If an INTEL_FEATURE\_XXX is not defined, the compiler
          must be fully functional, except for the disabled feature's support.

..

  These guards must be used the same in either modified LLVM files or
  Intel-added source files.  `INTEL_CUSTOMIZATION` guard may be omitted
  in Intel-added source, unless the file is intended for external collaboration
  (i.e. the whole file is guarded with `INTEL_COLLAB`) - in this case,
  the `INTEL_CUSTOMIZATION` guard must be used.

- Files that are not run through the preprocessor must use the appropriate
  markup syntax from the
  :ref:`Intel code markup references table <supported-markups>`.

.. _whole-file-guards:

- If you add a feature specific file (thus, Intel-added file) into
  a community directory, you must enclose the whole file content
  into the corresponding `INTEL_FEATURE\_` region, i.e. the region
  must start at the first line and end at the last line of the file.
  The file name may contain the feature name, e.g. `Intel_ISA_AVX512VL.cpp`.
  If the file's complete contents are not guarded, then this file
  may leak to :ref:`xmain-shared repository <xmain-shared-process>`.

.. _feature-specific-dir:

- For convenience, feature specific directories may be marked,
  so that the files inside such directories may avoid `INTEL_FEATURE\_`
  guards.  To mark a directory as feature specific you create
  `.intel_features` file inside it and put the corresponding feature
  name into it like this:

.. parsed-literal::

  INTEL_FEATURE\_ISA_AVX512VL
  <EOF>

..

  The file must contain **exactly** one feature name.  With this,
  all the files inside this directory may be written without
  `INTEL_FEATURE\_` guards.  Obviously, only Intel-added directories
  may be marked as feature specific.  Do not forget to name the directory
  using `Intel_` prefix.  Alternatively, you may choose to put all your
  new directories into `llvm/Intel_OptionalComponents` sub-directory
  with a name corresponding to the feature name, e.g. `ISA_AVX512VL`.

- Changes in header files that are shipped with the compiler release package
  must also be guarded.  If you make feature specific modifications in an
  existing header file, you must use C-style region guards like this, unless
  this is a known C++ header that can use C++-style region guards:

.. _header-mod:

.. parsed-literal::

  \/\* INTEL_FEATURE\_ISA_AVX512VL \*\/
  \/\*
   \* Most likely your ISA_AVX512VL compiler will define a macro,
   \* which you can use to actually guard the declarations below:
   \*\/
  #if defined(__AVX512VL__)
  \/\*
   \* Declarations guarded by a macro check that is only true
   \* for ISA_AVX512VL compiler build.
   \*\/
  #endif
  \/\* end INTEL_FEATURE\_ISA_AVX512VL \*\/

..

  These region guards in header files are used for both
  :ref:`xmain-shared repository <xmain-shared-process>` update process and
  :ref:`release builds <release-compiler-build>` process,
  so it is very important to have them in place.

  If you add a new feature specific header, you must either place it
  into a :ref:`feature specific directory <feature-specific-dir>` or
  :ref:`guard the whole header's contents <whole-file-guards>`.

  The recommended solution is to create a
  :ref:`feature specific directory <feature-specific-dir>`, e.g. a sub-directory
  inside the headers directory, place your new header files into this
  sub-directory and include them as shown :ref:`here <header-mod>`.
  You must also modify the copy-lists or the corresponding `make install`
  rules (**TBD**) to copy your new header files into the deploy structure's
  header directory (not into the sub-directory, in which they exist in
  the repository.

- Just as with the compiler headers, any files shipped with the release
  package must have proper regions markups in place.

- Avoid any feature specific changes in LLVMBuild.txt files
  because the correct regions markup is not always possible.  Instead,
  modify the corresponding CMakeLists.txt as shown
  :ref:`here <cmake-customization>`.

- Changes in `dpd_icl-xtoolsup` repository (e.g. in copylist.txt)
  cannot be currently guarded in any way.  Since we are deprecating
  the copylist usage, it is allowed to make feature specific changes unguarded
  in these files.

Feature specific Intel compiler builds
======================================

This section describes methods that allow producing customized compiler
builds from the common source base of xmain.

When you add source files for a new feature, you must use
the `LLVM_INTEL_FEATURES` CMake variable to enable the corresponding
preprocessing macro and include your new files in the compiler build.

.. _vrd-config:

- First, you create a new or use an existing ICS VRD file located in
  `icsconfig` directory of the ICS workspace.  A custom value of the
  `LLVM_INTEL_FEATURES` variable may be passed to ICS build scripts
  using the following option:

.. parsed-literal::

  \-intel-features=\"INTEL_FEATURE\_ISA_AVX512VL;INTEL_FEATURE\_ISA_AVX512F\"

..

.. note:: `LLVM_INTEL_FEATURES` is used for building `llvm-config` utility,
          so it is always possible to get the list of features enabled
          for a particular compiler build by running
          `llvm-config --intel-features`

..

.. _supported-features:

- Second, you add the feature name into
  `llvm/Intel_OptionalComponents/Intel_SupportedFeatures.txt`, otherwise,
  the compiler build will fail instructing you to add the feature name
  into the file.  At this point, the feature has to be classified as
  either `public` or `secret` in
  `llvm/Intel_OptionalComponents/Intel_SupportedFeatures.txt` file.
  The classification is only used for
  :ref:`xmain-shared repository <xmain-shared-process>`
  update process, and it does not affect
  :ref:`release builds <release-compiler-build>` process,
  i.e. a release build may be done with whatever features both `secret`
  and `public`.  Please refer to
  `llvm/Intel_OptionalComponents/Intel_SupportedFeatures.txt` for more details
  on the syntax and the feature naming conventions.

- Third, you modify the corresponding CMakeLists.txt files to include your
  new feature specific files only into builds that support this feature:

.. parsed-literal::

  # INTEL_CUSTOMIZATION
  set(INTEL_SOURCE_FILES_TO_BUILD)
  # Add 'Intel_ExistingCustomFile.cpp' to INTEL_SOURCE_FILES_TO_BUILD,
  # if INTEL_CUSTOMIZATION is enabled;  add it to LLVM_OPTIONAL_SOURCES
  # otherwise.
  intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
    COMPLEMENT LLVM_OPTIONAL_SOURCES
    Intel_ExistingCustomFile.cpp
    )

  # INTEL_FEATURE\_ISA_AVX512VL
  # Add 'Intel_ISA_AVX512VL.cpp' into INTEL_SOURCE_FILES_TO_BUILD,
  # if Intel feature ISA_AVX512VL and INTEL_CUSTOMIZATION are enabled;
  # add it to LLVM_OPTIONAL_SOURCES otherwise.
  intel_add_file(INTEL_SOURCE_FILES_TO_BUILD
    COMPLEMENT LLVM_OPTIONAL_SOURCES
    FEATURE ISA_AVX512VL
    ${LLVM_MAIN_SRC_DIR}/Intel_OptionalComponents/AVX512VL/lib/Transforms/Intel_ISA_AVX512VL.cpp
    )
  # end INTEL_FEATURE\_ISA_AVX512VL
  # end INTEL_CUSTOMIZATION

  add_llvm_library(LLVMCodeGen
  # INTEL_CUSTOMIZATION
    ${INTEL_SOURCE_FILES_TO_BUILD}
  # end INTEL_CUSTOMIZATION
    ...
    )

..

.. note:: LLVM_OPTIONAL_SOURCES variable helps to avoid build errors for files
          that are not used during build but are present in the source tree.

..

  If you create new header files that need to be included in existing files,
  then you must modify `llvm/CMakeLists.txt` like this:

.. parsed-literal::

  # INTEL_CUSTOMIZATION
  set(INTEL_FEATURESPECIFIC_INCLUDE_DIRS)
  # INTEL_FEATURE\_ISA_AVX512VL
  intel_add_file(INTEL_FEATURESPECIFIC_INCLUDE_DIRS
    FEATURE ISA_AVX512VL
    ${LLVM_MAIN_SRC_DIR}/Intel_OptionalComponents/AVX512VL/include
  )
  # end INTEL_FEATURE\_ISA_AVX512VL
  include_directories(AFTER ${INTEL_FEATURESPECIFIC_INCLUDE_DIRS})
  # end INTEL_CUSTOMIZATION

..

  If you create a new LLVM component library for your feature, you must
  put all the files into a feature specific
  :ref:`directory <feature-specific-dir>` and create the corresponding
  LLVMBuild.txt file declaring this library as `optional` (as long as
  it will not be built in all ICS configurations):

.. code-block:: text

  [component_0]
  type = OptionalLibrary
  name = Intel_ISA_AVX512VLSupport
  ...

..

  To add dependencies to this new library avoid modifying the existing
  (community and Intel-added) LLVMBuild.txt files, such as:

.. parsed-literal::

  [component_0]
  type = Library
  name = Scalar
  parent = Transforms
  library_name = ScalarOpts
  required_libraries = AggressiveInstCombine Analysis Core InstCombine Support TransformUtils
  ; INTEL_CUSTOMIZATION
  ; INTEL_FEATURE\_ISA_AVX512VL
    Intel_ISA_AVX512VLSupport
  ; end INTEL_FEATURE\_ISA_AVX512VL
  ; end INTEL_CUSTOMIZATION

..

.. _cmake-customization:

  **Instead**, you must modify the corresponding CMakeLists.txt file like this:

.. parsed-literal::

  # INTEL_CUSTOMIZATION
  # INTEL_FEATURE\_ISA_AVX512VL
  # Set p to TRUE, if ISA_AVX512VL is enabled.
  is_intel_feature_enabled(p ISA_AVX512VL)
  if (p)
    target_link_libraries(LLVMScalarOpts PRIVATE LLVMIntel_ISA_AVX512VLSupport)
  endif()
  # end INTEL_FEATURE\_ISA_AVX512VL
  # end INTEL_CUSTOMIZATION

..

  The two provided methods (`intel_add_file` and `is_intel_feature_enabled`)
  should allow you to do whatever customization in CMakeLists.txt files.
  Please remember to guard your feature specific modifications in these files.

LIT testing for feature specific Intel compiler builds
======================================================

The recommended way of adding feature specific LIT tests is to put
the tests into the corresponding :ref:`sub-directory <feature-specific-dir>`
of `llvm/Intel_OptionalComponents`, and add the test suite conditionally
based on the `LLVM_INTEL_FEATURES` CMake variable value.  For example,
see how `DTrans` tests are added in
`llvm/Intel_OptionalComponents/DTrans/test/CMakeLists.txt`.

For convenience, it is allowed to add new tests into the existing test
suites.  As usual, the test files must be properly
:ref:`guarded <whole-file-guards>`.  You may use the
`REQUIRES\: \<feature\>` and `UNSUPPORTED\: \<feature\>` directives,
supported by `llvm-lit`, to identify whether a LIT test
must run with the current compiler build.  `\<feature\>` is a lower-case
version of the corresponding feature from `LLVM_INTEL_FEATURES`, e.g.
`INTEL_FEATURE\_ISA_AVX512VL` compiler feature enables
`intel_feature_isa_avx512vl` LIT feature.

Every feature specific LIT test must use the corresponding `REQUIRES`
directive.  For example, a C++ LIT test will look like this:

.. parsed-literal::

  // INTEL_FEATURE\_ISA_AVX512VL
  // REQUIRES: intel_feature_isa_avx512vl
  void foo() {} // sanity test
  // end INTEL_FEATURE\_ISA_AVX512VL

..

Existing LIT tests that become invalid for a feature specific compiler build
must use `UNSUPPORTED` directive to disable the test for this particular
build, e.g.:

.. parsed-literal::

  // INTEL_FEATURE\_ISA_AVX512VL
  // UNSUPPORTED: intel_feature_isa_avx512vl
  // end INTEL_FEATURE\_ISA_AVX512VL
  void foo() {} // sanity test

..

TC testing for feature specific Intel compiler builds
=====================================================

TC has comprehensive list of controls to enable/disable particular
tests, based, for example, on the compiler build's ICS configuration.

There is currently no way to partition the TC tests data base and
use different access rights to different portions.  This means
any person having access to TC tests data base, has access to all
tests.  This does not constrain adding TC tests for `secret` features,
but special care must be taken when a person is granted access
to TC test data base.  Basically, the person must be approved
to get access to all `secret` features.

.. _release-compiler-build:

**Release** feature specific Intel compiler builds
==================================================

ICL develops many `public` and `secret` features in xmain,
and we have to be able to build a `release` compiler at any point
in time for any subset of the features :ref:`supported <supported-features>`
by the compiler.  This section describes a process of building a `release`
compiler, and it starts with the requirements:

- Each `release` compiler build is defined by a list of features
  (both `public` and `secret`).  We will refer to this list
  as `features-list`.

- A built `release` compiler must support all features from `features-list`
  e.g. the compiler binaries are functional a for these features.
  Any features not listed must not be supported by the compiler binaries.

- Compiler header files shipped with the compiler package may contain
  code for the features from `features-list` - this code must remain
  in the heder files, otherwise, the code for any feature not listed
  must be stripped from the `release` compiler package.

- The special `INTEL_INTERNAL_BUILD` feature must never be
  listed in `features-list` for a `release` build.

ICL uses the following process for building `release` compilers:

- Every different flavor of the `release` compilers is defined
  by the corresponding ICS configuration file (.vrd), for example,
  xmainavx512vlefi2linux - compiler with `ISA_AVX512VL` support
  that may be shipped to customer `A` under NDA; xmainfutureisaefi2linux -
  compiler with all future `ISA` support that may be shipped to AEs
  for early evaluation; xmainefi2linux - compiler without any `secret`
  features that may be used as a generic product release.

- Such a configuration file :ref:`defines <vrd-config>` all features from
  `features-list` for the ICS build tools.  This guarantees that the compiler
  binary files only support the specified list of features.
  'prod' and 'debug' configurations must list `INTEL_INTERNAL_BUILD`,
  whereas 'release' configuration must not list it.

- The default 'debug' and 'prod' builds should have the same feature list
  as the default release build with the exception of
  `INTEL_INTERNAL_BUILD`.

- ICS QA/deploy tools use the same list of features from the configuration file
  to process source files shipped with the `release` package, such as compiler
  header files.  The tools strip regions for all features not from
  `features-list`.  The list of formats of the regions for stripping
  is in the :ref:`Intel code markup references table <supported-markups>`.
  There is a filtering :ref:`tool <feature-filter-tool>` that must be used
  for stripping the regions.  The QA/deploy tools specify the list of features
  from the ICS configuration file to the filtering
  :ref:`tool <feature-filter-tool>` - the tool filters out all the not listed
  features' regions.

Intel code markup references table
==================================

.. _supported-markups:

The table below lists all the supported ways of marking Intel custom code.
There are different allowed markups for INTEL_CUSTOMIZATION and INTEL_FEATURE\_.
All the markups allow an arbitrary amount of whitespace, but otherwise should
be used exactly as written in the table.

If it is not listed in the table, then it is **NOT SUPPORTED**.
If you use something that is not in the table, expect that your code will
fail QA verification.  For example, you cannot use negation for INTEL_FEATURE\_
checks, but you can use it for INTEL_CUSTOMIZATION checks:

.. note:: The table may be extended.  All extensions need to be discussed with
          `ICL Process Management Team <mailto:ICLProcessTeam@intel.com>`_

.. parsed-literal::

  // This usage is **not** allowed.
  #if !INTEL_FEATURE\_ISA_AVX512VL
  // Some code.
  #endif // INTEL_FEATURE\_ISA_AVX512VL

.. parsed-literal::

  // This usage with empty #if clause is allowed.
  #if INTEL_FEATURE\_ISA_AVX512VL
  #else // INTEL_FEATURE\_ISA_AVX512VL
  // Some code.
  #endif // INTEL_FEATURE\_ISA_AVX512VL

.. |br| raw:: html

   <br />

+-------------------------+-------------------------------------+------------------------------------+
| File type               | Intel customization markup          | Intel feature markup               |
+=========================+=====================================+====================================+
| `.cpp/.h`               |                                     |                                    |
| |br|                    | `#if INTEL_CUSTOMIZATION`           | `#if INTEL_FEATURE\_XXX`           |
| Other files             | |br|                                | |br|                               |
| included into           | `...`                               | `...`                              |
| C/C++ files,            | |br|                                | |br|                               |
| e.g. `.def`             | `#endif // INTEL_CUSTOMIZATION`     | `#endif // INTEL_FEATURE\_XXX`     |
|                         +-------------------------------------+------------------------------------+
|                         | `#if INTEL_CUSTOMIZATION`           | `#if INTEL_FEATURE\_XXX`           |
|                         | |br|                                | |br|                               |
|                         | `...`                               | `...`                              |
|                         | |br|                                | |br|                               |
|                         | `#else // INTEL_CUSTOMIZATION`      | `#else // INTEL_FEATURE\_XXX`      |
|                         | |br|                                | |br|                               |
|                         | `...`                               | `...`                              |
|                         | |br|                                | |br|                               |
|                         | `#endif // INTEL_CUSTOMIZATION`     | `#endif // INTEL_FEATURE\_XXX`     |
|                         +-------------------------------------+------------------------------------+
|                         | `#if !INTEL_CUSTOMIZATION`          |                                    |
|                         | |br|                                |                                    |
|                         | `...`                               |                                    |
|                         | |br|                                |                                    |
|                         | `#endif // INTEL_CUSTOMIZATION`     |                                    |
|                         +-------------------------------------+                                    |
|                         | `#if !INTEL_CUSTOMIZATION`          |                                    |
|                         | |br|                                |                                    |
|                         | `...`                               |                                    |
|                         | |br|                                |                                    |
|                         | `#else // INTEL_CUSTOMIZATION`      |                                    |
|                         | |br|                                |                                    |
|                         | `...`                               |                                    |
|                         | |br|                                |                                    |
|                         | `#endif // INTEL_CUSTOMIZATION`     |                                    |
|                         +-------------------------------------+                                    |
|                         | `\<single-line change\> // INTEL`   |                                    |
|                         +-------------------------------------+                                    |
|                         | `\<single-line change\>`            |                                    |
|                         | `// INTEL_CUSTOMIZATION`            |                                    |
+-------------------------+-------------------------------------+------------------------------------+
| Compiler header         |                                     |                                    |
| files that are shipped  | `\/\* INTEL_CUSTOMIZATION \*\/`     | `\/\* INTEL_FEATURE\_XXX \*\/`     |
| with the compiler       | |br|                                | |br|                               |
| package                 | `...`                               | `...`                              |
|                         | |br|                                | |br|                               |
|                         | `\/\* end INTEL_CUSTOMIZATION \*\/` | `\/\* end INTEL_FEATURE\_XXX \*\/` |
|                         +-------------------------------------+------------------------------------+
|                         | `\<single-line change\>`            |                                    |
|                         | `\/\* INTEL \*\/`                   |                                    |
|                         +-------------------------------------+                                    |
|                         | `\<single-line change\>`            |                                    |
|                         | `// INTEL_CUSTOMIZATION`            |                                    |
+-------------------------+-------------------------------------+------------------------------------+
| `.td`                   | `#if INTEL_CUSTOMIZATION`           | `#if INTEL_FEATURE\_XXX`           |
|                         | |br|                                | |br|                               |
|                         | `...`                               | `...`                              |
|                         | |br|                                | |br|                               |
|                         | `#endif // INTEL_CUSTOMIZATION`     | `#endif // INTEL_FEATURE\_XXX`     |
+-------------------------+-------------------------------------+------------------------------------+
| `CMakeLists.txt`        | `# INTEL_CUSTOMIZATION`             | `# INTEL_FEATURE\_XXX`             |
| |br|                    | |br|                                | |br|                               |
| Other files             | `...`                               | `...`                              |
| recognizing `\#`        | |br|                                | |br|                               |
| as a comment, e.g.      | `# end INTEL_CUSTOMIZATION`         | `# end INTEL_FEATURE\_XXX`         |
| `.py`, `.mir`,          +-------------------------------------+                                    |
| `.gitignore`,           | `\<single-line change\> # INTEL`    |                                    |
| `.gitattribute`         +-------------------------------------+                                    |
|                         | `\<single-line change\>`            |                                    |
|                         | `# INTEL_CUSTOMIZATION`             |                                    |
+-------------------------+-------------------------------------+------------------------------------+
| Dynamic checks in       | `if(INTEL_CUSTOMIZATION)`           | Dynamic checks are supported       |
| `CMakeLists.txt` only   | |br|                                | by is_intel_feature_enabled()      |
|                         | `...`                               | macro, but they still              |
|                         | |br|                                | have to be guarded as shown        |
|                         | `else(INTEL_CUSTOMIZATION)`         | in the cell above.                 |
|                         | |br|                                |                                    |
|                         | `...`                               |                                    |
|                         | |br|                                |                                    |
|                         | `endif(INTEL_CUSTOMIZATION)`        |                                    |
|                         +-------------------------------------+                                    |
|                         | `if(INTEL_CUSTOMIZATION)`           |                                    |
|                         | |br|                                |                                    |
|                         | `...`                               |                                    |
|                         | |br|                                |                                    |
|                         | `endif(INTEL_CUSTOMIZATION)`        |                                    |
+-------------------------+-------------------------------------+------------------------------------+
| `LLVMBuild.txt`         | `; INTEL_CUSTOMIZATION`             | `; INTEL_FEATURE\_XXX`             |
| |br|                    | |br|                                | |br|                               |
| Other files recognizing | `...`                               | `...`                              |
| `\;` as a comment, e.g. | |br|                                | |br|                               |
| `.ll`                   | `; end INTEL_CUSTOMIZATION`         | `; end INTEL_FEATURE\_XXX`         |
|                         +-------------------------------------+                                    |
|                         | `\<single-line change\> ; INTEL`    |                                    |
|                         +-------------------------------------+                                    |
|                         | `\<single-line change\>`            |                                    |
|                         | `; INTEL_CUSTOMIZATION`             |                                    |
+-------------------------+-------------------------------------+------------------------------------+
| `.rst` |br|             | `\<blank line\>`                    | `\<blank line\>`                   |
| In some constructs      | |br|                                | |br|                               |
| it is not possible      | `\.\. INTEL_CUSTOMIZATION \.\*`     | `\.\. INTEL_FEATURE\_XXX`          |
| to use this syntax,     | |br|                                | |br|                               |
| though.                 | `\<blank line\>`                    | `\<blank line\>`                   |
| `\<blank line\>`  may   | |br|                                | |br|                               |
| be omitted in some      | `\.\. end INTEL_CUSTOMIZATION \.\*` | `\.\. end INTEL_FEATURE\_XXX`      |
| cases                   +-------------------------------------+                                    |
|                         | `\<blank line\>`                    |                                    |
|                         | |br|                                |                                    |
|                         | `\.\. INTEL_CUSTOMIZATION \.\*`     |                                    |
|                         | |br|                                |                                    |
|                         | `\<blank line\>`                    |                                    |
|                         | `\<single-line change\>`            |                                    |
+-------------------------+-------------------------------------+------------------------------------+

.. _xmain-shared-process:

Maintaining xmain-shared collaborative repository
=================================================

ICL provides access to its own IP added to LLVM compiler.  The access for Intel
teams is done via `xmain-shared` collaborative repository.  Right now we
anticipate only one `xmain-shared` repository that will not contain code
related to any `secret` features.  In the future, it may be possible
to have a set of collaborative repositories with different sets of shared
features.  This section describes the process of creation and regular
updates of `xmain-shared` from `xmain`.  Herefrom, `xmain` stands
for only the following repositories: `dpd_icl-llvm`, `dpd_icl-clang`
and `dpd_icl-openmp` (this list may be extended, as needed by collaborating
teams).

`dpd_icl-xtoolsup` repository must never be made available to
collaborators, because there is currently no way to guard feature
specific changes in these files.

- `xmain-shared` is created from the `xmain` repository that does not
  contain any `secret` features' implementations.  The reference date
  for such a `clean xmain` is 1/1/2017.

- Every commit to `xmain` happened after the reference date
  must be processed and put into `xmain-shared`:

  * Branch merge commit to `xmain` is processed as a single commit,
    i.e. the history of the branch commits does not get propagated to
    `xmain-shared`.

  * Merges from LLorg cannot hold any `secrets`, so they are merged
    into `xmain-shared` as merge-from-master commits, and the corresponding
    individual commits history is preserved in `xmain-shared`.

  * For each single commit, the `xmain-shared` update demon runs
    the :ref:`filtering tool <feature-filter-tool>` and passes all
    `public` features listed in :ref:`llvm/Intel_OptionalComponents/Intel_SupportedFeatures.txt <supported-features>`.
    The filtering tool is applied to a complete ICS workspace (regardless
    of the files that were modified by the single commit).
    The ICS workspace created as the result of filtering is copied over
    the current `head` `xmain-shared` workspace.
    If there are no modified files after the filtering, then this commit
    is ignored, i.e. it is not put into `xmain-shared`.

  * The original `xmain` commit message is filtered by the `xmain-shared` update
    demon regarding `INTEL_FEATURE` :ref:`regions <secret-commits>`.
    If the commit message becomes empty due to filtering, the `xmain-shared`
    commit message must say "Commit message filtered out".

- In future we may want to maintain several features list
  :ref:`files <supported-features>` for different `xmain-shared` repositories.

.. _feature-filter-tool:

Feature filtering tool specification
====================================

The following describes the functionality of the feature filtering tool:

- The tool supports feature regions guard formats listed in the
  :ref:`Intel code markup references table <supported-markups>`.

- The tool accepts `source` and `destination` paths.  All files and
  directories from `source` are recursively filtered as defined below
  and put into `destination`.

- **TBD** The tool accepts a list of features that must not be filtered out.
  The regions for features not listed must be filtered out.

- If a `source` directory contains a file named
  :ref:`.intel_features <feature-specific-dir>`,
  and inside the file there is a feature that must be filtered out
  (i.e. it is not in the list passed to the tool), then the whole
  directory is left out of `destination`.

- The tool does not process `.git` and `.repo` directories and does not
  copy them into `destination`.

- The tool reports errors on malformed regions.

- If a file from `source` must be processed (i.e. it is not located
  inside :ref:`feature specific directory <feature-specific-dir>` that
  must be completely filtered out), it contains at least one
  feature regions, and it becomes empty due to filtering, then
  this file is not copied to `destination`.

Coding Standards
================

Xmain developers are expected to adhere to the same coding standards as open
source developers. Those coding standards are documented
:doc:`here <../CodingStandards>`. The purpose of this policy is provide a
consistent set of coding standards and to make it easier to upstream changes
from xmain when we choose to do so.

We enforce this policy primarily through code reviews. If you notice any
violations, you are encouraged to fix them.

.. _unit-testing:

Unit Test Development
=====================

All functional changes to xmain must be accompanied by unit tests using the
LIT infrastructure. This requirement is no different from what the open source
community expects.

Additionally, new programmer visible features should be accompanied by
end-to-end tests in our ``tc`` test suites. Changes to the test suite are
normally made using the ``TMT`` tool.

All test changes must be code reviewed following the same
:ref:`code review <code-reviews>` processes used for compiler changes. This
includes both LIT changes and ``tc`` test changes.

.. _code-reviews:

Code Reviews
============

Our code review policy requires that every piece of code in xmain is thoroughly
understood and accepted by more than one person. Code reviews ensure
consistently high code quality and maintainability, increase understanding of
the code base among more developers, and provide a mechanism for fostering best
coding practices across our development teams.

Code reviews should be seen as more than just a final check for coding errors.
Code reviews present an opportunity for developers to learn from one another and
help one another improve their code as it is committed. Having a second person
read through your code and attempt to understand it helps identify pieces that
are confusing, inefficient, or incorrect. Code reviews are a critical mechanism
for ensuring that the code we commit to xmain is of the highest quality.

Code Review Tool
----------------

`Gerrit <https://git-amr-2.devtools.intel.com/gerrit>`_ is the official code
review tool for xmain development. All xmain code reviews should be done
through gerrit.

Choosing code reviewers
------------------------

Each change set should be reviewed by at least two developers. The code author
should designate a primary reviewer, who is responsible for thoroughly
understanding the change set and providing design-level feedback and guidance.
In addition, a secondary reviewer should be chosen. The secondary reviewer
is not required to be familiar with the particular code area being modified but
should provide general feedback on the change set, focusing on clarity,
complexity, common coding errors, data structure choice, etc.

Developers are encouraged to seek out "critical" reviewers. Keeping in mind
that improving your code is a primary goal of the review, receiving more
feedback from reviewers should be viewed as a positive outcome.

Developers are also encouraged to consider choosing reviewers who might benefit
from seeing the changes. Remember that code reviews are an opportunity for
reviewers to learn about parts of the code base with which they were not
previously familiar. Selecting a reviewer from another team can extend their
knowledge base while providing a fresh perspective for your changes.

If you are unsure who should review your changes, the advice of the LLVM
community documented `here <../Phabricator.html>`_ works just as well for
xmain. That is,

- Use ``git blame`` and the commit log to find names of people who have recently
  modified the same area of code that you are modifying.
- If you've discussed the change with others, they are good candidates to be
  your reviewers.

.. note:: We do not currently have an xmain equivalent of CODE_OWNERS.txt, but
          we are working on creating one. In case this document is out of date,
          check the root llvm directory for intel_code_owners.map or something
          similar.

Expectations of change sets
---------------------------

- Changes should be small and incremental. Do not wait until a feature is
  complete to begin the code review. Large change sets are more difficult for
  reviewers to thoroughly comprehend and discourage design-level suggestions
  that might have improved the entire implementation if they had been received
  early in the development process. Incremental changes also encourage more
  thorough testing.

- Changes should have a single purpose. Avoid combining small changes into
  patches for unrelated features. Combined changes cause details to be hidden
  in the revision history and complicate the process of isolating the cause
  of failures.

- All new functionality should be tested in some way. Change sets should include
  a regression test that verifies the correctness of the change. A well-written
  test also helps to document the intended effect of the new code.

- If the change transforms IR, debug information should be preserved whenever
  possible. Changes which copy, clone, or replace instructions should include
  a test verifying that debug information is preserved.

- All changes should be appropriately documented. The level of documentation
  required depends on the scope of the change. For trivial changes, the commit
  message may be sufficient. More complex changes should be described in code
  comments. High level design for features such as new optimization passes
  should be accompanied by RST files describing the design of the feature.
  The exact level of documentation required is at the discretion of the code
  author and reviewers. In all cases, the commit message should provide a
  good explanation of what you are trying to accomplish in the change set and
  establish any necessary context.

- Change sets should not include large scale re-formatting of existing code.
  While running clang-format on a new file before uploading it for review is a
  good practice, you should not reformat existing files in this way unless their
  formatting was previously compliant. Formatting changes can obfuscate the
  revision history and make it more difficult to identify the source of changes.
  If it is necessary to re-format a file, the formatting changes should be
  submitted as a separate change set marked "NFC" (no functional changes).

- Change sets should not be rebased mid-review if the files being modified are
  also being updated outside the change set. Rebasing files makes it more
  difficult for reviewers to determine what the author of the patch changed
  between updates of the review. It will often be necessary to rebase the code
  before it can be committed, but unless new functionality introduced by other
  commits is integral to the progression of the change under review, rebasing
  should be deferred until the author and reviewers believe the change is ready
  to be committed. If you **must** update your sources, it is helpful to upload
  a version of your changes that **only** reflects the update with no other
  changes.


Expectations of code reviewers
------------------------------
- It is the job of the primary code reviewer to **thoroughly** understand the
  code changes under review. This reviewer must understand both the high level
  design and the low level details. Every change in xmain must be given a
  detailed line-by-line code review. A cursory reading of the code is not an
  adequate code review.

- Secondary reviewers should inspect the code carefully with a focus on
  clarity and correctness.

- Code reviewers and code authors are equally responsible for the quality of
  code that gets committed to xmain.

- Reviews should be timely. At this time, we do not have a specific rule for
  how long a review should take. But remember that the code review is usually
  on the critical path for getting code committed. So make code reviews a
  priority! The appropriate time for a review depends on the scope of the
  changes. Reviewers should attempt to respond within a day for very small
  change sets (less than 50 lines of code). If a reviewer cannot begin a review
  in a timely manner, the author of the changes should be notified. For very
  large change sets the code author and the primary reviewer should have a
  discussion to form a review plan.

- Reviewers should offer positive and constructive feedback. As a reviewer
  you are collaborating with the author to ensure high quality code. Give the
  sort of feedback you would like to receive.

- Reviewers should have confidence in the code author. Start from a position
  of trusting that the author had a reason for the way the code was implemented.
  If something doesn't make sense to you, ask for an explanation.

- Reviewers are encouraged to ask questions. It is not necessary to have
  spotted a specific problem in order to provide valuable feedback. If something
  is unclear to you, it may be unclear to others. It is best to have that
  addressed during the review. It is also possible that your uncertainty is
  caused by some condition that the code's author had not considered. At the
  very least, asking questions will increase your understanding of the code.

- Reviewers should be as specific as possible with their comments and
  suggestions. Rather than just saying "this seems wrong" offer specific
  suggestions for how it can be improved.

- Reviewers should consider idioms and data structures, not just correctness.
  There are many ways to correctly implement the same algorithm. By suggesting
  better implementations during reviews we can all pass along our best
  practices to one another. The author of the code may not be aware of a data
  structure that can simplify the implementation.

- For important issues that you find, e.g. correctness or efficiency problems,
  insist that the author either fix the problem or convince you that there is
  no problem. Escalate if necessary!

- Defer to the code author on issues that are purely matters of personal
  preference. By all means make suggestions, but give the author the final say.


Expectations of code authors
----------------------------

- First and foremost, be appreciative of the time people take to review your
  code. We are all busy people.

- Proofread and test your code before requesting a code review. It is
  frustrating for code reviewers to have to correct your typos, formatting
  errors, etc.

- Respond to code review comments in a timely manner so that reviewers don't
  lose their train of thought.

- Respond to all questions asked by the reviewers. In most cases it is
  preferable to have these answers included in the review itself so that it
  can serve as a reference to anyone who might consult the review at a later
  date. If the reviewer's question causes you to rethink your implementation
  and re-write the code being asked about, still offer an answer so that the
  reviewers have some insight into your thought process.

- Address all comments and suggestions from the reviewer. In some cases it may
  be sufficient to just implement the suggested change, but if there is any
  ambiguity please respond saying how you think your changes address the
  feedback. You are not required to implement all of the reviewers' suggestions,
  but in cases where you do not agree with the suggestion you should at least
  provide an explanation of why you do not agree. Ideally the code author and
  reviewers will reach a consensus.

- Be receptive to feedback from the reviewers. Remember that the code review is
  a collaborative activity where the author and the reviewers are working
  together to improve the code. This should never feel like an adversarial
  relationship.

- Explain why you have done things as you did but avoid being defensive.
  Trust that the reviewers are trying to be helpful and are not attacking your
  code or questioning your abilities. There will be times when the reviewers
  simply do not understand what you have done. Be patient with your explanations.

- Document your response. In many cases it will be useful for code authors and
  reviewers to talk offline to discuss a change set. This is a good practice,
  but try to capture all important points that were discussed and mention them
  either in code comments or review comments for the benefit of anyone else who
  might have the same questions later.

Suggestions for further reading
-------------------------------

| `How to Do Code Reviews Like a Human (Part One) <https://mtlynch.io/human-code-reviews-1>`_
| `How to Do Code Reviews Like a Human (Part Two) <https://mtlynch.io/human-code-reviews-2>`_
| `Unlearning toxic behaviors in a code review culture <https://medium.freecodecamp.org/unlearning-toxic-behaviors-in-a-code-review-culture-b7c295452a3c>`_

.. _testing-requirements:

Testing Requirements
====================

Commits to xmain are expected to meet a minimum level of stability and
performance. Prior to requesting commit permission, developers should run
xmain_checkin for stability testing and zperf_checkin_xmain for performance
testing. The following alloy command is suitable.

::

    alloy run -file xmain_checkin -file zperf_checkin_xmain -ref_comp ws -notify

Of course, good judgment should always prevail. The gatekeeper may choose to
permit less testing for low risk change sets and may choose to require extra
testing for high risk change sets. In particular, for change sets that only
modify LIT tests, running only the alloy LIT tasks is both sufficient and more
efficient, i.e.

::

    alloy run -file xmain_lit -notify

Developers can also take advantage of integrated
:doc:`AlloyGerrit <XmainAlloyGerrit>` testing infrastructure.

Expectations Regarding New Stability Failures
---------------------------------------------

All new stability failures must be analyzed and understood. The xmain gatekeeper
will never approve a checkin request with an unanalyzed stability failure,
because without understanding the failure, it is impossible to assess its
impact.

If the developer and gatekeeper agree that a new failure has low impact, the
gatekeeper may approve the checkin in spite of the failure, provided that the
developer first submit a CQ. This commonly occurs when the failure is caused
by an error in the failing test itself.

For new LIT failures, in addition to submitting a CQ, you must mark the test as
an expected failure by adding a line like this.

::

  ; INTEL - This test is marked XFAIL due to cq415116,cq415117. Once those
  ; problems are fixed, we can restore this test to the community version.
  ; XFAIL: *
  ; END INTEL


Expectations Regarding Performance Regressions
----------------------------------------------

All performance regressions need to be justified before the gatekeeper will
approve a checkin request. Unanalyzed regressions are often allowed if they
are small and are accompanied by offsetting improvements in other tests.
Large regressions always need to be analyzed and understood. The gatekeeper
will usually not approve checkin requests involving large performance
regressions, but there may be exceptions in some cases.

The developer must submit a JIRA report for any performance regression that
requires follow-up work before the gatekeeper will approve the checkin request.

Expectations Regarding Compile Time Regressions
-----------------------------------------------

All compile time regressions need to be approved by the architecture team
prior to checkin. In general, compile time regressions will require
improvements in generated code performance to justify the cost.
