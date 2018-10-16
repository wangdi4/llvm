; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -S | FileCheck %s --check-prefix=CHECK-HST --check-prefix=CHECK-ALL
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt' -S | FileCheck %s --check-prefix=CHECK-HST --check-prefix=CHECK-ALL
;
; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt -switch-to-offload -S | FileCheck %s --check-prefix=CHECK-TGT --check-prefix=CHECK-ALL
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-cfg-restructuring),vpo-paropt' -switch-to-offload -S | FileCheck %s --check-prefix=CHECK-TGT --check-prefix=CHECK-ALL
;
; This tests checks paropt lowering of 'omp target' construct.
;
; void foo() {
; #pragma omp target
;   {}
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

; Check that offload entry is created.
; CHECK-HST: [[ID:@.+\.region_id]] = weak constant i8 0
; CHECK-ALL: [[NAME:@.+]] = internal target_declare unnamed_addr constant [{{[0-9]+}} x i8] c"[[OUTLINEDTARGET:.+]]\00"
; CHECK-HST: [[ENTRY:@.+]] = weak target_declare constant {{.+}} i8* [[ID]], i8* getelementptr inbounds ({{.+}} [[NAME]],
; CHECK-HST-SAME: section ".omp_offloading.entries"
; CHECK-TGT: [[ENTRY:@.+]] = weak target_declare constant {{.+}} i8* bitcast (void ()* @[[OUTLINEDTARGET]] to i8*), i8* getelementptr inbounds ({{.+}} [[NAME]],
; CHECK-TGT-SAME: section ".omp_offloading.entries"

; Function containing target region should remain in the host compilation, but not in target.
; CHECK-HST:     void @foo()
; CHECK-TGT-NOT: void @foo()
define dso_local void @foo() {
entry:
; Host code should try offload and fall back to host in casse of offload failure.
; CHECK-HST: call i32 @__tgt_target(i64 -1, i8* [[ID]],
; CHECK-HST: call void @[[OUTLINEDTARGET]]()
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"() ], !omp_offload.entry !1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Check presence of the outlined target region.
; CHECK-ALL: define internal void @[[OUTLINEDTARGET]]()

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 54, i32 -698850821, !"foo", i32 2, i32 0}
!1 = distinct !{i32 0}
