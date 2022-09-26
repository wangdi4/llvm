; RUN: opt -auto-cpu-clone < %s -S | FileCheck %s
; RUN: opt -passes=auto-cpu-clone < %s -S | FileCheck %s

; CHECK: @_ZN3fooC1Ev.A = dso_local unnamed_addr alias void (%class.foo*), void (%class.foo*)* @_ZN3fooC2Ev.A
; CHECK-NEXT: @_ZN3fooC1Ev = dso_local unnamed_addr alias void (%class.foo*), void (%class.foo*)* @_ZN3fooC2Ev
; CHECK-NEXT: @_ZN3fooC1Ev.a = dso_local unnamed_addr alias void (%class.foo*), void (%class.foo*)* @_ZN3fooC2Ev.a

; CHECK: define dso_local noundef i32 @main.A() #1 !llvm.acd.clone !0 {
; CHECK:   call void @_ZN3fooC1Ev.A(%class.foo* noundef nonnull align 4 dereferenceable(4) %f)

; CHECK: define dso_local noundef i32 @main.a() #4 !llvm.acd.clone !0 {
; CHECK:   call void @_ZN3fooC1Ev.a(%class.foo* noundef nonnull align 4 dereferenceable(4) %f)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.foo = type { i32 }

@_ZN3fooC1Ev = dso_local unnamed_addr alias void (%class.foo*), void (%class.foo*)* @_ZN3fooC2Ev

; Function Attrs: nounwind uwtable
define dso_local void @_ZN3fooC2Ev(%class.foo* noundef nonnull align 4 dereferenceable(4) %this) unnamed_addr #0 align 2 !llvm.auto.cpu.dispatch !3 {
entry:
  %this.addr = alloca %class.foo*, align 8
  store %class.foo* %this, %class.foo** %this.addr, align 8
  %this1 = load %class.foo*, %class.foo** %this.addr, align 8
  ret void
}

; Function Attrs: mustprogress norecurse uwtable
define dso_local noundef i32 @main() #1 !llvm.auto.cpu.dispatch !3 {
entry:
  %f = alloca %class.foo, align 4
  %0 = bitcast %class.foo* %f to i8*
  call void @_ZN3fooC1Ev(%class.foo* noundef nonnull align 4 dereferenceable(4) %f)
  %1 = bitcast %class.foo* %f to i8*
  ret i32 0
}

attributes #0 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress norecurse uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!3 = !{!4}
!4 = !{!"auto-cpu-dispatch-target", !"skylake-avx512"}
