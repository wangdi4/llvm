; RUN: opt < %s -hir-ssa-deconstruction -hir-runtime-dd -print-before=hir-runtime-dd -print-after=hir-runtime-dd 2>&1 | FileCheck %s

; Verify that the test is successfully compiled. Originally, there was an 
; overflow during lcm computation when runtime dd invoked addition of
; CanonExprs. The overflow led to an assertion when the resulting lcm of 0 was
; being set as the denominator.


; CHECK: Dump Before HIR RuntimeDD
; CHECK: %9 = (@_Z21ut_crc32_slice8_table)[0][0][(%1)/u72057594037927936];

; CHECK: Dump After HIR RuntimeDD
; CHECK: %9 = (@_Z21ut_crc32_slice8_table)[0][0][(%1)/u72057594037927936];


@_Z21ut_crc32_slice8_table = internal unnamed_addr global [8 x [256 x i32]] zeroinitializer, align 16

define void @foo(i32 %crc.init, i64 %len.init, i64 %buf.init) {
entry:
 br label %while.body9

while.body9:                                      ; preds = %entry, %while.body9
  %crc.3637 = phi i32 [ %xor26.i.i, %while.body9 ], [ %crc.init, %entry ]
  %len.addr.3636 = phi i64 [ %sub.i, %while.body9 ], [ %len.init, %entry ]
  %buf.addr.sroa.0.3635 = phi i64 [ %11, %while.body9 ], [ %buf.init, %entry ]
  %0 = inttoptr i64 %buf.addr.sroa.0.3635 to i64*
  %1 = load i64, i64* %0, align 8
  %conv.i.i = zext i32 %crc.3637 to i64
  %xor.i.i = xor i64 %1, %conv.i.i
  %and.i.i = and i64 %xor.i.i, 255
  %arrayidx.i.i = getelementptr inbounds [8 x [256 x i32]], [8 x [256 x i32]]* @_Z21ut_crc32_slice8_table, i64 0, i64 7, i64 %and.i.i
  %2 = load i32, i32* %arrayidx.i.i, align 4
  %shr.i.i = lshr i64 %xor.i.i, 8
  %and1.i.i = and i64 %shr.i.i, 255
  %arrayidx2.i.i = getelementptr inbounds [8 x [256 x i32]], [8 x [256 x i32]]* @_Z21ut_crc32_slice8_table, i64 0, i64 6, i64 %and1.i.i
  %3 = load i32, i32* %arrayidx2.i.i, align 4
  %xor3.i.i = xor i32 %3, %2
  %shr4.i.i = lshr i64 %xor.i.i, 16
  %and5.i.i = and i64 %shr4.i.i, 255
  %arrayidx6.i.i = getelementptr inbounds [8 x [256 x i32]], [8 x [256 x i32]]* @_Z21ut_crc32_slice8_table, i64 0, i64 5, i64 %and5.i.i
  %4 = load i32, i32* %arrayidx6.i.i, align 4
  %xor7.i.i = xor i32 %xor3.i.i, %4
  %shr8.i.i = lshr i64 %xor.i.i, 24
  %and9.i.i = and i64 %shr8.i.i, 255
  %arrayidx10.i.i = getelementptr inbounds [8 x [256 x i32]], [8 x [256 x i32]]* @_Z21ut_crc32_slice8_table, i64 0, i64 4, i64 %and9.i.i
  %5 = load i32, i32* %arrayidx10.i.i, align 4
  %xor11.i.i = xor i32 %xor7.i.i, %5
  %shr12.i.i = lshr i64 %1, 32
  %and13.i.i = and i64 %shr12.i.i, 255
  %arrayidx14.i.i = getelementptr inbounds [8 x [256 x i32]], [8 x [256 x i32]]* @_Z21ut_crc32_slice8_table, i64 0, i64 3, i64 %and13.i.i
  %6 = load i32, i32* %arrayidx14.i.i, align 4
  %xor15.i.i = xor i32 %xor11.i.i, %6
  %shr16.i.i = lshr i64 %1, 40
  %and17.i.i = and i64 %shr16.i.i, 255
  %arrayidx18.i.i = getelementptr inbounds [8 x [256 x i32]], [8 x [256 x i32]]* @_Z21ut_crc32_slice8_table, i64 0, i64 2, i64 %and17.i.i
  %7 = load i32, i32* %arrayidx18.i.i, align 4
  %xor19.i.i = xor i32 %xor15.i.i, %7
  %shr20.i.i = lshr i64 %1, 48
  %and21.i.i = and i64 %shr20.i.i, 255
  %arrayidx22.i.i = getelementptr inbounds [8 x [256 x i32]], [8 x [256 x i32]]* @_Z21ut_crc32_slice8_table, i64 0, i64 1, i64 %and21.i.i
  %8 = load i32, i32* %arrayidx22.i.i, align 4
  %xor23.i.i = xor i32 %xor19.i.i, %8
  %shr24.i.i = lshr i64 %1, 56
  %arrayidx25.i.i = getelementptr inbounds [8 x [256 x i32]], [8 x [256 x i32]]* @_Z21ut_crc32_slice8_table, i64 0, i64 0, i64 %shr24.i.i
  %9 = load i32, i32* %arrayidx25.i.i, align 4
  %xor26.i.i = xor i32 %xor23.i.i, %9
  %10 = inttoptr i64 %buf.addr.sroa.0.3635 to i8*
  %add.ptr.i = getelementptr inbounds i8, i8* %10, i64 8
  %11 = ptrtoint i8* %add.ptr.i to i64
  %sub.i = add i64 %len.addr.3636, -8
  %cmp8 = icmp ugt i64 %sub.i, 7
  br i1 %cmp8, label %while.body9, label %while.cond11.preheader.loopexit

while.cond11.preheader.loopexit:
  ret void
}
