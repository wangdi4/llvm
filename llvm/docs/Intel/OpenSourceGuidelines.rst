======================
Open Source Guidelines
======================

.. contents::
   :local:

The Intel LLVM development process documentation is currently under review. If
anything looks wrong to you, please contact `David Kreitzer
<mailto:david.l.kreitzer@intel.com>`_.

Introduction
============

This document describes the process for deciding whether a code change should
be targeted for LLVM open source or kept Intel proprietary in xmain.

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
| Why we do it | (A) Retain performance         | (D) Maximize reach of IA     |
|              |     differentiation for IA only|     enabling and             |
|              |                                |     optimizations to all     |
|              | (B) Retain unique SW features  |     LLVM-based tools         |
|              |     for IA only                |     customers.               |
|              |                                |                              |
|              | (C) Create direct developers   | (E) Driving language/        |
|              |     influence point            |     programming models       |
|              |                                |     (Secondary goal) Need    |
|              |                                |     community influence where|
|              |                                |     needed to meet primary   |
|              |                                |     goals (D), (E) above     |
+--------------+--------------------------------+------------------------------+

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

Decision Making Process
=======================

- Refer to the :ref:`Open Source Guide <open-source-guide>` to find out whether
  your code should be open sourced.

- Submit the code category your code falls into and corresponding guidance to
  your manager to get decision approved.

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
