; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: asserts
; RUN: opt  -passes=tight-loop-emitter -tight-loop-emitter-run=remark -debug-only=tight-loop-emitter  -disable-output < %s 2>&1 | FileCheck %s

; Verify tight-loop-emitter outputs cycles of fma, where cycle lengths are short enough.

; CHECK: Intel Tight Loop Emitter : _Z5cgemmPdS_S_i
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.24.0 = phi <4 x double> [ %vla100.sroa.24.0.copyload, %for.cond5.preheader.us.us.us ], [ %35, %for.body16.us.us.us ]
; CHECK-NEXT:   %35 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.11, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.24.0)
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.22.0 = phi <4 x double> [ %vla100.sroa.22.0.copyload, %for.cond5.preheader.us.us.us ], [ %33, %for.body16.us.us.us ]
; CHECK-NEXT:   %33 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.10, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.22.0)
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.20.0 = phi <4 x double> [ %vla100.sroa.20.0.copyload, %for.cond5.preheader.us.us.us ], [ %31, %for.body16.us.us.us ]
; CHECK-NEXT:   %31 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.9, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.20.0)
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.18.0 = phi <4 x double> [ %vla100.sroa.18.0.copyload, %for.cond5.preheader.us.us.us ], [ %29, %for.body16.us.us.us ]
; CHECK-NEXT:   %29 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.8, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.18.0)
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.16.0 = phi <4 x double> [ %vla100.sroa.16.0.copyload, %for.cond5.preheader.us.us.us ], [ %27, %for.body16.us.us.us ]
; CHECK-NEXT:   %27 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.7, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.16.0)
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.14.0 = phi <4 x double> [ %vla100.sroa.14.0.copyload, %for.cond5.preheader.us.us.us ], [ %25, %for.body16.us.us.us ]
; CHECK-NEXT:   %25 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.6, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.14.0)
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.12.0 = phi <4 x double> [ %vla100.sroa.12.0.copyload, %for.cond5.preheader.us.us.us ], [ %23, %for.body16.us.us.us ]
; CHECK-NEXT:   %23 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.5, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.12.0)
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.10.0 = phi <4 x double> [ %vla100.sroa.10.0.copyload, %for.cond5.preheader.us.us.us ], [ %21, %for.body16.us.us.us ]
; CHECK-NEXT:   %21 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.4, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.10.0)
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.8.0 = phi <4 x double> [ %vla100.sroa.8.0.copyload, %for.cond5.preheader.us.us.us ], [ %19, %for.body16.us.us.us ]
; CHECK-NEXT:   %19 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.3, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.8.0)
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.6.0 = phi <4 x double> [ %vla100.sroa.6.0.copyload, %for.cond5.preheader.us.us.us ], [ %17, %for.body16.us.us.us ]
; CHECK-NEXT:   %17 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.2, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.6.0)
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.4.0 = phi <4 x double> [ %vla100.sroa.4.0.copyload, %for.cond5.preheader.us.us.us ], [ %15, %for.body16.us.us.us ]
; CHECK-NEXT:   %15 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.1, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.4.0)
; CHECK: Tight cycle found for Loop: for.body16.us.us.us
; CHECK-NEXT:   %vla100.sroa.0.0 = phi <4 x double> [ %vla100.sroa.0.0.copyload, %for.cond5.preheader.us.us.us ], [ %13, %for.body16.us.us.us ]
; CHECK-NEXT:   %13 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.0.0)

; Verify tight loop information is put into opt-report.

; RUN: opt <%s -passes="tight-loop-emitter,intel-ir-optreport-emitter" -tight-loop-emitter-run=remark --intel-opt-report-emitter=ir -intel-opt-report=high -disable-output 2>&1 | FileCheck %s -check-prefix=OPTRPT

