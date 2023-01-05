; REQUIRES: asserts

; RUN: opt -opaque-pointers < %s -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; Test that the bad casting analyzer does not do anything because it has
; no structure which qualifies as a candidate root type.

; CHECK: LLVMType: %struct._ZTS11mynextcoder.mynextcoder
; CHECK: Safety data: Global instance{{ *$}}
; CHECK: LLVMType: %struct._ZTS8mycoder1.mycoder1
; CHECK: Safety data: No issues found
; CHECK: LLVMType: %struct._ZTS8mycoder2.mycoder2
; CHECK: Safety data: No issues found

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._ZTS11mynextcoder.mynextcoder = type { ptr, i32, i32 }
%struct._ZTS8mycoder1.mycoder1 = type { i32, ptr }
%struct._ZTS8mycoder2.mycoder2 = type { ptr, i32 }

@localnextcoder1 = internal global %struct._ZTS11mynextcoder.mynextcoder zeroinitializer, align 8
@localnextcoder2 = internal global %struct._ZTS11mynextcoder.mynextcoder zeroinitializer, align 8
@localnextcoder3 = internal global %struct._ZTS11mynextcoder.mynextcoder zeroinitializer, align 8
@localnextcoder4 = internal global %struct._ZTS11mynextcoder.mynextcoder zeroinitializer, align 8
@myglobalint1 = internal global i32 50, align 4
@myglobalint2 = internal global i32 100, align 4

; Function Attrs: nounwind uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %dtnorm = getelementptr %struct._ZTS11mynextcoder.mynextcoder, ptr @localnextcoder1, i64 0, i32 0
  store ptr null, ptr %dtnorm, align 8
  store i32 0, ptr getelementptr inbounds (%struct._ZTS11mynextcoder.mynextcoder, ptr @localnextcoder1, i64 0, i32 1), align 8
  store i32 0, ptr getelementptr inbounds (%struct._ZTS11mynextcoder.mynextcoder, ptr @localnextcoder1, i64 0, i32 2), align 8
  %dtnorm1 = getelementptr %struct._ZTS11mynextcoder.mynextcoder, ptr @localnextcoder2, i64 0, i32 0
  store ptr null, ptr %dtnorm1, align 8
  store i32 0, ptr getelementptr inbounds (%struct._ZTS11mynextcoder.mynextcoder, ptr @localnextcoder2, i64 0, i32 1), align 8
  store i32 0, ptr getelementptr inbounds (%struct._ZTS11mynextcoder.mynextcoder, ptr @localnextcoder2, i64 0, i32 2), align 8
  %t1 = tail call fastcc i32 @myoperation(ptr nonnull @localnextcoder1)
  %t2 = tail call fastcc i32 @myoperation(ptr nonnull @localnextcoder2)
  %t3 = tail call fastcc i32 @myoperation(ptr nonnull @localnextcoder3)
  %t4 = tail call fastcc i32 @myoperation(ptr nonnull @localnextcoder4)
  %t5 = add i32 %t1, %t2
  %t6 = add i32 %t3, %t4
  %t7 = add i32 %t5, %t6
  ret i32 %t7
}

