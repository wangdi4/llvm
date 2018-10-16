; RUN: opt < %s -whole-program-assume  -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test verifies that we are able to identify a couple of idioms for
; swapping pointer values in a dynamic array of pointers

; Swap pointers in 32-bit chunks.
%struct.test01 = type { i32, i32 }
define void @test1(i8* %p) {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test01**
  %other = getelementptr i8, i8* %p, i64 8
  br label %pre_swap

pre_swap:
  %Cast1 = bitcast i8* %p to i32*
  %Cast2 = bitcast i8* %other to i32*
  br label %swap

swap:
  %Count = phi i64 [ 2, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi i32* [ %Cast1, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi i32* [ %Cast2, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, i32* %HalfPtr1
  %HalfVal2 = load i32, i32* %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, i32* %HalfPtr1, i64 1
  store i32 %HalfVal2, i32* %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, i32* %HalfPtr2, i64 1
  store i32 %HalfVal1, i32* %HalfPtr2
  %NextCount = add nsw i64 %Count, -1
  %Cmp = icmp sgt i64 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test01 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Swap pointers in 8-bit chunks.
%struct.test02 = type { i32, i32 }
define void @test2(i8* %p) {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test02**
  %other = getelementptr i8, i8* %p, i64 8
  br label %swap

swap:
  %Count = phi i64 [ 8, %entry ], [ %NextCount, %swap ]
  %PartPtr1 = phi i8* [ %p, %entry ], [ %NextPart1, %swap ]
  %PartPtr2 = phi i8* [ %other, %entry ], [ %NextPart2, %swap ]
  %PartVal1 = load i8, i8* %PartPtr1
  %PartVal2 = load i8, i8* %PartPtr2
  %NextPart1 = getelementptr inbounds i8, i8* %PartPtr1, i64 1
  store i8 %PartVal2, i8* %PartPtr1
  %NextPart2 = getelementptr inbounds i8, i8* %PartPtr2, i64 1
  store i8 %PartVal1, i8* %PartPtr2
  %NextCount = add nsw i64 %Count, -1
  %Cmp = icmp sgt i64 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test02 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Swap a variable number of pointers in 32-bit chunks.
%struct.test03 = type { i32, i32 }
define void @test3(i8* %p, i64 %n) {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test03**
  %offset = mul i64 %n, 8
  %init_count = mul i64 %n, 2
  %other = getelementptr i8, i8* %p, i64 %offset
  br label %pre_swap

pre_swap:
  %Cast1 = bitcast i8* %p to i32*
  %Cast2 = bitcast i8* %other to i32*
  br label %swap

swap:
  %Count = phi i64 [ %init_count, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi i32* [ %Cast1, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi i32* [ %Cast2, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, i32* %HalfPtr1
  %HalfVal2 = load i32, i32* %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, i32* %HalfPtr1, i64 1
  store i32 %HalfVal2, i32* %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, i32* %HalfPtr2, i64 1
  store i32 %HalfVal1, i32* %HalfPtr2
  %NextCount = add nsw i64 %Count, -1
  %Cmp = icmp sgt i64 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test03 = type { i32, i32 }
; CHECK: Safety data: No issues found

; Swap incomplete chunks of pointers.
%struct.test04 = type { i32, i32 }
define void @test4(i8* %p) {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test04**
  %other = getelementptr i8, i8* %p, i64 8
  br label %pre_swap

pre_swap:
  %cur_p = phi i8* [ %p, %entry ], [ %prev_p, %swap ]
  %prev_p = getelementptr inbounds i8, i8* %cur_p, i64 -8
  br i1 undef, label %swap, label %exit

swap:
  %p1 = bitcast i8* %cur_p to i32*
  %val1 = load i32, i32* %p1
  %p2 = bitcast i8* %prev_p to i32*
  %val2 = load i32, i32* %p2
  store i32 %val2, i32* %p1
  store i32 %val1, i32* %p2
  br i1 undef, label %pre_swap, label %exit

exit:
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test04 = type { i32, i32 }
; CHECK: Safety data: Bad casting | Unsafe pointer store | Unsafe pointer merge

; Swap 32-bit chunks without looping.
%struct.test05 = type { i32, i32 }
define void @test5(i8* %p) {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test05**
  %other = getelementptr i8, i8* %p, i64 8
  br label %pre_swap

pre_swap:
  %Cast1 = bitcast i8* %p to i32*
  %Cast2 = bitcast i8* %other to i32*
  br label %swap

swap:
  %HalfPtr1 = phi i32* [ %Cast1, %pre_swap ], [ %other_p1, %other_bb ]
  %HalfPtr2 = phi i32* [ %Cast2, %pre_swap ], [ %other_p2, %other_bb ]
  %HalfVal1 = load i32, i32* %HalfPtr1
  %HalfVal2 = load i32, i32* %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, i32* %HalfPtr1, i64 1
  store i32 %HalfVal2, i32* %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, i32* %HalfPtr2, i64 1
  store i32 %HalfVal1, i32* %HalfPtr2
  br label %other_bb

other_bb:
  %tmp1 = getelementptr i8, i8* %p, i64 16
  %tmp2 = getelementptr i8, i8* %p, i64 24
  %other_p1 = bitcast i8* %tmp1 to i32*
  %other_p2 = bitcast i8* %tmp2 to i32*
  br i1 undef, label %swap, label %exit

exit:
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test05 = type { i32, i32 }
; CHECK: Safety data: Bad casting | Unsafe pointer store | Unsafe pointer merge

; Swap pointers in 32-bit chunks with a bad pointer increment.
%struct.test06 = type { i32, i32 }
define void @test6(i8* %p) {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test06**
  %other = getelementptr i8, i8* %p, i64 8
  br label %pre_swap

pre_swap:
  %Cast1 = bitcast i8* %p to i32*
  %Cast2 = bitcast i8* %other to i32*
  br label %swap

swap:
  %Count = phi i64 [ 2, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi i32* [ %Cast1, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi i32* [ %Cast2, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, i32* %HalfPtr1
  %HalfVal2 = load i32, i32* %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, i32* %HalfPtr1, i64 2
  store i32 %HalfVal2, i32* %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, i32* %HalfPtr2, i64 2
  store i32 %HalfVal1, i32* %HalfPtr2
  %NextCount = add nsw i64 %Count, -1
  %Cmp = icmp sgt i64 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test06 = type { i32, i32 }
; CHECK: Safety data: Bad casting | Unsafe pointer store | Unsafe pointer merge

; Swap a variable number of pointers in 32-bit chunks.
; This is a minor variation of test3
%struct.test07 = type { i32, i32 }
define void @test7(i8* %p, i64 %n) {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test07**
  %offset = mul i64 %n, 8
  %init_count = mul i64 %n, 2
  %other = getelementptr i8, i8* %p, i64 %offset
  br label %pre_swap

pre_swap:
  %Cast1 = bitcast i8* %p to i32*
  %Cast2 = bitcast i8* %other to i32*
  br label %swap

swap:
  %Count = phi i64 [ %init_count, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi i32* [ %Cast1, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi i32* [ %Cast2, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, i32* %HalfPtr1
  %HalfVal2 = load i32, i32* %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, i32* %HalfPtr1, i64 1
  store i32 %HalfVal2, i32* %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, i32* %HalfPtr2, i64 1
  store i32 %HalfVal1, i32* %HalfPtr2
  %NextCount = add nsw i64 %Count, -1
  %Cmp = icmp sgt i64 %NextCount, 0 
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}

; CHECK-LABEL: LLVMType: %struct.test07 = type { i32, i32 }
; CHECK: Safety data: No issues found
