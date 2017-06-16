; ModuleID = 'mybi.cl'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @testcase(double addrspace(1)* nocapture %pl, double addrspace(1)* nocapture %pr, double addrspace(1)* nocapture %res) nounwind {
entry:
  %call = tail call i64 @get_global_id(i32 0) nounwind readnone
  %arrayidx = getelementptr inbounds double addrspace(1)* %pl, i64 %call
  %0 = load double addrspace(1)* %arrayidx, align 8, !tbaa !3
  %call3 = tail call double @_Z3mixddd(double %0, double %0, double %0) nounwind readnone
  %arrayidx5 = getelementptr inbounds double addrspace(1)* %res, i64 %call
  store double %call3, double addrspace(1)* %arrayidx5, align 8, !tbaa !3
  ret void
}

declare i64 @get_global_id(i32) nounwind readnone

declare double @_Z3mixddd(double, double, double) nounwind readnone

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = !{void (double addrspace(1)*, double addrspace(1)*, double addrspace(1)*)* @testcase, !1}
!1 = !{!"image_access_qualifier", i32 3, i32 3, i32 3}
!2 = !{!"-cl-std=CL1.2"}
!3 = !{!"double", !4}
!4 = !{!"omnipotent char", !5}
!5 = !{!"Simple C/C++ TBAA"}
