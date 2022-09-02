; RUN: opt -passes=dpcpp-kernel-barrier -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -dpcpp-kernel-barrier -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=dpcpp-kernel-barrier -S %s | FileCheck %s
; RUN: opt -dpcpp-kernel-barrier -S %s | FileCheck %s
; Checks the uniform result of work_group_reduce_add will be stored into the alloca (Group-B.2: cross-barrier usage).
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent
declare i32 @_Z21work_group_reduce_addj(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #2

; Function Attrs: convergent norecurse nounwind
define dso_local void @test(i32 addrspace(1)* noalias %m) !vectorized_kernel !15 {
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) local_unnamed_addr #4

; Function Attrs: convergent noinline norecurse nounwind
define dso_local fastcc <4 x i32> @_ZGVbN4v_foo(<4 x i32> %a) unnamed_addr #0 !vectorized_kernel !6 !vectorized_width !8 !vectorization_dimension !14 {
entry:
  call void @dummy_barrier.()
  %AllocaWGResult = alloca <4 x i32>, align 16
  store <4 x i32> zeroinitializer, <4 x i32>* %AllocaWGResult, align 16
  br label %"Barrier BB5"

"Barrier BB5":                                    ; preds = %entry
  call void @dummy_barrier.()
  %0 = add <4 x i32> %a, <i32 0, i32 1, i32 2, i32 3>
  %CallWGForItem = call <4 x i32> @_Z21work_group_reduce_addDv4_jPS_(<4 x i32> %0, <4 x i32>* %AllocaWGResult) #15
  br label %"Barrier BB3"

"Barrier BB3":                                    ; preds = %"Barrier BB5"
  call void @_Z18work_group_barrierj(i32 1)
  %LoadWGFinalResult = load <4 x i32>, <4 x i32>* %AllocaWGResult, align 16
  %CallFinalizeWG = call <4 x i32> @_Z32__finalize_work_group_reduce_addDv4_j(<4 x i32> %LoadWGFinalResult)
; CHECK-LABEL: define dso_local fastcc <4 x i32> @_ZGVbN4v_foo
; CHECK: [[FINALIZE_RES:%.*]] = call <4 x i32> @_Z32__finalize_work_group_reduce_addDv4_j
; CHECK-NEXT: store <4 x i32> [[FINALIZE_RES]], <4 x i32>* [[FINALIZE_RES_ADDR:%.*]], align 16
  store <4 x i32> zeroinitializer, <4 x i32>* %AllocaWGResult, align 16
  br label %"Barrier BB4"

"Barrier BB4":                                    ; preds = %"Barrier BB3"
  call void @dummy_barrier.()
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB4"
  call void @_Z18work_group_barrierj(i32 1)
; CHECK-LABEL: RetBB:
; CHECK-NEXT: [[LOADED2:%.*]] = load <4 x i32>, <4 x i32>* [[FINALIZE_RES_ADDR]]
; CHECK-NEXT: ret <4 x i32> [[LOADED2]]
  ret <4 x i32> %CallFinalizeWG
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @_ZGVbN4u_test(i32 addrspace(1)* noalias %m) local_unnamed_addr #3 !no_barrier_path !9 !vectorized_kernel !6 !scalar_kernel !4 !vectorized_width !8 !kernel_execution_length !18 !kernel_has_barrier !9 !vectorization_dimension !14 {
entry:
  call void @dummy_barrier.()
  %call = tail call i64 @_Z12get_local_idj(i32 0) #16
  %0 = trunc i64 %call to i32
  %broadcast.splatinsert = insertelement <4 x i32> poison, i32 %0, i32 0
  %broadcast.splat = shufflevector <4 x i32> %broadcast.splatinsert, <4 x i32> poison, <4 x i32> zeroinitializer
  %1 = add nuw <4 x i32> %broadcast.splat, <i32 0, i32 1, i32 2, i32 3>
  br label %"Barrier BB2"

"Barrier BB2":                                    ; preds = %entry
  call void @_Z18work_group_barrierj(i32 1)
  %2 = call fastcc <4 x i32> @_ZGVbN4v_foo(<4 x i32> %1) #11
  br label %"Barrier BB3"

"Barrier BB3":                                    ; preds = %"Barrier BB2"
  call void @dummy_barrier.()
  %.extract.3. = extractelement <4 x i32> %2, i32 3
  store i32 %.extract.3., i32 addrspace(1)* %m, align 4
  br label %"Barrier BB"

"Barrier BB":                                     ; preds = %"Barrier BB3"
  call void @_Z18work_group_barrierj(i32 1)
  ret void
}

; Function Attrs: argmemonly nofree nosync nounwind readonly willreturn
declare <4 x i32> @llvm.masked.load.v4i32.p0v4i32(<4 x i32>*, i32 immarg, <4 x i1>, <4 x i32>) #6

; Function Attrs: convergent
declare <4 x i32> @_Z21work_group_reduce_addDv4_jS_(<4 x i32>, <4 x i32>) local_unnamed_addr #1

; Function Attrs: argmemonly nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.store.v4i32.p0v4i32(<4 x i32>, <4 x i32>*, i32 immarg, <4 x i1>) #7