; OPTRPT: Global optimization report for : _Z5cgemmPdS_S_i
;
; OPTRPT: LOOP BEGIN at fma.cpp (8, 3)
;
; OPTRPT:     LOOP BEGIN at fma.cpp (9, 5)
;
; OPTRPT:         LOOP BEGIN at fma.cpp (14, 7)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.24.0 = phi <4 x double> [ %vla100.sroa.24.0.copyload, %for.cond5.preheader.us.us.us ], [ %35, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %35 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.11, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.24.0)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.22.0 = phi <4 x double> [ %vla100.sroa.22.0.copyload, %for.cond5.preheader.us.us.us ], [ %33, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %33 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.10, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.22.0)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.20.0 = phi <4 x double> [ %vla100.sroa.20.0.copyload, %for.cond5.preheader.us.us.us ], [ %31, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %31 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.9, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.20.0)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.18.0 = phi <4 x double> [ %vla100.sroa.18.0.copyload, %for.cond5.preheader.us.us.us ], [ %29, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %29 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.8, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.18.0)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.16.0 = phi <4 x double> [ %vla100.sroa.16.0.copyload, %for.cond5.preheader.us.us.us ], [ %27, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %27 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.7, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.16.0)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.14.0 = phi <4 x double> [ %vla100.sroa.14.0.copyload, %for.cond5.preheader.us.us.us ], [ %25, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %25 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.6, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.14.0)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.12.0 = phi <4 x double> [ %vla100.sroa.12.0.copyload, %for.cond5.preheader.us.us.us ], [ %23, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %23 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.5, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.12.0)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.10.0 = phi <4 x double> [ %vla100.sroa.10.0.copyload, %for.cond5.preheader.us.us.us ], [ %21, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %21 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.4, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.10.0)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.8.0 = phi <4 x double> [ %vla100.sroa.8.0.copyload, %for.cond5.preheader.us.us.us ], [ %19, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %19 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.3, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.8.0)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.6.0 = phi <4 x double> [ %vla100.sroa.6.0.copyload, %for.cond5.preheader.us.us.us ], [ %17, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %17 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.2, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.6.0)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.4.0 = phi <4 x double> [ %vla100.sroa.4.0.copyload, %for.cond5.preheader.us.us.us ], [ %15, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %15 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.1, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.4.0)
; OPTRPT:             remark: Tight cycle found for Loop for.body16.us.us.us
; OPTRPT-NEXT:        remark:   %vla100.sroa.0.0 = phi <4 x double> [ %vla100.sroa.0.0.copyload, %for.cond5.preheader.us.us.us ], [ %13, %for.body16.us.us.us ]
; OPTRPT-NEXT:        remark:   %13 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.0.0)
;
; OPTRPT:             LOOP BEGIN at fma.cpp (16, 9)
; OPTRPT:                 remark: LLorg: Loop has been completely unrolled
; OPTRPT:             LOOP END
; OPTRPT:         LOOP END
; OPTRPT:     LOOP END
; OPTRPT: LOOP END

; ModuleID = 'fma.cpp'
source_filename = "fma.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly mustprogress nofree nosync nounwind uwtable
define dso_local void @_Z5cgemmPdS_S_i(ptr nocapture noundef %Y, ptr nocapture noundef readonly %A, ptr nocapture noundef readonly %B, i32 noundef %n) local_unnamed_addr #0 !dbg !6 {
entry:
  %cmp108 = icmp sgt i32 %n, 0, !dbg !9
  br i1 %cmp108, label %for.cond1.preheader.us.us.preheader, label %for.cond.cleanup, !dbg !10

for.cond1.preheader.us.us.preheader:              ; preds = %entry
  %0 = zext i32 %n to i64, !dbg !10
  %1 = shl nuw nsw i64 %0, 3, !dbg !11
  %2 = add nsw i32 %n, -1, !dbg !10
  %3 = udiv i32 %2, 48, !dbg !10
  %4 = add nuw nsw i32 %3, 1, !dbg !10
  %wide.trip.count162 = zext i32 %4 to i64
  br label %for.cond1.preheader.us.us, !dbg !10

for.cond1.preheader.us.us:                        ; preds = %for.cond1.preheader.us.us.preheader, %for.cond1.for.cond.cleanup3_crit_edge.split.us.us.us
  %indvar = phi i64 [ 0, %for.cond1.preheader.us.us.preheader ], [ %indvar.next, %for.cond1.for.cond.cleanup3_crit_edge.split.us.us.us ]
  %5 = mul i64 %1, %indvar, !dbg !11
  %6 = mul nsw i64 %indvar, %0, !dbg !12
  br label %for.cond5.preheader.us.us.us, !dbg !13

