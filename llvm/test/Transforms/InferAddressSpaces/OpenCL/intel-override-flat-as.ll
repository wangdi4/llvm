; RUN: opt -passes="infer-address-spaces" -S -mtriple=x86_64-unknown-unknown -override-flat-addr-space=4 %s | FileCheck %s --check-prefix=CHECK-OVERRIDE
; RUN: opt -passes="infer-address-spaces" -S -mtriple=x86_64-unknown-unknown %s | FileCheck %s --check-prefix=CHECK-NOOVERRIDE

define i32 @foo(ptr addrspace(1)) local_unnamed_addr #0 {
entry:
  %gen = addrspacecast ptr addrspace(1) %0 to ptr addrspace(4)
  %val = load i32, ptr addrspace(4) %gen
; CHECK-OVERRIDE:   %val = load i32, ptr addrspace(1) %0
; CHECK-NOOVERRIDE: %val = load i32, ptr addrspace(4) %gen
  ret i32 %val
}

attributes #0 = { convergent nounwind }

