; Check if we create correct bundles for the linear values.

; RUN: opt -vec-clone -S < %s | FileCheck %s
; RUN: opt -passes="vec-clone" -S < %s | FileCheck %s

; CHECK-LABEL: @_ZGVbN4l8l20l4u_foo1
; CHECK-LABEL: simd.begin.region
; CHECK: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: i32** %alloca.a
; CHECK-SAME: i32 2
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: i32** %alloca.b
; CHECK-SAME: i32 5
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: i32** %alloca.c
; CHECK-SAME: i32 1

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
@a = common dso_local local_unnamed_addr global [4096 x i32] zeroinitializer, align 16
@b = common dso_local local_unnamed_addr global [4096 x i32] zeroinitializer, align 16
@c = common dso_local local_unnamed_addr global [4096 x i32] zeroinitializer, align 16
; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo1(i32* %a, i32* %b, i32* %c, i64 %index) local_unnamed_addr #0 {
entry:
  %arrayidx_a = getelementptr inbounds i32, i32* %a, i64 %index
  %array_data_a = load i32, i32* %arrayidx_a, align 4
  %add_a = add nsw i32 %array_data_a, 20
  store i32 %add_a, i32* %arrayidx_a, align 4
  %arrayidx_b = getelementptr inbounds i32, i32* %b, i64 %index
  %array_data_b = load i32, i32* %arrayidx_b, align 4
  %add_b = add nsw i32 %array_data_b, 20
  store i32 %add_b, i32* %arrayidx_b, align 4
  %arrayidx_c = getelementptr inbounds i32, i32* %c, i64 %index
  %array_data_c = load i32, i32* %arrayidx_c, align 4
  %add_c = add nsw i32 %array_data_c, 20
  store i32 %add_c, i32* %arrayidx_c, align 4
  %add1 = add nsw i32 %add_a, %add_b
  %add2 = add nsw i32 %add1, %add_c
  ret i32 %add2
}

; CHECK-LABEL: @_ZGVbN4l8l8l8u_foo2
; CHECK-LABEL: simd.begin.region
; CHECK: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: i32** %alloca.a
; CHECK-SAME: i32 2
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: i32** %alloca.b
; CHECK-SAME: i32 2
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: i32** %alloca.c
; CHECK-SAME: i32 2

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo2(i32* %a, i32* %b, i32* %c, i64 %index) local_unnamed_addr #1 {
entry:
  %arrayidx_a = getelementptr inbounds i32, i32* %a, i64 %index
  %array_data_a = load i32, i32* %arrayidx_a, align 4
  %add_a = add nsw i32 %array_data_a, 20
  store i32 %add_a, i32* %arrayidx_a, align 4
  %arrayidx_b = getelementptr inbounds i32, i32* %b, i64 %index
  %array_data_b = load i32, i32* %arrayidx_b, align 4
  %add_b = add nsw i32 %array_data_b, 20
  store i32 %add_b, i32* %arrayidx_b, align 4
  %arrayidx_c = getelementptr inbounds i32, i32* %c, i64 %index
  %array_data_c = load i32, i32* %arrayidx_c, align 4
  %add_c = add nsw i32 %array_data_c, 20
  store i32 %add_c, i32* %arrayidx_c, align 4
  %add1 = add nsw i32 %add_a, %add_b
  %add2 = add nsw i32 %add1, %add_c
  ret i32 %add2
}

; CHECK-LABEL: @_ZGVbN4l4l4l4u_foo3
; CHECK-LABEL: simd.begin.region
; CHECK: %entry.region = call token @llvm.directive.region.entry()
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: i32** %alloca.a
; CHECK-SAME: i32 1
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: i32** %alloca.b
; CHECK-SAME: i32 1
; CHECK-SAME: QUAL.OMP.LINEAR
; CHECK-SAME: i32** %alloca.c
; CHECK-SAME: i32 1

; Function Attrs: norecurse nounwind uwtable
define dso_local i32 @foo3(i32* %a, i32* %b, i32* %c, i64 %index) local_unnamed_addr #2 {
entry:
  %arrayidx_a = getelementptr inbounds i32, i32* %a, i64 %index
  %array_data_a = load i32, i32* %arrayidx_a, align 4
  %add_a = add nsw i32 %array_data_a, 20
  store i32 %add_a, i32* %arrayidx_a, align 4
  %arrayidx_b = getelementptr inbounds i32, i32* %b, i64 %index
  %array_data_b = load i32, i32* %arrayidx_b, align 4
  %add_b = add nsw i32 %array_data_b, 20
  store i32 %add_b, i32* %arrayidx_b, align 4
  %arrayidx_c = getelementptr inbounds i32, i32* %c, i64 %index
  %array_data_c = load i32, i32* %arrayidx_c, align 4
  %add_c = add nsw i32 %array_data_c, 20
  store i32 %add_c, i32* %arrayidx_c, align 4
  %add1 = add nsw i32 %add_a, %add_b
  %add2 = add nsw i32 %add1, %add_c
  ret i32 %add2
}

attributes #0 = { "vector-variants"="_ZGVbN4l8l20l4u_foo1" }
attributes #1 = { "vector-variants"="_ZGVbN4l8l8l8u_foo2" }
attributes #2 = { "vector-variants"="_ZGVbN4l4l4l4u_foo3" }
