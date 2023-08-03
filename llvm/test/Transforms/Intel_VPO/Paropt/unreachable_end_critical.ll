; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,loop-simplify,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Original code:
; #include <stdlib.h>
;
; void foo();
;
; void bar() {
; #pragma omp parallel
;   try {
;     foo();
;   } catch(int t) {
;     if (t != 0) {
; #pragma omp critical
;       exit(0);
;     }
;   }
; }

; Verify that VPO Paropt does not fail to extract the parallel region:
; CHECK: define internal void @_Z3barv.DIR.OMP.PARALLEL

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

$__clang_call_terminate = comdat any

@_ZTIi = external dso_local constant ptr

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z3barv() #0 personality ptr @__gxx_personality_v0 {
entry:
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  %t = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %t, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %exn.slot, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %ehselector.slot, i32 0, i32 1) ]

  invoke void @_Z3foov()
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  br label %try.cont

lpad:                                             ; preds = %entry
  %1 = landingpad { ptr, i32 }
          catch ptr @_ZTIi
          catch ptr null
  %2 = extractvalue { ptr, i32 } %1, 0
  store ptr %2, ptr %exn.slot, align 8
  %3 = extractvalue { ptr, i32 } %1, 1
  store i32 %3, ptr %ehselector.slot, align 4
  br label %catch.dispatch

catch.dispatch:                                   ; preds = %lpad
  %sel = load i32, ptr %ehselector.slot, align 4
  %4 = call i32 @llvm.eh.typeid.for(ptr @_ZTIi) #1
  %matches = icmp eq i32 %sel, %4
  br i1 %matches, label %catch, label %terminate.handler

catch:                                            ; preds = %catch.dispatch
  %exn = load ptr, ptr %exn.slot, align 8
  %5 = call ptr @__cxa_begin_catch(ptr %exn) #1
  %6 = load i32, ptr %5, align 4
  store i32 %6, ptr %t, align 4
  %7 = load i32, ptr %t, align 4
  %cmp = icmp ne i32 %7, 0
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %catch
  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.CRITICAL"() ]

  fence acquire
  call void @exit(i32 0) #6
  unreachable

dummy:                                            ; No predecessors!
  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.CRITICAL"() ]

  br label %if.end

if.end:                                           ; preds = %dummy, %catch
  call void @__cxa_end_catch() #1
  br label %try.cont

try.cont:                                         ; preds = %if.end, %invoke.cont
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void

terminate.handler:                                ; preds = %catch.dispatch
  %exn1 = load ptr, ptr %exn.slot, align 8
  call void @__clang_call_terminate(ptr %exn1) #6
  unreachable
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @_Z3foov() #2

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: nounwind readnone
declare i32 @llvm.eh.typeid.for(ptr) #3

; Function Attrs: noinline noreturn nounwind
define linkonce_odr hidden void @__clang_call_terminate(ptr %0) #4 comdat {
  %2 = call ptr @__cxa_begin_catch(ptr %0) #1
  call void @_ZSt9terminatev() #6
  unreachable
}

declare dso_local ptr @__cxa_begin_catch(ptr)

declare dso_local void @_ZSt9terminatev()

; Function Attrs: noreturn nounwind
declare dso_local void @exit(i32) #5

declare dso_local void @__cxa_end_catch()

attributes #0 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-jump-tables"="false" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #3 = { nounwind readnone }
attributes #4 = { noinline noreturn nounwind }
attributes #5 = { noreturn nounwind "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" "use-soft-float"="false" }
attributes #6 = { noreturn nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.0"}
