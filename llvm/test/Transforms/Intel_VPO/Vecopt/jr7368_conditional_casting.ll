; RUN: opt -S -mem2reg -loop-simplify -lcssa -vpo-cfg-restructuring -vplan-vec %s | FileCheck %s
; RUN: opt -S -passes="mem2reg,loop-simplify,lcssa,vpo-cfg-restructuring,vplan-vec" %s | FileCheck %s
; CHECK-LABEL: vector.body
; ModuleID = 'main'
source_filename = "1"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@gint = addrspace(1) global i32 1, align 4
@testKernel.lint = internal addrspace(3) global i32 undef, align 4

; Function Attrs: convergent noduplicate nounwind
define void @testKernel(i32 addrspace(1)* noalias %results) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !kernel_arg_name !12 !vectorized_kernel !13 !no_barrier_path !11 !vectorized_width !7 !scalarized_kernel !14 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #4
  store i32 2, i32 addrspace(3)* @testKernel.lint, align 4, !tbaa !15
  %rem14 = and i64 %call, 1
  %tobool = icmp eq i64 %rem14, 0
  %.gint = select i1 %tobool, i32 addrspace(1)* addrspacecast (i32 addrspace(3)* @testKernel.lint to i32 addrspace(1)*), i32 addrspace(1)* @gint
  tail call void @_Z7barrierj(i32 2) #5
  br i1 %tobool, label %land.lhs.true13, label %land.lhs.true

land.lhs.true:                                    ; preds = %entry
  br i1 icmp eq (i8 addrspace(1)* bitcast (i32 addrspace(1)* @gint to i8 addrspace(1)*), i8 addrspace(1)* null), label %land.end, label %land.rhs

land.rhs:                                         ; preds = %land.lhs.true
  %0 = load i32, i32 addrspace(1)* %.gint, align 4
  %cmp = icmp eq i32 %0, 1
  %phitmp11 = zext i1 %cmp to i32
  br label %land.end

land.end:                                         ; preds = %land.lhs.true, %land.rhs
  %1 = phi i32 [ 0, %land.lhs.true ], [ %phitmp11, %land.rhs ]
  %idxprom = and i64 %call, 4294967295
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %results, i64 %idxprom
  store i32 %1, i32 addrspace(1)* %arrayidx, align 4, !tbaa !15
  br label %if.end22

land.lhs.true13:                                  ; preds = %entry
  br i1 icmp eq (i8 addrspace(3)* bitcast (i32 addrspace(3)* @testKernel.lint to i8 addrspace(3)*), i8 addrspace(3)* null), label %land.end18, label %land.rhs15

land.rhs15:                                       ; preds = %land.lhs.true13
  %2 = load i32, i32 addrspace(1)* %.gint, align 4
  %cmp16 = icmp eq i32 %2, 2
  %phitmp = zext i1 %cmp16 to i32
  br label %land.end18

land.end18:                                       ; preds = %land.lhs.true13, %land.rhs15
  %3 = phi i32 [ 0, %land.lhs.true13 ], [ %phitmp, %land.rhs15 ]
  %idxprom20 = and i64 %call, 4294967295
  %arrayidx21 = getelementptr inbounds i32, i32 addrspace(1)* %results, i64 %idxprom20
  store i32 %3, i32 addrspace(1)* %arrayidx21, align 4, !tbaa !15
  br label %if.end22

if.end22:                                         ; preds = %land.end18, %land.end
  ret void
}

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32) local_unnamed_addr #1

; Function Attrs: convergent noduplicate
declare void @_Z7barrierj(i32) local_unnamed_addr #2

; Function Attrs: convergent noduplicate nounwind
define void @_ZGVdN8u_testKernel(i32 addrspace(1)* noalias %results) local_unnamed_addr #0 !kernel_arg_addr_space !7 !kernel_arg_access_qual !8 !kernel_arg_type !9 !kernel_arg_base_type !9 !kernel_arg_type_qual !10 !kernel_arg_host_accessible !11 !kernel_arg_pipe_depth !6 !kernel_arg_pipe_io !10 !kernel_arg_buffer_location !10 !kernel_arg_name !12 !vectorized_kernel !14 !no_barrier_path !11 !ocl_recommended_vector_length !19 !vectorized_width !19 !vectorization_dimension !6 !scalarized_kernel !5 !can_unite_workgroups !11 {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #4
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 8), "QUAL.OMP.UNIFORM"(i32 addrspace(1)* %results, i32 0, i32 1) ]
  br label %simd.loop

