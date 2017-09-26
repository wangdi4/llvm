; This IR was generated from the following source code:
; __kernel void
; producer (write_only pipe int __attribute__((blocking)) c0) {
;     for (int i = 0; i < 10; i++) {
;         write_pipe( c0, &i  );
;     }
; }
;
; __kernel void
; consumer (__global int * restrict dst,
;         read_only pipe int __attribute__((blocking)) __attribute__((depth(10))) c0) {
;     for (int i = 0; i < 5; i++) {
;         read_pipe( c0, &dst[i]  );
;     }
; }

; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -spir-materializer -verify -S %s | FileCheck %s

%opencl.pipe_t = type opaque

; Function Attrs: noinline nounwind optnone
define spir_kernel void @producer(%opencl.pipe_t addrspace(1)* %c0) #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 {
entry:
  %c0.addr = alloca %opencl.pipe_t addrspace(1)*, align 4
  %i = alloca i32, align 4
  store %opencl.pipe_t addrspace(1)* %c0, %opencl.pipe_t addrspace(1)** %c0.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %c0.addr, align 4
  %2 = addrspacecast i32* %i to i8 addrspace(4)*
  %3 = call i32 @__write_pipe_2_bl(%opencl.pipe_t addrspace(1)* %1, i8 addrspace(4)* %2, i32 4, i32 4)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %4 = load i32, i32* %i, align 4
  %inc = add nsw i32 %4, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}
; CHECK: define void @producer
; CHECK: for.body:
; CHECK: %{{[0-9]+}} = call i32 @__write_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}})

declare i32 @__write_pipe_2_bl(%opencl.pipe_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

; Function Attrs: noinline nounwind optnone
define spir_kernel void @consumer(i32 addrspace(1)* noalias %dst, %opencl.pipe_t addrspace(1)* %c0) #0 !kernel_arg_addr_space !8 !kernel_arg_access_qual !9 !kernel_arg_type !10 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 {
entry:
  %dst.addr = alloca i32 addrspace(1)*, align 4
  %c0.addr = alloca %opencl.pipe_t addrspace(1)*, align 4
  %i = alloca i32, align 4
  store i32 addrspace(1)* %dst, i32 addrspace(1)** %dst.addr, align 4
  store %opencl.pipe_t addrspace(1)* %c0, %opencl.pipe_t addrspace(1)** %c0.addr, align 4
  store i32 0, i32* %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %0 = load i32, i32* %i, align 4
  %cmp = icmp slt i32 %0, 5
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %1 = load %opencl.pipe_t addrspace(1)*, %opencl.pipe_t addrspace(1)** %c0.addr, align 4
  %2 = load i32 addrspace(1)*, i32 addrspace(1)** %dst.addr, align 4
  %3 = load i32, i32* %i, align 4
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %2, i32 %3
  %4 = addrspacecast i32 addrspace(1)* %arrayidx to i8 addrspace(4)*
  %5 = call i32 @__read_pipe_2_bl(%opencl.pipe_t addrspace(1)* %1, i8 addrspace(4)* %4, i32 4, i32 4)
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %6 = load i32, i32* %i, align 4
  %inc = add nsw i32 %6, 1
  store i32 %inc, i32* %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  ret void
}
; CHECK: define void @consumer
; CHECK: entry:
; CHECK: %{{[0-9]+}} = {{.*}}call i32 @__read_pipe_2_bl_intel(%struct.__pipe_t addrspace(1)* %{{[0-9]+}}, i8 addrspace(4)* %{{[0-9]+}})

declare i32 @__read_pipe_2_bl(%opencl.pipe_t addrspace(1)*, i8 addrspace(4)*, i32, i32)

attributes #0 = { noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"clang version 5.0.0 (cfe/trunk)"}
!4 = !{i32 1}
!5 = !{!"write_only"}
!6 = !{!"int"}
!7 = !{!"pipe"}
!8 = !{i32 1, i32 1}
!9 = !{!"none", !"read_only"}
!10 = !{!"int*", !"int"}
!11 = !{!"restrict", !"pipe"}