; Function Attrs: noinline nounwind uwtable
define internal fastcc i32 @myoperation(ptr nocapture readonly "intel_dtrans_func_index"="1" %arg0) unnamed_addr #2 !intel.dtrans.func.type !34 {
entry:
  %field1 = getelementptr inbounds %struct._ZTS11mynextcoder.mynextcoder, ptr %arg0, i64 0, i32 1
  %i1 = load i32, ptr %field1, align 8
  %field2 = getelementptr inbounds %struct._ZTS11mynextcoder.mynextcoder, ptr %arg0, i64 0, i32 2
  %i2 = load i32, ptr %field2, align 8
  %result = add i32 %i1, %i2
  ret i32 %result
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn
declare !intel.dtrans.func.type !35 dso_local noalias noundef "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind uwtable willreturn writeonly
define internal void @coder1_startup(ptr nocapture "intel_dtrans_func_index"="1" %arg0) #4 !intel.dtrans.func.type !36 {
entry:
  %field0 = getelementptr inbounds %struct._ZTS8mycoder1.mycoder1, ptr %arg0, i64 0, i32 0
  store i32 150, ptr %field0, align 8
  %field1 = getelementptr inbounds %struct._ZTS8mycoder1.mycoder1, ptr %arg0, i64 0, i32 1
  store ptr @myglobalint2, ptr %field1, align 8
  ret void
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind uwtable willreturn writeonly
define internal void @coder1_shutdown(ptr nocapture "intel_dtrans_func_index"="1" %arg0) #4 !intel.dtrans.func.type !37 {
entry:
  %field0 = getelementptr inbounds %struct._ZTS8mycoder1.mycoder1, ptr %arg0, i64 0, i32 0
  store i32 0, ptr %field0, align 8
  %field1 = getelementptr inbounds %struct._ZTS8mycoder1.mycoder1, ptr %arg0, i64 0, i32 1
  store ptr @myglobalint1, ptr %field1, align 8
  ret void
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind uwtable willreturn writeonly
define internal void @coder2_startup(ptr nocapture "intel_dtrans_func_index"="1" %arg0) #4 !intel.dtrans.func.type !38 {
entry:
  %field1 = getelementptr inbounds %struct._ZTS8mycoder2.mycoder2, ptr %arg0, i64 0, i32 1
  store i32 200, ptr %field1, align 8
  %field0 = getelementptr inbounds %struct._ZTS8mycoder2.mycoder2, ptr %arg0, i64 0, i32 0
  store ptr @myglobalint1, ptr %field0, align 8
  ret void
}

; Function Attrs: mustprogress nofree noinline norecurse nosync nounwind uwtable willreturn writeonly
define internal void @coder2_shutdown(ptr nocapture "intel_dtrans_func_index"="1" %arg0) #4 !intel.dtrans.func.type !40 {
entry:
  %field1 = getelementptr inbounds %struct._ZTS8mycoder2.mycoder2, ptr %arg0, i64 0, i32 1
  store i32 0, ptr %field1, align 8
  %field0 = getelementptr inbounds %struct._ZTS8mycoder2.mycoder2, ptr %arg0, i64 0, i32 0
  store ptr @myglobalint2, ptr %field0, align 8
  ret void
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress nofree noinline nounwind uwtable willreturn "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { noinline nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { inaccessiblememonly mustprogress nofree nounwind willreturn "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #4 = { mustprogress nofree noinline norecurse nosync nounwind uwtable willreturn writeonly "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5, !10, !13}
!llvm.ident = !{!14}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct._ZTS11mynextcoder.mynextcoder zeroinitializer, i32 3, !6, !11, !11}
!6 = !{i8 0, i32 1}
!7 = !{!8, i32 1}
!8 = !{!"F", i1 false, i32 1, !9, !6}
!9 = !{!"void", i32 0}
!10 = !{!"S", %struct._ZTS8mycoder1.mycoder1 zeroinitializer, i32 2, !11, !12}
!11 = !{i32 0, i32 0}
!12 = !{i32 0, i32 1}
!13 = !{!"S", %struct._ZTS8mycoder2.mycoder2 zeroinitializer, i32 2, !12, !11}
!14 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!15 = !{!16, !17, i64 0}
!16 = !{!"struct@", !17, i64 0, !20, i64 8, !20, i64 16}
!17 = !{!"pointer@_ZTSPv", !18, i64 0}
!18 = !{!"omnipotent char", !19, i64 0}
!19 = !{!"Simple C/C++ TBAA"}
!20 = !{!"pointer@_ZTSPFvPvE", !18, i64 0}
!21 = !{!16, !20, i64 8}
!22 = !{!16, !20, i64 16}
!23 = distinct !{!24}
!24 = !{%struct._ZTS11mynextcoder.mynextcoder zeroinitializer, i32 1}
!25 = !{!26, !27, i64 0}
!26 = !{!"struct@", !27, i64 0, !28, i64 8}
!27 = !{!"int", !18, i64 0}
!28 = !{!"pointer@_ZTSPi", !18, i64 0}
!29 = !{!26, !28, i64 8}
!30 = distinct !{!24}
!31 = !{!32, !27, i64 8}
!32 = !{!"struct@", !28, i64 0, !27, i64 8}
!33 = !{!27, !27, i64 0}
!34 = distinct !{!24}
!35 = distinct !{!6}
!36 = distinct !{!6}
!37 = distinct !{!6}
!38 = distinct !{!6}
!39 = !{!32, !28, i64 0}
!40 = distinct !{!6}
