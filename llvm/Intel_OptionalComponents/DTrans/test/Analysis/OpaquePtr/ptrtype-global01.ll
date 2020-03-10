; REQUIRES: asserts

; RUN: opt -disable-output -whole-program-assume -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR
; RUN: opt -disable-output -whole-program-assume -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s --check-prefix=CHECK --check-prefix=CHECK-CUR

; Test type recovery for global variables with and without metadata

; Lines marked with CHECK-CUR are tests for the current form of IR.
; Lines marked with CHECK-FUT are placeholders for check lines that will
;   changed when the future opaque pointer form of IR is used.
; Lines marked with CHECK should remain the same when changing to use opaque
;   pointers.

%struct.type01 = type { i32, i64 }
@test_fcp01 = global i32* null, !dtrans_type !1
; CHECK-CUR: @test_fcp01 = global i32* null
; CHECK-FUT: @test_fcp01 = global p0 null
; CHECK:      Aliased types:
; CHECK:        i32**

@test_fcp02 = global double** null, !dtrans_type !3
; CHECK-CUR: @test_fcp02 = global double** null
; CHECK-FUT: @test_fcp02 = global p0 null
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

@test_arp01 = global [64 x i64]* null, !dtrans_type !5
; CHECK-CUR: @test_arp01 = global [64 x i64]* null
; CHECK-FUT: @test_arp01 = global p0 null
; CHECK:      Aliased types:
; CHECK:        [64 x i64]**

@test_arp02 = global [8 x [12 x [64 x i32]]]* null, !dtrans_type !8
; CHECK-CUR: @test_arp02 = global [8 x [12 x [64 x i32]]]* null
; CHECK-FUT: @test_arp02 = global p0 null
; CHECK:      Aliased types:
; CHECK:        [8 x [12 x [64 x i32]]]**

@test_arp03 = global [2 x [4 x [8 x i32*]]] zeroinitializer, !dtrans_type !12
; CHECK-CUR: @test_arp03 = global [2 x [4 x [8 x i32*]]] zeroinitializer
; CHECK-FUT: @test_arp03 = global [2 x [4 x [8 x p0]]] zeroinitializer
; CHECK:      Aliased types:
; CHECK:        [2 x [4 x [8 x i32*]]]*

@test_arp04 = global [10 x %struct.type01*] zeroinitializer, !dtrans_type !15
; CHECK-CUR: @test_arp04 = global [10 x %struct.type01*] zeroinitializer
; CHECK-FUT: @test_arp04 = global [10 x p0] zeroinitializer
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

@test_vecp01 = global <2 x i64*> zeroinitializer, !dtrans_type !18
; CHECK-CUR: @test_vecp01 = global <2 x i64*> zeroinitializer
; CHECK-FUT: @test_vecp01 = global <2 x p0> zeroinitializer
; CHECK:      Aliased types:
; CHECK:        <2 x i64*>*

@test_vec01 = global <4 x i32> zeroinitializer
; CHECK: @test_vec01 = global <4 x i32> zeroinitializer
; CHECK:      Aliased types:
; CHECK:        <4 x i32>*

@test_stp01 = global %struct.type01** null, !dtrans_type !20
; CHECK-CUR: @test_stp01 = global %struct.type01** null
; CHECK-FUT: @test_stp01 = global p0 null
; CHECK:      Aliased types:
; CHECK:        %struct.type01***

@test_st01 = global %struct.type01 zeroinitializer
; CHECK: @test_st01 = global %struct.type01 zeroinitializer
; CHECK:      Aliased types:
; CHECK:        %struct.type01*


!1 = !{i32 0, i32 1}  ; i32*
!2 = !{i32 0, i32 0}  ; i32
!3 = !{double 0.0e+00, i32 2}  ; double**
!4 = !{double 0.0e+00, i32 0}  ; double
!5 = !{!6, i32 1}  ; [64 x i64]*
!6 = !{!"A", i32 64, !7}  ; [64 x i64]
!7 = !{i64 0, i32 0}  ; i64
!8 = !{!9, i32 1}  ; [8 x [12 x [64 x i32]]]*
!9 = !{!"A", i32 8, !10}  ; [8 x [12 x [64 x i32]]]
!10 = !{!"A", i32 12, !11}  ; [12 x [64 x i32]]
!11 = !{!"A", i32 64, !2}  ; [64 x i32]
!12 = !{!"A", i32 2, !13}  ; [2 x [4 x [8 x i32*]]]
!13 = !{!"A", i32 4, !14}  ; [4 x [8 x i32*]]
!14 = !{!"A", i32 8, !1}  ; [8 x i32*]
!15 = !{!"A", i32 10, !16}  ; [ 10 x %struct.type01*]
!16 = !{!17, i32 1}  ; %struct.type01*
!17 = !{!"R", %struct.type01 zeroinitializer, i32 0}  ; %struct.type01
!18 = !{!"V", i32 2, !19}  ; <2 x i64*>
!19 = !{i64 0, i32 1}  ; i64*
!20 = !{!17, i32 2}  ; %struct.type01**
!21 = !{!"S", %struct.type01  zeroinitializer, i32 2, !2, !7} ; { i32, i64 }

!dtrans_types = !{!21}
