; RUN: llvm-as < %s | llvm-dis | FileCheck %s

define void @f1() alwaysinline_recursive
; CHECK: define void @f1() #0
{
  call void @f1()
  ret void
}

define void @f2() inlinehint_recursive
; CHECK: define void @f2() #1
{
  call void @f2()
  ret void
}

;CHECK: attributes #0 = { alwaysinline_recursive }
;CHECK: attributes #1 = { inlinehint_recursive }
