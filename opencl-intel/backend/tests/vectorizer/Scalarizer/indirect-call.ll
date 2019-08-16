; RUN: llvm-as %s -o %t.bc
; check that SclaraizeFunction pass ignores indirect calls while in VPlan pipeline
; RUN: %oclopt -scalarize -force-scalarizer-in-vplan-pipeline=true -verify %t.bc -S -o %t.2.bc

; ModuleID = 'Program'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define void @foo(i32 %val, <4 x i32> %input, i32 addrspace(1)* %ret) {
  %ptr = inttoptr i32 %val to i32 (<4 x i32>)*
  %call = call i32 %ptr(<4 x i32> %input)
  store i32 %call, i32 addrspace(1)* %ret
  ret void
}

define void @_ZGVe4uuu_foovec(i32 %val, <4 x i32> %input, i32 addrspace(1)* %ret) {
  %ptr = inttoptr i32 %val to i32 (<4 x i32>)*
  %call = call i32 %ptr(<4 x i32> %input)
  store i32 %call, i32 addrspace(1)* %ret
  ret void
}


