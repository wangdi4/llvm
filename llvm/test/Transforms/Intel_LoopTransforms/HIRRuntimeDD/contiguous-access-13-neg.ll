; RUN: opt -passes='hir-ssa-deconstruction,hir-runtime-dd,print<hir>' -disable-output < %s 2>&1 | FileCheck %s

; Verify that RTDD MV doesn't happen because following two refs don't come up with a group
; of a contiguous accesses.
; (%arg)[2 * i1 + sext.i32.i64(%arg2)] and (%arg)[2 * i1 + sext.i32.i64(%arg2) + 1]
; One is load and the other is store. Only the accesses for a same type are considered for
; contiguous accesses.

; Before transformation
;
;  BEGIN REGION { }
;        + DO i1 = 0, 99, 1   <DO_LOOP>
;        |   %load = (%arg1)[i1];
;        |   %my = (%arg)[2 * i1 + sext.i32.i64(%arg2)];
;        |   (%arg)[2 * i1 + sext.i32.i64(%arg2) + 1] = %load + %my;
;        + END LOOP
;  END REGION

; tests size: 1
; Group 0 contains (1) refs:
; (%arg1)[i1]
; Group 1 contains (2) refs:
; (%arg)[2 * i1 + sext.i32.i64(%arg2)]
; (%arg)[2 * i1 + sext.i32.i64(%arg2) + 1]
; LOOPOPT_OPTREPORT: [RTDD] Loop 22: Subscript multiversioning is non-profitable
; Function: widget

; After transformation
;
; CHECK: Function: widget
; CHECK: BEGIN REGION { }
; CHECK:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK:       |   %load = (%arg1)[i1];
; CHECK:       |   %my = (%arg)[2 * i1 + sext.i32.i64(%arg2)];
; CHECK:       |   (%arg)[2 * i1 + sext.i32.i64(%arg2) + 1] = %load + %my;
; CHECK:       + END LOOP
; CHECK: END REGION

source_filename = "strided.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local void @widget(ptr nocapture noundef writeonly %arg, ptr nocapture noundef readonly %arg1, i32 noundef %arg2) local_unnamed_addr #0 {
bb:
  %sext = sext i32 %arg2 to i64
  br label %bb3

bb3:                                              ; preds = %bb3, %bb
  %phi = phi i64 [ 0, %bb ], [ %add5, %bb3 ]
  %getelementptr = getelementptr inbounds i32, ptr %arg1, i64 %phi
  %load = load i32, ptr %getelementptr, align 4, !tbaa !3
  %shl = shl nuw nsw i64 %phi, 1
  %add = add nsw i64 %shl, %sext
  %getelementptr4 = getelementptr inbounds i32, ptr %arg, i64 %add
  %my = load i32, ptr %getelementptr4, align 4, !tbaa !3
  %add5 = add nuw nsw i64 %phi, 1
  %getelementptr6 = getelementptr inbounds i32, ptr %arg1, i64 %add5
  %add8 = add nsw i64 %add, 1
  %getelementptr9 = getelementptr inbounds i32, ptr %arg, i64 %add8
  %add10 = add nuw nsw i64 %phi, 2
  %add13 = add nsw i64 %add, 1
  %getelementptr14 = getelementptr inbounds i32, ptr %arg, i64 %add13
  %sum_all = add nsw i32 %load, %my
  store i32 %sum_all, ptr %getelementptr14, align 4, !tbaa !3
  %icmp = icmp eq i64 %add5, 100
  br i1 %icmp, label %bb15, label %bb3, !llvm.loop !7

bb15:                                             ; preds = %bb3
  ret void
}

 attributes #0 = { nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

 !llvm.module.flags = !{!0, !1}
 !llvm.ident = !{!2}

 !0 = !{i32 1, !"wchar_size", i32 4}
 !1 = !{i32 7, !"uwtable", i32 2}
 !2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.0.0 (2023.x.0.YYYYMMDD)"}
 !3 = !{!4, !4, i64 0}
 !4 = !{!"int", !5, i64 0}
 !5 = !{!"omnipotent char", !6, i64 0}
 !6 = !{!"Simple C/C++ TBAA"}
 !7 = distinct !{!7, !8}
 !8 = !{!"llvm.loop.mustprogress"}
