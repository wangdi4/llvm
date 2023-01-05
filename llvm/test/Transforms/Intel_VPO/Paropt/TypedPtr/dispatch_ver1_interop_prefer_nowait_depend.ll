; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -S <%s | FileCheck %s
; RUN: opt -vpo-paropt-dispatch-codegen-version=1 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S <%s | FileCheck %s
;
; // C++ test
; #include <omp.h>
; void __attribute__((nothrow,noinline))  foo_gpu(int aaa, void* interop) {
;   // printf("\n *** VARIANT FUNCTION (NOWAIT) ***\n");
; }
; #pragma omp declare variant(foo_gpu) match(construct={dispatch}, device={arch(gen)}) \
;                 append_args(interop(targetsync,prefer_type("level_zero","sycl","opencl")))
; void __attribute__((nothrow,noinline))  foo(int aaa) {
;   // printf("\n *** BASE FUNCTION ***\n");
; }
;
; void test1_with_nowait() {
;   #pragma omp dispatch device(0) nowait
;     foo(123);
; }
;
; void test2_with_depend() {
;   int aaa;
;   #pragma omp dispatch device(0) depend(out:aaa)
;     foo(123);
; }
;
; Given the prefer_type("level_zero","sycl","opencl") specified, this array is generated
; for each of the 3 dispatch constructs:
; CHECK: @.prefer.list{{.*}} = private unnamed_addr constant [3 x i32] [i32 6, i32 4, i32 3]
; CHECK: @.prefer.list{{.*}} = private unnamed_addr constant [3 x i32] [i32 6, i32 4, i32 3]
;
; ModuleID = 'dispatch_ver1_interop_prefer_nowait_depend.cpp'
source_filename = "dispatch_ver1_interop_prefer_nowait_depend.cpp"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.kmp_depend_info = type { i64, i64, i8 }

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

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z17test1_with_nowaitv() #2 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IMPLICIT"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0),
    "QUAL.OMP.NOWAIT"() ]

  call void @_Z3fooi(i32 noundef 123) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]

; CHECK-LABEL: test1_with_nowait
; CHECK-LABEL: if.then:
; BBlock "if.then" has the code for the variant call
; CHECK: [[TASK1:%[^ ]+]] = call i8* @__kmpc_get_current_task(i32 %my.tid{{.*}})
; CHECK: [[IOP1:%[^ ]+]] = call i8* @__tgt_get_interop_obj(%struct.ident_t* @.kmpc_loc{{.*}}, i32 1, i32 3, i8* bitcast ([3 x i32]* @.prefer.list{{.*}} to i8*), i64 0, i32 %my.tid{{.*}}, i8* [[TASK1]])
; CHECK: call void @_Z7foo_gpuiPv(i32 123, i8* [[IOP1]])

; Since NOWAIT is specified, do not emit the __tgt_target_sync call after the variant call
; CHECK-NEXT: br label %if.end

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.DISPATCH"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

; Function Attrs: mustprogress noinline nounwind optnone uwtable
define dso_local void @_Z17test2_with_dependv() #2 {
entry:
  %aaa = alloca i32, align 4
  %.dep.arr.addr = alloca [1 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  %0 = getelementptr inbounds [1 x %struct.kmp_depend_info], [1 x %struct.kmp_depend_info]* %.dep.arr.addr, i64 0, i64 0
  %1 = ptrtoint i32* %aaa to i64
  %2 = getelementptr %struct.kmp_depend_info, %struct.kmp_depend_info* %0, i64 0
  %3 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %2, i32 0, i32 0
  store i64 %1, i64* %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %2, i32 0, i32 1
  store i64 4, i64* %4, align 8
  %5 = getelementptr inbounds %struct.kmp_depend_info, %struct.kmp_depend_info* %2, i32 0, i32 2
  store i8 3, i8* %5, align 8
  store i64 1, i64* %dep.counter.addr, align 8
  %6 = bitcast %struct.kmp_depend_info* %0 to i8*
  %7 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.IF"(i32 0),
    "QUAL.OMP.IMPLICIT"(),
    "QUAL.OMP.DEPARRAY"(i32 1, i8* %6) ]

  %8 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0) ]

  call void @_Z3fooi(i32 noundef 123) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]

; CHECK-LABEL: test2_with_depend
; CHECK-LABEL: if.then:
; CHECK: [[TASK2:%[^ ]+]] = call i8* @__kmpc_get_current_task(i32 %my.tid{{.*}})
; CHECK: [[IOP2:%[^ ]+]] = call i8* @__tgt_get_interop_obj(%struct.ident_t* @.kmpc_loc{{.*}}, i32 1, i32 3, i8* bitcast ([3 x i32]* @.prefer.list{{.*}} to i8*), i64 0, i32 %my.tid{{.*}}, i8* [[TASK2]])

; For the DEPEND clause emit call to __kmpc_omp_wait_deps but not the task_begin/complete_if0 calls
; CHECK: call void @__kmpc_omp_wait_deps
; CHECK-NOT: call void @__kmpc_omp_task_begin_if0
; CHECK-NOT: call void @__kmpc_omp_task_complete_if0

; CHECK: call void @_Z7foo_gpuiPv(i32 123, i8* [[IOP2]])

; Since NOWAIT is not specified, emit the __tgt_target_sync call after the variant call
; CHECK-NEXT: call void @__tgt_target_sync(%struct.ident_t* @.kmpc_loc{{.*}}, i32 %my.tid{{.*}}, i8* [[TASK2]], i8* null)

  call void @llvm.directive.region.exit(token %8) [ "DIR.OMP.END.DISPATCH"() ]
  call void @llvm.directive.region.exit(token %7) [ "DIR.OMP.END.TASK"() ]
  ret void
}

attributes #0 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z7foo_gpuiPv;construct:dispatch;arch:gen;interop:targetsync,6,4,3" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { mustprogress noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
