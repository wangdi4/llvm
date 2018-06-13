; RUN: opt < %s -S -dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01,struct.test02 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-aostosoa -dtrans-aostosoa-heur-override=struct.test01,struct.test02 2>&1 | FileCheck %s

; This test verifies the basic functionality for creating a peeled type
; for the AOS to SOA transformation to verify that the transformation
; works when there are multiple types to be converted.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
%struct.test01 = type { i32, i64, %struct.test01* }
%struct.test02 = type { i32*, %struct.test02* }

; CHECK-DAG: %__SOA_struct.test01 = type { i32*, i64*, i64* }
; CHECK-DAG: %__SOA_struct.test02 = type { i32**, i64* }
; CHECK-DAG: @__soa_struct.test01 = internal global %__SOA_struct.test01 zeroinitializer
; CHECK-DAG: @__soa_struct.test02 = internal global %__SOA_struct.test02 zeroinitializer

define void @test01() {
; We need some use of the type so that opt will generate it in the output.
  %local1 = alloca %struct.test01*
  %local2 = alloca %struct.test02*
  ret void
}
; Verify that a scalar integer is now used for the local.
; CHECK: define void @test01()
; CHECK:   %local1 = alloca i64
; CHECK:   %local2 = alloca i64
