; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src: codegen for the scope construct -- without nowait clause
;void foo()
;{
;  int x = 0;
;  int l = 0;
;  #pragma omp scope private(x) reduction(+:l)
;  {
;    x = 10;
;    ++l;
;  }
;}

; Note: The test IR is hand-generated

; CHECK: %l.red = alloca i32, align 4
; CHECK: %x.priv = alloca i32, align 4
; CHECK: call void @__kmpc_scope(
; CHECK: store i32 10, i32* %x.priv, align 4
; CHECK: call i32 @__kmpc_reduce(
; CHECK: call void @__kmpc_end_reduce(
; CHECK: call void @__kmpc_end_scope(
; CHECK: call void @__kmpc_barrier(

; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %x = alloca i32, align 4
  %l = alloca i32, align 4
  store i32 0, i32* %x, align 4
  store i32 0, i32* %l, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCOPE"(), "QUAL.OMP.PRIVATE"(i32* %x), "QUAL.OMP.REDUCTION.ADD"(i32* %l) ]
  store i32 10, i32* %x, align 4
  %1 = load i32, i32* %l, align 4
  %inc = add nsw i32 %1, 1
  store i32 %inc, i32* %l, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SCOPE"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{i32 7, !"frame-pointer", i32 2}
