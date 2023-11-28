; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-prepare -vpo-paropt-use-interop=false -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -vpo-paropt-use-interop=false -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; Test for TARGET VARIANT DISPATCH construct with a USE_DEVICE_PTR clause
;
; This test has hand-modified version of the test
; target_variant_dispatch_usedeviceptr_intfunc_floatPtrPtr.ll with float*
; operand to the clause, instead of an float**.

; C Source
; int __attribute__((nothrow)) foo_gpu(float* p1, float* p2, int dummy) { return 456; }
;
; #pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; int __attribute__((nothrow)) foo(float* p1, float* p2, int dummy) { return 123; }
;
; float *a_cpu;
; int main() {
;   float *b_cpu;
;   int rrr;
;   #pragma omp target variant dispatch use_device_ptr(a_cpu, b_cpu)
;      rrr = foo(a_cpu, b_cpu, 77777);
;   return rrr;
; }
;

; Check that the device pointers for @a_cpu and %b_cpu, are obtained using a
; tgt_target_data_begin call. The map-type should be TGT_RETURN_PARAM (64).

; CHECK: @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 64, i64 64]

; Check for the maps used to get device pointers for a and b
; CHECK: [[A_GEP:%[^ ]+]] = getelementptr inbounds [2 x ptr], ptr %.offload_baseptrs, i32 0, i32 0
; CHECK: store ptr @a_cpu, ptr [[A_GEP]], align 8

; CHECK: [[B_GEP:%[^ ]+]] = getelementptr inbounds [2 x ptr], ptr %.offload_baseptrs, i32 0, i32 1
; CHECK: store ptr %b_cpu, ptr [[B_GEP]], align 8
; CHECK: call void @__tgt_target_data_begin({{.*}})

; Check that updated values for a and b are passed to the outlined region
; created around the variant function
; CHECK: [[A_UPDATED:%[^ ]+]] = load ptr, ptr [[A_GEP]], align 8
; CHECK: [[B_UPDATED:%[^ ]+]] = load ptr, ptr [[B_GEP]], align 8
; CHECK: call void @[[VARIANT_WRAPPER:[^ ]*foo_gpu.wrapper[^ (]*]](ptr [[A_UPDATED]], ptr [[B_UPDATED]], ptr %rrr)
; CHECK: call void @__tgt_target_data_end({{.*}})

; Check that variant function is called in the variant wrapper.
; CHECK-DAG: define internal void @[[VARIANT_WRAPPER]](ptr [[A1:%a_cpu[^, ]*]], ptr [[B1:%b_cpu[^ ,]*]], ptr %rrr)
; CHECK: [[RET_VAL:%[^ ]+]] = call i32 @foo_gpu(ptr [[A1]], ptr [[B1]], i32 77777)
; CHECK: store i32 [[RET_VAL]], ptr %rrr

source_filename = "target_variant_dispatch_usedeviceptr_intfunc_floatStar.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-unknown-linux-gnu"

@a_cpu = common dso_local global float 0.000000e+00, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo_gpu(ptr %p1, ptr %p2, i32 %dummy) #0 {
entry:
  %p1.addr = alloca ptr, align 8
  %p2.addr = alloca ptr, align 8
  %dummy.addr = alloca i32, align 4
  store ptr %p1, ptr %p1.addr, align 8
  store ptr %p2, ptr %p2.addr, align 8
  store i32 %dummy, ptr %dummy.addr, align 4
  ret i32 456
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo(ptr %p1, ptr %p2, i32 %dummy) #1 {
entry:
  %p1.addr = alloca ptr, align 8
  %p2.addr = alloca ptr, align 8
  %dummy.addr = alloca i32, align 4
  store ptr %p1, ptr %p1.addr, align 8
  store ptr %p2, ptr %p2.addr, align 8
  store i32 %dummy, ptr %dummy.addr, align 4
  ret i32 123
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %b_cpu.addr = alloca ptr, align 8
  %rrr = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %b_cpu = load ptr, ptr %b_cpu.addr, align 8
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(),
    "QUAL.OMP.USE_DEVICE_PTR"(ptr @a_cpu, ptr %b_cpu) ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %call = call i32 @foo(ptr @a_cpu, ptr %b_cpu, i32 77777) #3
  store i32 %call, ptr %rrr, align 4
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  %1 = load i32, ptr %rrr, align 4
  ret i32 %1
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
