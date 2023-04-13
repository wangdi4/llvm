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

@G = global ptr null

define i32 @main() {
entry:
  %P = alloca i32
  store ptr %P, ptr @G
  %call = call i32 @stores(ptr nonnull %P)
  ret i32 %call
}

define internal i32 @stores(ptr nocapture %X) {
entry:
  store i32 1, ptr %X
  %pG = load ptr, ptr @G
  store i32 0, ptr %pG
  %r = load i32, ptr %X
  ret i32 %r
}

