; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define i8 @test_i8_eq(i8 %a, i8 %b) nounwind readnone {
; KNF: cmpb
; KNF: je
  %1 = icmp eq i8 %a, %b
  %2 = select i1 %1, i8 %a, i8 %b
  ret i8 %2
}

define i32 @test_i32_eq(i32 %a, i32 %b) nounwind readnone {
; KNF: cmpl
; KNF: je
  %1 = icmp eq i32 %a, %b
  %2 = select i1 %1, i32 %a, i32 %b
  ret i32 %2
}

define i64 @test_i64_eq(i64 %a, i64 %b) nounwind readnone {
; KNF: cmpq
; KNF: je
  %1 = icmp eq i64 %a, %b
  %2 = select i1 %1, i64 %a, i64 %b
  ret i64 %2
}

define i8 @test_i8_ne(i8 %a, i8 %b) nounwind readnone {
; KNF: cmpb
; KNF: jne
  %1 = icmp ne i8 %a, %b
  %2 = select i1 %1, i8 %a, i8 %b
  ret i8 %2
}

define i32 @test_i32_ne(i32 %a, i32 %b) nounwind readnone {
; KNF: cmpl
; KNF: jne
  %1 = icmp ne i32 %a, %b
  %2 = select i1 %1, i32 %a, i32 %b
  ret i32 %2
}

define i64 @test_i64_ne(i64 %a, i64 %b) nounwind readnone {
; KNF: cmpq
; KNF: jne
  %1 = icmp ne i64 %a, %b
  %2 = select i1 %1, i64 %a, i64 %b
  ret i64 %2
}

define i8 @test_i8_ugt(i8 %a, i8 %b) nounwind readnone {
; KNF: cmpb
; KNF: ja
  %1 = icmp ugt i8 %a, %b
  %2 = select i1 %1, i8 %a, i8 %b
  ret i8 %2
}

define i32 @test_i32_ugt(i32 %a, i32 %b) nounwind readnone {
; KNF: cmpl
; KNF: ja
  %1 = icmp ugt i32 %a, %b
  %2 = select i1 %1, i32 %a, i32 %b
  ret i32 %2
}

define i64 @test_i64_ugt(i64 %a, i64 %b) nounwind readnone {
; KNF: cmpq
; KNF: ja
  %1 = icmp ugt i64 %a, %b
  %2 = select i1 %1, i64 %a, i64 %b
  ret i64 %2
}

define i8 @test_i8_uge(i8 %a, i8 %b) nounwind readnone {
; KNF: cmpb
; KNF: jae
  %1 = icmp uge i8 %a, %b
  %2 = select i1 %1, i8 %a, i8 %b
  ret i8 %2
}

define i32 @test_i32_uge(i32 %a, i32 %b) nounwind readnone {
; KNF: cmpl
; KNF: jae
  %1 = icmp uge i32 %a, %b
  %2 = select i1 %1, i32 %a, i32 %b
  ret i32 %2
}

define i64 @test_i64_uge(i64 %a, i64 %b) nounwind readnone {
; KNF: cmpq
; KNF: jae
  %1 = icmp uge i64 %a, %b
  %2 = select i1 %1, i64 %a, i64 %b
  ret i64 %2
}

define i8 @test_i8_ult(i8 %a, i8 %b) nounwind readnone {
; KNF: cmpb
; KNF: jb
  %1 = icmp ult i8 %a, %b
  %2 = select i1 %1, i8 %a, i8 %b
  ret i8 %2
}

define i32 @test_i32_ult(i32 %a, i32 %b) nounwind readnone {
; KNF: cmpl
; KNF: jb
  %1 = icmp ult i32 %a, %b
  %2 = select i1 %1, i32 %a, i32 %b
  ret i32 %2
}

define i64 @test_i64_ult(i64 %a, i64 %b) nounwind readnone {
; KNF: cmpq
; KNF: jb
  %1 = icmp ult i64 %a, %b
  %2 = select i1 %1, i64 %a, i64 %b
  ret i64 %2
}

