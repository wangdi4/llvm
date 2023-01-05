; REQUIRES: asserts

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes="require<dtrans-safetyanalyzer>" -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s

; Check that both structures are marked as partial writes.
; For %struct._ZTS7test_01.test_01, field 0 and field 1 are 'Bottom Alloc Function' because
; they are written with a non-zero value, and field 2 is 'Top Alloc Function' because it is
; not written.
; For %struct._ZTS7test_02.test_02, field 1 is 'Single Alloc Function: ptr @malloc' because
; it is written with the return of malloc, and the other fields are 'Top Alloc Function'
; because they are written with 0 or not written.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS7test_01.test_01 = type { ptr, ptr, ptr }
; CHECK: Name: struct._ZTS7test_01.test_01
; CHECK: Number of fields: 3
; CHECK: 0)Field LLVM Type: ptr
; CHECK: DTrans Type: i8*
; CHECK: Bottom Alloc Function
; CHECK: 1)Field LLVM Type: ptr
; CHECK: DTrans Type: i8*
; CHECK: Bottom Alloc Function
; CHECK: 2)Field LLVM Type: ptr
; CHECK: DTrans Type: i8*
; CHECK: Top Alloc Function
; CHECK: Safety data: Memfunc partial write
; CHECK: End LLVMType: %struct._ZTS7test_01.test_01

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS7test_02.test_02 = type { ptr, ptr, ptr }
; CHECK: Name: struct._ZTS7test_02.test_02
; CHECK: Number of fields: 3
; CHECK: 0)Field LLVM Type: ptr
; CHECK: DTrans Type: i8*
; CHECK: Top Alloc Function
; CHECK: 1)Field LLVM Type: ptr
; CHECK: DTrans Type: i8*
; CHECK: Single Alloc Function: ptr @malloc
; CHECK: 2)Field LLVM Type: ptr
; CHECK: DTrans Type: i8*
; CHECK: Top Alloc Function
; CHECK: Safety data: Memfunc partial write
; CHECK: End LLVMType: %struct._ZTS7test_02.test_02

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS7test_01.test_01 = type { ptr, ptr, ptr }
%struct._ZTS7test_02.test_02 = type { ptr, ptr, ptr }

; Function Attrs: nounwind uwtable
define internal void @foo_01(ptr "intel_dtrans_func_index"="1" %a) local_unnamed_addr #0 !intel.dtrans.func.type !9 {
entry:
  %call = call align 16 dereferenceable_or_null(100) ptr @malloc(i64 100)
  %field1 = getelementptr inbounds %struct._ZTS7test_01.test_01, ptr %a, i64 0, i32 1, !intel-tbaa !11
  store ptr %call, ptr %field1, align 8, !tbaa !11
  %i = bitcast ptr %a to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %i, i8 16, i64 16, i1 false)
  ret void
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn
declare !intel.dtrans.func.type !16 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #1

; Function Attrs: nounwind uwtable
define internal void @foo_02(ptr "intel_dtrans_func_index"="1" %a) local_unnamed_addr #0 !intel.dtrans.func.type !17 {
entry:
  %call = call align 16 dereferenceable_or_null(100) ptr @malloc(i64 100)
  %field1 = getelementptr inbounds %struct._ZTS7test_02.test_02, ptr %a, i64 0, i32 1, !intel-tbaa !11
  store ptr %call, ptr %field1, align 8, !tbaa !11
  %i = bitcast ptr %a to ptr
  call void @llvm.memset.p0.i64(ptr noundef nonnull align 8 dereferenceable(16) %i, i8 0, i64 16, i1 false)
  ret void
}

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0.i64(ptr nocapture writeonly, i8, i64, i1 immarg) #2

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { argmemonly nofree nounwind willreturn writeonly }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5, !7}
!llvm.ident = !{!8}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS7test_01.test_01 zeroinitializer, i32 3, !6, !6, !6}
!6 = !{i8 0, i32 1}
!7 = !{!"S", %struct._ZTS7test_02.test_02 zeroinitializer, i32 3, !6, !6, !6}
!8 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!9 = distinct !{!10}
!10 = !{%struct._ZTS7test_01.test_01 zeroinitializer, i32 1}
!11 = !{!12, !13, i64 8}
!12 = !{!"struct@", !13, i64 0, !13, i64 8, !13, i64 16}
!13 = !{!"pointer@_ZTSPv", !14, i64 0}
!14 = !{!"omnipotent char", !15, i64 0}
!15 = !{!"Simple C/C++ TBAA"}
!16 = distinct !{!6}
!17 = distinct !{!18}
!18 = !{%struct._ZTS7test_02.test_02 zeroinitializer, i32 1}
