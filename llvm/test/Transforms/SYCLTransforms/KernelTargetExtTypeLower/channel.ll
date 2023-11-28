; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-target-ext-type-lower -S %s | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@c1 = addrspace(1) global [4 x target("spirv.Channel")] zeroinitializer, align 8, !packet_size !0, !packet_align !0
@c2 = addrspace(1) global [4 x [8 x target("spirv.Channel")]] zeroinitializer, align 8, !packet_size !0, !packet_align !0
@c3 = addrspace(1) global [4 x [8 x [5 x target("spirv.Channel")]]] zeroinitializer, align 8, !packet_size !0, !packet_align !0
@c4 = addrspace(1) global [4 x [8 x [5 x [3 x target("spirv.Channel")]]]] zeroinitializer, align 8, !packet_size !0, !packet_align !0

; CHECK: @c1 = addrspace(1) global [4 x ptr addrspace(1)] zeroinitializer, align 8, !packet_size [[MD0:![0-9]+]], !packet_align [[MD0]]
; CHECK: @c2 = addrspace(1) global [4 x [8 x ptr addrspace(1)]] zeroinitializer, align 8, !packet_size [[MD0]], !packet_align [[MD0]]
; CHECK: @c3 = addrspace(1) global [4 x [8 x [5 x ptr addrspace(1)]]] zeroinitializer, align 8, !packet_size [[MD0]], !packet_align [[MD0]]
; CHECK: @c4 = addrspace(1) global [4 x [8 x [5 x [3 x ptr addrspace(1)]]]] zeroinitializer, align 8, !packet_size [[MD0]], !packet_align [[MD0]]

define dso_local spir_func i32 @foo(target("spirv.Channel") %c) #0 {
entry:
  %c.addr = alloca target("spirv.Channel"), align 8
  store target("spirv.Channel") %c, ptr %c.addr, align 8, !tbaa !2
  %0 = load target("spirv.Channel"), ptr %c.addr, align 8, !tbaa !2
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(target("spirv.Channel") %0) #1
  ret i32 %call
}

declare i32 @_Z18read_channel_intel11ocl_channeli(target("spirv.Channel")) #1

define dso_local spir_kernel void @test(ptr addrspace(1) noundef align 4 %dst) #2 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 {
entry:
; CHECK-LABEL: define dso_local spir_kernel void @test(
; CHECK: load ptr addrspace(1), ptr addrspace(1) @c1, align 8,
; CHECK: load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([8 x ptr addrspace(1)], ptr addrspace(1) @c2, i64 0, i64 2), align 8,
; CHECK: load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([8 x [5 x ptr addrspace(1)]], ptr addrspace(1) @c3, i64 0, i64 2), align 8,
; CHECK: load ptr addrspace(1), ptr addrspace(1) getelementptr inbounds ([3 x ptr addrspace(1)], ptr addrspace(1) getelementptr inbounds ([8 x [5 x [3 x ptr addrspace(1)]]], ptr addrspace(1) @c4, i64 0, i64 2), i64 0, i64 1), align 8,
; CHECK: call spir_func i32 @foo(ptr addrspace(1)

  %dst.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %dst, ptr %dst.addr, align 8, !tbaa !8
  %0 = load target("spirv.Channel"), ptr addrspace(1) @c1, align 8, !tbaa !2
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(target("spirv.Channel") %0) #1
  %1 = load target("spirv.Channel"), ptr addrspace(1) getelementptr inbounds ([8 x target("spirv.Channel")], ptr addrspace(1) @c2, i64 0, i64 2), align 8, !tbaa !2
  %call1 = call i32 @_Z18read_channel_intel11ocl_channeli(target("spirv.Channel") %1) #1
  %add = add nsw i32 %call, %call1
  %2 = load target("spirv.Channel"), ptr addrspace(1) getelementptr inbounds ([8 x [5 x target("spirv.Channel")]], ptr addrspace(1) @c3, i64 0, i64 2), align 8, !tbaa !2
  %call2 = call i32 @_Z18read_channel_intel11ocl_channeli(target("spirv.Channel") %2) #1
  %add3 = add nsw i32 %add, %call2
  %3 = load target("spirv.Channel"), ptr addrspace(1) getelementptr inbounds ([3 x target("spirv.Channel")], ptr addrspace(1) getelementptr inbounds ([8 x [5 x [3 x target("spirv.Channel")]]], ptr addrspace(1) @c4, i64 0, i64 2), i64 0, i64 1), align 8, !tbaa !2
  %call4 = call spir_func i32 @foo(target("spirv.Channel") %3) #1
  %add5 = add nsw i32 %add3, %call4
  %4 = load ptr addrspace(1), ptr %dst.addr, align 8, !tbaa !8
  store i32 %add5, ptr addrspace(1) %4, align 4, !tbaa !10
  ret void
}

; CHECK-LABEL: define dso_local spir_func i32 @foo(ptr addrspace(1)
; CHECK: %c.addr = alloca ptr addrspace(1), align 8
; CHECK: store ptr addrspace(1) %c, ptr %c.addr, align 8,
; CHECK: load ptr addrspace(1), ptr %c.addr, align 8,
; CHECK: %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1)

attributes #0 = { convergent noinline norecurse nounwind }
attributes #1 = { convergent nounwind }
attributes #2 = { convergent norecurse nounwind }

!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!1}

!0 = !{i32 4}
!1 = !{i32 1, i32 2}
!2 = !{!3, !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{i32 1}
!6 = !{!"none"}
!7 = !{!"int*"}
!8 = !{!9, !9, i64 0}
!9 = !{!"any pointer", !3, i64 0}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !3, i64 0}

; DEBUGIFY-NOT: WARNING
