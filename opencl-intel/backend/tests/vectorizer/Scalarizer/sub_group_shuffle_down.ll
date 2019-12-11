; RUN: %oclopt  -runtimelib %p/../Full/runtime.bc -scalarize -verify -S -o - %s \
; RUN: | FileCheck %s

; ModuleID = 'main'
source_filename = "2"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux"

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32 %0) local_unnamed_addr

; Function Attrs: convergent
declare i32 @_Z18get_sub_group_sizev() local_unnamed_addr

; Function Attrs: convergent
declare i32 @_Z22get_sub_group_local_idv() local_unnamed_addr

; Function Attrs: convergent
declare <2 x i32> @_Z28intel_sub_group_shuffle_downDv2_iS_j(<2 x i32> %0, <2 x i32> %1, i32 %2) local_unnamed_addr
; Function Attrs: convergent nounwind readnone
declare i32 @_Z3allDv2_i(<2 x i32> %0) local_unnamed_addr

declare i64 @_Z14get_local_sizej(i32 %0)

declare i64 @get_base_global_id.(i32 %0)

; CHECK-LABEL: @__Vectorized_.testKernel
; Function Attrs: convergent nounwind
define void @__Vectorized_.testKernel(i32 addrspace(1)* noalias %shuffle_results) local_unnamed_addr {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0)
  %call1 = tail call i32 @_Z18get_sub_group_sizev()
  %call2 = tail call i32 @_Z22get_sub_group_local_idv()
  %add = add i32 %call2, 1
  %splat.splatinsert = insertelement <2 x i32> undef, i32 %add, i32 0
  %splat.splat = shufflevector <2 x i32> %splat.splatinsert, <2 x i32> undef, <2 x i32> zeroinitializer
  %add3 = add <2 x i32> %splat.splat, <i32 100, i32 100>
; CHECK: [[A0:%[_a-z0-9]+]] = call <2 x i32> @fake.insert.element{{[0-9]*}}(<2 x i32> undef, i32 %add, i32 0)
; CHECK: [[B0:%[_a-z0-9]+]] = call <2 x i32> @fake.insert.element{{[0-9]*}}(<2 x i32> %0, i32 %add, i32 1)
; CHECK: [[A1:%[_a-z0-9]+]] = call <2 x i32> @fake.insert.element{{[0-9]*}}(<2 x i32> undef, i32 %add3{{[_a-z0-9]+}}, i32 0)
; CHECK: [[B1:%[_a-z0-9]+]] = call <2 x i32> @fake.insert.element{{[0-9]*}}(<2 x i32> %2, i32 %add3{{[_a-z0-9]+}}, i32 1)
  %call4 = tail call <2 x i32> @_Z28intel_sub_group_shuffle_downDv2_iS_j(<2 x i32> %splat.splat, <2 x i32> %add3, i32 0)
; CHECK: [[R0:%[_a-z0-9]+]] = tail call <2 x i32> @_Z28intel_sub_group_shuffle_downDv2_iS_j(<2 x i32> [[B0]], <2 x i32> [[B1]], i32 0)
; CHECK: call i32 @fake.extract.element{{[0-9]*}}(<2 x i32> [[R0]], i32 0)
; CHECK: call i32 @fake.extract.element{{[0-9]*}}(<2 x i32> [[R0]], i32 1)
  %cmp6 = icmp ult i32 %call2, %call1
  %sub = sub i32 100, %call1
  %add11 = select i1 %cmp6, i32 0, i32 %sub
  %0 = add i32 %add, %add11
  %splat.splatinsert12 = insertelement <2 x i32> undef, i32 %0, i32 0
  %splat.splat13 = shufflevector <2 x i32> %splat.splatinsert12, <2 x i32> undef, <2 x i32> zeroinitializer
  %cmp14 = icmp eq <2 x i32> %call4, %splat.splat13
  %sext = sext <2 x i1> %cmp14 to <2 x i32>
  %call15 = tail call i32 @_Z3allDv2_i(<2 x i32> %sext)
  %tobool = icmp ne i32 %call15, 0
  %1 = zext i1 %tobool to i32
  %arrayidx22 = getelementptr inbounds i32, i32 addrspace(1)* %shuffle_results, i64 %call
  store i32 %1, i32 addrspace(1)* %arrayidx22, align 4
  ret void
}
