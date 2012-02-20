; XFAIL: win32
;
; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:       -march=y86-64 -mcpu=knf \
; RUN:     | FileCheck %s -check-prefix=KNF
;

target datalayout = "e-p:64:64"

define i1 @extelm (<16 x i1> %val) nounwind {
entry:

; KNF: vkmov   {{%k[0-9]+}}, {{%[a-z0-9]+}}
; KNF: shrl    $13, {{%[a-z0-9]+}}
; KNF: andl    $1, {{%[a-z0-9]+}}
;
  %extract40 = extractelement <16 x i1> %val, i32 13

  ret i1 %extract40
}

