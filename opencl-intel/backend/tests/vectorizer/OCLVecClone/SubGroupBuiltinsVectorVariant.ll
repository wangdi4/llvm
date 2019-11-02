; RUN: %oclopt --ocl-vecclone -VPlanDriver --ocl-vec-clone-isa-encoding-override=AVX512Core -enable-vp-value-codegen=false < %s -S -o - | FileCheck %s
; RUN: %oclopt --ocl-vecclone -VPlanDriver --ocl-vec-clone-isa-encoding-override=AVX512Core -enable-vp-value-codegen=true < %s -S -o - | FileCheck %s

; ModuleID = '<stdin>'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: declare spir_func i32 @_Z13sub_group_alli(i32) local_unnamed_addr #[[SCALAR_ALL_ATTR:.*]]

; Function Attrs: convergent nounwind
define spir_kernel void @a(i32 addrspace(1)* nocapture readonly %a, i32 addrspace(1)* nocapture %b) local_unnamed_addr #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 !ocl_recommended_vector_length !15 {
entry:
; CHECK-LABEL: vector.body
; CHECK: [[VEC_IND:%.*]] = phi <4 x i32> [ <i32 0, i32 1, i32 2, i32 3>, %vector.ph ], [ [[VEC_IND_NEXT:%.*]], %vector.body ]
; CHECK-NEXT: [[VSLID:%.*]] = add nuw <4 x i32> [[VEC_IND]], [[SLID_BROADCAST:%.*]]
;
  %call = tail call spir_func i64 @_Z13get_global_idj(i32 0) #3
  %slid = tail call i32 @_Z22get_sub_group_local_idv() #3
  %slid.reverse = sub nuw i32 16, %slid
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %a, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !10
  %1 = trunc i32 %0 to i16
; CHECK: [[WIDE_LOAD_TRUNCi16:%.*]] = trunc <4 x i32> %wide.load to <4 x i16>
  %2 = trunc i32 %0 to i8
; CHECK: [[WIDE_LOAD_TRUNCi8:%.*]] = trunc <4 x i32> %wide.load to <4 x i8>
  %3 = zext i32 %0 to i64
; CHECK: [[WIDE_LOAD_TRUNCi64:%.*]] = zext <4 x i32> %wide.load to <4 x i64>

  %val = call i32 @_Z23intel_sub_group_shuffleij(i32 %0, i32 %slid.reverse)
  %val2 = call i32 @_Z23intel_sub_group_shufflejj(i32 %0, i32 %slid.reverse)
