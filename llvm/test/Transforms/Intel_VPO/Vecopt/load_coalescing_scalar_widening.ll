; RUN: opt -passes='load-coalescing' -mtriple=x86_64-unknown-linux-gnu -mcpu=skylake-avx512 -load-coalescing-allow-scalars -S < %s 2>&1 | FileCheck %s

@B = common dso_local local_unnamed_addr global [1024 x double] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1024 x double] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [1024 x double] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
  br label %1

; <label>:1:                                      ; preds = %0, %1
  %indvars.iv = phi i64 [ 0, %0 ], [ %indvars.iv.next, %1 ]
  %2 = getelementptr inbounds [1024 x double], ptr @B, i64 0, i64 %indvars.iv
  %3 = getelementptr inbounds [1024 x double], ptr @C, i64 0, i64 %indvars.iv
  %4 = getelementptr inbounds [1024 x double], ptr @A, i64 0, i64 %indvars.iv
  %5 = or i64 %indvars.iv, 1
  %6 = getelementptr inbounds [1024 x double], ptr @B, i64 0, i64 %5
  %7 = load double, ptr %2, align 16, !tbaa !2
  %8 = getelementptr inbounds [1024 x double], ptr @C, i64 0, i64 %5
  %9 = load double, ptr %3, align 16, !tbaa !2
  %10 = fadd double %7, %9
  %11 = getelementptr inbounds [1024 x double], ptr @A, i64 0, i64 %5
  store double %10, ptr %4, align 16, !tbaa !2
  %12 = or i64 %indvars.iv, 1
  %13 = getelementptr inbounds [1024 x double], ptr @B, i64 0, i64 %12
  %14 = getelementptr inbounds [1024 x double], ptr @C, i64 0, i64 %12
  %15 = or i64 %indvars.iv, 4
  %16 = getelementptr inbounds [1024 x double], ptr @A, i64 0, i64 %15
  %17 = or i64 %indvars.iv, 3
  %18 = getelementptr inbounds [1024 x double], ptr @B, i64 0, i64 %17
  %19 = load double, ptr %13, align 16, !tbaa !2
  %20 = getelementptr inbounds [1024 x double], ptr @C, i64 0, i64 %17
  %21 = load double, ptr %14, align 16, !tbaa !2
  %22 = fadd double %19, %21
  %23 = or i64 %indvars.iv, 5
  %24 = getelementptr inbounds [1024 x double], ptr @A, i64 0, i64 %23
  store double %22, ptr %16, align 16, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 8
  %25 = icmp eq i64 %indvars.iv.next, 1024
  br i1 %25, label %26, label %1

; <label>:32:                                     ; preds = %1
  ret i32 0
}

!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

; CHECK:  [[L1:%.*]] = load <2 x double>, ptr
; CHECK-DAG:  [[SH1:%.*]] = extractelement <2 x double> [[L1]], i32 1
; CHECK-DAG:  [[SH2:%.*]] = extractelement <2 x double> [[L1]], i32 0

; CHECK-DAG:  [[L2:%.*]] = load <2 x double>, ptr
; CHECK-DAG:  [[SH3:%.*]] = extractelement <2 x double> [[L2]], i32 1
; CHECK-DAG:  [[SH4:%.*]] = extractelement <2 x double> [[L2]], i32 0