for.cond5.preheader.us.us.us:                     ; preds = %for.cond13.for.cond46.preheader_crit_edge.us.us.us, %for.cond1.preheader.us.us
  %indvars.iv159 = phi i64 [ %indvars.iv.next160, %for.cond13.for.cond46.preheader_crit_edge.us.us.us ], [ 0, %for.cond1.preheader.us.us ]
  %indvar141 = phi i64 [ %indvar.next142, %for.cond13.for.cond46.preheader_crit_edge.us.us.us ], [ 0, %for.cond1.preheader.us.us ]
  %7 = mul nuw nsw i64 %indvar141, 384, !dbg !14
  %8 = add i64 %5, %7, !dbg !14
  %uglygep153 = getelementptr i8, ptr %Y, i64 %8, !dbg !14
  %vla100.sroa.0.0.copyload = load <4 x double>, ptr %uglygep153, align 1, !dbg !15, !tbaa !16
  %vla100.sroa.4.0.uglygep.sroa_idx = getelementptr inbounds i8, ptr %uglygep153, i64 32, !dbg !15
  %vla100.sroa.4.0.copyload = load <4 x double>, ptr %vla100.sroa.4.0.uglygep.sroa_idx, align 1, !dbg !15, !tbaa !16
  %vla100.sroa.6.0.uglygep.sroa_idx = getelementptr inbounds i8, ptr %uglygep153, i64 64, !dbg !15
  %vla100.sroa.6.0.copyload = load <4 x double>, ptr %vla100.sroa.6.0.uglygep.sroa_idx, align 1, !dbg !15, !tbaa !16
  %vla100.sroa.8.0.uglygep.sroa_idx = getelementptr inbounds i8, ptr %uglygep153, i64 96, !dbg !15
  %vla100.sroa.8.0.copyload = load <4 x double>, ptr %vla100.sroa.8.0.uglygep.sroa_idx, align 1, !dbg !15, !tbaa !16
  %vla100.sroa.10.0.uglygep.sroa_idx = getelementptr inbounds i8, ptr %uglygep153, i64 128, !dbg !15
  %vla100.sroa.10.0.copyload = load <4 x double>, ptr %vla100.sroa.10.0.uglygep.sroa_idx, align 1, !dbg !15, !tbaa !16
  %vla100.sroa.12.0.uglygep.sroa_idx = getelementptr inbounds i8, ptr %uglygep153, i64 160, !dbg !15
  %vla100.sroa.12.0.copyload = load <4 x double>, ptr %vla100.sroa.12.0.uglygep.sroa_idx, align 1, !dbg !15, !tbaa !16
  %vla100.sroa.14.0.uglygep.sroa_idx = getelementptr inbounds i8, ptr %uglygep153, i64 192, !dbg !15
  %vla100.sroa.14.0.copyload = load <4 x double>, ptr %vla100.sroa.14.0.uglygep.sroa_idx, align 1, !dbg !15, !tbaa !16
  %vla100.sroa.16.0.uglygep.sroa_idx = getelementptr inbounds i8, ptr %uglygep153, i64 224, !dbg !15
  %vla100.sroa.16.0.copyload = load <4 x double>, ptr %vla100.sroa.16.0.uglygep.sroa_idx, align 1, !dbg !15, !tbaa !16
  %vla100.sroa.18.0.uglygep.sroa_idx = getelementptr inbounds i8, ptr %uglygep153, i64 256, !dbg !15
  %vla100.sroa.18.0.copyload = load <4 x double>, ptr %vla100.sroa.18.0.uglygep.sroa_idx, align 1, !dbg !15, !tbaa !16
  %vla100.sroa.20.0.uglygep.sroa_idx = getelementptr inbounds i8, ptr %uglygep153, i64 288, !dbg !15
  %vla100.sroa.20.0.copyload = load <4 x double>, ptr %vla100.sroa.20.0.uglygep.sroa_idx, align 1, !dbg !15, !tbaa !16
  %vla100.sroa.22.0.uglygep.sroa_idx = getelementptr inbounds i8, ptr %uglygep153, i64 320, !dbg !15
  %vla100.sroa.22.0.copyload = load <4 x double>, ptr %vla100.sroa.22.0.uglygep.sroa_idx, align 1, !dbg !15, !tbaa !16
  %vla100.sroa.24.0.uglygep.sroa_idx = getelementptr inbounds i8, ptr %uglygep153, i64 352, !dbg !15
  %vla100.sroa.24.0.copyload = load <4 x double>, ptr %vla100.sroa.24.0.uglygep.sroa_idx, align 1, !dbg !15, !tbaa !16
  br label %for.body16.us.us.us, !dbg !19

