; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; extern int *a, *b;
; void foo() {
;   #pragma omp target enter data map(to:a[:1])
;   #pragma omp target update to(b[:1])
;   #pragma omp target exit data map(from:a[:1])
; }


; Check that the baseptrs allocations for all three constructs happen in the
; original, foo, and no other outlined function is created.
; CHECK-LABEL: define dso_local void @foo()
; CHECK-COUNT-3: %.offload_baseptrs{{.*}} = alloca [1 x ptr], align 8
; CHECK-NOT: define{{.*}}foo.{{.*}}

; Check for the RTL calls emitted for the constructs.
; CHECK: call void @__tgt_target_data_begin_mapper
; CHECK: call void @__tgt_target_data_update_mapper
; CHECK: call void @__tgt_target_data_end_mapper

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

@a = external dso_local global ptr, align 8
@b = external dso_local global ptr, align 8

define dso_local void @foo() {
entry:
  %0 = load ptr, ptr @a, align 8
  %arrayidx = getelementptr inbounds i32, ptr %0, i64 0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.ENTER.DATA"(),
    "QUAL.OMP.MAP.TO"(ptr @a, ptr %arrayidx, i64 4, i64 17, ptr null, ptr null) ] ; MAP type: 17 = 0x11 = PTR_AND_OBJ (0x10) | TO (0x1)

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET.ENTER.DATA"() ]

  %2 = load ptr, ptr @b, align 8
  %arrayidx1 = getelementptr inbounds i32, ptr %2, i64 0
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.UPDATE"(),
    "QUAL.OMP.MAP.TO"(ptr @b, ptr %arrayidx1, i64 4, i64 17, ptr null, ptr null) ] ; MAP type: 17 = 0x11 = PTR_AND_OBJ (0x10) | TO (0x1)

  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TARGET.UPDATE"() ]

  %4 = load ptr, ptr @a, align 8
  %arrayidx2 = getelementptr inbounds i32, ptr %4, i64 0
  %5 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.EXIT.DATA"(),
    "QUAL.OMP.MAP.FROM"(ptr @a, ptr %arrayidx2, i64 4, i64 18, ptr null, ptr null) ] ; MAP type: 18 = 0x12 = PTR_AND_OBJ (0x10) | FROM (0x2)

  call void @llvm.directive.region.exit(token %5) [ "DIR.OMP.END.TARGET.EXIT.DATA"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
