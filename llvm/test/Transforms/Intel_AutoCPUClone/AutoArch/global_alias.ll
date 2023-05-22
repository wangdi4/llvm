; RUN: opt -passes=auto-cpu-clone -disable-selective-mv < %s -S | FileCheck %s

; This test checks that GlobalAliases to a multi-versioned function are also multi-versioned.
; This test also checks that aliasees are multiversioned using wrapper-based resolvers.

; CHECK: @_ZN3fooC1Ev.A = dso_local unnamed_addr alias void (ptr), ptr @_ZN3fooC2Ev.A
; CHECK-NEXT: @_ZN3fooC1Ev = dso_local unnamed_addr alias void (ptr), ptr @_ZN3fooC2Ev
; CHECK-NEXT: @_ZN3fooC1Ev.a = dso_local unnamed_addr alias void (ptr), ptr @_ZN3fooC2Ev.a

; Make sure each clone of main() calls the right clone of GlobalAlias @_ZN3fooC1Ev
; CHECK: define dso_local noundef i32 @main.A() #1 !llvm.acd.clone !0 {
; CHECK:   call void @_ZN3fooC1Ev.A(ptr noundef nonnull align 4 dereferenceable(4) %f)

; Use wrapper-based resolver to multiversion aliasee @_ZN3fooC2Ev
; CHECK: define dso_local void @_ZN3fooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this) unnamed_addr #0 align 2

; Make sure each clone of main() calls the right clone of GlobalAlias @_ZN3fooC1Ev
; CHECK: define dso_local noundef i32 @main.a() #4 !llvm.acd.clone !0 {
; CHECK:   call void @_ZN3fooC1Ev.a(ptr noundef nonnull align 4 dereferenceable(4) %f)

; Use ifunc-based resolver to multiversion functions that are not aliasees
; CHECK: define dso_local ptr @main.resolver() #3 {


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.foo = type { i32 }

@_ZN3fooC1Ev = dso_local unnamed_addr alias void (ptr), ptr @_ZN3fooC2Ev

; Function Attrs: nounwind uwtable
define dso_local void @_ZN3fooC2Ev(ptr noundef nonnull align 4 dereferenceable(4) %this) unnamed_addr #0 align 2 !llvm.auto.arch !0 {
entry:
  %this.addr = alloca ptr, align 8
  store ptr %this, ptr %this.addr, align 8
  %this1 = load ptr, ptr %this.addr, align 8
  ret void
}

; Function Attrs: mustprogress norecurse uwtable
define dso_local noundef i32 @main() #1 !llvm.auto.arch !0 {
entry:
  %f = alloca %class.foo, align 4
  %0 = bitcast ptr %f to ptr
  call void @_ZN3fooC1Ev(ptr noundef nonnull align 4 dereferenceable(4) %f)
  %1 = bitcast ptr %f to ptr
  ret i32 0
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!0 = !{!"skylake-avx512"}
