; RUN: opt -passes=instcombine -S %s | FileCheck %s
; CHECK-NOT: call{{.*}}powi

; powi doesn't vectorize, we would prefer to avoid it for IA targets.

target datalayout = "e-m:w-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc19.33.31629"

@global = external dso_local local_unnamed_addr global float, align 4

; Function Attrs: mustprogress nounwind uwtable
define dso_local noundef nofpclass(nan inf) double @baz(ptr noundef %arg, ptr noundef %arg1, i32 noundef %arg2, i32 noundef %arg3) local_unnamed_addr #0 {
bb:
  br label %bb4

bb4:                                              ; preds = %bb15, %bb
  %phi = phi i32 [ 0, %bb ], [ %add, %bb15 ]
  %icmp = icmp ult i32 %phi, %arg2
  br i1 %icmp, label %bb6, label %bb5

bb5:                                              ; preds = %bb4
  ret double 0.000000e+00

bb6:                                              ; preds = %bb4
  %zext = zext i32 %phi to i64
  %getelementptr = getelementptr inbounds i32, ptr %arg, i64 %zext
  %load = load i32, ptr %getelementptr, align 4, !tbaa !5
  %icmp7 = icmp sgt i32 %load, 3
  %select = select i1 %icmp7, i32 %arg3, i32 0
  %icmp8 = icmp eq i32 %load, 0
  br i1 %icmp8, label %bb12, label %bb9

bb9:                                              ; preds = %bb6
  %load10 = load float, ptr @global, align 4, !tbaa !9
  %fpext = fpext float %load10 to double
  %sub = sub nsw i32 0, %load
  %sub11 = sub nsw i32 %sub, %select
  %sitofp = sitofp i32 %sub11 to double
  %call = call fast double @llvm.pow.f64(double %fpext, double %sitofp)
  %fmul = fmul fast double 1.000000e+30, %call
  br label %bb12

bb12:                                             ; preds = %bb9, %bb6
  %phi13 = phi fast double [ %fmul, %bb9 ], [ 1.000000e+30, %bb6 ]
  %fcmp = fcmp fast olt double %phi13, 1.000000e+10
  br i1 %fcmp, label %bb14, label %bb15

bb14:                                             ; preds = %bb12
  br label %bb15

bb15:                                             ; preds = %bb14, %bb12
  %phi16 = phi double [ 1.000000e+10, %bb14 ], [ %phi13, %bb12 ]
  %fptoui = fptoui double %phi16 to i32
  %getelementptr17 = getelementptr inbounds i32, ptr %arg1, i64 %zext
  store i32 %fptoui, ptr %getelementptr17, align 4, !tbaa !5
  %add = add i32 %phi, 1
  br label %bb4, !llvm.loop !11
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.pow.f64(double, double) #1

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+sse" "unsafe-fp-math"="true" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 2}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 1, !"MaxTLSAlign", i32 65536}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"float", !7, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}
