; This test is to verify the complete transpose candidate
; selection/profitability/transformation implementation for a
; case where the result of a subscript call gets put into a
; PHINode. (CMPLRLLVM-27761)

; RUN: opt < %s -S -dtrans-transpose 2>&1 | FileCheck %s
; RUN: opt < %s -S -passes=dtrans-transpose 2>&1 | FileCheck %s

; This case should be transformed to swap the first and last strides.
@test_var01 = internal global [16 x [16 x [16 x i32]]] zeroinitializer
define void @test01() {
  %"var$01" = alloca { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }
  %"var$01_$field0$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 0
  %"var$01_$field1$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 1
  %"var$01_$field2$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 2
  %"var$01_$field3$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 3
  %"var$01_$field4$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 4
  %"var$01_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %"var$01", i64 0, i32 6, i64 0
  %"var$01_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 0
  %"var$01_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 1
  %"var$01_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 2
  store i64 4, i64* %"var$01_$field1$"
  store i64 3, i64* %"var$01_$field4$"
  store i64 0, i64* %"var$01_$field2$"
  %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 0)
  store i64 4, i64* %t0
  %t1 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field2$", i32 0)
  store i64 1, i64* %t1
  %t2 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field0$", i32 0)
  store i64 16, i64* %t2
  %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 1)
  store i64 64, i64* %t3
  %t4 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field2$", i32 1)
  store i64 1, i64* %t4
  %t5 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field0$", i32 1)
  store i64 16, i64* %t5
  %t6 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 2)
  store i64 1024, i64* %t6
  %t7 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field2$", i32 2)
  store i64 1, i64* %t7
  %t8 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field0$", i32 2)
  store i64 16, i64* %t8
  store i32* getelementptr inbounds ([16 x [16 x [16 x i32]]], [16 x [16 x [16 x i32]]]* @test_var01, i64 0, i64 0, i64 0, i64 0), i32** %"var$01_$field0$"
  store i64 1, i64* %"var$01_$field3$"

  call void @test01dv({ i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }]}* %"var$01")
  call void @test02dv({ i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }]}* %"var$01")
  ret void
}
; CHECK-LABEL: define void @test01
; CHECK: %t0 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 0)
; CHECK: store i64 1024, i64* %t0
; CHECK: %t3 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 1)
; CHECK: store i64 64, i64* %t3
; CHECK: %t6 = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* nonnull elementtype(i64) %"var$01_$field6$_$field1$", i32 2)
; CHECK: store i64 4, i64* %t6

; This routine uses the subscript result in a PHINode
define void @test01dv({ i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* noalias nocapture readonly %MYBLOCK) {
entry:
  ; Load the address of the array variable.
  %"MYBLOCK_$field0$" = getelementptr { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %MYBLOCK, i64 0, i32 0
  %"MYBLOCK_$field0$1" = load i32*, i32** %"MYBLOCK_$field0$"

  ; Load the stride for each dimension of the array
  %"var$01_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %MYBLOCK, i64 0, i32 6, i64 0
  %"var$01_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$01_$field6$", i64 0, i32 1

  %stride0_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$01_$field6$_$field1$", i32 0)
  %stride1_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$01_$field6$_$field1$", i32 1)
  %stride2_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$01_$field6$_$field1$", i32 2)
  %stride0 = load i64, i64* %stride0_addr
  %stride1 = load i64, i64* %stride1_addr
  %stride2 = load i64, i64* %stride2_addr

  br label %loop1_top
loop1_top:
  %loop1_cnt = phi i64 [1, %entry], [%loop1_cnt1, %loop1_bottom]
  br label %loop2_top
loop2_top:
  %loop2_cnt = phi i64 [1, %loop1_top], [%loop2_cnt, %loop2_bottom]
  br label %loop3_top
loop3_top:
  %loop3_cnt = phi i64 [1, %loop2_top], [%loop3_cnt1, %loop3_body]
  br i1 undef, label %loop3_topA, label %loop3_topB

loop3_topA:
  %ptr_part1A = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 %stride2, i32* elementtype(i32) %"MYBLOCK_$field0$1", i64 %loop3_cnt)
  br label %loop3_body

loop3_topB:
  %ptr_part1B = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 %stride2, i32* elementtype(i32) %"MYBLOCK_$field0$1", i64 %loop3_cnt)
  br label %loop3_body

loop3_body:
  ; Test using a PHINode on the subscript result.
  %ptr_part1.phi = phi i32* [ %ptr_part1A, %loop3_topA ], [ %ptr_part1B, %loop3_topB ]
  %ptr_part2 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %stride1, i32* elementtype(i32) %ptr_part1.phi, i64 %loop2_cnt)
  %ptr = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride0, i32* elementtype(i32) %ptr_part2, i64 %loop1_cnt)

  %val = load i32, i32* %ptr
  %newval = add i32 %val, 100
  store i32 %newval, i32* %ptr

  ; Test using a select instruction on the subscript result.
  %ptr_part2b = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %stride1, i32* elementtype(i32) %ptr_part1.phi, i64 %loop2_cnt)
  %ptr_part2.sel = select i1 undef, i32* %ptr_part2, i32* %ptr_part2b
  %ptr.sel = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride0, i32* elementtype(i32) %ptr_part2.sel, i64 %loop1_cnt)
  %val.sel = load i32, i32* %ptr.sel

  %loop3_cnt1 = add nsw i64 %loop3_cnt, 1
  %cmp3 = icmp ult i64 %loop3_cnt1, 16
  br i1 %cmp3, label %loop3_top, label %loop2_bottom

