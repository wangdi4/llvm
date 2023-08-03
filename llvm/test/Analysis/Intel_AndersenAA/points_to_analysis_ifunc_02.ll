; CMPLRLLVM-36058: This test verifies that store to %notconst is not
; eliminated by InstCombine using AndersensAA.
; AndersensAA was incorrectly computing points-to info for %notconst
; because it was not handling ifuncs.

; RUN: opt < %s -passes="require<anders-aa>,instcombine" -S  2>&1 | FileCheck %s

; Check store instruction in @bar is not eliminated.
; CHECK: define internal fastcc void @bar()
; CHECK: store i32 2, ptr %notconst, align 8

%struct.s_net = type { ptr, i32, ptr, float, float }

@net = internal unnamed_addr global ptr null, align 8
@__intel_cpu_feature_indicator_x = external dso_local local_unnamed_addr global [2 x i64]

@my_malloc = dso_local ifunc ptr (i64), ptr @my_malloc.resolver

define internal nonnull ptr @my_malloc.resolver() {
bb:
  tail call void @__intel_cpu_features_init_x()
  %i = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  %i1 = and i64 %i, 429926490092
  %i2 = icmp eq i64 %i1, 429926490092
  %i3 = and i64 %i, 10330092
  %i4 = icmp eq i64 %i3, 10330092
  %i5 = select i1 %i4, ptr @my_malloc.V, ptr @my_malloc.A
  %i6 = select i1 %i2, ptr @my_malloc.a, ptr %i5
  ret ptr %i6
}

define internal noalias ptr @my_malloc.V(i64 noundef %arg) {
bb:
  %i = tail call noalias align 16 ptr @malloc(i64 noundef %arg)
  %i1 = icmp eq ptr %i, null
  br i1 %i1, label %bb2, label %bb5

bb2:                                              ; preds = %bb
  unreachable

bb5:                                              ; preds = %bb
  ret ptr %i
}

define internal noalias ptr @my_malloc.A(i64 noundef %arg) {
bb:
  %i = tail call noalias align 16 ptr @malloc(i64 noundef %arg)
  %i1 = icmp eq ptr %i, null
  br i1 %i1, label %bb2, label %bb5

bb2:                                              ; preds = %bb
  unreachable

bb5:                                              ; preds = %bb
  ret ptr %i
}

define internal noalias ptr @my_malloc.a(i64 noundef %arg) {
bb:
  %i = tail call noalias align 16 ptr @malloc(i64 noundef %arg)
  %i1 = icmp eq ptr %i, null
  br i1 %i1, label %bb2, label %bb5

bb2:                                              ; preds = %bb
  unreachable

bb5:                                              ; preds = %bb
  ret ptr %i
}

define internal fastcc void @init_parse(i32 noundef %arg) unnamed_addr {
bb:
  %i10 = tail call ptr @my_malloc(i64 noundef 256)
  store ptr %i10, ptr @net, align 8
  ret void
}

define internal fastcc void @bar() unnamed_addr {
bb:
  %i120 = load ptr, ptr @net, align 8
  %i121 = getelementptr inbounds %struct.s_net, ptr %i120, i64 2
  %notconst = getelementptr inbounds %struct.s_net, ptr %i120, i64 0, i32 1
  store i32 2, ptr %notconst, align 8
  ret void
}

declare dso_local noalias noundef ptr @malloc(i64 noundef)

declare dso_local void @__intel_cpu_features_init_x()
