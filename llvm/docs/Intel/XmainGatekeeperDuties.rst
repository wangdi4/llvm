=======================
Xmain Gatekeeper Duties
=======================

.. contents::
   :local:

Introduction
============

This document describes the responsibilities and expectations for the xmain
gatekeeper.

Responsibilities of the Xmain Gatekeeper
========================================
The xmain gatekeeper is charged with reviewing all xmain checkin requests,
reviewing these requests for adherence to the xmain checkin process, and for
coordinating checkins with the goal of keeping xmain quality and performance
stable or preferably on a positive trend.

#. The gatekeeper should be familiar with the
   :ref:`xmain gatekeeping process <xmain_gatekeeping>`. The checklist on that
   page can serve as a reminder of things to watch for when reviewing a
   request. In particular, it is a good idea to examine all new modifications
   to community files to ensure that the customizations are appropriate and
   properly marked.

#. The gatekeeper should check for and respond to new gatekeeping requests
   several times per day. That includes checkin requests made both through
   email and through gerrit.

#. The following queries can be used to check for pending checkin requests:
   `git-amr-2 <https://git-amr-2.devtools.intel.com/gerrit/#/q/status:open+AND+reviewer:%22xmain+gatekeeper+%253Cxmain.gatekeeper%2540intel.com%253E%22+label:Code-Review%253D%252B1+-label:Code-Review%253D%252B2+label:Verified%253D%252B1>`_
   and
   `git-amr-1 <https://git-amr-1.devtools.intel.com/gerrit/#/q/status:open+AND+reviewer:%22xmain+gatekeeper+%253Cxmain.gatekeeper%2540intel.com%253E%22+label:Code-Review%253D%252B1+-label:Code-Review%253D%252B2+label:Verified%253D%252B1>`_.

Desirable Qualities of an Xmain Gatekeeper
==========================================

#. Disciplined compiler engineer
#. Knowledge of much of the compiler source base
#. Senior and preferably known to much of the compiler team
#. Ability to assess checkin risk
#. Ability not to be pressured into allowing/compromising checkin requirements

Xmain Gatekeepers Assignments
=============================

`CE DevOps team <mailto:CE.DevOps@intel.com>`_ manages xmain gatekeeper
duties assignments. The current xmain gatekeeper assignments can be seen
at any time on the `CE DevOps team wiki <https://wiki.ith.intel.com/display/ITSCompilersDevOps/Pulldown+coordinators+and+gatekeepers>`_.