loop2_bottom:
  %loop2_cnt1 = add nsw i64 %loop2_cnt, 1
  %cmp2 = icmp ult i64 %loop2_cnt1, 16
  br i1 %cmp2, label %loop2_top, label %loop1_bottom

loop1_bottom:
  %loop1_cnt1 = add nsw i64 %loop1_cnt, 1
  %cmp1 = icmp ult i64 %loop1_cnt1, 16
  br i1 %cmp2, label %loop1_top, label %exit

exit:
  ret void
}
; CHECK-LABEL: define void @test01dv
; CHECK: %ptr_part1A = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride2, i32* elementtype(i32) %"MYBLOCK_$field0$1", i64 %loop3_cnt)
; CHECK: %ptr_part1B = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride2, i32* elementtype(i32) %"MYBLOCK_$field0$1", i64 %loop3_cnt)

; CHECK: %ptr_part1.phi = phi i32* [ %ptr_part1A, %loop3_topA ], [ %ptr_part1B, %loop3_topB ]
; CHECK: %ptr_part2 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %stride1, i32* elementtype(i32) %ptr_part1.phi, i64 %loop2_cnt)
; CHECK: %ptr = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 %stride0, i32* elementtype(i32) %ptr_part2, i64 %loop1_cnt)

; CHECK: %val = load i32, i32* %ptr
; CHECK: %newval = add i32 %val, 100
; CHECK: store i32 %newval, i32* %ptr

; CHECK: %ptr_part2b = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %stride1, i32* elementtype(i32) %ptr_part1.phi, i64 %loop2_cnt)
; CHECK: %ptr_part2.sel = select i1 undef, i32* %ptr_part2, i32* %ptr_part2b
; CHECK: %ptr.sel = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 %stride0, i32* elementtype(i32) %ptr_part2.sel, i64 %loop1_cnt)
; CHECK: %val.sel = load i32, i32* %ptr.sel

; This function is just here to trigger the profitabilty condition to make sure
; the transformation happens.
define void @test02dv({ i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* noalias nocapture readonly %MYBLOCK) {
entry:
  ; Load the address of the array variable.
  %"MYBLOCK_$field0$" = getelementptr { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %MYBLOCK, i64 0, i32 0
  %"MYBLOCK_$field0$1" = load i32*, i32** %"MYBLOCK_$field0$"

  ; Load the stride for each dimension of the array
  %"var$02_$field6$" = getelementptr inbounds { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, { i32*, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }* %MYBLOCK, i64 0, i32 6, i64 0
  %"var$02_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"var$02_$field6$", i64 0, i32 1

  %stride0_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$02_$field6$_$field1$", i32 0)
  %stride1_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$02_$field6$_$field1$", i32 1)
  %stride2_addr = tail call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"var$02_$field6$_$field1$", i32 2)
  %stride0 = load i64, i64* %stride0_addr
  %stride1 = load i64, i64* %stride1_addr
  %stride2 = load i64, i64* %stride2_addr

  br label %loop1_top
loop1_top:
  %loop1_cnt = phi i64 [1, %entry], [%loop1_cnt1, %loop1_bottom]
  br label %loop2_top
loop2_top:
  %loop2_cnt = phi i64 [1, %loop1_top], [%loop2_cnt, %loop2_bottom]
  br label %loop3_top
loop3_top:
  %loop3_cnt = phi i64 [1, %loop2_top], [%loop3_cnt1, %loop3_top]
  %ptr_part1 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 %stride2, i32* elementtype(i32) %"MYBLOCK_$field0$1", i64 %loop3_cnt)
  %ptr_part2 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %stride1, i32* elementtype(i32) %ptr_part1, i64 %loop2_cnt)
  %ptr = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride0, i32* elementtype(i32) %ptr_part2, i64 %loop1_cnt)

  %val = load i32, i32* %ptr
  %newval = add i32 %val, 100
  store i32 %newval, i32* %ptr

  %loop3_cnt1 = add nsw i64 %loop3_cnt, 1
  %cmp3 = icmp ult i64 %loop3_cnt1, 16
  br i1 %cmp3, label %loop3_top, label %loop2_bottom

loop2_bottom:
  %loop2_cnt1 = add nsw i64 %loop2_cnt, 1
  %cmp2 = icmp ult i64 %loop2_cnt1, 16
  br i1 %cmp2, label %loop2_top, label %loop1_bottom

loop1_bottom:
  %loop1_cnt1 = add nsw i64 %loop1_cnt, 1
  %cmp1 = icmp ult i64 %loop1_cnt1, 16
  br i1 %cmp2, label %loop1_top, label %exit

exit:
  ret void
}
; CHECK-LABEL: define void @test02dv
; CHECK: %ptr_part1 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 %stride2, i32* elementtype(i32) %"MYBLOCK_$field0$1", i64 %loop3_cnt)
; CHECK: %ptr_part2 = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 1, i64 1, i64 %stride1, i32* elementtype(i32) %ptr_part1, i64 %loop2_cnt)
; CHECK: %ptr = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 2, i64 1, i64 %stride0, i32* elementtype(i32) %ptr_part2, i64 %loop1_cnt)

declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64)
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8, i64, i32, i64*, i32)
