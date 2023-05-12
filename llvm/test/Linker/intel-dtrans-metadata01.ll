; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; RUN: llvm-link -S %s %S/Inputs/intel-dtrans-metadata01a.ll | FileCheck %s

; Test that when merging in declarations that have DTrans metadata attachments
; and the types referenced in the declaration get remapped based on types
; already merged in, the metadata attachments also get remapped appropriately,
; without leaving references to deleted types.

%struct.op = type { ptr, ptr, ptr, i64, i16, i8, i8 }

define "intel_dtrans_func_index"="1" ptr @test01(ptr "intel_dtrans_func_index"="2" returned %0) !intel.dtrans.func.type !7 {
  ret ptr %0
}

!1 = !{%struct.op zeroinitializer, i32 1}  ; %struct.op*
!2 = !{!"F", i1 false, i32 0, !1}  ; %struct.op* ()
!3 = !{!2, i32 1}  ; %struct.op* ()*
!4 = !{i64 0, i32 0}  ; i64
!5 = !{i16 0, i32 0}  ; i16
!6 = !{i8 0, i32 0}  ; i8
!7 = distinct !{!1, !1}
!8 = !{!"S", %struct.op zeroinitializer, i32 7, !1, !1, !3, !4, !5, !6, !6} ; { %struct.op*, %struct.op*, %struct.op* ()*, i64, i16, i8, i8 }

!intel.dtrans.types = !{!8}

; CHECK: declare !intel.dtrans.func.type ![[TEST02_MD:[0-9]+]] "intel_dtrans_func_index"="1" ptr @test02(ptr "intel_dtrans_func_index"="2")
; CHECK: ![[SOP_MD:[0-9]+]] = !{%struct.op.0 zeroinitializer, i32 1}
; CHECK: ![[TEST02_MD]] = distinct !{![[SOP_MD]], ![[SOP_MD]]}
; CHECK-NOT: !{%"type 0x

; end INTEL_FEATURE_SW_DTRANS
