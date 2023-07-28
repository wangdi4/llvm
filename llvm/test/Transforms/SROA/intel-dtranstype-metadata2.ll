; INTEL_FEATURE_SW_DTRANS

; RUN: opt -S -passes=sroa %s 2>&1 | FileCheck %s 

; Test that a literal structure created by SROA by extracting fields from a
; structure that has DTrans type metadata gets DTrans metadata associated with
; in when the literal structure contains a pointer type. Also, verify that an
; alloca of a pointer created from extracting a single field from a structure
; gets DTrans metadata.

%struct.coder = type { ptr, i64, i64, ptr, ptr, ptr, ptr, ptr }
%struct.allocator = type { ptr, ptr, ptr }
%struct.filter = type { i64, ptr }

define void @test() {
  %coder = alloca %struct.coder, align 8
  call void @llvm.memset.p0.i64(ptr %coder, i8 0, i64 64, i1 false)
  %id = getelementptr inbounds %struct.coder, ptr %coder, i32 0, i32 1
  store i64 0, ptr %id, align 8
  ret void
}

; CHECK: %coder.sroa.{{[0-9]+}} = alloca ptr, align 8, !intel_dtrans_type ![[MD_PSF:[0-9]+]]
; CHECK: %coder.sroa.{{[0-9]+}} = alloca { i64, ptr, ptr, ptr, ptr, ptr }, align 8, !intel_dtrans_type ![[MD_L:[0-9]+]]

; CHECK: ![[MD_PSF]] = !{%struct.filter zeroinitializer, i32 1}
; CHECK: ![[MD_I64:[0-9]+]] = !{i64 0, i32 0}
; CHECK: ![[MD_PSA:[0-9]+]] = !{%struct.allocator zeroinitializer, i32 1}
; CHECK: ![[MD_P32:[0-9]+]] = !{i32 0, i32 1}
; CHECK: ![[MD_L]] = !{!"L", i32 6, ![[MD_I64]], ![[MD_PSA]], ![[MD_PSA]], ![[MD_P32]], ![[MD_P32]], ![[MD_PSF]]}

declare !intel.dtrans.func.type !11 void @llvm.memset.p0.i64(ptr "intel_dtrans_func_index"="1" nocapture writeonly, i8, i64, i1 immarg)

!1 = !{%struct.filter zeroinitializer, i32 1}  ; ptr
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.allocator zeroinitializer, i32 1}  ; ptr
!4 = !{i32 0, i32 1}  ; ptr
!5 = !{!"F", i1 false, i32 3, !6, !6, !2, !2}  ; ptr (ptr, i64, i64)
!6 = !{i8 0, i32 1}  ; ptr
!7 = !{!5, i32 1}  ; ptr
!8 = !{!"F", i1 false, i32 2, !9, !6, !6}  ; void (ptr, ptr)
!9 = !{!"void", i32 0}  ; void
!10 = !{!8, i32 1}  ; ptr
!11 = distinct !{!6}
!12 = !{!"S", %struct.coder zeroinitializer, i32 8, !1, !2, !2, !3, !3, !4, !4, !1} ; { ptr, i64, i64, ptr, ptr, ptr, ptr, ptr }
!13 = !{!"S", %struct.allocator zeroinitializer, i32 3, !7, !10, !6} ; { ptr, ptr, ptr }
!14 = !{!"S", %struct.filter zeroinitializer, i32 2, !2, !6} ; { i64, ptr }

!intel.dtrans.types = !{!12, !13, !14}

; end INTEL_FEATURE_SW_DTRANS
