; This test verifies that AndersAA handles ifuncs.
; Checks that AndersAA computes  that %i10 points-to @my_malloc.V:%i

; RUN: opt < %s -passes="require<anders-aa>" -disable-output -print-anders-points-to 2>&1 | FileCheck %s

; CHECK: [1] init_parse:i10        --> ({{[0-9a-f]+}}): my_malloc.V:i

%struct.s_net = type { i8*, i32, i32*, float, float }
@net = internal unnamed_addr global %struct.s_net* null, align 8
@my_malloc = dso_local ifunc i8* (i64), i8* (i64)* ()* @my_malloc.resolver

define internal nonnull i8* (i64)* @my_malloc.resolver() {
bb:
  ret i8* (i64)* @my_malloc.V
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

; Function Attrs: nounwind strictfp uwtable
define internal fastcc void @init_parse(i32 noundef %arg) unnamed_addr {
bb:
  %i10 = tail call i8* @my_malloc(i64 noundef 256)
  store i8* %i10, i8** bitcast (%struct.s_net** @net to i8**), align 8
  %i119 = tail call noalias align 16 i8* @malloc(i64 100)
  %i120 = load %struct.s_net*, %struct.s_net** @net, align 8
  %i122 = getelementptr inbounds %struct.s_net, %struct.s_net* %i120, i64 2, i32 2
  %notempty = bitcast i32** %i122 to i8**
  store i8* %i119, i8** %notempty, align 8
  %i123 = load i8*, i8** %notempty, align 8
  ret void
}

declare dso_local noalias noundef i8* @malloc(i64 noundef)
