; RUN: opt -passes="hir-ssa-deconstruction,hir-pre-vec-complete-unroll,hir-vec-dir-insert,hir-vplan-vec,hir-scalarrepl-array,print<hir>" -aa-pipeline="basic-aa" -disable-output < %s 2>&1 | FileCheck %s
;
; This is a bug triggerd when enhancing HIR Scalar Replacement Support for vector types. The bug starts from HIRLoopLocality
; pass, but really is in the CE compare function.
; When sorting a group of DDRefs, the algorithm tries to compare scalar CE and vector CE. Merge function should allow to
; merge scalar CE into vector CE, but not the other way around. Compare function now has different logic from the
; merge function, and no longer sharing the mergeable logic.
;
; The fix works around the merge logic and instead uses direct type comparision across scalar CE(s) and vector CE(s)
; to decide the comparison result.
;
; E.g. comparision between
; (<8 x i32>*)(%coef)[0][10][%.vec]
; (<8 x i32>*)(%coef)[0][1][i2 + 10]
; ...
; where %.vec = (<8 x i32>*)(@indi)[0][i2 + 10];
;
; With the fix, this LIT test will no longer crash during compilation.
;
; [Source code: test.c]
;#include <stdio.h>
;int indi[30];
;int val[30][30][30][30];
;int main(){
;  int i, j, k, n, ii, col, index;
;  int coef[30][30];
;  int data[30][30];
;
;  for(n=0;n<30;n++) {
;    for(i=10;i<20;i++) {
;      if(indi[i] != -9999999) {
;        for(k=10; k>0; k--) {
;          ii = indi[i];
;
;          col = coef[k][ii] * data[ii][n];
;          for(index=10; index>0; index--) {
;            val[n][k][index][i] = coef[index][i] * col;
;          }
;        }
;      }
;    }
;  }
;}
;
; compile cmd: icx -c  -D__cdecl=" " -O2 -xAVX test.c
;

; CHECK:          BEGIN REGION { modified }

; === ---------------------------------------------------------------- ===
; Following is the LLVM's input code!
; === ---------------------------------------------------------------- ===
;
;Module Before HIR
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@indi = dso_local local_unnamed_addr global [30 x i32] zeroinitializer, align 16
@val = dso_local local_unnamed_addr global [30 x [30 x [30 x [30 x i32]]]] zeroinitializer, align 16

; Function Attrs: nofree nosync nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %coef = alloca [30 x [30 x i32]], align 16
  %data = alloca [30 x [30 x i32]], align 16
  %0 = bitcast ptr %coef to ptr
  call void @llvm.lifetime.start.p0(i64 3600, ptr nonnull %0) #2
  %1 = bitcast ptr %data to ptr
  call void @llvm.lifetime.start.p0(i64 3600, ptr nonnull %1) #2
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc39
  %indvars.iv74 = phi i64 [ 0, %entry ], [ %indvars.iv.next75, %for.inc39 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.inc37
  %indvars.iv72 = phi i64 [ 10, %for.cond1.preheader ], [ %indvars.iv.next73, %for.inc37 ]
  %arrayidx = getelementptr inbounds [30 x i32], ptr @indi, i64 0, i64 %indvars.iv72, !intel-tbaa !3
  %2 = load i32, ptr %arrayidx, align 4, !tbaa !3
  %cmp4.not = icmp eq i32 %2, -9999999
  br i1 %cmp4.not, label %for.inc37, label %for.cond5.preheader

for.cond5.preheader:                              ; preds = %for.body3
  %idxprom12 = sext i32 %2 to i64
  %arrayidx17 = getelementptr inbounds [30 x [30 x i32]], ptr %data, i64 0, i64 %idxprom12, i64 %indvars.iv74, !intel-tbaa !8
  %3 = load i32, ptr %arrayidx17, align 4, !tbaa !8
  br label %for.body7

for.body7:                                        ; preds = %for.cond5.preheader, %for.inc34
  %indvars.iv70 = phi i64 [ 10, %for.cond5.preheader ], [ %indvars.iv.next71, %for.inc34 ]
  %arrayidx13 = getelementptr inbounds [30 x [30 x i32]], ptr %coef, i64 0, i64 %indvars.iv70, i64 %idxprom12, !intel-tbaa !8
  %4 = load i32, ptr %arrayidx13, align 4, !tbaa !8
  %mul = mul nsw i32 %3, %4
  br label %for.body20

for.body20:                                       ; preds = %for.body7, %for.body20
  %indvars.iv = phi i64 [ 10, %for.body7 ], [ %indvars.iv.next, %for.body20 ]
  %arrayidx24 = getelementptr inbounds [30 x [30 x i32]], ptr %coef, i64 0, i64 %indvars.iv, i64 %indvars.iv72, !intel-tbaa !8
  %5 = load i32, ptr %arrayidx24, align 4, !tbaa !8
  %mul25 = mul nsw i32 %mul, %5
  %arrayidx33 = getelementptr inbounds [30 x [30 x [30 x [30 x i32]]]], ptr @val, i64 0, i64 %indvars.iv74, i64 %indvars.iv70, i64 %indvars.iv, i64 %indvars.iv72, !intel-tbaa !10
  store i32 %mul25, ptr %arrayidx33, align 4, !tbaa !10
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %cmp19 = icmp ugt i64 %indvars.iv, 1
  br i1 %cmp19, label %for.body20, label %for.inc34, !llvm.loop !13

for.inc34:                                        ; preds = %for.body20
  %indvars.iv.next71 = add nsw i64 %indvars.iv70, -1
  %cmp6 = icmp ugt i64 %indvars.iv70, 1
  br i1 %cmp6, label %for.body7, label %for.inc37.loopexit, !llvm.loop !15

for.inc37.loopexit:                               ; preds = %for.inc34
  br label %for.inc37

for.inc37:                                        ; preds = %for.inc37.loopexit, %for.body3
  %indvars.iv.next73 = add nuw nsw i64 %indvars.iv72, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next73, 20
  br i1 %exitcond.not, label %for.inc39, label %for.body3, !llvm.loop !16

for.inc39:                                        ; preds = %for.inc37
  %indvars.iv.next75 = add nuw nsw i64 %indvars.iv74, 1
  %exitcond76.not = icmp eq i64 %indvars.iv.next75, 30
  br i1 %exitcond76.not, label %for.end41, label %for.cond1.preheader, !llvm.loop !17

for.end41:                                        ; preds = %for.inc39
  call void @llvm.lifetime.end.p0(i64 3600, ptr nonnull %1) #2
  call void @llvm.lifetime.end.p0(i64 3600, ptr nonnull %0) #2
  ret i32 0
}

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn mustprogress
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nofree nosync nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="corei7-avx" "target-features"="+avx,+cx16,+cx8,+fxsr,+mmx,+pclmul,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly nofree nosync nounwind willreturn mustprogress }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!3 = !{!4, !5, i64 0}
!4 = !{!"array@_ZTSA30_i", !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}
!8 = !{!9, !5, i64 0}
!9 = !{!"array@_ZTSA30_A30_i", !4, i64 0}
!10 = !{!11, !5, i64 0}
!11 = !{!"array@_ZTSA30_A30_A30_A30_i", !12, i64 0}
!12 = !{!"array@_ZTSA30_A30_A30_i", !9, i64 0}
!13 = distinct !{!13, !14}
!14 = !{!"llvm.loop.mustprogress"}
!15 = distinct !{!15, !14}
!16 = distinct !{!16, !14}
!17 = distinct !{!17, !14}
