; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-dynamic-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

; CHECK: @const_bitcast
; CHECK-NOT:  %call = call i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)* bitcast ([1 x i32] addrspace(3)* @const_bitcast.loc to i8 addrspace(4)*))
; CHECK: %ToNamedPtr = bitcast i8 addrspace(4)* bitcast ([1 x i32] addrspace(3)* @const_bitcast.loc to i8 addrspace(4)*) to i8 addrspace(1)*
; CHECK: ret

; CHECK: @const_gep
; CHECK-NOT: %call = call i8 addrspace(3)* @_Z8to_localPKU3AS4v(i8 addrspace(4)* bitcast (i32 addrspace(4)* getelementptr inbounds (i32 addrspace(4)* bitcast ([5 x i32] addrspace(3)* @const_gep.loc to i32 addrspace(4)*), i32 2) to i8 addrspace(4)*))
; CHECK:  %ToNamedPtr = bitcast i8 addrspace(4)* bitcast (i32 addrspace(4)* getelementptr inbounds (i32 addrspace(4)* bitcast ([5 x i32] addrspace(3)* @const_gep.loc to i32 addrspace(4)*), i32 2) to i8 addrspace(4)*) to i8 addrspace(3)*
; CHECK: ret

; CHECK: @const_select
; CHECK-NOT:  %call = call i8* @_Z10to_privatePKU3AS4v(i8 addrspace(4)* bitcast (i32 addrspace(4)* select (i1 icmp ne (i32 ptrtoint ([5 x i32] addrspace(3)* @const_select.loc1 to i32), i32 0), i32 addrspace(4)* bitcast ([5 x i32] addrspace(3)* @const_select.loc1 to i32 addrspace(4)*), i32 addrspace(4)* bitcast ([5 x i32] addrspace(3)* @const_select.loc2 to i32 addrspace(4)*)) to i8 addrspace(4)*))
; CHECK: %ToNamedPtr = bitcast i8 addrspace(4)* bitcast (i32 addrspace(4)* select (i1 icmp ne (i32 ptrtoint ([5 x i32] addrspace(3)* @const_select.loc1 to i32), i32 0), i32 addrspace(4)* bitcast ([5 x i32] addrspace(3)* @const_select.loc1 to i32 addrspace(4)*), i32 addrspace(4)* bitcast ([5 x i32] addrspace(3)* @const_select.loc2 to i32 addrspace(4)*)) to i8 addrspace(4)*) to i8*
; CHECK: ret


@const_bitcast.loc = internal addrspace(3) global [1 x i32] zeroinitializer, align 4
@const_gep.loc = internal addrspace(3) global [5 x i32] zeroinitializer, align 4
@const_select.loc1 = internal addrspace(3) global [5 x i32] zeroinitializer, align 4
@const_select.loc2 = internal addrspace(3) global [5 x i32] zeroinitializer, align 4

define void @const_bitcast(i32 addrspace(1)* %dst) nounwind {
entry:
  %call = call i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)* bitcast ([1 x i32] addrspace(3)* @const_bitcast.loc to i8 addrspace(4)*))
  %0 = bitcast i8 addrspace(1)* %call to i32 addrspace(1)*
  ret void
}

declare i8 addrspace(1)* @_Z9to_globalPKU3AS4v(i8 addrspace(4)*)

define void @const_gep(i32 addrspace(1)* %dst) nounwind {
entry:
  %call = call i8 addrspace(3)* @_Z8to_localPKU3AS4v(i8 addrspace(4)* bitcast (i32 addrspace(4)* getelementptr inbounds (i32 addrspace(4)* bitcast ([5 x i32] addrspace(3)* @const_gep.loc to i32 addrspace(4)*), i32 2) to i8 addrspace(4)*))
  %0 = bitcast i8 addrspace(3)* %call to i32 addrspace(3)*
  ret void
}

declare i8 addrspace(3)* @_Z8to_localPKU3AS4v(i8 addrspace(4)*)

define void @const_select(i32 addrspace(1)* %dst) nounwind {
entry:
  %call = call i8* @_Z10to_privatePKU3AS4v(i8 addrspace(4)* bitcast (i32 addrspace(4)* select (i1 icmp ne (i32 ptrtoint ([5 x i32] addrspace(3)* @const_select.loc1 to i32), i32 0), i32 addrspace(4)* bitcast ([5 x i32] addrspace(3)* @const_select.loc1 to i32 addrspace(4)*), i32 addrspace(4)* bitcast ([5 x i32] addrspace(3)* @const_select.loc2 to i32 addrspace(4)*)) to i8 addrspace(4)*))
  %0 = bitcast i8* %call to i32*
  ret void
}

declare i8* @_Z10to_privatePKU3AS4v(i8 addrspace(4)*)

!opencl.kernels = !{!0, !1, !2}
!opencl.enable.FP_CONTRACT = !{}

!0 = metadata !{void (i32 addrspace(1)*)* @const_bitcast}
!1 = metadata !{void (i32 addrspace(1)*)* @const_gep}
!2 = metadata !{void (i32 addrspace(1)*)* @const_select}

;; ----- ConstExprCasesDynamic.cl -----
;; Command line: clang.exe -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I <clang_headers> -include opencl_.h -D__OPENCL_C_VERSION__=200 ConstExprCasesDynamic.cl -o ConstExprCasesDynamicTmp.ll
;;               oclopt.exe -mem2reg -verify ConstExprCasesDynamicTmp.ll -S -o ConstExprCasesDynamic.ll

;;__kernel void const_bitcast(__global int* dst)
;;{
;;      __local int loc[1];
;;      __global int* a = to_global(&(((int*)(loc))[0]));
;;}

;;__kernel void const_gep(__global int* dst)
;;{
;;     __local int loc[5];
;;     __local int* a = to_local(&(((int*)(loc))[2]));
;;}


;;__kernel void const_select(__global int* dst)
;;{
;;     __local int loc1[5];
;;     __local int loc2[5];
;;               
;;     __private int* a = to_private(&(((int)loc1 ? (int*)loc1 : (int*)loc2)[0]));
;;}
