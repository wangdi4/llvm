target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

define dso_local spir_func i32 @foo_with_barrier(i32 %m) #0 {
entry:
  %cond = icmp eq i32 %m, 0
  br i1 %cond, label %if.then, label %if.else
if.then:
  ret i32 0
if.else:
  %m.addr = alloca i32, align 4
  store i32 %m, ptr %m.addr, align 4, !tbaa !9
  call spir_func void @_Z7barrierj(i32 1) #4
  %0 = load i32, ptr %m.addr, align 4, !tbaa !9
  %sub1 = sub nsw i32 %0, 1
  %call1 = call spir_func i32 @foo_with_barrier(i32 %sub1) #4
  %sub2 = sub nsw i32 %0, 2
  %call2 = call spir_func i32 @foo_with_barrier(i32 %sub2) #4
  %add = add nsw i32 %call1, %call2
  ret i32 %add
}

declare spir_func void @_Z7barrierj(i32) #1

; Function Attrs: convergent norecurse nounwind
define dso_local spir_kernel void @test(ptr addrspace(1) %m) #2 !kernel_arg_type !22 !kernel_arg_base_type !22 {
entry:
  %call = call spir_func i64 @_Z12get_local_idj(i32 0) #5
  %conv = trunc i64 %call to i32
  %call1 = call spir_func i32 @foo_with_barrier(i32 %conv) #4
  store i32 %call1, ptr addrspace(1) %m, align 4, !tbaa !9
  ret void
}

; Function Attrs: convergent nounwind readnone
declare spir_func i64 @_Z12get_local_idj(i32) #3

attributes #0 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind readnone "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent }
attributes #5 = { convergent nounwind readnone }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!opencl.stat.type = !{!4}
!opencl.stat.exec_time = !{!5}
!opencl.stat.run_time_version = !{!6}
!opencl.stat.workload_name = !{!7}
!opencl.stat.module_name = !{!8}
!spirv.Source = !{!21}
!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"-cl-std=CL2.0"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (2021.2.0.YYYYMMDD)"}
!4 = !{!""}
!5 = !{!"2021-03-11 09:13:31"}
!6 = !{!"2021.11.3.0"}
!7 = !{!"test_opt"}
!8 = !{!"test_opt1"}
!9 = !{!10, !10, i64 0}
!10 = !{!"int", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{i32 1}
!14 = !{!"none"}
!15 = !{!"int*"}
!16 = !{!"m"}
!17 = !{i1 false}
!18 = !{i32 0}
!19 = !{!20, !20, i64 0}
!20 = !{!"any pointer", !11, i64 0}
!21 = !{i32 4, i32 100000}
!22 = !{!"int*"}
