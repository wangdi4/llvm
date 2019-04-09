; RUN: opt -S -mtriple=x86_64-unknown-unknown -infer-address-spaces -override-flat-addr-space=4 -infer-as-rewrite-opencl-bis %s | FileCheck %s --check-prefix=CHECK-REWRITE
; RUN: opt -S -mtriple=x86_64-unknown-unknown -infer-address-spaces -override-flat-addr-space=4 %s | FileCheck %s --check-prefix=CHECK-NOREWRITE

declare float @_Z5fractfPU3AS4f(float %x, float addrspace(4)* nocapture %iptr) #1

define float @f1(float %x, float addrspace(1)* nocapture %iptr) #1 {
  %1 = addrspacecast float addrspace(1)* %iptr to float addrspace(4)*
  %2 = call float @_Z5fractfPU3AS4f(float %x, float addrspace(4)* %1)
  ret float %2

; CHECK-REWRITE:        %1 = call float @_Z5fractfPU3AS1f(float %x, float addrspace(1)* %iptr)
; CHECK-REWRITE-NEXT:   ret float %1

; CHECK-NOREWRITE:      %2 = call float @_Z5fractfPU3AS4f(float %x, float addrspace(4)* %1)
; CHECK-NOREWRITE-NEXT: ret float %2
}

attributes #1 = { convergent nounwind writeonly }
