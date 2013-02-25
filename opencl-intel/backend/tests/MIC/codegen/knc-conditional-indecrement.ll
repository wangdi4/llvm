; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

define i32 @test1(i32 %a, i32 %b) nounwind readnone {
  %not.cmp = icmp ne i32 %a, 0
  %inc = zext i1 %not.cmp to i32
  %retval.0 = add i32 %inc, %b
  ret i32 %retval.0
; KNF: test1:
; KNF: cmpl $1, %edi
; KNF: sbbl $0, %esi
; KNF: incl %esi
}

define i32 @test3(i32 %a, i32 %b) nounwind readnone {
  %cmp = icmp eq i32 %a, 0
  %inc = zext i1 %cmp to i32
  %retval.0 = add i32 %inc, %b
  ret i32 %retval.0
; KNF: test3:
; KNF: cmpl      $1, %edi
; KNF: adcl      $0, %esi
}

define i32 @test5(i32 %a, i32 %b) nounwind readnone {
  %not.cmp = icmp ne i32 %a, 0
  %inc = zext i1 %not.cmp to i32
  %retval.0 = sub i32 %b, %inc
  ret i32 %retval.0
; KNF: test5:
; KNF: cmpl      $1, %edi
; KNF: adcl      $0, %esi
; KNF: decl      %esi
}

define i32 @test6(i32 %a, i32 %b) nounwind readnone {
  %cmp = icmp eq i32 %a, 0
  %inc = zext i1 %cmp to i32
  %retval.0 = sub i32 %b, %inc
  ret i32 %retval.0
; KNF: test6:
; KNF: cmpl      $1, %edi
; KNF: sbbl      $0, %esi
}
