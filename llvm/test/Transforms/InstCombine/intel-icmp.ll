; This test checks that comparison sizes are optimized and 
; unnecessary sext/zext or other instructions are eliminated
;
; RUN: opt < %s -instcombine -S -mtriple=x86_64-unknown-linux-gnu | FileCheck %s

define i32 @test_s32_to_s8(i32 %a, i8 %b) {
; CHECK-LABEL: @test_s32_to_s8(
; CHECK-NEXT:    [[TMP1:%.*]] = trunc i32 [[A:%.*]] to i8
; CHECK-NEXT:    [[TMP2:%.*]] = and i8 [[TMP1]], 127
; CHECK-NEXT:    [[E:%.*]] = icmp sge i8 [[TMP2]], [[B:%.*]]
  %c = and i32 %a, 127
  %d = sext i8 %b to i32
  %e = icmp sge i32 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_s32_to_s16(i32 %a, i16 %b) {
; CHECK-LABEL: @test_s32_to_s16(
; CHECK-NEXT:    [[TMP1:%.*]] = trunc i32 [[A:%.*]] to i16
; CHECK-NEXT:    [[TMP2:%.*]] = and i16 [[TMP1]], 1000
; CHECK-NEXT:    [[E:%.*]] = icmp sge i16 [[TMP2]], [[B:%.*]]
  %c = and i32 %a, 1000
  %d = sext i16 %b to i32
  %e = icmp sge i32 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_s64_to_s32(i64 %a, i32 %b) {
; CHECK-LABEL: @test_s64_to_s32(
; CHECK-NEXT:    [[TMP1:%.*]] = trunc i64 [[A:%.*]] to i32
; CHECK-NEXT:    [[TMP2:%.*]] = and i32 [[TMP1]], 2147483647
; CHECK-NEXT:    [[E:%.*]] = icmp sge i32 [[TMP2]], [[B:%.*]]
  %c = and i64 %a, 2147483647
  %d = sext i32 %b to i64
  %e = icmp sge i64 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_u32_to_u8(i32 %a, i8 %b) {
; CHECK-LABEL: @test_u32_to_u8(
; CHECK-NEXT:    [[TMP1:%.*]] = trunc i32 [[A:%.*]] to i8
; CHECK-NEXT:    [[TMP2:%.*]] = and i8 [[TMP1]], -2
; CHECK-NEXT:    [[E:%.*]] = icmp uge i8 [[TMP2]], [[B:%.*]]
  %c = and i32 %a, 254
  %d = zext i8 %b to i32
  %e = icmp uge i32 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_u32_to_u16(i32 %a, i16 %b) {
; CHECK-LABEL: @test_u32_to_u16(
; CHECK-NEXT:    [[TMP1:%.*]] = trunc i32 [[A:%.*]] to i16
; CHECK-NEXT:    [[TMP2:%.*]] = and i16 [[TMP1]], 1000
; CHECK-NEXT:    [[E:%.*]] = icmp uge i16 [[TMP2]], [[B:%.*]]
  %c = and i32 %a, 1000
  %d = zext i16 %b to i32
  %e = icmp uge i32 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_u64_to_u32(i64 %a, i32 %b) {
; CHECK-LABEL: @test_u64_to_u32(
; CHECK-NEXT:    [[TMP1:%.*]] = trunc i64 [[A:%.*]] to i32
; CHECK-NEXT:    [[TMP2:%.*]] = and i32 [[TMP1]], -2
; CHECK-NEXT:    [[E:%.*]] = icmp uge i32 [[TMP2]], [[B:%.*]]
  %c = and i64 %a, 4294967294
  %d = zext i32 %b to i64
  %e = icmp uge i64 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_s64_to_u32(i64 %a, i32 %b) {
; CHECK-LABEL: @test_s64_to_u32(
; CHECK-NEXT:    [[TMP1:%.*]] = trunc i64 [[A:%.*]] to i32
; CHECK-NEXT:    [[TMP2:%.*]] = and i32 [[TMP1]], -2
; CHECK-NEXT:    [[E:%.*]] = icmp uge i32 [[TMP2]], [[B:%.*]]
  %c = and i64 %a, 4294967294
  %d = zext i32 %b to i64
  %e = icmp sge i64 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_s32_to_u8(i32 %a, i8 %b) {
; CHECK-LABEL: @test_s32_to_u8(
; CHECK-NEXT:    [[TMP1:%.*]] = trunc i32 [[A:%.*]] to i8
; CHECK-NEXT:    [[TMP2:%.*]] = and i8 [[TMP1]], -2
; CHECK-NEXT:    [[E:%.*]] = icmp uge i8 [[TMP2]], [[B:%.*]]
  %c = and i32 %a, 254
  %d = zext i8 %b to i32
  %e = icmp sge i32 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_s32_to_s8_two_sext(i8 %a, i8 %b) {
; CHECK-LABEL: @test_s32_to_s8_two_sext(
; CHECK-NEXT:    [[E:%.*]] = icmp sge i8 [[A:%.*]], [[B:%.*]]
  %a1 = sext i8 %a to i32
  %b1 = sext i8 %b to i32
  %e = icmp sge i32 %a1, %b1
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_u32_to_u8_two_zext(i8 %a, i8 %b) {
; CHECK-LABEL: @test_u32_to_u8_two_zext(
; CHECK-NEXT:    [[E:%.*]] = icmp uge i8 [[A:%.*]], [[B:%.*]]
  %a1 = zext i8 %a to i32
  %b1 = zext i8 %b to i32
  %e = icmp uge i32 %a1, %b1
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

