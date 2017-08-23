; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-dynamic-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;; Testing of caseswhich cannot be generated out of OpenCL code: Intrinsic call

; CHECK: @test
; CHECK: %AddrSpace = addrspacecast i8 addrspace(4)* %0 to i8*
; CHECK: %AddrSpace1 = addrspacecast i8 addrspace(4)* %1 to i8*
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i32(i8* %AddrSpace, i8* %AddrSpace1, i32 1000, i32 1, i1 false)
; CHECK: ret

; CHECK: declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture writeonly, i8* nocapture readonly, i32, i32, i1)

declare void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)*, i8 addrspace(4)*, i32, i32, i1)


define void @test(i8 addrspace(1)* %a, i8 addrspace(1)* %b, i1 %param) nounwind {
entry:
  %0 = addrspacecast i8 addrspace(1)* %a to i8 addrspace(4)*
  %1 = addrspacecast i8 addrspace(1)* %b to i8 addrspace(4)*
  call void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)* %0, i8 addrspace(4)* %1, i32 1000, i32 1, i1 false)
  ret void
}
