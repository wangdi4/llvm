; Test to verify that VPlan vectorizer code generators correctly handle masks
; for complex float-type math library calls vectorized using SVML.

; RUN: opt -passes=vplan-vec -vector-library=SVML -S < %s 2>&1 | FileCheck %s --check-prefixes=IR
; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vector-library=SVML -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=HIR

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Checks for LLVM-IR codegen.
; IR:           define void @foo(ptr noalias nocapture [[A0:%.*]]) {
; IR:             [[MASK_CMP:%.*]] = icmp eq <4 x i64> [[IV_PHI:%.*]], <i64 42, i64 42, i64 42, i64 42>
; IR:             [[TMP5:%.*]] = sext <4 x i1> [[MASK_CMP]] to <4 x i32>
; IR:             [[TMP6:%.*]] = call fast svml_cc nofpclass(nan inf) <8 x float> @__svml_cexpf4_mask(<8 x float> noundef nofpclass(nan inf) [[WIDE_MASKED_LOAD:%.*]], <4 x i32> [[TMP5]])
;
; Checks for HIR codegen.
; HIR:   BEGIN REGION { modified }
; HIR:         + DO i1 = 0, %loop.ub, 4   <DO_LOOP> <simd-vectorized> <nounroll> <novectorize>
; HIR:         |   %.vec4 = i1 + <i64 0, i64 1, i64 2, i64 3> == 42;
; HIR:         |   %sext = sext.<4 x i1>.<4 x i32>(%.vec4);
; HIR:         |   %__svml_cexpf4_mask = @__svml_cexpf4_mask(%.vec5,  %sext);
; HIR:         + END LOOP
; HIR:   END REGION

define void @foo(ptr nocapture noalias %A, i64 %n) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %loop

loop:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %if.merge ]
  %cond = icmp eq i64 %iv, 42
  br i1 %cond, label %if.then, label %if.merge

if.then:
  %A.idx = getelementptr inbounds <2 x float>, ptr %A, i64 %iv
  %A.ld = load <2 x float>, ptr %A.idx, align 1
  %exp = call fast nofpclass(nan inf) <2 x float> @cexpf(<2 x float> noundef nofpclass(nan inf) %A.ld)
  store <2 x float> %exp, ptr %A.idx, align 1
  br label %if.merge

if.merge:
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv, %n
  br i1 %exitcond, label %exit, label %loop

exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare <2 x float> @cexpf(<2 x float>) local_unnamed_addr #0
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { mustprogress nocallback nofree nosync nounwind speculatable willreturn memory(none) }

