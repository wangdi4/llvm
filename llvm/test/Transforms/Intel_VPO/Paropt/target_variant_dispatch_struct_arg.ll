; RUN: opt -bugpoint-enable-legacy-pm -vpo-paropt-prepare -S %s | FileCheck %s
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
;    %available = call i32 @__tgt_is_device_available(i64 0, ptr inttoptr (i64 15 to ptr))
;    %dispatch = icmp ne i32 %available, 0
;    br label %dispatch.check
;
; dispatch.check:
;    br i1 %dispatch, label %base.call, label %variant.call

; variant.call:
;    call void @foo.foo_offload.wrapper(...)
;    br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

;  base.call:
;    call void @bar(i64 undef, i32 undef) #2
;    br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4
;
; define internal void @foo.foo_offload.wrapper(...) {
;    ...
;    %interop.obj.sync = call ptr @__tgt_create_interop_obj(i64 0, i8 0, ptr null)
;    call void @foo_offload(i64 undef, i32 undef, ptr %interop.obj.sync)
;    %0 = call i32 @__tgt_release_interop_obj(ptr %interop.obj.sync)
;    ...
; }
;
; CHECK: [[AVAIL:%[a-zA-Z._0-9]+]] = call i32 @__tgt_is_device_available(i64 0, ptr inttoptr (i64 15 to ptr))
; CHECK-NEXT: [[DISPATCH:%[a-zA-Z._0-9]+]] = icmp ne i32 [[AVAIL]], 0
; CHECK: br i1 [[DISPATCH]], label %[[VARIANTLBL:[a-zA-Z._0-9]+]], label %[[BASELBL:[a-zA-Z._0-9]+]]

; CHECK-DAG: [[VARIANTLBL]]:
; CHECK: call void @[[VARIANT_WRAPPER:[^ ]*.foo_offload.wrapper[^ (]*]]

; CHECK-DAG: [[BASELBL]]:
; CHECK: call void @bar

; CHECK-DAG: define internal void @[[VARIANT_WRAPPER]]
; CHECK: {{%[a-zA-Z._0-9]+}} = call ptr @__tgt_create_interop_obj
; CHECK-NEXT: call void @foo_offload

source_filename = "target_variant_dispatch_struct_arg.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%struct.matrix_descr = type { i32, i32, i32 }

define dso_local void @foo() {
entry:
  %descrA = alloca %struct.matrix_descr, align 4
  %descrA.coerce = alloca { i64, i32 }, align 4
  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.1

DIR.OMP.TARGET.VARIANT.DISPATCH.1:                ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"(),
    "QUAL.OMP.DEVICE"(i32 0) ]

  br label %DIR.OMP.TARGET.VARIANT.DISPATCH.2

DIR.OMP.TARGET.VARIANT.DISPATCH.2:                ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.1
  call void @llvm.memcpy.p0.p0.i64(ptr align 4 %descrA.coerce, ptr align 4 %descrA, i64 12, i1 false)
  %1 = getelementptr inbounds { i64, i32 }, ptr %descrA.coerce, i32 0, i32 0
  %2 = load i64, ptr %1, align 4
  %3 = getelementptr inbounds { i64, i32 }, ptr %descrA.coerce, i32 0, i32 1
  %4 = load i32, ptr %3, align 4
  call void @bar(i64 %2, i32 %4) #1 [ "QUAL.OMP.DISPATCH.CALL"() ]
  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3

DIR.OMP.END.TARGET.VARIANT.DISPATCH.3:            ; preds = %DIR.OMP.TARGET.VARIANT.DISPATCH.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]

  br label %DIR.OMP.END.TARGET.VARIANT.DISPATCH.4

DIR.OMP.END.TARGET.VARIANT.DISPATCH.4:            ; preds = %DIR.OMP.END.TARGET.VARIANT.DISPATCH.3
  ret void
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare dso_local void @bar(i64, i32) #2

declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg)

attributes #2 = { "openmp-variant"="name:foo_offload;construct:target_variant_dispatch;arch:gen" }

