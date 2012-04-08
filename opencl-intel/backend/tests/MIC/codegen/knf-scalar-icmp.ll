; XFAIL: *
; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

target datalayout = "e-p:64:64"


declare i32 @foo()

define void @test_int8_t_gte(i8 signext %a, i8 signext %b) nounwind {
; KNF: test_int8_t_gte
; KNF: jge
  %1 = icmp sge i8 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int8_t_eq(i8 signext %a, i8 signext %b) nounwind {
; KNF: test_int8_t_eq
; KNF: jne
  %1 = icmp eq i8 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int8_t_lte(i8 signext %a, i8 signext %b) nounwind {
; KNF: test_int8_t_lte
; KNF: jle
  %1 = icmp sle i8 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int8_t_neq(i8 signext %a, i8 signext %b) nounwind {
; KNF: test_int8_t_neq
; KNF: jne
  %1 = icmp ne i8 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int8_t_lt(i8 signext %a, i8 signext %b) nounwind {
; KNF: test_int8_t_lt
; KNF: jge
  %1 = icmp slt i8 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int8_t_gt(i8 signext %a, i8 signext %b) nounwind {
; KNF: test_int8_t_gt
; KNF: jle
  %1 = icmp sgt i8 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int16_t_gte(i16 signext %a, i16 signext %b) nounwind {
; KNF: test_int16_t_gte
; KNF: jge
  %1 = icmp sge i16 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int16_t_eq(i16 signext %a, i16 signext %b) nounwind {
; KNF: test_int16_t_eq
; KNF: jne
  %1 = icmp eq i16 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int16_t_lte(i16 signext %a, i16 signext %b) nounwind {
; KNF: test_int16_t_lte
; KNF: jle
  %1 = icmp sle i16 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int16_t_neq(i16 signext %a, i16 signext %b) nounwind {
; KNF: test_int16_t_neq
; KNF: jne
  %1 = icmp ne i16 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int16_t_lt(i16 signext %a, i16 signext %b) nounwind {
; KNF: test_int16_t_lt
; KNF: jge
  %1 = icmp slt i16 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int16_t_gt(i16 signext %a, i16 signext %b) nounwind {
; KNF: test_int16_t_gt 
; KNF: jle
  %1 = icmp sgt i16 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int32_t_gte(i32 %a, i32 %b) nounwind {
; KNF: test_int32_t_gte
; KNF: jge
  %1 = icmp sge i32 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int32_t_eq(i32 %a, i32 %b) nounwind {
; KNF: test_int32_t_eq
; KNF: jne
  %1 = icmp eq i32 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int32_t_lte(i32 %a, i32 %b) nounwind {
; KNF: test_int32_t_lte
; KNF: jle
  %1 = icmp sle i32 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int32_t_neq(i32 %a, i32 %b) nounwind {
; KNF: test_int32_t_neq
; KNF: jne
  %1 = icmp ne i32 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int32_t_lt(i32 %a, i32 %b) nounwind {
; KNF: test_int32_t_lt
; KNF: jge
  %1 = icmp slt i32 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int32_t_gt(i32 %a, i32 %b) nounwind {
; KNF: test_int32_t_gt
; KNF: jle
  %1 = icmp sgt i32 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int64_t_gte(i64 %a, i64 %b) nounwind {
; KNF: test_int64_t_gte
; KNF: jge
  %1 = icmp sge i64 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int64_t_eq(i64 %a, i64 %b) nounwind {
; KNF: test_int64_t_eq
; KNF: jne
  %1 = icmp eq i64 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int64_t_lte(i64 %a, i64 %b) nounwind {
; KNF: test_int64_t_lte
; KNF: jle
  %1 = icmp sle i64 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int64_t_neq(i64 %a, i64 %b) nounwind {
; KNF: test_int64_t_neq
; KNF: jne
  %1 = icmp ne i64 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int64_t_lt(i64 %a, i64 %b) nounwind {
; KNF: test_int64_t_lt
; KNF: jge
  %1 = icmp slt i64 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_int64_t_gt(i64 %a, i64 %b) nounwind {
; KNF: test_int64_t_gt
; KNF: jle
  %1 = icmp sgt i64 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint8_t_gte(i8 zeroext %a, i8 zeroext %b) nounwind {
; KNF: test_uint8_t_gte
; KNF: jae
  %1 = icmp uge i8 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint8_t_lte(i8 zeroext %a, i8 zeroext %b) nounwind {
; KNF: test_uint8_t_lte
; KNF: jbe
  %1 = icmp ule i8 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint8_t_lt(i8 zeroext %a, i8 zeroext %b) nounwind {
; KNF: test_uint8_t_lt
; KNF: jae
  %1 = icmp ult i8 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint8_t_gt(i8 zeroext %a, i8 zeroext %b) nounwind {
; KNF: test_uint8_t_gt
; KNF: jbe
  %1 = icmp ugt i8 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint16_t_gte(i16 zeroext %a, i16 zeroext %b) nounwind {
; KNF: test_uint16_t_gte
; KNF: jae
  %1 = icmp uge i16 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint16_t_lte(i16 zeroext %a, i16 zeroext %b) nounwind {
; KNF: test_uint16_t_lte
; KNF: jbe
  %1 = icmp ule i16 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint16_t_lt(i16 zeroext %a, i16 zeroext %b) nounwind {
; KNF: test_uint16_t_lt
; KNF: jae
  %1 = icmp ult i16 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint16_t_gt(i16 zeroext %a, i16 zeroext %b) nounwind {
; KNF: test_uint16_t_gt 
; KNF: jbe
  %1 = icmp ugt i16 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint32_t_gte(i32 %a, i32 %b) nounwind {
; KNF: test_uint32_t_gte
; KNF: jae
  %1 = icmp uge i32 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint32_t_lte(i32 %a, i32 %b) nounwind {
; KNF: test_uint32_t_lte
; KNF: jbe
  %1 = icmp ule i32 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint32_t_lt(i32 %a, i32 %b) nounwind {
; KNF: test_uint32_t_lt
; KNF: jae
  %1 = icmp ult i32 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint32_t_gt(i32 %a, i32 %b) nounwind {
; KNF: test_uint32_t_gt
; KNF: jbe
  %1 = icmp ugt i32 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint64_t_gte(i64 %a, i64 %b) nounwind {
; KNF: test_uint64_t_gte
; KNF: jae
  %1 = icmp uge i64 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint64_t_lte(i64 %a, i64 %b) nounwind {
; KNF: test_uint64_t_lte
; KNF: jbe
  %1 = icmp ule i64 %a, %b
  br i1 %1, label %4, label %2

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint64_t_lt(i64 %a, i64 %b) nounwind {
; KNF: test_uint64_t_lt
; KNF: jae
  %1 = icmp ult i64 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}

define void @test_uint64_t_gt(i64 %a, i64 %b) nounwind {
; KNF: test_uint64_t_gt
; KNF: jbe
  %1 = icmp ugt i64 %a, %b
  br i1 %1, label %2, label %4

; <label>:2
  %3 = call i32 @foo() nounwind
  br label %4

; <label>:4
  ret void
}
