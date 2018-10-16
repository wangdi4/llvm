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

..
    The following paragraph provides a link that automatically opens up an email
    with the xmain checkin request form. It is not very human-readable, because
    spaces and other special characters are replaced by hex directives, e.g.
    %20. We should change this if there is a more human-readable form that
    achieves the same functionality.

For branch promotions, gatekeeping is done via email. When a developer is ready
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

- Some files, e.g. CMakeLists.txt, are not run through the preprocessor.
  Use # INTEL_CUSTOMIZATION, # end INTEL_CUSTOMIZATION pair. In situations
  where # INTEL_CUSTOMIZATION does not work, e.g. tablegen (.td), additions
  should be enclosed in comments like this.

.. code-block:: c++

  // INTEL_CUSTOMIZATION
  // X86_RegCall return-value convention.
  multiclass RetCC_X86_RegCall<RC_X86_RegCall RC, CallingConv CC> :
    X86_RegCall_base<RC, CC>;

  defm RetCC_X86_32_RegCall :
       RetCC_X86_RegCall<RC_X86_32_RegCall, RetCC_X86Common>;
  defm RetCC_X86_64_RegCall_Win :
       RetCC_X86_RegCall<RC_X86_64_RegCall_Win, RetCC_X86_Win64_C>;
  defm RetCC_X86_64_RegCall_Lin :
       RetCC_X86_RegCall<RC_X86_64_RegCall_Lin, RetCC_X86Common>;
  // end INTEL_CUSTOMIZATION

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

- For Intel-added files, you do not need to put any special markups in the
  sources. Instead, the fully qualified file name should contain ``Intel``
  or ``intel``. Intel-added files should be headed by an Intel copyright
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

- For code which should be excluded from final release builds but included
  in 'prod' builds during development (such as IR printing capabilities),
  you should use the 'INTEL_PRODUCT_RELEASE' preprocessor symbol.  This
  symbol will be defined only for 'release' builds when ics usage is set to
  qa mode (using 'ics set usage qa').  For example:

.. code-block:: c++

  void MyClass::print(raw_ostream &OS) const {
  #if !INTEL_PRODUCT_RELEASE
    // Print the IR for MyClass to OS.
    OS << MyClass.A << "\n";
  #endif // !INTEL_PRODUCT_RELEASE
  }

..

  This preprocessor symbol should be used the same in either modified LLVM
  files or Intel-specific source files.

Guarding Intel Proprietary Features in Xmain
============================================

``Customized compiler builds``
------------------------------

This section describes development practices that allow producing customized
compiler builds from the common source base.  Development of some SW features
and early support of HW features may require build time controls, so that
Intel secret features are not exposed in product compiler builds.

We use CMake list variable 'LLVM_INTEL_FEATURES' to hold a list of features
enabled for a particular compiler build.  Additional features may be enabled
via ICS build command option:

.. code-block:: console

  ics build FEATURES="AVX3_1,AVX3_2"

.. note:: Feature names may only consist of symbols that are valid for C/C++ identifiers.

.. note:: During the initial run of `ics build`, the list of features is fixed
          in CMake configuration, so any changes in the `FEATURES="..."` list
          in the consequent `ics build` commands will not take effect.
          To build a compiler with new list of features, please, build it
          from scratch.

'LLVM_INTEL_FEATURES' may be used for conditional CMake processing.  For example,
we have the following code in `llvm/CMakeLists.txt`:

.. code-block:: cmake

  foreach(f ${LLVM_INTEL_FEATURES})
    string(CONCAT FOPT "-DINTEL_FEATURE_" ${f} "=1")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FOPT}")
  endforeach(f)

This code populates C++ compilation flags with options like '-DINTEL_FEATURE_XXX=1'
based on the list of features provided in 'LLVM_INTEL_FEATURES' list.

.. note:: We do not currently update CMAKE_C_FLAGS, so pure C files are compiled
          without any INTEL_FEATURE\_ macros enabled.

To guard Intel secret features in C/C++ files use feature checks in addition to
'INTEL_CUSTOMIZATION' checks, e.g.:

.. code-block:: c++

  #if INTEL_CUSTOMIZATION
  #if INTEL_FEATURE_AVX3_2
  // AVX3_2 specific code.
  #endif // INTEL_FEATURE_AVX3_2
  #endif // INTEL_CUSTOMIZATION

.. note:: The compiler must build with and without any of INTEL_FEATURE_XXX defined.

To completely exclude a C/C++ file from compilation, when some feature is not
enabled, we can use conditional processing of CMake files.  In the following
example in `llvm/lib/CodeGen/CMakeLists.txt` we conditionally add `Intel_Avx3_2.cpp`
file to compilation only if AVX3_2 feature is enabled:

.. _conditional-comp:

.. code-block:: cmake

  # INTEL_CUSTOMIZATION
  set(INTEL_SOURCE_FILES
    Intel_MachineLoopOptReportEmitter.cpp
    )
  set(INTEL_AVX3_2_SOURCE_FILES
    Intel_Avx3_2.cpp
    )
  set(INTEL_SOURCE_FILES_TO_BUILD)
  if (INTEL_CUSTOMIZATION)
    if(";${LLVM_INTEL_FEATURES};" MATCHES ";AVX3_2;")
      list(APPEND INTEL_SOURCE_FILES ${INTEL_AVX3_2_SOURCE_FILES})
    else()
      list(APPEND LLVM_OPTIONAL_SOURCES ${INTEL_AVX3_2_SOURCE_FILES})
    endif()
    set(INTEL_SOURCE_FILES_TO_BUILD ${INTEL_SOURCE_FILES})
  else()
    list(APPEND LLVM_OPTIONAL_SOURCES ${INTEL_SOURCE_FILES} ${INTEL_AVX3_2_SOURCE_FILES})
  endif(INTEL_CUSTOMIZATION)
  # end INTEL_CUSTOMIZATION

  add_llvm_library(LLVMCodeGen
  # INTEL_CUSTOMIZATION
    ${INTEL_SOURCE_FILES_TO_BUILD}
  # end INTEL_CUSTOMIZATION
    ...
    )

