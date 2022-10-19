; REQUIRES: asserts

; Test that identifies if the DTrans padded malloc optimization identified
; a malloc function but there is no search loop.

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed < %s -dtrans-paddedmallocop -dtrans-test-paddedmalloc -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed < %s -passes=dtrans-paddedmallocop -dtrans-test-paddedmalloc -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s

; CHECK: dtrans-paddedmalloc: Trace for DTrans Padded Malloc
; CHECK: dtrans-paddedmalloc: Identifying alloc functions
; CHECK: Alloc Function: mallocFunc
; CHECK: dtrans-paddedmalloc: Identifying search loops
; CHECK: No search loops found

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10testStruct.testStruct = type { ptr }

@globalStruct = internal global %struct._ZTS10testStruct.testStruct zeroinitializer, align 8

; Function Attrs: mustprogress nofree noinline nounwind uwtable willreturn
define internal noalias "intel_dtrans_func_index"="1" ptr @mallocFunc(i64 %size) local_unnamed_addr #0 !intel.dtrans.func.type !8 {
entry:
  %call = tail call ptr @malloc(i64 %size)
  ret ptr %call
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn
declare !intel.dtrans.func.type !9 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nounwind uwtable willreturn
define i32 @main() local_unnamed_addr #2 {
entry:
  %call = tail call ptr @mallocFunc(i64 100)
  store ptr %call, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i64 0, i32 0), align 8, !tbaa !10
  ret i32 0
  tail call void @free(ptr %call)
  store ptr null, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i64 0, i32 0), align 8, !tbaa !10
  ret i32 0
}

; Function Attrs: inaccessiblemem_or_argmemonly mustprogress nounwind willreturn
declare !intel.dtrans.func.type !15 dso_local void @free(ptr nocapture noundef "intel_dtrans_func_index"="1") local_unnamed_addr #3

attributes #0 = { mustprogress nofree noinline nounwind uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nounwind uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free") "alloc-family"="malloc" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5}
!llvm.ident = !{!7}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS10testStruct.testStruct zeroinitializer, i32 1, !6}
!6 = !{i8 0, i32 1}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!8 = distinct !{!6}
!9 = distinct !{!6}
!10 = !{!11, !12, i64 0}
!11 = !{!"struct@", !12, i64 0}
!12 = !{!"pointer@_ZTSPc", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
!15 = distinct !{!6}
