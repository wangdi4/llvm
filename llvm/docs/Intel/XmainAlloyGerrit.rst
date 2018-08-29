=============================================
Xmain Integrated Gerrit Alloy Testing Process
=============================================

.. contents::
   :local:
.. role:: red

Introduction
------------

This document describes how to use Integrated Gerrit Alloy testing by just
selecting reviewers. Currently it supports the reviewers mentioned
below, which either correspond to a specific alloy load file, or need user
input to select them. A user simply adds one of these reviewers in the gerrit
review window, and it automatically runs the corresponding alloy testing, and
post the results to Gerrit. In this process, the user does not need to know
details about the infrastructure or alloy command line machine usage etc.

Supported Load File
-------------------

::

        xmain_checkin_sanity: Run "alloy run -f xmain_checkin_sanity"
        xmain_checkin: Run "alloy run -f xmain_checkin"
        zperf_checkin_xmain: Run "alloy run -f zperf_checkin_xmain"
        xmain_alloy: Run the generic testing as selected by users.

Supported Xmain Repo
--------------------

::

        llvm
        clang
        compiler-rt
        openmp
        xdev
        ocl-rt
        ocl-headers
        ocl-icd
        ocl-cclang
        ocl-spirv-headers
        ocl-externals
        xtoolsup
        xalloy
        icsconfig
        icsproject
        sycl-xmain
        llvm_opencl-icd-loader_worldread
        opencl_headers_worldread
        llvm_spirv_translator_worldread


Reviewer ( ``xmain_checkin_sanity, xmain_checkin, zperf_checkin_xmain`` )
-------------------------------------------------------------------------

Once a user select this reviewer in Gerrit, it will immediately checkout an
xmain workspace and apply the user's patch on top of it.

If the patch fails to apply, the user should get a Gerrit post like this:

::

        xmain_checkin
        May 8 5:52 PM
        ↩
        Patch Set 8: Verified-1
        Gerrit Patch failed to apply:git pull --no-edit ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm refs/changes/97/125497/8

If the patch is successfully applied, the user should get a Gerrit post like
this:

::

        xmain_checkin
        Apr 20 4:42 PM
        ↩
        Patch Set 1:
        !!gralloyid!!lab_iclt!!1!!
        Alloy run xmain_checkin 0 started.
        Review Log: http://dss-sc.intel.com/review/lab_iclt/fresh/1250641.0/review.log
        To abort this run reply to this gerrit post with #abort# keyword.

To abort an Alloy test, one can just reply to this Gerrit post with:

::

        > !!gralloyid!!lab_icla!!2!!
        > Alloy run -f xmain_checkin -f zperf_checkin_xmain 0 started.
        > Review Log: http://dss-sc.intel.com/review/lab_icla/fresh/1187772.0/review.log
        > To abort this run reply to this gerrit post with #abort# keyword.
        #abort#

Users can track the progress of an alloy run by clicking on the Review Log link,
such as:

::

        Review Log: http://dss-sc.intel.com/review/lab_iclt/fresh/1250641.0/review.log

If testing fails due to a bad patch, the user should get a Gerrit post like
this:

::

        xmain_checkin
        Apr 21 1:11 AM
        ↩
        Patch Set 2: Verified-1
        !!gralloyid!!lab_iclt!!2!!
        Alloy Run Failed:alloy run -f xmain_checkin
        All fail log: http://dss-sc.intel.com/review/lab_iclt/fresh/1250642.0/all_fail.log
        Fail log: http://dss-sc.intel.com/review/lab_iclt/fresh/1250642.0/fail.log
        Status log: http://dss-sc.intel.com/review/lab_iclt/fresh/1250642.0/status.log
        Zperf BT log: http://dss-sc.intel.com/review/lab_iclt/fresh/1250642.0/zperf_bt_rpt.log
        Review Log: http://dss-sc.intel.com/review/lab_iclt/fresh/1250642.0/review.log
        Alloy Triage log: http://dss-sc.intel.com/review/lab_iclt/fresh/1250642.0/alloy_triage.log
        Reply to this gerrit post with #restartfail# keyword to retest failed alloy run

If testing hits problem due to an alloy infrastructure problem, the user should
get a Gerrit post like this:

