; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-HOST
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-HOST
;
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -switch-to-offload -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DEVICE
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -switch-to-offload -S %s 2>&1 | FileCheck %s --check-prefixes=CHECK,CHECK-DEVICE
;
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -pass-remarks-output=%t1.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t1.opt.yaml --check-prefix=CHECK-YAML
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -pass-remarks-output=%t2.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t2.opt.yaml --check-prefix=CHECK-YAML

; Test src:
;
; void test1(int A) {
;   int B;
; #pragma omp target
; #pragma omp parallel firstprivate(A) private(B)
;   {}
; }

; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'A.addr' on 'parallel' construct is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'A.addr' on 'target' construct is redundant
; CHECK-HOST:       remark:{{.*}} FIRSTPRIVATE clause for variable 'B' on 'target' construct is redundant
;
; CHECK-DEVICE-NOT: remark:{{.*}} FIRSTPRIVATE clause for variable '{{.+}}' is redundant
;
; CHECK-YAML:      --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test1
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          A.addr
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          parallel
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test1
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          A.addr
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          target
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test1
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          FIRSTPRIVATE
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          B
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          target
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

define dso_local void @test1(i32 noundef %A) {
; CHECK-LABEL: @test1(
entry:
  %A.addr = alloca i32, align 4
  %B = alloca i32, align 4
  store i32 %A, ptr %A.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %A.addr, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %B, i32 0, i32 1) ]
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %A.addr, i32 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %B, i32 0, i32 1) ]

; CHECK:   call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr null, i32 0, i32 1)
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %A.addr, i32 0, i32 1)

  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 53, i32 -1916230714, !"_Z5test1", i32 3, i32 0, i32 0}
