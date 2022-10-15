; REQUIRES: asserts
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -S -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-aostosoaop -dtrans-aostosoaop-index32=true -dtrans-aostosoaop-typelist=struct.test01 -dtrans-aostosoaop-qual-override=true %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that pointer shrinking of the index to 32-bits gets
; inhibited when safety conditions for a dependent type are found that prevent
; modifying the size of the dependent type.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, %struct.test01*, i32* }
%struct.test01dep = type { %struct.test01*, i32* }

; CHECK-NONOPAQUE-DAG: %__SOA_struct.test01 = type { i32*, i64*, i32** }
; CHECK-NONOPAQUE-DAG: %__SOADT_struct.test01dep = type { i64, i32* }

; CHECK-OPAQUE-DAG: %__SOA_struct.test01 = type { ptr, ptr, ptr }
; CHECK-OPAQUE-DAG: %__SOADT_struct.test01dep = type { i64, ptr }

@glob = internal global %struct.test01dep* zeroinitializer, !intel_dtrans_type !4

define i32 @main() {
  ; Allocate the dependent type with a value that is not a multiple of
  ; the structure size. This will inhibit using a 32-bit peeling index
  ; because the transformation will not be able to modify the allocation
  ; size of the dependent structure.
  %mem = call i8* @calloc(i64 1, i64 36)
  %dep_mem = bitcast i8* %mem to %struct.test01dep*
  store %struct.test01dep* %dep_mem, %struct.test01dep** @glob
  ret i32 0
}

declare !intel.dtrans.func.type !6 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{%struct.test01dep zeroinitializer, i32 1}  ; %struct.test01dep*
!5 = !{i8 0, i32 1}  ; i8*
!6 = distinct !{!5}
!7 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !3} ; { i32, %struct.test01*, i32* }
!8 = !{!"S", %struct.test01dep zeroinitializer, i32 2, !2, !3} ; { %struct.test01*, i32* }

!intel.dtrans.types = !{!7, !8}

; CHECK: !intel.dtrans.types = !{![[SMD1:[0-9]+]], ![[SMD2:[0-9]+]]}

; CHECK: ![[SMD1]] = !{!"S", %__SOA_struct.test01 zeroinitializer, i32 3, ![[P32:[0-9]+]], ![[P64:[0-9]+]], ![[PP32:[0-9]+]]}
; CHECK: ![[P32]] = !{i32 0, i32 1}
; CHECK: ![[P64]] = !{i64 0, i32 1}
; CHECK: ![[PP32]] = !{i32 0, i32 2}

; CHECK: ![[SMD2]] = !{!"S", %__SOADT_struct.test01dep zeroinitializer, i32 2, ![[I64:[0-9]+]], ![[P32]]}
; CHECK: ![[I64]] = !{i64 0, i32 0}
