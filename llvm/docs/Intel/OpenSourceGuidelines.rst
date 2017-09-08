======================
Open Source Guidelines
======================

.. contents::
   :local:

.. toctree::
   :hidden:

   SPECOpenSourceGuidelines
   OpenSourceDecisionFlow

Introduction
============

This document describes our Xmain and LLORG open sourcing guidelines.

Our proprietary compilers (ICC/ICX) should demonstrate industry leading
performance on all IA platforms (Intel and AMD) against all relevant
compilers. At the same time, open source compilers (GCC/LLVM) should
perform best on IA against other architectures. In order to achieve
these goals, we need to maintain a fine balance of performance and
features between Xmain and LLORG.

There are several benefits of committing our LLVM/clang improvements to open
source, and there are some good reasons to keep our LLVM/clang improvements to
ourselves as illustrated in the following table.

+--------------+--------------------------------+------------------------------+
|              | Proprietary LLVM-based product | Contribution to LLVM open    |
|              |                                | source                       |
+==============+================================+==============================+
| What we do   | All possible performance       | IA enabling and IA specific  |
|              | optimizations, unique SW       | optimizations                |
|              | features                       |                              |
+--------------+--------------------------------+------------------------------+
| Why we do it | (A) Retain performance         | (E) Maximize reach of IA     |
|              |     differentiation for IA only|     enabling and             |
|              |                                |     optimizations to all     |
|              | (B) Retain unique SW features  |     LLVM-based tools         |
|              |     for IA only                |     customers                |
|              |                                |                              |
|              | (C) Create direct developers   | (F) Encourage CSPs to switch |
|              |     influence point            |     from GCC to LLVM to have |
|              |                                |     greater influence, and   |
|              | (D) Demonstrate a significant  |     increase likelihood of   |
|              |     lead in SPEC scores on     |     switching to ICX         |
|              |     Intel processors vs. AMD   |                              |
|              |     to help Intel's bottom line| (G) Driving language /       |
|              |                                |     programming models       |
|              |                                |     (Secondary goal)         |
|              |                                |                              |
|              |                                | (H) Need community influence |
|              |                                |     where needed to meet     |
|              |                                |     primary goals (E), (F),  |
|              |                                |     (G) above                |
+--------------+--------------------------------+------------------------------+

**CPU2017 Compiler Performance Considerations**

In addition to demonstrating industry leading performance, it is also crucial
that we seek and maintain a significant lead in SPEC2017 on Intel Architectures
against AMD, as stated in the table above.

It is also important to improve CPU2017 performance in LLVM Open Source to
achieve goal (E) above, that is, to encourage CSPs to switch from GCC to LLVM.

These two goals can sometimes work against each other. For more guidance,
please see :doc:`CPU2017 Performance Guidelines <SPECOpenSourceGuidelines>`

**General Open Sourcing Considerations**

The decision making process on whether to open source a code change should start
before actual design of a code change. This has several benefits.

- Ensures that code changes land in the right code repository.

- Helps developers understand dependencies early, e.g. by understanding whether
  to use xmain or open source infrastructure.

- Saves development resources.

Open Source Guiding Principles
==============================

1. **DO NOT OPEN SOURCE** platform independent optimizations which raise all
   boats.

2. **DO NOT OPEN SOURCE** unique SW features creating stickiness to IA.

3. **DO NOT OPEN SOURCE** features providing a significant differentiation vs.
   IA competitors.

4. **OPEN SOURCE** code required for enabling and optimizing for IA if
   corresponding ISA is public.

5. **DO NOT OPEN SOURCE** features differentiating product vs. other compilers.

6. **OPEN SOURCE** code which requires high maintenance effort relative to value
   as an IA-only product.

7. **OPEN SOURCE** language and parallel programming model features, except
   performance.

8. **DO NOT OPEN SOURCE** changes that hurt our competitive standing in SPEC
   vs AMD.

Decision Making Process
=======================

- A high level summary flow of decision making and testing is illustrated
  here : :doc:`Open Source Decision Flow <OpenSourceDecisionFlow>`

- Additional details on open sourcing guidelines for individual components
  can be found here : :ref:`Open Source Guide <open-source-guide>`

- If you cannot identify the Code Categories your code belongs to, or you would
  like to appeal for an exception, send mail to
  `David Kreitzer <mailto:david.l.kreitzer@intel.com>`_ and/or
  `Alice Chan <mailto:alice.s.chan@intel.com>`_.

.. _open-source-guide:

Open Source Guide
=================

