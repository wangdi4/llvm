; RUN: opt -disable-output -enable-intel-advanced-opts  -xmain-opt-level=3 -passes="hir-ssa-deconstruction,hir-runtime-dd,print<hir>" -aa-pipeline="basic-aa,tbaa" -hir-runtime-dd-min-numivs-delinear-noalias=2 < %s 2>&1 | FileCheck %s

; Verify that the MV happens at the right level only with delinearization.
; With %load12, preconditions can not go outside of i1-loop.
; Also notice that the conditions are all from delinearization of memrefs not from aliasing.
; basic-aa and tbaa resolve aliasing between (%load)[i1] and (%arg)[%arg4 * i1 + i2 + %arg3].

; From
;
;       + DO i1 = 0, %arg2 + -1, 1   <DO_LOOP>
;       |   %load12 = (%load)[i1];
;       |
;       |      %phi22 = 0.000000e+00;
;       |   + DO i2 = 0, %load12 + -1 * %arg3 + -1, 1   <DO_LOOP>
;       |   |   %phi22 = %phi22  +  1.000000e-01;
;       |   |   (%arg)[%arg4 * i1 + i2 + %arg3] = %phi22;
;       |   + END LOOP
;       + END LOOP
;
; To
;
; CHECK: Function: barney

;         BEGIN REGION { }
;               + DO i1 = 0, %arg2 + -1, 1   <DO_LOOP>
;               |   %load12 = (%load)[i1];
;               |   if (%load12 >u %arg3)
;               |   {
;               |      %phi22 = 0.000000e+00;
;               |      if (%arg4 > 1 & %load12 + -1 * %arg3 + -1 < %arg4)  <MVTag: 34>
;               |      {
;               |         + DO i2 = 0, %load12 + -1 * %arg3 + -1, 1   <DO_LOOP>  <MVTag: 34, Delinearized: %arg>
;               |         |   %phi22 = %phi22  +  1.000000e-01;
;               |         |   (%arg)[%arg4 * i1 + i2 + %arg3] = %phi22;
;               |         + END LOOP
;               |      }
;               |      else
;               |      {
;               |         + DO i2 = 0, %load12 + -1 * %arg3 + -1, 1   <DO_LOOP>  <MVTag: 34> <nounroll> <novectorize>
;               |         |   %phi22 = %phi22  +  1.000000e-01;
;               |         |   (%arg)[%arg4 * i1 + i2 + %arg3] = %phi22;
;               |         + END LOOP
;               |      }
;               |   }
;               + END LOOP
;         END REGION

; CHECK:     DO i1 =
; CHECK: 	   if (%arg4 > 1 & %load12 + -1 * %arg3 + -1 < %arg4)  <MVTag: [[TAG:[0-9]+]]>
; CHECK:        DO i2 = 0, %load12 + -1 * %arg3 + -1, 1   <DO_LOOP>  <MVTag: [[TAG]], Delinearized: %arg>

; ModuleID = 'renamed.ll'
source_filename = "CQ243223.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.hoge = type { %struct.ham }
%struct.ham = type { %struct.ham.0 }
%struct.ham.0 = type { ptr, ptr, ptr }

define dso_local void @barney(ptr nocapture noundef writeonly %arg, ptr nocapture noundef nonnull readonly align 8 dereferenceable(24) %arg1, i64 noundef %arg2, i64 noundef %arg3, i64 noundef %arg4) local_unnamed_addr {
bb:
  %icmp = icmp eq i64 %arg2, 0
  br i1 %icmp, label %bb8, label %bb5

bb5:                                              ; preds = %bb
  %getelementptr = getelementptr inbounds %struct.hoge, ptr %arg1, i64 0, i32 0, !intel-tbaa !2
  %getelementptr6 = getelementptr inbounds %struct.ham.0, ptr %getelementptr, i64 0, i32 0, !intel-tbaa !9
  %load = load ptr, ptr %getelementptr6, align 8, !tbaa !9, !std.container.ptr !10
  br label %bb9

bb7:                                              ; preds = %bb16
  br label %bb8

bb8:                                              ; preds = %bb7, %bb
  ret void

bb9:                                              ; preds = %bb16, %bb5
  %phi = phi ptr [ %arg, %bb5 ], [ %getelementptr17, %bb16 ]
  %phi10 = phi i64 [ 0, %bb5 ], [ %add18, %bb16 ]
  %getelementptr11 = getelementptr inbounds i64, ptr %load, i64 %phi10, !intel-tbaa !11
  %load12 = load i64, ptr %getelementptr11, align 8, !tbaa !11, !std.container.ptr !10
  %icmp13 = icmp ugt i64 %load12, %arg3
  br i1 %icmp13, label %bb14, label %bb16

bb14:                                             ; preds = %bb9
  br label %bb20

bb15:                                             ; preds = %bb20
  br label %bb16

bb16:                                             ; preds = %bb15, %bb9
  %getelementptr17 = getelementptr inbounds double, ptr %phi, i64 %arg4, !intel-tbaa !13
  %add18 = add nuw i64 %phi10, 1
  %icmp19 = icmp eq i64 %add18, %arg2
  br i1 %icmp19, label %bb7, label %bb9, !llvm.loop !15

bb20:                                             ; preds = %bb20, %bb14
  %phi21 = phi i64 [ %add, %bb20 ], [ %arg3, %bb14 ]
  %phi22 = phi double [ %fadd, %bb20 ], [ 0.000000e+00, %bb14 ]
  %getelementptr23 = getelementptr inbounds double, ptr %phi, i64 %phi21
  %fadd = fadd fast double %phi22, 1.000000e-01
  store double %fadd, ptr %getelementptr23, align 8, !tbaa !13
  %add = add nuw i64 %phi21, 1
  %icmp24 = icmp eq i64 %add, %load12
  br i1 %icmp24, label %bb15, label %bb20, !llvm.loop !17
}

!llvm.module.flags = !{!0, !1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@_ZTSSt12_Vector_baseImSaImEE", !4, i64 0}
!4 = !{!"struct@_ZTSNSt12_Vector_baseImSaImEE12_Vector_implE", !5, i64 0}
!5 = !{!"struct@_ZTSNSt12_Vector_baseImSaImEE17_Vector_impl_dataE", !6, i64 0, !6, i64 8, !6, i64 16}
!6 = !{!"pointer@_ZTSPm", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!5, !6, i64 0}
!10 = !{i32 0}
!11 = !{!12, !12, i64 0}
!12 = !{!"long", !7, i64 0}
!13 = !{!14, !14, i64 0}
!14 = !{!"double", !7, i64 0}
!15 = distinct !{!15, !16}
!16 = !{!"llvm.loop.mustprogress"}
!17 = distinct !{!17, !16}
