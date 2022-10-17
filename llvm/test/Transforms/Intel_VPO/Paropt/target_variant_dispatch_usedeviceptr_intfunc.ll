; RUN: opt -enable-new-pm=0 -vpo-paropt-prepare -vpo-paropt-use-interop=false -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -vpo-paropt-use-interop=false -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; Test for TARGET VARIANT DISPATCH construct with a USE_DEVICE_PTR clause

; Test src:
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

; Expected IR:
;  %4 = call i32 @__tgt_is_device_available(i64 %3, ptr inttoptr (i64 15 to ptr))
;  %available = icmp ne i32 %4, 0
;  br i1 %dispatch, label %variant.call, label %base.call
;
;variant.call:                                     ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
;  %a_cpu.load = load ptr, ptr @a_cpu, align 8
;  %b_cpu.load = load ptr, ptr %b_cpu, align 8
;...
;  call void @__tgt_target_data_begin(i64 %12, i32 2, ptr %9, ptr %10, ptr @.offload_sizes, ptr @.offload_maptypes)
;  %a_cpu.updated.val = load ptr, ptr %5, align 8
;  store ptr %a_cpu.updated.val, ptr %a_cpu.new, align 8
;  %b_cpu.updated.val = load ptr, ptr %7, align 8
;  store ptr %b_cpu.updated.val, ptr %b_cpu.new, align 8
;  call void @main.foo_gpu.wrapper(ptr %a_cpu.new, ptr %b_cpu.new, ptr %rrr)
;  call void @__tgt_target_data_end(i64 %14, i32 2, ptr %9, ptr %10, ptr @.offload_sizes, ptr @.offload_maptypes)
;  br label %if.end
;
;base.call:                                        ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
;  %15 = load ptr, ptr @a_cpu, align 8
;  %16 = load ptr, ptr %b_cpu, align 8
;  %call.clone = call i32 @foo(ptr %15, ptr %16, i32 77777)
;  store i32 %call.clone, ptr %rrr, align 4
;  br label %if.end
;
;....
; define internal void @main.foo_gpu.wrapper(ptr %a_cpu, ptr %b_cpu, ptr %rrr) {
;   %0 = load ptr, ptr %a_cpu, align 8
;   %1 = load ptr, ptr %b_cpu, align 8
;   %variant = call i32 @foo_gpu(ptr %0, ptr %1, i32 77777)
;   store i32 %variant, ptr %rrr, align 4
; }

;Is device ready?
; CHECK: call i32 @__tgt_is_device_available(i64 %{{.*}}, ptr inttoptr (i64 15 to ptr))
; CHECK-NEXT: [[DISPATCH:%[a-zA-Z._0-9]+]] = icmp ne
; CHECK: br i1 [[DISPATCH]], label %[[VARIANTLBL:[a-zA-Z._0-9]+]], label %[[BASELBL:[a-zA-Z._0-9]+]]
;
;Variant Call
; CHECK: [[VARIANTLBL]]:
; CHECK: call void @__tgt_target_data_begin({{.+}})
; CHECK: call void @[[VARIANT_WRAPPER:[^ ]*foo_gpu.wrapper[^ (]*]](ptr %a_cpu.new, ptr %b_cpu.new, ptr %rrr)
; CHECK: call void @__tgt_target_data_end({{.+}})

;Base Call:
; CHECK: [[BASELBL]]:
; CHECK: [[BASE:%[a-zA-Z._0-9]+]] = call i32 @foo(ptr
;
;Variant Wrapper:
; CHECK-DAG: define internal void @[[VARIANT_WRAPPER]](ptr [[A:%[^ ,]+]], ptr [[B:%[^ ]+]], ptr %rrr)
; CHECK-DAG: [[ARG1:%[a-zA-Z._0-9]+]] = load ptr, ptr [[A]]
; CHECK-DAG: [[ARG2:%[a-zA-Z._0-9]+]] = load ptr, ptr [[B]]
; CHECK: [[VARIANT:%[a-zA-Z._0-9]+]] = call i32 @foo_gpu(ptr [[ARG1]], ptr [[ARG2]]

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@floatPtr = common dso_local global ptr null, align 8
@a_cpu = common dso_local global ptr null, align 8

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
  %b_cpu = alloca ptr, align 8
  %rrr = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  %0 = load ptr, ptr @floatPtr, align 8
  %1 = bitcast ptr %0 to ptr
  store ptr %1, ptr %b_cpu, align 8
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(),
    "QUAL.OMP.USE_DEVICE_PTR:PTR_TO_PTR"(ptr @a_cpu, ptr %b_cpu) ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %3 = load ptr, ptr @a_cpu, align 8
  %4 = load ptr, ptr %b_cpu, align 8
  %call = call i32 @foo(ptr %3, ptr %4, i32 77777) #3
  store i32 %call, ptr %rrr, align 4
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  %5 = load i32, ptr %rrr, align 4
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

!0 = !{i32 1, !"wchar_size", i32 4}
