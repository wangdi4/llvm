; RUN: opt -opaque-pointers=1 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S %s 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s 2>&1 | FileCheck %s

; Test src:
;
; struct A {
;   long  type;
;   long  mode;
;   long  diag;
; };
; void fooGPU(struct A aaa);
;
; #pragma omp declare variant(fooGPU) match(construct={dispatch}, device={arch(gen)})
; void foo(struct A aaa);
;
; void bar() {
;     struct A aaa;
; #pragma omp dispatch device(0)
;     foo(aaa);  // aaa is a struct passed by val
; }

; The base call has a struct argument that is passed by value.
; Check that the corresponding argument in the variant call is also passed by value.
;
; variant call:
; CHECK: call void @fooGPU(ptr byval(%struct.A) align 8 %aaa)
;
; base call:
; CHECK: call void @foo(ptr noundef byval(%struct.A) align 8 %aaa)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.A = type { i64, i64, i64 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bar() #0 {
entry:
  %aaa = alloca %struct.A, align 8
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0) ]
  call void @foo(ptr noundef byval(%struct.A) align 8 %aaa) #1 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local void @foo(ptr noundef byval(%struct.A) align 8) #2

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }
attributes #2 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:fooGPU;construct:dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
