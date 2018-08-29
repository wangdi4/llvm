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

#. This `gerrit query <https://git-amr-2.devtools.intel.com/gerrit/#/q/status:open+reviewer:%22xmain+gatekeeper+%253Cxmain.gatekeeper%2540intel.com%253E%22>`_
   is an easy way to check for pending checkin requests. The CR column shows
   whether gatekeeper review is still needed.

Desirable Qualities of an Xmain Gatekeeper
==========================================

#. Disciplined compiler engineer
#. Knowledge of much of the compiler source base
#. Senior and preferably known to much of the compiler team
#. Ability to assess checkin risk
#. Ability not to be pressured into allowing/compromising checkin requirements

Historical Xmain Gatekeepers
============================

2018
----
- Q4: TBD
- Q3: Zia Ansari, Hideki Saito (coverage during Dave's sabbatical)
- Q2: Dave Kreitzer
- Q1: Dave Kreitzer

Prior Years
-----------
- Kevin B Smith was the gatekeeper from the initiation of the xmain project
  until his retirement in June 2016.
- Dave Kreitzer took over from Kevin and was the gatekeeper through 2017.
