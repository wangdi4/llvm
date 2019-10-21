; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -S < %s | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S | FileCheck %s

; Original code:
; void __attribute__((nothrow)) foo_gpu(void *ptr);
; #pragma omp declare variant(foo_gpu) match(construct={target variant dispatch}, device={arch(gen)})
; void __attribute__((nothrow)) foo(void *ptr);

; void bar() {
;   float *host_ptr;
; #pragma omp target variant dispatch use_device_ptr(host_ptr)
;   foo(host_ptr);
; }

; CHECK: [[BUFFER:%[a-zA-Z._0-9]+]] = call i8* @__tgt_create_buffer(i64 -1, i8* [[PTR:%[a-zA-Z._0-9]]])
; CHECK: store i8* [[BUFFER]], i8** [[TGT_BUFFER:%[a-zA-Z._0-9]+]]

; Check that the host pointer casted from float* to i8* is not directly used in the call:
; CHECK-NOT: call void @foo_gpu(i8* [[PTR]])

; CHECK-DAG: [[LOAD1:%[a-zA-Z._0-9]+]] = load i8*, i8** [[TGT_BUFFER]]
; CHECK-DAG: icmp ne i8* [[LOAD1]], null

; CHECK-DAG: [[LOAD2:%[a-zA-Z._0-9]+]] = load i8*, i8** [[TGT_BUFFER]]
; CHECK-DAG: call void @foo_gpu(i8* [[LOAD2]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

; Function Attrs: nounwind uwtable
define dso_local void @bar() #0 {
entry:
  %host_ptr = alloca float*, align 8
  %0 = bitcast float** %host_ptr to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %0) #2
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(), "QUAL.OMP.USE_DEVICE_PTR"(float** %host_ptr) ]
  %2 = load float*, float** %host_ptr, align 8, !tbaa !2
  %3 = bitcast float* %2 to i8*
  call void @foo(i8* %3) #2
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  %4 = bitcast float** %host_ptr to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %4) #2
  ret void
}

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: nounwind
declare dso_local void @foo(i8*) #3

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
!2 = !{!3, !3, i64 0}
!3 = !{!"pointer@_ZTSPf", !4, i64 0}
!4 = !{!"omnipotent char", !5, i64 0}
!5 = !{!"Simple C/C++ TBAA"}