::

        xmain_checkin
        Apr 23 9:14 PM
        ↩
        Patch Set 2: Verified-1 (-1 if issue with user code otherwise no markup for verified field)
        !!gralloyid!!lab_iclt!!2!!
        Alloy Run Failed:alloy run -f xmain_checkin
        Soft Problem log: http://dss-sc.intel.com/review/lab_iclt/fresh/1246862.0/soft-problem.log
        All fail log: http://dss-sc.intel.com/review/lab_iclt/fresh/1246862.0/all_fail.log
        Fail log: http://dss-sc.intel.com/review/lab_iclt/fresh/1246862.0/fail.log
        Problem log: http://dss-sc.intel.com/review/lab_iclt/fresh/1246862.0/problem.log
        Status log: http://dss-sc.intel.com/review/lab_iclt/fresh/1246862.0/status.log
        Zperf BT log: http://dss-sc.intel.com/review/lab_iclt/fresh/1246862.0/zperf_bt_rpt.log
        Review Log: http://dss-sc.intel.com/review/lab_iclt/fresh/1246862.0/review.log
        Alloy Triage log: http://dss-sc.intel.com/review/lab_iclt/fresh/1246862.0/alloy_triage.log
        Reply to this gerrit post with #restartfail# keyword to restart problematic run. OR #restartscratch# keyword to restart all over again ( NOT recommended )

If testing gets finished successfully, with no failure, the user should get
a Gerrit post like this:

::

        xmain_checkin
        Apr 23 6:39 AM
        ↩
        Patch Set 1: Verified+1
        !!gralloyid!!lab_iclt!!1!!
        Alloy Run Success:alloy run -f xmain_checkin
        All fail log: http://dss-sc.intel.com/review/lab_iclt/restartfail/1249871.1/all_fail.log
        Status log: http://dss-sc.intel.com/review/lab_iclt/restartfail/1249871.1/status.log
        Zperf BT log: http://dss-sc.intel.com/review/lab_iclt/restartfail/1249871.1/zperf_bt_rpt.log
        Review Log: http://dss-sc.intel.com/review/lab_iclt/restartfail/1249871.1/review.log

If alloy testing is already running/finished, and user uploads a new patch, the
user should get a Gerrit post like this:

::

        xmain_checkin
        Apr 23 10:01 AM
        ↩
        Patch Set 2:
        !!gralloyid!!lab_iclt!!2!!
        Patch 1 is already run: http://dss-sc.intel.com/review/lab_iclt/fresh/1250641.0/review.log
        To run alloy with patch 2 , reply to this gerrit post with #run#

Reviewer ( ``xmain_alloy`` )
----------------------------

This is most flexible reviewer in terms of selecting load files. Remember,
flexibility comes at a cost. There is no error checking due to its limited i/o
capability. If you make a typo in load files' names or syntax it will simply
error out. It is NOT recommended to use this reviewer unless you absolutely
need it. As soon as a user select ``xmain_alloy`` reviewer, it should
immediately post a message in Gerrit like this.

::

        xmain_alloy
        6:27 AM
        ↩
        Patch Set 1:
        !!gralloyid!!lab_icla!!1!!
        To choose a custom load file run, Reply this gerrit post with comma separated loadfile keyword
        Example:
        #custom#xmain_checkin,zperf_checkin_xmain# OR 
        #custom#sycl_checkin#sycl#

**Generic**

The user can reply to this Gerrit post with the desired alloy load file name
( or a comma separated list if more than one )

::

        > !!gralloyid!!lab_icla!!2!!
        >
        > To choose a custom load file run, Reply this gerrit post with comma
        > separated loadfile keyword
        > Example:
        > #custom#xmain_checkin,zperf_checkin_xmain# OR
        > #custom#sycl_checkin#sycl#

        #custom#xmain_checkin,zperf_checkin_xmain#

After replying to this message, the user should expect a Gerrit post like this:

::

        xmain_alloy
        3:35 PM
        ↩
        Patch Set 2:
        !!gralloyid!!lab_icla!!2!!
        Alloy run -f xmain_checkin -f zperf_checkin_xmain 0 started.
        Review Log: http://dss-sc.intel.com/review/lab_icla/fresh/1187772.0/review.log
        To abort this run reply this gerrit post with #abort# keyword.

The user can provide the reference workspace name as # separated third field as
explained in the `Sycl`_ example.

**Opencl**

The user can reply to this Gerrit post with the desired alloy load file name
( or a comma separated list if more than one )

::

        > !!gralloyid!!lab_icla!!2!!
        > 
        > To choose a custom load file run, Reply this gerrit post with comma
        > separated loadfile keyword
        > Example:
        > #custom#xmain_checkin,zperf_checkin_xmain# OR
        > #custom#sycl_checkin#sycl#

        #custom#ocl_checkin#

After replying to this message, the user should expect a Gerrit post like this:

