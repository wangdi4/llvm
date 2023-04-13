; RUN: opt -opaque-pointers=0 -passes="infer-address-spaces" -S -mtriple=x86_64-unknown-unknown -override-flat-addr-space=4 -infer-as-rewrite-opencl-bis %s | FileCheck %s --check-prefix=CHECK-REWRITE
; RUN: opt -opaque-pointers=0 -passes="infer-address-spaces" -S -mtriple=x86_64-unknown-unknown -override-flat-addr-space=4 %s | FileCheck %s --check-prefix=CHECK-NOREWRITE

declare float @_Z5fractfPU3AS4f(float %x, float addrspace(4)* nocapture %iptr) #1

declare i1 @_Z30atomic_compare_exchange_strongPU3AS4VU7_AtomiciPU3AS4ii(i32 addrspace(4)*, i32 addrspace(4)*, i32) #1

define float @f1(float %x, float addrspace(1)* nocapture %iptr) #1 {
  %1 = addrspacecast float addrspace(1)* %iptr to float addrspace(4)*
  %2 = call float @_Z5fractfPU3AS4f(float %x, float addrspace(4)* %1)
  ret float %2

; CHECK-REWRITE:        %1 = call float @_Z5fractfPU3AS1f(float %x, float addrspace(1)* %iptr)
; CHECK-REWRITE-NEXT:   ret float %1

; CHECK-NOREWRITE:      %2 = call float @_Z5fractfPU3AS4f(float %x, float addrspace(4)* %1)
; CHECK-NOREWRITE-NEXT: ret float %2
}

define i1 @f2(float %x, i32 addrspace(1)* nocapture %iptr) #1 {
  %expected = alloca i32, align 4
  store i32 42, i32* %expected, align 4
  %1 = addrspacecast i32 addrspace(1)* %iptr to i32 addrspace(4)*
  %2 = addrspacecast i32* %expected to i32 addrspace(4)*

  %3 = call zeroext i1 @_Z30atomic_compare_exchange_strongPU3AS4VU7_AtomiciPU3AS4ii(i32 addrspace(4)* %1, i32 addrspace(4)* %2, i32 43)
  ret i1 %3
; CHECK-REWRITE:        call zeroext i1 @_Z30atomic_compare_exchange_strongPU3AS1VU7_AtomiciPii(i32 addrspace(1)* %iptr, i32* %expected, i32 43)
; CHECK-REWRITE-NEXT:   ret i1 %1

; CHECK-NOREWRITE:      call zeroext i1 @_Z30atomic_compare_exchange_strongPU3AS4VU7_AtomiciPU3AS4ii(i32 addrspace(4)* %1, i32 addrspace(4)* %2, i32 43)
; CHECK-NOREWRITE-NEXT: ret i1 %3
}

attributes #1 = { convergent nounwind writeonly }
