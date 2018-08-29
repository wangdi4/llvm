; RUN: opt -dtransanalysis -whole-program-assume -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="require<dtransanalysis>" -whole-program-assume -dtrans-print-types -disable-output < %s 2>&1 | FileCheck %s

; Test checks that malloc call with non-constant size is parsed correctly.

%struct.S1 = type { i32 }
%struct.S2 = type { i32 }
%struct.S3 = type { i32, [0 x i8] }
%struct.S4 = type { i32, [0 x i8] }
%struct.S5 = type { i32, [4 x i8] }
%struct.S6 = type { i32, [4 x i8] }

@g_S1 = external dso_local global %struct.S1*, align 8
@g_S2 = external dso_local global %struct.S2*, align 8
@g_S3 = external dso_local global %struct.S3*, align 8
@g_S4 = external dso_local global %struct.S4*, align 8
@g_S5 = external dso_local global %struct.S5*, align 8
@g_S6 = external dso_local global %struct.S6*, align 8

; Check the type with the single i32 field and a malloc with size which doesn't fit into the structure type.
define dso_local void @foo1(i64 %n) {
  entry:
  %shl = mul i64 %n, 3
  %add = add i64 %shl, 2
  %call = tail call noalias i8* @malloc(i64 %add)
  store i8* %call, i8** bitcast (%struct.S1** @g_S1 to i8**), align 8
  ret void
}

; Check the type with the single i32 field and a malloc with size which fits into the structure type.
define dso_local void @foo2(i64 %n) {
  entry:
  %shl = shl i64 %n, 2
  %add = add i64 %shl, 4
  %call = tail call noalias i8* @malloc(i64 %add)
  store i8* %call, i8** bitcast (%struct.S2** @g_S2 to i8**), align 8
  ret void
}

; Check the type with the zero-sized array field and a malloc with size which doesn't fit into the structure type.
define dso_local void @foo3(i64 %n) {
  entry:
  %shl = shl i64 %n, 1
  %add = add i64 %shl, 1
  %call = tail call noalias i8* @malloc(i64 %add)
  store i8* %call, i8** bitcast (%struct.S3** @g_S3 to i8**), align 8
  ret void
}

; Check the type with the zero-sized array field and a malloc with size which fits into the structure type.
define dso_local void @foo4(i64 %n) {
  entry:
  %shl = shl i64 %n, 1
  %add = add i64 %shl, 3
  %call = tail call noalias i8* @malloc(i64 %add)
  store i8* %call, i8** bitcast (%struct.S4** @g_S4 to i8**), align 8
  ret void
}

; Check the type with the fixed-size array field and a malloc with size which doesn't fit into the structure type.
define dso_local void @foo5(i64 %n) {
  entry:
  %shl = shl i64 %n, 1
  %add = add i64 %shl, 4
  %call = tail call noalias i8* @malloc(i64 %add)
  store i8* %call, i8** bitcast (%struct.S5** @g_S5 to i8**), align 8
  ret void
}

; Check the type with the fixed-size array field and a malloc with size which fits into structure type.
define dso_local void @foo6(i64 %n) {
  entry:
  %shl = shl i64 %n, 3
  %add = add i64 %shl, 8
  %call = tail call noalias i8* @malloc(i64 %add)
  store i8* %call, i8** bitcast (%struct.S6** @g_S6 to i8**), align 8
  ret void
}

declare dso_local noalias i8* @malloc(i64) local_unnamed_addr

define dso_local i32 @bar1() local_unnamed_addr #0 {
  entry:
  call void @foo1(i64 1)
  call void @foo2(i64 0)
  call void @foo3(i64 1)
  call void @foo4(i64 2)
  call void @foo5(i64 1)
  call void @foo6(i64 1)

  ret i32 0
}

define dso_local i32 @bar2() local_unnamed_addr #0 {
  entry:
  call void @foo1(i64 2)
  call void @foo2(i64 5)
  call void @foo3(i64 2)
  call void @foo4(i64 5)
  call void @foo5(i64 2)
  call void @foo6(i64 2)
  ret i32 0
}

; CHECK:  LLVMType: %struct.S1 = type { i32 }
; CHECK:  Safety data: Bad alloc size

; CHECK:  LLVMType: %struct.S2 = type { i32 }
; CHECK:  Safety data: No issues found

; CHECK:  LLVMType: %struct.S3 = type { i32, [0 x i8] }
; CHECK:  Safety data: Bad alloc size | Has zero-sized array

; CHECK:  LLVMType: %struct.S4 = type { i32, [0 x i8] }
; CHECK:  Safety data: Has zero-sized array

; CHECK:  LLVMType: %struct.S5 = type { i32, [4 x i8] }
; CHECK:  Safety data: Bad alloc size

; CHECK:  LLVMType: %struct.S6 = type { i32, [4 x i8] }
; CHECK:  Safety data: No issues found

