; XFAIL: x86_64-pc-win32, i686-pc-win32
; RUN: llc < %s -mcpu=corei7 -march=x86-64 -mattr=+sse2,-sse41 | grep {pshufd	\$3, %xmm0, %xmm0}

define float @foo(<8 x float> %a) nounwind {
  %c = extractelement <8 x float> %a, i32 3
  ret float %c
}