; Function Attrs: convergent
declare <4 x i32> @_Z21work_group_reduce_addDv4_j(<4 x i32>) local_unnamed_addr #1

; Function Attrs: nofree nosync nounwind willreturn writeonly
declare void @llvm.masked.scatter.v4i32.v4p1i32(<4 x i32>, <4 x i32 addrspace(1)*>, i32 immarg, <4 x i1>) #8

; Function Attrs: nounwind readnone
declare i32 @_Z18get_sub_group_sizev() #9

declare i64 @_Z14get_local_sizej(i32)

declare void @dummy_barrier.()

; Function Attrs: nofree norecurse nounwind willreturn
declare i32 @_Z21work_group_reduce_addjPj(i32, i32* nocapture) #10

; Function Attrs: convergent
declare void @_Z18work_group_barrierj(i32) #11

; Function Attrs: nofree norecurse nounwind willreturn
declare <4 x i32> @_Z21work_group_reduce_addDv4_jS_PS_(<4 x i32>, <4 x i32>, <4 x i32>* nocapture) #12

; Function Attrs: norecurse nounwind readnone willreturn
declare <4 x i32> @_Z32__finalize_work_group_reduce_addDv4_jS_(<4 x i32>, <4 x i32>) #13

; Function Attrs: nofree norecurse nounwind willreturn
declare <4 x i32> @_Z21work_group_reduce_addDv4_jPS_(<4 x i32>, <4 x i32>* nocapture) #12

; Function Attrs: norecurse nounwind readnone willreturn
declare <4 x i32> @_Z32__finalize_work_group_reduce_addDv4_j(<4 x i32>) #13

attributes #0 = { convergent noinline norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "has-sub-groups" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbM4v_foo,_ZGVbN4v_foo" }
attributes #1 = { convergent "disable-tail-calls"="false" "frame-pointer"="none" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "opencl-vec-uniform-return" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent norecurse nounwind "disable-tail-calls"="false" "frame-pointer"="none" "has-sub-groups" "kernel-call-once" "kernel-convergent-call" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVbN4u_test,_ZGVbM4u_test" }
attributes #4 = { convergent nounwind readnone "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind }
attributes #6 = { argmemonly nofree nosync nounwind readonly willreturn }
attributes #7 = { argmemonly nofree nosync nounwind willreturn writeonly }
attributes #8 = { nofree nosync nounwind willreturn writeonly }
attributes #9 = { nounwind readnone }
attributes #10 = { nofree norecurse nounwind willreturn "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #11 = { convergent }
attributes #12 = { nofree norecurse nounwind willreturn "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #13 = { norecurse nounwind readnone willreturn "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="128" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "target-cpu"="corei7" "target-features"="+cx16,+cx8,+fxsr,+mmx,+popcnt,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #14 = { convergent nounwind }
attributes #15 = { convergent nounwind "kernel-call-once" "kernel-convergent-call" }
attributes #16 = { convergent nounwind readnone }
attributes #17 = { convergent nounwind "call-params-num"="1" "kernel-call-once" "kernel-convergent-call" }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}
!sycl.kernels = !{!4}

!0 = !{i32 2, i32 0}
!1 = !{}
!2 = !{!"-cl-std=CL2.0"}
!3 = !{!"Intel(R) oneAPI DPC++ Compiler 2021.2.0 (2021.2.0.YYYYMMDD)"}
!4 = !{void (i32 addrspace(1)*)* @test}
!5 = !{<4 x i32> (<4 x i32>)* @_ZGVbN4v_foo}
!6 = !{null}
!7 = !{i32 1}
!8 = !{i32 4}
!9 = !{i1 false}
!10 = !{!"none"}
!11 = !{!"int*"}
!12 = !{!""}
!13 = !{!"m"}
!14 = !{i32 0}
!15 = !{void (i32 addrspace(1)*)* @_ZGVbN4u_test}
!16 = !{i1 true}
!17 = !{i32 5}
!18 = !{i32 9}

;; alloca for uniform cross-barrier value
;DEBUGIFY:  WARNING: Instruction with empty DebugLoc in function _ZGVbN4v_foo -- %CallWGForItem5 = alloca <4 x i32>, align 16
;; barrier key values
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4v_foo -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4v_foo -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4v_foo -- %pLocalIds = alloca [3 x i64], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4v_foo -- %pSB = call i8* @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4v_foo -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4v_foo -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4v_foo -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)
;; argument
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4v_foo -- %loadedValue = load <4 x i32>, <4 x i32>* %pSB_LocalId, align 16
;; barrier key values
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_test -- %pCurrBarrier = alloca i32, align 4
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_test -- %pCurrSBIndex = alloca i64, align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_test -- %pLocalIds = alloca [3 x i64], align 8
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_test -- %pSB = call i8* @get_special_buffer.()
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_test -- %LocalSize_0 = call i64 @_Z14get_local_sizej(i32 0)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_test -- %LocalSize_1 = call i64 @_Z14get_local_sizej(i32 1)
;DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _ZGVbN4u_test -- %LocalSize_2 = call i64 @_Z14get_local_sizej(i32 2)

; DEBUGIFY-NOT: WARNING
