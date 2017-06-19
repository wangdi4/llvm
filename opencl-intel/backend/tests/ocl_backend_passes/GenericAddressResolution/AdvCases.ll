; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-static-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;; Testing of cases which cannot be generated out of OpenCL code:
;; 1. Select instruction
;; 2. Intrinsic call

; CHECK: @test
; CHECK: %2 = select i1 %param, i8 addrspace(1)* %0, i8 addrspace(1)* %1
; CHECK: call void @llvm.memcpy.p1i8.p1i8.i32(i8 addrspace(1)* %0, i8 addrspace(1)* %1, i32 1000, i32 1, i1 false)
; CHECK: ret

; CHECK: declare void @llvm.memcpy.p1i8.p1i8.i32(i8 addrspace(1)* nocapture writeonly, i8 addrspace(1)* nocapture readonly, i32, i32, i1)
; CHECK-NOT: !opencl.compiler.2_0.gen_addr_space_pointer_counter = !{!4}

declare void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)*, i8 addrspace(4)*, i32, i32, i1)

define void @test(i8 addrspace(1)* %a, i8 addrspace(1)* %b, i1 %param) nounwind {
entry:
  %0 = addrspacecast i8 addrspace(1)* %a to i8 addrspace(4)*
  %1 = addrspacecast i8 addrspace(1)* %b to i8 addrspace(4)*
  %2 = select i1 %param, i8 addrspace(4)* %0, i8 addrspace(4)* %1
  call void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)* %0, i8 addrspace(4)* %1, i32 1000, i32 1, i1 false)
  ret void
}
