; RUN: opt -vpo-paropt-prepare -S -vpo-paropt-use-interop=false < %s | FileCheck %s  -check-prefix=BUFFPTR
; RUN: opt < %s -passes='function(vpo-paropt-prepare)' -S -vpo-paropt-use-interop=false | FileCheck %s  -check-prefix=BUFFPTR
; RUN: opt -vpo-paropt-prepare -vpo-paropt-use-raw-dev-ptr=true -vpo-paropt-use-interop=false -S < %s | FileCheck %s -check-prefix=RAWPTR
; RUN: opt < %s -passes='function(vpo-paropt-prepare)' -vpo-paropt-use-raw-dev-ptr=true -vpo-paropt-use-interop=false -S | FileCheck %s -check-prefix=RAWPTR
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
;
; When -vpo-paropt-use-raw-dev-ptr=false which is default
; See comment in target_variant_dispatch_usedeviceptr_intfunc.ll for the
; expected IR. The main differences here are the extra bitcast instructions
; in the variant.call BasicBlock:
;
; variant.call:                                     ; preds = %end.if.device.available.create.buffers
;   %buffer13 = load i8*, i8** %tgt.buffer
;   %buffer.cast = bitcast i8* %buffer13 to float*
;   %buffer14 = load i8*, i8** %tgt.buffer2
;   %buffer.cast15 = bitcast i8* %buffer14 to float*
;   %variant = call i32 @foo_gpu(float* %buffer.cast, float* %buffer.cast15, i32 77777)
;   ...

; BUFFPTR: variant.call:
; BUFFPTR-DAG: [[CAST1:%[a-zA-Z._0-9]+]] = bitcast i8* %{{.*}} to float*
; BUFFPTR-DAG: [[CAST2:%[a-zA-Z._0-9]+]] = bitcast i8* %{{.*}} to float*
; BUFFPTR: call i32 @foo_gpu(float* [[CAST1]], float* [[CAST2]]
;
; When -vpo-paropt-use-raw-dev-ptr=true, tgt_target_data_begin/end are used to obtain
; the device pointers for @a_cpu and %b_cpu, and the map-type for target data is
; TGT_PARAM | TGT_RETURN_PARAM (96):

;  %a_cpu.cast = bitcast i8** %5 to float**
;  %b_cpu.cast = bitcast i8** %9 to float**
;  %a_cpu.buffer.cast = load float*, float** %a_cpu.cast, align 8
;  %b_cpu.buffer.cast = load float*, float** %b_cpu.cast, align 8
;  %variant = call i32 @foo_gpu(float* %a_cpu.buffer.cast, float* %b_cpu.buffer.cast, i32 77777)

; RAWPTR: @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 96, i64 96]
; RAWPTR: variant.call:
; RAWPTR: [[A_LOAD:%[^ ]+]] = load float*, float** @a_cpu, align 8
; RAWPTR: [[B_LOAD:%[^ ]+]] = load float*, float** %b_cpu, align 8
; RAWPTR: [[A_LOAD_CAST:%[^ ]+]] = bitcast float* [[A_LOAD]] to i8*
; RAWPTR: [[A_GEP:%[^ ]+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
; RAWPTR: store i8* [[A_LOAD_CAST]], i8** [[A_GEP]], align 8
; RAWPTR: [[B_LOAD_CAST:%[^ ]+]] = bitcast float* [[B_LOAD]] to i8*
; RAWPTR: [[B_GEP:%[^ ]+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 1
; RAWPTR: store i8* [[B_LOAD_CAST]], i8** [[B_GEP]], align 8
; RAWPTR: call void @__tgt_target_data_begin({{.*}})
; RAWPTR: [[A_GEP_CAST:%[^ ]+]] = bitcast i8** [[A_GEP]] to float**
; RAWPTR: [[B_GEP_CAST:%[^ ]+]] = bitcast i8** [[B_GEP]] to float**
; RAWPTR: [[A_UPDATED:%[^ ]+]] = load float*, float** [[A_GEP_CAST]], align 8
; RAWPTR: [[B_UPDATED:%[^ ]+]] = load float*, float** [[B_GEP_CAST]], align 8
; RAWPTR: call i32 @foo_gpu(float* [[A_UPDATED]], float* [[B_UPDATED]], i32 77777)
; RAWPTR: call void @__tgt_target_data_end({{.*}})

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
