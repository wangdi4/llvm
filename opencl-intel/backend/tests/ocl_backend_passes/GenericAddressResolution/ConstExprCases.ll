; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-static-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @const_bitcast
; CHECK: 0 = load i32 addrspace(3)* getelementptr inbounds ([1 x i32] addrspace(3)* @const_bitcast.loc, i32 0, i32 0), align 4
; CHECK: ret

; CHECK: @const_gep
; CHECK: %0 = load i32 addrspace(3)* getelementptr inbounds ([5 x i32] addrspace(3)* @const_gep.loc, i32 0, i32 2), align 4
; CHECK: ret

; CHECK: @const_select
; CHECK: %0 = load i32 addrspace(3)* select (i1 icmp ne (i32 ptrtoint ([5 x i32] addrspace(3)* @const_select.loc1 to i32), i32 0), i32 addrspace(3)* getelementptr inbounds ([5 x i32] addrspace(3)* @const_select.loc1, i32 0, i32 0), i32 addrspace(3)* getelementptr inbounds ([5 x i32] addrspace(3)* @const_select.loc2, i32 0, i32 0)), align 4
; CHECK: ret

; CHECK-NOT: !opencl.compiler.2_0.gen_addr_space_pointer_warnings
; CHECK-NOT: !opencl.compiler.2_0.gen_addr_space_pointer_counter

@const_bitcast.loc = internal addrspace(3) global [1 x i32] zeroinitializer, align 4
@const_gep.loc = internal addrspace(3) global [5 x i32] zeroinitializer, align 4
@const_select.loc1 = internal addrspace(3) global [5 x i32] zeroinitializer, align 4
@const_select.loc2 = internal addrspace(3) global [5 x i32] zeroinitializer, align 4

define void @const_bitcast(i32 addrspace(1)* %dst) nounwind {
entry:
  %0 = load i32 addrspace(4)* addrspacecast ([1 x i32] addrspace(3)* @const_bitcast.loc to i32 addrspace(4)*), align 4
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %dst, i32 0
  store i32 %0, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

define void @const_gep(i32 addrspace(1)* %dst) nounwind {
entry:
  %0 = load i32 addrspace(4)* getelementptr inbounds (i32 addrspace(4)* addrspacecast ([5 x i32] addrspace(3)* @const_gep.loc to i32 addrspace(4)*), i32 2), align 4
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %dst, i32 0
  store i32 %0, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

define void @const_select(i32 addrspace(1)* %dst) nounwind {
entry:
  %0 = load i32 addrspace(4)* select (i1 icmp ne (i32 ptrtoint ([5 x i32] addrspace(3)* @const_select.loc1 to i32), i32 0), i32 addrspace(4)* addrspacecast ([5 x i32] addrspace(3)* @const_select.loc1 to i32 addrspace(4)*), i32 addrspace(4)* addrspacecast ([5 x i32] addrspace(3)* @const_select.loc2 to i32 addrspace(4)*)), align 4
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %dst, i32 0
  store i32 %0, i32 addrspace(1)* %arrayidx, align 4
  ret void
}

!opencl.kernels = !{!0, !1, !2}
!opencl.enable.FP_CONTRACT = !{}

!0 = !{void (i32 addrspace(1)*)* @const_bitcast}
!1 = !{void (i32 addrspace(1)*)* @const_gep}
!2 = !{void (i32 addrspace(1)*)* @const_select}

;; ----- ConstExprCases.cl -----
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h  -D__OPENCL_C_VERSION__=200 ConstExprCases.cl -o ConstExprCasesTmp.ll
;;               oclopt.exe -mem2reg -verify ConstExprCasesTmp.ll -S -o ConstExprCases.ll

;;__kernel void const_bitcast(__global int* dst)
;;{
;;      __local int loc[1];
;;      dst[0] = ((int*)(loc))[0];
;;}

;;__kernel void const_gep(__global int* dst)
;;{
;;     __local int loc[5];
;;     dst[0] = ((int*)(loc))[2];
;;}


;;__kernel void const_select(__global int* dst)
;;{
;;     __local int loc1[5];
;;     __local int loc2[5];
               
;;     dst[0] = ((int)loc1 ? (int*)loc1 : (int*)loc2)[0];
;;}
