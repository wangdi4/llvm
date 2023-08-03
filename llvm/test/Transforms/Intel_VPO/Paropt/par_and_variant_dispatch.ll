; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare)' -S %s 2>&1 | FileCheck %s

; Check that we can successfully outline the variant region,
; and code-extractor doesn't complain about DT being invalid.
; CHECK-NOT: DominatorTree is different than a freshly computed one

; CHECK: define internal void @{{[^ ]*}}foo_gpu.wrapper{{[^ (]*}}(i64 %0)
; CHECK: %[[INTEROP:[^ ]+]] = call ptr @__tgt_create_interop_obj({{.*}})
; CHECK: call void @foo_gpu(ptr %[[INTEROP]])
; CHECK: all i32 @__tgt_release_interop_obj(ptr %[[INTEROP]])

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

declare void @foo_gpu(ptr %Interop) 

declare void @foo_base() #1

define dso_local i32 @main() {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.VARIANT.DISPATCH"() ]
  call void @foo_base() #3
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.VARIANT.DISPATCH"() ]

  ret i32 0
}

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

attributes #1 = { "openmp-variant"="name:foo_gpu;construct:target_variant_dispatch;arch:gen" }

!llvm.module.flags = !{!0}

!0 = !{i32 1, !"wchar_size", i32 4}
