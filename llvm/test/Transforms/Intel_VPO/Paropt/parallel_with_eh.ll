; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; This test checks support for exception handling within omp constructs.

; Test src:
;
; extern void bar();
;
; void foo()
; {
; #pragma omp parallel
;   try {
;     bar();
;   } catch (...) {
;   }
; }

; CHECK: call void {{.*}} @__kmpc_fork_call({{.+}}, {{.+}}, ptr [[PAR_OUTLINED:@.+]])
; CHECK: define internal void [[PAR_OUTLINED]]({{.+}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local void @_Z3foov() personality ptr @__gxx_personality_v0 {
entry:
  %exn.slot = alloca ptr, align 8
  %ehselector.slot = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %exn.slot, ptr null, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %ehselector.slot, i32 0, i32 1) ]

  invoke void @_Z3barv()
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  br label %try.cont

lpad:                                             ; preds = %entry
  %1 = landingpad { ptr, i32 }
          catch ptr null
  %2 = extractvalue { ptr, i32 } %1, 0
  store ptr %2, ptr %exn.slot, align 8
  %3 = extractvalue { ptr, i32 } %1, 1
  store i32 %3, ptr %ehselector.slot, align 4
  br label %catch

catch:                                            ; preds = %lpad
  %exn = load ptr, ptr %exn.slot, align 8
  %4 = call ptr @__cxa_begin_catch(ptr %exn)
  invoke void @__cxa_end_catch()
          to label %invoke.cont1 unwind label %terminate.lpad

invoke.cont1:                                     ; preds = %catch
  br label %try.cont

try.cont:                                         ; preds = %invoke.cont1, %invoke.cont
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void

terminate.lpad:                                   ; preds = %catch
  %5 = landingpad { ptr, i32 }
          catch ptr null
  %6 = extractvalue { ptr, i32 } %5, 0
  call void @__clang_call_terminate(ptr %6)
  unreachable
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local void @_Z3barv()

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local ptr @__cxa_begin_catch(ptr)

declare dso_local void @__cxa_end_catch()

define linkonce_odr hidden void @__clang_call_terminate(ptr noundef %0) {
  %2 = call ptr @__cxa_begin_catch(ptr %0)
  call void @_ZSt9terminatev()
  unreachable
}

declare dso_local void @_ZSt9terminatev()
