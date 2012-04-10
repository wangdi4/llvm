; zrackove: I opened the following bug in llvm.org Bugs:
; http://llvm.org/bugs/show_bug.cgi?id=11334

; XFAIL: win32
; XFAIL: *
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;
; RUN:  cat file_which_doesn't_exist
; guyblank: this is done just so the test wouldn't be marked as unresolved, 
; guyblank: once the test doesn't get stuck in KNC, this can be removed

target datalayout = "e-p:64:64"

define <8 x double> @f2d_ext_vec(<8 x float> %v1) nounwind {
entry:
; KNF: vcvtps2pd       $0, %v0, %v0
  %f1 = fpext <8 x float> %v1 to <8 x double>
  ret <8 x double> %f1
}