; CHECK: call <4 x i32> @_Z23intel_sub_group_shuffleDv4_iDv4_jS0_(<4 x i32> %wide.load, <4 x i32> [[SLID_REVERSE:%.*]], <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: call <4 x i32> @_Z23intel_sub_group_shuffleDv4_jS_S_(<4 x i32> %wide.load, <4 x i32> [[SLID_REVERSE]], <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)

  %call1 = tail call spir_func i32 @_Z13sub_group_alli(i32 %0) #4
  %call2 = tail call spir_func i32 @_Z16get_sub_group_idv() #4
; CHECK: [[VECTOR_ALL:%.*]] = call <4 x i32> @_Z13sub_group_allDv4_iDv4_j(<4 x i32> %wide.load, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; TODO: VPValue-based CG does not preserve any attributes for serialized Calls.
; Check JIRA : CMPLRLLVM-10806
; CHECK: [[UNIFORM_SUB_GROUP_ID:%.*]] = {{call|tail call spir_func}} i32 @_Z16get_sub_group_idv()

  %call3 = tail call spir_func i64 @_Z19sub_group_broadcastlj(i64 %3, i32 0) #4
  %call4 = tail call spir_func i32 @_Z19sub_group_broadcastij(i32 %0, i32 0) #4
  %call5 = tail call spir_func i16 @_Z19sub_group_broadcastsj(i16 %1, i32 0) #4
  %call6 = tail call spir_func i8  @_Z19sub_group_broadcastcj(i8 %2, i32 0) #4
; CHECK: = call <4 x i64> @_Z19sub_group_broadcastDv4_ljDv4_j(<4 x i64> [[WIDE_LOAD_TRUNCi64]], i32 0, <4 x i64> <i64 -1, i64 -1, i64 -1, i64 -1>)
; CHECK: = call <4 x i32> @_Z19sub_group_broadcastDv4_ijDv4_j(<4 x i32> %wide.load, i32 0, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: = call <4 x i16> @_Z19sub_group_broadcastDv4_sjDv4_j(<4 x i16> [[WIDE_LOAD_TRUNCi16]], i32 0, <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>)
; CHECK: = call <4 x i8> @_Z19sub_group_broadcastDv4_cjDv4_j(<4 x i8> [[WIDE_LOAD_TRUNCi8]], i32 0, <4 x i8> <i8 -1, i8 -1, i8 -1, i8 -1>)

  %call7 = tail call spir_func i32 @_Z20sub_group_reduce_addi(i32 %0) #4
  %call8 = tail call spir_func i16 @_Z20sub_group_reduce_mins(i16 %1) #4
  %call9 = tail call spir_func i8  @_Z20sub_group_reduce_maxc(i8  %2) #4
; CHECK: = call <4 x i32> @_Z20sub_group_reduce_addDv4_iDv4_j(<4 x i32> %wide.load, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: = call <4 x i16> @_Z20sub_group_reduce_minDv4_sDv4_j(<4 x i16> [[WIDE_LOAD_TRUNCi16]], <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>)
; CHECK: = call <4 x i8>  @_Z20sub_group_reduce_maxDv4_cDv4_j(<4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> <i8 -1, i8 -1, i8 -1, i8 -1>)

  %call10 = tail call spir_func i32 @_Z28sub_group_scan_exclusive_addi(i32 %0) #4
  %call11 = tail call spir_func i16 @_Z28sub_group_scan_exclusive_mins(i16 %1) #4
  %call12 = tail call spir_func i8  @_Z28sub_group_scan_exclusive_maxc(i8  %2) #4
; CHECK: = call <4 x i32> @_Z28sub_group_scan_exclusive_addDv4_iDv4_j(<4 x i32> %wide.load, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: = call <4 x i16> @_Z28sub_group_scan_exclusive_minDv4_sDv4_j(<4 x i16> [[WIDE_LOAD_TRUNCi16]], <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>)
; CHECK: = call <4 x i8> @_Z28sub_group_scan_exclusive_maxDv4_cDv4_j(<4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> <i8 -1, i8 -1, i8 -1, i8 -1>)

  %call13 = tail call spir_func i32 @_Z28sub_group_scan_inclusive_addi(i32 %0) #4
  %call14 = tail call spir_func i16 @_Z28sub_group_scan_inclusive_mins(i16 %1) #4
  %call15 = tail call spir_func i8  @_Z28sub_group_scan_inclusive_maxc(i8  %2) #4
; CHECK: = call <4 x i32> @_Z28sub_group_scan_inclusive_addDv4_iDv4_j(<4 x i32> %wide.load, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)
; CHECK: = call <4 x i16> @_Z28sub_group_scan_inclusive_minDv4_sDv4_j(<4 x i16> [[WIDE_LOAD_TRUNCi16]], <4 x i16> <i16 -1, i16 -1, i16 -1, i16 -1>)
; CHECK: = call <4 x i8> @_Z28sub_group_scan_inclusive_maxDv4_cDv4_j(<4 x i8> [[WIDE_LOAD_TRUNCi8]], <4 x i8> <i8 -1, i8 -1, i8 -1, i8 -1>)

  %call16 = tail call spir_func i64 @_Z28intel_sub_group_shuffle_downllj(i64 %call, i64 42, i32 %slid.reverse)
; CHECK: = call <4 x i64> @_Z28intel_sub_group_shuffle_downDv4_lS_Dv4_jS0_(

  %call17 = tail call spir_func <4 x i32> @_Z28intel_sub_group_shuffle_downDv4_iS_j(<4 x i32> <i32 1, i32 2, i32 3, i32 4>, <4 x i32> <i32 41, i32 42, i32 43, i32 44>, i32 %slid.reverse)
; CHECK: = call <16 x i32> @_Z28intel_sub_group_shuffle_downDv16_iS_Dv4_jS0_(

  %blk_read = call <2 x i32> @_Z27intel_sub_group_block_read2PU3AS1Kj(i32 addrspace(1)* %a)
; CHECK: = call <8 x i32> @_Z29intel_sub_group_block_read2_4PU3AS1Kj(i32 addrspace(1)* [[LOAD_a:%.*]])
  %blk_read.x2 = mul <2 x i32> %blk_read, <i32 2, i32 2>
  call void @_Z28intel_sub_group_block_write2PU3AS1jDv2_j(i32 addrspace(1)* %b, <2 x i32> %blk_read.x2)
; CHECK: call void @_Z30intel_sub_group_block_write2_4PU3AS1jDv8_j(i32 addrspace(1)* [[LOAD_b:%.*]], <8 x i32> {{%.*}})

  %call18 = call <4 x i32> @_Z22intel_sub_group_balloti(i32 %0)
; CHECK: call <16 x i32> @_Z22intel_sub_group_ballotDv4_iDv4_j(<4 x i32> %wide.load, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)

  %tobool = icmp ne i32 %0, 0
  %call19 = call i32 @intel_sub_group_ballot(i1 zeroext %tobool)
; CHECK: call <4 x i32> @intel_sub_group_ballot_vf4(<4 x i1> zeroext {{%.*}}, <4 x i32> <i32 -1, i32 -1, i32 -1, i32 -1>)

  %mul = mul i32 %call4, 1000
  %conv = zext i32 %mul to i64
  %add = add i64 %call, %conv
  %arrayidx4 = getelementptr inbounds i32, i32 addrspace(1)* %b, i64 %add
  store i32 %call1, i32 addrspace(1)* %arrayidx4, align 4, !tbaa !10
; FIXME: Use uniformity/linearity information to not have scatter.
; CHECK: {{call.*.llvm.masked.scatter.*\(|store }}<4 x i32> [[VECTOR_ALL]]

  %cmp = icmp eq i32 %slid, 0
; CHECK: [[VICMP:%.*]] = icmp eq <4 x i32> [[VSLID]], zeroinitializer
; CHECK: [[MASK:%.*]] = xor <4 x i1> [[VICMP]], <i1 true, i1 true, i1 true, i1 true>
; CHECK: [[MASKEXT:%.*]] = sext <4 x i1> [[MASK]] to <4 x i32>
  br i1 %cmp, label %slid.zero, label %slid.nonzero

slid.zero:
  br label %end

slid.nonzero:
  %masked_load = load i32, i32 addrspace(1)* %arrayidx, align 4, !tbaa !10
  %masked_shuffle = call i32 @_Z23intel_sub_group_shuffleij(i32 %0, i32 4)
; CHECK: call <4 x i32> @_Z23intel_sub_group_shuffleDv4_iDv4_jS0_(<4 x i32> %wide.load, <4 x i32> <i32 4, i32 4, i32 4, i32 4>, <4 x i32> [[MASKEXT]])
  br label %end

end:
  ret void
}

; Function Attrs: convergent
declare spir_func i32 @_Z13sub_group_alli(i32) local_unnamed_addr #1

; Function Attrs: convergent
declare spir_func i64 @_Z19sub_group_broadcastlj(i64, i32) local_unnamed_addr #1
declare spir_func i32 @_Z19sub_group_broadcastij(i32, i32) local_unnamed_addr #1
declare spir_func i16 @_Z19sub_group_broadcastsj(i16, i32) local_unnamed_addr #1
declare spir_func i8 @_Z19sub_group_broadcastcj(i8, i32) local_unnamed_addr #1

; Function Attrs: convergent
declare spir_func i32 @_Z20sub_group_reduce_addi(i32) local_unnamed_addr #1
declare spir_func i16 @_Z20sub_group_reduce_mins(i16) local_unnamed_addr #1
declare spir_func i8  @_Z20sub_group_reduce_maxc(i8) local_unnamed_addr #1

; Function Attrs: convergent
declare spir_func i32 @_Z28sub_group_scan_exclusive_addi(i32) local_unnamed_addr #1
declare spir_func i16 @_Z28sub_group_scan_exclusive_mins(i16) local_unnamed_addr #1
declare spir_func i8  @_Z28sub_group_scan_exclusive_maxc(i8) local_unnamed_addr #1

; Function Attrs: convergent
declare spir_func i32 @_Z28sub_group_scan_inclusive_addi(i32) local_unnamed_addr #1
declare spir_func i16 @_Z28sub_group_scan_inclusive_mins(i16) local_unnamed_addr #1
declare spir_func i8  @_Z28sub_group_scan_inclusive_maxc(i8) local_unnamed_addr #1

; Function Attrs: convergent nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) local_unnamed_addr #2

; Function Attrs: convergent
declare spir_func i32 @_Z16get_sub_group_idv() local_unnamed_addr #1

; Function Attrs: convergent
declare <4 x i32> @_Z22intel_sub_group_balloti(i32 %0) local_unnamed_addr #1

; Function Attrs: convergent
declare i32 @intel_sub_group_ballot(i1 zeroext) local_unnamed_addr #1

declare spir_func i32 @_Z23intel_sub_group_shuffleij(i32, i32) local_unnamed_addr #1
declare spir_func i32 @_Z23intel_sub_group_shufflejj(i32, i32) local_unnamed_addr #1
declare spir_func i32 @_Z22get_sub_group_local_idv() local_unnamed_addr #1
declare spir_func i64 @_Z28intel_sub_group_shuffle_downllj(i64, i64, i32) local_unnamed_addr #1

declare spir_func <4 x i32> @_Z28intel_sub_group_shuffle_downDv4_iS_j(<4 x i32>, <4 x i32>, i32) local_unnamed_addr #1

declare <2 x i32> @_Z27intel_sub_group_block_read2PU3AS1Kj(i32 addrspace(1)*) local_unnamed_addr #1
declare void @_Z28intel_sub_group_block_write2PU3AS1jDv2_j(i32 addrspace(1)*, <2 x i32>) local_unnamed_addr #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #1 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false"  "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #2 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false"  "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87"}
attributes #3 = { convergent nounwind readnone }
attributes #4 = { convergent nounwind }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}

!opencl.kernels = !{!14}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!4 = !{i32 1, i32 1}
!5 = !{!"none", !"none"}
!6 = !{!"int*", !"int*"}
!7 = !{!"", !""}
!8 = !{i1 false, i1 false}
!9 = !{i32 0, i32 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{void (i32 addrspace(1)*, i32 addrspace(1)*)* @a}
!15 = !{i32 4}
