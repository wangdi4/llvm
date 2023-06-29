; RUN: opt -S -passes=pgo-instr-gen -disable-looptc-vp=false -looptc-min-depth=3 %s | FileCheck %s

; Test to check the instrumentation pass insertion of value profiling
; data collection for loop trip counts only applies counts into the
; loop nest at a loop depth of 3.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z8ConvolveiiPfS_Pii(i32 noundef %height, i32 noundef %width, ptr noundef %sum, ptr noundef %P, ptr noundef %Kernel, i32 noundef %channel) local_unnamed_addr #0 {
entry:
  br label %for.cond

; Outer loop header
for.cond:                                         ; preds = %for.cond.cleanup3, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc21, %for.cond.cleanup3 ]
  %cmp = icmp slt i32 %i.0, %height
  br i1 %cmp, label %for.cond1, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

; Middle loop header
for.cond1:                                        ; preds = %for.cond, %for.cond.cleanup9
  %j.0 = phi i32 [ %inc18, %for.cond.cleanup9 ], [ 0, %for.cond ]
  %cmp2 = icmp slt i32 %j.0, %width
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond1
  %inc21 = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !0

for.body4:                                        ; preds = %for.cond1
  %idx.ext = zext i32 %j.0 to i64
  %add.ptr = getelementptr inbounds float, ptr %P, i64 %idx.ext
  %mul = mul nsw i32 %i.0, %width
  %idx.ext5 = sext i32 %mul to i64
  %add.ptr6 = getelementptr inbounds float, ptr %add.ptr, i64 %idx.ext5
  br label %for.cond7

; Inner loop header
for.cond7:                                        ; preds = %for.body10, %for.body4
  %sum0.0 = phi float [ 0.000000e+00, %for.body4 ], [ %add, %for.body10 ]
  %c.0 = phi i32 [ 0, %for.body4 ], [ %inc, %for.body10 ]
  %cmp8 = icmp slt i32 %c.0, %channel
  br i1 %cmp8, label %for.body10, label %for.cond.cleanup9

for.cond.cleanup9:                                ; preds = %for.cond7
  %arrayidx15 = getelementptr inbounds float, ptr %sum, i64 %idx.ext
  %0 = load float, ptr %arrayidx15
  %add16 = fadd fast float %0, %sum0.0
  store float %add16, ptr %arrayidx15
  %inc18 = add nuw nsw i32 %j.0, 1
  br label %for.cond1, !llvm.loop !2

for.body10:                                       ; preds = %for.cond7
  %idxprom = zext i32 %c.0 to i64
  %arrayidx = getelementptr inbounds float, ptr %add.ptr6, i64 %idxprom
  %1 = load float, ptr %arrayidx
  %arrayidx12 = getelementptr inbounds i32, ptr %Kernel, i64 %idxprom
  %2 = load i32, ptr %arrayidx12
  %conv = sitofp i32 %2 to float
  %mul13 = fmul fast float %1, %conv
  %add = fadd fast float %sum0.0, %mul13
  %inc = add nuw nsw i32 %c.0, 1
  br label %for.cond7, !llvm.loop !3
}

; The loop headers of the outer and middle loop should not get changed.
; CHECK: for.cond:
; CHECK-NEXT: %i.0 = phi i32 [ 0, %entry ], [ %inc21, %for.cond.cleanup3 ]
; CHECK-NEXT: %cmp = icmp slt i32 %i.0, %height
; CHECK-NEXT: br i1 %cmp, label %for.cond1, label %for.cond.cleanup

; CHECK: for.cond1:
; CHECK-NEXT: %j.0 = phi i32 [ %inc18, %for.cond.cleanup9 ], [ 0, %for.cond ]
; CHECK-NEXT: %cmp2 = icmp slt i32 %j.0, %width
; CHECK-NEXT: br i1 %cmp2, label %for.body4, label %for.cond.cleanup3

; The inner loop should get the instrumentation instructions.
; CHECK: for.cond7:
; CHECK: %lc = phi i64 [ %lc_incr, %for.body10 ], [ 0, %for.body4 ]
; CHECK: %lc_incr = add nuw nsw i64 %lc, 1

; CHECK: for.cond.cleanup9:
; CHECK: %lc_exit = phi i64 [ %lc, %for.cond7 ]
; CHECK: call void @llvm.instrprof.value.profile(ptr @__profn__Z8ConvolveiiPfS_Pii, i64 {{[0-9]+}}, i64 %lc_exit, i32 2, i32 0)

!0 = distinct !{!0, !1}
!1 = !{!"llvm.loop.mustprogress"}
!2 = distinct !{!2, !1}
!3 = distinct !{!3, !1}
