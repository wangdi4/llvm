; RUN: opt -S -passes=remove-all-dtranstypemetadata %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that all DTrans type metadata is removed from the IR.

%struct.test01 = type { i32 }
%struct.test02 = type { i64, ptr }
%struct.test03 = type { i32, i32 }
%struct.test04 = type { ptr }
%struct.test05 = type { ptr, ptr }

; Verify metadata is removed from global variables
@gVar = internal global ptr null, !intel_dtrans_type !5
@gConst = internal constant ptr null, !intel_dtrans_type !5

; CHECK: @gVar = internal global ptr null
; CHECK-NOT: !intel_dtrans_type
; CHECK: @gConst = internal constant ptr null
; CHECK-NOT: !intel_dtrans_type

; Verify attributes and metadata are removed from return type
define "intel_dtrans_func_index"="1" ptr @used() !intel.dtrans.func.type !7 {
  ; Verify metadata is removed from alloca
  %alloc = alloca ptr, !intel_dtrans_type !8
  %mem = call ptr @malloc(i64 64)
  ret ptr null
}
; CHECK: define ptr @used()
; CHECK-NOT: !intel.dtrans.func.type
 ;CHECK %alloc = alloca ptr
; CHECK-NOT: !intel_dtrans_type


; Verify attributes and metadata are removed from argument
define "intel_dtrans_func_index"="1" ptr @unused(ptr "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type !10 {
  ret  ptr null
}
; CHECK: define ptr @unused(ptr %in)
; CHECK-NOT: !intel.dtrans.func.type


define void @fptruser(ptr "intel_dtrans_func_index"="1" %fptr) !intel.dtrans.func.type !15 {
  %t4 = alloca %struct.test04
  ; Verify metadata is removed from indirect function call
  %res = call ptr %fptr(ptr %t4), !intel_dtrans_type !13
  ret void
}
; CHECK: define void @fptruser(ptr %fptr) {
; CHECK-NOT: !intel.dtrans.func.type
; CHECK:  %res = call ptr %fptr(ptr %t4)
; CHECK-NOT: !intel_dtrans_type


; Verify attributes and metadata are removed from function declaration
declare !intel.dtrans.func.type !17 "intel_dtrans_func_index"="1" ptr @malloc(i64)
; CHECK: declare ptr @malloc(i64)

; Verify named metadata is removed
; CHECK-NOT: !intel.dtrans.types
!intel.dtrans.types = !{!18, !19, !20, !21, !22}

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!4 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!5 = !{i64 0, i32 1}  ; i64*
!6 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!7 = distinct !{!6}
!8 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!9 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!10 = distinct !{!9, !9}
!11 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!12 = distinct !{!11}
!13 = !{!"F", i1 false, i32 1, !6, !11}  ; %struct.test01* (%struct.test04*)
!14 = !{!13, i32 1}  ; %struct.test01* (%struct.test04*)*
!15 = distinct !{!14}
!16 = !{i8 0, i32 1}  ; i8*
!17 = distinct !{!16}
!18 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { i32 }
!19 = !{!"S", %struct.test02 zeroinitializer, i32 2, !2, !3} ; { i64, %struct.test05* }
!20 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!21 = !{!"S", %struct.test04 zeroinitializer, i32 1, !4} ; { %struct.test03* }
!22 = !{!"S", %struct.test05 zeroinitializer, i32 2, !5, !5} ; { i64*, i64* }
