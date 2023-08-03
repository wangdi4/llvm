; RUN: opt -passes=sycl-kernel-coerce-types -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes=sycl-kernel-coerce-types -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

%struct.B = type { %struct.F, %struct.F }
%struct.F = type { i32, i64 }
; CHECK: %struct.F.coerce.0 = type { i64, i64 }
; CHECK-NEXT: %struct.F.coerce = type { i64, i64 }

@__const.test.b = private unnamed_addr addrspace(2) constant %struct.B { %struct.F { i32 1, i64 2 }, %struct.F { i32 3, i64 4 } }, align 8
@__const.test.f = private unnamed_addr addrspace(2) constant %struct.F { i32 1, i64 2 }, align 8

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @foo1(ptr byval(%struct.B) align 8 %b) #0 {
  ret void
}

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @foo2(ptr byval(%struct.F) align 8 %f, ptr byval(%struct.B) align 8 %b) #0 {
  ret void
}

; Function Attrs: convergent noinline norecurse nounwind
define dso_local void @foo3(ptr byval(<4 x i64>) align 32 %0) #0 {
  ret void
}

; Function Attrs: convergent norecurse nounwind
define dso_local void @test() #1 !kernel_arg_addr_space !1 !kernel_arg_access_qual !1 !kernel_arg_type !1 !kernel_arg_base_type !1 !kernel_arg_type_qual !1 !kernel_arg_name !1 !kernel_arg_host_accessible !1 !kernel_arg_pipe_depth !1 !kernel_arg_pipe_io !1 !kernel_arg_buffer_location !1 {
entry:
; CHECK: [[ALLOCA0:%[a-zA-Z0-9]+]] = alloca %struct.B
; CHECK-NEXT: [[ALLOCA1:%[a-zA-Z0-9]+]] = alloca %struct.B
  %b = alloca %struct.B, align 8
  %f = alloca %struct.F, align 8
  %call = call i64 @_Z13get_global_idj(i32 0) #5
  %cmp = icmp eq i64 %call, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry


; CHECK: [[GEP0:%[a-zA-Z0-9]+]] = getelementptr inbounds %struct.B, ptr [[ALLOCA0]]
; CHECK-NEXT: call void @llvm.memcpy.p0.p0.i64(ptr align 8 [[GEP0]], ptr align 8 [[ALLOCA1]], i64 32, i1 false)

  call void @llvm.memcpy.p0.p2.i64(ptr align 8 %b, ptr addrspace(2) align 8 @__const.test.b, i64 32, i1 false)
  call void @llvm.memcpy.p0.p2.i64(ptr align 8 %f, ptr addrspace(2) align 8 @__const.test.f, i64 16, i1 false)
  call void @foo2(ptr byval(%struct.F) align 8 %f, ptr byval(%struct.B) align 8 %b) #7
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  ret void
}

; CHECK: define void @foo1(ptr %b)
; CHECK: define void @foo2(i64 %f.coerce.high, i64 %f.coerce.low, ptr %b)
; CHECK: define void @foo3(ptr %0)

; Function Attrs: convergent nounwind readnone willreturn
declare i64 @_Z13get_global_idj(i32) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.memcpy.p0.p2.i64(ptr noalias nocapture writeonly, ptr addrspace(2) noalias nocapture readonly, i64, i1 immarg) #3

attributes #0 = { convergent noinline norecurse nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #1 = { convergent norecurse nounwind "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" "uniform-work-group-size"="true" }
attributes #2 = { convergent nounwind readnone willreturn "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #3 = { argmemonly nofree nosync nounwind willreturn }
attributes #4 = { convergent "frame-pointer"="none" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "stackrealign" }
attributes #5 = { convergent nounwind readnone willreturn }
attributes #6 = { nounwind }
attributes #7 = { convergent }

!llvm.linker.options = !{}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!0}
!opencl.spir.version = !{!0}
!opencl.used.extensions = !{!1}
!opencl.used.optional.core.features = !{!1}
!opencl.compiler.options = !{!1}
!llvm.ident = !{!2}
!sycl.kernels = !{!3}

!0 = !{i32 1, i32 2}
!1 = !{}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.2.0 (2021.x.0.YYYYMMDD)"}
!3 = !{ptr @test}
!4 = !{!5, !7, i64 0}
!5 = !{!"B", !6, i64 0, !6, i64 16}
!6 = !{!"F", !7, i64 0, !10, i64 8}
!7 = !{!"int", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C/C++ TBAA"}
!10 = !{!"long", !8, i64 0}
!11 = !{!5, !10, i64 8}
!12 = !{!5, !7, i64 16}
!13 = !{!5, !10, i64 24}

; DEBUGIFY-NOT: WARNING
