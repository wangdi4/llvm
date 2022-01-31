; Check that the preheader of the loop is extracted before the trip count check
;
; RUN: opt -hir-ssa-deconstruction -hir-temp-cleanup -hir-idiom -hir-cg -print-after=hir-idiom -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-idiom,print<hir>,hir-cg" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
;*** IR Dump Before HIR Loop Idiom Recognition (hir-idiom) ***
;Function: main
;
;<12>         BEGIN REGION { }
;<46>               + DO i1 = 0, 55, 1   <DO_LOOP>
;<19>               |      %9 = zext.i32.i64(i1 + 1);
;<47>               |   + DO i2 = 0, zext.i32.i64((5 + (-1 * trunc.i64.i32(%9)))), 1   <DO_LOOP>  <MAX_TC_EST = 44>
;<28>               |   |   (%sw)[0][i1 + i2 + 1] = (%fr8)[0][i1 + i2];
;<47>               |   + END LOOP
;<46>               + END LOOP
;<12>         END REGION
;
;*** IR Dump After HIR Loop Idiom Recognition (hir-idiom) ***
;Function: main
;
; CHECK:    BEGIN REGION { modified }
; CHECK:           + DO i1 = 0, 55, 1   <DO_LOOP>
; CHECK-NEXT:      |   if (i1 + 1 <u 6)
; CHECK-NEXT:      |   {
; CHECK-NEXT:      |      %9 = zext.i32.i64(i1 + 1);
; CHECK-NEXT:      |      if (zext.i32.i64((5 + (-1 * trunc.i64.i32(%9)))) + 1 >u 12)
; CHECK-NEXT:      |      {
; CHECK-NEXT:      |         @llvm.memcpy.p0i8.p0i8.i64(&((i8*)(%sw)[0][i1 + 1]),  &((i8*)(%fr8)[0][i1]),  4 * zext.i32.i64((5 + (-1 * trunc.i64.i32(%9)))) + 4,  0);
; CHECK-NEXT:      |      }
; CHECK-NEXT:      |      else
; CHECK-NEXT:      |      {
; CHECK-NEXT:      |         + DO i2 = 0, zext.i32.i64((5 + (-1 * trunc.i64.i32(%9)))), 1   <DO_LOOP>  <MAX_TC_EST = 12>   <LEGAL_MAX_TC = 12> <max_trip_count = 12>
; CHECK-NEXT:      |         |   (%sw)[0][i1 + i2 + 1] = (%fr8)[0][i1 + i2];
; CHECK-NEXT:      |         + END LOOP
; CHECK-NEXT:      |      }
; CHECK-NEXT:      |   }
; CHECK-NEXT:      + END LOOP
; CHECK-NEXT: END REGION
;
;Module Before HIR
; ModuleID = 'atg_CMPLRLLVM-28394.c'
source_filename = "atg_CMPLRLLVM-28394.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [15 x i8] c"%u %u %u %u %u\00", align 1

