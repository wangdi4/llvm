; REQUIRES: asserts

; Test that identifies that the DTrans padded malloc optimization
; didn't find a malloc function.

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed < %s -passes=dtrans-paddedmallocop -dtrans-test-paddedmalloc -debug-only=dtrans-paddedmalloc -disable-output 2>&1 | FileCheck %s

; CHECK: dtrans-paddedmalloc: Trace for DTrans Padded Malloc
; CHECK: dtrans-paddedmalloc: Identifying alloc functions
; CHECK: No alloc functions found

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10testStruct.testStruct = type { ptr }

@globalStruct = dso_local local_unnamed_addr global %struct._ZTS10testStruct.testStruct zeroinitializer, align 8

; Function Attrs: nounwind uwtable
define internal "intel_dtrans_func_index"="1" ptr @mallocFunc(i64 %size, ptr "intel_dtrans_func_index"="2" %func) local_unnamed_addr #0 !intel.dtrans.func.type !8 {
entry:
  %tobool.not = icmp eq ptr %func, null
  br i1 %tobool.not, label %if.else, label %if.then

if.then:                                          ; preds = %entry
  %call = tail call ptr %func(i64 %size) #3, !intel_dtrans_type !10
  br label %if.end

if.else:                                          ; preds = %entry
  %call1 = tail call ptr @malloc(i64 %size)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %myptr.0 = phi ptr [ %call, %if.then ], [ %call1, %if.else ]
  ret ptr %myptr.0
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn
declare !intel.dtrans.func.type !12 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #1

; Function Attrs: mustprogress nounwind uwtable willreturn
define dso_local i32 @main() local_unnamed_addr #2 {
entry:
  store ptr null, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i64 0, i32 0), align 8, !tbaa !13
  ret i32 0
}

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress nounwind uwtable willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

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
!8 = distinct !{!6, !9}
!9 = !{!10, i32 1}
!10 = !{!"F", i1 false, i32 1, !6, !11}
!11 = !{i64 0, i32 0}
!12 = distinct !{!6}
!13 = !{!14, !15, i64 0}
!14 = !{!"struct@", !15, i64 0}
!15 = !{!"pointer@_ZTSPc", !16, i64 0}
!16 = !{!"omnipotent char", !17, i64 0}
!17 = !{!"Simple C/C++ TBAA"}
