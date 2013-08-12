; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-dynamic-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;; Testing of cases which cannot be generated out of OpenCL code:
;; 1. Select instruction
;; 2. Intrinsic call

; CHECK: @test
; CHECK: %AddrSpace = select i1 %param, i32 1, i32 1
; CHECK: %AddrSpace1 = icmp eq i32 %AddrSpace, 0
; CHECK: %AddrSpace2 = bitcast i8 addrspace(4)* %0 to i8*
; CHECK: %AddrSpace3 = bitcast i8 addrspace(4)* %1 to i8*
; CHECK-NOT: @_Z10is_privatePKU3AS4v
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i32(i8* %AddrSpace2, i8* %AddrSpace3, i32 1000, i32 1, i1 false)
; CHECK: ret

; CHECK: declare void @llvm.memcpy.p0i8.p0i8.i32(i8* nocapture, i8* nocapture, i32, i32, i1) nounwind

declare void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)*, i8 addrspace(4)*, i32, i32, i1)

declare zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)*)

define void @test(i8 addrspace(1)* %a, i8 addrspace(1)* %b, i1 %param) nounwind {
entry:
  %0 = bitcast i8 addrspace(1)* %a to i8 addrspace(4)*
  %1 = bitcast i8 addrspace(1)* %b to i8 addrspace(4)*
  %2 = select i1 %param, i8 addrspace(4)* %0, i8 addrspace(4)* %1
  %3 = call zeroext i1 @_Z10is_privatePKU3AS4v(i8 addrspace(4)* %2)
  call void @llvm.memcpy.p4i8.p4i8.i32(i8 addrspace(4)* %0, i8 addrspace(4)* %1, i32 1000, i32 1, i1 false)
  ret void
}
