; RUN: opt < %s -passes=gvn -S | FileCheck %s

; CHECK: define i32 @main(i32 %i) {
; CHECK-NEXT: block1:
; CHECK-NEXT:   %z1.fpga.reg = call i32 @llvm.fpga.reg.i32(i32 %i)
; CHECK-NEXT:   %z2.fpga.reg = call i32 @llvm.fpga.reg.i32(i32 %i)
; CHECK-NEXT:   ret i32 %i
; CHECK-NEXT: }

; This test ensures that even if the return value of 
; a call to llvm.fpga.reg is not used, it still won't
; be treated as a dead instruction, as users specifically
; asked for a register to be inserted.

source_filename = "llvm-link"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

; Function Attrs: inaccessiblememonly nounwind willreturn nofree
declare i32 @llvm.fpga.reg.i32(i32) #0

define i32 @main(i32 %i) {
block1:
  %z1 = bitcast i32 %i to i32
  %z1.fpga.reg = call i32 @llvm.fpga.reg.i32(i32 %z1)
  br label %block2
block2:
  %z2 = bitcast i32 %i to i32
  %z2.fpga.reg = call i32 @llvm.fpga.reg.i32(i32 %z2)
  ret i32 %z2
}

attributes #0 = { inaccessiblememonly nounwind willreturn nofree }
