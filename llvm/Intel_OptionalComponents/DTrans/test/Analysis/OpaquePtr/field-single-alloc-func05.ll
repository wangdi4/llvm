; REQUIRES: asserts
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-safetyanalyzer -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Show that non-escaping conditions do not invalidate the single alloc function

; CGECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS10testStruct.testStruct = type { ptr }
; CHECK: Name: struct._ZTS10testStruct.testStruct
; CHECK: Number of fields: 1
; CHECK: 0)Field LLVM Type: ptr
; CHECK: DTrans Type: i32*
; CHECK: Field info: Read Written
; CHECK: Multiple Value: [ null ] <incomplete>
; CHECK: Single Alloc Function: ptr @malloc
; CHECK: Safety data: Global instance
; CHECK: End LLVMType: %struct._ZTS10testStruct.testStruct

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS10testStruct.testStruct = type { ptr }

@globalStruct = internal local_unnamed_addr global %struct._ZTS10testStruct.testStruct zeroinitializer, align 8

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %call = call align 16 ptr @malloc(i64 100)
  store ptr %call, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i32 0, i32 0), align 8, !tbaa !8
  %i1 = load i32, ptr %call, align 4, !tbaa !13
  %tobool = icmp ne i32 %i1, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  call void @free(ptr %call)
  %i2 = load ptr, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i32 0, i32 0), align 8, !tbaa !8
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %i2, ptr align 4 %i2, i64 5, i1 false)
  %i4 = load ptr, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i32 0, i32 0), align 8, !tbaa !8
  call void @llvm.memset.p0.i64(ptr align 4 %i4, i8 0, i64 24, i1 false)
  %i6 = load ptr, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i32 0, i32 0), align 8, !tbaa !8
  call void @llvm.memmove.p0.p0.i64(ptr align 4 %i6, ptr align 4 %i6, i64 8, i1 false)
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  store ptr null, ptr getelementptr inbounds (%struct._ZTS10testStruct.testStruct, ptr @globalStruct, i32 0, i32 0), align 8, !tbaa !8
  ret i32 %i1
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn
declare !intel.dtrans.func.type !15 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #1

; Function Attrs: inaccessiblemem_or_argmemonly mustprogress nounwind willreturn
declare !intel.dtrans.func.type !17 dso_local void @free(ptr nocapture noundef "intel_dtrans_func_index"="1") local_unnamed_addr #2

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #4

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memmove.p0.p0.i64(ptr nocapture writeonly, ptr nocapture readonly, i64, i1 immarg) #3

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free") "alloc-family"="malloc" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly nofree nounwind willreturn }
attributes #4 = { argmemonly nofree nounwind willreturn writeonly }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5}
!llvm.ident = !{!7}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS10testStruct.testStruct zeroinitializer, i32 1, !6}
!6 = !{i32 0, i32 1}
!7 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!8 = !{!9, !10, i64 0}
!9 = !{!"struct@", !10, i64 0}
!10 = !{!"pointer@_ZTSPi", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !11, i64 0}
!15 = distinct !{!16}
!16 = !{i8 0, i32 1}
!17 = distinct !{!16}
