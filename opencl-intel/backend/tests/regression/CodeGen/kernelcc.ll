; RUN: llc < %s -mtriple=x86_64-pc-win32 | FileCheck --check-prefix=CHECK_WIN64 %s
; RUN: llc < %s -mtriple=i686-pc-win32 | FileCheck --check-prefix=CHECK_WIN32 %s
;
; This test should pass as long as the code-generator assumes the following for kernelcc arguments passed on the stack frame:
; 1. v2i32 has an address alignment of 16-bytes on Win64
; 2. v2i32 has an address alignment of 8-bytes on Win32
; 3. v3i32 has an address alignment of 16-bytes on Win64
; 4. v3i32 has an address alignment of 8-bytes on Win32
; 5. v2i32 occupies a 16-byte wide frame slot
; 6. v3i32 occupies a 16-byte wide frame slot

define x86_ocl_kernelcc void @test_v2i32(i32* nocapture %output, i8 %i1, i8 %i2, <2 x i32> %myvec2, i8 %i3) nounwind {
; Win64 Frame Objects:
;  %i3      fi#-5: size=1, align=16, fixed, at location [SP+88]
;  %myvec2  fi#-4: size=16, align=16, fixed, at location [SP+72]
;  %i2      fi#-3: size=1, align=16, fixed, at location [SP+56]
;  %i1      fi#-2: size=1, align=8, fixed, at location [SP+48]
;  %output  fi#-1: size=8, align=16, fixed, at location [SP+40]

; Red zone
; CHECK_WIN64: subq $32, %rsp

; myvec2.x
; CHECK_WIN64: movl 104(%rsp), %eax
; myvec2.y
; CHECK_WIN64: movl 108(%rsp), %ecx
; output
; CHECK_WIN64: movq 72(%rsp), %rdx
; CHECK_WIN64: movl %eax, (%rdx)
; CHECK_WIN64: movl %ecx, 4(%rdx)
; CHECK_WIN64: addq $32, %rsp

; Win32 Frame Objects:
;  %i3      fi#-5: size=1, align=8, fixed, at location [SP+36]
;  %myvec2  fi#-4: size=16, align=8, fixed, at location [SP+20]
;  %i2      fi#-3: size=1, align=8, fixed, at location [SP+12]
;  %i1      fi#-2: size=1, align=4, fixed, at location [SP+8]
;  %output  fi#-1: size=4, align=8, fixed, at location [SP+4]

; CHECK_WIN32: movl 20(%esp), %eax
; CHECK_WIN32: movl 24(%esp), %ecx
; CHECK_WIN32: movl 4(%esp), %edx
; CHECK_WIN32: movl %eax, (%edx)
; CHECK_WIN32: movl %ecx, 4(%edx)


  %1 = extractelement <2 x i32> %myvec2, i32 0
  store i32 %1, i32* %output, align 4
  %2 = extractelement <2 x i32> %myvec2, i32 1
  %3 = getelementptr inbounds i32* %output, i64 1
  store i32 %2, i32* %3, align 4
  ret void
}


define x86_ocl_kernelcc void @test_v3i32(i32* nocapture %output, i8 %i1, i8 %i2, <3 x i32> %myvec2, i8 %i3) nounwind {

; Red zone
; CHECK_WIN64: subq $32, %rsp

; myvec2.x
; CHECK_WIN64: movl 104(%rsp), %eax
; myvec2.y
; CHECK_WIN64: movl 108(%rsp), %ecx
; output
; CHECK_WIN64: movq 72(%rsp), %rdx
; CHECK_WIN64: movl %eax, (%rdx)
; CHECK_WIN64: movl %ecx, 4(%rdx)
; CHECK_WIN64: addq $32, %rsp


; CHECK_WIN32: movl 20(%esp), %eax
; CHECK_WIN32: movl 24(%esp), %ecx
; CHECK_WIN32: movl 4(%esp), %edx
; CHECK_WIN32: movl %eax, (%edx)
; CHECK_WIN32: movl %ecx, 4(%edx)
; CHECK_WIN32: movl 28(%esp), %eax
; CHECK_WIN32: movl %eax, 8(%edx)
; CHECK_WIN32: movl 32(%esp), %eax
; CHECK_WIN32: movl %eax, 12(%edx)


  %1 = extractelement <3 x i32> %myvec2, i32 0
  store i32 %1, i32* %output, align 4
  %2 = extractelement <3 x i32> %myvec2, i32 1
  %3 = getelementptr inbounds i32* %output, i64 1
  store i32 %2, i32* %3, align 4
  %4 = extractelement <3 x i32> %myvec2, i32 2
  %5 = getelementptr inbounds i32* %output, i64 2
  store i32 %4, i32* %5, align 4
  %6 = extractelement <3 x i32> %myvec2, i32 3
  %7 = getelementptr inbounds i32* %output, i64 3
  store i32 %6, i32* %7, align 4
  ret void
}

