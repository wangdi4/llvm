; This test verifies that AndersAA handles ifuncs.
; Checks that AndersAA computes  that %i10 points-to @my_malloc.V:%i

; RUN: opt < %s -passes="require<anders-aa>" -disable-output -print-anders-points-to 2>&1 | FileCheck %s

; CHECK: [1] init_parse:i10        --> ({{[0-9a-f]+}}): my_malloc.V:i

%struct.s_net = type { ptr, i32, ptr, float, float }
@net = internal unnamed_addr global ptr null, align 8
@my_malloc = dso_local ifunc ptr (i64), ptr @my_malloc.resolver

define internal nonnull ptr @my_malloc.resolver() {
bb:
  ret ptr @my_malloc.V
}

; Function Attrs: nofree nounwind strictfp uwtable
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

; Function Attrs: nounwind strictfp uwtable
define internal fastcc void @init_parse(i32 noundef %arg) unnamed_addr {
bb:
  %i10 = tail call ptr @my_malloc(i64 noundef 256)
  store ptr %i10, ptr bitcast (ptr @net to ptr), align 8
  %i119 = tail call noalias align 16 ptr @malloc(i64 100)
  %i120 = load ptr, ptr @net, align 8
  %i122 = getelementptr inbounds %struct.s_net, ptr %i120, i64 2, i32 2
  %notempty = bitcast ptr %i122 to ptr
  store ptr %i119, ptr %notempty, align 8
  %i123 = load ptr, ptr %notempty, align 8
  ret void
}

declare dso_local noalias noundef ptr @malloc(i64 noundef)
