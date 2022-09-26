; Test to check validity of opt-report emitted for vectorization scenarios where
; different types of peel and remainder loops are utilized by VPlan.

; RUN: opt -vplan-vec-scenario="n0;v4;v2s1" -disable-output -vplan-vec -vplan-enable-peeling -intel-ir-optreport-emitter -intel-opt-report=low %s 2>&1 | FileCheck %s --check-prefix=SCEN1
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-vec-scenario="n0;v4;v2s1" -disable-output -vplan-enable-peeling -hir-optreport-emitter -intel-opt-report=low -vplan-enable-masked-variant-hir %s 2>&1 | FileCheck %s --check-prefix=SCEN1
; SCEN1-LABEL: {{Global optimization report|Report from: HIR Loop optimizations framework}} for : test_store
; SCEN1-EMPTY:
; SCEN1-NEXT: LOOP BEGIN
; SCEN1-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; SCEN1-NEXT:     remark #15305: vectorization support: vector length 4
; SCEN1-NEXT: LOOP END
; SCEN1-EMPTY:
; SCEN1-NEXT: LOOP BEGIN
; SCEN1-NEXT: <Remainder loop for vectorization>
; SCEN1-NEXT:     remark #15439: remainder loop was vectorized (unmasked)
; SCEN1-NEXT:     remark #15305: vectorization support: vector length 2
; SCEN1-NEXT: LOOP END
; SCEN1-EMPTY:
; SCEN1-NEXT: LOOP BEGIN
; SCEN1-NEXT: <Remainder loop for vectorization>
; SCEN1-NEXT: LOOP END

; RUN: opt -vplan-vec-scenario="n0;v4;m2" -disable-output -vplan-vec -vplan-enable-peeling -intel-ir-optreport-emitter -intel-opt-report=low %s 2>&1 | FileCheck %s --check-prefix=SCEN2
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-vec-scenario="n0;v4;m2" -disable-output -vplan-enable-peeling -hir-optreport-emitter -intel-opt-report=low -vplan-enable-masked-variant-hir %s 2>&1 | FileCheck %s --check-prefix=SCEN2
; SCEN2-LABEL: {{Global optimization report|Report from: HIR Loop optimizations framework}} for : test_store
; SCEN2-EMPTY:
; SCEN2-NEXT: LOOP BEGIN
; SCEN2-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; SCEN2-NEXT:     remark #15305: vectorization support: vector length 4
; SCEN2-NEXT: LOOP END
; SCEN2-EMPTY:
; SCEN2-NEXT: LOOP BEGIN
; SCEN2-NEXT: <Remainder loop for vectorization>
; SCEN2-NEXT:     remark #15440: remainder loop was vectorized (masked)
; SCEN2-NEXT:     remark #15305: vectorization support: vector length 2
; SCEN2-NEXT: LOOP END

; RUN: opt -vplan-vec-scenario="s1;v4;v2s1" -disable-output -vplan-vec -vplan-enable-peeling -intel-ir-optreport-emitter -intel-opt-report=low %s 2>&1 | FileCheck %s --check-prefix=SCEN3
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-vec-scenario="s1;v4;v2s1" -disable-output -vplan-enable-peeling -hir-optreport-emitter -intel-opt-report=low -vplan-enable-masked-variant-hir %s 2>&1 | FileCheck %s --check-prefix=SCEN3
; SCEN3-LABEL: {{Global optimization report|Report from: HIR Loop optimizations framework}} for : test_store
; SCEN3-EMPTY:
; SCEN3-NEXT: LOOP BEGIN
; SCEN3-NEXT: <Peeled loop for vectorization>
; SCEN3-NEXT: LOOP END
; SCEN3-EMPTY:
; SCEN3-NEXT: LOOP BEGIN
; SCEN3-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; SCEN3-NEXT:     remark #15305: vectorization support: vector length 4
; SCEN3-NEXT: LOOP END
; SCEN3-EMPTY:
; SCEN3-NEXT: LOOP BEGIN
; SCEN3-NEXT: <Remainder loop for vectorization>
; SCEN3-NEXT:     remark #15439: remainder loop was vectorized (unmasked)
; SCEN3-NEXT:     remark #15305: vectorization support: vector length 2
; SCEN3-NEXT: LOOP END
; SCEN3-EMPTY:
; SCEN3-NEXT: LOOP BEGIN
; SCEN3-NEXT: <Remainder loop for vectorization>
; SCEN3-NEXT: LOOP END

; RUN: opt -vplan-vec-scenario="m4;v4;m4s1" -disable-output -vplan-vec -vplan-enable-peeling -intel-ir-optreport-emitter -intel-opt-report=low %s 2>&1 | FileCheck %s --check-prefix=SCEN4
; SCEN4-LABEL: Global optimization report for : test_store
; SCEN4-EMPTY:
; SCEN4-NEXT: LOOP BEGIN
; SCEN4-NEXT: <Peeled loop for vectorization>
; SCEN4-NEXT:     remark #15437: peel loop was vectorized
; SCEN4-NEXT:     remark #15305: vectorization support: vector length 4
; SCEN4-NEXT: LOOP END
; SCEN4-EMPTY:
; SCEN4-NEXT: LOOP BEGIN
; SCEN4-NEXT: <Remainder loop for vectorization>
; SCEN4-NEXT: LOOP END
; SCEN4-EMPTY:
; SCEN4-NEXT: LOOP BEGIN
; SCEN4-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; SCEN4-NEXT:     remark #15305: vectorization support: vector length 4
; SCEN4-NEXT: LOOP END
; SCEN4-EMPTY:
; SCEN4-NEXT: LOOP BEGIN
; SCEN4-NEXT: <Remainder loop for vectorization>
; SCEN4-NEXT:     remark #15440: remainder loop was vectorized (masked)
; SCEN4-NEXT:     remark #15305: vectorization support: vector length 4
; SCEN4-NEXT: LOOP END


target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "x86_64-unknown-linux-gnu"

define void @test_store(i64* nocapture %ary, i32 %c) {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %ptr = getelementptr inbounds i64, i64* %ary, i64 %indvars.iv
  %cc = sext i32 %c to i64
  %add = add i64 %cc, %indvars.iv
  store i64 %add, i64* %ptr, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1026
  br i1 %cmp, label %for.body, label %for.end

for.end:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
