; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -S <%s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt' -S <%s | FileCheck %s

; // C source
; void foo() {
;   int aaa;
;   #pragma omp target update to(aaa) nowait
; 
;   #pragma omp target enter data map(to:aaa) nowait
; 
;   #pragma omp target exit  data map(from:aaa) nowait
; }

; For the target UPDATE / ENTER DATA / ENTER DATA constructs with NOWAIT
; there is an implicit target task around each. The target task is created
; with flag bit 0x80 set, telling runtime to use hidden helper threads.

; Check that there are 3 calls to @__kmpc_omp_task_alloc with flag=129
; CHECK-LABEL: define {{.*}} @foo
; CHECK: call ptr @__kmpc_omp_task_alloc(ptr {{.*}}, i32 {{.*}}, i32 129
; CHECK: call ptr @__kmpc_omp_task_alloc(ptr {{.*}}, i32 {{.*}}, i32 129
; CHECK: call ptr @__kmpc_omp_task_alloc(ptr {{.*}}, i32 {{.*}}, i32 129

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() {
entry:
  %aaa = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.TARGET.TASK"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %aaa, i32 0, i32 1) ]

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.UPDATE"(),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.MAP.TO"(ptr %aaa, ptr %aaa, i64 4, i64 1, ptr null, ptr null) ] ; MAP type: 1 = 0x1 = TO (0x1)

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.UPDATE"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]

  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.TARGET.TASK"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %aaa, i32 0, i32 1) ]

  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.MAP.TO"(ptr %aaa, ptr %aaa, i64 4, i64 1, ptr null, ptr null) ] ; MAP type: 1 = 0x1 = TO (0x1)

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]

  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TASK"() ]

  %4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.TARGET.TASK"(),
    "QUAL.OMP.SHARED:TYPED"(ptr %aaa, i32 0, i32 1) ]

  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.EXIT.DATA"(),
    "QUAL.OMP.NOWAIT"(),
    "QUAL.OMP.MAP.FROM"(ptr %aaa, ptr %aaa, i64 4, i64 2, ptr null, ptr null) ] ; MAP type: 2 = 0x2 = FROM (0x2)

  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]

  call void @llvm.directive.region.exit(token %4) [ "DIR.OMP.END.TASK"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
