; This test checks usage of thread-local storage global variables instead of
; implicit arguments to resolve get_global/local_id calls.
;
; The IR is dumped at the beginning of BarrierPass::runOnModule()
; Source:
; size_t foo(uint dim) {
;   return get_local_id(dim);
; }
;
; size_t bar() {
;   return get_global_id(0);
; }
;
; __kernel void test(__global size_t *out) {
;   size_t sum = 0;
;   for (size_t i = 0; i < 3; ++i) {
;     sum += foo(i);
;   }
;   out[bar()] = sum;
; }

; RUN: opt -passes=sycl-kernel-barrier %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-barrier %s -S | FileCheck %s

source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@__LocalIds = internal thread_local global [3 x i64] undef, align 16

; Function Attrs: convergent noinline nounwind
; CHECK: define internal i64 @foo(i32 %dim)
define internal i64 @foo(i32 %dim) #0 {
entry:
; CHECK: get.wi.properties:
; CHECK: %pLocalId_var = getelementptr inbounds [3 x i64], ptr @__LocalIds, i64 0, i32 %dim
; CHECK: %LocalId_var = load i64, ptr %pLocalId_var
  %call = call i64 @_Z12get_local_idj(i32 %dim) #4
  ret i64 %call
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) #1

; Function Attrs: convergent noinline nounwind
; CHECK: define internal i64 @bar()
define internal i64 @bar() #0 {
entry:
; CHECK: %BaseGlobalId_0 = call i64 @get_base_global_id.(i32 0)
; CHECK: %LocalId_0 = load i64, ptr @__LocalIds
; CHECK: %GlobalID_0 = add i64 %LocalId_0, %BaseGlobalId_0

  %call = call i64 @_Z13get_global_idj(i32 0) #4
  ret i64 %call
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) #1

; Function Attrs: convergent noinline nounwind
define void @test(ptr addrspace(1) noalias %out) #2 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !10 !kernel_arg_type_qual !11 !kernel_arg_host_accessible !12 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 !kernel_arg_name !13 !no_barrier_path !12 !kernel_execution_length !14 !kernel_has_barrier !12 !kernel_has_global_sync !12 {
entry:
  call void @dummy_barrier.()
  %bar.addr = alloca ptr addrspace(4), align 8
  store ptr addrspace(4) addrspacecast (ptr @bar to ptr addrspace(4)), ptr %bar.addr, align 8
  br label %for.cond

; CHECK: FirstBB:
; CHECK: store i64 0, ptr @__LocalIds
; CHECK: store i64 0, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 1)
; CHECK: store i64 0, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 2)

; CHECK: SyncBB1:
; CHECK: store ptr addrspace(4) addrspacecast (ptr @bar to ptr addrspace(4))

for.cond:                                         ; preds = %for.body, %entry
  %0 = phi i64 [ 0, %entry ], [ %add, %for.body ]
  %1 = phi i64 [ 0, %entry ], [ %inc, %for.body ]
  %cmp = icmp ult i64 %1, 3
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %conv = trunc i64 %1 to i32
  %call = call i64 @foo(i32 %conv) #3
  %add = add i64 %0, %call
  %inc = add i64 %1, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %call1 = call i64 @bar() #3
  %arrayidx = getelementptr inbounds i64, ptr addrspace(1) %out, i64 %call1
  store i64 %0, ptr addrspace(1) %arrayidx, align 8
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %for.end
  call void @_Z18work_group_barrierj(i32 1)
  ret void

; CHECK: "Barrier BB":
; CHECK: %LocalId_0 = load i64, ptr @__LocalIds
; CHECK: %{{.*}} = add nuw i64 %LocalId_0, 1
; CHECK: store i64 %{{.*}}, ptr @__LocalIds


; CHECK: LoopEnd_0:
; CHECK:   store i64 0, ptr @__LocalIds
; CHECK:   %LocalId_1 = load i64, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 1)
; CHECK:   %{{.*}} = add nuw i64 %LocalId_1, 1
; CHECK:   store i64 %{{.*}}, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 1)

; CHECK: LoopEnd_1:
; CHECK:   store i64 0, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 1)
; CHECK:   %LocalId_2 = load i64, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 2)
; CHECK:   %{{.*}} = add nuw i64 %LocalId_2, 1
; CHECK:   store i64 %{{.*}}, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 2)

; CHECK: LoopEnd_2:
; CHECK:   store i64 0, ptr getelementptr inbounds ([3 x i64], ptr @__LocalIds, i64 0, i32 2)

}

declare void @dummy_barrier.()

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) #3

attributes #0 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent }
attributes #4 = { convergent nounwind readnone }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!sycl.kernels = !{!5}
!opencl.global_variable_total_size = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"-cl-opt-disable"}
!4 = !{!"icx (ICX) dev.8.x.0"}
!5 = !{ptr @test}
!6 = !{i32 0}
!7 = !{i32 1}
!8 = !{!"none"}
!9 = !{!"size_t*"}
!10 = !{!"ulong*"}
!11 = !{!""}
!12 = !{i1 false}
!13 = !{!"out"}
!14 = !{i32 120}

;; get_global_id resolve
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function bar -- %BaseGlobalId_0 = call i64 @get_base_global_id.(i32 0)
;; barrier key values
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %pSB = call ptr @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function test -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)

; DEBUGIFY-NOT: WARNING
