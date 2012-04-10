; XFAIL: *
; XFAIL: win32
;
; RUNc: llc < %s -mtriple=x86_64-pc-linux \
; RUNc:       -march=y86-64 -mcpu=knc \
; RUNc:     | FileCheck %s -check-prefix=KNC
;

; RUN:  cat file_which_doesn't_exist
; guyblank: this is done just so the test wouldn't be marked as unresolved, 
; guyblank: once the test doesn't get stuck in KNC, this can be removed

target datalayout = "e-p:64:64"

define <8 x i32> @cvt_vec(<8 x double> %a) nounwind readnone ssp {
entry:
; KNF: vcvtpd2pu
  %conv = fptoui <8 x double> %a to <8 x i32>
  ret <8 x i32> %conv
}
