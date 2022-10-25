; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -passes='require<dtrans-safetyanalyzer>' -dtrans-print-callinfo -disable-output %s 2>&1 | FileCheck %s

; Test creation of CallInfo objects for memory allocation and free calls
; analyzed by the DTransSafetyAnalyer.

; Allocation of a pointer-to-pointer to a structure type.
%struct.test00 = type { i32, i32, i32 }
@var_test00 = internal global %struct.test00** zeroinitializer, !intel_dtrans_type !2
define internal void @test00() {
  %mem_i8 = call i8* @malloc(i64 64)
  %ptrtoptr = bitcast i8* %mem_i8 to %struct.test00**
  store %struct.test00** %ptrtoptr, %struct.test00*** @var_test00
  ret void
}
; CHECK-LABEL: Function: test00
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test00*{{ *}}


; Allocation of a structure type using malloc.
%struct.test01member = type { i64, i64 }
%struct.test01 = type { %struct.test01member* }
@var_test01 = internal global %struct.test01 zeroinitializer
define internal void @test01() {
  %mem_i8 = call i8* @malloc(i64 16)
  %mystruct = bitcast i8* %mem_i8 to %struct.test01member*
  store %struct.test01member* %mystruct, %struct.test01member** getelementptr (%struct.test01, %struct.test01* @var_test01, i64 0, i32 0)
  ret void
}

; Free of a structure type.
define internal void @test01f() {
  %mem = load %struct.test01member*, %struct.test01member** getelementptr (%struct.test01, %struct.test01* @var_test01, i64 0, i32 0)
  %mem.p8 = bitcast %struct.test01member* %mem to i8*
  call void @free(i8* %mem.p8)
  ret void
}
; CHECK-LABEL: Function: test01
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test01member = type { i64, i64 }

; CHECK-LABEL: Function: test01f
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test01member = type { i64, i64 }


; Allocation of a structure type using malloc that is resolved based on
; function return type metadata.
%struct.test02 = type { i32, i32 }
define "intel_dtrans_func_index"="1" %struct.test02* @test02() !intel.dtrans.func.type !6 {
  %mem = call i8* @malloc(i64 8)
  %mem.as.struct = bitcast i8* %mem to %struct.test02*
  ret %struct.test02* %mem.as.struct
}

