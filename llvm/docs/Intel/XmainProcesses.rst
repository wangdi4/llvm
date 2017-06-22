===========================
Xmain Development Processes
===========================

.. contents::
   :local:

The Intel LLVM development process documentation is currently under review. If
anything looks wrong to you, please contact `David Kreitzer
<mailto:david.l.kreitzer@intel.com>`_.

Introduction
============

This document describes our processes for code development in xmain. There is a
separate :doc:`document <OpenSourceProcesses>` describing our processes for open
source LLVM/clang code development.

The xmain project stays in sync with open source via our pulldown process.
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

Xmain Gatekeeping
=================

All commits to the xmain trunk must be approved by the xmain gatekeeper, whose
job it is to watch for potential conflicts and ensure that all processes were
followed.

..
    The following paragraph provides a link that automatically opens up an email
    with the xmain checkin request form. It is not very human-readable, because
    spaces and other special characters are replaced by hex directives, e.g.
    %20. We should change this if there is a more human-readable form that
    achieves the same functionality.

When a developer is ready to commit a change, the `xmain checkin request form
<mailto:icl.xmain.gatekeeper@intel.com?
subject=xmain%20checkin%20request%20(Description%2001/01/2016)&
body=1.%20Describe%20the%20new%20features%20or%20changes.%20Include%20tracker%23
%20where%20applicable.%0D%0A%0D%0A%0D%0A%0D%0A
2.%20Please%20list%20all%20modified,%20added%20or%20deleted%20files%20and%20
directories.%0D%0A%0D%0A%0D%0A%0D%0A
3.%20Was%20every%20change%20in%20this%20change-set%20code%20reviewed%3F%20If%20
this%20is%20anything%20other%20than%20a%20single%20component%20promotion%20
checkin%20request,%20please%20list%20the%20code%20reviewers.%0D%0A%0D%0A%0D%0A
%0D%0A
4.%20Does%20every%20change%20in%20the%20LLVM/Clang%20portions%20of%20the%20
source%20tree%20have%20corresponding%20changes%20that%20provide%20unit%20
testing%20coverage%3F%20Are%20any%20of%20the%20newly%20added%20unit%20tests%20
currently%20failing%3F%0D%0A%0D%0A%0D%0A%0D%0A
5.%20What%20stability%20testing%20was%20done%20(list%20the%20exact%20command
%20used%20to%20run%20alloy)%3F%20Please%20explain%20anything%20in%20the%20
fail.log%20or%20problem.log%20files,%20and%20why%20the%20checkin%20should%20
be%20allowed%20with%20these%20failures.%20For%20every%20new%20or%20flaky%20
failure%20in%20fail.log,%20a%20CQ%20must%20be%20filed%20if%20one%20does%20not
%20already%20exist,%20and%20the%20CQ%20number%20provided.%20Was%20any%20
testing%20done%20in%20addition%20to%20alloy%3F%0D%0A%0D%0A%0D%0A%0D%0A
Please%20attach%20the%20following%20files%20from%20your%20alloy%20run,%20if%20
applicable%3A%20status.log,%20fail.log,%20problem.log,%20and%20
zperf%5Frt%5Frpt.log.%0D%0Axmain%20checkin%20questionnaire%20version%201>`_
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

- Some files, e.g. tablegen (.td) files, are not run through the preprocessor,
  so the #if INTEL_CUSTOMIZATION method does not work. For those types of files,
  multi-line additions should be enclosed in comments like this.

.. code-block:: c++

  // if INTEL_CUSTOMIZATION
  // X86_RegCall return-value convention.
  multiclass RetCC_X86_RegCall<RC_X86_RegCall RC, CallingConv CC> :
    X86_RegCall_base<RC, CC>;

  defm RetCC_X86_32_RegCall :
       RetCC_X86_RegCall<RC_X86_32_RegCall, RetCC_X86Common>;
  defm RetCC_X86_64_RegCall_Win :
       RetCC_X86_RegCall<RC_X86_64_RegCall_Win, RetCC_X86_Win64_C>;
  defm RetCC_X86_64_RegCall_Lin :
       RetCC_X86_RegCall<RC_X86_64_RegCall_Lin, RetCC_X86Common>;
  // endif INTEL_CUSTOMIZATION

