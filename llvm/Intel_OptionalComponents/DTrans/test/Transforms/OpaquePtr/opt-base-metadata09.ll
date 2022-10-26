; RUN: opt -dtransop-allow-typed-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01 < %s 2>&1 | FileCheck %s --check-prefix=CHECK-NONOPAQUE
; RUN: opt -opaque-pointers -S -passes=dtransop-optbasetest -dtransop-optbasetest-typelist=struct.test01 < %s 2>&1 | FileCheck %s --check-prefix=CHECK-OPAQUE

target triple = "x86_64-unknown-linux-gnu"

; Test the metadata attachments on instructions get updated
; in a cloned function when the base class changes types.

%struct.test01 = type { i32 }
%struct.test02 = type { i32 }
%struct.test01dep = type { %struct.test01* }

; CHECK-NONOPAQUE-DAG: %struct.test02 = type { i32 }
; CHECK-NONOPAQUE-DAG: %__DTT_struct.test01 = type { i32 }
; CHECK-NONOPAQUE-DAG: %__DDT_struct.test01dep = type { %__DTT_struct.test01* }

; CHECK-OPAQUE-DAG: %struct.test01dep = type { ptr }
; CHECK-OPAQUE-DAG: %struct.test02 = type { i32 }
; CHECK-OPAQUE-DAG: %__DTT_struct.test01 = type { i32 }

@g_test01 = private global %struct.test01 zeroinitializer
@g_test02 = private global %struct.test02 zeroinitializer
@gFn = private global void (%struct.test01**)* @foo, !intel_dtrans_type !0

; Test metadata handling on a function that gets cloned
define void @test01(%struct.test01 %in) {
  ; The information within the metadata attachments should reflect the
  ; resulting types for the case of a transformed type, a dependent type
  ; and a non-transformed type.
  %local01_ptr01 = alloca %struct.test01*, align 8, !intel_dtrans_type !7
  %local01_depptr01 = alloca %struct.test01dep*, align 8, !intel_dtrans_type !10
  %local01_ptr02 = alloca %struct.test02*, align 8, !intel_dtrans_type !11

  %fn = load void (%struct.test01**)*, void (%struct.test01**)** @gFn, align 8
  call void %fn(%struct.test01** %local01_ptr01), !intel_dtrans_type !1

  store %struct.test01* @g_test01, %struct.test01** %local01_ptr01, align 8
  store %struct.test02* @g_test02, %struct.test02** %local01_ptr02, align 8
  %s0 = load %struct.test01*, %struct.test01** %local01_ptr01, align 8
  %f0 = getelementptr %struct.test01, %struct.test01* %s0, i64 0, i32 0
  ret void
}

define void @foo(%struct.test01** "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !12 {
  ret void
}

!intel.dtrans.types = !{!4, !6, !8}

; Note: Normally DTrans metadata is not marked with 'distinct', but using it
; on the input IR to better track that each one is updated, but will not check
; for it in the output.
!0 = !{!1, i32 1}
!1 = !{!"F", i1 false, i32 1, !2, !3}
!2 = !{!"void", i32 0}
!3 = distinct !{%struct.test01 zeroinitializer, i32 2}

!4 = !{!"S", %struct.test01 zeroinitializer, i32 1, !5}
!5 = !{i32 0, i32 0}
!6 = !{!"S", %struct.test01dep zeroinitializer, i32 1, !7}
!7 = distinct !{%struct.test01 zeroinitializer, i32 1}

!8 = !{!"S", %struct.test02 zeroinitializer, i32 1, !5}
!9 = distinct !{%struct.test01 zeroinitializer, i32 1}

!10 = distinct !{%struct.test01dep zeroinitializer, i32 1}
!11 = distinct !{%struct.test02 zeroinitializer, i32 1}

!12 = distinct !{!13}
!13 = distinct !{%struct.test02 zeroinitializer, i32 2}

; CHECK-NONOPAQUE-LABEL: define internal void @test01.1
; CHECK-NONOPAQUE: %local01_ptr01 = alloca %__DTT_struct.test01*, align 8, !intel_dtrans_type ![[L1P1_MD:[0-9]+]]
; CHECK-NONOPAQUE: %local01_depptr01 = alloca %__DDT_struct.test01dep*, align 8, !intel_dtrans_type ![[L1DP1_MD:[0-9]+]]
; CHECK-NONOPAQUE: %local01_ptr02 = alloca %struct.test02*, align 8, !intel_dtrans_type ![[L1P2_MD:[0-9]+]]
; CHECK-NONOPAQUE: call void %fn(%__DTT_struct.test01** %local01_ptr01), !intel_dtrans_type ![[L1ICALL_MD:[0-9]+]]

; CHECK-NONOPAQUE-DAG: ![[L1P1_MD]] = {{.*}}!{%__DTT_struct.test01 zeroinitializer, i32 1}
; CHECK-NONOPAQUE-DAG: ![[L1DP1_MD]] = {{.*}}!{%__DDT_struct.test01dep zeroinitializer, i32 1}
; CHECK-NONOPAQUE-DAG: ![[L1P2_MD]] = {{.*}}!{%struct.test02 zeroinitializer, i32 1}
; CHECK-NONOPAQUE-DAG: ![[L1ICALL_MD]] = {{.*}}!{!"F", i1 false, i32 1, ![[F1VOID_MD:[0-9]+]], ![[F1ARG1_MD:[0-9]+]]}
; CHECK-NONOPAQUE-DAG: ![[F1VOID_MD]] = {{.*}}!{!"void", i32 0}
; CHECK-NONOPAQUE-DAG: ![[F1ARG1_MD]] = {{.*}}!{%__DTT_struct.test01 zeroinitializer, i32 2}

; CHECK-OPAQUE-LABEL: define internal void @test01.1
; CHECK-OPAQUE: %local01_ptr01 = alloca ptr, align 8, !intel_dtrans_type ![[L1P1_MD:[0-9]+]]
; CHECK-OPAQUE: %local01_depptr01 = alloca ptr, align 8, !intel_dtrans_type ![[L1DP1_MD:[0-9]+]]
; CHECK-OPAQUE: %local01_ptr02 = alloca ptr, align 8, !intel_dtrans_type ![[L1P2_MD:[0-9]+]]
; CHECK-OPAQUE: call void %fn(ptr %local01_ptr01), !intel_dtrans_type ![[L1ICALL_MD:[0-9]+]]

; CHECK-OPAQUE-DAG: ![[L1P1_MD]] = {{.*}}!{%__DTT_struct.test01 zeroinitializer, i32 1}
; CHECK-OPAQUE-DAG: ![[L1DP1_MD]] = {{.*}}!{%struct.test01dep zeroinitializer, i32 1}
; CHECK-OPAQUE-DAG: ![[L1P2_MD]] = {{.*}}!{%struct.test02 zeroinitializer, i32 1}
; CHECK-OPAQUE-DAG: ![[L1ICALL_MD]] = {{.*}}!{!"F", i1 false, i32 1, ![[F1VOID_MD:[0-9]+]], ![[F1ARG1_MD:[0-9]+]]}
; CHECK-OPAQUE-DAG: ![[F1VOID_MD]] = {{.*}}!{!"void", i32 0}
; CHECK-OPAQUE-DAG: ![[F1ARG1_MD]] = {{.*}}!{%__DTT_struct.test01 zeroinitializer, i32 2}