for.body16.us.us.us:                              ; preds = %for.cond5.preheader.us.us.us, %for.body16.us.us.us
  %vla100.sroa.24.0 = phi <4 x double> [ %vla100.sroa.24.0.copyload, %for.cond5.preheader.us.us.us ], [ %35, %for.body16.us.us.us ]
  %vla100.sroa.22.0 = phi <4 x double> [ %vla100.sroa.22.0.copyload, %for.cond5.preheader.us.us.us ], [ %33, %for.body16.us.us.us ]
  %vla100.sroa.20.0 = phi <4 x double> [ %vla100.sroa.20.0.copyload, %for.cond5.preheader.us.us.us ], [ %31, %for.body16.us.us.us ]
  %vla100.sroa.18.0 = phi <4 x double> [ %vla100.sroa.18.0.copyload, %for.cond5.preheader.us.us.us ], [ %29, %for.body16.us.us.us ]
  %vla100.sroa.16.0 = phi <4 x double> [ %vla100.sroa.16.0.copyload, %for.cond5.preheader.us.us.us ], [ %27, %for.body16.us.us.us ]
  %vla100.sroa.14.0 = phi <4 x double> [ %vla100.sroa.14.0.copyload, %for.cond5.preheader.us.us.us ], [ %25, %for.body16.us.us.us ]
  %vla100.sroa.12.0 = phi <4 x double> [ %vla100.sroa.12.0.copyload, %for.cond5.preheader.us.us.us ], [ %23, %for.body16.us.us.us ]
  %vla100.sroa.10.0 = phi <4 x double> [ %vla100.sroa.10.0.copyload, %for.cond5.preheader.us.us.us ], [ %21, %for.body16.us.us.us ]
  %vla100.sroa.8.0 = phi <4 x double> [ %vla100.sroa.8.0.copyload, %for.cond5.preheader.us.us.us ], [ %19, %for.body16.us.us.us ]
  %vla100.sroa.6.0 = phi <4 x double> [ %vla100.sroa.6.0.copyload, %for.cond5.preheader.us.us.us ], [ %17, %for.body16.us.us.us ]
  %vla100.sroa.4.0 = phi <4 x double> [ %vla100.sroa.4.0.copyload, %for.cond5.preheader.us.us.us ], [ %15, %for.body16.us.us.us ]
  %vla100.sroa.0.0 = phi <4 x double> [ %vla100.sroa.0.0.copyload, %for.cond5.preheader.us.us.us ], [ %13, %for.body16.us.us.us ]
  %indvars.iv = phi i64 [ 0, %for.cond5.preheader.us.us.us ], [ %indvars.iv.next, %for.body16.us.us.us ]
  %9 = add nuw nsw i64 %indvars.iv, %6, !dbg !20
  %arrayidx20.us.us.us = getelementptr inbounds double, ptr %A, i64 %9, !dbg !21
  %10 = load double, ptr %arrayidx20.us.us.us, align 8, !dbg !21, !tbaa !22
  %vecinit.i.i.us.us.us = insertelement <4 x double> undef, double %10, i64 0, !dbg !24
  %vecinit3.i.i.us.us.us = shufflevector <4 x double> %vecinit.i.i.us.us.us, <4 x double> poison, <4 x i32> zeroinitializer, !dbg !24
  %11 = mul nsw i64 %indvars.iv, %0, !dbg !25
  %12 = add nuw nsw i64 %indvars.iv159, %11, !dbg !26
  %arrayidx32.us.us.us = getelementptr inbounds double, ptr %B, i64 %12, !dbg !27
  %arrayidx32.val.us.us.us = load <4 x double>, ptr %arrayidx32.us.us.us, align 1, !dbg !28, !tbaa !16
  %13 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.0.0), !dbg !29
  %14 = add nuw nsw i64 %12, 4, !dbg !30
  %arrayidx32.us.us.us.1 = getelementptr inbounds double, ptr %B, i64 %14, !dbg !27
  %arrayidx32.val.us.us.us.1 = load <4 x double>, ptr %arrayidx32.us.us.us.1, align 1, !dbg !28, !tbaa !16
  %15 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.1, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.4.0), !dbg !29
  %16 = add nuw nsw i64 %12, 8, !dbg !30
  %arrayidx32.us.us.us.2 = getelementptr inbounds double, ptr %B, i64 %16, !dbg !27
  %arrayidx32.val.us.us.us.2 = load <4 x double>, ptr %arrayidx32.us.us.us.2, align 1, !dbg !28, !tbaa !16
  %17 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.2, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.6.0), !dbg !29
  %18 = add nuw nsw i64 %12, 12, !dbg !30
  %arrayidx32.us.us.us.3 = getelementptr inbounds double, ptr %B, i64 %18, !dbg !27
  %arrayidx32.val.us.us.us.3 = load <4 x double>, ptr %arrayidx32.us.us.us.3, align 1, !dbg !28, !tbaa !16
  %19 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.3, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.8.0), !dbg !29
  %20 = add nuw nsw i64 %12, 16, !dbg !30
  %arrayidx32.us.us.us.4 = getelementptr inbounds double, ptr %B, i64 %20, !dbg !27
  %arrayidx32.val.us.us.us.4 = load <4 x double>, ptr %arrayidx32.us.us.us.4, align 1, !dbg !28, !tbaa !16
  %21 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.4, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.10.0), !dbg !29
  %22 = add nuw nsw i64 %12, 20, !dbg !30
  %arrayidx32.us.us.us.5 = getelementptr inbounds double, ptr %B, i64 %22, !dbg !27
  %arrayidx32.val.us.us.us.5 = load <4 x double>, ptr %arrayidx32.us.us.us.5, align 1, !dbg !28, !tbaa !16
  %23 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.5, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.12.0), !dbg !29
  %24 = add nuw nsw i64 %12, 24, !dbg !30
  %arrayidx32.us.us.us.6 = getelementptr inbounds double, ptr %B, i64 %24, !dbg !27
  %arrayidx32.val.us.us.us.6 = load <4 x double>, ptr %arrayidx32.us.us.us.6, align 1, !dbg !28, !tbaa !16
  %25 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.6, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.14.0), !dbg !29
  %26 = add nuw nsw i64 %12, 28, !dbg !30
  %arrayidx32.us.us.us.7 = getelementptr inbounds double, ptr %B, i64 %26, !dbg !27
  %arrayidx32.val.us.us.us.7 = load <4 x double>, ptr %arrayidx32.us.us.us.7, align 1, !dbg !28, !tbaa !16
  %27 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.7, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.16.0), !dbg !29
  %28 = add nuw nsw i64 %12, 32, !dbg !30
  %arrayidx32.us.us.us.8 = getelementptr inbounds double, ptr %B, i64 %28, !dbg !27
  %arrayidx32.val.us.us.us.8 = load <4 x double>, ptr %arrayidx32.us.us.us.8, align 1, !dbg !28, !tbaa !16
  %29 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.8, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.18.0), !dbg !29
  %30 = add nuw nsw i64 %12, 36, !dbg !30
  %arrayidx32.us.us.us.9 = getelementptr inbounds double, ptr %B, i64 %30, !dbg !27
  %arrayidx32.val.us.us.us.9 = load <4 x double>, ptr %arrayidx32.us.us.us.9, align 1, !dbg !28, !tbaa !16
  %31 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.9, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.20.0), !dbg !29
  %32 = add nuw nsw i64 %12, 40, !dbg !30
  %arrayidx32.us.us.us.10 = getelementptr inbounds double, ptr %B, i64 %32, !dbg !27
  %arrayidx32.val.us.us.us.10 = load <4 x double>, ptr %arrayidx32.us.us.us.10, align 1, !dbg !28, !tbaa !16
  %33 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.10, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.22.0), !dbg !29
  %34 = add nuw nsw i64 %12, 44, !dbg !30
  %arrayidx32.us.us.us.11 = getelementptr inbounds double, ptr %B, i64 %34, !dbg !27
  %arrayidx32.val.us.us.us.11 = load <4 x double>, ptr %arrayidx32.us.us.us.11, align 1, !dbg !28, !tbaa !16
  %35 = tail call <4 x double> @llvm.fma.v4f64(<4 x double> %arrayidx32.val.us.us.us.11, <4 x double> %vecinit3.i.i.us.us.us, <4 x double> %vla100.sroa.24.0), !dbg !29
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !31
  %exitcond.not = icmp eq i64 %indvars.iv.next, %0, !dbg !32
  br i1 %exitcond.not, label %for.cond13.for.cond46.preheader_crit_edge.us.us.us, label %for.body16.us.us.us, !dbg !19, !llvm.loop !33

