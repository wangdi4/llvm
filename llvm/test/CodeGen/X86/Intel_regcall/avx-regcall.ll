; RUN: llc < %s -mtriple=i686-apple-darwin -mcpu=corei7-avx -mattr=+avx | FileCheck -check-prefix=X32 %s
; RUN: llc < %s -mtriple=i386-pc-win32 -mcpu=corei7-avx -mattr=+avx | FileCheck -check-prefix=X32 %s
; RUN: llc < %s -mtriple=x86_64-win32 -mcpu=corei7-avx -mattr=+avx | FileCheck -check-prefix=WIN64 %s
; RUN: llc < %s -mtriple=x86_64-apple-darwin -mcpu=corei7-avx -mattr=+avx | FileCheck -check-prefix=LINUXOSX64 %s

declare <64 x float> @func_float32_ptr(<64 x float>, <64 x float> *)
declare i32 @func_i32(i32, i32, i32, i32)
declare <4 x float> @func_floatx4(<4 x float>)

; WIN64: testf32_inp
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm([0-7])}}
; WIN64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm([0-7])}}
; WIN64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm([0-7])}}
; WIN64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm([0-7])}}
; WIN64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm([0-7])}}
; WIN64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm([0-7])}}
; WIN64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm([0-7])}}
; WIN64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm([0-7])}}
; WIN64: leaq	{{.*}}(%rsp), %rax
; WIN64: call
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; WIN64: ret

; X32: testf32_inp
; X32: vmovaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  {{#+}} 16-byte Spill
; X32: vmovaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  {{#+}} 16-byte Spill
; X32: vmovaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  {{#+}} 16-byte Spill
; X32: vmovaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  {{#+}} 16-byte Spill
; X32: addps  {{.*}}, {{%ymm[0-7]}}
; X32: addps  {{.*}}, {{%ymm[0-7]}}
; X32: addps  {{.*}}, {{%ymm[0-7]}}
; X32: addps  {{.*}}, {{%ymm[0-7]}}
; X32: addps  {{.*}}, {{%ymm[0-7]}}
; X32: addps  {{.*}}, {{%ymm[0-7]}}
; X32: addps  {{.*}}, {{%ymm[0-7]}}
; X32: addps  {{.*}}, {{%ymm[0-7]}}
; X32: leal	{{.*}}(%esp), %eax
; X32: call
; X32: vmovaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  {{#+}} 16-byte Reload
; X32: vmovaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  {{#+}} 16-byte Reload
; X32: vmovaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  {{#+}} 16-byte Reload
; X32: vmovaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  {{#+}} 16-byte Reload
; X32: ret

; LINUXOSX64: testf32_inp
; LINUXOSX64: vmovaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: vmovaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: vmovaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: vmovaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: vmovaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: vmovaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: vmovaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: vmovaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; LINUXOSX64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm[0-7]}}
; LINUXOSX64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm[0-7]}}
; LINUXOSX64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm[0-7]}}
; LINUXOSX64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm[0-7]}}
; LINUXOSX64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm[0-7]}}
; LINUXOSX64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm[0-7]}}
; LINUXOSX64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm[0-7]}}
; LINUXOSX64: addps  {{%ymm([8-9]|1[0-5])}}, {{%ymm[0-7]}}
; LINUXOSX64: leaq   {{.*}}(%rsp), %rdi
; LINUXOSX64: call
; LINUXOSX64: vmovaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: vmovaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: vmovaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: vmovaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: vmovaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: vmovaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: vmovaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: vmovaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  {{#+}} 16-byte Reload
; LINUXOSX64: ret

;test calling conventions - input parameters, callee saved XMMs
define __regcall <64 x float> @testf32_inp(<64 x float> %a, <64 x float> %b) nounwind {
  %y = alloca <64 x float>, align 64
  %x = fadd <64 x float> %a, %b
  %1 = call <64 x float> @func_float32_ptr(<64 x float> %x, <64 x float>* %y) 
  %2 = load <64 x float>, <64 x float>* %y, align 32
  %3 = fadd <64 x float> %2, %1
  ret <64 x float> %3
}

; X32: pushl	%edi
; X32: pushl	%esi
; X32: subl	%ecx, %eax
; X32: subl	%edi, %edx
; X32: movl	{{.*(%esp).*}}, %edi
; X32: subl	%esi, %edx
; X32: movl	{{.*(%esp).*}}, %esi
; X32: subl	{{.*(%esp).*}}, %esi
; X32: subl	{{.*(%esp).*}}, %edi
; X32: subl	{{.*(%esp).*}}, %edi
; X32: movl	%edx, %ecx
; X32: movl	%esi, %edx
; X32: calll	{{.*func_i32.*}}
; X32: popl	%esi
; X32: popl	%edi
; X32: retl

; WIN64: pushq	%r12
; WIN64: pushq	%r11
; WIN64: pushq	%r10
; WIN64: subl	%ecx, %eax
; WIN64: subl	%edi, %edx
; WIN64: subl	%esi, %edx
; WIN64: subl	%r9d, %r8d
; WIN64: subl	%r11d, %r10d
; WIN64: subl	%r12d, %r10d
; WIN64: movl	%edx, %ecx
; WIN64: movl	%r8d, %edx
; WIN64: movl	%r10d, %edi
; WIN64: callq	{{.*func_i32.*}}
; WIN64: popq	%r10
; WIN64: popq	%r11
; WIN64: popq	%r12
; WIN64: retq

; LINUXOSX64: pushq	%r14
; LINUXOSX64: pushq	%r13
; LINUXOSX64: pushq	%r12
; LINUXOSX64: subl	%ecx, %eax
; LINUXOSX64: subl	%edi, %edx
; LINUXOSX64: subl	%esi, %edx
; LINUXOSX64: subl	%r9d, %r8d
; LINUXOSX64: subl	%r13d, %r12d
; LINUXOSX64: subl	%r14d, %r12d
; LINUXOSX64: movl	%edx, %ecx
; LINUXOSX64: movl	%r8d, %edx
; LINUXOSX64: movl	%r12d, %edi
; LINUXOSX64: callq	{{.*func_i32.*}}
; LINUXOSX64: popq	%r12
; LINUXOSX64: popq	%r13
; LINUXOSX64: popq	%r14
; LINUXOSX64: retq

;test calling conventions - input parameters, callee saved GPRs
define __regcall i32 @testi32_inp(i32 %a1, i32 %a2, i32 %a3, i32 %a4, i32 %a5,
                                  i32 %b1, i32 %b2, i32 %b3, i32 %b4, i32 %b5) nounwind {
  %x1 = sub i32 %a1, %a2
  %y1 = sub i32 %a3, %a4
  %z1 = sub i32 %y1, %a5
  %x2 = sub i32 %b1, %b2
  %y2 = sub i32 %b3, %b4
  %z2 = sub i32 %y2, %b5
  %r = call __regcall i32 @func_i32(i32 %x1, i32 %z1, i32 %x2, i32 %z2)
  ret i32 %r
}
