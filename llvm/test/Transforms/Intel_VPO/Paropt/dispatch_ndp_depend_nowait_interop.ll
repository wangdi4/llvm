; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s -check-prefix=OCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s -check-prefix=NCG -check-prefix=ALL

; // Test src:

; void __attribute__((nothrow,noinline))  foo_gpu(int aaa, int *bbb, void* interop) {
;   // printf("\n *** VARIANT FUNCTION (NOWAIT) ***\n");
; }
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)}) \
;                                      append_args(interop(targetsync)) \
;                                      adjust_args(need_device_ptr:bbb)
; void __attribute__((nothrow,noinline))  foo(int aaa, int* bbb) {
;   // printf("\n *** BASE FUNCTION ***\n");
; }
; int main() {
;   int *ptr;
;   int ccc,ddd;
;   #pragma omp dispatch device(0) depend(in:ccc) depend(out:ddd) nowait
;     foo(0,ptr);
;   return 0;
; }


; ;;; create interop obj for async NOWAIT
; OCG: [[ASYNCOBJ:%[a-zA-Z._0-9]+]] = call ptr @__kmpc_omp_task_alloc(ptr {{.*}}, i32 0, i32 16, i64 24, i64 0, ptr null)
; OCG: [[INTEROPASYNC:%[a-zA-Z._0-9]+]] = call ptr @__tgt_create_interop_obj(i64 0, i8 1, ptr [[ASYNCOBJ]])
; NCG: [[INTEROPASYNC:%[^ ]+]] = call ptr @__tgt_get_interop_obj(ptr @{{.*}}, i32 1, i32 0, ptr null, i64 0, i32 %my.tid, ptr %current.task)
;
; ;;; handle NEED_DEVICE_PTR
; ;ALL: [[GEP:%.+]] = getelementptr inbounds [1 x ptr], ptr %.offload_baseptrs, i32 0, i32 0
; ;ALL: call void @__tgt_target_data_begin_mapper
; ;ALL: [[UPDATEDVAL:%[a-zA-Z._0-9]+]] = load ptr, ptr [[GEP]], align 8
; ;
; ; ;;;; handle DEPEND
; ; ;OCG: call ptr @__kmpc_omp_task_alloc(ptr @{{.*}}, i32 %{{.*}}, i32 0, i64 0, i64 0, ptr null)
; ; ;ALL: call void @__kmpc_omp_wait_deps(ptr @{{.*}}, i32 %{{.*}}, i32 2, ptr %0, i32 0, ptr null)
; ; ;OCG: call void @__kmpc_omp_task_begin_if0(ptr @{{.*}}, i32 %{{.*}}, ptr %{{.*}})
; ; ;
; ; ;   ;;; variant call foo_gpu(0, ptr, interop)
; ; ;   ALL: call void @_Z7foo_gpuiPiPv(i32 0, ptr [[UPDATEDVAL]], ptr [[INTEROPASYNC]])
; ; ;
; ; ;OCG: call void @__kmpc_omp_task_complete_if0(ptr @{{.*}}, i32 %{{.*}}, ptr %{{.*}})
; ;
; ;ALL: call void @__tgt_target_data_end_mapper


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.kmp_depend_info = type { i64, i64, i8 }

; Function Attrs: mustprogress noinline norecurse nounwind optnone uwtable
define dso_local noundef i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %ptr = alloca ptr, align 8
  %ccc = alloca i32, align 4
  %ddd = alloca i32, align 4
  %.dep.arr.addr = alloca [2 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  store i32 0, ptr %retval, align 4
  %0 = getelementptr inbounds [2 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  %1 = ptrtoint ptr %ccc to i64
  %2 = getelementptr %struct.kmp_depend_info, ptr %0, i64 0
  %3 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 0
  store i64 %1, ptr %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 1
  store i64 4, ptr %4, align 8
  %5 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 2
  store i8 1, ptr %5, align 8
  %6 = ptrtoint ptr %ddd to i64
  %7 = getelementptr %struct.kmp_depend_info, ptr %0, i64 1
  %8 = getelementptr inbounds %struct.kmp_depend_info, ptr %7, i32 0, i32 0
  store i64 %6, ptr %8, align 8
  %9 = getelementptr inbounds %struct.kmp_depend_info, ptr %7, i32 0, i32 1
  store i64 4, ptr %9, align 8
  %10 = getelementptr inbounds %struct.kmp_depend_info, ptr %7, i32 0, i32 2
  store i8 3, ptr %10, align 8
  store i64 2, ptr %dep.counter.addr, align 8

  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IMPLICIT"(),
    "QUAL.OMP.DEPARRAY"(i32 2, ptr %0),
    "QUAL.OMP.SHARED:TYPED"(ptr %ptr, ptr null, i32 1) ]

  %12 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0),
    "QUAL.OMP.NOWAIT"() ]

  %13 = load ptr, ptr %ptr, align 8
  call void @_Z3fooiPi(i32 noundef 0, ptr noundef %13) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]

  call void @llvm.directive.region.exit(token %12) [ "DIR.OMP.END.DISPATCH"() ]
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.TASK"() ]
  ret i32 0
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z7foo_gpuiPiPv(i32 noundef %aaa, ptr noundef %bbb, ptr noundef %interop) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  %interop.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  store ptr %interop, ptr %interop.addr, align 8
  ret void
}

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z3fooiPi(i32 noundef %aaa, ptr noundef %bbb) #1 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca ptr, align 8
  store i32 %aaa, ptr %aaa.addr, align 4
  store ptr %bbb, ptr %bbb.addr, align 8
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z7foo_gpuiPiPv;construct:dispatch;arch:gen;need_device_ptr:F,T,F;interop:targetsync" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress noinline norecurse nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }
