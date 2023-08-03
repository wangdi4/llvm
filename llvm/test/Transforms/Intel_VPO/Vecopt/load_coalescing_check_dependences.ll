; RUN: opt -passes='load-coalescing' -mtriple=x86_64-unknown-linux-gnu -mcpu=skylake-avx512 -S < %s 2>&1 | FileCheck %s

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
  ; load B[i+0:i+1]
  %7 = load <2 x double>, ptr %2, align 16, !tbaa !2
  %8 = getelementptr inbounds [1024 x double], ptr @C, i64 0, i64 %5
  ; load C[i+0:i+1]
  %9 = load <2 x double>, ptr %3, align 16, !tbaa !2
  %10 = fadd <2 x double> %7, %9
  %11 = getelementptr inbounds [1024 x double], ptr @A, i64 0, i64 %5
  ; store A[i+0:i+1]
  store <2 x double> %10, ptr %4, align 16, !tbaa !2



  %12 = or i64 %indvars.iv, 2
  %13 = getelementptr inbounds [1024 x double], ptr @B, i64 0, i64 %12
  %14 = getelementptr inbounds [1024 x double], ptr @C, i64 0, i64 %12
  %15 = or i64 %indvars.iv, 4
  %16 = getelementptr inbounds [1024 x double], ptr @A, i64 0, i64 %15
  %17 = or i64 %indvars.iv, 3
  %18 = getelementptr inbounds [1024 x double], ptr @B, i64 0, i64 %17

; RAW dependency with %9.
  store <2 x double> %9, ptr %13, align 16, !tbaa !2

; load B[i+2:i+3] . No dependency with %7. This should be coalesced with %7.
; So, this along with the store should be moved up by the scheduler.
  %19 = load <2 x double>, ptr %13, align 16, !tbaa !2
  %20 = getelementptr inbounds [1024 x double], ptr @C, i64 0, i64 %17

; RAW dependency with %9.
  store <2 x double> %9, ptr %14, align 16, !tbaa !2

; load C[i+2:i+3]. MEM RAW dependency with the previous store and a transient dependency with %9.
; So it should not be coalesced with load %9.
  %21 = load <2 x double>, ptr %14, align 16, !tbaa !2
  %22 = fadd <2 x double> %19, %21
  %23 = or i64 %indvars.iv, 5
  %24 = getelementptr inbounds [1024 x double], ptr @A, i64 0, i64 %23
  ; Store A[i+4:i+5]
  store <2 x double> %22, ptr %16, align 16, !tbaa !2
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

; We expect one coalesced load and 2 non-coalesced
; CHECK:  [[L0:%.*]] = load <2 x double>, ptr
; CHECK:  [[L1:%.*]] = load <4 x double>, ptr
; CHECK:  [[L2:%.*]] = load <2 x double>, ptr
