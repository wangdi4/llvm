==============================================
Dealing With The Current LIT Failures In Xmain
==============================================

.. contents::
   :local:

Introduction
============

Xmain compiler development team has a goal to have 100% passing rate
for the compiler in-tree LIT tests, since they are considered testing
basic functionality that must always work. Sometimes it happens that
a LIT failure sneaks into xmain trunk unnoticed and then discovered
later by someone running testing for unrelated changes.
This document describes the process for dealing with such situations.
It mostly targets ``xmain gatekeeper`` team, as they are the main
coordinators of this process.

LIT Failure Notification
========================

Developers are responsible for notifying ``xmain gatekeeper`` about
LIT failures unrelated to their changes either via `Gerrit <https://git-amr-2.devtools.intel.com/gerrit>`_
or via :ref:`mail <xmain-lit-notification>`.
When an ``xmain gatekeeper`` acts as a developer, one should follow
the same notification process.

Actions For Resolving LIT Failures
==================================

.. |br| raw:: html

   <br />

Upon receiving the notification:

#. The ``xmain gatekeeper`` asks the developer who notified them of the problem
   to submit a new JIRA following the pattern below. If more than one developer
   reports the same problem, the gatekeeper should choose only one.

   * *Project*: **Compiler-LLVM**
   * *Issue Type*: **Bug**
   * *Summary*: **xmain LIT failure: <one test name>**
     |br|
     Please strictly follow this pattern, so that anyone can look up
     the currently unresolved LIT failures in JIRA.
   * *Affects Version/s*: **xmain**
   * *Component/s*: **<Test's component or Unknown>**
   * *Operating System/s*: **Linux/Windows**
   * *Exposure*: **1-Critical**
   * *Submitter Role*: **Developer**
   * *How Found*: **Testing**
   * *Problem Classification*: **Stability/Crash**
   * *Is this a Regression?*: **Yes**
   * *Assignee*: **Developer assigns to self**
   * *Description*: **Describe all failing tests (e.g. text from fail.log)
     and any details you find relevant. Attach files/links to the alloy's
     gerrit.log file/response (or otherwise specify git revision of the test's
     repository where the fail was seen).**
   * **TBD** Integrate creation of JIRA for LIT failures with :doc:`AlloyGerrit <XmainAlloyGerrit>`
     to save developers' time for filing JIRA fields.

#. When test failures are caused by a problem in the testing infrastructure
   and are not limited to a specific set of failing tests, ``xmain gatekeeper``
   should contact `the compiler architect and the tools team
   <mailto:andrew.kaylor@intel.com;IACompilerToolsandInfrastructure@intel.com>`_
   to determine a temporary course of action until the problem can be resolved.

   * For example, timing out LIT tests may mean that the testing is set up
     incorrectly, and disabling the currently failing LIT tests does not
     mean that more tests will not fail shortly after.

#. ``xmain gatekeeper`` requests from the developer to disable
   the failing LIT tests(s):

   * The LIT failure must always be fixed in a standalone commit,
     reviewed and approved solely by the ``xmain gatekeeper``.
   * ``xmain_lit`` or local LIT testing are both approved ways
     for testing the changes that disable the LIT failure.
   * In most of the cases it should be enough to disable the LIT test(s)
     with `"XFAIL: *"`. If the failure is caused by a hanging test,
     then it should be disabled using appropriate `"UNSUPPORTED/REQUIRES: ..."`
     rules, e.g. `"UNSUPPORTED: linux, windows"`.
   * Tests that fail intermittently should not be disabled using `"XFAIL: *"`,
     because they may then be reported as unexpected passes. Please use
     appropriate `"UNSUPPORTED/REQUIRES: ..."` rules.

#. After the LIT test is disabled in ``xmain``, the developer should
   *Update Exposure* of the JIRA to **2 - High**, and find
   the appropriate *Assignee* for resolving the underlying issue
   and re-enabling the LIT test(s):

   * If it is easy to identify the commit that broke the LIT test(s),
     then the JIRA must be assigned to the author of the commit.
   * Otherwise, identify the most related component,
     and leave *Assignee* field **Automatic**.

#. Ignore this particular LIT failure in all other xmain checkin
   requests:

   * ``xmain gatekeeper`` may always request a developer to run
     ``xmain_lit`` or local LIT testing with one's changes rebased
     on top of the commit disabling the failing LIT test(s)
     (note that :doc:`AlloyGerrit <XmainAlloyGerrit>` always
     rebases changes on top of xmain head for clean alloy runs,
     so an explicit rebase is not necessary).
   * ``xmain gatekeeper`` may approve commits with known LIT failures,
     for which JIRA(s) has been already submitted, without requesting
     any further testing.
