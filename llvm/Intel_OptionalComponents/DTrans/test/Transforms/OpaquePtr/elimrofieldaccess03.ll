; REQUIRES: asserts
; RUN: opt -opaque-pointers < %s -whole-program-assume -intel-libirc-allowed -passes=dtrans-elim-ro-field-access-op -debug-only=elim-ro-field-access -disable-output 2>&1 | FileCheck %s

; This test verifies the DTrans eliminate read-only field access pass.
; Second IF basic block doesn't meet the criteria.

; CHECK: DTRANS-ELIM-RO-FIELD-ACCESS: Analysing lzma_alloc
; CHECK: DTRANS-ELIM-RO-FIELD-ACCESS: Second IF BB did not fit - skipping.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.lzma_allocator = type { ptr, ptr, ptr }

; Function Attrs: nounwind uwtable
define dso_local "intel_dtrans_func_index"="1" ptr @lzma_alloc(i64 %size, ptr readonly "intel_dtrans_func_index"="2" %allocator) local_unnamed_addr #0 !intel.dtrans.func.type !14 {
entry:
  %cmp = icmp eq i64 %size, 0
  %spec.store.select = select i1 %cmp, i64 1, i64 %size
  %cmp1.not = icmp eq ptr %allocator, null
  br i1 %cmp1.not, label %if.else, label %land.lhs.true

land.lhs.true:                                    ; preds = %entry
  %alloc = getelementptr inbounds %struct.lzma_allocator, ptr %allocator, i64 1, i32 0
  %i = load ptr, ptr %alloc, align 8, !tbaa !16
  %cmp2.not = icmp eq ptr %i, null
  br i1 %cmp2.not, label %if.else, label %if.then3

if.then3:                                         ; preds = %land.lhs.true
  %opaque = getelementptr inbounds %struct.lzma_allocator, ptr %allocator, i64 0, i32 2, !intel-tbaa !23
  %i1 = load ptr, ptr %opaque, align 8, !tbaa !23
  %call = tail call ptr %i(ptr %i1, i64 1, i64 %spec.store.select) #2, !intel_dtrans_type !7
  br label %if.end6

if.else:                                          ; preds = %land.lhs.true, %entry
  %call5 = tail call noalias align 16 ptr @malloc(i64 %spec.store.select) #2
  br label %if.end6

if.end6:                                          ; preds = %if.else, %if.then3
  %ptr.0 = phi ptr [ %call, %if.then3 ], [ %call5, %if.else ]
  %tmp = getelementptr inbounds %struct.lzma_allocator, ptr %allocator, i64 0, i32 2
  ret ptr %ptr.0
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn
declare !intel.dtrans.func.type !24 dso_local noalias noundef align 16 "intel_dtrans_func_index"="1" ptr @malloc(i64 noundef) local_unnamed_addr #1

attributes #0 = { nounwind uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #1 = { inaccessiblememonly mustprogress nofree nounwind willreturn "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!intel.dtrans.types = !{!5}
!llvm.ident = !{!13}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, !"Virtual Function Elim", i32 0}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 1, !"ThinLTO", i32 0}
!4 = !{i32 1, !"EnableSplitLTOUnit", i32 1}
!5 = !{!"S", %struct.lzma_allocator zeroinitializer, i32 3, !6, !10, !8}
!6 = !{!7, i32 1}
!7 = !{!"F", i1 false, i32 3, !8, !8, !9, !9}
!8 = !{i8 0, i32 1}
!9 = !{i64 0, i32 0}
!10 = !{!11, i32 1}
!11 = !{!"F", i1 false, i32 2, !12, !8, !8}
!12 = !{!"void", i32 0}
!13 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!14 = distinct !{!8, !15}
!15 = !{%struct.lzma_allocator zeroinitializer, i32 1}
!16 = !{!17, !18, i64 0}
!17 = !{!"struct@", !18, i64 0, !21, i64 8, !22, i64 16}
!18 = !{!"pointer@_ZTSPFPvS_mmE", !19, i64 0}
!19 = !{!"omnipotent char", !20, i64 0}
!20 = !{!"Simple C/C++ TBAA"}
!21 = !{!"pointer@_ZTSPFvPvS_E", !19, i64 0}
!22 = !{!"pointer@_ZTSPv", !19, i64 0}
!23 = !{!17, !22, i64 16}
!24 = distinct !{!8}
