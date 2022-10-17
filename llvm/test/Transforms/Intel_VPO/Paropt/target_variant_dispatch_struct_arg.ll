; RUN: opt -vpo-paropt-prepare -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-paropt-prepare)' -S %s | FileCheck %s
; Test for TARGET VARIANT DISPATCH with struct argument in the construct
;
; C Source;
;
; struct matrix_descr { int type; int mode; int diag; };
;
; void foo_offload(const struct matrix_descr descr);
;
; #pragma omp declare variant (foo_offload) \
;    match(construct={target variant dispatch}, device={arch(gen)})
; void bar(const struct matrix_descr descr);
;
; void foo()
; {
;   struct matrix_descr descrA;
;
;   #pragma omp target variant dispatch device(0)
;     bar(descrA);
;   return;
; }
;
; The dispatch code looks like this:
;    %available = call i32 @__tgt_is_device_available(i64 0, i8* inttoptr (i64 15 to i8*))
;    %dispatch = icmp ne i32 %available, 0
;    br label %dispatch.check
;
; dispatch.check:
;    br i1 %dispatch, label %base.call, label %variant.call

; variant.call:
;    call void @foo.foo_offload.wrapper(...)
;    br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

;  base.call:
;    tail call void @bar(i64 undef, i32 undef) #2
;    br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4
;
; define internal void @foo.foo_offload.wrapper(...) {
;    ...
;    %interop.obj.sync = tail call i8* @__tgt_create_interop_obj(i64 0, i8 0, i8* null)
;    tail call void @foo_offload(i64 undef, i32 undef, i8* %interop.obj.sync)
;    %0 = tail call i32 @__tgt_release_interop_obj(i8* %interop.obj.sync)
;    ...
; }
;
;   DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:

; CHECK: [[AVAIL:%[a-zA-Z._0-9]+]] = call i32 @__tgt_is_device_available(i64 0, i8* inttoptr (i64 15 to i8*))
; CHECK-NEXT: [[DISPATCH:%[a-zA-Z._0-9]+]] = icmp ne i32 [[AVAIL]], 0
; CHECK: br i1 [[DISPATCH]], label %[[VARIANTLBL:[a-zA-Z._0-9]+]], label %[[BASELBL:[a-zA-Z._0-9]+]]

; CHECK-DAG: [[VARIANTLBL]]:
; CHECK: call void @[[VARIANT_WRAPPER:[^ ]*.foo_offload.wrapper[^ (]*]]

; CHECK-DAG: [[BASELBL]]:
; CHECK: call void @bar

; CHECK-DAG: define internal void @[[VARIANT_WRAPPER]]
; CHECK: {{%[a-zA-Z._0-9]+}} = call i8* @__tgt_create_interop_obj
; CHECK-NEXT: call void @foo_offload

; ModuleID = '/target_variant_dispatch_struct_arg.o'
source_filename = "target_variant_dispatch_struct_arg.o"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.matrix_descr = type { i32, i32, i32 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 {
entry:
  %descrA = alloca %struct.matrix_descr, align 4
  %descrA.coerce = alloca { i64, i32 }, align 4
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(), "QUAL.OMP.DEVICE"(i32 0) ]
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  %1 = bitcast { i64, i32 }* %descrA.coerce to i8*
  %2 = bitcast %struct.matrix_descr* %descrA to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %1, i8* align 4 %2, i64 12, i1 false)
  %3 = getelementptr inbounds { i64, i32 }, { i64, i32 }* %descrA.coerce, i32 0, i32 0
  %4 = load i64, i64* %3, align 4
  %5 = getelementptr inbounds { i64, i32 }, { i64, i32 }* %descrA.coerce, i32 0, i32 1
  %6 = load i32, i32* %5, align 4
  call void @bar(i64 %4, i32 %6) #1
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  ret void
}

declare dso_local void @bar(i64, i32) local_unnamed_addr #1

declare token @llvm.directive.region.entry()

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg)

declare void @llvm.directive.region.exit(token)

declare i32 @__tgt_is_device_available(i64, i8*) local_unnamed_addr

declare i8* @__tgt_create_interop_obj(i64, i8, i8*) local_unnamed_addr

declare void @foo_offload(i64, i32, i8*) local_unnamed_addr

declare i32 @__tgt_release_interop_obj(i8*) local_unnamed_addr

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "may-have-openmp-directive"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "openmp-variant"="name:foo_offload;construct:target_variant_dispatch;arch:gen" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 9.0.0"}
