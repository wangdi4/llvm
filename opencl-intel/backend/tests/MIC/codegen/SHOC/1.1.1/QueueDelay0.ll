; XFAIL: win32
; RUN: llc < %s -mtriple=x86_64-pc-linux -march=y86-64 -mcpu=knf
; ModuleID = 'Program'
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v64:64:64-v128:128:128-a0:0:64-s0:64:64-f80:128:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare void @__one_original(i32 addrspace(1)* nocapture) nounwind readnone

declare void @__two_original(i32 addrspace(1)* nocapture) nounwind readnone

declare void @__three_original(i32 addrspace(1)* nocapture) nounwind readnone

declare void @__four_original(i32 addrspace(1)* nocapture) nounwind readnone

declare [7 x i64] @__WG.boundaries.one_original(i32 addrspace(1)*)

declare i64 @get_local_size(i32)

declare i64 @get_base_global_id.(i32)

declare [7 x i64] @__WG.boundaries.two_original(i32 addrspace(1)*)

declare [7 x i64] @__WG.boundaries.three_original(i32 addrspace(1)*)

declare [7 x i64] @__WG.boundaries.four_original(i32 addrspace(1)*)

declare i1 @allOne(i1)

declare i1 @allZero(i1)

declare void @__one_separated_args(i32 addrspace(1)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind readnone alwaysinline

declare void @__two_separated_args(i32 addrspace(1)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind readnone alwaysinline

declare void @__three_separated_args(i32 addrspace(1)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind readnone alwaysinline

declare void @__four_separated_args(i32 addrspace(1)* nocapture, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*) nounwind readnone alwaysinline

declare [7 x i64] @WG.boundaries.one(i32 addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare [7 x i64] @WG.boundaries.two(i32 addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare [7 x i64] @WG.boundaries.three(i32 addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

declare [7 x i64] @WG.boundaries.four(i32 addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)

define void @two(i8* %pBuffer) {
entry:
  ret void
}

define void @three(i8* %pBuffer) {
entry:
  ret void
}

define void @four(i8* %pBuffer) {
entry:
  ret void
}

define void @one(i8* %pBuffer) {
entry:
  ret void
}

!opencl.kernels = !{!0, !2, !3, !4}
!opencl.build.options = !{!5}
!cl.noBarrierPath.kernels = !{!6}
!opencl.wrappers = !{!7, !8, !9, !10}

!0 = metadata !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__one_separated_args, metadata !1}
!1 = metadata !{metadata !"image_access_qualifier", i32 3}
!2 = metadata !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__two_separated_args, metadata !1}
!3 = metadata !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__three_separated_args, metadata !1}
!4 = metadata !{void (i32 addrspace(1)*, i8 addrspace(3)*, { i32, [3 x i64], [3 x i64], [3 x i64], [3 x i64] }*, i64*, <{ [4 x i64] }>*, <{ [4 x i64] }>*, i64*, i64, i8*, i64*)* @__four_separated_args, metadata !1}
!5 = metadata !{}
!6 = metadata !{metadata !"one", metadata !"two", metadata !"three", metadata !"four"}
!7 = metadata !{void (i8*)* @one}
!8 = metadata !{void (i8*)* @two}
!9 = metadata !{void (i8*)* @three}
!10 = metadata !{void (i8*)* @four}
