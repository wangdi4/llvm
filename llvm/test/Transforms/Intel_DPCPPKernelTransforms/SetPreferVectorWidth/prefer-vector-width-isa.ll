; RUN: opt -passes=dpcpp-kernel-set-prefer-vector-width -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s | FileCheck %s
; RUN: opt -passes=dpcpp-kernel-set-prefer-vector-width -dpcpp-vector-variant-isa-encoding-override=AVX512Core -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

define void @foo() #0 {
; CHECK-LABEL: define void @foo
; CHECK-SAME: [[ATTR:#[0-9]+]]
  ret void
}

; 'prefer-vector-width' shouldn't be set on llvm intrinsics.
declare <16 x float> @llvm.masked.gather.v16f32.v16p1f32(<16 x float addrspace(1)*>, i32 immarg, <16 x i1>, <16 x float>) #0
; CHECK-LABEL: declare <16 x float> @llvm.masked.gather.v16f32.v16p1f32
; CHECK-SAME: [[ATTR1:#[0-9]+]]

; 'prefer-vector-width' should be set on declarations as well, so that it
; matches between caller and callee.
declare void @bar() #0
; CHECK-LABEL: declare void @bar
; CHECK-SAME: [[ATTR]]

attributes #0 = { nounwind willreturn }
; CHECK: attributes [[ATTR]] =
; CHECK-SAME: "prefer-vector-width"="512"
; CHECK: attributes [[ATTR1]] =
; CHECK-NOT: "prefer-vector-width"

; DEBUGIFY-NOT: WARNING
