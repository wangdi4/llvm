; RUN: llc < %s -mtriple=i686-apple-darwin -mcpu=knl | FileCheck -check-prefix=X32 %s
; RUN: llc < %s -mtriple=i386-pc-win32 -mcpu=knl | FileCheck -check-prefix=X32 %s
; RUN: llc < %s -mtriple=x86_64-win32 -mcpu=knl | FileCheck -check-prefix=WIN64 %s
; RUN: llc < %s -mtriple=x86_64-apple-darwin -mcpu=knl | FileCheck -check-prefix=LINUXOSX64 %s

declare <128 x float> @func_float32_ptr(<128 x float>, <128 x float> *)

; WIN64: testf32_inp
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: movaps {{%xmm([8-9]|1[0-5])}}, {{.*(%rbp).*}}  {{#+}} 16-byte Spill
; WIN64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm([0-7])}}
; WIN64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm([0-7])}}
; WIN64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm([0-7])}}
; WIN64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm([0-7])}}
; WIN64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm([0-7])}}
; WIN64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm([0-7])}}
; WIN64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm([0-7])}}
; WIN64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm([0-7])}}
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
; X32: addps  {{.*}}, {{%zmm[0-7]}}
; X32: addps  {{.*}}, {{%zmm[0-7]}}
; X32: addps  {{.*}}, {{%zmm[0-7]}}
; X32: addps  {{.*}}, {{%zmm[0-7]}}
; X32: addps  {{.*}}, {{%zmm[0-7]}}
; X32: addps  {{.*}}, {{%zmm[0-7]}}
; X32: addps  {{.*}}, {{%zmm[0-7]}}
; X32: addps  {{.*}}, {{%zmm[0-7]}}
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
; LINUXOSX64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm[0-7]}}
; LINUXOSX64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm[0-7]}}
; LINUXOSX64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm[0-7]}}
; LINUXOSX64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm[0-7]}}
; LINUXOSX64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm[0-7]}}
; LINUXOSX64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm[0-7]}}
; LINUXOSX64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm[0-7]}}
; LINUXOSX64: addps  {{%zmm([8-9]|1[0-5])}}, {{%zmm[0-7]}}
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
define __regcall <128 x float> @testf32_inp(<128 x float> %a, <128 x float> %b) nounwind {
  %y = alloca <128 x float>, align 64
  %x = fadd <128 x float> %a, %b
  %1 = call <128 x float> @func_float32_ptr(<128 x float> %x, <128 x float>* %y) 
  %2 = load <128 x float>, <128 x float>* %y, align 32
  %3 = fadd <128 x float> %2, %1
  ret <128 x float> %3
}