+----------------------+--------------+-----------+----------------------------+
| Code Categories      | Open Source? | Last Edit | Rationale                  |
+======================+==============+===========+============================+
| All changes that     | Maybe        | 8/31/2017 | See  :doc:`CPU2017         |
| affect SPEC          |              |           | Performance Guidelines     |
| performance          |              |           | <SPECOpenSourceGuidelines>`|
+----------------------+--------------+-----------+----------------------------+
| Interprocedural &    | No           | 6/9/2015  | Raises all boats, don't    |
| Loop Optimizations   |              |           | want competitive compilers |
|                      |              |           | or architectures to        |
|                      |              |           | benefit, based on guiding  |
|                      |              |           | principle #1.              |
+----------------------+--------------+-----------+----------------------------+
| Bi-Endian            | No           | 6/9/2015  | Don't want to enable non-IA|
|                      |              |           | and don't want to lose     |
|                      |              |           | Cisco stickiness to Intel  |
|                      |              |           | tools, based on guiding    |
|                      |              |           | principle #2.              |
+----------------------+--------------+-----------+----------------------------+
| General masking      | Yes          | 6/9/2015  | Features required for IA   |
| infrastructure       |              |           | enabling, based on guiding |
|                      |              |           | principle #4.              |
+----------------------+--------------+-----------+----------------------------+
| Fortran FE           | No           | 6/9/2015  | Providing a significant    |
|                      |              |           | differentiation vs some    |
|                      |              |           | IA competitors, based on   |
|                      |              |           | guiding principle #3.      |
+----------------------+--------------+-----------+----------------------------+
| Fortran RTL          | No           | 6/9/2015  | Market leading product,    |
|                      |              |           | compatibility with other   |
|                      |              |           | compilers or runtime is not|
|                      |              |           | important, based on        |
|                      |              |           | principle #5.              |
+----------------------+--------------+-----------+----------------------------+
| A fix for a          | Yes          | 6/9/2015  | All regressions on IA must |
| regression happened  |              |           | be fixed, based on         |
| in open source       |              |           | principle #4.              |
| compiler             |              |           |                            |
+----------------------+--------------+-----------+----------------------------+
| MSVC compatibility   | Yes          | 6/9/2015  | Benefits open source for   |
|                      |              |           | IA and not of much help for|
|                      |              |           | IA competitors to take     |
|                      |              |           | customers away from IA,    |
|                      |              |           | saves maintenance efforts, |
|                      |              |           | based on principle #6. We  |
|                      |              |           | will have MSFT proprietary |
|                      |              |           | PDB information as a       |
|                      |              |           | differentiator.            |
+----------------------+--------------+-----------+----------------------------+
| GCC compatibility    | Yes          | 6/9/2015  | Helps everyone equally,    |
|                      |              |           | doesn't hurt core          |
|                      |              |           | differentiation, saves     |
|                      |              |           | resources on maintenance,  |
|                      |              |           | builds community goodwill  |
|                      |              |           | if open sourced, based on  |
|                      |              |           | principle #6.              |
+----------------------+--------------+-----------+----------------------------+
| Cross OS             | No           | 6/9/2015  | Differentiator in our      |
| compatibility options|              |           | proprietary compiler       |
| & features           |              |           | products, based on         |
|                      |              |           | principle #5.              |
+----------------------+--------------+-----------+----------------------------+
| Code size            | Yes          | 6/9/2015  | Code size is important for |
| optimizations        |              |           | IA, both in MCU, and       |
|                      |              |           | Android, LLVM is behind    |
|                      |              |           | compared to GCC, based on  |
|                      |              |           | principle #4. This will    |
|                      |              |           | help all llvm on IA and we |
|                      |              |           | will still have the only   |
|                      |              |           | MCU compiler.              |
+----------------------+--------------+-----------+----------------------------+
| Vectorizer Code      | Yes          | 6/9/2015  | Helps everyone equally,    |
| Generation           |              |           | doesn't hurt core          |
|                      |              |           | differentiation without    |
|                      |              |           | advanced analysis          |
|                      |              |           | technique, saves resources |
|                      |              |           | on maintenance, builds     |
|                      |              |           | community goodwill if open |
|                      |              |           | sourced (e.g. AVX512,      |
|                      |              |           | SVML), based on principle  |
|                      |              |           | #6.                        |
+----------------------+--------------+-----------+----------------------------+
| Vectorizer Advanced  | No           | 4/26/2016 | Raises all boats, don't    |
| Analysis &           |              |           | want competitive compilers |
| Optimization         |              |           | or architectures to        |
|                      |              |           | benefit, based on guiding  |
|                      |              |           | principle #1.              |
|                      |              |           |                            |
|                      |              |           | If analysis & optimization |
|                      |              |           | is unique to proprietary   |
|                      |              |           | Intel Compilers and can    |
|                      |              |           | benefit competition, do not|
|                      |              |           | open source, based on      |
|                      |              |           | principles #1, #2, and #5. |
+----------------------+--------------+-----------+----------------------------+
| OpenMP, CilkPlus FE  | Yes          | 6/9/2015  | Support Intel Parallel     |
| and RTL              |              |           | initiatives, deliberately  |
|                      |              |           | raising all boats, based on|
|                      |              |           | principle #7.              |
+----------------------+--------------+-----------+----------------------------+
| OpenMP, CilkPlus BE  | No           | 6/9/2015  | BE outlining proprietary   |
|                      |              |           | based on principle #1.     |
+----------------------+--------------+-----------+----------------------------+
| GEN Code Generation  | Yes          | 6/9/2015  | Already open source by VPG,|
|                      |              |           | based on principle #4.     |
+----------------------+--------------+-----------+----------------------------+
| OpenMP SIMD          | Yes          | 4/26/2016 | Helps everyone equally     |
| functionality        |              |           | (typically helps IA more), |
| (Generate Vector Code|              |           | doesn't hurt core          |
| for OpenMP SIMD)     |              |           | differentiation without    |
|                      |              |           | advanced analysis          |
|                      |              |           | technique, saves resources |
|                      |              |           | on maintenance, builds     |
|                      |              |           | community goodwill if open |
|                      |              |           | sourced, based on principle|
|                      |              |           | #7.                        |
+----------------------+--------------+-----------+----------------------------+
| Basic                | Yes          | 4/26/2016 | Helps everyone equally,    |
| Auto-Vectorization   |              |           | doesn't hurt core          |
| (catch up with GCC   |              |           | differentiation without    |
| vectorizer)          |              |           | advanced analysis          |
|                      |              |           | technique, saves resources |
|                      |              |           | on maintenance, builds     |
|                      |              |           | community goodwill if open |
|                      |              |           | sourced, based on principle|
|                      |              |           | #6.                        |
+----------------------+--------------+-----------+----------------------------+
