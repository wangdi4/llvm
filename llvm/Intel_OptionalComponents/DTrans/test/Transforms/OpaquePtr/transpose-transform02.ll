; This test is to verify the complete transpose candidate
; selection/profitability/transformation implementation.

target triple = "x86_64-unknown-linux-gnu"

; RUN: opt -opaque-pointers < %s -S -passes=dtrans-transpose 2>&1 | FileCheck %s

@test_var01 = internal global [16 x [16 x [16 x i32]]] zeroinitializer
@test_var02 = internal global [16 x [16 x [16 x i32]]] zeroinitializer

; This case should be transformed to swap the first and last strides.
define void @test01() {
entry:
  br label %loop1_top

loop1_top:                                        ; preds = %loop1_bottom, %entry
  %loop1_cnt = phi i64 [ 1, %entry ], [ %loop1_cnt1, %loop1_bottom ]
  br label %loop2_top

loop2_top:                                        ; preds = %loop2_bottom, %loop1_top
  %loop2_cnt = phi i64 [ 1, %loop1_top ], [ %loop2_cnt, %loop2_bottom ]
  br label %loop3_top

loop3_top:                                        ; preds = %loop3_top, %loop2_top
  %loop3_cnt = phi i64 [ 1, %loop2_top ], [ %loop3_cnt1, %loop3_top ]
  %ptr_part1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 1024, ptr elementtype(i32) @test_var01, i64 %loop3_cnt)
  %ptr_part2 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 64, ptr elementtype(i32) %ptr_part1, i64 %loop2_cnt)
  %ptr = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4, ptr elementtype(i32) %ptr_part2, i64 %loop1_cnt)
  %val = load i32, ptr %ptr, align 4
  %newval = add i32 %val, 100
  store i32 %newval, ptr %ptr, align 4
  %loop3_cnt1 = add nsw i64 %loop3_cnt, 1
  %cmp3 = icmp ult i64 %loop3_cnt1, 16
  br i1 %cmp3, label %loop3_top, label %loop2_bottom

loop2_bottom:                                     ; preds = %loop3_top
  %loop2_cnt1 = add nsw i64 %loop2_cnt, 1
  %cmp2 = icmp ult i64 %loop2_cnt1, 16
  br i1 %cmp2, label %loop2_top, label %loop1_bottom

loop1_bottom:                                     ; preds = %loop2_bottom
  %loop1_cnt1 = add nsw i64 %loop1_cnt, 1
  %cmp1 = icmp ult i64 %loop1_cnt1, 16
  br i1 %cmp2, label %loop1_top, label %exit

exit:                                             ; preds = %loop1_bottom
  ret void
}
; Verify the dimension and stride values were updated.
; CHECK-LABEL: define void @test01
; CHECK: %ptr_part1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 4,
; CHECK: %ptr_part2 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 64,
; CHECK: %ptr = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 1024,

; This case should be transformed to swap the first and last strides.
define void @test02() {
bb:
  %"var$02" = alloca { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, align 8
  %"var$02_$field0$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 0
  %"var$02_$field1$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 1
  %"var$02_$field2$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 2
  %"var$02_$field3$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 3
  %"var$02_$field4$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 4
  %"var$02_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %"var$02", i64 0, i32 6, i64 0
  %"var$02_$field6$_$field0$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$02_$field6$", i64 0, i32 0
  %"var$02_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$02_$field6$", i64 0, i32 1
  %"var$02_$field6$_$field2$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$02_$field6$", i64 0, i32 2
  store i64 4, ptr %"var$02_$field1$", align 4
  store i64 3, ptr %"var$02_$field4$", align 4
  store i64 0, ptr %"var$02_$field2$", align 4
  %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field1$", i32 0)
  store i64 4, ptr %t0, align 4
  %t1 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field2$", i32 0)
  store i64 1, ptr %t1, align 4
  %t2 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field0$", i32 0)
  store i64 16, ptr %t2, align 4
  %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field1$", i32 1)
  store i64 64, ptr %t3, align 4
  %t4 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field2$", i32 1)
  store i64 1, ptr %t4, align 4
  %t5 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field0$", i32 1)
  store i64 16, ptr %t5, align 4
  %t6 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field1$", i32 2)
  store i64 1024, ptr %t6, align 4
  %t7 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field2$", i32 2)
  store i64 1, ptr %t7, align 4
  %t8 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field0$", i32 2)
  store i64 16, ptr %t8, align 4
  store ptr @test_var02, ptr %"var$02_$field0$", align 8
  store i64 1, ptr %"var$02_$field3$", align 4
  call void @test02dv(ptr %"var$02")
  ret void
}

define void @test02dv(ptr noalias nocapture readonly %MYBLOCK) {
entry:
  %"MYBLOCK_$field0$" = getelementptr { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %MYBLOCK, i64 0, i32 0
  %"MYBLOCK_$field0$1" = load ptr, ptr %"MYBLOCK_$field0$", align 8
  %"var$02_$field6$" = getelementptr inbounds { ptr, i64, i64, i64, i64, i64, [3 x { i64, i64, i64 }] }, ptr %MYBLOCK, i64 0, i32 6, i64 0
  %"var$02_$field6$_$field1$" = getelementptr inbounds { i64, i64, i64 }, ptr %"var$02_$field6$", i64 0, i32 1
  %stride0_addr = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) %"var$02_$field6$_$field1$", i32 0)
  %stride1_addr = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) %"var$02_$field6$_$field1$", i32 1)
  %stride2_addr = tail call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr elementtype(i64) %"var$02_$field6$_$field1$", i32 2)
  %stride0 = load i64, ptr %stride0_addr, align 4
  %stride1 = load i64, ptr %stride1_addr, align 4
  %stride2 = load i64, ptr %stride2_addr, align 4
  br label %loop1_top

