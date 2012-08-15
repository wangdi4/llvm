; XFAIL: *
; XFAIL: win32

; RUN: llc < %s -mtriple=x86_64-pc-linux \
; RUN:        -march=y86-64 -mcpu=knc 

;

target datalayout = "e-p:64:64"

declare <16 x float> @llvm.x86.mic.cvt.roundpd.pslo(<8 x double>, i32)

define <16 x float> @f_cvt_roundpd_pslo(<8 x double> %arg0) {
; KNF: f_cvt_roundpd_pslo:
; KNF: vcvtpd2ps {rn}, %v{{[0-9]*}}, %v{{[0-9]*}}
entry:
  %ret = call <16 x float> @llvm.x86.mic.cvt.roundpd.pslo(<8 x double> %arg0, i32 0)

 ret <16 x float> %ret
}

