; RUN: llc < %s -mtriple=i686-apple-darwin -mcpu=corei7-avx -mattr=+avx | FileCheck -check-prefix=X32 %s
; RUN: llc < %s -mtriple=i386-pc-win32 -mcpu=corei7-avx -mattr=+avx | FileCheck -check-prefix=X32 %s
; RUN: llc < %s -mtriple=x86_64-win32 -mcpu=corei7-avx -mattr=+avx | FileCheck -check-prefix=WIN64 %s
; RUN: llc < %s -mtriple=x86_64-apple-darwin -mcpu=corei7-avx -mattr=+avx | FileCheck -check-prefix=LINUXOSX64 %s

; WIN64: testf32_inp
; WIN64: movaps {{%xmm(1[2-5])}}, {{.*(%rsp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm(1[2-5])}}, {{.*(%rsp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm(1[2-5])}}, {{.*(%rsp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm(1[2-5])}}, {{.*(%rsp).*}}  {{#+}} 16-byte Spill
; WIN64: {{.*}} {{%ymm([0-9]|1[0-1])}}, {{%ymm([0-9]|1[0-1])}}, {{%ymm(1[2-5])}}
; WIN64: {{.*}} {{%ymm([0-9]|1[0-1])}}, {{%ymm([0-9]|1[0-1])}}, {{%ymm(1[2-5])}}
; WIN64: {{.*}} {{%ymm([0-9]|1[0-1])}}, {{%ymm([0-9]|1[0-1])}}, {{%ymm(1[2-5])}}
; WIN64: {{.*}} {{%ymm([0-9]|1[0-1])}}, {{%ymm([0-9]|1[0-1])}}, {{%ymm(1[2-5])}}
; WIN64: movaps {{.*(%rsp).*}}, {{%xmm(1[2-5])}}  {{#+}} 16-byte Reload
; WIN64: movaps {{.*(%rsp).*}}, {{%xmm(1[2-5])}}  {{#+}} 16-byte Reload
; WIN64: movaps {{.*(%rsp).*}}, {{%xmm(1[2-5])}}  {{#+}} 16-byte Reload
; WIN64: movaps {{.*(%rsp).*}}, {{%xmm(1[2-5])}}  {{#+}} 16-byte Reload
; WIN64: retq

; X32: testf32_inp
; X32: movaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  {{#+}} 16-byte Spill
; X32: movaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  {{#+}} 16-byte Spill
; X32: movaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  {{#+}} 16-byte Spill
; X32: movaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  {{#+}} 16-byte Spill
; X32: {{.*}} {{%ymm[0-7]}}, {{%ymm[0-7]}}, {{%ymm[4-7]}}
; X32: {{.*}} {{%ymm[0-7]}}, {{%ymm[0-7]}}, {{%ymm[4-7]}}
; X32: {{.*}} {{%ymm[0-7]}}, {{%ymm[0-7]}}, {{%ymm[4-7]}}
; X32: {{.*}} {{%ymm[0-7]}}, {{%ymm[0-7]}}, {{%ymm[4-7]}}
; X32: vmovaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  {{#+}} 16-byte Reload
; X32: vmovaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  {{#+}} 16-byte Reload
; X32: vmovaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  {{#+}} 16-byte Reload
; X32: vmovaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  {{#+}} 16-byte Reload
; X32: retl

; LINUXOSX64: testf32_inp
; LINUXOSX64: movaps {{%xmm(1[2-5])}}, {{.*(%rsp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: movaps {{%xmm(1[2-5])}}, {{.*(%rsp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: movaps {{%xmm(1[2-5])}}, {{.*(%rsp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: movaps {{%xmm(1[2-5])}}, {{.*(%rsp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: {{.*}} {{%ymm([0-9]|1[0-1])}}, {{%ymm([0-9]|1[0-1])}}, {{%ymm(1[2-5])}}
; LINUXOSX64: {{.*}} {{%ymm([0-9]|1[0-1])}}, {{%ymm([0-9]|1[0-1])}}, {{%ymm(1[2-5])}}
; LINUXOSX64: {{.*}} {{%ymm([0-9]|1[0-1])}}, {{%ymm([0-9]|1[0-1])}}, {{%ymm(1[2-5])}}
; LINUXOSX64: {{.*}} {{%ymm([0-9]|1[0-1])}}, {{%ymm([0-9]|1[0-1])}}, {{%ymm(1[2-5])}}
; LINUXOSX64: movaps {{.*(%rsp).*}}, {{%xmm(1[2-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: movaps {{.*(%rsp).*}}, {{%xmm(1[2-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: movaps {{.*(%rsp).*}}, {{%xmm(1[2-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: movaps {{.*(%rsp).*}}, {{%xmm(1[2-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: retq

;test calling conventions - input parameters, callee saved XMMs
define x86_regcallcc <32 x float> @testf32_inp(<32 x float> %a, <32 x float> %b, <32 x float> %c) nounwind {
  %x1 = fadd <32 x float> %a, %b
  %x2 = fmul <32 x float> %a, %b
  %x3 = fsub <32 x float> %x1, %x2
  %x4 = fadd <32 x float> %x3, %c
  ret <32 x float> %x4
}

; X32: pushl {{%e(si|di|bx|bp)}}
; X32: pushl {{%e(si|di|bx|bp)}}
; X32: pushl {{%e(si|di|bx|bp)}}
; X32: pushl {{%e(si|di|bx|bp)}}
; X32: popl {{%e(si|di|bx|bp)}}
; X32: popl {{%e(si|di|bx|bp)}}
; X32: popl {{%e(si|di|bx|bp)}}
; X32: popl {{%e(si|di|bx|bp)}}
; X32: retl

; WIN64: pushq	{{%r(bp|bx|1[0-5])}}
; WIN64: pushq	{{%r(bp|bx|1[0-5])}}
; WIN64: pushq	{{%r(bp|bx|1[0-5])}}
; WIN64: pushq	{{%r(bp|bx|1[0-5])}}
; WIN64: pushq	{{%r(bp|bx|1[0-5])}}
; WIN64: popq	{{%r(bp|bx|1[0-5])}}
; WIN64: popq	{{%r(bp|bx|1[0-5])}}
; WIN64: popq	{{%r(bp|bx|1[0-5])}}
; WIN64: popq	{{%r(bp|bx|1[0-5])}}
; WIN64: popq	{{%r(bp|bx|1[0-5])}}
; WIN64: retq

; LINUXOSX64: pushq	{{%r(bp|bx|1[2-5])}}
; LINUXOSX64: pushq	{{%r(bp|bx|1[2-5])}}
; LINUXOSX64: pushq	{{%r(bp|bx|1[2-5])}}
; LINUXOSX64: pushq	{{%r(bp|bx|1[2-5])}}
; LINUXOSX64: popq	{{%r(bp|bx|1[2-5])}}
; LINUXOSX64: popq	{{%r(bp|bx|1[2-5])}}
; LINUXOSX64: popq	{{%r(bp|bx|1[2-5])}}
; LINUXOSX64: popq	{{%r(bp|bx|1[2-5])}}
; LINUXOSX64: retq

;test calling conventions - input parameters, callee saved GPRs
define x86_regcallcc i32 @testi32_inp(i32 %a1, i32 %a2, i32 %a3, i32 %a4, i32 %a5, i32 %a6,
                                      i32 %b1, i32 %b2, i32 %b3, i32 %b4, i32 %b5, i32 %b6) nounwind {
  %x1 = sub i32 %a1, %a2
  %x2 = sub i32 %a3, %a4
  %x3 = sub i32 %a5, %a6
  %y1 = sub i32 %b1, %b2
  %y2 = sub i32 %b3, %b4
  %y3 = sub i32 %b5, %b6
  %v1 = add i32 %a1, %a2
  %v2 = add i32 %a3, %a4
  %v3 = add i32 %a5, %a6
  %w1 = add i32 %b1, %b2
  %w2 = add i32 %b3, %b4
  %w3 = add i32 %b5, %b6
  %s1 = mul i32 %x1, %y1
  %s2 = mul i32 %x2, %y2
  %s3 = mul i32 %x3, %y3
  %t1 = mul i32 %v1, %w1
  %t2 = mul i32 %v2, %w2
  %t3 = mul i32 %v3, %w3
  %m1 = add i32 %s1, %s2
  %m2 = add i32 %m1, %s3
  %n1 = add i32 %t1, %t2
  %n2 = add i32 %n1, %t3
  %r1 = add i32 %m2, %n2
  ret i32 %r1
}
