; RUN: opt -opaque-pointers -passes=auto-cpu-clone < %s -S | FileCheck %s

; The test verifies that variadic functions are multiversioned correctly.
; Original C code:
;
; #include <stdarg.h>
;
; int bar(int Size, ...) {
;   return 33;
; }
; int main() {
;  return bar(1, 42);
; }


; CHECK: @__intel_cpu_feature_indicator = external global [2 x i64]
; CHECK: @_Z3bariz = dso_local ifunc i32 (i32, ...), ptr @_Z3bariz.resolver
; CHECK: @main = dso_local ifunc i32 (), ptr @main.resolver

; CHECK: define dso_local i32 @_Z3bariz.A(i32 %Size, ...)
; CHECK: define dso_local i32 @main.A()
; CHECK: define dso_local i32 @_Z3bariz.V(i32 %Size, ...)
; CHECK: define dso_local ptr @_Z3bariz.resolver()
; CHECK: define dso_local i32 @main.V()
; CHECK: define dso_local ptr @main.resolver()


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind uwtable
define dso_local i32 @_Z3bariz(i32 %Size, ...) #0 !llvm.auto.cpu.dispatch !4 {
entry:
  %Size.addr = alloca i32, align 4
  store i32 %Size, ptr %Size.addr, align 4
  ret i32 33
}

; Function Attrs: mustprogress noinline norecurse nounwind uwtable
define dso_local i32 @main() #1 !llvm.auto.cpu.dispatch !4 {
entry:
  %retval = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %call = call i32 (i32, ...) @_Z3bariz(i32 1, i32 42)
  ret i32 %call
}

attributes #0 = { mustprogress noinline nounwind uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress noinline norecurse nounwind uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{i32 7, !"frame-pointer", i32 2}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!4 = !{!5}
!5 = !{!"auto-cpu-dispatch-target", !"core-avx2"}