for.cond13.for.cond46.preheader_crit_edge.us.us.us: ; preds = %for.body16.us.us.us
  store <4 x double> %13, ptr %uglygep153, align 1, !dbg !72, !tbaa !16
  store <4 x double> %15, ptr %vla100.sroa.4.0.uglygep.sroa_idx, align 1, !dbg !72, !tbaa !16
  store <4 x double> %17, ptr %vla100.sroa.6.0.uglygep.sroa_idx, align 1, !dbg !72, !tbaa !16
  store <4 x double> %19, ptr %vla100.sroa.8.0.uglygep.sroa_idx, align 1, !dbg !72, !tbaa !16
  store <4 x double> %21, ptr %vla100.sroa.10.0.uglygep.sroa_idx, align 1, !dbg !72, !tbaa !16
  store <4 x double> %23, ptr %vla100.sroa.12.0.uglygep.sroa_idx, align 1, !dbg !72, !tbaa !16
  store <4 x double> %25, ptr %vla100.sroa.14.0.uglygep.sroa_idx, align 1, !dbg !72, !tbaa !16
  store <4 x double> %27, ptr %vla100.sroa.16.0.uglygep.sroa_idx, align 1, !dbg !72, !tbaa !16
  store <4 x double> %29, ptr %vla100.sroa.18.0.uglygep.sroa_idx, align 1, !dbg !72, !tbaa !16
  store <4 x double> %31, ptr %vla100.sroa.20.0.uglygep.sroa_idx, align 1, !dbg !72, !tbaa !16
  store <4 x double> %33, ptr %vla100.sroa.22.0.uglygep.sroa_idx, align 1, !dbg !72, !tbaa !16
  store <4 x double> %35, ptr %vla100.sroa.24.0.uglygep.sroa_idx, align 1, !dbg !72, !tbaa !16
  %indvars.iv.next160 = add nuw nsw i64 %indvars.iv159, 48, !dbg !73
  %indvar.next142 = add nuw nsw i64 %indvar141, 1, !dbg !13
  %exitcond163.not = icmp eq i64 %indvar.next142, %wide.trip.count162, !dbg !74
  br i1 %exitcond163.not, label %for.cond1.for.cond.cleanup3_crit_edge.split.us.us.us, label %for.cond5.preheader.us.us.us, !dbg !13, !llvm.loop !75

