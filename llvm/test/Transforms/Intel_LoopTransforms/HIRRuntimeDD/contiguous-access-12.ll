; RUN: opt -passes='hir-ssa-deconstruction,hir-runtime-dd,print<hir>' -disable-output < %s 2>&1 | FileCheck %s -check-prefix=CHECK-DIS

; This test case checks that RuntimeDD for contiguous access didn't work
; since the number of bits accessed by the contiguous stride is larger
; than the vector width supported. It uses the default value of the vector
; width, which is dictated by the TargetTransformInfo analysis (function
; getRegisterBitWidth). In this case, the sse2 indicates that the max vector
; bitwidth is 128 bits, but the number of bits accessed by the contiguous
; strides in %a is 640 bits.

; HIR before:

; BEGIN REGION { }
;       + DO i1 = 0, 99, 1   <DO_LOOP>
;       |   %0 = (%b)[i1];
;       |   (%a)[9 * i1] = %0;
;       |   %2 = (%b)[i1 + 1];
;       |   (%a)[9 * i1 + 1] = %2;
;       |   %5 = (%b)[i1 + 2];
;       |   (%a)[9 * i1 + 2] = %5;
;       |   %8 = (%b)[i1 + 3];
;       |   (%a)[9 * i1 + 3] = %8;
;       |   %11 = (%b)[i1 + 4];
;       |   (%a)[9 * i1 + 4] = %11;
;       |   %14 = (%b)[i1 + 5];
;       |   (%a)[9 * i1 + 5] = %14;
;       |   %17 = (%b)[i1 + 6];
;       |   (%a)[9 * i1 + 6] = %17;
;       |   %20 = (%b)[i1 + 7];
;       |   (%a)[9 * i1 + 7] = %20;
;       |   %23 = (%b)[i1 + 8];
;       |   (%a)[9 * i1 + 8] = %23;
;       + END LOOP
; END REGION

; RUN: opt -passes='hir-ssa-deconstruction,hir-runtime-dd,print<hir>' -hir-runtime-dd-contiguous-access-threshold=256 -disable-output < %s 2>&1 | FileCheck %s -check-prefix=CHECK-DIS

; This test case checks that RuntimeDD for contiguous access didn't work
; since the number of bits accessed by the contiguous stride (640 bits) is
; larger than the number set in -hir-runtime-dd-contiguous-access-threshold
; (256 bits).

; HIR after:

; CHECK-DIS:      BEGIN REGION { }
; CHECK-DIS-NEXT:       + DO i1 = 0, 99, 1   <DO_LOOP>
; CHECK-DIS-NEXT:       |   %0 = (%b)[i1];
; CHECK-DIS-NEXT:       |   (%a)[9 * i1] = %0;
; CHECK-DIS-NEXT:       |   %2 = (%b)[i1 + 1];
; CHECK-DIS-NEXT:       |   (%a)[9 * i1 + 1] = %2;
; CHECK-DIS-NEXT:       |   %5 = (%b)[i1 + 2];
; CHECK-DIS-NEXT:       |   (%a)[9 * i1 + 2] = %5;
; CHECK-DIS-NEXT:       |   %8 = (%b)[i1 + 3];
; CHECK-DIS-NEXT:       |   (%a)[9 * i1 + 3] = %8;
; CHECK-DIS-NEXT:       |   %11 = (%b)[i1 + 4];
; CHECK-DIS-NEXT:       |   (%a)[9 * i1 + 4] = %11;
; CHECK-DIS-NEXT:       |   %14 = (%b)[i1 + 5];
; CHECK-DIS-NEXT:       |   (%a)[9 * i1 + 5] = %14;
; CHECK-DIS-NEXT:       |   %17 = (%b)[i1 + 6];
; CHECK-DIS-NEXT:       |   (%a)[9 * i1 + 6] = %17;
; CHECK-DIS-NEXT:       |   %20 = (%b)[i1 + 7];
; CHECK-DIS-NEXT:       |   (%a)[9 * i1 + 7] = %20;
; CHECK-DIS-NEXT:       |   %23 = (%b)[i1 + 8];
; CHECK-DIS-NEXT:       |   (%a)[9 * i1 + 8] = %23;
; CHECK-DIS-NEXT:       + END LOOP
; CHECK-DIS-NEXT: END REGION

; RUN: opt -passes='hir-ssa-deconstruction,hir-runtime-dd,print<hir>' -hir-runtime-dd-contiguous-access-threshold=1024 -disable-output < %s 2>&1 | FileCheck %s -check-prefix=CHECK-ENABLED

; This test case checks that RuntimeDD for contiguous access works since
; the threshold for contiguous stride (1024 bits) is larger than the number
; of bits accessed (640 bits).

; RUN: opt -passes='hir-ssa-deconstruction,hir-runtime-dd,print<hir>' -hir-runtime-dd-contiguous-access-threshold=-1 -disable-output < %s 2>&1 | FileCheck %s -check-prefix=CHECK-ENABLED

; This test case checks that RuntimeDD for contiguous access works since
; the threshold for contiguous stride is disabled (-1).