loop1_top:                                        ; preds = %loop1_bottom, %entry
  %loop1_cnt = phi i64 [ 1, %entry ], [ %loop1_cnt1, %loop1_bottom ]
  br label %loop2_top

loop2_top:                                        ; preds = %loop2_bottom, %loop1_top
  %loop2_cnt = phi i64 [ 1, %loop1_top ], [ %loop2_cnt, %loop2_bottom ]
  br label %loop3_top

loop3_top:                                        ; preds = %loop3_top, %loop2_top
  %loop3_cnt = phi i64 [ 1, %loop2_top ], [ %loop3_cnt1, %loop3_top ]
  %ptr_part1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %stride2, ptr elementtype(i32) %"MYBLOCK_$field0$1", i64 %loop3_cnt)
  %ptr_part2 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %stride1, ptr elementtype(i32) %ptr_part1, i64 %loop2_cnt)
  %ptr = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %stride0, ptr elementtype(i32) %ptr_part2, i64 %loop1_cnt)
  %val = load i32, ptr %ptr, align 4
  %newval = add i32 %val, 100
  store i32 %newval, ptr %ptr, align 4
  %loop3_cnt1 = add nsw i64 %loop3_cnt, 1
  %cmp3 = icmp ult i64 %loop3_cnt1, 16
  br i1 %cmp3, label %loop3_top, label %loop2_bottom

loop2_bottom:                                     ; preds = %loop3_top
  %loop2_cnt1 = add nsw i64 %loop2_cnt, 1
  %cmp2 = icmp ult i64 %loop2_cnt1, 16
  br i1 %cmp2, label %loop2_top, label %loop1_bottom

loop1_bottom:                                     ; preds = %loop2_bottom
  %loop1_cnt1 = add nsw i64 %loop1_cnt, 1
  %cmp1 = icmp ult i64 %loop1_cnt1, 16
  br i1 %cmp2, label %loop1_top, label %exit

exit:                                             ; preds = %loop1_bottom
  ret void
}
; CHECK-LABEL: define void @test02
; CHECK: %t0 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field1$", i32 0)
; CHECK: store i64 1024, ptr %t0
; CHECK: %t3 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field1$", i32 1)
; CHECK: store i64 64, ptr %t3
; CHECK: %t6 = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"var$02_$field6$_$field1$", i32 2)
; CHECK: store i64 4, ptr %t6

; CHECK-LABEL: define void @test02dv
; CHECK: %ptr_part1 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %stride2, ptr elementtype(i32) %"MYBLOCK_$field0$1", i64 %loop3_cnt)
; CHECK: %ptr_part2 = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 1, i64 1, i64 %stride1, ptr elementtype(i32) %ptr_part1, i64 %loop2_cnt)
; CHECK: %ptr = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 2, i64 1, i64 %stride0, ptr elementtype(i32) %ptr_part2, i64 %loop1_cnt)


declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #0
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #0

attributes #0 = { nounwind readnone speculatable }
