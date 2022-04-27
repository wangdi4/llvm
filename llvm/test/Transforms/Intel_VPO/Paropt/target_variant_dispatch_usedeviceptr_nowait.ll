; RUN: opt -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -S %s | FileCheck %s
; Test for TARGET VARIANT DISPATCH NOWAIT with USE_DEVICE_PTR clause
; Presence of a NOWAIT clause implies asynchronous execution

; // C Source
; int __attribute__((nothrow)) foo_gpu(float* p1, double* p2, void* interop_obj) { return 456; }
;
; #pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; int __attribute__((nothrow)) foo(float* p1, double* p2) { return 123; }
;
; float  *aaa;
; double *bbb;
; int main() {
;   int rrr;
;   #pragma omp target variant dispatch use_device_ptr(aaa, bbb) nowait
;      rrr = foo(aaa, bbb);
;   return rrr;
; }

; The IR of the BBlock with the call to the variant function is different
; from the synchronous case in the first implementation. Main differences:
;   1. Create and initialize the AsyncObj
;   2. Create the InteropObj and pass it as last argument to variant function

; 1. Allocate AsyncObj (type: %__struct.AsyncObj) and create a pointer to it
; CHECK: [[ASYNCOBJ:%[a-zA-Z._0-9]+]] = call i8* @__kmpc_omp_task_alloc(%struct.ident_t* @.kmpc_loc{{.*}}, i32 0, i32 16, i64 24, i64 0, i8* null)
; CHECK: [[ASYNCPTR:%[a-zA-Z._0-9]+]] = bitcast i8* [[ASYNCOBJ]] to %__struct.AsyncObj*

; 2a. Create the InteropObj
; CHECK: [[INTEROPOBJ:%[a-zA-Z._0-9]+]] = call i8* @__tgt_create_interop_obj(i64 %{{.*}}, i8 1, i8* [[ASYNCOBJ]])

; 2b. The InteropObj becomes the last argument of the call to foo_gpu
; CHECK: call i32 @foo_gpu(float* %{{.*}}, double* %{{.*}}, i8* [[INTEROPOBJ]])

source_filename = "target_variant_dispatch_usedeviceptr_nowait.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@aaa = common dso_local global float* null, align 8
@bbb = common dso_local global double* null, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %rrr = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(), "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(float** @aaa, double** @bbb), "QUAL.OMP.NOWAIT"() ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %1 = load float*, float** @aaa, align 8
  %2 = load double*, double** @bbb, align 8
  %call = call i32 @foo(float* %1, double* %2) #3
  store i32 %call, i32* %rrr, align 4
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  %3 = load i32, i32* %rrr, align 4
  ret i32 %3
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo_gpu(float* %p1, double* %p2, i8* %interop_obj) #0 {
entry:
  %p1.addr = alloca float*, align 8
  %p2.addr = alloca double*, align 8
  %interop_obj.addr = alloca i8*, align 8
  store float* %p1, float** %p1.addr, align 8
  store double* %p2, double** %p2.addr, align 8
  store i8* %interop_obj, i8** %interop_obj.addr, align 8
  ret i32 456
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo(float* %p1, double* %p2) #1 {
entry:
  %p1.addr = alloca float*, align 8
  %p2.addr = alloca double*, align 8
  store float* %p1, float** %p1.addr, align 8
  store double* %p2, double** %p2.addr, align 8
  ret i32 123
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