; CHECK-ENABLED: BEGIN REGION
; CHECK-ENABLED:       %mv.test = &((%b)[107]) >=u &((%a)[0]);
; CHECK-ENABLED:       %mv.test1 = &((%a)[899]) >=u &((%b)[0]);
; CHECK-ENABLED:       %mv.and = %mv.test  &  %mv.test1;
; CHECK-ENABLED:       if (%mv.and == 0)  <MVTag: 60>
; CHECK-ENABLED:       {
; CHECK-ENABLED:          + DO i1 = 0, 99, 1   <DO_LOOP>  <MVTag: 60>
; CHECK-ENABLED:          + END LOOP
; CHECK-ENABLED:       }
; CHECK-ENABLED:       else
; CHECK-ENABLED:       {
; CHECK-ENABLED:          + DO i1 = 0, 99, 1   <DO_LOOP>  <MVTag: 60> <nounroll> <novectorize>
; CHECK-ENABLED:          + END LOOP
; CHECK-ENABLED:       }
; CHECK-ENABLED: END REGION


; ModuleID = 'simple.cpp'
source_filename = "simple.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable
define dso_local void @_Z3fooPmS_(ptr nocapture noundef writeonly %a, ptr nocapture noundef readonly %b) local_unnamed_addr #0 {
entry:
  br label %for.body

for.cond.cleanup:                                 ; preds = %for.body
  ret void

for.body:                                         ; preds = %entry, %for.body
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i64, ptr %b, i64 %indvars.iv
  %0 = load i64, ptr %arrayidx, align 8, !tbaa !3
  %1 = mul nuw nsw i64 %indvars.iv, 9
  %arrayidx2 = getelementptr inbounds i64, ptr %a, i64 %1
  store i64 %0, ptr %arrayidx2, align 8, !tbaa !3
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %arrayidx4 = getelementptr inbounds i64, ptr %b, i64 %indvars.iv.next
  %2 = load i64, ptr %arrayidx4, align 8, !tbaa !3
  %3 = add nuw nsw i64 %1, 1
  %arrayidx8 = getelementptr inbounds i64, ptr %a, i64 %3
  store i64 %2, ptr %arrayidx8, align 8, !tbaa !3
  %4 = add nuw nsw i64 %indvars.iv, 2
  %arrayidx11 = getelementptr inbounds i64, ptr %b, i64 %4
  %5 = load i64, ptr %arrayidx11, align 8, !tbaa !3
  %6 = add nuw nsw i64 %1, 2
  %arrayidx15 = getelementptr inbounds i64, ptr %a, i64 %6
  store i64 %5, ptr %arrayidx15, align 8, !tbaa !3
  %7 = add nuw nsw i64 %indvars.iv, 3
  %arrayidx18 = getelementptr inbounds i64, ptr %b, i64 %7
  %8 = load i64, ptr %arrayidx18, align 8, !tbaa !3
  %9 = add nuw nsw i64 %1, 3
  %arrayidx22 = getelementptr inbounds i64, ptr %a, i64 %9
  store i64 %8, ptr %arrayidx22, align 8, !tbaa !3
  %10 = add nuw nsw i64 %indvars.iv, 4
  %arrayidx25 = getelementptr inbounds i64, ptr %b, i64 %10
  %11 = load i64, ptr %arrayidx25, align 8, !tbaa !3
  %12 = add nuw nsw i64 %1, 4
  %arrayidx29 = getelementptr inbounds i64, ptr %a, i64 %12
  store i64 %11, ptr %arrayidx29, align 8, !tbaa !3
  %13 = add nuw nsw i64 %indvars.iv, 5
  %arrayidx32 = getelementptr inbounds i64, ptr %b, i64 %13
  %14 = load i64, ptr %arrayidx32, align 8, !tbaa !3
  %15 = add nuw nsw i64 %1, 5
  %arrayidx36 = getelementptr inbounds i64, ptr %a, i64 %15
  store i64 %14, ptr %arrayidx36, align 8, !tbaa !3
  %16 = add nuw nsw i64 %indvars.iv, 6
  %arrayidx39 = getelementptr inbounds i64, ptr %b, i64 %16
  %17 = load i64, ptr %arrayidx39, align 8, !tbaa !3
  %18 = add nuw nsw i64 %1, 6
  %arrayidx43 = getelementptr inbounds i64, ptr %a, i64 %18
  store i64 %17, ptr %arrayidx43, align 8, !tbaa !3
  %19 = add nuw nsw i64 %indvars.iv, 7
  %arrayidx46 = getelementptr inbounds i64, ptr %b, i64 %19
  %20 = load i64, ptr %arrayidx46, align 8, !tbaa !3
  %21 = add nuw nsw i64 %1, 7
  %arrayidx50 = getelementptr inbounds i64, ptr %a, i64 %21
  store i64 %20, ptr %arrayidx50, align 8, !tbaa !3
  %22 = add nuw nsw i64 %indvars.iv, 8
  %arrayidx53 = getelementptr inbounds i64, ptr %b, i64 %22
  %23 = load i64, ptr %arrayidx53, align 8, !tbaa !3
  %24 = add nuw nsw i64 %1, 8
  %arrayidx57 = getelementptr inbounds i64, ptr %a, i64 %24
  store i64 %23, ptr %arrayidx57, align 8, !tbaa !3
  %exitcond.not = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond.not, label %for.cond.cleanup, label %for.body, !llvm.loop !7
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind memory(argmem: readwrite) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"long", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
