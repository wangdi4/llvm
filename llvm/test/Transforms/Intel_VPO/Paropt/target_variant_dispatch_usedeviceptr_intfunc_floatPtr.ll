; RUN: opt -vpo-paropt-prepare -vpo-paropt-use-interop=false -S < %s | FileCheck %s -check-prefix=BUFFPTR
; RUN: opt < %s -passes='function(vpo-paropt-prepare)' -vpo-paropt-use-interop=false -S | FileCheck %s -check-prefix=BUFFPTR
; RUN: opt -vpo-paropt-prepare -vpo-paropt-use-raw-dev-ptr=true -vpo-paropt-use-interop=false -S < %s | FileCheck %s -check-prefix=RAWPTR
; RUN: opt < %s -passes='function(vpo-paropt-prepare)' -vpo-paropt-use-raw-dev-ptr=true -vpo-paropt-use-interop=false -S | FileCheck %s -check-prefix=RAWPTR
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

; Check that @a_cpu and %b_cpu are sent in directly to the tgt_create_buffer call,
; instead of creating a load from them (like in
; target_variant_dispatch_usedeviceptr_intfunc_floatStarStar.ll).
; BUFFPTR-DAG:[[BCAST:[^ ]+]] = bitcast float* %b_cpu to i8*
; BUFFPTR: [[B_TGT:%[^ ]+]] = call i8* @__tgt_create_buffer(i64 -1, i8* [[BCAST]])
; BUFFPTR: store i8* [[B_TGT]], i8** [[B_TGT_ADDR:%[^ ]+]], align 8
; BUFFPTR: [[A_TGT:%[^ ]+]] = call i8* @__tgt_create_buffer(i64 -1, i8* bitcast (float* @a_cpu to i8*))
; BUFFPTR: store i8* [[A_TGT]], i8** [[A_TGT_ADDR:%[^ ]+]], align 8
; BUFFPTR: variant.call:
; BUFFPTR: [[A_TGT_ADDR_LOAD:%[^ ]+]] = load i8*, i8** [[A_TGT_ADDR]], align 8
; BUFFPTR: [[A_TGT_CAST:%[^ ]+]] = bitcast i8* [[A_TGT_ADDR_LOAD]] to float*
; BUFFPTR: [[B_TGT_ADDR_LOAD:%[^ ]+]] = load i8*, i8** [[B_TGT_ADDR]], align 8
; BUFFPTR: [[B_TGT_CAST:%[^ ]+]] = bitcast i8* [[B_TGT_ADDR_LOAD]] to float*
; BUFFPTR: call i32 @foo_gpu(float* [[A_TGT_CAST]], float* [[B_TGT_CAST]], i32 77777)


; If -vpo-paropt-use-raw-dev-ptr is true, check that the device pointers for @a_cpu and %b_cpu,
; are obtained using a tgt_target_data_begin call. The map-type should be TGT_PARAM | TGT_RETURN_PARAM (96).
; RAWPTR: @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 96, i64 96]
; RAWPTR: [[A_GEP:%[^ ]+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 0
; RAWPTR: store i8* bitcast (float* @a_cpu to i8*), i8** %0, align 8
; RAWPTR: [[B_CAST:%[^ ]+]] = bitcast float* %b_cpu to i8*
; RAWPTR: [[B_GEP:%[^ ]+]] = getelementptr inbounds [2 x i8*], [2 x i8*]* %.offload_baseptrs, i32 0, i32 1
; RAWPTR: store i8* [[B_CAST]], i8** [[B_GEP]], align 8
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

@a_cpu = common dso_local global float 0.0, align 8

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
  %b_cpu.addr = alloca float*, align 8
  %rrr = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %b_cpu = load float*, float** %b_cpu.addr, align 8
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(), "QUAL.OMP.USE_DEVICE_PTR"(float* @a_cpu, float* %b_cpu) ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %call = call i32 @foo(float* @a_cpu, float* %b_cpu, i32 77777) #3
  store i32 %call, i32* %rrr, align 4
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  %1 = load i32, i32* %rrr, align 4
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
