; INTEL_FEATURE_SW_DTRANS

; RUN: opt -S -passes=sroa %s 2>&1 | FileCheck %s
; Test that alloca instructions created by SROA extracting fields from a
; structure that has DTrans type metadata gets DTrans metadata associated with
; it. In this case, the type used in one of the alloca instructions is a single
; pointer type taken from a nested structure, so the DTrans metadata propagation
; needs to navigate to that structure to find the type to attach to alloca of the
; pointer type.

%struct.coder = type { %struct.filter, ptr, i64, i64, ptr, ptr }
%struct.allocator = type { ptr, ptr, ptr }
%struct.filter = type { ptr }

define void @test() {
  %coder = alloca %struct.coder, align 8
  call void @llvm.memset.p0.i64(ptr %coder, i8 0, i64 64, i1 false)
  %id = getelementptr inbounds %struct.coder, ptr %coder, i32 0, i32 1
  store ptr null, ptr %id, align 8
  ret void
}
; CHECK: %coder.sroa.{{[0-9]+}} = alloca ptr, align 8, !intel_dtrans_type ![[MD_PP64:[0-9]+]]
; CHECK: %coder.sroa.{{[0-9]+}} = alloca { i64, i64, ptr, ptr }, align 8, !intel_dtrans_type ![[MD_L:[0-9]+]]

; CHECK: ![[MD_I64:[0-9]+]] = !{i64 0, i32 0}
; CHECK: ![[MD_PSA:[0-9]+]] = !{%struct.allocator zeroinitializer, i32 1}
; CHECK: ![[MD_PPSF:[0-9]+]] = !{%struct.filter zeroinitializer, i32 2}
; CHECK: ![[MD_PP64]] = !{i64 0, i32 2}
; CHECK: ![[MD_L]] = !{!"L", i32 4, ![[MD_I64]], ![[MD_I64]], ![[MD_PSA]], ![[MD_PPSF]]}

declare !intel.dtrans.func.type !12 void @llvm.memset.p0.i64(ptr "intel_dtrans_func_index"="1" nocapture writeonly, i8, i64, i1 immarg)

!1 = !{%struct.filter zeroinitializer, i32 0}  ; %struct.filter
!2 = !{i8 0, i32 1}  ; ptr
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct.allocator zeroinitializer, i32 1}  ; ptr
!5 = !{%struct.filter zeroinitializer, i32 2}  ; ptr
!6 = !{!"F", i1 false, i32 3, !2, !2, !3, !3}  ; ptr (ptr, i64, i64)
!7 = !{!6, i32 1}  ; ptr
!8 = !{!"F", i1 false, i32 2, !9, !2, !2}  ; void (ptr, ptr)
!9 = !{!"void", i32 0}  ; void
!10 = !{!8, i32 1}  ; ptr
!11 = !{i64 0, i32 2}  ; ptr
!12 = distinct !{!2}
!13 = !{!"S", %struct.coder zeroinitializer, i32 6, !1, !2, !3, !3, !4, !5} ; { %struct.filter, ptr, i64, i64, ptr, ptr }
!14 = !{!"S", %struct.allocator zeroinitializer, i32 3, !7, !10, !2} ; { ptr, ptr, ptr }
!15 = !{!"S", %struct.filter zeroinitializer, i32 1, !11} ; { ptr }

!intel.dtrans.types = !{!13, !14, !15}

; end INTEL_FEATURE_SW_DTRANS
