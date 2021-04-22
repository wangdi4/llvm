; RUN: opt -dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-add-implicit-args %s -S | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-add-implicit-args %s -S | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; This tests checks that
; 1. new function has same attributes like old function
; 2. new function arguments has same attributes like old function arguments
; 3. new function linkage has attributes like old function linkage
; 4. old function became a declaration with no linkage

define internal void @foo(<2 x i8>* byval(<2 x i8>) align 8 %x) nounwind {
entry:
  ret void
}

; CHECK: declare {{.*}} void @__foo_before.AddImplicitArgs(<2 x i8>* byval(<2 x i8>) align 8) #0
; CHECK: define internal void @foo(<2 x i8>* byval(<2 x i8>) align 8 %x,
; CHECK:  #0 {

; CHECK:  #0 = { nounwind }

; DEBUGIFY-NOT: WARNING
