; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Test src:
;
; void foo() {}
;
; int main() {
;   char a;
;
;   #pragma omp taskgroup
;     a = 2;
;   #pragma omp taskgroup
;     foo();
;   return a;
; }

; This test checks the codegen of the OMP taskgroup construct.
; CHECK:  call void @__kmpc_taskgroup({{.*}})
; CHECK:  call void @__kmpc_end_taskgroup({{.*}})
                                                    
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() {
entry:
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() {
entry:
  %retval = alloca i32, align 4
  %a = alloca i8, align 1
  store i32 0, ptr %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"() ]

  store i8 2, ptr %a, align 1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASKGROUP"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"() ]

  call void @foo() #2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TASKGROUP"() ]

  %2 = load i8, ptr %a, align 1
  %conv = sext i8 %2 to i32
  ret i32 %conv
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
