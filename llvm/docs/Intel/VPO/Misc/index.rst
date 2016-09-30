====================================================================
Optimal code gen for references with truncated induction canon exprs
====================================================================

:Author: Satish Guggilla
:Date: 2016-07-26

Overview
========

Consider the following snippet of code:

**Example code**

.. code-block:: c

  int sarr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  int arr[100];
  void foo() {
    int i;
    for (i = 0; i < 100; i++)
       arr[i] = sarr[i + 1 & 3];
  }

The HIR before and after vectorization is shown below. As can be seen, the type
of i1 in `{al:4}(@sarr)[0][i1 + 1]` is a 2-bit integer. The canon expr `i1 + 1` is
marked as linear and this causes the reference to be treated as unit stride causing
bad code to be generated.

**Handled loop before vector code generation:**

.. code:: python

  <13>      + Ztt: No
  <13>      + NumExits: 1
  <13>      + Innermost: Yes
  <13>      + NSW: Yes
  <13>      + LiveIn symbases:
  <13>      + LiveOut symbases:
  <13>      + Loop metadata: No
  <13>      + DO i64 i1 = 0, 99, 1   <DO_LOOP>
  <5>       |   %0 = {al:4}(@sarr)[0][i1 + 1];
  <5>       |   <LVAL-REG> NON-LINEAR i32 %0 {sb:7}
  <5>       |   <RVAL-REG> {al:4}(LINEAR [10 x i32]* @sarr)[i64 0][LINEAR zext.i2.i64(i1 + 1)] !tbaa !3 {sb:12}
  <5>       |      <BLOB> LINEAR [10 x i32]* @sarr {sb:8}
  <5>       |
  <7>       |   {al:4}(@arr)[0][i1] = %0;
  <7>       |   <LVAL-REG> {al:4}(LINEAR [100 x i32]* @arr)[i64 0][LINEAR i64 i1] !tbaa !8 {sb:13}
  <7>       |      <BLOB> LINEAR [100 x i32]* @arr {sb:10}
  <7>       |   <RVAL-REG> NON-LINEAR i32 %0 {sb:7}
  <7>       |
  <13>      + END LOOP

**Handled loop after vector code generation:**

.. code:: python

  <15>      + Ztt: No
  <15>      + NumExits: 1
  <15>      + Innermost: Yes
  <15>      + NSW: Yes
  <15>      + LiveIn symbases:
  <15>      + LiveOut symbases:
  <15>      + Loop metadata: !llvm.loop <0xa4b5b08>
  <15>      + DO i64 i1 = 0, 99, 4   <DO_LOOP>
  <16>      |   %.vec = {al:4}(<4 x i32>*)(@sarr)[0][i1 + 1];
  <16>      |   <LVAL-REG> NON-LINEAR <4 x i32> %.vec {sb:14}
  <16>      |   <RVAL-REG> {al:4}(LINEAR bitcast.[10 x i32]*.<4 x i32>*(@sarr))[i64 0][LINEAR zext.i2.i64(i1 + 1)] !tbaa !3 {sb:12}
  <16>      |      <BLOB> LINEAR [10 x i32]* @sarr {sb:8}
  <16>      |
  <17>      |   {al:4}(<4 x i32>*)(@arr)[0][i1] = %.vec;
  <17>      |   <LVAL-REG> {al:4}(LINEAR bitcast.[100 x i32]*.<4 x i32>*(@arr))[i64 0][LINEAR i64 i1] !tbaa !8 {sb:13}
  <17>      |      <BLOB> LINEAR [100 x i32]* @arr {sb:10}
  <17>      |   <RVAL-REG> NON-LINEAR <4 x i32> %.vec {sb:14}
  <17>      |
  <15>      + END LOOP


In order to fix this issue and generate optimal code for **sarr[0,3,2,1]** in the vector loop, the following
needs to be done:

  1) Make it a gather checking for zext/sext canon exprs
  2) Recognize loop-invariance of the gather index patterns and make it constant index vector
  3) Move the gather outside of the loop
  4) Get it optimized via codegen-prepare (dependent on 2, but not 3) and reflect it in
     the vectorizer cost model.

