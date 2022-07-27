; RUN: opt -vpo-paropt-prepare -vpo-paropt-use-interop=false -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -vpo-paropt-use-interop=false -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; Test for TARGET VARIANT DISPATCH construct with a USE_DEVICE_PTR clause
;
; This test is similar to target_variant_dispatch_usedeviceptr_intfunc.ll
; except that the pointers passed to foo() and foo_gpu() are float* instead
; of void*. The IR is similar, except that in this case the compiler generates
; bitcast instructions (from void* to float*) when emitting the variant call.

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

; Check that tgt_target_data_begin/end are used to obtain
; the device pointers for @a_cpu and %b_cpu, and the map-type for target data is
; TGT_RETURN_PARAM (64):

;  %a_cpu.cast = bitcast i8** %5 to float**
;  %b_cpu.cast = bitcast i8** %9 to float**
;  %a_cpu.buffer.cast = load float*, float** %a_cpu.cast, align 8
;  %b_cpu.buffer.cast = load float*, float** %b_cpu.cast, align 8
;  %variant = call i32 @foo_gpu(float* %a_cpu.buffer.cast, float* %b_cpu.buffer.cast, i32 77777)

; CHECK: @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 64, i64 64]

; Check that the maps are created on loads of use_device_ptr operands.
; CHECK: [[A_CAST:%[^ ]+]] = bitcast i8* bitcast (float** @a_cpu to i8*) to float**
; CHECK: [[A_LOAD:%[^ ]+]] = load float*, float** [[A_CAST]], align 8
; CHECK: [[B_LOAD:%[^ ]+]] = load float*, float** %b_cpu, align 8
; CHECK: [[A_LOAD_CAST:%[^ ]+]] = bitcast float* [[A_LOAD]] to i8*
; CHECK: [[A_GEP:%[^ ]+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
; CHECK: store i8* [[A_LOAD_CAST]], i8** [[A_GEP]], align 8
; CHECK: [[B_LOAD_CAST:%[^ ]+]] = bitcast float* [[B_LOAD]] to i8*
; CHECK: [[B_GEP:%[^ ]+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 1
; CHECK: store i8* [[B_LOAD_CAST]], i8** [[B_GEP]], align 8
; CHECK: call void @__tgt_target_data_begin({{.*}})

; Check that updated values for a and b are passed to the outlined region
; created around the variant function
; CHECK: [[A_GEP_CAST:%[^ ]+]] = bitcast i8** [[A_GEP]] to float**
; CHECK: [[A_UPDATED:%[^ ]+]] = load float*, float** [[A_GEP_CAST]], align 8
; CHECK: store float* [[A_UPDATED]], float** [[A_NEW:%[^ ,]+]], align 8
; CHECK: [[B_GEP_CAST:%[^ ]+]] = bitcast i8** [[B_GEP]] to float**
; CHECK: [[B_UPDATED:%[^ ]+]] = load float*, float** [[B_GEP_CAST]], align 8
; CHECK: store float* [[B_UPDATED]], float** [[B_NEW:%[^ ,]+]], align 8
; CHECK: call void @[[VARIANT_WRAPPER:[^ ]*foo_gpu.wrapper[^ (]*]](float** [[A_NEW]], float** [[B_NEW]], i32* %rrr)
; CHECK: call void @__tgt_target_data_end({{.*}})

; Check that variant function is called in the variant wrapper.
; CHECK-DAG: define internal void @[[VARIANT_WRAPPER]](float** [[A1:%a_cpu[^, ]*]], float** [[B1:%b_cpu[^ ,]*]], i32* %rrr)
; CHECK: [[A_VAL:%[^ ]+]] = load float*, float** [[A1]], align 8
; CHECK: [[B_VAL:%[^ ]+]] = load float*, float** [[B1]], align 8
; CHECK: [[RET_VAL:%[^ ]+]] = call i32 @foo_gpu(float* [[A_VAL]], float* [[B_VAL]], i32 77777)
; CHECK: store i32 [[RET_VAL]], i32* %rrr

source_filename = "target_variant_dispatch_usedeviceptr_intfunc_floatStar.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-unknown-linux-gnu"

@a_cpu = common dso_local global float* null, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo_gpu(float* %p1, float* %p2, i32 %dummy) #0 {
entry:
  %p1.addr = alloca float*, align 8
  %p2.addr = alloca float*, align 8
  %dummy.addr = alloca i32, align 4
  store float* %p1, float** %p1.addr, align 8
  store float* %p2, float** %p2.addr, align 8
  store i32 %dummy, i32* %dummy.addr, align 4
  ret i32 456
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo(float* %p1, float* %p2, i32 %dummy) #1 {
entry:
  %p1.addr = alloca float*, align 8
  %p2.addr = alloca float*, align 8
  %dummy.addr = alloca i32, align 4
  store float* %p1, float** %p1.addr, align 8
  store float* %p2, float** %p2.addr, align 8
  store i32 %dummy, i32* %dummy.addr, align 4
  ret i32 123
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %b_cpu = alloca float*, align 8
  %rrr = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(), "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(float** @a_cpu, float** %b_cpu) ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %1 = load float*, float** @a_cpu, align 8
  %2 = load float*, float** %b_cpu, align 8
  %call = call i32 @foo(float* %1, float* %2, i32 77777) #3
  store i32 %call, i32* %rrr, align 4
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  %3 = load i32, i32* %rrr, align 4
  ret i32 %3
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
