; RUN: opt -vpo-paropt-prepare -vpo-paropt-use-interop=false -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -vpo-paropt-use-interop=false -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; Test for TARGET VARIANT DISPATCH construct with a USE_DEVICE_PTR clause

; C Source
; int __attribute__((nothrow)) foo_gpu(void* p1, void* p2, int dummy) {return 456;}
;
; #pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; int __attribute__((nothrow)) foo(void* p1, void* p2, int dummy) {return 123;}
;
; void *a_cpu; float *floatPtr;
; int main() {
;   void *b_cpu = (void *) floatPtr;
;   int rrr;
;   #pragma omp target variant dispatch use_device_ptr(a_cpu, b_cpu)
;      rrr = foo(a_cpu, b_cpu, 77777);
;   return rrr;
; }

; Expected IR: when -vpo-paropt-use-raw-dev-ptr=true
;  %available = call i32 @__tgt_is_device_available(i64 %{{.*}}, i8* inttoptr (i64 15 to i8*))
;  %dispatch = icmp ne i32 %available, 0
;  br i1 %dispatch, label %variant.call, label %base.call
;
;variant.call:                                     ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
;  %a_cpu.load = load i8*, i8** @a_cpu, align 8
;  %b_cpu.load = load i8*, i8** %b_cpu, align 8
;...
;  call void @__tgt_target_data_begin(i64 %{{.*}}, i32 2, i8** %10, i8** %11, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_sizes, i32 0, i32 0), i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes, i32 0, i32 0))
;  %a_cpu.buffer.cast = load i8*, i8** %6, align 8
;  %b_cpu.buffer.cast = load i8*, i8** %8, align 8
;  %variant = call i32 @main.foo_gpu.wrapper(i8* %a_cpu.buffer.cast, i8* %b_cpu.buffer.cast, i32 77777)
;  call void @__tgt_target_data_end(i64 %{{.*}}, i32 2, i8** %10, i8** %11, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_sizes, i32 0, i32 0), i64* getelementptr inbounds ([2 x i64], [2 x i64]* @.offload_maptypes, i32 0, i32 0))
;  br label %if.end
;
;base.call:                                        ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
;  %4 = load i8*, i8** %a_cpu, align 8
;  %5 = load i8*, i8** %b_cpu, align 8
;  %call.clone = call i32 @foo(i8* %4, i8* %5, i32 77777)
;  store i32 %call.clone, i32* %rrr, align 4
;  br label %if.end
;
;....
; define internal void @main.foo_gpu.wrapper(i8** %a_cpu, i8** %b_cpu, i32* %rrr) {
;   %0 = load i8*, i8** %a_cpu, align 8
;   %1 = load i8*, i8** %b_cpu, align 8
;   %variant = call i32 @foo_gpu(i8* %0, i8* %1, i32 77777)
;   store i32 %variant, i32* %rrr, align 4
; }

;Is device ready?
; CHECK: call i32 @__tgt_is_device_available(i64 %{{.*}}, i8* inttoptr (i64 15 to i8*))
; CHECK-NEXT: [[DISPATCH:%[a-zA-Z._0-9]+]] = icmp ne
; CHECK: br i1 [[DISPATCH]], label %[[VARIANTLBL:[a-zA-Z._0-9]+]], label %[[BASELBL:[a-zA-Z._0-9]+]]
;
;Variant Call
; CHECK: [[VARIANTLBL]]:
; CHECK: call void @__tgt_target_data_begin({{.+}})
; CHECK: call void @[[VARIANT_WRAPPER:[^ ]*foo_gpu.wrapper[^ (]*]](i8** %a_cpu.new, i8** %b_cpu.new, i32* %rrr)
; CHECK: call void @__tgt_target_data_end({{.+}})

;Base Call:
; CHECK: [[BASELBL]]:
; CHECK: [[BASE:%[a-zA-Z._0-9]+]] = call i32 @foo(i8*
;
;Variant Wrapper:
; CHECK-DAG: define internal void @[[VARIANT_WRAPPER]](i8** [[A:%[^ ,]+]], i8** [[B:%[^ ]+]], i32* %rrr)
; CHECK-DAG: [[ARG1:%[a-zA-Z._0-9]+]] = load i8*, i8** [[A]]
; CHECK-DAG: [[ARG2:%[a-zA-Z._0-9]+]] = load i8*, i8** [[B]]
; CHECK: [[VARIANT:%[a-zA-Z._0-9]+]] = call i32 @foo_gpu(i8* [[ARG1]], i8* [[ARG2]]

; ModuleID = 'target_variant_dispatch_usedeviceptr_intfunc.c'
source_filename = "target_variant_dispatch_usedeviceptr_intfunc.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@floatPtr = common dso_local global float* null, align 8
@a_cpu = common dso_local global i8* null, align 8

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo_gpu(i8* %p1, i8* %p2, i32 %dummy) #0 {
entry:
  %p1.addr = alloca i8*, align 8
  %p2.addr = alloca i8*, align 8
  %dummy.addr = alloca i32, align 4
  store i8* %p1, i8** %p1.addr, align 8
  store i8* %p2, i8** %p2.addr, align 8
  store i32 %dummy, i32* %dummy.addr, align 4
  ret i32 456
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo(i8* %p1, i8* %p2, i32 %dummy) #1 {
entry:
  %p1.addr = alloca i8*, align 8
  %p2.addr = alloca i8*, align 8
  %dummy.addr = alloca i32, align 4
  store i8* %p1, i8** %p1.addr, align 8
  store i8* %p2, i8** %p2.addr, align 8
  store i32 %dummy, i32* %dummy.addr, align 4
  ret i32 123
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #2 {
entry:
  %retval = alloca i32, align 4
  %b_cpu = alloca i8*, align 8
  %rrr = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  %0 = load float*, float** @floatPtr, align 8
  %1 = bitcast float* %0 to i8*
  store i8* %1, i8** %b_cpu, align 8
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(), "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(i8** @a_cpu, i8** %b_cpu) ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %3 = load i8*, i8** @a_cpu, align 8
  %4 = load i8*, i8** %b_cpu, align 8
  %call = call i32 @foo(i8* %3, i8* %4, i32 77777) #3
  store i32 %call, i32* %rrr, align 4
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  %5 = load i32, i32* %rrr, align 4
  ret i32 %5
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3


attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
