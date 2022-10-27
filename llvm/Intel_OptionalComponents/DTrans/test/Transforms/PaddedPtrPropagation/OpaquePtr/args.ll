; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="module(dtrans-normalizeop),padded-pointer-prop-op" < %s 2>&1 | FileCheck %s

; Checks merging of the padding of function arguments

; CHECK: ==== INITIAL FUNCTION SET ====
; CHECK: Function info(caller1):
; CHECK-NEXT: HasUnknownCallSites: 0
; CHECK-NEXT: Value paddings:
; CHECK-NEXT: %t3 = call ptr @llvm.ptr.annotation.p0(ptr %arr, ptr @0, ptr @.str, i32 9, ptr null) :: 32
; CHECK-NEXT: %t1 = call ptr @llvm.ptr.annotation.p0(ptr %arrayidx, ptr @2, ptr @.str, i32 8, ptr null) :: 4
; CHECK: Function info(caller2):
; CHECK-NEXT: HasUnknownCallSites: 0
; CHECK-NEXT: Value paddings:
; CHECK-NEXT: %t3 = call ptr @llvm.ptr.annotation.p0(ptr %arr, ptr @3, ptr @.str, i32 9, ptr null) :: 16
; CHECK-NEXT: %t1 = call ptr @llvm.ptr.annotation.p0(ptr %arrayidx, ptr @1, ptr @.str, i32 8, ptr null) :: 8
; CHECK: ==== END OF INITIAL FUNCTION SET ====

; CHECK: ==== TRANSFORMED FUNCTION SET ====
; CHECK: Function info(callee):
; CHECK-NEXT: HasUnknownCallSites: 0
; CHECK-NEXT: Arguments' Padding:
; CHECK-NEXT: ptr %ip : 4
; CHECK-NEXT: ptr %fp : 16
; CHECK-NEXT: Value paddings:
; CHECK-NEXT: ptr %ip :: 4
; CHECK-NEXT: ptr %fp :: 16
; CHECK: Function info(caller1):
; CHECK-NEXT: HasUnknownCallSites: 0
; CHECK-NEXT: Value paddings:
; CHECK-NEXT: %t3 = call ptr @llvm.ptr.annotation.p0(ptr %arr, ptr @0, ptr @.str, i32 9, ptr null) :: 32
; CHECK-NEXT: %t1 = call ptr @llvm.ptr.annotation.p0(ptr %arrayidx, ptr @2, ptr @.str, i32 8, ptr null) :: 4
; CHECK: Function info(caller2):
; CHECK-NEXT: HasUnknownCallSites: 0
; CHECK-NEXT: Value paddings:
; CHECK-NEXT: %t3 = call ptr @llvm.ptr.annotation.p0(ptr %arr, ptr @3, ptr @.str, i32 9, ptr null) :: 16
; CHECK-NEXT: %t1 = call ptr @llvm.ptr.annotation.p0(ptr %arrayidx, ptr @1, ptr @.str, i32 8, ptr null) :: 8
; CHECK: ==== END OF TRANSFORMED FUNCTION SET ====


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private constant [7 x i8] c"args.c\00"
@0 = private constant [16 x i8] c"padded 32 bytes\00"
@1 = private constant [15 x i8] c"padded 8 bytes\00"
@2 = private constant [15 x i8] c"padded 4 bytes\00"
@3 = private constant [16 x i8] c"padded 16 bytes\00"

declare ptr @llvm.ptr.annotation.p0(ptr, ptr, ptr, i32, ptr)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define internal i32 @callee(ptr nocapture readonly "intel_dtrans_func_index"="1" %ip, ptr nocapture readonly "intel_dtrans_func_index"="2" %fp) local_unnamed_addr #0 !intel.dtrans.func.type !6 {
entry:
  %0 = load float, ptr %fp, align 4, !tbaa !9
  %1 = load i32, ptr %ip, align 4, !tbaa !13
  %conv = sitofp i32 %1 to float
  %cmp = fcmp fast olt float %0, %conv
  %conv1 = zext i1 %cmp to i32
  ret i32 %conv1
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn
define internal i32 @caller1() local_unnamed_addr #1 {
entry:
  %arr = alloca [16 x i32]
  %arrayidx = getelementptr [16 x i32], ptr %arr, i64 0, i64 0
  %t1 = call ptr @llvm.ptr.annotation.p0(ptr %arrayidx, ptr getelementptr ([15 x i8], ptr @2, i64 0, i64 0), ptr getelementptr ([7 x i8], ptr @.str, i64 0, i64 0), i32 8, ptr null)
  %t3 = call ptr @llvm.ptr.annotation.p0(ptr %arr, ptr getelementptr ([16 x i8], ptr @0, i64 0, i64 0), ptr getelementptr ([7 x i8], ptr @.str, i64 0, i64 0), i32 9, ptr null)
  %call = call i32 @callee(ptr %t1, ptr %t3)
  ret i32 %call
}

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn
define internal i32 @caller2() local_unnamed_addr #1 {
  %arr = alloca [16 x i32]
  %arrayidx = getelementptr [16 x i32], ptr %arr, i64 0, i64 0
  %t1 = call ptr @llvm.ptr.annotation.p0(ptr %arrayidx, ptr getelementptr ([15 x i8], ptr @1, i64 0, i64 0), ptr getelementptr ([7 x i8], ptr @.str, i64 0, i64 0), i32 8, ptr null)
  %t3 = call ptr @llvm.ptr.annotation.p0(ptr %arr, ptr getelementptr ([16 x i8], ptr @3, i64 0, i64 0), ptr getelementptr ([7 x i8], ptr @.str, i64 0, i64 0), i32 9, ptr null)
  %call = call i32 @callee(ptr %t1, ptr %t3)
  ret i32 %call
}

attributes #0 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree norecurse nosync nounwind readnone uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!6 = distinct !{!7, !8}
!7 = !{i32 0, i32 1}
!8 = !{float 0.000000e+00, i32 1}
!9 = !{!10, !10, i64 0}
!10 = !{!"float", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !11, i64 0}
