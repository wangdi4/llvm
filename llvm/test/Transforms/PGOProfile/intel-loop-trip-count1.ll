; Basic test for annotating loops with multiple possible loop trip counts based
; on PGO value profiling.

; RUN: llvm-profdata merge %S/Inputs/intel-loop-trip-count1.proftext -o %t.profdata
; RUN: opt %s -S -pgo-looptc-annotate=true -passes=pgo-instr-use -pgo-test-profile-file=%t.profdata | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z8ConvolveiiPfS_Pii(i32 noundef %height, i32 noundef %width, ptr noundef %sum, ptr noundef %P, ptr noundef %Kernel, i32 noundef %channel) {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.cond.cleanup3, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc21, %for.cond.cleanup3 ]
  %cmp = icmp slt i32 %i.0, %height
  br i1 %cmp, label %for.cond1, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.cond1:                                        ; preds = %for.cond, %for.cond.cleanup9
  %j.0 = phi i32 [ %inc18, %for.cond.cleanup9 ], [ 0, %for.cond ]
  %cmp2 = icmp slt i32 %j.0, %width
  br i1 %cmp2, label %for.body4, label %for.cond.cleanup3

for.cond.cleanup3:                                ; preds = %for.cond1
  %inc21 = add nuw nsw i32 %i.0, 1
  br label %for.cond, !llvm.loop !3

for.body4:                                        ; preds = %for.cond1
  %idx.ext = zext i32 %j.0 to i64
  %add.ptr = getelementptr inbounds float, ptr %P, i64 %idx.ext
  %mul = mul nsw i32 %i.0, %width
  %idx.ext5 = sext i32 %mul to i64
  %add.ptr6 = getelementptr inbounds float, ptr %add.ptr, i64 %idx.ext5
  br label %for.cond7

for.cond7:                                        ; preds = %for.body10, %for.body4
  %sum0.0 = phi float [ 0.000000e+00, %for.body4 ], [ %add, %for.body10 ]
  %c.0 = phi i32 [ 0, %for.body4 ], [ %inc, %for.body10 ]
  %cmp8 = icmp slt i32 %c.0, %channel
  br i1 %cmp8, label %for.body10, label %for.cond.cleanup9

for.cond.cleanup9:                                ; preds = %for.cond7
  %arrayidx15 = getelementptr inbounds float, ptr %sum, i64 %idx.ext
  %0 = load float, ptr %arrayidx15, align 4
  %add16 = fadd fast float %0, %sum0.0
  store float %add16, ptr %arrayidx15, align 4
  %inc18 = add nuw nsw i32 %j.0, 1
  br label %for.cond1, !llvm.loop !9

for.body10:                                       ; preds = %for.cond7
  %idxprom = zext i32 %c.0 to i64
  %arrayidx = getelementptr inbounds float, ptr %add.ptr6, i64 %idxprom
  %1 = load float, ptr %arrayidx, align 4
  %arrayidx12 = getelementptr inbounds i32, ptr %Kernel, i64 %idxprom
  %2 = load i32, ptr %arrayidx12, align 4
  %conv = sitofp i32 %2 to float
  %mul13 = fmul fast float %1, %conv
  %add = fadd fast float %sum0.0, %mul13
  %inc = add nuw nsw i32 %c.0, 1
  ; This loop metadata will be updated to contain the trip count based
  ; on the PGO data.
  br label %for.cond7, !llvm.loop !12
}

!3 = distinct !{!3, !4}
!4 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !4}
!12 = distinct !{!12, !4}

; CHECK: br label %for.cond7, !llvm.loop ![[LOOP_MD:[0-9]+]]
; CHECK: ![[LOOP_MD]] = distinct !{![[LOOP_MD]], ![[LOOP_MUST_PROGRESS:[0-9]+]], ![[LOOP_COUNT:[0-9]+]]}
; CHECK: ![[LOOP_COUNT]] = !{!"llvm.loop.intel.loopcount", i32 8, i32 4, i32 2}
