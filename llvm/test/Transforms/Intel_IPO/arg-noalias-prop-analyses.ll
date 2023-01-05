; RUN: opt -aa-pipeline=basic-aa -passes=arg-noalias-prop -S %s | FileCheck %s

; ArgNoAliasProp runs on an entire Module and must take care to request the
; correct Function's DominatorTree and AliasAnalysis results when analyzing
; call sites.
;
; If we pass DT analysis for @stores when performing CaptureTracking analysis
; on @main then we incorrectly conclude that the store in @main does not
; capture %P. This test ensures that this does not happen.

; CHECK-LABEL: @stores
; CHECK-NOT: noalias

@G = global i32* null

define i32 @main() {
entry:
  %P = alloca i32
  store i32* %P, i32** @G
  %call = call i32 @stores(i32* nonnull %P)
  ret i32 %call
}

define internal i32 @stores(i32* nocapture %X) {
entry:
  store i32 1, i32* %X
  %pG = load i32*, i32** @G
  store i32 0, i32* %pG
  %r = load i32, i32* %X
  ret i32 %r
}

