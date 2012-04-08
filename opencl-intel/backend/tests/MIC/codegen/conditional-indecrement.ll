; XFAIL: *
; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

define i32 @test1(i32 %a, i32 %b) nounwind readnone {
  %not.cmp = icmp ne i32 %a, 0
  %inc = zext i1 %not.cmp to i32
  %retval.0 = add i32 %inc, %b
  ret i32 %retval.0
; KNF: test1:
; KNF: cmpl $1
; KNF: sbbl $-1
; KNF: ret
}

define i32 @test2(i32 %a, i32 %b) nounwind readnone {
  %cmp = icmp eq i32 %a, 0
  %inc = zext i1 %cmp to i32
  %retval.0 = add i32 %inc, %b
  ret i32 %retval.0
; KNF: test2:
; KNF: cmpl $1
; KNF: adcl $0
; KNF: ret
}

define i32 @test3(i32 %a, i32 %b) nounwind readnone {
  %cmp = icmp eq i32 %a, 0
  %inc = zext i1 %cmp to i32
  %retval.0 = add i32 %inc, %b
  ret i32 %retval.0
; KNF: test3:
; KNF: cmpl $1
; KNF: adcl $0
; KNF: ret
}

define i32 @test4(i32 %a, i32 %b) nounwind readnone {
  %not.cmp = icmp ne i32 %a, 0
  %inc = zext i1 %not.cmp to i32
  %retval.0 = add i32 %inc, %b
  ret i32 %retval.0
; KNF: test4:
; KNF: cmpl $1
; KNF: sbbl $-1
; KNF: ret
}

define i32 @test5(i32 %a, i32 %b) nounwind readnone {
  %not.cmp = icmp ne i32 %a, 0
  %inc = zext i1 %not.cmp to i32
  %retval.0 = sub i32 %b, %inc
  ret i32 %retval.0
; KNF: test5:
; KNF: cmpl $1
; KNF: adcl $-1
; KNF: ret
}

define i32 @test6(i32 %a, i32 %b) nounwind readnone {
  %cmp = icmp eq i32 %a, 0
  %inc = zext i1 %cmp to i32
  %retval.0 = sub i32 %b, %inc
  ret i32 %retval.0
; KNF: test6:
; KNF: cmpl $1
; KNF: sbbl $0
; KNF: ret
}

define i32 @test7(i32 %a, i32 %b) nounwind readnone {
  %cmp = icmp eq i32 %a, 0
  %inc = zext i1 %cmp to i32
  %retval.0 = sub i32 %b, %inc
  ret i32 %retval.0
; KNF: test7:
; KNF: cmpl $1
; KNF: sbbl $0
; KNF: ret
}

define i32 @test8(i32 %a, i32 %b) nounwind readnone {
  %not.cmp = icmp ne i32 %a, 0
  %inc = zext i1 %not.cmp to i32
  %retval.0 = sub i32 %b, %inc
  ret i32 %retval.0
; KNF: test8:
; KNF: cmpl $1
; KNF: adcl $-1
; KNF: ret
}
