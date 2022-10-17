; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s

; // C++ source
; // #include <stdio.h>
; #include <omp.h>
; void __attribute__((nothrow,noinline))  foo_gpu(int aaa, omp_interop_t interop) {
;   // printf("\n *** VARIANT FUNCTION ***\n");
; }
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)}) \
;                                      append_args(interop(targetsync))
; void __attribute__((nothrow,noinline))  foo(int aaa) {
;   // printf("\n *** BASE FUNCTION ***\n");
; }
; int main() {
;   omp_interop_t iop;
;   // #pragma omp interop init(targetsync: iop) device(2)
;   #pragma omp dispatch interop(iop) device(2)
;     foo(123);
;   return 0;
; }

; When both interop clause and device clause are specified, use the device num
; from the device(2) clause. Do not extract the device num from the interop obj
; CHECK-NOT: omp_get_interop_int
; CHECK: call i32 @__tgt_is_device_available(i64 2, i8* inttoptr (i64 15 to i8*))

; Check that the interop obj '%i0' is passed to the variant function
; CHECK-DAG:  call void @_Z7foo_gpuiPv(i32 123, i8* %i0)
; CHECK-DAG:  call void @_Z3fooi(i32 noundef 123)

; ModuleID = 'dispatch_interop_clause_device.cpp'
source_filename = "dispatch_interop_clause_device.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z7foo_gpuiPv(i32 noundef %aaa, i8* noundef %interop) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %interop.addr = alloca i8*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i8* %interop, i8** %interop.addr, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fooi(i32 noundef %aaa) #1 {
entry:
  %aaa.addr = alloca i32, align 4
  store i32 %aaa, i32* %aaa.addr, align 4
  ret void
}

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %iop = alloca i8*, align 8
  store i32 0, i32* %retval, align 4
  %i0 = load i8*, i8** %iop, align 8
  %i1 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.INTEROP"(i8* %i0),
    "QUAL.OMP.DEVICE"(i32 2) ]

  call void @_Z3fooi(i32 noundef 123) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %i1) [ "DIR.OMP.END.DISPATCH"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z7foo_gpuiPv;construct:dispatch;arch:gen;interop:targetsync" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress noinline norecurse nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
