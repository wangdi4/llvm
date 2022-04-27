; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; // C++ source:
; // #include <stdio.h>
; void __attribute__((nothrow,noinline))  foo_gpu(int aaa, int* bbb) {
;   // printf("\n *** VARIANT FUNCTION ***\n");
; }
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)})
; void __attribute__((nothrow,noinline))  foo(int aaa, int* bbb) {
;   // printf("\n *** BASE FUNCTION ***\n");
; }
; int main() {
;   int *ptr;
;   int ccc, ddd;
;   #pragma omp dispatch device(0) depend(in:ccc) depend(out:ddd)
;     foo(0,ptr);
;   return 0;
; }
;
; CHECK:  call i8* @__kmpc_omp_task_alloc(%struct.ident_t* @{{.*}}, i32 0, i32 0, i64 0, i64 0, i8* null)
; CHECK:  call void @__kmpc_omp_wait_deps(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 2, i8* %{{.*}}, i32 0, i8* null)
; CHECK:  call void @__kmpc_omp_task_begin_if0(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i8* %{{.*}})
; CHECK:  call void @_Z7foo_gpuiPi(i32 0, i32* %{{.*}})
; CHECK:  call void @__kmpc_omp_task_complete_if0(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i8* %{{.*}})

; ModuleID = 'dispatch_depend.cpp'
source_filename = "dispatch_depend.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline norecurse nounwind optnone uwtable mustprogress
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %ptr = alloca i32*, align 8
  %ccc = alloca i32, align 4
  %ddd = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.IF"(i32 0), "QUAL.OMP.IMPLICIT"(), "QUAL.OMP.DEPEND.IN"(i32* %ccc), "QUAL.OMP.DEPEND.OUT"(i32* %ddd), "QUAL.OMP.SHARED"(i32** %ptr) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(), "QUAL.OMP.DEVICE"(i32 0) ]
  %2 = load i32*, i32** %ptr, align 8
  call void @_Z3fooiPi(i32 0, i32* %2) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.DISPATCH"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z7foo_gpuiPi(i32 %aaa, i32* %bbb) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  ret void
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z3fooiPi(i32 %aaa, i32* %bbb) #1 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z7foo_gpuiPi;construct:dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { noinline norecurse nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
