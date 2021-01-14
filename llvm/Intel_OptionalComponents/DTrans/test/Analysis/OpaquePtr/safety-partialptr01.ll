; REQUIRES: asserts
; RUN: opt -whole-program-assume -dtrans-safetyanalyzer -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s
; RUN: opt -whole-program-assume -passes='require<dtrans-safetyanalyzer>' -dtrans-print-types -disable-output %s 2>&1 | FileCheck %s

; Test safety analysis flags for structures involved in the partial pointer swap
; idiom.

; Swap pointers in 32-bit chunks.
%struct.test01 = type { i32, i32 }
define void @test01(i8* %p) !dtrans_type !2 {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test01**
  %tmp = call i1 @test01identify(%struct.test01** %identify)
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
define i1 @test01identify(%struct.test01** %in) !dtrans_type !5 {
  %p = load %struct.test01*, %struct.test01** %in
  %a1 = getelementptr %struct.test01, %struct.test01* %p, i64 0, i32 0
  %a2 = getelementptr %struct.test01, %struct.test01* %p, i64 0, i32 0
  %v1 = load i32, i32* %a1
  %v2 = load i32, i32* %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test01
; CHECK: Safety data: No issues found


; Swap pointers in 8-bit chunks.
%struct.test02 = type { i32, i32 }
define void @test02(i8* %p) !dtrans_type !2 {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test02**
  %tmp = call i1 @test02identify(%struct.test02** %identify)
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
define i1 @test02identify(%struct.test02** %in) !dtrans_type !9 {
  %p = load %struct.test02*, %struct.test02** %in
  %a1 = getelementptr %struct.test02, %struct.test02* %p, i64 0, i32 0
  %a2 = getelementptr %struct.test02, %struct.test02* %p, i64 0, i32 0
  %v1 = load i32, i32* %a1
  %v2 = load i32, i32* %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test02
; CHECK: Safety data: No issues found


; Swap a variable number of pointers in 32-bit chunks.
%struct.test03 = type { i32, i32 }
define void @test03(i8* %p, i64 %n) !dtrans_type !12 {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test03**
  %tmp = call i1 @test03identify(%struct.test03** %identify)
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
define i1 @test03identify(%struct.test03** %in) !dtrans_type !14 {
  %p = load %struct.test03*, %struct.test03** %in
  %a1 = getelementptr %struct.test03, %struct.test03* %p, i64 0, i32 0
  %a2 = getelementptr %struct.test03, %struct.test03* %p, i64 0, i32 0
  %v1 = load i32, i32* %a1
  %v2 = load i32, i32* %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test03
; CHECK: Safety data: No issues found


; Swap incomplete chunks of pointers. This case does not match the partial
; pointer access idiom.
%struct.test04 = type { i32, i32 }
define void @test04(i8* %p) !dtrans_type !2 {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test04**
  %tmp = call i1 @test04identify(%struct.test04** %identify)
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
define i1 @test04identify(%struct.test04** %in) !dtrans_type !17 {
  %p = load %struct.test04*, %struct.test04** %in
  %a1 = getelementptr %struct.test04, %struct.test04* %p, i64 0, i32 0
  %a2 = getelementptr %struct.test04, %struct.test04* %p, i64 0, i32 0
  %v1 = load i32, i32* %a1
  %v2 = load i32, i32* %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test04
; CHECK: Safety data: Bad casting | Ambiguous GEP | Unsafe pointer merge{{ *}}


; Swap 32-bit chunks without looping. This case does not match the partial
; pointer access idiom.
%struct.test05 = type { i32, i32 }
define void @test05(i8* %p) !dtrans_type !2 {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test05**
  %tmp = call i1 @test05identify(%struct.test05** %identify)
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
define i1 @test05identify(%struct.test05** %in) !dtrans_type !20 {
  %p = load %struct.test05*, %struct.test05** %in
  %a1 = getelementptr %struct.test05, %struct.test05* %p, i64 0, i32 0
  %a2 = getelementptr %struct.test05, %struct.test05* %p, i64 0, i32 0
  %v1 = load i32, i32* %a1
  %v2 = load i32, i32* %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test05
; CHECK: Safety data: Bad casting | Ambiguous GEP | Unsafe pointer merge{{ *}}


; Swap pointers in 32-bit chunks with a bad pointer increment. This case does
; not match the partial pointer access idiom.
%struct.test06 = type { i32, i32 }
define void @test06(i8* %p) !dtrans_type !2 {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test06**
  %tmp = call i1 @test06identify(%struct.test06** %identify)
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
define i1 @test06identify(%struct.test06** %in) !dtrans_type !23 {
  %p = load %struct.test06*, %struct.test06** %in
  %a1 = getelementptr %struct.test06, %struct.test06* %p, i64 0, i32 0
  %a2 = getelementptr %struct.test06, %struct.test06* %p, i64 0, i32 0
  %v1 = load i32, i32* %a1
  %v2 = load i32, i32* %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test06
; CHECK: Safety data: Bad casting | Ambiguous GEP | Unsafe pointer merge{{ *}}


; Swap a variable number of pointers in 32-bit chunks. This is a minor variation
; of test3 WRT the loop control test to match the pattern seen following some
; upstream transformation changes.
%struct.test07 = type { i32, i32 }
define void @test07(i8* %p, i64 %n) !dtrans_type !12 {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test07**
  %tmp = call i1 @test07identify(%struct.test07** %identify)
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
define i1 @test07identify(%struct.test07** %in) !dtrans_type !26 {
  %p = load %struct.test07*, %struct.test07** %in
  %a1 = getelementptr %struct.test07, %struct.test07* %p, i64 0, i32 0
  %a2 = getelementptr %struct.test07, %struct.test07* %p, i64 0, i32 0
  %v1 = load i32, i32* %a1
  %v2 = load i32, i32* %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test07
; CHECK: Safety data: No issues found


; Swap pointers in 32-bit chunks. This is same as test1 except the type of
; %Count is i32.
%struct.test08 = type { i32, i32 }
define void @test08(i8* %p) !dtrans_type !2 {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test08**
  %tmp = call i1 @test08identify(%struct.test08** %identify)
  %other = getelementptr i8, i8* %p, i64 8
  br label %pre_swap

pre_swap:
  %Cast1 = bitcast i8* %p to i32*
  %Cast2 = bitcast i8* %other to i32*
  br label %swap

swap:
  %Count = phi i32 [ 2, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi i32* [ %Cast1, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi i32* [ %Cast2, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, i32* %HalfPtr1
  %HalfVal2 = load i32, i32* %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, i32* %HalfPtr1, i64 1
  store i32 %HalfVal2, i32* %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, i32* %HalfPtr2, i64 1
  store i32 %HalfVal1, i32* %HalfPtr2
  %NextCount = add nsw i32 %Count, -1
  %Cmp = icmp sgt i32 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}
define i1 @test08identify(%struct.test08** %in) !dtrans_type !29 {
  %p = load %struct.test08*, %struct.test08** %in
  %a1 = getelementptr %struct.test08, %struct.test08* %p, i64 0, i32 0
  %a2 = getelementptr %struct.test08, %struct.test08* %p, i64 0, i32 0
  %v1 = load i32, i32* %a1
  %v2 = load i32, i32* %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test08
; CHECK: Safety data: No issues found


; Swap pointers in 32-bit chunks. This is same as test8 except operands
; of "add" are swapped.
%struct.test09 = type { i32, i32 }
define void @test09(i8* %p) !dtrans_type !2 {
entry:
  ; This is here to establish what %p actually points to.
  %identify = bitcast i8* %p to %struct.test09**
  %tmp = call i1 @test09identify(%struct.test09** %identify)
  %other = getelementptr i8, i8* %p, i64 8
  br label %pre_swap

pre_swap:
  %Cast1 = bitcast i8* %p to i32*
  %Cast2 = bitcast i8* %other to i32*
  br label %swap

swap:
  %Count = phi i32 [ 2, %pre_swap ], [ %NextCount, %swap ]
  %HalfPtr1 = phi i32* [ %Cast1, %pre_swap ], [ %NextHalf1, %swap ]
  %HalfPtr2 = phi i32* [ %Cast2, %pre_swap ], [ %NextHalf2, %swap ]
  %HalfVal1 = load i32, i32* %HalfPtr1
  %HalfVal2 = load i32, i32* %HalfPtr2
  %NextHalf1 = getelementptr inbounds i32, i32* %HalfPtr1, i64 1
  store i32 %HalfVal2, i32* %HalfPtr1
  %NextHalf2 = getelementptr inbounds i32, i32* %HalfPtr2, i64 1
  store i32 %HalfVal1, i32* %HalfPtr2
  %NextCount = add nsw i32 -1, %Count
  %Cmp = icmp sgt i32 %Count, 1
  br i1 %Cmp, label %swap, label %exit

exit:
  ret void
}
define i1 @test09identify(%struct.test09** %in) !dtrans_type !32 {
  %p = load %struct.test09*, %struct.test09** %in
  %a1 = getelementptr %struct.test09, %struct.test09* %p, i64 0, i32 0
  %a2 = getelementptr %struct.test09, %struct.test09* %p, i64 0, i32 0
  %v1 = load i32, i32* %a1
  %v2 = load i32, i32* %a2
  %cmp = icmp sgt i32 %v1, %v2
  ret i1 %cmp
}
; CHECK-LABEL: DTRANS_StructInfo:
; CHECK: Name: struct.test09
; CHECK: Safety data: No issues found


!1 = !{i32 0, i32 0}  ; i32
!2 = !{!"F", i1 false, i32 1, !3, !4}  ; void (i8*)
!3 = !{!"void", i32 0}  ; void
!4 = !{i8 0, i32 1}  ; i8*
!5 = !{!"F", i1 false, i32 1, !6, !7}  ; i1 (%struct.test01**)
!6 = !{i1 0, i32 0}  ; i1
!7 = !{!8, i32 2}  ; %struct.test01**
!8 = !{!"R", %struct.test01 zeroinitializer, i32 0}  ; %struct.test01
!9 = !{!"F", i1 false, i32 1, !6, !10}  ; i1 (%struct.test02**)
!10 = !{!11, i32 2}  ; %struct.test02**
!11 = !{!"R", %struct.test02 zeroinitializer, i32 0}  ; %struct.test02
!12 = !{!"F", i1 false, i32 2, !3, !4, !13}  ; void (i8*, i64)
!13 = !{i64 0, i32 0}  ; i64
!14 = !{!"F", i1 false, i32 1, !6, !15}  ; i1 (%struct.test03**)
!15 = !{!16, i32 2}  ; %struct.test03**
!16 = !{!"R", %struct.test03 zeroinitializer, i32 0}  ; %struct.test03
!17 = !{!"F", i1 false, i32 1, !6, !18}  ; i1 (%struct.test04**)
!18 = !{!19, i32 2}  ; %struct.test04**
!19 = !{!"R", %struct.test04 zeroinitializer, i32 0}  ; %struct.test04
!20 = !{!"F", i1 false, i32 1, !6, !21}  ; i1 (%struct.test05**)
!21 = !{!22, i32 2}  ; %struct.test05**
!22 = !{!"R", %struct.test05 zeroinitializer, i32 0}  ; %struct.test05
!23 = !{!"F", i1 false, i32 1, !6, !24}  ; i1 (%struct.test06**)
!24 = !{!25, i32 2}  ; %struct.test06**
!25 = !{!"R", %struct.test06 zeroinitializer, i32 0}  ; %struct.test06
!26 = !{!"F", i1 false, i32 1, !6, !27}  ; i1 (%struct.test07**)
!27 = !{!28, i32 2}  ; %struct.test07**
!28 = !{!"R", %struct.test07 zeroinitializer, i32 0}  ; %struct.test07
!29 = !{!"F", i1 false, i32 1, !6, !30}  ; i1 (%struct.test08**)
!30 = !{!31, i32 2}  ; %struct.test08**
!31 = !{!"R", %struct.test08 zeroinitializer, i32 0}  ; %struct.test08
!32 = !{!"F", i1 false, i32 1, !6, !33}  ; i1 (%struct.test09**)
!33 = !{!34, i32 2}  ; %struct.test09**
!34 = !{!"R", %struct.test09 zeroinitializer, i32 0}  ; %struct.test09
!35 = !{!"S", %struct.test01 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!36 = !{!"S", %struct.test02 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!37 = !{!"S", %struct.test03 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!38 = !{!"S", %struct.test04 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!39 = !{!"S", %struct.test05 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!40 = !{!"S", %struct.test06 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!41 = !{!"S", %struct.test07 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!42 = !{!"S", %struct.test08 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }
!43 = !{!"S", %struct.test09 zeroinitializer, i32 2, !1, !1} ; { i32, i32 }

!dtrans_types = !{!35, !36, !37, !38, !39, !40, !41, !42, !43}
