; Test to verify that structptreq search loop idiom is recognized on this loop, and correct vector code with peel loop and alignment is generated.

; Check if search loop was recognized
; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation  -hir-vec-dir-insert -hir-vplan-vec -debug-only=vplan-idioms -disable-output < %s 2>&1 | FileCheck --check-prefix=WAS-RECOGNIZED-CHECK %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec" -xmain-opt-level=3 -debug-only=vplan-idioms -disable-output < %s 2>&1 | FileCheck --check-prefix=WAS-RECOGNIZED-CHECK %s


; Check final vectorized codegen
; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation \
; RUN:      -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -enable-intel-advanced-opts -disable-output < %s 2>&1 | FileCheck --check-prefix=CG-CHECK %s


; RUN: opt -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-last-value-computation,hir-vec-dir-insert,hir-vplan-vec"\
; RUN:     -print-after=hir-vplan-vec \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -enable-intel-advanced-opts -disable-output < %s 2>&1 | FileCheck --check-prefix=CG-CHECK %s

; REQUIRES: asserts

; HIR before VPlan-
;  BEGIN REGION { modified }
;        + DO i1 = 0, %n + -1, 1   <DO_MULTI_EXIT_LOOP>
;        |   if ((%t4)[i1] == &((%t1)[0]))
;        |   {
;        |      %gep = &((%t4)[i1]);
;        |      goto found;
;        |   }
;        + END LOOP
;  END REGION

; WAS-RECOGNIZED-CHECK: PtrEq loop has PeelArray:(%t4)[i1]
; WAS-RECOGNIZED-CHECK: Search loop was recognized.

; -----------------------------------------------------------------------------
; After VPlan
; CG-CHECK:           BEGIN REGION { modified }
; CG-CHECK-NEXT:           %arr.base.cast = ptrtoint.%struct2**.i64(&((%t4)[0]));
; CG-CHECK-NEXT:           %alignment = %arr.base.cast  &  31;
; CG-CHECK-NEXT:           %peel.factor = 32  -  %alignment;
; CG-CHECK-NEXT:           %peel.factor1 = %peel.factor  >>  3;
; CG-CHECK-NEXT:           %peel.factor1 = (%n <=u %peel.factor1) ? %n : %peel.factor1;
; CG-CHECK-NEXT:           if (%peel.factor1 != 0)
; CG-CHECK-NEXT:           {
; CG-CHECK-NEXT:              + DO i1 = 0, %peel.factor1 + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 31>
; CG-CHECK-NEXT:              |   if ((%t4)[i1] == &((%t1)[0]))
; CG-CHECK-NEXT:              |   {
; CG-CHECK-NEXT:              |      %gep = &((%t4)[i1]);
; CG-CHECK-NEXT:              |      goto found;
; CG-CHECK-NEXT:              |   }
; CG-CHECK-NEXT:              + END LOOP
; CG-CHECK-NEXT:           }
; CG-CHECK-NEXT:           if (%peel.factor1 <u %n)
; CG-CHECK-NEXT:           {
; CG-CHECK-NEXT:              %tgu = (%n + -1 * %peel.factor1)/u4;
; CG-CHECK-NEXT:              if (0 <u 4 * %tgu)
; CG-CHECK-NEXT:              {
; CG-CHECK-NEXT:                 + DO i1 = 0, 4 * %tgu + -1, 4   <DO_MULTI_EXIT_LOOP> <auto-vectorized> <nounroll> <novectorize>
; CG-CHECK-NEXT:                 |   %wide.cmp. = (<4 x %struct2*>*)(%t4)[i1 + %peel.factor1] == &((<4 x %struct2*>)(%t1)[0]);
; CG-CHECK-NEXT:                 |   %intmask = bitcast.<4 x i1>.i4(%wide.cmp.);
; CG-CHECK-NEXT:                 |   if (%intmask != 0)
; CG-CHECK-NEXT:                 |   {
; CG-CHECK-NEXT:                 |      %.vec = %wide.cmp.  ^  -1;
; CG-CHECK-NEXT:                 |      %bsfintmask = bitcast.<4 x i1>.i4(%wide.cmp.);
; CG-CHECK-NEXT:                 |      %bsf = @llvm.cttz.i4(%bsfintmask,  1);
; CG-CHECK-NEXT:                 |      %cast = zext.i4.i64(%bsf);
; CG-CHECK-NEXT:                 |      %gep = &((%t4)[i1 + %peel.factor1 + %cast]);
; CG-CHECK-NEXT:                 |      goto found;
; CG-CHECK-NEXT:                 |   }
; CG-CHECK-NEXT:                 + END LOOP
; CG-CHECK-NEXT:              }
; CG-CHECK:                   + DO i1 = 4 * %tgu, %n + -1 * %peel.factor1 + -1, 1   <DO_MULTI_EXIT_LOOP>  <MAX_TC_EST = 3> <LEGAL_MAX_TC = 3> <nounroll> <novectorize> <max_trip_count = 3>
; CG-CHECK-NEXT:              |   if ((%t4)[i1 + %peel.factor1] == &((%t1)[0]))
; CG-CHECK-NEXT:              |   {
; CG-CHECK-NEXT:              |      %gep = &((%t4)[i1 + %peel.factor1]);
; CG-CHECK-NEXT:              |      goto found;
; CG-CHECK-NEXT:              |   }
; CG-CHECK-NEXT:              + END LOOP
; CG-CHECK-NEXT:           }
; CG-CHECK-NEXT:     END REGION

; Check that VF=16 is used for targets with 2K or higher DSB size (icelake-server and alderlake).
; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation \
; RUN:      -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -enable-intel-advanced-opts -disable-output -mcpu=alderlake < %s 2>&1 | FileCheck --check-prefix=VF16-CHECK %s
; RUN: opt -xmain-opt-level=3 -hir-ssa-deconstruction -hir-temp-cleanup -hir-last-value-computation \
; RUN:      -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec \
; RUN:     -mtriple=x86_64-unknown-unknown -mattr=+avx2 -enable-intel-advanced-opts -disable-output -mcpu=icelake-server < %s 2>&1 | FileCheck --check-prefix=VF16-CHECK %s
; VF16-CHECK:           BEGIN REGION { modified }
; VF16-CHECK:                 + DO i1 = 0, 16 * %tgu + -1, 16   <DO_MULTI_EXIT_LOOP> <auto-vectorized> <nounroll> <novectorize>

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct2 = type <{ i32, [4 x i8] }>

; Function Attrs: uwtable
define %struct2** @foo(%struct2** %t4, %struct2* %t1, i64 %n) {
entry:
  br label %header

header:
  %iv = phi i64 [0, %entry], [%iv.next, %latch]
  %gep = getelementptr %struct2*, %struct2 **%t4, i64 %iv
  %ptr = load %struct2 *, %struct2** %gep
  %cmp.found = icmp eq %struct2 *%ptr, %t1
  br i1 %cmp.found, label %found, label %latch

latch:
  %iv.next = add i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, %n
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  br label %exit

found:
  %lcssa = phi %struct2 ** [ %gep , %header ]
  br label %exit

exit:
  %val = phi %struct2 ** [ %lcssa, %found ], [ zeroinitializer, %loop.exit ]
  ret %struct2 ** %val
}
