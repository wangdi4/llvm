; RUN: SATest -OCL -REF -config=%s.cfg -neat=1
; ModuleID = 'call_int.cl'
target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"
target triple = "i686-pc-win32"

define i32 @func(i32 addrspace(1)* %b) nounwind {
entry:
  %b.addr = alloca i32 addrspace(1)*, align 4
  %tid = alloca i32, align 4
  store i32 addrspace(1)* %b, i32 addrspace(1)** %b.addr, align 4
  %call = call i32 @get_global_id(i32 0) nounwind readnone
  store i32 %call, i32* %tid, align 4
  %0 = load i32* %tid, align 4
  %1 = load i32 addrspace(1)** %b.addr, align 4
  %arrayidx = getelementptr inbounds i32 addrspace(1)* %1, i32 %0
  store i32 3, i32 addrspace(1)* %arrayidx
  ret i32 3
}

declare i32 @get_global_id(i32) nounwind readnone

define void @call(i32 addrspace(1)* %a) nounwind {
entry:
  %a.addr = alloca i32 addrspace(1)*, align 4
  store i32 addrspace(1)* %a, i32 addrspace(1)** %a.addr, align 4
  %0 = load i32 addrspace(1)** %a.addr, align 4
  %call = call i32 @func(i32 addrspace(1)* %0)
  ret void
}

!opencl.kernels = !{!0}
!opencl.build.options = !{!2}

!0 = metadata !{void (i32 addrspace(1)*)* @call, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3}
!2 = metadata !{metadata !"-cl-std=CL1.2"}