define i8 @test_i8_ule(i8 %a, i8 %b) nounwind readnone {
; KNF: cmpb
; KNF: jbe
  %1 = icmp ule i8 %a, %b
  %2 = select i1 %1, i8 %a, i8 %b
  ret i8 %2
}

define i32 @test_i32_ule(i32 %a, i32 %b) nounwind readnone {
; KNF: cmpl
; KNF: jbe
  %1 = icmp ule i32 %a, %b
  %2 = select i1 %1, i32 %a, i32 %b
  ret i32 %2
}

define i64 @test_i64_ule(i64 %a, i64 %b) nounwind readnone {
; KNF: cmpq
; KNF: jbe
  %1 = icmp ule i64 %a, %b
  %2 = select i1 %1, i64 %a, i64 %b
  ret i64 %2
}

define i8 @test_i8_sgt(i8 %a, i8 %b) nounwind readnone {
; KNF: cmpb
; KNF: jg
  %1 = icmp sgt i8 %a, %b
  %2 = select i1 %1, i8 %a, i8 %b
  ret i8 %2
}

define i32 @test_i32_sgt(i32 %a, i32 %b) nounwind readnone {
; KNF: cmpl
; KNF: jg
  %1 = icmp sgt i32 %a, %b
  %2 = select i1 %1, i32 %a, i32 %b
  ret i32 %2
}

define i64 @test_i64_sgt(i64 %a, i64 %b) nounwind readnone {
; KNF: cmpq
; KNF: jg
  %1 = icmp sgt i64 %a, %b
  %2 = select i1 %1, i64 %a, i64 %b
  ret i64 %2
}

define i8 @test_i8_sge(i8 %a, i8 %b) nounwind readnone {
; KNF: cmpb
; KNF: jge
  %1 = icmp sge i8 %a, %b
  %2 = select i1 %1, i8 %a, i8 %b
  ret i8 %2
}

define i32 @test_i32_sge(i32 %a, i32 %b) nounwind readnone {
; KNF: cmpl
; KNF: jge
  %1 = icmp sge i32 %a, %b
  %2 = select i1 %1, i32 %a, i32 %b
  ret i32 %2
}

define i64 @test_i64_sge(i64 %a, i64 %b) nounwind readnone {
; KNF: cmpq
; KNF: jge
  %1 = icmp sge i64 %a, %b
  %2 = select i1 %1, i64 %a, i64 %b
  ret i64 %2
}

define i8 @test_i8_slt(i8 %a, i8 %b) nounwind readnone {
; KNF: cmpb
; KNF: jl
  %1 = icmp slt i8 %a, %b
  %2 = select i1 %1, i8 %a, i8 %b
  ret i8 %2
}

define i32 @test_i32_slt(i32 %a, i32 %b) nounwind readnone {
; KNF: cmpl
; KNF: jl
  %1 = icmp slt i32 %a, %b
  %2 = select i1 %1, i32 %a, i32 %b
  ret i32 %2
}

define i64 @test_i64_slt(i64 %a, i64 %b) nounwind readnone {
; KNF: cmpq
; KNF: jl
  %1 = icmp slt i64 %a, %b
  %2 = select i1 %1, i64 %a, i64 %b
  ret i64 %2
}

define i8 @test_i8_sle(i8 %a, i8 %b) nounwind readnone {
; KNF: cmpb
; KNF: jle
  %1 = icmp sle i8 %a, %b
  %2 = select i1 %1, i8 %a, i8 %b
  ret i8 %2
}

define i32 @test_i32_sle(i32 %a, i32 %b) nounwind readnone {
; KNF: cmpl
; KNF: jle
  %1 = icmp sle i32 %a, %b
  %2 = select i1 %1, i32 %a, i32 %b
  ret i32 %2
}

define i64 @test_i64_sle(i64 %a, i64 %b) nounwind readnone {
; KNF: cmpq
; KNF: jle
  %1 = icmp sle i64 %a, %b
  %2 = select i1 %1, i64 %a, i64 %b
  ret i64 %2
}
