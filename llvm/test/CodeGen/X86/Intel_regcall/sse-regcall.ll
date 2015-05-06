; RUN: llc < %s -mtriple=i386-pc-win32 -mcpu=nehalem | FileCheck -check-prefix=WIN32 %s
; RUN: llc < %s -mtriple=x86_64-win32 -mcpu=nehalem | FileCheck -check-prefix=WIN64 %s
; RUN: llc < %s -mtriple=x86_64-apple-darwin -mcpu=nehalem | FileCheck -check-prefix=LINUXOSX %s

declare <32 x float> @func_float32_ptr(<32 x float>, <32 x float> *)
declare <32 x float> @func_float32(<32 x float>, <32 x float>)
declare i32 @func_i32(i32, i32, i32, i32)
declare <4 x float> @func_floatx4(<4 x float>)

; WIN64: testf32_inp
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  # 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  # 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  # 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  # 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  # 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  # 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  # 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  # 16-byte Spill
; WIN64: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm([0-7])}}
; WIN64: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm([0-7])}}
; WIN64: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm([0-7])}}
; WIN64: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm([0-7])}}
; WIN64: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm([0-7])}}
; WIN64: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm([0-7])}}
; WIN64: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm([0-7])}}
; WIN64: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm([0-7])}}
; WIN64: leaq	{{.*}}(%rsp), %rax
; WIN64: call
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  # 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  # 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  # 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  # 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  # 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  # 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  # 16-byte Reload
; WIN64: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  # 16-byte Reload
; WIN64: ret

; WIN32: testf32_inp
; WIN32: movaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  # 16-byte Spill
; WIN32: movaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  # 16-byte Spill
; WIN32: movaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  # 16-byte Spill
; WIN32: movaps {{%xmm([4-7])}}, {{.*(%ebp).*}}  # 16-byte Spill
; WIN32: addps  {{.*}}, {{%xmm[0-7]}}
; WIN32: addps  {{.*}}, {{%xmm[0-7]}}
; WIN32: addps  {{.*}}, {{%xmm[0-7]}}
; WIN32: addps  {{.*}}, {{%xmm[0-7]}}
; WIN32: addps  {{.*}}, {{%xmm[0-7]}}
; WIN32: addps  {{.*}}, {{%xmm[0-7]}}
; WIN32: addps  {{.*}}, {{%xmm[0-7]}}
; WIN32: addps  {{.*}}, {{%xmm[0-7]}}
; WIN32: leal	{{.*}}(%esp), %eax
; WIN32: call
; WIN32: movaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  # 16-byte Reload
; WIN32: movaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  # 16-byte Reload
; WIN32: movaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  # 16-byte Reload
; WIN32: movaps {{.*(%ebp).*}}, {{%xmm([4-7])}}  # 16-byte Reload
; WIN32: ret

; LINUXOSX: testf32_inp
; LINUXOSX: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  ## 16-byte Spill
; LINUXOSX: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  ## 16-byte Spill
; LINUXOSX: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  ## 16-byte Spill
; LINUXOSX: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  ## 16-byte Spill
; LINUXOSX: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  ## 16-byte Spill
; LINUXOSX: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  ## 16-byte Spill
; LINUXOSX: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  ## 16-byte Spill
; LINUXOSX: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  ## 16-byte Spill
; LINUXOSX: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm[0-7]}}
; LINUXOSX: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm[0-7]}}
; LINUXOSX: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm[0-7]}}
; LINUXOSX: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm[0-7]}}
; LINUXOSX: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm[0-7]}}
; LINUXOSX: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm[0-7]}}
; LINUXOSX: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm[0-7]}}
; LINUXOSX: addps  {{%xmm([8-9]|1[0-5])}}, {{%xmm[0-7]}}
; LINUXOSX: leaq   {{.*}}(%rsp), %rax
; LINUXOSX: call
; LINUXOSX: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  ## 16-byte Reload
; LINUXOSX: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  ## 16-byte Reload
; LINUXOSX: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  ## 16-byte Reload
; LINUXOSX: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  ## 16-byte Reload
; LINUXOSX: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  ## 16-byte Reload
; LINUXOSX: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  ## 16-byte Reload
; LINUXOSX: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  ## 16-byte Reload
; LINUXOSX: movaps {{.*(%rbp).*}}, {{%xmm([8-9]|1[0-5])}}  ## 16-byte Reload
; LINUXOSX: ret

;test calling conventions - input parameters, callee saved XMMs
define __regcall <32 x float> @testf32_inp(<32 x float> %a, <32 x float> %b) nounwind {
  %y = alloca <32 x float>, align 32
  %x = fadd <32 x float> %a, %b
  %1 = call __regcall <32 x float> @func_float32_ptr(<32 x float> %x, <32 x float>* %y) 
  %2 = load <32 x float>, <32 x float>* %y, align 32
  %3 = fadd <32 x float> %2, %1
  ret <32 x float> %3
}

; WIN32: pushl	%edi
; WIN32: pushl	%esi
; WIN32: subl	%ecx, %eax
; WIN32: subl	%edi, %edx
; WIN32: movl	20(%esp), %edi
; WIN32: subl	%esi, %edx
; WIN32: movl	12(%esp), %esi
; WIN32: subl	16(%esp), %esi
; WIN32: subl	24(%esp), %edi
; WIN32: subl	28(%esp), %edi
; WIN32: movl	%edx, %ecx
; WIN32: movl	%esi, %edx
; WIN32: calll	_func_i32
; WIN32: popl	%esi
; WIN32: popl	%edi
; WIN32: retl

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
; WIN64: callq	func_i32
; WIN64: popq	%r10
; WIN64: popq	%r11
; WIN64: popq	%r12
; WIN64: retq

; LINUXOSX: pushq	%r14
; LINUXOSX: pushq	%r13
; LINUXOSX: pushq	%r12
; LINUXOSX: subl	%ecx, %eax
; LINUXOSX: subl	%edi, %edx
; LINUXOSX: subl	%esi, %edx
; LINUXOSX: subl	%r9d, %r8d
; LINUXOSX: subl	%r13d, %r12d
; LINUXOSX: subl	%r14d, %r12d
; LINUXOSX: movl	%edx, %ecx
; LINUXOSX: movl	%r8d, %edx
; LINUXOSX: movl	%r12d, %edi
; LINUXOSX: callq	_func_i32
; LINUXOSX: popq	%r12
; LINUXOSX: popq	%r13
; LINUXOSX: popq	%r14
; LINUXOSX: retq

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
