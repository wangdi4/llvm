; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test pointer type recovery for the partial pointer swap idiom when the loop
; control predicate uses an unsigned comparison operation.

; This test verifies that we are able to identify a couple of idioms for
; swapping pointer values in a dynamic array of pointers

; Swap pointers in 32-bit chunks.
%struct.test01 = type { i32, i32 }
define void @test01(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !3 {
entry:
  ; This is here to establish what %p actually points to.
  %tmp = call i1 @test01identify(ptr %p)
  %other = getelementptr i8, ptr %p, i64 8
  br label %pre_swap

pre_swap:
  br label %swap

swap:
  %Count = phi i64 [ 2, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, ptr %HalfPtr1
  %HalfVal2 = load i32, ptr %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, ptr %HalfPtr1, i64 1
  store i32 %HalfVal2, ptr %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, ptr %HalfPtr2, i64 1
  store i32 %HalfVal1, ptr %HalfPtr2
  %NextCount = add nsw i64 %Count, -1
  %Cmp = icmp ugt i64 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}
define i1 @test01identify(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !5 {
  %p = load ptr, ptr %in
  %a1 = getelementptr %struct.test01, ptr %p, i64 0, i32 0
  %a2 = getelementptr %struct.test01, ptr %p, i64 0, i32 0
  %v1 = load i32, ptr %a1
  %v2 = load i32, ptr %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}

; Even though the load uses the pointer as a i32* type, the partial pointer
; idiom recognition should avoid adding that type to the list of aliased types
; for the pointer operand of the load.

; CHECK-LABEL: Input Parameters: test01
; CHECK: Arg 0: ptr %p
; CHECK:      Aliased types:
; CHECK:        %struct.test01**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
; CHECK:      LocalPointerInfo:
; CHECK-SAME: PARTIAL PTR
; CHECK:      Aliased types:
; CHECK:        %struct.test01**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
; CHECK:      LocalPointerInfo:
; CHECK-SAME: PARTIAL PTR
; CHECK:      Aliased types:
; CHECK:        %struct.test01**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.


; Swap pointers in 8-bit chunks.
%struct.test02 = type { i32, i32 }
define void @test02(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !6 {
entry:
  ; This is here to establish what %p actually points to.
  %tmp = call i1 @test02identify(ptr %p)
  %other = getelementptr i8, ptr %p, i64 8
  br label %swap

swap:
  %Count = phi i64 [ 8, %entry ], [ %NextCount, %swap ]
  %PartPtr1 = phi ptr [ %p, %entry ], [ %NextPart1, %swap ]
  %PartPtr2 = phi ptr [ %other, %entry ], [ %NextPart2, %swap ]
  %PartVal1 = load i8, ptr %PartPtr1
  %PartVal2 = load i8, ptr %PartPtr2
  %NextPart1 = getelementptr inbounds i8, ptr %PartPtr1, i64 1
  store i8 %PartVal2, ptr %PartPtr1
  %NextPart2 = getelementptr inbounds i8, ptr %PartPtr2, i64 1
  store i8 %PartVal1, ptr %PartPtr2
  %NextCount = add nsw i64 %Count, -1
  %Cmp = icmp ugt i64 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}
define i1 @test02identify(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !8 {
  %p = load ptr, ptr %in
  %a1 = getelementptr %struct.test02, ptr %p, i64 0, i32 0
  %a2 = getelementptr %struct.test02, ptr %p, i64 0, i32 0
  %v1 = load i32, ptr %a1
  %v2 = load i32, ptr %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; The partial pointer access is done with i8* types here, however the initial
; pointer has been declared as i8* so that type will appear in the result types.

; CHECK-LABEL: Input Parameters: test02
; CHECK: Arg 0: ptr %p
; CHECK:      Aliased types:
; CHECK:        %struct.test02**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK: %PartPtr1 = phi ptr [ %p, %entry ], [ %NextPart1, %swap ]
; CHECK:      LocalPointerInfo:
; CHECK-SAME: PARTIAL PTR
; CHECK:      Aliased types:
; CHECK:        %struct.test02**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK: %PartPtr2 = phi ptr [ %other, %entry ], [ %NextPart2, %swap ]
; CHECK:      LocalPointerInfo:
; CHECK-SAME: PARTIAL PTR
; CHECK:      Aliased types:
; CHECK:        %struct.test02**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.


; Swap a variable number of pointers in 32-bit chunks.
%struct.test03 = type { i32, i32 }
define void @test03(ptr "intel_dtrans_func_index"="1" %p, i64 %n) !intel.dtrans.func.type !9 {
entry:
  ; This is here to establish what %p actually points to.
  %tmp = call i1 @test03identify(ptr %p)
  %offset = mul i64 %n, 8
  %init_count = mul i64 %n, 2
  %other = getelementptr i8, ptr %p, i64 %offset
  br label %pre_swap

pre_swap:
  br label %swap

swap:
  %Count = phi i64 [ %init_count, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, ptr %HalfPtr1
  %HalfVal2 = load i32, ptr %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, ptr %HalfPtr1, i64 1
  store i32 %HalfVal2, ptr %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, ptr %HalfPtr2, i64 1
  store i32 %HalfVal1, ptr %HalfPtr2
  %NextCount = add nsw i64 %Count, -1
  %Cmp = icmp ugt i64 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}
define i1 @test03identify(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !11 {
  %p = load ptr, ptr %in
  %a1 = getelementptr %struct.test03, ptr %p, i64 0, i32 0
  %a2 = getelementptr %struct.test03, ptr %p, i64 0, i32 0
  %v1 = load i32, ptr %a1
  %v2 = load i32, ptr %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; Even though the load uses the pointer as a i32* type, the partial pointer
; idiom recognition should avoid adding that type to the list of aliased types
; for the pointer operand of the load.

; CHECK-LABEL: Input Parameters: test03
; CHECK: Arg 0: ptr %p
; CHECK:      Aliased types:
; CHECK:        %struct.test03**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
; CHECK:      Aliased types:
; CHECK:        %struct.test03**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
; CHECK:      Aliased types:
; CHECK:        %struct.test03**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.


; Swap incomplete chunks of pointers. This case does not match the partial
; pointer access idiom.
%struct.test04 = type { i32, i32 }
define void @test04(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !12 {
entry:
  ; This is here to establish what %p actually points to.
  %tmp = call i1 @test04identify(ptr %p)
  %other = getelementptr i8, ptr %p, i64 8
  br label %pre_swap

pre_swap:
  %cur_p = phi ptr [ %p, %entry ], [ %prev_p, %swap ]
  %prev_p = getelementptr inbounds i8, ptr %cur_p, i64 -8
  br i1 undef, label %swap, label %exit

swap:
  %val1 = load i32, ptr %cur_p
  %val2 = load i32, ptr %prev_p
  store i32 %val2, ptr %cur_p
  store i32 %val1, ptr %prev_p
  br i1 undef, label %pre_swap, label %exit

exit:
  ret void
}
define i1 @test04identify(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !14 {
  %p = load ptr, ptr %in
  %a1 = getelementptr %struct.test04, ptr %p, i64 0, i32 0
  %a2 = getelementptr %struct.test04, ptr %p, i64 0, i32 0
  %v1 = load i32, ptr %a1
  %v2 = load i32, ptr %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: Input Parameters: test04
; CHECK: Arg 0: ptr %p
; CHECK:      Aliased types:
; CHECK:        %struct.test04**{{ *}}
; CHECK:        i32*{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; Swap 32-bit chunks without looping. This case does not match the partial
; pointer access idiom.
%struct.test05 = type { i32, i32 }
define void @test05(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !15 {
entry:
  ; This is here to establish what %p actually points to.
  %tmp = call i1 @test05identify(ptr %p)
  %other = getelementptr i8, ptr %p, i64 8
  br label %pre_swap

pre_swap:
  br label %swap

swap:
  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %tmp1, %other_bb ]
  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %tmp2, %other_bb ]
  %HalfVal1 = load i32, ptr %HalfPtr1
  %HalfVal2 = load i32, ptr %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, ptr %HalfPtr1, i64 1
  store i32 %HalfVal2, ptr %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, ptr %HalfPtr2, i64 1
  store i32 %HalfVal1, ptr %HalfPtr2
  br label %other_bb

other_bb:
  %tmp1 = getelementptr i8, ptr %p, i64 16
  %tmp2 = getelementptr i8, ptr %p, i64 24
  br i1 undef, label %swap, label %exit

exit:
  ret void
}
define i1 @test05identify(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !17 {
  %p = load ptr, ptr %in
  %a1 = getelementptr %struct.test05, ptr %p, i64 0, i32 0
  %a2 = getelementptr %struct.test05, ptr %p, i64 0, i32 0
  %v1 = load i32, ptr %a1
  %v2 = load i32, ptr %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; Because the idiom is not matched, the input parameter type will pick up
; i32* as an aliased type.

; CHECK-LABEL: Input Parameters: test05
; CHECK: Arg 0: ptr %p
; CHECK:      Aliased types:
; CHECK:        %struct.test05**{{ *}}
; CHECK:        i32*{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %tmp1, %other_bb ]
; CHECK:      Aliased types:
; CHECK:        %struct.test05**{{ *}}
; CHECK:        i32*{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %tmp2, %other_bb ]
; CHECK:      Aliased types:
; CHECK:        %struct.test05**{{ *}}
; CHECK:        i32*{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.


; Swap pointers in 32-bit chunks with a bad pointer increment. This case does
; not match the partial pointer access idiom.
%struct.test06 = type { i32, i32 }
define void @test06(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !18 {
entry:
  ; This is here to establish what %p actually points to.
  %tmp = call i1 @test06identify(ptr %p)
  %other = getelementptr i8, ptr %p, i64 8
  br label %pre_swap

pre_swap:
  br label %swap

swap:
  %Count = phi i64 [ 2, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, ptr %HalfPtr1
  %HalfVal2 = load i32, ptr %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, ptr %HalfPtr1, i64 2
  store i32 %HalfVal2, ptr %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, ptr %HalfPtr2, i64 2
  store i32 %HalfVal1, ptr %HalfPtr2
  %NextCount = add nsw i64 %Count, -1
  %Cmp = icmp ugt i64 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}
define i1 @test06identify(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !20 {
  %p = load ptr, ptr %in
  %a1 = getelementptr %struct.test06, ptr %p, i64 0, i32 0
  %a2 = getelementptr %struct.test06, ptr %p, i64 0, i32 0
  %v1 = load i32, ptr %a1
  %v2 = load i32, ptr %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; Because the idiom is not matched, the input parameter type will pick up
; i32* as an aliased type.

; CHECK-LABEL: Input Parameters: test06
; CHECK: Arg 0: ptr %p
; CHECK:      Aliased types:
; CHECK:        %struct.test06**{{ *}}
; CHECK:        i32*{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
; CHECK:      Aliased types:
; CHECK:        %struct.test06**{{ *}}
; CHECK:        i32*{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
; CHECK:      Aliased types:
; CHECK:        %struct.test06**{{ *}}
; CHECK:        i32*{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.


; Swap a variable number of pointers in 32-bit chunks. This is a minor variation
; of test3 WRT the loop control test to match the pattern seen following some
; upstream transformation changes.
%struct.test07 = type { i32, i32 }
define void @test07(ptr "intel_dtrans_func_index"="1" %p, i64 %n) !intel.dtrans.func.type !21 {
entry:
  ; This is here to establish what %p actually points to.
  %tmp = call i1 @test07identify(ptr %p)
  %offset = mul i64 %n, 8
  %init_count = mul i64 %n, 2
  %other = getelementptr i8, ptr %p, i64 %offset
  br label %pre_swap

pre_swap:
  br label %swap

swap:
  %Count = phi i64 [ %init_count, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, ptr %HalfPtr1
  %HalfVal2 = load i32, ptr %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, ptr %HalfPtr1, i64 1
  store i32 %HalfVal2, ptr %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, ptr %HalfPtr2, i64 1
  store i32 %HalfVal1, ptr %HalfPtr2
  %NextCount = add nsw i64 %Count, -1
  %Cmp = icmp ugt i64 %NextCount, 0
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}
define i1 @test07identify(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !23 {
  %p = load ptr, ptr %in
  %a1 = getelementptr %struct.test07, ptr %p, i64 0, i32 0
  %a2 = getelementptr %struct.test07, ptr %p, i64 0, i32 0
  %v1 = load i32, ptr %a1
  %v2 = load i32, ptr %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: Input Parameters: test07
; CHECK: Arg 0: ptr %p
; CHECK:      Aliased types:
; CHECK:        %struct.test07**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
; CHECK:      Aliased types:
; CHECK:        %struct.test07**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
; CHECK:      Aliased types:
; CHECK:        %struct.test07**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.


; Swap pointers in 32-bit chunks. This is same as test1 except the type of
; %Count is i32.
%struct.test08 = type { i32, i32 }
define void @test08(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !24 {
entry:
  ; This is here to establish what %p actually points to.
  %tmp = call i1 @test08identify(ptr %p)
  %other = getelementptr i8, ptr %p, i64 8
  br label %pre_swap

pre_swap:
  br label %swap

swap:
  %Count = phi i32 [ 2, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, ptr %HalfPtr1
  %HalfVal2 = load i32, ptr %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, ptr %HalfPtr1, i64 1
  store i32 %HalfVal2, ptr %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, ptr %HalfPtr2, i64 1
  store i32 %HalfVal1, ptr %HalfPtr2
  %NextCount = add nsw i32 %Count, -1
  %Cmp = icmp ugt i32 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}
define i1 @test08identify(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !26 {
  %p = load ptr, ptr %in
  %a1 = getelementptr %struct.test08, ptr %p, i64 0, i32 0
  %a2 = getelementptr %struct.test08, ptr %p, i64 0, i32 0
  %v1 = load i32, ptr %a1
  %v2 = load i32, ptr %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: Input Parameters: test08
; CHECK: Arg 0: ptr %p
; CHECK:      Aliased types:
; CHECK:        %struct.test08**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
; CHECK:      LocalPointerInfo:
; CHECK-SAME: PARTIAL PTR
; CHECK:      Aliased types:
; CHECK:        %struct.test08**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
; CHECK:      LocalPointerInfo:
; CHECK-SAME: PARTIAL PTR
; CHECK:      Aliased types:
; CHECK:        %struct.test08**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.


; Swap pointers in 32-bit chunks. This is same as test8 except operands
; of "add" are swapped.
%struct.test09 = type { i32, i32 }
define void @test09(ptr "intel_dtrans_func_index"="1" %p) !intel.dtrans.func.type !27 {
entry:
  ; This is here to establish what %p actually points to.
  %tmp = call i1 @test09identify(ptr %p)
  %other = getelementptr i8, ptr %p, i64 8
  br label %pre_swap

pre_swap:
  br label %swap

swap:
  %Count = phi i32 [ 2, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, ptr %HalfPtr1
  %HalfVal2 = load i32, ptr %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, ptr %HalfPtr1, i64 1
  store i32 %HalfVal2, ptr %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, ptr %HalfPtr2, i64 1
  store i32 %HalfVal1, ptr %HalfPtr2
  %NextCount = add nsw i32 -1, %Count
  %Cmp = icmp ugt i32 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}
define i1 @test09identify(ptr "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !29 {
  %p = load ptr, ptr %in
  %a1 = getelementptr %struct.test09, ptr %p, i64 0, i32 0
  %a2 = getelementptr %struct.test09, ptr %p, i64 0, i32 0
  %v1 = load i32, ptr %a1
  %v2 = load i32, ptr %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}

; CHECK-LABEL: Input Parameters: test09
; CHECK: Arg 0: ptr %p
; CHECK:      Aliased types:
; CHECK:        %struct.test09**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr1 = phi ptr [ %p, %pre_swap ], [ %NextHalf1, %swap ]
; CHECK:      LocalPointerInfo:
; CHECK-SAME: PARTIAL PTR
; CHECK:      Aliased types:
; CHECK:        %struct.test09**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

; CHECK:  %HalfPtr2 = phi ptr [ %other, %pre_swap ], [ %NextHalf2, %swap ]
; CHECK:      LocalPointerInfo:
; CHECK-SAME: PARTIAL PTR
; CHECK:      Aliased types:
; CHECK:        %struct.test09**{{ *}}
; CHECK:        i8*{{ *}}
; CHECK:      No element pointees.

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i8 0, i32 1}  ; i8*
!3 = distinct !{!2}
!4 = !{%struct.test01 zeroinitializer, i32 2}  ; %struct.test01**
!5 = distinct !{!4}
!6 = distinct !{!2}
!7 = !{%struct.test02 zeroinitializer, i32 2}  ; %struct.test02**
!8 = distinct !{!7}
!9 = distinct !{!2}
!10 = !{%struct.test03 zeroinitializer, i32 2}  ; %struct.test03**
!11 = distinct !{!10}
!12 = distinct !{!2}
!13 = !{%struct.test04 zeroinitializer, i32 2}  ; %struct.test04**
!14 = distinct !{!13}
!15 = distinct !{!2}
!16 = !{%struct.test05 zeroinitializer, i32 2}  ; %struct.test05**
!17 = distinct !{!16}
!18 = distinct !{!2}
!19 = !{%struct.test06 zeroinitializer, i32 2}  ; %struct.test06**
!20 = distinct !{!19}
!21 = distinct !{!2}
!22 = !{%struct.test07 zeroinitializer, i32 2}  ; %struct.test07**
!23 = distinct !{!22}
!24 = distinct !{!2}
!25 = !{%struct.test08 zeroinitializer, i32 2}  ; %struct.test08**
!26 = distinct !{!25}
!27 = distinct !{!2}
!28 = !{%struct.test09 zeroinitializer, i32 2}  ; %struct.test09**
!29 = distinct !{!28}
!30 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!31 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!32 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!33 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!34 = !{!"S", %struct.test05 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!35 = !{!"S", %struct.test06 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!36 = !{!"S", %struct.test07 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!37 = !{!"S", %struct.test08 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!38 = !{!"S", %struct.test09 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!intel.dtrans.types = !{!30, !31, !32, !33, !34, !35, !36, !37, !38}
