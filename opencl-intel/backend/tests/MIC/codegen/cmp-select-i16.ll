; XFAIL: *
; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
;;;;; Replace this line with real checks once this tests compiles ok
; KNF: ret

target datalayout = "e-p:64:64"

define i16 @test_i16_ne(i16 %a, i16 %b) nounwind readnone {
  %1 = icmp ne i16 %a, %b
  %2 = select i1 %1, i16 %a, i16 %b
  ret i16 %2
}

define i16 @test_i16_ugt(i16 %a, i16 %b) nounwind readnone {
  %1 = icmp ugt i16 %a, %b
  %2 = select i1 %1, i16 %a, i16 %b
  ret i16 %2
}

define i16 @test_i16_uge(i16 %a, i16 %b) nounwind readnone {
  %1 = icmp uge i16 %a, %b
  %2 = select i1 %1, i16 %a, i16 %b
  ret i16 %2
}

define i16 @test_i16_ult(i16 %a, i16 %b) nounwind readnone {
  %1 = icmp ult i16 %a, %b
  %2 = select i1 %1, i16 %a, i16 %b
  ret i16 %2
}

define i16 @test_i16_ule(i16 %a, i16 %b) nounwind readnone {
  %1 = icmp ule i16 %a, %b
  %2 = select i1 %1, i16 %a, i16 %b
  ret i16 %2
}

define i16 @test_i16_sgt(i16 %a, i16 %b) nounwind readnone {
  %1 = icmp sgt i16 %a, %b
  %2 = select i1 %1, i16 %a, i16 %b
  ret i16 %2
}

define i16 @test_i16_sge(i16 %a, i16 %b) nounwind readnone {
  %1 = icmp sge i16 %a, %b
  %2 = select i1 %1, i16 %a, i16 %b
  ret i16 %2
}

define i16 @test_i16_slt(i16 %a, i16 %b) nounwind readnone {
  %1 = icmp slt i16 %a, %b
  %2 = select i1 %1, i16 %a, i16 %b
  ret i16 %2
}

define i16 @test_i16_sle(i16 %a, i16 %b) nounwind readnone {
  %1 = icmp sle i16 %a, %b
  %2 = select i1 %1, i16 %a, i16 %b
  ret i16 %2
}

define i16 @test_i16_eq(i16 %a, i16 %b) nounwind readnone {
  %1 = icmp eq i16 %a, %b
  %2 = select i1 %1, i16 %a, i16 %b
  ret i16 %2
}
