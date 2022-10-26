; RUN: opt -dtransop-allow-typed-pointers -S -passes=remove-dead-dtranstypemetadata %s | FileCheck %s
; RUN: opt -opaque-pointers -S -passes=remove-dead-dtranstypemetadata %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test that unreferenced DTrans type metadata is removed from the DTrans structure type list.
;
; %struct.test01 - Needs to be kept because a function signature uses it
; %struct.test02 - Can be removed
; %struct.test03 - Needs to be kept because %struct.test4 references it
; %struct.test04 - Needs to be kept because a function signature uses it
; %struct.test05 - Can be removed because the only reference is another type that can be removed
; %struct.test06 - Needs to be kept because an IR instruction references it
; %struct.test07 - Needs to be kept because a global variable references it

%struct.test01 = type { i64 }
%struct.test02 = type { i64, %struct.test05* }
%struct.test03 = type { i64, i64 }
%struct.test04 = type { %struct.test03* }
%struct.test05 = type { i64*, i64* }
%struct.test06 = type { i64*, i64* }
%struct.test07 = type { i64*, i64* }

@gVar = internal global %struct.test07* null, !intel_dtrans_type !5

define "intel_dtrans_func_index"="1" %struct.test01* @used() !intel.dtrans.func.type !7 {
  %alloc = alloca %struct.test06**, !intel_dtrans_type !8
  %mem = call i8* @malloc(i64 64)
  ret %struct.test01* null
}

define void @extractee(%struct.test04* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !10 {
  %tmp = call %struct.test01* @used()
  ret void
}

declare !intel.dtrans.func.type !12 "intel_dtrans_func_index"="1" i8* @malloc(i64)

!intel.dtrans.types = !{!13, !14, !15, !16, !17, !18, !19}

!1 = !{i64 0, i32 0}  ; i64
!2 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!3 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test07 zeroinitializer, i32 1}  ; %struct.test07*
!6 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!7 = distinct !{!6}
!8 = !{%struct.test06 zeroinitializer, i32 2}  ; %struct.test06**
!9 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!10 = distinct !{!9}
!11 = !{i8 0, i32 1}  ; i8*
!12 = distinct !{!11}
!13 = !{!"S", %struct.test01 zeroinitializer, i32 1, !1} ; { i64 }
!14 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !2} ; { i64, %struct.test05* }
!15 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i64, i64 }
!16 = !{!"S", %struct.test04 zeroinitializer, i32 1, !3} ; { %struct.test03* }
!17 = !{!"S", %struct.test05 zeroinitializer, i32 2, !4, !4} ; { i64*, i64* }
!18 = !{!"S", %struct.test06 zeroinitializer, i32 2, !4, !4} ; { i64*, i64* }
!19 = !{!"S", %struct.test07 zeroinitializer, i32 2, !4, !4} ; { i64*, i64* }

; CHECK: !intel.dtrans.types = !{![[MD_TEST01:[0-9]+]], ![[MD_TEST03:[0-9]+]], ![[MD_TEST04:[0-9]+]], ![[MD_TEST06:[0-9]+]], ![[MD_TEST07:[0-9]+]]}

; CHECK: ![[MD_TEST01]] = !{!"S", %struct.test01 zeroinitializer
; CHECK: ![[MD_TEST03]] = !{!"S", %struct.test03 zeroinitializer
; CHECK: ![[MD_TEST04]] = !{!"S", %struct.test04 zeroinitializer
; CHECK: ![[MD_TEST06]] = !{!"S", %struct.test06 zeroinitializer
; CHECK: ![[MD_TEST07]] = !{!"S", %struct.test07 zeroinitializer
