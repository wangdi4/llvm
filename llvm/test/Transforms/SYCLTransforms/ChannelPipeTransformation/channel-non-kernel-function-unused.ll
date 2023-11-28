; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Check user of channel builtin is erased in non-kernel function 'bar' that
; isn't used by any kernel.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

@a = addrspace(1) global ptr addrspace(1) null, align 8, !packet_size !0, !packet_align !0

define dso_local void @test() #0 !arg_type_null_val !2 {
entry:
  %0 = load ptr addrspace(1), ptr addrspace(1) @a, align 8
  call void @foo(ptr addrspace(1) %0) #2
  ret void
}

define internal void @foo(ptr addrspace(1) %ch) #1 !arg_type_null_val !3 {
entry:
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %ch, i32 noundef 5) #2
  ret void
}

define internal i32 @bar(ptr addrspace(1) %b) #1 !arg_type_null_val !3 {
entry:
; CHECK-LABEL: define internal i32 @bar(
; CHECK-NEXT: entry:
; CHECK-NEXT: ret i32 undef

  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %b, i32 noundef 5) #2
  %call = call i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1) %b) #2
  ret i32 %call
}

declare void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1), i32 noundef) #2

declare i32 @_Z18read_channel_intel11ocl_channeli(ptr addrspace(1)) #2

attributes #0 = { convergent norecurse nounwind }
attributes #1 = { convergent noinline norecurse nounwind }
attributes #2 = { convergent nounwind }

!sycl.kernels = !{!1}

!0 = !{i32 4}
!1 = !{ptr @test}
!2 = !{}
!3 = !{target("spirv.Channel") zeroinitializer}

; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function foo --  %write.src = alloca i32, align 4
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor --  call void @__pipe_init_fpga(ptr addrspace(1) @a.bs, i32 4, i32 0, i32 0)
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor --  store ptr addrspace(1) @a.bs, ptr addrspace(1) @a, align 8
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function __pipe_global_ctor --  ret void
; DEBUGIFY: WARNING: Missing line 6
; DEBUGIFY: WARNING: Missing line 7
; DEBUGIFY-NOT: WARNING
