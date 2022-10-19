; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-OPAQUE

; Test type recovery for global variables with and without metadata

; Lines marked with CHECK-NONOPAQUE are tests for the current form of IR.
; Lines marked with CHECK-OPAQUE are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.type01 = type { i32, i64 }
@test_fcp01 = global i32* null, !intel_dtrans_type !3
; CHECK-NONOPAQUE: @test_fcp01 = global i32* null
; CHECK-OPAQUE: @test_fcp01 = global ptr null
; CHECK:      Aliased types:
; CHECK:        i32**

@test_fcp02 = global double** null, !intel_dtrans_type !4
; CHECK-NONOPAQUE: @test_fcp02 = global double** null
; CHECK-OPAQUE: @test_fcp02 = global ptr null
; CHECK:      Aliased types:
; CHECK:        double***

@test_fc01 = global i32 0
; CHECK: @test_fc01 = global i32 0
; CHECK:      Aliased types:
; CHECK:        i32*

@test_fc02 = global float 0x7FF8000000000000
; CHECK: @test_fc02 = global float 0x7FF8000000000000
; CHECK:      Aliased types:
; CHECK:        float*

; Verify 'constant' variables, as well.
@test_fc03 = constant i32 0
; CHECK: @test_fc03 = constant i32 0
; CHECK:      Aliased types:
; CHECK:        i32*

@test_arp01 = global [64 x i64]* null, !intel_dtrans_type !5
; CHECK-NONOPAQUE: @test_arp01 = global [64 x i64]* null
; CHECK-OPAQUE: @test_arp01 = global ptr null
; CHECK:      Aliased types:
; CHECK:        [64 x i64]**

@test_arp02 = global [8 x [12 x [64 x i32]]]* null, !intel_dtrans_type !7
; CHECK-NONOPAQUE: @test_arp02 = global [8 x [12 x [64 x i32]]]* null
; CHECK-OPAQUE: @test_arp02 = global ptr null
; CHECK:      Aliased types:
; CHECK:        [8 x [12 x [64 x i32]]]**

@test_arp03 = global [2 x [4 x [8 x i32*]]] zeroinitializer, !intel_dtrans_type !11
; CHECK-NONOPAQUE: @test_arp03 = global [2 x [4 x [8 x i32*]]] zeroinitializer
; CHECK-OPAQUE: @test_arp03 = global [2 x [4 x [8 x ptr]]] zeroinitializer
; CHECK:      Aliased types:
; CHECK:        [2 x [4 x [8 x i32*]]]*

@test_arp04 = global [10 x %struct.type01*] zeroinitializer, !intel_dtrans_type !14
; CHECK-NONOPAQUE: @test_arp04 = global [10 x %struct.type01*] zeroinitializer
; CHECK-OPAQUE: @test_arp04 = global [10 x ptr] zeroinitializer
; CHECK:      Aliased types:
; CHECK:        [10 x %struct.type01*]*

@test_ar01 = global [3 x [3 x [3 x i64]]] zeroinitializer
; CHECK: @test_ar01 = global [3 x [3 x [3 x i64]]] zeroinitializer
; CHECK:      Aliased types:
; CHECK:        [3 x [3 x [3 x i64]]]*

@test_ar02 = global [2 x {i32, i8}] zeroinitializer
; CHECK: @test_ar02 = global [2 x { i32, i8 }] zeroinitializer
; CHECK:      Aliased types:
; CHECK:        [2 x { i32, i8 }]*

@test_ar03 = global [16 x %struct.type01] zeroinitializer
; CHECK: @test_ar03 = global [16 x %struct.type01] zeroinitializer
; CHECK:      Aliased types:
; CHECK:        [16 x %struct.type01]*

@test_vecp01 = global <2 x i64*> zeroinitializer, !intel_dtrans_type !16
; CHECK-NONOPAQUE: @test_vecp01 = global <2 x i64*> zeroinitializer
; CHECK-OPAQUE: @test_vecp01 = global <2 x ptr> zeroinitializer
; CHECK:      Aliased types:
; CHECK:        <2 x i64*>*

@test_vec01 = global <4 x i32> zeroinitializer
; CHECK: @test_vec01 = global <4 x i32> zeroinitializer
; CHECK:      Aliased types:
; CHECK:        <4 x i32>*

@test_stp01 = global %struct.type01** null, !intel_dtrans_type !18
; CHECK-NONOPAQUE: @test_stp01 = global %struct.type01** null
; CHECK-OPAQUE: @test_stp01 = global ptr null
; CHECK:      Aliased types:
; CHECK:        %struct.type01***

@test_st01 = global %struct.type01 zeroinitializer
; CHECK: @test_st01 = global %struct.type01 zeroinitializer
; CHECK:      Aliased types:
; CHECK:        %struct.type01*


!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{double 0.0e+00, i32 2}  ; double**
!5 = !{!6, i32 1}  ; [64 x i64]*
!6 = !{!"A", i32 64, !2}  ; [64 x i64]
!7 = !{!8, i32 1}  ; [8 x [12 x [64 x i32]]]*
!8 = !{!"A", i32 8, !9}  ; [8 x [12 x [64 x i32]]]
!9 = !{!"A", i32 12, !10}  ; [12 x [64 x i32]]
!10 = !{!"A", i32 64, !1}  ; [64 x i32]
!11 = !{!"A", i32 2, !12}  ; [2 x [4 x [8 x i32*]]]
!12 = !{!"A", i32 4, !13}  ; [4 x [8 x i32*]]
!13 = !{!"A", i32 8, !3}  ; [8 x i32*]
!14 = !{!"A", i32 10, !15}  ; [10 x %struct.type01*]
!15 = !{%struct.type01 zeroinitializer, i32 1}  ; %struct.type01*
!16 = !{!"V", i32 2, !17}  ; <2 x i64*>
!17 = !{i64 0, i32 1}  ; i64*
!18 = !{%struct.type01 zeroinitializer, i32 2}  ; %struct.type01**
!19 = !{!"S", %struct.type01 zeroinitializer, i32 2, !1, !2} ; { i32, i64 }

!intel.dtrans.types = !{!19}
