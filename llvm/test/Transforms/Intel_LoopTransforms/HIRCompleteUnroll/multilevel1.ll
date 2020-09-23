; Test for Complete Unrolling with 2-level loops.
; Both the loops should be unrolled as they have small trip count.

;*** IR Dump Before HIR PostVec Complete Unroll ***
;Function: foo
;
;<0>       BEGIN REGION { }
;<25>            + DO i1 = 0, 1, 1   <DO_LOOP>
;<26>            |   + DO i2 = 0, 4, 1   <DO_LOOP>
;<9>             |   |   %0 = (@A)[0][i1 + 2 * i2 + -1];
;<11>            |   |   (@A)[0][i1 + 2 * i2] = %0;
;<26>            |   + END LOOP
;<25>            + END LOOP
;<0>       END REGION

; RUN: opt -loop-simplify -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -S < %s | FileCheck %s
; RUN: opt -passes="loop-simplify,hir-ssa-deconstruction,hir-post-vec-complete-unroll,hir-cg" -S < %s | FileCheck %s
; CHECK: entry

; terminator of entry bblock should point to new unrolled region.
; CHECK: for.cond1.preheader:
; CHECK: br i1 true, {{.*}}label %region

; check loop is completely unrolled.
; CHECK: region.0:
; CHECK: getelementptr inbounds ([550 x i32], [550 x i32]* @A, i64 0, i64 -1)
; CHECK: getelementptr inbounds ([550 x i32], [550 x i32]* @A, i64 0, i64 0)
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr
; CHECK: getelementptr inbounds ([550 x i32], [550 x i32]* @A, i64 0, i64 8)
; CHECK: getelementptr inbounds ([550 x i32], [550 x i32]* @A, i64 0, i64 9)
; CHECK-NEXT: br label %for.end{{.*}}

; Check that proper optreport order is emitted for deleted loops (Completely Unrolled).
; Emitted structure has one remark for completely unrolled loops assigned to parent loop (because all inner loops are unrolled).
; RUN: opt -hir-ssa-deconstruction -hir-post-vec-complete-unroll -hir-cg -intel-loop-optreport=low -simplifycfg -intel-ir-optreport-emitter %s 2>&1 < %s -S | FileCheck %s --check-prefix=OPTREPORT --strict-whitespace
; RUN: opt -passes="hir-ssa-deconstruction,hir-post-vec-complete-unroll,hir-cg,loop-simplifycfg,intel-ir-optreport-emitter" -intel-loop-optreport=low %s 2>&1 < %s -S | FileCheck %s --check-prefix=OPTREPORT --strict-whitespace

; OPTREPORT: LOOP BEGIN
; OPTREPORT-NEXT:     Remark: Loopnest completely unrolled{{[[:space:]]}}
; OPTREPORT-NEXT:     LOOP BEGIN
; OPTREPORT-NEXT:     LOOP END
; OPTREPORT-NEXT: LOOP END

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@A = common global [550 x i32] zeroinitializer, align 16
@B = common global [550 x i32] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo(i32 %n, i32 %b) {
entry:
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc7, %entry
  %i.019 = phi i64 [ 0, %entry ], [ %inc8, %for.inc7 ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %j.018 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %mul = shl nsw i64 %j.018, 1
  %add = add nuw nsw i64 %mul, %i.019
  %sub = add nsw i64 %add, -1
  %arrayidx = getelementptr inbounds [550 x i32], [550 x i32]* @A, i64 0, i64 %sub
  %0 = load i32, i32* %arrayidx, align 4
  %arrayidx6 = getelementptr inbounds [550 x i32], [550 x i32]* @A, i64 0, i64 %add
  store i32 %0, i32* %arrayidx6, align 4
  %inc = add nuw nsw i64 %j.018, 1
  %exitcond = icmp eq i64 %inc, 5
  br i1 %exitcond, label %for.inc7, label %for.body3

for.inc7:                                         ; preds = %for.body3
  %inc8 = add nuw nsw i64 %i.019, 1
  %exitcond20 = icmp eq i64 %inc8, 2
  br i1 %exitcond20, label %for.end9, label %for.cond1.preheader

for.end9:                                         ; preds = %for.inc7
  ret void
}

; Function Attrs: nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture)

; Function Attrs: nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture)