simd.loop:                                        ; preds = %simd.loop.exit, %simd.begin.region
  %index = phi i32 [ 0, %simd.begin.region ], [ %indvar, %simd.loop.exit ]
  %0 = sext i32 %index to i64
  %add = add nuw i64 %0, %call
  store i32 2, i32 addrspace(3)* @testKernel.lint, align 4, !tbaa !15
  %rem14 = and i64 %add, 1
  %tobool = icmp eq i64 %rem14, 0
  %.gint = select i1 %tobool, i32 addrspace(1)* addrspacecast (i32 addrspace(3)* @testKernel.lint to i32 addrspace(1)*), i32 addrspace(1)* @gint
  tail call void @_Z7barrierj(i32 2) #5
  br i1 %tobool, label %land.lhs.true13, label %land.lhs.true

land.lhs.true:                                    ; preds = %simd.loop
  br i1 icmp eq (i8 addrspace(1)* bitcast (i32 addrspace(1)* @gint to i8 addrspace(1)*), i8 addrspace(1)* null), label %land.end, label %land.rhs

land.rhs:                                         ; preds = %land.lhs.true
  %1 = load i32, i32 addrspace(1)* %.gint, align 4
  %cmp = icmp eq i32 %1, 1
  %phitmp11 = zext i1 %cmp to i32
  br label %land.end

land.end:                                         ; preds = %land.rhs, %land.lhs.true
  %2 = phi i32 [ 0, %land.lhs.true ], [ %phitmp11, %land.rhs ]
  %idxprom = and i64 %add, 4294967295
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %results, i64 %idxprom
  store i32 %2, i32 addrspace(1)* %arrayidx, align 4, !tbaa !15
  br label %if.end22

land.lhs.true13:                                  ; preds = %simd.loop
  br i1 icmp eq (i8 addrspace(3)* bitcast (i32 addrspace(3)* @testKernel.lint to i8 addrspace(3)*), i8 addrspace(3)* null), label %land.end18, label %land.rhs15

land.rhs15:                                       ; preds = %land.lhs.true13
  %3 = load i32, i32 addrspace(1)* %.gint, align 4
  %cmp16 = icmp eq i32 %3, 2
  %phitmp = zext i1 %cmp16 to i32
  br label %land.end18

land.end18:                                       ; preds = %land.rhs15, %land.lhs.true13
  %4 = phi i32 [ 0, %land.lhs.true13 ], [ %phitmp, %land.rhs15 ]
  %idxprom20 = and i64 %add, 4294967295
  %arrayidx21 = getelementptr inbounds i32, i32 addrspace(1)* %results, i64 %idxprom20
  store i32 %4, i32 addrspace(1)* %arrayidx21, align 4, !tbaa !15
  br label %if.end22

if.end22:                                         ; preds = %land.end18, %land.end
  br label %simd.loop.exit

simd.loop.exit:                                   ; preds = %if.end22
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 8
  br i1 %vl.cond, label %simd.loop, label %simd.end.region, !llvm.loop !20

simd.end.region:                                  ; preds = %simd.loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"(), "DIR.QUAL.LIST.END"() ]
  br label %return

return:                                           ; preds = %simd.end.region
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { convergent noduplicate nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" "vector-variants"="_ZGVdN8u_testKernel" }
attributes #1 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { convergent noduplicate "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "stackrealign" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { convergent nounwind readnone }
attributes #5 = { convergent nounwind }

!llvm.linker.options = !{}
!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}
!opencl.kernels = !{!5}
!opencl.gen_addr_space_pointer_counter = !{!6}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 2, i32 0}
!2 = !{}
!3 = !{!"-cl-std=CL2.0"}
!4 = !{!"clang version 8.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang 73a7cd4b8b270182f03b0d325c3fd4cd6e6dbf56) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm bee4537ea28bde70841c48e6a4811ac4f86f36d9)"}
!5 = !{void (i32 addrspace(1)*)* @testKernel}
!6 = !{i32 0}
!7 = !{i32 1}
!8 = !{!"none"}
!9 = !{!"uint*"}
!10 = !{!""}
!11 = !{i1 false}
!12 = !{!"results"}
!13 = !{void (i32 addrspace(1)*)* @_ZGVdN8u_testKernel}
!14 = !{null}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !17, i64 0}
!17 = !{!"omnipotent char", !18, i64 0}
!18 = !{!"Simple C/C++ TBAA"}
!19 = !{i32 8}
!20 = distinct !{!20, !21}
!21 = !{!"llvm.loop.unroll.disable"}
