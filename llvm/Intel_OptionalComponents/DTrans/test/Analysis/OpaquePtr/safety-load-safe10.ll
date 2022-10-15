; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Verify loading element zero of a structure that is contained within another
; structure is treated as safe when the PtrTypeAnalyzer lists multiple element
; pointees, where one of the pointees is the aggregate type, and the other is
; the element zero type of the aggregate.

%struct._ZTS12lzma_coder_s.lzma_coder_s.175 = type { %struct._ZTS15lzma_lz_encoder.lzma_lz_encoder, %struct._ZTS9lzma_mf_s.lzma_mf_s, %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s }
%struct._ZTS15lzma_lz_encoder.lzma_lz_encoder = type { ptr, ptr, ptr, ptr }
%struct._ZTS9lzma_mf_s.lzma_mf_s = type { ptr, i32, i32, i32, i32, i32, i32, i32, i32, i32, ptr, ptr, ptr, ptr, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct._ZTS17lzma_next_coder_s.lzma_next_coder_s = type { ptr, i64, i64, ptr, ptr, ptr, ptr, ptr }
%struct._ZTS11lzma_filter.lzma_filter = type { i64, ptr }
%struct._ZTS14lzma_allocator.lzma_allocator = type { ptr, ptr, ptr }
%struct._ZTS10lzma_match.lzma_match = type { i32, i32 }

define i32 @lz_encode(ptr %arg) {
bb:
  %i10 = getelementptr inbounds %struct._ZTS12lzma_coder_s.lzma_coder_s.175, ptr %arg, i64 0, i32 1
  %i51 = load ptr, ptr %i10, align 8
  ret i32 0
}

; CHECK: DTRANS_StructInfo:
; CHECK: LLVMType: %struct._ZTS9lzma_mf_s.lzma_mf_s
; CHECK: Safety data: Nested structure | Has function ptr{{ *$}}
; CHECK: End LLVMType: %struct._ZTS9lzma_mf_s.lzma_mf_s

!intel.dtrans.types = !{!0, !3, !9, !24, !32, !33, !38}

!0 = !{!"S", %struct._ZTS11lzma_filter.lzma_filter zeroinitializer, i32 2, !1, !2}
!1 = !{i64 0, i32 0}
!2 = !{i8 0, i32 1}
!3 = !{!"S", %struct._ZTS14lzma_allocator.lzma_allocator zeroinitializer, i32 3, !4, !6, !2}
!4 = !{!5, i32 1}
!5 = !{!"F", i1 false, i32 3, !2, !2, !1, !1}
!6 = !{!7, i32 1}
!7 = !{!"F", i1 false, i32 2, !8, !2, !2}
!8 = !{!"void", i32 0}
!9 = !{!"S", %struct._ZTS17lzma_next_coder_s.lzma_next_coder_s zeroinitializer, i32 8, !2, !1, !1, !10, !15, !17, !19, !21}
!10 = !{!11, i32 1}
!11 = !{!"F", i1 false, i32 9, !12, !2, !13, !2, !14, !1, !2, !14, !1, !12}
!12 = !{i32 0, i32 0}
!13 = !{%struct._ZTS14lzma_allocator.lzma_allocator zeroinitializer, i32 1}
!14 = !{i64 0, i32 1}
!15 = !{!16, i32 1}
!16 = !{!"F", i1 false, i32 2, !8, !2, !13}
!17 = !{!18, i32 1}
!18 = !{!"F", i1 false, i32 1, !12, !2}
!19 = !{!20, i32 1}
!20 = !{!"F", i1 false, i32 4, !12, !2, !14, !14, !1}
!21 = !{!22, i32 1}
!22 = !{!"F", i1 false, i32 4, !12, !2, !13, !23, !23}
!23 = !{%struct._ZTS11lzma_filter.lzma_filter zeroinitializer, i32 1}
!24 = !{!"S", %struct._ZTS9lzma_mf_s.lzma_mf_s zeroinitializer, i32 23, !2, !12, !12, !12, !12, !12, !12, !12, !12, !12, !25, !29, !31, !31, !12, !12, !12, !12, !12, !12, !12, !12, !12}
!25 = !{!26, i32 1}
!26 = !{!"F", i1 false, i32 2, !12, !27, !28}
!27 = !{%struct._ZTS9lzma_mf_s.lzma_mf_s zeroinitializer, i32 1}
!28 = !{%struct._ZTS10lzma_match.lzma_match zeroinitializer, i32 1}
!29 = !{!30, i32 1}
!30 = !{!"F", i1 false, i32 2, !8, !27, !12}
!31 = !{i32 0, i32 1}
!32 = !{!"S", %struct._ZTS10lzma_match.lzma_match zeroinitializer, i32 2, !12, !12}
!33 = !{!"S", %struct._ZTS15lzma_lz_encoder.lzma_lz_encoder zeroinitializer, i32 4, !2, !34, !15, !36}
!34 = !{!35, i32 1}
!35 = !{!"F", i1 false, i32 5, !12, !2, !27, !2, !14, !1}
!36 = !{!37, i32 1}
!37 = !{!"F", i1 false, i32 2, !12, !2, !23}
!38 = !{!"S", %struct._ZTS12lzma_coder_s.lzma_coder_s.175 zeroinitializer, i32 3, !39, !40, !41}
!39 = !{%struct._ZTS15lzma_lz_encoder.lzma_lz_encoder zeroinitializer, i32 0}
!40 = !{%struct._ZTS9lzma_mf_s.lzma_mf_s zeroinitializer, i32 0}
!41 = !{%struct._ZTS17lzma_next_coder_s.lzma_next_coder_s zeroinitializer, i32 0}
