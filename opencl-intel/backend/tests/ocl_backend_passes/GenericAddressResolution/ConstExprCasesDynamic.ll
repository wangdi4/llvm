; RUN: llvm-as %s -o %t.bc
; RUN: opt -generic-addr-dynamic-resolution -verify %t.bc -S -o %t1.ll
; RUN: FileCheck %s --input-file=%t1.ll

;; ----- ConstExprCasesDynamic.cl -----
;; Command line: clang -cc1 -cl-std=CL2.0 -emit-llvm -O0 -x cl -I clang_headers -include opencl_.h -D__OPENCL_C_VERSION__=200 -triple=spir64-unknown-unknown ConstExprCasesDynamic.cl -o ConstExprCasesDynamicTmp.ll
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

; CHECK-NOT:   call spir_func i8 addrspace(1)* @__to_global
; CHECK:       %ToNamedPtr = addrspacecast i8 addrspace(4)* {{.*}} to i8 addrspace(1)*
; CHECK:       bitcast i8 addrspace(1)* %ToNamedPtr to i32 addrspace(1)*
; CHECK:       ret void

; CHECK-NOT:   call spir_func i8 addrspace(3)* @_Z8to_localPU3AS4Kv
; CHECK:       %ToNamedPtr = addrspacecast i8 addrspace(4)* {{.*}} to i8 addrspace(3)*
; CHECK:       bitcast i8 addrspace(3)* %ToNamedPtr to i32 addrspace(3)*
; CHECK:       ret void

; CHECK-NOT:   call spir_func i8* @__to_private
; CHECK:       %ToNamedPtr = addrspacecast i8 addrspace(4)* {{.*}} to i8*
; CHECK:       bitcast i8* %ToNamedPtr to i32*
; CHECK:       ret void

; ModuleID = 'ConstExprCasesDynamic.cl'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

@const_bitcast.loc = internal addrspace(3) global [1 x i32] undef, align 4
@const_gep.loc = internal addrspace(3) global [5 x i32] undef, align 4
@const_select.loc1 = internal addrspace(3) global [5 x i32] undef, align 4
@const_select.loc2 = internal addrspace(3) global [5 x i32] undef, align 4

; Function Attrs: nounwind
define spir_kernel void @const_bitcast(i32 addrspace(1)* %dst) #0 {
  %1 = call spir_func i8 addrspace(1)* @__to_global(i8 addrspace(4)* addrspacecast (i8 addrspace(3)* bitcast ([1 x i32] addrspace(3)* @const_bitcast.loc to i8 addrspace(3)*) to i8 addrspace(4)*)) #2
  %2 = bitcast i8 addrspace(1)* %1 to i32 addrspace(1)*
  ret void
}

; Function Attrs: nounwind readnone
declare spir_func i8 addrspace(1)* @__to_global(i8 addrspace(4)*) #1

; Function Attrs: nounwind
define spir_kernel void @const_gep(i32 addrspace(1)* %dst) #0 {
  %1 = call spir_func i8 addrspace(3)* @__to_local(i8 addrspace(4)* bitcast (i32 addrspace(4)* getelementptr inbounds (i32, i32 addrspace(4)* addrspacecast (i32 addrspace(3)* getelementptr inbounds ([5 x i32], [5 x i32] addrspace(3)* @const_gep.loc, i32 0, i32 0) to i32 addrspace(4)*), i64 2) to i8 addrspace(4)*)) #2
  %2 = bitcast i8 addrspace(3)* %1 to i32 addrspace(3)*
  ret void
}

; Function Attrs: nounwind readnone
declare spir_func i8 addrspace(3)* @__to_local(i8 addrspace(4)*) #1

; Function Attrs: nounwind
define spir_kernel void @const_select(i32 addrspace(1)* %dst) #0 {
  %1 = call spir_func i8* @__to_private(i8 addrspace(4)* bitcast (i32 addrspace(4)* select (i1 icmp ne (i32 ptrtoint ([5 x i32] addrspace(3)* @const_select.loc1 to i32), i32 0), i32 addrspace(4)* addrspacecast (i32 addrspace(3)* getelementptr inbounds ([5 x i32], [5 x i32] addrspace(3)* @const_select.loc1, i32 0, i32 0) to i32 addrspace(4)*), i32 addrspace(4)* addrspacecast (i32 addrspace(3)* getelementptr inbounds ([5 x i32], [5 x i32] addrspace(3)* @const_select.loc2, i32 0, i32 0) to i32 addrspace(4)*)) to i8 addrspace(4)*)) #2
  %2 = bitcast i8* %1 to i32*
  ret void
}

; Function Attrs: nounwind readnone
declare spir_func i8* @__to_private(i8 addrspace(4)*) #1

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!opencl.kernels = !{!0, !6, !7}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!8}
!opencl.ocl.version = !{!9}
!opencl.used.extensions = !{!10}
!opencl.used.optional.core.features = !{!10}
!opencl.compiler.options = !{!10}
!llvm.ident = !{!11}

!0 = !{void (i32 addrspace(1)*)* @const_bitcast, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"int*"}
!4 = !{!"kernel_arg_base_type", !"int*"}
!5 = !{!"kernel_arg_type_qual", !""}
!6 = !{void (i32 addrspace(1)*)* @const_gep, !1, !2, !3, !4, !5}
!7 = !{void (i32 addrspace(1)*)* @const_select, !1, !2, !3, !4, !5}
!8 = !{i32 1, i32 2}
!9 = !{i32 2, i32 0}
!10 = !{}
!11 = !{!"clang version 3.6.0 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 83869a5aa2cc8e6efb5dab84d4f034a88fa5515f) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm 50546c308a35b18ee2afb43648a5c2b0e414227f)"}
