; CMPLRLLVM-36058: This test verifies that store to %notconst is not
; eliminated by InstCombine using AndersensAA.
; AndersensAA was incorrectly computing points-to info for %notconst
; because it was not handling ifuncs.

; RUN: opt < %s -passes="require<anders-aa>,instcombine" -S  2>&1 | FileCheck %s

; Check store instruction in @bar is not eliminated.
; CHECK: define internal fastcc void @bar()
; CHECK: store i32 2, i32* %notconst, align 8

%struct.s_net = type { i8*, i32, i32*, float, float }
@net = internal unnamed_addr global %struct.s_net* null, align 8
@__intel_cpu_feature_indicator_x = external dso_local local_unnamed_addr global [2 x i64]
@my_malloc = dso_local ifunc i8* (i64), i8* (i64)* ()* @my_malloc.resolver

define internal nonnull i8* (i64)* @my_malloc.resolver() {
bb:
  tail call void @__intel_cpu_features_init_x()
  %i = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 0), align 8
  %i1 = and i64 %i, 429926490092
  %i2 = icmp eq i64 %i1, 429926490092
  %i3 = and i64 %i, 10330092
  %i4 = icmp eq i64 %i3, 10330092
  %i5 = select i1 %i4, i8* (i64)* @my_malloc.V, i8* (i64)* @my_malloc.A
  %i6 = select i1 %i2, i8* (i64)* @my_malloc.a, i8* (i64)* %i5
  ret i8* (i64)* %i6
}

; Function Attrs: nofree nounwind strictfp uwtable
define internal noalias i8* @my_malloc.V(i64 noundef %arg) {
bb:
  %i = tail call noalias align 16 i8* @malloc(i64 noundef %arg)
  %i1 = icmp eq i8* %i, null
  br i1 %i1, label %bb2, label %bb5

bb2:                                              ; preds = %bb
  unreachable

bb5:                                              ; preds = %bb
  ret i8* %i
}

; Function Attrs: nofree nounwind strictfp uwtable
define internal noalias i8* @my_malloc.A(i64 noundef %arg) {
bb:
  %i = tail call noalias align 16 i8* @malloc(i64 noundef %arg)
  %i1 = icmp eq i8* %i, null
  br i1 %i1, label %bb2, label %bb5

bb2:                                              ; preds = %bb
  unreachable

bb5:                                              ; preds = %bb
  ret i8* %i
}

; Function Attrs: nofree nounwind strictfp uwtable
define internal noalias i8* @my_malloc.a(i64 noundef %arg) {
bb:
  %i = tail call noalias align 16 i8* @malloc(i64 noundef %arg)
  %i1 = icmp eq i8* %i, null
  br i1 %i1, label %bb2, label %bb5

bb2:                                              ; preds = %bb
  unreachable

bb5:                                              ; preds = %bb
  ret i8* %i
}

; Function Attrs: nounwind strictfp uwtable
define internal fastcc void @init_parse(i32 noundef %arg) unnamed_addr {
bb:
  %i10 = tail call i8* @my_malloc(i64 noundef 256)
  store i8* %i10, i8** bitcast (%struct.s_net** @net to i8**), align 8
  ret void
}

; Function Attrs: nounwind strictfp uwtable
define internal fastcc void @bar() unnamed_addr {
bb:
  %i120 = load %struct.s_net*, %struct.s_net** @net, align 8
  %i121 = getelementptr inbounds %struct.s_net, %struct.s_net* %i120, i64 2
  %notconst = getelementptr inbounds %struct.s_net, %struct.s_net* %i120, i64 0, i32 1
  store i32 2, i32* %notconst, align 8
  ret void
}

declare dso_local noalias noundef i8* @malloc(i64 noundef)
declare dso_local void @__intel_cpu_features_init_x()