::

        xmain_alloy
        Aug 5 12:41 AM
        ↩
        Patch Set 2:
        !!gralloyid!!lab_icla!!2!!
        Alloy run -f ocl_checkin 0 started.
        Review Log: http://dss-sc.intel.com/review/lab_icla/fresh/1422282.0/review.log
        To abort this run reply to this gerrit post with #abort# keyword.

.. _Sycl:

**Sycl**

The user can reply to this Gerrit post with the desired alloy load file name
( or a comma separated list if more than one ). It also needs # separated
worspace name as the third field ( ``#sycl#`` ). ``#sycl#`` tells the
auto-reviewer to use the current ``sycl head`` as the reference compiler for
alloy testing, instead of ``xmain``. The workspace to be tested will contain
the patch, applied on top of ``sycl head``.

::

        > !!gralloyid!!lab_icla!!1!!
        > 
        > To choose a custom load file run, Reply this gerrit post with comma
        > separated loadfile keyword
        > Example:
        > #custom#xmain_checkin,zperf_checkin_xmain# OR
        > #custom#sycl_checkin#sycl#

        #custom#sycl_checkin#sycl#

After replying to this message, the user should expect a Gerrit post like this:

::

        xmain_alloy
        6:43 AM
        ↩
        Patch Set 1:
        !!gralloyid!!lab_icla!!1!!
        Alloy run -f sycl_checkin 0 started.
        Review Log: http://dss-sc.intel.com/review/lab_icla/fresh/1430071.0/review.log
        To abort this run reply to this gerrit post with #abort# keyword.

Rest of the functionality is same as the regular reviewer ( ``xmain_checkin``,
``xmain_checkin_sanity``, ``zperf_checkin_xmain`` ) discussed above.

.. note::

      - DO NOT remove anything from Gerrit reply message as it contains unique
        information to identify the workspace patch-set.
      - Sometime patch change resulting from rebase is not detected by Gerrit
        auto reviewers. If it happens and you really want to run alloy on the
        rebased patch, just remove and re-add Gerrit auto reviewers.
      - If Gerrit auto reviewers ( ``xmain_checkin_sanity``, ``xmain_checkin``,
        ``zperf_checkin_xmain``, ``xmain_alloy`` ) removed, it will not detect
        any new patches.

Fallback
--------

If for any reason Alloy/Gerrit infrastructure does not fit your need you can
always run alloy manually, it should create gerrit.log file under alloy/results
directory. Post its contents to the Gerrit review as comment, all links are
clickable for anyone to access the results inside Gerrit.

::

        $ cat gerrit.log
        All fail log:      http://dss-sc.intel.com/problem_dir/lab_icltI68628812lab_26809-1/all_fail.log
        Alloy command txt: http://dss-sc.intel.com/problem_dir/lab_icltI68628812lab_26809-1/alloy_command.txt
        Fail log:          http://dss-sc.intel.com/problem_dir/lab_icltI68628812lab_26809-1/fail.log
        Status log:        http://dss-sc.intel.com/problem_dir/lab_icltI68628812lab_26809-1/status.log
        Stop suite log:    http://dss-sc.intel.com/problem_dir/lab_icltI68628812lab_26809-1/stop_suite.log
        Warning log:       http://dss-sc.intel.com/problem_dir/lab_icltI68628812lab_26809-1/warning.log
        Zperf bt rpt log:  http://dss-sc.intel.com/problem_dir/lab_icltI68628812lab_26809-1/zperf_bt_rpt.log

When using this option, please add a comment indicating the patch set on which
the tests were run, e.g. "Tests run on Patch Set 5". The information is helpful
to the gatekeeper.

Troubleshooting Tips
--------------------

Reviewer added but did not get any acknowledgement as Gerrit post.
New patch uploaded but no Gerrit post from alloy reviewer.

- Remove the xmain reviewer and re-add it. The issue could be Gerrit or mail
  server IT downtime.

No update in review.log for an extended period of time.

- Abort current alloy run by replying ``#abort#``, and start from scratch by
  replying ``#restartscratch#``.

Restart alloy gerrit run after an infrastructure failure(picl/crun/copylist etc)

- Clean all alloy gerrit runs for the patch in question by replying ``#clean#``.
  It will abort ongoing alloy run and remove workspace, recreate workspace from
  latest head and run corresponding alloy run.

To reproduce exact workspace used by Alloy Gerrit, look for reproducer link in
Gerrit post.

::

        Manifest: http://dss-sc.intel.com/review/lab_iclt/1315035.xml
        Reproduce WS: http://dss-sc.intel.com/review/lab_iclt/1315035.reproduce.txt

Any other issue with Alloy Gerrit infrastructure: Contact
sunil.k.pandey@intel.com
