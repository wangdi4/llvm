; __kernel void _addition(__global int* A, __global int* B, __global int* C, __global int* Pred)
; {
;   size_t gid = get_global_id(0);
;   if (Pred[gid])
;     C[gid] = A[gid] - B[gid];
; }

; SPIR was generated with the following invocation:
; clang -cc1 -triple spir64-unknown-unknown -O0 -include opencl-c.h simple_loop.cl -emit-llvm -o simple_loop.ll

; The IR was preprocessed with:
; opt -S -O3 -inline -lowerswitch -scalarize -mergereturn -loop-simplify -phicanon

; Check that all ones bypasses are not insterted.
; RUN: %oclopt -S -predicate -all-ones=false < %s > %t_allones_off.ll
; RUN: %oclopt -S -predicate < %s > %t_allones_on.ll
; RUN: FileCheck %s --input-file=%t_allones_off.ll --check-prefix=CHECK-ALLONESOFF
; RUN: FileCheck %s --input-file=%t_allones_on.ll --check-prefix=CHECK-ALLONESON

; CHECK-ALLONESON: if.then_allOnes
; CHECK-ALLONESOFF-NOT: if.then_allOnes

; ModuleID = '<stdin>'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: nounwind
define spir_kernel void @_addition(i32 addrspace(1)* nocapture readonly %A, i32 addrspace(1)* nocapture readonly %B, i32 addrspace(1)* nocapture %C, i32 addrspace(1)* nocapture readonly %Pred) #0 {
entry:
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0) #2
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %Pred, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %tobool = icmp eq i32 %0, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %arrayidx1 = getelementptr inbounds i32, i32 addrspace(1)* %A, i64 %call
  %1 = load i32, i32 addrspace(1)* %arrayidx1, align 4
  %arrayidx2 = getelementptr inbounds i32, i32 addrspace(1)* %B, i64 %call
  %2 = load i32, i32 addrspace(1)* %arrayidx2, align 4
  %sub = sub nsw i32 %1, %2
  %arrayidx3 = getelementptr inbounds i32, i32 addrspace(1)* %C, i64 %call
  store i32 %sub, i32 addrspace(1)* %arrayidx3, align 4
  br label %if.end

if.end:                                           ; preds = %entry, %if.then
  ret void
}

; Function Attrs: nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) #1

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind readnone }

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!6}
!opencl.spir.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}
!llvm.ident = !{!9}

!0 = !{void (i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*, i32 addrspace(1)*)* @_addition, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 1, i32 1, i32 1, i32 1}
!2 = !{!"kernel_arg_access_qual", !"none", !"none", !"none", !"none"}
!3 = !{!"kernel_arg_type", !"int*", !"int*", !"int*", !"int*"}
!4 = !{!"kernel_arg_base_type", !"int*", !"int*", !"int*", !"int*"}
!5 = !{!"kernel_arg_type_qual", !"", !"", !"", !""}
!6 = !{i32 1, i32 0}
!7 = !{i32 1, i32 2}
!8 = !{}
!9 = !{!"clang version 3.8.1 (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-clang 0a5d1ba6e117013ed9e89050184f535ad554534b) (ssh://nnopencl-git-01.inn.intel.com/home/git/repo/opencl_qa-llvm d8cb065ea5619ebe1ab75ad5771502900422857a)"}