; Function Attrs: nofree norecurse nosync nounwind uwtable willreturn writeonly mustprogress
define dso_local void @init(i32* nocapture %a, i32 %n, i32 %seed) local_unnamed_addr #0 {
entry:
  store i32 1, i32* %a, align 4, !tbaa !3
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #2 {
entry:
  %jh = alloca i32, align 4
  %j = alloca i32, align 4
  %w = alloca i32, align 4
  %jv4 = alloca i32, align 4
  %jw = alloca i32, align 4
  %sw = alloca [100 x i32], align 16
  %fr8 = alloca [100 x i32], align 16
  %0 = bitcast i32* %jh to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #5
  store i32 0, i32* %jh, align 4, !tbaa !3
  %1 = bitcast i32* %j to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1) #5
  store i32 0, i32* %j, align 4, !tbaa !3
  %2 = bitcast i32* %w to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #5
  store i32 0, i32* %w, align 4, !tbaa !3
  %3 = bitcast i32* %jv4 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3) #5
  store i32 0, i32* %jv4, align 4, !tbaa !3
  %4 = bitcast i32* %jw to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %4) #5
  store i32 0, i32* %jw, align 4, !tbaa !3
  %5 = bitcast [100 x i32]* %sw to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %5) #5
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(400) %5, i8 0, i64 400, i1 false)
  %6 = bitcast [100 x i32]* %fr8 to i8*
  call void @llvm.lifetime.start.p0i8(i64 400, i8* nonnull %6) #5
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 16 dereferenceable(400) %6, i8 0, i64 400, i1 false)
  %call = call i32 (i8*, ...) @scanf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @.str, i64 0, i64 0), i32* nonnull %jh, i32* nonnull %j, i32* nonnull %w, i32* nonnull %jv4, i32* nonnull %jw)
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv57 = phi i64 [ 40, %entry ], [ %indvars.iv.next58, %for.body ]
  %arrayidx2 = getelementptr inbounds [100 x i32], [100 x i32]* %sw, i64 0, i64 %indvars.iv57, !intel-tbaa !7
  %7 = load i32, i32* %arrayidx2, align 4, !tbaa !7
  %sub = add i32 %7, -1
  store i32 %sub, i32* %arrayidx2, align 4, !tbaa !7
  %indvars.iv.next58 = add nsw i64 %indvars.iv57, -1
  %cmp = icmp ugt i64 %indvars.iv.next58, 1
  br i1 %cmp, label %for.body, label %for.body11.preheader, !llvm.loop !9

for.body11.preheader:                             ; preds = %for.body
  br label %for.body11

for.body11:                                       ; preds = %for.body11.preheader, %for.inc23
  %8 = phi i32 [ %inc24, %for.inc23 ], [ 1, %for.body11.preheader ]
  %cmp1352 = icmp ult i32 %8, 6
  br i1 %cmp1352, label %for.body14.preheader, label %for.inc23

for.body14.preheader:                             ; preds = %for.body11
  %9 = zext i32 %8 to i64
  br label %for.body14

for.body14:                                       ; preds = %for.body14.preheader, %for.body14
  %indvars.iv = phi i64 [ %9, %for.body14.preheader ], [ %indvars.iv.next, %for.body14 ]
  %sub15 = add nuw i64 %indvars.iv, 4294967295
  %idxprom16 = and i64 %sub15, 4294967295
  %arrayidx17 = getelementptr inbounds [100 x i32], [100 x i32]* %fr8, i64 0, i64 %idxprom16, !intel-tbaa !7
  %10 = load i32, i32* %arrayidx17, align 4, !tbaa !7
  %arrayidx19 = getelementptr inbounds [100 x i32], [100 x i32]* %sw, i64 0, i64 %indvars.iv, !intel-tbaa !7
  store i32 %10, i32* %arrayidx19, align 4, !tbaa !7
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %lftr.wideiv = trunc i64 %indvars.iv.next to i32
  %exitcond.not = icmp eq i32 %lftr.wideiv, 6
  br i1 %exitcond.not, label %for.inc23.loopexit, label %for.body14, !llvm.loop !11

for.inc23.loopexit:                               ; preds = %for.body14
  br label %for.inc23

for.inc23:                                        ; preds = %for.inc23.loopexit, %for.body11
  %inc24 = add i32 %8, 1
  %cmp10 = icmp ult i32 %inc24, 57
  br i1 %cmp10, label %for.body11, label %for.end25, !llvm.loop !12

for.end25:                                        ; preds = %for.inc23
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %6) #5
  call void @llvm.lifetime.end.p0i8(i64 400, i8* nonnull %5) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %4) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #5
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly mustprogress
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #3

; Function Attrs: nofree nounwind
declare dso_local noundef i32 @scanf(i8* nocapture noundef readonly, ...) local_unnamed_addr #4

attributes #0 = { nofree norecurse nosync nounwind uwtable willreturn writeonly mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn mustprogress }
attributes #2 = { nofree nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly nofree nosync nounwind willreturn writeonly mustprogress }
attributes #4 = { nofree nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
!7 = !{!8, !4, i64 0}
!8 = !{!"array@_ZTSA100_j", !4, i64 0}
!9 = distinct !{!9, !10}
!10 = !{!"llvm.loop.mustprogress"}
!11 = distinct !{!11, !10}
!12 = distinct !{!12, !10}
