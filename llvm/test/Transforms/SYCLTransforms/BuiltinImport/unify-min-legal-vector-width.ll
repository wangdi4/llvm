; RUN: llvm-as %s.rtl -o %t.rtl.bc
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%t.rtl.bc -passes=sycl-kernel-builtin-import %s -S | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "x86_64-pc-linux"

declare <16 x i32> @builtin_requires_zmm(<16 x i32> %in)
declare <16 x i32> @builtin_calling_other_builtin(<16 x i32> %in)
declare <16 x i32> @builtin_without_restriction(<16 x i32> %in)

define void @kernel1() {
; CHECK-DAG: define void @kernel1() #[[#ATTR:]]
entry:
  %call = call <16 x i32> @builtin_requires_zmm(<16 x i32> undef)
  ret void
}

define void @kernel2() #0 {
;; "min-legal-vector-width" of this kernel will be overwritten to 512.
; CHECK-DAG: define void @kernel2() #[[#ATTR:]]
entry:
  %call = call <16 x i32> @builtin_requires_zmm(<16 x i32> undef)
  ret void
}

define void @kernel3() #1 {
; CHECK-DAG: define void @kernel3() #[[#ATTR:]]
entry:
  %call = call <16 x i32> @builtin_without_restriction(<16 x i32> undef)
  ret void
}
;; "min-legal-vector-width" of the callee builtin is reset according to its caller function.
; CHECK-DAG: define internal <16 x i32> @builtin_without_restriction{{.*}} #[[#ATTR:]]

define void @kernel4() {
; CHECK-DAG: define void @kernel4() #[[#ATTR:]]
entry:
  %call = call <16 x i32> @builtin_calling_other_builtin(<16 x i32> undef)
  ret void
}
;; "min-legal-vector-width" of the callee builtin is reset because it calls builtin_requires_zmm
; CHECK-DAG: define internal <16 x i32> @builtin_calling_other_builtin{{.*}} #[[#ATTR:]]

attributes #0 = { "min-legal-vector-width"="0" }
attributes #1 = { "min-legal-vector-width"="512" }

; CHECK: attributes #[[#ATTR]] = { "min-legal-vector-width"="512" }

; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function builtin_requires_zmm
; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function builtin_calling_other_builtin
; DEBUGIFY-COUNT-2: WARNING: Instruction with empty DebugLoc in function builtin_without_restriction
; DEBUGIFY-NOT: WARNING
; DEBUGIFY: PASS
