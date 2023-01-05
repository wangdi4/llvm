; REQUIRES: asserts

; Test that identifies that the DTrans padded malloc optimization was applied
; outside the OpenMP region.

; RUN: opt -opaque-pointers < %s -whole-program-assume -intel-libirc-allowed -passes=dtrans-paddedmallocop -dtrans-test-paddedmalloc -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s

; CHECK: dtrans-paddedmalloc: Trace for DTrans Padded Malloc
; CHECK: dtrans-paddedmalloc: Identifying alloc functions
; CHECK: Alloc Function: mallocFunc
; CHECK: dtrans-paddedmalloc: Identifying search loops
; CHECK: Search loop found in: searchloop
; CHECK: dtrans-paddedmalloc: Global variable: __Intel_PaddedMallocCounter
; CHECK: dtrans-paddedmalloc: Interface function: __Intel_PaddedMallocInterface
; CHECK: dtrans-paddedmalloc: Applying padded malloc
; CHECK: Function updated: mallocFunc

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10testStruct.testStruct = type { ptr }

@globalStruct = internal global %struct._ZTS10testStruct.testStruct zeroinitializer, align 8
@arr1 = internal global [10 x i32] zeroinitializer, align 16, !intel_dtrans_type !0
@arr2 = internal global [10 x i32] zeroinitializer, align 16, !intel_dtrans_type !0

; Function Attrs: mustprogress noinline uwtable
define internal "intel_dtrans_func_index"="1" ptr @mallocFunc(i64 %size) local_unnamed_addr #0 !intel.dtrans.func.type !11 {
entry:
  %call = tail call ptr @malloc(i64 %size)
  ret ptr %call
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare !intel.dtrans.func.type !12 dso_local "intel_dtrans_func_index"="1" ptr @malloc(i64) local_unnamed_addr #6

; Function Attrs: mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn
define internal zeroext i1 @searchloop() local_unnamed_addr #3 {
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.cond, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.cond ]
  %idxprom = zext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds [10 x i32], ptr @arr1, i64 0, i64 %idxprom
  %i = load i32, ptr %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [10 x i32], ptr @arr2, i64 0, i64 %idxprom
  %i1 = load i32, ptr %arrayidx2, align 4
  %cmp = icmp eq i32 %i, %i1
  %inc = add i32 %i.0, 1
  br i1 %cmp, label %if.then, label %for.cond, !llvm.loop !13

if.then:                                          ; preds = %for.cond
  ret i1 true
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind readnone uwtable willreturn
define internal void @_Z9doNothingv() local_unnamed_addr #4 {
entry:
  ret void
}

; Function Attrs: mustprogress norecurse uwtable
define dso_local i32 @main() local_unnamed_addr #5 {
bb:
  %i = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  br label %DIR.OMP.PARALLEL.START

DIR.OMP.PARALLEL.START:                           ; preds = %bb
  call void @_Z9doNothingv()
  br label %DIR.OMP.END.PARALLEL.EXIT

DIR.OMP.END.PARALLEL.EXIT:                        ; preds = %DIR.OMP.PARALLEL.START
  call void @llvm.directive.region.exit(token %i) [ "DIR.OMP.END.PARALLEL"() ]
  br label %DIR.OMP.END.PARALLEL.AFTEROMP

DIR.OMP.END.PARALLEL.AFTEROMP:                    ; preds = %DIR.OMP.END.PARALLEL.EXIT
  %call = call ptr @mallocFunc(i64 100)
  store ptr %call, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i64 0, i32 0), align 8
  call void @_Z4freePc(ptr %call)
  store ptr null, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i64 0, i32 0), align 8
  ret i32 0
}

declare !intel.dtrans.func.type !13 dso_local void @_Z4freePc(ptr "intel_dtrans_func_index"="1") local_unnamed_addr #2

attributes #0 = { mustprogress noinline uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { mustprogress nofree norecurse nosync nounwind readonly uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { mustprogress nofree noinline norecurse nosync nounwind readnone uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { mustprogress norecurse uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #6 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!2, !3, !4, !5, !6, !7}
!intel.dtrans.types = !{!8}
!llvm.ident = !{!10}
!nvvm.annotations = !{}

!0 = !{!"A", i32 10, !1}
!1 = !{i32 0, i32 0}
!2 = !{i32 1, !"wchar_size", i32 4}
!3 = !{i32 1, !"Virtual Function Elim", i32 0}
!4 = !{i32 7, !"openmp", i32 50}
!5 = !{i32 7, !"uwtable", i32 1}
!6 = !{i32 1, !"ThinLTO", i32 0}
!7 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!8 = !{!"S", %struct._ZTS10testStruct.testStruct zeroinitializer, i32 1, !9}
!9 = !{i8 0, i32 1}
!10 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!11 = distinct !{!9}
!12 = distinct !{!9}
!13 = distinct !{!9}