.. note:: LLVM_OPTIONAL_SOURCES variable helps to avoid build errors for files
          that are not used during build but are present in the source tree.

C/C++ preprocessor is not run for files of other types, so we have to use
different techniques to conditionally compile them.

One of the most often changes in `LLVMBuild.txt` files is adding dependencies
to some Intel proprietary LLVM component libraries.  `LLVMBuild.txt` also do not
support inline comments, so we used to put INTEL_CUSTOMIZATION comments on
separate lines - this complicated merges for these files a little bit.

An alternative solution is to add LLVM component libraries dependencies
in CMakeLists.txt.  For example, instead of having the following code
in `llvm/lib/Transforms/Scalar/LLVMBuild.txt`:

.. code-block:: text

  [component_0]
  type = Library
  name = Scalar
  parent = Transforms
  library_name = ScalarOpts
  required_libraries = AggressiveInstCombine Analysis Core InstCombine Support TransformUtils Intel_OptReport
  ; ***INTEL: Intel_OptReport

We may have the following in `llvm/lib/Transforms/Scalar/CMakeLists.txt`:

.. _lib-dependencies:

.. code-block:: cmake

  # INTEL_CUSTOMIZATION
  target_link_libraries(LLVMScalarOpts PRIVATE LLVMIntel_OptReport)
  # end INTEL_CUSTOMIZATION

It is obvious how the LLVM component library dependence may be added
in a CMakeLists.txt based, for example, on LLVM_INTEL_FEATURES.
The only caveat is: if an Intel proprietary LLVM component library is not
used in some compiler build, this build will complain about an unused library
not being marked as `optional`.  In this case, we should mark this library
as `optional` using the following `LLVMBuild.txt` syntax:

.. _optional-lib:

.. code-block:: text

  [component_0]
  type = OptionalLibrary
  name = Intel_Avx3_2ScalarOpt
  ...

.. note:: Otherwise, we do not currently support any sort of preprocessing
          of LLVMBuild.txt files during the compiler build.

Conditional processing of tablegen (.td) files is not currently supported,
but we have agreed on the following direction:

- We will support simple preprocessor syntax, such as:

.. code-block:: c++

  // INTEL_FEATURE_AVX3_2
  // AVX3_2 specific code.
  // end INTEL_FEATURE_AVX3_2

- A special Intel tool will be called from `llvm/cmake/modules/TableGen.cmake`
  (and, maybe, other cmake scripts) to preprocess a .td file into a temporary
  .td file before passing it to the tablegen.

- The preprocessing will only work for .td files participating in tablegen()
  commands in `CMakeLists.txt` files, i.e. preprocessing will not work
  for "included" .td files.

.. note:: TODO: update this text, when the preprocessing tool is ready and
          plugged into the build process.

``Source code isolation``
-------------------------

There may be cases, when some parts of the source code must be isolated
into a separate directory with its own access rights.  For example,
the compiler team is developing a secret feature, which cannot be exposed
in any way to other teams that have access to xmain repository.
The approved solution for this is to put such source code into a subdirectory
of `llvm/Intel_OptionalComponents` and plug in things from this subdirectory
during the compiler build using LLVM_INTEL_FEATURES controls.  Each subdirectory
of `llvm/Intel_OptionalComponents` may be put into a separate repository
providing a way for access control.

If implementation of a feature may done as a LLVM component library, then
this library must be declared as :ref:`optional <optional-lib>` and it may
be optionally :ref:`linked <lib-dependencies>` to some existing LLVM component
library to extend its functionality.

In most cases, we will have to have source code references to the isolated
feature implmenentation, e.g. to C/C++ header files containing common
interfaces.  This may be done by setting include directories for the compiler
build in `llvm/CMakeLists.txt`:

.. code-block:: cmake

  if (";${LLVM_INTEL_FEATURES};" MATCHES ";AVX3_2;")
    set(INTEL_ANY_OPTIONAL_COMPONENTS TRUE)
    set(INTEL_AVX3_2_INCLUDE_DIR ${LLVM_MAIN_SRC_DIR}/Intel_OptionalComponents/AVX3_2/include)
    include_directories(AFTER ${INTEL_AVX3_2_INCLUDE_DIR})
    SET(LLVMOPTIONALCOMPONENTS ${LLVMOPTIONALCOMPONENTS} Intel_Avx3_2ScalarOpt)
  endif()

This code allows using flat C/C++ include paths for header files located
in `llvm/Intel_OptionalComponents/AVX3_2/include`.  Such include directives
obviously need to be guarded with the corresponding INTEL_FEATURE_AVX3_2
macro check.

The same way, C/C++ source files may be conditionally added to the compiler
build using full paths like :ref:`${LLVM_MAIN_SRC_DIR}/Intel_OptionalComponents/\
AVX3_2/lib/Transforms/Intel_Avx3_2.cpp <conditional-comp>`.

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
