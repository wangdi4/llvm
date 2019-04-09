; RUN: opt -S -mtriple=x86_64-unknown-unknown -infer-address-spaces -override-flat-addr-space=4 %s | FileCheck %s --check-prefix=CHECK-OVERRIDE
; RUN: opt -S -mtriple=x86_64-unknown-unknown -infer-address-spaces %s | FileCheck %s --check-prefix=CHECK-NOOVERRIDE

define i32 @foo(i32 addrspace(1)*) local_unnamed_addr #0 {
entry:
  %gen = addrspacecast i32 addrspace(1)* %0 to i32 addrspace(4)*
  %val = load i32, i32 addrspace(4)* %gen
; CHECK-OVERRIDE:   %val = load i32, i32 addrspace(1)* %0
; CHECK-NOOVERRIDE: %val = load i32, i32 addrspace(4)* %gen
  ret i32 %val
}

attributes #0 = { convergent nounwind }