- For small additions or modifications, it is often clearer to add a comment at
  the end of **each** modified line like this.

.. code-block:: c++

  Inliner::Inliner(char &ID, bool InsertLifetime)
      : CallGraphSCCPass(ID), InsertLifetime(InsertLifetime), // INTEL
        Report(IntelInlineReportLevel) {}                     // INTEL

- For Intel-added files, you do not need to put any special markups in the
  sources. Instead, the fully qualified file name should contain ``Intel``
  or ``intel``.

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

Coding Standards
================

Xmain developers are expected to adhere to the same coding standards as open
source developers. Those coding standards are documented
:doc:`here <../CodingStandards>`. The purpose of this policy is provide a
consistent set of coding standards and to make it easier to upstream changes
from xmain when we choose to do so.

We enforce this policy primarily through code reviews. If you notice any
violations, you are encouraged to fix them.

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
understood and accepted by more than one person. Having a second person read
through your code and attempt to understand it helps identify pieces that are
confusing, inefficient, or incorrect. Code reviews are a critical mechanism for
ensuring that the code we commit to xmain is of the highest quality.

Code Review Tool
----------------

`Code Collaborator <https://ir-codecollab.intel.com/ui>`_ is the official code
review tool for xmain development. Individual teams may use other tools
internally but are expected to use Code Collaborator when working with other
teams.

Choosing a code reviewer
------------------------

If you are unsure who should review your changes, the advice of the LLVM
community documented `here <../Phabricator.html>`_ works just as well for
xmain. That is,

- Use ``svn blame`` and the commit log to find names of people who have recently
  modified the same area of code that you are modifying.
- If you've discussed the change with others, they are good candidates to be
  your reviewers.

.. note:: We do not currently have an xmain equivalent of CODE_OWNERS.txt, but
          we are working on creating one. In case this document is out of date,
          check the root llvm directory for intel_code_owners.map or something
          similar.

Expectations of code reviewers
------------------------------

- It is the job of the code reviewer to **thoroughly** understand the code
  changes under review. Reviewers must understand both the high level design
  and the low level details. Every change in xmain must be given a detailed
  line-by-line code review. A cursory reading of the code is not an adequate
  code review. Code reviewers and code authors are equally responsible for the
  quality of code that gets committed to xmain.

- Reviews should be timely. At this time, we do not have a specific rule for
  how long a review should take. But remember that the code reviewer is usually
  on the critical path for getting code committed. So make code reviews a
  priority!

- For important issues that you find, e.g. correctness or efficiency problems,
  insist that the author either fix the problem or convince you that there is
  no problem. Escalate if necessary!

- Defer to the code author on issues that are purely matters of personal
  preference. By all means make suggestions, but give the author the final say.

Expectations of code authors
----------------------------

- First and foremost, be appreciative of the time people take to review your
  code. We are all busy people.

- Make things as easy as possible on your code reviewers, specifically

   - Partition large pieces of work into small, self-contained change sets.

   - Proofread your code before requesting a code review. It is frustrating for
     code reviewers to have to correct your typos, formatting errors, etc.

   - Accompany each code review request with a good explanation of what you are
     trying to accomplish in the change set, providing any necessary context.
     Well-written unit tests are often the best way to establish context for a
     review since they should illustrate what the change set is trying to
     accomplish.

   - Document your code well, either via source comments or via higher level
     documentation in the llvm/docs area.

   - Respond to code review comments in a timely manner so that reviewers don't
     lose their train of thought.

   - Avoid updating your sources in between code review iterations. That makes
     it difficult for reviewers to do incremental reviews. If you **must**
     update your sources, it is helpful to upload a version of your changes
     that **only** reflects the update with no other changes.

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
testing for high risk change sets.

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

The developer must submit a CQ for any performance regression that requires
follow-up work before the gatekeeper will approve the checkin request.
