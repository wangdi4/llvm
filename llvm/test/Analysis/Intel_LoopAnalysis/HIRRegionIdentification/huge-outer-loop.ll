; RUN: opt < %s -analyze -enable-new-pm=0 -hir-region-identification -hir-huge-loop-size=5 -xmain-opt-level=3 | FileCheck %s
; RUN: opt < %s -passes="print<hir-region-identification>" -disable-output -hir-huge-loop-size=5 -xmain-opt-level=3 2>&1 | FileCheck %s

; Test checks that the huge outer loop is not throttled considering the inner unknown loop.

; CHECK: Region 1
; CHECK:    EntryBB: %for.cond1.preheader
; CHECK:    ExitBB: %while.end

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@x = external dso_local local_unnamed_addr global i64, align 8
@a = external dso_local local_unnamed_addr global [100 x i64], align 16
@y = external dso_local local_unnamed_addr global i64, align 8
@z = external dso_local local_unnamed_addr global i64, align 8
@p = external dso_local local_unnamed_addr global i8*, align 8

; Function Attrs: nofree norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr {
entry:
  %0 = load i64, i64* @x, align 8, !tbaa !2
  %1 = load i64, i64* @y, align 8, !tbaa !2
  %2 = load i64, i64* @z, align 8, !tbaa !2
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %while.end, %entry
  %i.081 = phi i64 [ 0, %entry ], [ %inc46, %while.end ]
  %sum.080 = phi i64 [ 0, %entry ], [ %add44, %while.end ]
  br label %for.body3

for.body3:                                        ; preds = %for.body3, %for.cond1.preheader
  %j.075 = phi i64 [ 0, %for.cond1.preheader ], [ %inc, %for.body3 ]
  %add = add nuw nsw i64 %j.075, %i.081
  %arrayidx = getelementptr inbounds [100 x i64], [100 x i64]* @a, i64 0, i64 %add, !intel-tbaa !6
  %3 = load i64, i64* %arrayidx, align 8, !tbaa !6
  %add4 = add nsw i64 %3, %0
  store i64 %add4, i64* %arrayidx, align 8, !tbaa !6
  %inc = add nuw nsw i64 %j.075, 1
  %exitcond = icmp eq i64 %inc, 10
  br i1 %exitcond, label %for.body7.preheader, label %for.body3

for.body7.preheader:                              ; preds = %for.body3
  br label %for.body7

for.body7:                                        ; preds = %for.body7.preheader, %for.body7
  %k.076 = phi i64 [ %inc11, %for.body7 ], [ 2, %for.body7.preheader ]
  %sub = sub nsw i64 %i.081, %k.076
  %arrayidx8 = getelementptr inbounds [100 x i64], [100 x i64]* @a, i64 0, i64 %sub, !intel-tbaa !6
  %4 = load i64, i64* %arrayidx8, align 8, !tbaa !6
  %sub9 = sub nsw i64 %4, %1
  store i64 %sub9, i64* %arrayidx8, align 8, !tbaa !6
  %inc11 = add nuw nsw i64 %k.076, 1
  %exitcond82 = icmp eq i64 %inc11, 20
  br i1 %exitcond82, label %for.body15.preheader, label %for.body7

for.body15.preheader:                             ; preds = %for.body7
  br label %for.body15

for.body15:                                       ; preds = %for.body15.preheader, %for.body15
  %l.077 = phi i64 [ %inc20, %for.body15 ], [ 10, %for.body15.preheader ]
  %add16 = add nuw nsw i64 %l.077, %i.081
  %arrayidx17 = getelementptr inbounds [100 x i64], [100 x i64]* @a, i64 0, i64 %add16, !intel-tbaa !6
  %5 = load i64, i64* %arrayidx17, align 8, !tbaa !6
  %add18 = add nsw i64 %5, %2
  store i64 %add18, i64* %arrayidx17, align 8, !tbaa !6
  %inc20 = add nuw nsw i64 %l.077, 1
  %exitcond83 = icmp eq i64 %inc20, 30
  br i1 %exitcond83, label %for.body24.preheader, label %for.body15

for.body24.preheader:                             ; preds = %for.body15
  br label %for.body24

for.body24:                                       ; preds = %for.body24.preheader, %for.body24
  %m.078 = phi i64 [ %inc29, %for.body24 ], [ 0, %for.body24.preheader ]
  %sub25 = sub nsw i64 %i.081, %m.078
  %arrayidx26 = getelementptr inbounds [100 x i64], [100 x i64]* @a, i64 0, i64 %sub25, !intel-tbaa !6
  %6 = load i64, i64* %arrayidx26, align 8, !tbaa !6
  %sub27 = sub nsw i64 %6, %0
  store i64 %sub27, i64* %arrayidx26, align 8, !tbaa !6
  %inc29 = add nuw nsw i64 %m.078, 1
  %exitcond84 = icmp eq i64 %inc29, 10
  br i1 %exitcond84, label %for.body33.preheader, label %for.body24

for.body33.preheader:                             ; preds = %for.body24
  br label %for.body33

for.body33:                                       ; preds = %for.body33.preheader, %for.body33
  %n.079 = phi i64 [ %inc38, %for.body33 ], [ 20, %for.body33.preheader ]
  %add34 = add nuw nsw i64 %n.079, %i.081
  %arrayidx35 = getelementptr inbounds [100 x i64], [100 x i64]* @a, i64 0, i64 %add34, !intel-tbaa !6
  %7 = load i64, i64* %arrayidx35, align 8, !tbaa !6
  %add36 = add nsw i64 %7, %1
  store i64 %add36, i64* %arrayidx35, align 8, !tbaa !6
  %inc38 = add nuw nsw i64 %n.079, 1
  %exitcond85 = icmp eq i64 %inc38, 50
  br i1 %exitcond85, label %for.end39, label %for.body33

for.end39:                                        ; preds = %for.body33
  %arrayidx40 = getelementptr inbounds [100 x i64], [100 x i64]* @a, i64 0, i64 %i.081, !intel-tbaa !6
  %8 = load i64, i64* %arrayidx40, align 8, !tbaa !6
  %9 = load i8*, i8** @p, align 8, !tbaa !8
  %add.ptr = getelementptr inbounds i8, i8* %9, i64 %8, !intel-tbaa !10
  br label %while.cond

while.cond:                                       ; preds = %while.cond, %for.end39
  %storemerge = phi i8* [ %add.ptr, %for.end39 ], [ %incdec.ptr, %while.cond ]
  store i8* %storemerge, i8** @p, align 8, !tbaa !8
  %10 = load i8, i8* %storemerge, align 1, !tbaa !10
  %cmp41 = icmp eq i8 %10, 97
  %incdec.ptr = getelementptr inbounds i8, i8* %storemerge, i64 1
  br i1 %cmp41, label %while.end, label %while.cond

while.end:                                        ; preds = %while.cond
  %add44 = add nuw nsw i64 %sum.080, 97
  %inc46 = add nuw nsw i64 %i.081, 1
  %exitcond86 = icmp eq i64 %inc46, 50
  br i1 %exitcond86, label %for.end47, label %for.cond1.preheader

for.end47:                                        ; preds = %while.end
  %add44.lcssa = phi i64 [ %add44, %while.end ]
  %conv48 = trunc i64 %add44.lcssa to i32
  ret i32 %conv48
}

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.1 (YYYY.x.0.MMDD)"}
!2 = !{!3, !3, i64 0}
!3 = !{!"long", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
!6 = !{!7, !3, i64 0}
!7 = !{!"array@_ZTSA100_l", !3, i64 0}
!8 = !{!9, !9, i64 0}
!9 = !{!"pointer@_ZTSPc", !4, i64 0}
!10 = !{!4, !4, i64 0}

