; RUN: opt < %s -S -whole-program-assume -dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s
; RUN: opt < %s -S -whole-program-assume -passes=dtrans-aostosoa -dtrans-aostosoa-index32=false -dtrans-aostosoa-heur-override=struct.test01 2>&1 | FileCheck %s

; This tests the case of a byte-flattened GEP being passed to a PHI node.

; The control flow of these test functions is based on the DTrans analysis
; test case cyclical.ll

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

; This is the data structure the test is going to transform.
%struct.test01 = type { i32, i32, i32 }

@g_test01ptr = internal unnamed_addr global %struct.test01* zeroinitializer

define i32 @main(i32 %argc, i8** %argv) {
  %mem = call i8* @malloc(i64 120)
  store i8* %mem, i8** bitcast (%struct.test01** @g_test01ptr to i8**)
  call void @test01()
  call void @test02()
  call void @test03()
  ret i32 0
}

; In this case the PHI node operates on the address that was
; created via a byte-flattened GEP after it is cast back to its original
; pointer type.
define void @test01() {
; CHECK-LABEL: define internal void @test01

entry:
  %var = load %struct.test01*, %struct.test01** @g_test01ptr

  ; Get a byte-pointer to the structure.
  %p = bitcast %struct.test01* %var to i8*
  %p8_A = getelementptr i8, i8* %p, i64 0
; CHECK:    [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:    [[AR0_BEGIN:%[0-9]+]] = load i32*, i32** [[AR0_PTR]]
; CHECK:    [[FIELD0_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 %var
; CHECK:    %p8_A = bitcast i32* [[FIELD0_PTR]] to i8*

  %p8_B = getelementptr i8, i8* %p, i64 4
; CHECK:    [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:    [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:    [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %var
; CHECK:    %p8_B = bitcast i32* [[FIELD1_PTR]] to i8*

  %p8_C = getelementptr i8, i8* %p, i64 8
; CHECK:    [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:    [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:    [[FIELD2_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %var
; CHECK:    %p8_C = bitcast i32* [[FIELD2_PTR]] to i8*

  %pA = bitcast i8* %p8_A to i32*
; CHECK:  %pA = bitcast i8* %p8_A to i32*

  %pB = bitcast i8* %p8_B to i32*
; CHECK:  %pB = bitcast i8* %p8_B to i32*

  %pC = bitcast i8* %p8_C to i32*
; CHECK:  %pC = bitcast i8* %p8_C to i32*

  br i1 undef, label %block_A, label %block_BorC

block_BorC:
  br i1 undef, label %block_B, label %block_C

block_C:
  %c = phi i32* [%a, %merge_AorC], [%b, %merge_BorC], [%pC, %block_BorC]
  br i1 undef, label %merge, label %block_AorB

block_AorB:
  br i1 undef, label %block_A, label %block_B

block_A:
  %a = phi i32* [%d, %merge], [%c, %block_AorB], [%pA, %entry]
  br i1 undef, label %merge_AorC, label %exit_A

merge_AorC:
  br i1 undef, label %merge, label %block_C

block_B:
  %b = phi i32* [%d, %merge], [%c, %block_AorB], [%pB, %block_BorC]
  br i1 undef, label %merge_BorC, label %exit_B

merge_BorC:
  br i1 undef, label %merge, label %block_C

merge:
  %d = phi i32* [%a, %merge_AorC], [%b, %merge_BorC], [%c, %block_C]
  br i1 undef, label %block_A, label %block_B

exit_A:
  store i32 1, i32* %a
  br label %exit

exit_B:
  br label %exit

exit:
  ret void
}

; In this case, the PHI node operates on the byte-flattened GEP
; address, which will be cast for use in the store.
define void @test02() {
; CHECK-LABEL: define internal void @test02

entry:
  %var = load %struct.test01*, %struct.test01** @g_test01ptr

  ; Get a byte-pointer to the structure.
  %p = bitcast %struct.test01* %var to i8*
  %p8_A = getelementptr i8, i8* %p, i64 0
; CHECK:    [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:    [[AR0_BEGIN:%[0-9]+]] = load i32*, i32** [[AR0_PTR]]
; CHECK:    [[FIELD0_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 %var
; CHECK:    %p8_A = bitcast i32* [[FIELD0_PTR]] to i8*

  %p8_B = getelementptr i8, i8* %p, i64 4
; CHECK:    [[AR1_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 1
; CHECK:    [[AR1_BEGIN:%[0-9]+]] = load i32*, i32** [[AR1_PTR]]
; CHECK:    [[FIELD1_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR1_BEGIN]], i64 %var
; CHECK:    %p8_B = bitcast i32* [[FIELD1_PTR]] to i8*

  %p8_C = getelementptr i8, i8* %p, i64 8
; CHECK:    [[AR2_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 2
; CHECK:    [[AR2_BEGIN:%[0-9]+]] = load i32*, i32** [[AR2_PTR]]
; CHECK:    [[FIELD2_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR2_BEGIN]], i64 %var
; CHECK:    %p8_C = bitcast i32* [[FIELD2_PTR]] to i8*

  br i1 undef, label %block_A, label %block_BorC

block_BorC:
  br i1 undef, label %block_B, label %block_C

block_C:
  %c = phi i8* [%a, %merge_AorC], [%b, %merge_BorC], [%p8_C, %block_BorC]
  br i1 undef, label %merge, label %block_AorB

block_AorB:
  br i1 undef, label %block_A, label %block_B

block_A:
  %a = phi i8* [%d, %merge], [%c, %block_AorB], [%p8_A, %entry]
  br i1 undef, label %merge_AorC, label %exit_A

merge_AorC:
  br i1 undef, label %merge, label %block_C

block_B:
  %b = phi i8* [%d, %merge], [%c, %block_AorB], [%p8_B, %block_BorC]
  br i1 undef, label %merge_BorC, label %exit_B

merge_BorC:
  br i1 undef, label %merge, label %block_C

merge:
  %d = phi i8* [%a, %merge_AorC], [%b, %merge_BorC], [%c, %block_C]
  br i1 undef, label %block_A, label %block_B

exit_A:
  %tmp32 = bitcast i8* %a to i32*
; CHECK:   %tmp32 = bitcast i8* %a to i32*

  store i32 1, i32* %tmp32
; CHECK:  store i32 1, i32* %tmp32

  br label %exit

exit_B:
  br label %exit

exit:
  ret void
}

; In this case the PHI node operates on the base address of the
; structure before it is converted into a byte-flattend GEP.
define void @test03() {
; CHECK-LABEL: define internal void @test03

entry:
  %var = load %struct.test01*, %struct.test01** @g_test01ptr

  ; Get a pointer to the structure.
  %pA = bitcast %struct.test01* %var to i8*
  %pB = bitcast %struct.test01* %var to i8*
  %pC = bitcast %struct.test01* %var to i8*
  br i1 undef, label %block_A, label %block_BorC

block_BorC:
  br i1 undef, label %block_B, label %block_C

block_C:
  %c = phi i8* [%a, %merge_AorC], [%b, %merge_BorC], [%pC, %block_BorC]
  ; Unfortunately, we cannot use named variables that have not been defined yet
  ; so we need to use regular expressions in the check statement.
; CHECK:  [[C_ADDR:%[0-9]+]] = phi i64 [ {{%[0-9]+}}, %merge_AorC ], [ {{%[0-9]+}}, %merge_BorC ], [ %var, %block_BorC ]
  br i1 undef, label %merge, label %block_AorB

block_AorB:
  br i1 undef, label %block_A, label %block_B

block_A:
  %a = phi i8* [%d, %merge], [%c, %block_AorB], [%pA, %entry]
; CHECK:  [[A_ADDR:%[0-9]+]] = phi i64 [ {{%[0-9]+}}, %merge ], [ [[C_ADDR]], %block_AorB ], [ %var, %entry ]
  br i1 undef, label %merge_AorC, label %exit_A

merge_AorC:
  br i1 undef, label %merge, label %block_C

block_B:
  %b = phi i8* [%d, %merge], [%c, %block_AorB], [%pB, %block_BorC]
; CHECK: [[B_ADDR:%[0-9]+]] = phi i64 [ {{%[0-9]+}}, %merge ], [ [[C_ADDR]], %block_AorB ], [ %var, %block_BorC ]
  br i1 undef, label %merge_BorC, label %exit_B

merge_BorC:
  br i1 undef, label %merge, label %block_C

merge:
  %d = phi i8* [%a, %merge_AorC], [%b, %merge_BorC], [%c, %block_C]
; CHECK: [[D_ADDR:%[0-9]+]] = phi i64 [ [[A_ADDR]], %merge_AorC ], [ [[B_ADDR]], %merge_BorC ], [ %0, %block_C ]

  br i1 undef, label %block_A, label %block_B

exit_A:
  %field8_A = getelementptr i8, i8* %a, i64 0
; CHECK:    [[AR0_PTR:%[0-9]+]] = getelementptr [[__SOA_STRUCT_TEST01:%.*]], %__SOA_struct.test01* @__soa_struct.test01, i64 0, i32 0
; CHECK:    [[AR0_BEGIN:%[0-9]+]] = load i32*, i32** [[AR0_PTR]]
; CHECK:    [[FIELD0_PTR:%[0-9]+]] = getelementptr i32, i32* [[AR0_BEGIN]], i64 [[A_ADDR]]
; CHECK:    field8_A = bitcast i32* [[FIELD0_PTR]] to i8*

  %field_A = bitcast i8* %field8_A to i32*
  ; CHECK: %field_A = bitcast i8* %field8_A to i32*

  store i32 1, i32* %field_A
  br label %exit

exit_B:
  br label %exit

exit:
  ret void
}

declare i8* @malloc(i64)
