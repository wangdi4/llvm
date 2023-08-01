; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -disable-output -vplan-print-after-vls -vplan-force-vf=4 < %s 2>&1 | FileCheck %s

; LIT test to demonstrate issue with VLS group formation in VPlan HIR path.
; When forming groups, we were getting stride information using the parent loop
; of the memory access. We should be using the original loop that we are trying
; to vectorize.
;
; Incoming HIR:
;
;               + DO i1 = 0, 1023, 1   <DO_LOOP> <simd>
;               |   + DO i2 = 0, 1023, 1   <DO_LOOP>
;               |   |   %0 = (@arr)[0][2 * i2];
;               |   |   %1 = (@arr)[0][2 * i2 + 1];
;               |   |   (@arr2)[0][i2][i1] = %0 + %1;
;               |   + END LOOP
;               + END LOOP
;
;
; CHECK: Function: foo
; CHECK:                     + DO i1 = 0, 1023, 4   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK-NEXT:                |   %phi.temp = 0;
; CHECK-NEXT:                |   
; CHECK-NEXT:                |   + DO i2 = 0, 1023, 1   <DO_LOOP> <novectorize>
; CHECK-NEXT:                |   |   %.unifload = (i64*)(@arr)[0][2 * i2];
; CHECK-NEXT:                |   |   %.unifload3 = (i64*)(@arr)[0][2 * i2 + 1];
; CHECK-NEXT:                |   |   (<4 x i64>*)(@arr2)[0][i2][i1] = %.unifload + %.unifload3;
; CHECK-NEXT:                |   |   %.vec = i2 + 1 < 1024;
; CHECK-NEXT:                |   |   %phi.temp = i2 + 1;
; CHECK-NEXT:                |   + END LOOP
; CHECK-NEXT:                + END LOOP
;
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr = dso_local local_unnamed_addr global [2048 x i64] zeroinitializer, align 16
@arr2 = dso_local local_unnamed_addr global [1024 x [1024 x i64]] zeroinitializer, align 16

define dso_local void @foo() {
entry:
  br label %bb1

bb1:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc9
  %l1.021 = phi i64 [ 0, %bb1 ], [ %inc10, %for.inc9 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %l2.020 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %mul = shl nuw nsw i64 %l2.020, 1
  %arrayidx = getelementptr inbounds [2048 x i64], ptr @arr, i64 0, i64 %mul
  %0 = load i64, ptr %arrayidx, align 16
  %add = add nuw nsw i64 %mul, 1
  %arrayidx5 = getelementptr inbounds [2048 x i64], ptr @arr, i64 0, i64 %add
  %1 = load i64, ptr %arrayidx5, align 8
  %add6 = add nsw i64 %1, %0
  %arrayidx8 = getelementptr inbounds [1024 x [1024 x i64]], ptr @arr2, i64 0, i64 %l2.020, i64 %l1.021
  store i64 %add6, ptr %arrayidx8, align 8
  %inc = add nuw nsw i64 %l2.020, 1
  %exitcond.not = icmp eq i64 %inc, 1024
  br i1 %exitcond.not, label %for.inc9, label %for.body3

for.inc9:                                         ; preds = %for.body3
  %inc10 = add nuw nsw i64 %l1.021, 1
  %exitcond22.not = icmp eq i64 %inc10, 1024
  br i1 %exitcond22.not, label %for.end11, label %for.cond1.preheader

for.end11:                                        ; preds = %for.inc9
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
