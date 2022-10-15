; REQUIRES: asserts

; RUN: opt -opaque-pointers < %s -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output -debug-only=dtrans-bca 2>&1 | FileCheck %s

; Test that %struct.test01a and %struct.test01b aren't marked as bad casting
; since the virtually bit cast argument will be used as a dead argument.

; CHECK: dtrans-bca: Begin bad casting analysis
; CHECK-NOT: dtrans-bca: Bad casting
; CHECK: dtrans-bca: End bad casting analysis

; CHECK-LABEL:  LLVMType: %struct._ZTS7test01a.test01a = type { i32, i64, i32 }
; CHECK-NOT: Safety data:{{.*}}Bad casting{{.*}}
; CHECK-LABEL:  LLVMType: %struct._ZTS7test01b.test01b = type { i32, i64, i32 }
; CHECK-NOT: Safety data:{{.*}}Bad casting{{.*}}

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS7test01b.test01b = type { i32, i64, i32 }
%struct._ZTS7test01a.test01a = type { i32, i64, i32 }

; Function Attrs: nounwind uwtable
define dso_local void @test01_a(ptr "intel_dtrans_func_index"="1" %p2) local_unnamed_addr #0 !intel.dtrans.func.type !10 {
entry:
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local void @test01(ptr "intel_dtrans_func_index"="1" %p) local_unnamed_addr #0 !intel.dtrans.func.type !12 {
entry:
  call void @test01_a(ptr %p)
  ret void
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) local_unnamed_addr #0 !intel.dtrans.func.type !14 {
entry:
  %call = call align 16 dereferenceable_or_null(16) ptr @malloc(i64 16)
  %i = bitcast ptr %call to ptr
  call void @test01(ptr %i)
  call void @free(ptr %call)
  ret i32 0
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn
declare !intel.dtrans.func.type !16 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #1

; Function Attrs: inaccessiblemem_or_argmemonly mustprogress nounwind willreturn
declare !intel.dtrans.func.type !18 dso_local void @free(ptr nocapture noundef "intel_dtrans_func_index"="1") local_unnamed_addr #2

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { inaccessiblemem_or_argmemonly mustprogress nounwind willreturn allockind("free") "alloc-family"="malloc" "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5, !8}
!llvm.ident = !{!9}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS7test01b.test01b zeroinitializer, i32 3, !6, !7, !6}
!6 = !{i32 0, i32 0}
!7 = !{i64 0, i32 0}
!8 = !{!"S", %struct._ZTS7test01a.test01a zeroinitializer, i32 3, !6, !7, !6}
!9 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!10 = distinct !{!11}
!11 = !{%struct._ZTS7test01b.test01b zeroinitializer, i32 1}
!12 = distinct !{!13}
!13 = !{%struct._ZTS7test01a.test01a zeroinitializer, i32 1}
!14 = distinct !{!15}
!15 = !{i8 0, i32 2}
!16 = distinct !{!17}
!17 = !{i8 0, i32 1}
!18 = distinct !{!17}
