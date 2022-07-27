; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s | FileCheck %s

; // C++ source:
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
; CHECK: [[ASYNCOBJ:%[a-zA-Z._0-9]+]] = call i8* @__kmpc_omp_task_alloc(%struct.ident_t* {{.*}}, i32 0, i32 16, i64 24, i64 0, i8* null)
; CHECK: [[INTEROPASYNC:%[a-zA-Z._0-9]+]] = call i8* @__tgt_create_interop_obj(i64 0, i8 1, i8* [[ASYNCOBJ]])
;
; ;;; handle NEED_DEVICE_PTR
; ;CHECK: call void @__tgt_target_data_begin_mapper
; ;CHECK: [[CAST:%[a-zA-Z._0-9]+]] = bitcast i8** %{{.*}} to i32**
; ;CHECK: [[UPDATEDVAL:%[a-zA-Z._0-9]+]] = load i32*, i32** [[CAST]], align 8
; ;
; ; ;;;; handle DEPEND
; ; ;CHECK: call i8* @__kmpc_omp_task_alloc(%struct.ident_t* @{{.*}}, i32 0, i32 0, i64 0, i64 0, i8* null)
; ; ;CHECK: call void @__kmpc_omp_wait_deps(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i32 2, i8* %{{.*}}, i32 0, i8* null)
; ; ;CHECK: call void @__kmpc_omp_task_begin_if0(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i8* %{{.*}})
; ; ;
; ; ;   ;;; variant call foo_gpu(0, ptr, interop)
; ; ;   CHECK: call void @_Z7foo_gpuiPiPv(i32 0, i32* [[UPDATEDVAL]], i8* [[INTEROPASYNC]])
; ; ;
; ; ;CHECK: call void @__kmpc_omp_task_complete_if0(%struct.ident_t* @{{.*}}, i32 %{{.*}}, i8* %{{.*}})
; ;
; ;CHECK: call void @__tgt_target_data_end_mapper


; ModuleID = 'dispatch_ndp_depend_nowait_interop.cpp'
source_filename = "dispatch_ndp_depend_nowait_interop.cpp"
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
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.IMPLICIT"(), "QUAL.OMP.DEPEND.IN"(i32* %ccc), "QUAL.OMP.DEPEND.OUT"(i32* %ddd), "QUAL.OMP.SHARED"(i32** %ptr) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.DISPATCH"(), "QUAL.OMP.DEVICE"(i32 0), "QUAL.OMP.NOWAIT"() ]
  %2 = load i32*, i32** %ptr, align 8
  call void @_Z3fooiPi(i32 0, i32* %2) #3 [ "QUAL.OMP.DISPATCH.CALL"() ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.DISPATCH"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret i32 0
}

; Function Attrs: noinline nounwind optnone uwtable mustprogress
define dso_local void @_Z7foo_gpuiPiPv(i32 %aaa, i32* %bbb, i8* %interop) #0 {
entry:
  %aaa.addr = alloca i32, align 4
  %bbb.addr = alloca i32*, align 8
  %interop.addr = alloca i8*, align 8
  store i32 %aaa, i32* %aaa.addr, align 4
  store i32* %bbb, i32** %bbb.addr, align 8
  store i8* %interop, i8** %interop.addr, align 8
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
attributes #1 = { noinline nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "openmp-variant"="name:_Z7foo_gpuiPiPv;construct:dispatch;arch:gen;need_device_ptr:F,T;interop:targetsync" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { noinline norecurse nounwind optnone uwtable mustprogress "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