; Free of a structure type that is resolved based on function  metadata.
define void @test02f(%struct.test02* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !7 {
  %mem.p8 = bitcast %struct.test02* %pStruct to i8*
  call void @free(i8* %mem.p8)
  ret void
}
; CHECK-LABEL: Function: test02
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test02 = type { i32, i32 }

; CHECK-LABEL: Function: test02f
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test02 = type { i32, i32 }


; Allocation of a pointer-to-pointer to a structure type using calloc.
%struct.test03 = type { %struct.test03* }
define internal void @test03(%struct.test03* "intel_dtrans_func_index"="1" %in, i64 %index) !intel.dtrans.func.type !9 {
  %mem_i8 = call i8* @calloc(i64 16, i64 8)
  %array = bitcast i8* %mem_i8 to %struct.test03**
  %array_elem = getelementptr %struct.test03*, %struct.test03** %array, i64 %index
  store %struct.test03* %in, %struct.test03** %array_elem

  call void @free(i8* %mem_i8)
  store %struct.test03* null, %struct.test03** %array_elem
  ret void
}
; CHECK-LABEL: Function: test03
; CHECK: AllocCallInfo:
; CHECK:   Kind: Calloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test03*{{ *}}

; CHECK-LABEL: Function: test03
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test03*{{ *}}


; Allocation using realloc.
%struct.test04 = type { i64, i64 }
define internal "intel_dtrans_func_index"="1" %struct.test04* @test04(%struct.test04* "intel_dtrans_func_index"="2" %in) !intel.dtrans.func.type !11 {
  %in_i8 = bitcast %struct.test04* %in to i8*
  %out_i8 = call i8* @realloc(i8* %in_i8, i64 256)
  %out = bitcast i8* %out_i8 to %struct.test04*
  ret %struct.test04* %out
}

; Free of a structure type.
define void @test04f(%struct.test04* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !12 {
  %mem.p8 = bitcast %struct.test04* %in to i8*
  call void @free(i8* %mem.p8)
  ret void
}
; CHECK-LABEL: Function: test04
; CHECK: AllocCallInfo:
; CHECK:   Kind: Realloc
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test04 = type { i64, i64 }

; CHECK-LABEL: Function: test04f
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test04 = type { i64, i64 }


; Allocation using new for a structure type
%struct.test05 = type { i64, i64 }
define "intel_dtrans_func_index"="1" %struct.test05* @test05() !intel.dtrans.func.type !14 {
  %mem = call i8* @_Znwm(i64 16)
  %mem.as.struct = bitcast i8* %mem to %struct.test05*
  ret %struct.test05* %mem.as.struct
}

; Free of a structure type using delete.
define void @test05f(%struct.test05* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !15 {
  %mem.p8 = bitcast %struct.test05* %pStruct to i8*
  call void @_ZdlPv(i8* %mem.p8)
  ret void
}
; CHECK-LABEL: Function: test05
; CHECK: AllocCallInfo:
; CHECK:   Kind: new/new[]
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test05 = type { i64, i64 }

; CHECK-LABEL: Function: test05f
; CHECK: FreeCallInfo:
; CHECK:   Kind: delete/delete[]
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test05 = type { i64, i64 }


; Allocation using new[] for a structure type.
%struct.test06 = type { i64, i64 }
define "intel_dtrans_func_index"="1" %struct.test06* @test06(i64 %n) !intel.dtrans.func.type !17 {
  %bytes = mul i64 %n, 16
  %mem = call i8* @_Znam(i64 %bytes)
  %mem.as.struct = bitcast i8* %mem to %struct.test06*
  ret %struct.test06* %mem.as.struct
}

; Free of a structure type using delete[]
define void @test06f(%struct.test06* "intel_dtrans_func_index"="1" %pStruct) !intel.dtrans.func.type !18 {
  %mem.p8 = bitcast %struct.test06* %pStruct to i8*
  call void @_ZdaPv(i8* %mem.p8)
  ret void
}
; CHECK-LABEL: Function: test06
; CHECK: AllocCallInfo:
; CHECK:   Kind: new/new[]
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test06 = type { i64, i64 }

; CHECK-LABEL: Function: test06f
; CHECK: FreeCallInfo:
; CHECK:   Kind: delete/delete[]
; CHECK:   Aliased types:
; CHECK:     Type: %struct.test06 = type { i64, i64 }


; Test an allocation that does not get used as an aggregate type.
%struct.test07 = type { i32* }
@var07 = internal global %struct.test07 zeroinitializer
define void @test07() {
  %mem = call i8* @malloc(i64 20)
  %mem.as.p32 = bitcast i8* %mem to i32*
  %field.addr = getelementptr %struct.test07, %struct.test07* @var07, i64 0, i32 0
  store i32* %mem.as.p32, i32** %field.addr
  ret void
}

; Free that is not resolved to be an aggregate type.
define void @test07f() {
  %field.addr = getelementptr %struct.test07, %struct.test07* @var07, i64 0, i32 0
  %mem = load i32*, i32** %field.addr
  %mem.as.p8 = bitcast i32* %mem to i8*
  call void @free(i8* %mem.as.p8)
  ret void
}
; CHECK-LABEL: Function: test07
; CHECK: AllocCallInfo:
; CHECK:   Kind: Malloc
; CHECK:   Aliased types:
; CHECK:     Type: Non-aggregate

; CHECK-LABEL: Function: test07f
; CHECK: FreeCallInfo:
; CHECK:   Kind: Free
; CHECK:   Aliased types:
; CHECK:     Type: Non-aggregate


declare !intel.dtrans.func.type !21 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0
declare !intel.dtrans.func.type !22 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #1
declare !intel.dtrans.func.type !23 "intel_dtrans_func_index"="1" i8* @realloc(i8* "intel_dtrans_func_index"="2", i64) #2
declare !intel.dtrans.func.type !24 void @free(i8* "intel_dtrans_func_index"="1") #3

declare !intel.dtrans.func.type !25 "intel_dtrans_func_index"="1" i8* @_Znwm(i64) ; new(unsigned long)
declare !intel.dtrans.func.type !26 void @_ZdlPv(i8* "intel_dtrans_func_index"="1") ; delete(void*)
declare !intel.dtrans.func.type !27 "intel_dtrans_func_index"="1" i8* @_Znam(i64) ;  new[](unsigned long)
declare !intel.dtrans.func.type !28 void @_ZdaPv(i8* "intel_dtrans_func_index"="1") ; delete(void*)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }
attributes #2 = { allockind("realloc") allocsize(1) "alloc-family"="malloc" }
attributes #3 = { allockind("free") "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test00 zeroinitializer, i32 2}  ; %struct.test00**
!3 = !{i64 0, i32 0}  ; i64
!4 = !{%struct.test01member zeroinitializer, i32 1}  ; %struct.test01member*
!5 = !{%struct.test02 zeroinitializer, i32 1}  ; %struct.test02*
!6 = distinct !{!5}
!7 = distinct !{!5}
!8 = !{%struct.test03 zeroinitializer, i32 1}  ; %struct.test03*
!9 = distinct !{!8}
!10 = !{%struct.test04 zeroinitializer, i32 1}  ; %struct.test04*
!11 = distinct !{!10, !10}
!12 = distinct !{!10}
!13 = !{%struct.test05 zeroinitializer, i32 1}  ; %struct.test05*
!14 = distinct !{!13}
!15 = distinct !{!13}
!16 = !{%struct.test06 zeroinitializer, i32 1}  ; %struct.test06*
!17 = distinct !{!16}
!18 = distinct !{!16}
!19 = !{i32 0, i32 1}  ; i32*
!20 = !{i8 0, i32 1}  ; i8*
!21 = distinct !{!20}
!22 = distinct !{!20}
!23 = distinct !{!20, !20}
!24 = distinct !{!20}
!25 = distinct !{!20}
!26 = distinct !{!20}
!27 = distinct !{!20}
!28 = distinct !{!20}
!29 = !{!"S", %struct.test00 zeroinitializer, i32 3, !1, !1, !1} ; { i32, i32, i32 }
!30 = !{!"S", %struct.test01member zeroinitializer, i32 2, !3, !3} ; { i64, i64 }
!31 = !{!"S", %struct.test01 zeroinitializer, i32 1, !4} ; { %struct.test01member* }
!32 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!33 = !{!"S", %struct.test03 zeroinitializer, i32 1, !8} ; { %struct.test03* }
!34 = !{!"S", %struct.test04 zeroinitializer, i32 2, !3, !3} ; { i64, i64 }
!35 = !{!"S", %struct.test05 zeroinitializer, i32 2, !3, !3} ; { i64, i64 }
!36 = !{!"S", %struct.test06 zeroinitializer, i32 2, !3, !3} ; { i64, i64 }
!37 = !{!"S", %struct.test07 zeroinitializer, i32 1, !19} ; { i32* }

!intel.dtrans.types = !{!29, !30, !31, !32, !33, !34, !35, !36, !37}
