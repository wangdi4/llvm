; RUN: %oclopt -sub-group-adaptation -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -sub-group-adaptation -verify -S < %s | FileCheck %s
;;*****************************************************************************
;; This test checks the SubGroupAdaptation pass for OpenCL 1.2.
;;    get_max_sub_group_size
;;*****************************************************************************
;; Source code and clang options:
;; clang -cc1 -x cl -cl-std=CL1.2 -triple spir64 -emit-llvm -disable-llvm-passes -finclude-default-header
;; __kernel void test(__global uint* result) {
;;    size_t gid = get_global_id(0);
;;      result[gid] = get_max_sub_group_size();
;; }
;;*****************************************************************************
; ModuleID = 'get_max_sub_group_size.cl'
source_filename = "get_max_sub_group_size.cl"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64"

; Function Attrs: convergent nounwind
define spir_kernel void @test(i32 addrspace(1)* %result) #0 !kernel_arg_addr_space !4 !kernel_arg_access_qual !5 !kernel_arg_type !6 !kernel_arg_base_type !6 !kernel_arg_type_qual !7 !kernel_arg_host_accessible !8 !kernel_arg_pipe_depth !9 !kernel_arg_pipe_io !7 !kernel_arg_buffer_location !7 {
entry:
  %result.addr = alloca i32 addrspace(1)*, align 8
  %gid = alloca i64, align 8
  store i32 addrspace(1)* %result, i32 addrspace(1)** %result.addr, align 8, !tbaa !10
  %0 = bitcast i64* %gid to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #4
  %call = call spir_func i64 @_Z13get_global_idj(i32 0) #5
  store i64 %call, i64* %gid, align 8, !tbaa !14
  %call1 = call spir_func i32 @_Z22get_max_sub_group_sizev() #6
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %result.addr, align 8, !tbaa !10
  %2 = load i64, i64* %gid, align 8, !tbaa !14
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %1, i64 %2
  store i32 %call1, i32 addrspace(1)* %arrayidx, align 4, !tbaa !16
  %3 = bitcast i64* %gid to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %3) #4
  ret void
}

; CHECK-LABEL: @_Z22get_max_sub_group_sizev
; CHECK: entry:
; CHECK: %lsz0 = call spir_func i64 @_Z14get_local_sizej(i32 0)
; CHECK: %lsz1 = call spir_func i64 @_Z14get_local_sizej(i32 1)
; CHECK: %lsz2 = call spir_func i64 @_Z14get_local_sizej(i32 2)
; CHECK: %op0 = mul i64 %lsz0, %lsz1
; CHECK: %res = mul i64 %op0, %lsz2
; CHECK: %cast = trunc i64 %res to i32
; CHECK: ret i32 %cast

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: convergent nounwind readnone
declare spir_func i64 @_Z13get_global_idj(i32) #2

; Function Attrs: convergent
declare spir_func i32 @_Z22get_max_sub_group_sizev() #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind readnone }
attributes #6 = { convergent }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 2}
!2 = !{}
!3 = !{!"clang version 8.0.0"}
!4 = !{i32 1}
!5 = !{!"none"}
!6 = !{!"uint*"}
!7 = !{!""}
!8 = !{i1 false}
!9 = !{i32 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"any pointer", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = !{!15, !15, i64 0}
!15 = !{!"long", !12, i64 0}
!16 = !{!17, !17, i64 0}
!17 = !{!"int", !12, i64 0}

; Instructions in two functions are added to replace one originl instruction. No DebugLoc.
; dbbugify count is used because other check detail warning check does not work here.
; DEBUGIFY-COUNT-7: WARNING:
