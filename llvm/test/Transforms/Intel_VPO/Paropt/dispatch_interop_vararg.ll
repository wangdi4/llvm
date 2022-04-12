; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s

; Check that OMP5.1 dispatch with interop works with vararg functions.
; The interop obj is expected to be the last explicit formal parameter in the
; variant function's declaration. In the variant function call the interop obj
; must be inserted into the corresponding position among the actual arguments.
;
; // C source
; #include <omp.h>
; #include <stdio.h>
; void __attribute__((nothrow,noinline))  foo_gpu(int a, int b, omp_interop_t interop, ...) {
;   printf("VARIANT FUNCTION\n");
; }
;
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)}) \
;                                      append_args(interop(target))
; void __attribute__((nothrow,noinline))  foo(int a, int b, ...) {
;   printf("BASE FUNCTION\n");
; }
;
; int main() {
;   #pragma omp dispatch
;     foo(111, 222, 333);
;   return 0;
; }

; ModuleID = 'dispatch_interop_vararg.cpp'
source_filename = "dispatch_interop_vararg.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [18 x i8] c"VARIANT FUNCTION\0A\00", align 1
@.str.1 = private unnamed_addr constant [15 x i8] c"BASE FUNCTION\0A\00", align 1

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z7foo_gpuiiPvz(i32 noundef %a, i32 noundef %b, i8* noundef %interop, ...) #0 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  %interop.addr = alloca i8*, align 8
  store i32 %a, i32* %a.addr, align 4
  store i32 %b, i32* %b.addr, align 4
  store i8* %interop, i8** %interop.addr, align 8
  %call = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i64 0, i64 0))
  ret void
}

declare dso_local i32 @printf(i8* noundef, ...) #1

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fooiiz(i32 noundef %a, i32 noundef %b, ...) #2 {
entry:
  %a.addr = alloca i32, align 4
  %b.addr = alloca i32, align 4
  store i32 %a, i32* %a.addr, align 4
  store i32 %b, i32* %b.addr, align 4
  %call = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([15 x i8], [15 x i8]* @.str.1, i64 0, i64 0))
  ret void
}

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #3 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"() ]
  call void (i32, i32, ...) @_Z3fooiiz(i32 noundef 111, i32 noundef 222, i32 noundef 333) #4 [ "QUAL.OMP.DISPATCH.CALL"() ]

; variant call: interop obj is right before the vararg list (333)
; CHECK-DAG: call void (i32, i32, i8*, ...) @_Z7foo_gpuiiPvz(i32 111, i32 222, i8* %interop.obj{{.*}}, i32 333)

; base call
; CHECK-DAG: call void (i32, i32, ...) @_Z3fooiiz(i32 noundef 111, i32 noundef 222, i32 noundef 333)

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.DISPATCH"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z7foo_gpuiiPvz;construct:dispatch;arch:gen;interop:target" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { mustprogress noinline norecurse nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
