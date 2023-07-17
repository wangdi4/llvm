; RUN: opt -passes='debugify,sycl-kernel-sg-emu-loop-construct,check-debugify' -S %s -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-sg-emu-loop-construct' -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent noinline norecurse nounwind
define i64 @lid() #0 {
entry:
  %call = call i64 @_Z12get_local_idj(i32 0) #6
  ret i64 %call
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) #1

; Function Attrs: convergent noinline norecurse nounwind
declare void @test1after_value_widen() #2

; Function Attrs: convergent
declare i32 @_Z18get_sub_group_sizev() #3

; Function Attrs: convergent noinline norecurse nounwind
define void @test2() #2 !kernel_has_sub_groups !6 {
entry:
  call void @dummybarrier.()
; CHECK: call i64 @lid(i32 0)
  %call = call i64 @lid() #4
  call void @_Z7barrierj(i32 1)
  ret void
}

declare i64 @_Z14get_local_sizej(i32)

declare void @dummybarrier.()

; Function Attrs: convergent
declare void @_Z7barrierj(i32) #4

declare void @_Z17sub_group_barrierj(i32)

declare void @dummy_sg_barrier()

; Function Attrs: convergent noinline norecurse nounwind
define void @test1() #5 !kernel_has_sub_groups !9 !sg_emu_size !11 {
sg.loop.exclude:
  call void @dummybarrier.()
  br label %entry

entry:                                            ; preds = %sg.loop.exclude
  br label %sg.dummy.bb.

sg.dummy.bb.:                                     ; preds = %entry
  call void @dummy_sg_barrier()
; CHECK: call i64 @lid(i32 %sg.lid.[[#SUFFIX:]])
  %call = call i64 @lid() #4
  %0 = call i64 @_Z14get_local_sizej(i32 0)
  %minus.vf = sub i64 0, 16
  %uniform.id.max = and i64 %minus.vf, %0
  %nonuniform.size = sub i64 %0, %uniform.id.max
  %1 = call i64 @_Z12get_local_idj(i32 0)
  %2 = icmp ult i64 %1, %uniform.id.max
  %3 = select i1 %2, i64 16, i64 %nonuniform.size
  %subgroup.size = trunc i64 %3 to i32
  br label %sg.barrier.bb.

sg.barrier.bb.:                                   ; preds = %sg.dummy.bb.
  call void @_Z17sub_group_barrierj(i32 1)
  call void @_Z7barrierj(i32 1)
  br label %sg.dummy.bb.1

sg.dummy.bb.1:                                    ; preds = %sg.barrier.bb.
  call void @dummy_sg_barrier()
  ret void
}

attributes #0 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { convergent nounwind readnone "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent }
attributes #5 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN16_test1" }
attributes #6 = { convergent nounwind readnone }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!opencl.stats.InstCounter.CantVectNonInlineUnsupportedFunctions = !{!4}
!sycl.kernels = !{!5}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"-cl-std=CL2.0", !"-cl-opt-disable"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (YYYY.x.0.MMDD)"}
!4 = !{!"Unable to vectorize because of calls to functions that can't be inlined"}
!5 = !{ptr @test2, ptr @test1}
!6 = !{i1 false}
!7 = !{i32 2}
!8 = !{i32 1}
!9 = !{i1 true}
!10 = !{i32 10}
!11 = !{i32 16}

; DEBUGIFY-NOT: WARNING
