; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers
; RUN: opt -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s

; Check that 'Bad mem func' is recorded as a safety check violation and
; fields are set to 'Bottom Alloc Func' when a write across field
; boundaries is done.

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test_01 = type { i8*,  i8* }
; CHECK: Name: struct.test_01
; CHECK: Number of fields: 2
; CHECK: 0)Field LLVM Type: i8*
; CHECK: Bottom Alloc Function
; CHECK: 1)Field LLVM Type: i8*
; CHECK: Bottom Alloc Function
; CHECK: Safety data: Bad memfunc size

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct.test_02 = type { i8*, i8* }
; CHECK: Name: struct.test_02
; CHECK: Number of fields: 2
; CHECK: 0)Field LLVM Type: i8*
; CHECK: Bottom Alloc Function
; CHECK: 1)Field LLVM Type: i8*
; CHECK: Bottom Alloc Function
; CHECK: Safety data: Bad memfunc size

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test_01 = type { i8*, i8* }
%struct.test_02 = type { i8*, i8* }

; Function Attrs: nounwind uwtable
define dso_local void @foo_01(%struct.test_01* %a) local_unnamed_addr #0 {
entry:
  %call = call align 16 dereferenceable_or_null(100) i8* @malloc(i64 100)
  %field0 = getelementptr inbounds %struct.test_01, %struct.test_01* %a, i64 0, i32 0, !intel-tbaa !6
  store i8* %call, i8** %field0, align 8, !tbaa !6
  %0 = bitcast %struct.test_01* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(12) %0, i8 16, i64 12, i1 false)
  ret void
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn
declare dso_local noalias noundef i8* @malloc(i64 noundef) local_unnamed_addr #1

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #2

; Function Attrs: nounwind uwtable
define dso_local void @foo_02(%struct.test_02* %a) local_unnamed_addr #0 {
entry:
  %call = call align 16 dereferenceable_or_null(100) i8* @malloc(i64 100)
  %field0 = getelementptr inbounds %struct.test_02, %struct.test_02* %a, i64 0, i32 0, !intel-tbaa !6
  store i8* %call, i8** %field0, align 8, !tbaa !6
  %0 = bitcast %struct.test_02* %a to i8*
  call void @llvm.memset.p0i8.i64(i8* noundef nonnull align 8 dereferenceable(12) %0, i8 0, i64 12, i1 false)
  ret void
}

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { argmemonly mustprogress nofree nounwind willreturn writeonly }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!6 = !{!7, !8, i64 0}
!7 = !{!"struct@", !8, i64 0, !8, i64 8}
!8 = !{!"pointer@_ZTSPv", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
