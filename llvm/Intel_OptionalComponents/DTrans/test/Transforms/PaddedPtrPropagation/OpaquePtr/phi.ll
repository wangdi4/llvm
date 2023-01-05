; REQUIRES: asserts
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output -padded-pointer-info -passes="padded-pointer-prop-op" < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Checks merging of padding for PHINode

; CHECK: ==== INITIAL FUNCTION SET ====
; CHECK: Function info(foo):
; CHECK-NEXT: HasUnknownCallSites: 0
; CHECK-NEXT: Return Padding: -1
; CHECK-NEXT: Arguments' Padding:
; CHECK-NEXT: ptr %p : -1
; CHECK: ==== END OF INITIAL FUNCTION SET ====

; CHECK: ==== TRANSFORMED FUNCTION SET ====
; CHECK: Function info(foo):
; CHECK-NEXT: HasUnknownCallSites: 0
; CHECK-NEXT: Return Padding: 2
; CHECK-NEXT: Arguments' Padding:
; CHECK-NEXT: ptr %p : -1
; CHECK-NEXT: Value paddings:
; CHECK: %x.0 = phi ptr [ %i5, %sw.default ], [ %i4, %sw.bb3 ], [ %i3, %sw.bb2 ], [ %i2, %sw.bb1 ], [ %i1, %sw.bb ] :: 2
; CHECK: ==== END OF TRANSFORMED FUNCTION SET ====

@0 = private unnamed_addr constant [15 x i8] c"padded 6 bytes\00"
@1 = private unnamed_addr constant [15 x i8] c"padded 5 bytes\00"
@2 = private unnamed_addr constant [15 x i8] c"padded 3 bytes\00"
@3 = private unnamed_addr constant [15 x i8] c"padded 4 bytes\00"
@4 = private unnamed_addr constant [15 x i8] c"padded 2 bytes\00"
@.str = private unnamed_addr constant [6 x i8] c"phi.c\00"

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare ptr @llvm.ptr.annotation.p0(ptr, ptr, ptr, i32, ptr) #0

; Function Attrs: nounwind uwtable
define internal "intel_dtrans_func_index"="1" ptr @foo(ptr "intel_dtrans_func_index"="2" %p) #1 !intel.dtrans.func.type !6 {
entry:
  %i = load i32, ptr %p, align 4
  switch i32 %i, label %sw.default [
    i32 2, label %sw.bb
    i32 3, label %sw.bb1
    i32 4, label %sw.bb2
    i32 5, label %sw.bb3
  ]

sw.bb:                                            ; preds = %entry
  %i1 = tail call ptr @llvm.ptr.annotation.p0(ptr nonnull %p, ptr getelementptr inbounds ([15 x i8], ptr @0, i64 0, i64 0), ptr getelementptr inbounds ([6 x i8], ptr @.str, i64 0, i64 0), i32 5, ptr null)
  br label %sw.epilog

sw.bb1:                                           ; preds = %entry
  %i2 = tail call ptr @llvm.ptr.annotation.p0(ptr nonnull %p, ptr getelementptr inbounds ([15 x i8], ptr @1, i64 0, i64 0), ptr getelementptr inbounds ([6 x i8], ptr @.str, i64 0, i64 0), i32 8, ptr null)
  br label %sw.epilog

sw.bb2:                                           ; preds = %entry
  %i3 = tail call ptr @llvm.ptr.annotation.p0(ptr nonnull %p, ptr getelementptr inbounds ([15 x i8], ptr @4, i64 0, i64 0), ptr getelementptr inbounds ([6 x i8], ptr @.str, i64 0, i64 0), i32 11, ptr null)
  br label %sw.epilog

sw.bb3:                                           ; preds = %entry
  %i4 = tail call ptr @llvm.ptr.annotation.p0(ptr nonnull %p, ptr getelementptr inbounds ([15 x i8], ptr @2, i64 0, i64 0), ptr getelementptr inbounds ([6 x i8], ptr @.str, i64 0, i64 0), i32 14, ptr null)
  br label %sw.epilog

sw.default:                                       ; preds = %entry
  %i5 = tail call ptr @llvm.ptr.annotation.p0(ptr nonnull %p, ptr getelementptr inbounds ([15 x i8], ptr @3, i64 0, i64 0), ptr getelementptr inbounds ([6 x i8], ptr @.str, i64 0, i64 0), i32 17, ptr null)
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb3, %sw.bb2, %sw.bb1, %sw.bb
  %x.0 = phi ptr [ %i5, %sw.default ], [ %i4, %sw.bb3 ], [ %i3, %sw.bb2 ], [ %i2, %sw.bb1 ], [ %i1, %sw.bb ]
  %x.010 = call ptr @llvm.ptr.annotation.p0(ptr %x.0, ptr getelementptr inbounds ([15 x i8], ptr @4, i64 0, i64 0), ptr getelementptr inbounds ([6 x i8], ptr @.str, i64 0, i64 0), i32 0, ptr null)
  ret ptr %x.010
}

attributes #0 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #1 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!6 = distinct !{!7, !7}
!7 = !{i32 0, i32 1}
