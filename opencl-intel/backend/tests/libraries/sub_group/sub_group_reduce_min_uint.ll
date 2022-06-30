; Checks that we are calling the correct unsigned intrinsics for uint data type.

; REQUIRES: 64bit
; RUN: llvm-extract %libdir/clbltfnz0.rtl -rfunc 'sub_group_reduce_minDv[0-9]+_j' -S -o - | FileCheck %s

; CHECK-NOT: call i32 @llvm{{.*}}smin
; CHECK-DAG: call i32 @llvm.vector.reduce.umin
; FIXME: We are using for-loop + scalr @llvm.umin for 32-way and 64-way
; implementations, which is not optimal. We should use @llvm.vector.reduce.umin
; instead.
; CHECK-DAG: call i32 @llvm.umin.i32
