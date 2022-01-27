; RUN: opt -enable-new-pm=0 -load-coalescing -mtriple=x86_64-unknown-linux-gnu -mcpu=skylake-avx512 -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes='load-coalescing' -mtriple=x86_64-unknown-linux-gnu -mcpu=skylake-avx512 -S < %s 2>&1 | FileCheck %s

@B = common dso_local local_unnamed_addr global [1024 x double] zeroinitializer, align 16
@C = common dso_local local_unnamed_addr global [1024 x double] zeroinitializer, align 16
@A = common dso_local local_unnamed_addr global [1024 x double] zeroinitializer, align 16

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
  br label %1

; <label>:1:                                      ; preds = %0, %1
  %indvars.iv = phi i64 [ 0, %0 ], [ %indvars.iv.next, %1 ]
  %2 = getelementptr inbounds [1024 x double], [1024 x double]* @B, i64 0, i64 %indvars.iv
  %3 = getelementptr inbounds [1024 x double], [1024 x double]* @C, i64 0, i64 %indvars.iv
  %4 = getelementptr inbounds [1024 x double], [1024 x double]* @A, i64 0, i64 %indvars.iv
  %5 = or i64 %indvars.iv, 1
  %6 = getelementptr inbounds [1024 x double], [1024 x double]* @B, i64 0, i64 %5
  %7 = bitcast double* %2 to <2 x double>*
  %8 = load <2 x double>, <2 x double>* %7, align 16, !tbaa !2
  %9 = getelementptr inbounds [1024 x double], [1024 x double]* @C, i64 0, i64 %5
  %10 = bitcast double* %3 to <2 x double>*
  %11 = load <2 x double>, <2 x double>* %10, align 16, !tbaa !2
  %12 = fadd <2 x double> %8, %11
  %13 = getelementptr inbounds [1024 x double], [1024 x double]* @A, i64 0, i64 %5
  %14 = bitcast double* %4 to <2 x double>*
  store <2 x double> %12, <2 x double>* %14, align 16, !tbaa !2
  %15 = or i64 %indvars.iv, 2
  %16 = getelementptr inbounds [1024 x double], [1024 x double]* @B, i64 0, i64 %15
  %17 = getelementptr inbounds [1024 x double], [1024 x double]* @C, i64 0, i64 %15
  %18 = or i64 %indvars.iv, 4
  %19 = getelementptr inbounds [1024 x double], [1024 x double]* @A, i64 0, i64 %18
  %20 = or i64 %indvars.iv, 3
  %21 = getelementptr inbounds [1024 x double], [1024 x double]* @B, i64 0, i64 %20
  %22 = bitcast double* %16 to <2 x double>*
  %23 = load <2 x double>, <2 x double>* %22, align 16, !tbaa !2
  %24 = getelementptr inbounds [1024 x double], [1024 x double]* @C, i64 0, i64 %20
  %25 = bitcast double* %17 to <2 x double>*
  %26 = load <2 x double>, <2 x double>* %25, align 16, !tbaa !2
  %27 = fadd <2 x double> %23, %26
  %28 = or i64 %indvars.iv, 5
  %29 = getelementptr inbounds [1024 x double], [1024 x double]* @A, i64 0, i64 %28
  %30 = bitcast double* %19 to <2 x double>*
  store <2 x double> %27, <2 x double>* %30, align 16, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 8
  %31 = icmp eq i64 %indvars.iv.next, 1024
  br i1 %31, label %32, label %1

; <label>:32:                                     ; preds = %1
  ret i32 0
}

!2 = !{!3, !3, i64 0}
!3 = !{!"double", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}

; CHECK:  [[L1:%.*]] = load <4 x double>, <4 x double>*
; CHECK-DAG:  [[SH1:%.*]] = shufflevector <4 x double> [[L1]], <4 x double> undef, <2 x i32> <i32 2, i32 3>
; CHECK-DAG:  [[SH2:%.*]] = shufflevector <4 x double> [[L1]], <4 x double> undef, <2 x i32> <i32 0, i32 1>

; CHECK-DAG:  [[L2:%.*]] = load <4 x double>, <4 x double>*
; CHECK-DAG:  [[SH4:%.*]] = shufflevector <4 x double> [[L2]], <4 x double> undef, <2 x i32> <i32 0, i32 1>
; CHECK-DAG:  [[SH3:%.*]] = shufflevector <4 x double> [[L2]], <4 x double> undef, <2 x i32> <i32 2, i32 3>
