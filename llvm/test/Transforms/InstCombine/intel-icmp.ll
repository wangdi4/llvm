; This test checks that comparison sizes are optimized and 
; unnecessary sext/zext or other instructions are eliminated
;
; RUN: opt < %s -instcombine -S -mtriple=x86_64-unknown-linux-gnu | FileCheck %s

define i32 @test_s32_to_s8(i32 %a, i8 %b) {           
; CHECK-LABEL: @test_s32_to_s8(
; CHECK-NEXT: %a.tr = trunc i32 %a to i8
; CHECK-NEXT: %1 = and i8 %a.tr, 127
; CHECK-NEXT: %e = icmp sge i8 %1, %b
  %c = and i32 %a, 127
  %d = sext i8 %b to i32
  %e = icmp sge i32 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_s32_to_s16(i32 %a, i16 %b) {           
; CHECK-LABEL: @test_s32_to_s16(
; CHECK-NEXT: %a.tr = trunc i32 %a to i16
; CHECK-NEXT: %1 = and i16 %a.tr, 1000
; CHECK-NEXT: %e = icmp sge i16 %1, %b
  %c = and i32 %a, 1000
  %d = sext i16 %b to i32
  %e = icmp sge i32 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}          

define i32 @test_s64_to_s32(i64 %a, i32 %b) {           
; CHECK-LABEL: @test_s64_to_s32(
; CHECK-NEXT: %a.tr = trunc i64 %a to i32
; CHECK-NEXT: %1 = and i32 %a.tr, 2147483647
; CHECK-NEXT: %e = icmp sge i32 %1, %b
  %c = and i64 %a, 2147483647
  %d = sext i32 %b to i64
  %e = icmp sge i64 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
} 

define i32 @test_u32_to_u8(i32 %a, i8 %b) {           
; CHECK-LABEL: @test_u32_to_u8(
; CHECK-NEXT: %a.tr = trunc i32 %a to i8
; CHECK-NEXT: %1 = and i8 %a.tr, -2
; CHECK-NEXT: %e = icmp uge i8 %1, %b
  %c = and i32 %a, 254
  %d = zext i8 %b to i32
  %e = icmp uge i32 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}                     

define i32 @test_u32_to_u16(i32 %a, i16 %b) {           
; CHECK-LABEL: @test_u32_to_u16(
; CHECK-NEXT: %a.tr = trunc i32 %a to i16
; CHECK-NEXT: %1 = and i16 %a.tr, 1000
; CHECK-NEXT: %e = icmp uge i16 %1, %b
  %c = and i32 %a, 1000
  %d = zext i16 %b to i32
  %e = icmp uge i32 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}          

define i32 @test_u64_to_u32(i64 %a, i32 %b) {           
; CHECK-LABEL: @test_u64_to_u32(
; CHECK-NEXT: %a.tr = trunc i64 %a to i32
; CHECK-NEXT: %1 = and i32 %a.tr, -2
; CHECK-NEXT: %e = icmp uge i32 %1, %b
  %c = and i64 %a, 4294967294
  %d = zext i32 %b to i64
  %e = icmp uge i64 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_s64_to_u32(i64 %a, i32 %b) {           
; CHECK-LABEL: @test_s64_to_u32(
; CHECK-NEXT: %a.tr = trunc i64 %a to i32
; CHECK-NEXT: %1 = and i32 %a.tr, -2
; CHECK-NEXT: %e = icmp uge i32 %1, %b
  %c = and i64 %a, 4294967294
  %d = zext i32 %b to i64
  %e = icmp sge i64 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_s32_to_u8(i32 %a, i8 %b) {           
; CHECK-LABEL: @test_s32_to_u8(
; CHECK-NEXT: %a.tr = trunc i32 %a to i8
; CHECK-NEXT: %1 = and i8 %a.tr, -2
; CHECK-NEXT: %e = icmp uge i8 %1, %b
  %c = and i32 %a, 254
  %d = zext i8 %b to i32
  %e = icmp sge i32 %c, %d
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_s32_to_s8_two_sext(i8 %a, i8 %b) {           
; CHECK-LABEL: @test_s32_to_s8_two_sext(
; CHECK-NEXT: %e = icmp sge i8 %a, %b
  %a1 = sext i8 %a to i32
  %b1 = sext i8 %b to i32
  %e = icmp sge i32 %a1, %b1
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

define i32 @test_u32_to_u8_two_zext(i8 %a, i8 %b) {           
; CHECK-LABEL: @test_u32_to_u8_two_zext(
; CHECK-NEXT: %e = icmp uge i8 %a, %b
  %a1 = zext i8 %a to i32
  %b1 = zext i8 %b to i32
  %e = icmp uge i32 %a1, %b1
  %e1 = sext i1 %e to i32
  ret i32 %e1
}

