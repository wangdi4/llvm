; RUN: opt -hir-ssa-deconstruction -hir-dead-store-elimination -print-after=hir-dead-store-elimination -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline="basic-aa" -passes="hir-ssa-deconstruction,hir-dead-store-elimination,print<hir-framework>" -disable-output 2>&1 < %s | FileCheck %s

; Test checks that the store on the else branch of the 'if' statment is removed,
; but the store in the end of the first i2 loop remains in place.

;            BEGIN REGION { }
;                  + DO i1 = 0, 99, 1   <DO_LOOP>
;                  |   + DO i2 = 0, 99, 1   <DO_LOOP>
;                  |   |   if (i2 > %n)
;                  |   |   {
;                  |   |      %1 = (@B)[0][i1];
;                  |   |      %x.049 = %1;
;                  |   |   }
;                  |   |   else
;                  |   |   {
;                  |   |      %2 = (@A)[0][i2];
;                  |   |      (@B)[0][i1] = %2;
;                  |   |   }
;                  |   |   (@B)[0][i1] = i2;
;                  |   + END LOOP
;                  |
;                  |   %x.049.out = %x.049;
;                  |   (@B)[0][i1] = i1;
;                  |
;                  |   + DO i2 = 0, 99, 1   <DO_LOOP>
;                  |   |   (@A)[0][i2] = i2 + %x.049.out;
;                  |   |   (@B)[0][i1] = i2;
;                  |   + END LOOP
;                  + END LOOP
;            END REGION

; CHECK: modified
; CHECK: %1 = (@B)[0][i1];
; CHECK-NOT: (@B)[0][i1] = %2;
; CHECK: (@B)[0][i1] = i2;
; CHECK: (@B)[0][i1] = i1;
; CHECK: (@B)[0][i1] = i2;

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@B = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16
@A = dso_local local_unnamed_addr global [100 x i32] zeroinitializer, align 16

; Function Attrs: nofree norecurse nosync nounwind uwtable
define dso_local void @foo(i32 noundef %n) local_unnamed_addr #0 {
entry:
  %0 = sext i32 %n to i64
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc23
  %indvars.iv53 = phi i64 [ 0, %entry ], [ %indvars.iv.next54, %for.inc23 ]
  %x.049 = phi i32 [ 0, %entry ], [ %x.2.lcssa, %for.inc23 ]
  %arrayidx8 = getelementptr inbounds [100 x i32], [100 x i32]* @B, i64 0, i64 %indvars.iv53
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %if.end
  %indvars.iv = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next, %if.end ]
  %x.146 = phi i32 [ %x.049, %for.cond1.preheader ], [ %x.2, %if.end ]
  %cmp4 = icmp sgt i64 %indvars.iv, %0
  br i1 %cmp4, label %if.then, label %if.else

if.then:                                          ; preds = %for.body3
  %1 = load i32, i32* %arrayidx8, align 4, !tbaa !3
  br label %if.end

if.else:                                          ; preds = %for.body3
  %arrayidx6 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv, !intel-tbaa !3
  %2 = load i32, i32* %arrayidx6, align 4, !tbaa !3
  store i32 %2, i32* %arrayidx8, align 4, !tbaa !3
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %x.2 = phi i32 [ %1, %if.then ], [ %x.146, %if.else ]
  %3 = trunc i64 %indvars.iv to i32
  store i32 %3, i32* %arrayidx8, align 4, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.end, label %for.body3, !llvm.loop !8

for.end:                                          ; preds = %if.end
  %x.2.lcssa = phi i32 [ %x.2, %if.end ]
  %4 = trunc i64 %indvars.iv53 to i32
  store i32 %4, i32* %arrayidx8, align 4, !tbaa !3
  br label %for.body15

for.body15:                                       ; preds = %for.end, %for.body15
  %indvars.iv50 = phi i64 [ 0, %for.end ], [ %indvars.iv.next51, %for.body15 ]
  %5 = trunc i64 %indvars.iv50 to i32
  %add = add nsw i32 %x.2.lcssa, %5
  %arrayidx17 = getelementptr inbounds [100 x i32], [100 x i32]* @A, i64 0, i64 %indvars.iv50, !intel-tbaa !3
  store i32 %add, i32* %arrayidx17, align 4, !tbaa !3
  store i32 %5, i32* %arrayidx8, align 4, !tbaa !3
  %indvars.iv.next51 = add nuw nsw i64 %indvars.iv50, 1
  %exitcond52.not = icmp eq i64 %indvars.iv.next51, 100
  br i1 %exitcond52.not, label %for.inc23, label %for.body15, !llvm.loop !10

for.inc23:                                        ; preds = %for.body15
  %indvars.iv.next54 = add nuw nsw i64 %indvars.iv53, 1
  %exitcond55.not = icmp eq i64 %indvars.iv.next54, 100
  br i1 %exitcond55.not, label %for.end25, label %for.cond1.preheader, !llvm.loop !11

for.end25:                                        ; preds = %for.inc23
  ret void
}

attributes #0 = { nofree norecurse nosync nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA100_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = distinct !{!8, !9}
!9 = !{!"llvm.loop.mustprogress"}
!10 = distinct !{!10, !9}
!11 = distinct !{!11, !9}