for.cond1.for.cond.cleanup3_crit_edge.split.us.us.us: ; preds = %for.cond13.for.cond46.preheader_crit_edge.us.us.us
  %indvar.next = add nuw nsw i64 %indvar, 1, !dbg !77
  %exitcond167.not = icmp eq i64 %indvar.next, %0, !dbg !9
  br i1 %exitcond167.not, label %for.cond.cleanup, label %for.cond1.preheader.us.us, !dbg !10, !llvm.loop !78

for.cond.cleanup:                                 ; preds = %for.cond1.for.cond.cleanup3_crit_edge.split.us.us.us, %entry
  ret void, !dbg !80
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn
declare <4 x double> @llvm.fma.v4f64(<4 x double>, <4 x double>, <4 x double>) #1

attributes #0 = { argmemonly mustprogress nofree nosync nounwind uwtable "frame-pointer"="none" "min-legal-vector-width"="256" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+avx,+avx2,+crc32,+cx8,+fma,+fxsr,+mmx,+popcnt,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave" "tune-cpu"="generic" }
attributes #1 = { mustprogress nocallback nofree nosync nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)", isOptimized: true, flags: " --driver-mode=g++ -c -S fma.cpp -mavx2 -mfma -O3 -emit-llvm -Xclang -opaque-pointers -mllvm -opaque-pointers -o dummy.ll -mllvm -tight-loop-emitter-run=remark -qopt-report=3", runtimeVersion: 0, emissionKind: LineTablesOnly, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "fma.cpp", directory: "/localdisk2/yoonseoc/royal")
!2 = !{i32 2, !"Debug Info Version", i32 3}
!3 = !{i32 1, !"wchar_size", i32 4}
!4 = !{i32 7, !"uwtable", i32 2}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
!6 = distinct !DISubprogram(name: "cgemm", scope: !1, file: !1, line: 3, type: !7, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !8)
!7 = !DISubroutineType(types: !8)
!8 = !{}
!9 = !DILocation(line: 8, column: 21, scope: !6)
!10 = !DILocation(line: 8, column: 3, scope: !6)
!11 = !DILocation(line: 11, column: 39, scope: !6)
!12 = !DILocation(line: 15, column: 41, scope: !6)
!13 = !DILocation(line: 9, column: 5, scope: !6)
!14 = !DILocation(line: 11, column: 43, scope: !6)
!15 = !DILocation(line: 11, column: 16, scope: !6)
!16 = !{!17, !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C++ TBAA"}
!19 = !DILocation(line: 14, column: 7, scope: !6)
!20 = !DILocation(line: 15, column: 45, scope: !6)
!21 = !DILocation(line: 15, column: 37, scope: !6)
!22 = !{!23, !23, i64 0}
!23 = !{!"double", !17, i64 0}
!24 = !DILocation(line: 15, column: 22, scope: !6)
!25 = !DILocation(line: 17, column: 47, scope: !6)
!26 = !DILocation(line: 17, column: 50, scope: !6)
!27 = !DILocation(line: 17, column: 42, scope: !6)
!28 = !DILocation(line: 17, column: 25, scope: !6)
!29 = !DILocation(line: 18, column: 20, scope: !6)
!30 = !DILocation(line: 17, column: 54, scope: !6)
!31 = !DILocation(line: 14, column: 31, scope: !6)
!32 = !DILocation(line: 14, column: 25, scope: !6)
!33 = distinct !{!33, !19, !34, !35, !36}
!34 = !DILocation(line: 20, column: 7, scope: !6)
!35 = !{!"llvm.loop.mustprogress"}
!36 = distinct !{!"intel.optreport.rootnode", !37}
!37 = distinct !{!"intel.optreport", !38, !39}
!38 = !{!"intel.optreport.debug_location", !19}
!39 = !{!"intel.optreport.first_child", !40}
!40 = distinct !{!"intel.optreport.rootnode", !41}
!41 = distinct !{!"intel.optreport", !42, !44}
!42 = !{!"intel.optreport.debug_location", !43}
!43 = !DILocation(line: 16, column: 9, scope: !6)
!44 = !{!"intel.optreport.remarks", !45}
!45 = !{!"intel.optreport.remark", i32 0, !"LLorg: Loop has been completely unrolled"}
!72 = !DILocation(line: 23, column: 9, scope: !6)
!73 = !DILocation(line: 9, column: 30, scope: !6)
!74 = !DILocation(line: 9, column: 23, scope: !6)
!75 = distinct !{!75, !13, !76, !35}
!76 = !DILocation(line: 25, column: 5, scope: !6)
!77 = !DILocation(line: 8, column: 27, scope: !6)
!78 = distinct !{!78, !10, !79, !35}
!79 = !DILocation(line: 26, column: 3, scope: !6)
!80 = !DILocation(line: 27, column: 1, scope: !6)
; end INTEL_FEATURE_SW_ADVANCED
