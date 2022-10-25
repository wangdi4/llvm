; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK-HOST
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK-HOST
;
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -switch-to-offload -S %s 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK-DEVICE
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -switch-to-offload -S %s 2>&1 | FileCheck %s -check-prefixes=CHECK,CHECK-DEVICE
;
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -instcombine -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -pass-remarks-output=%t1.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t1.opt.yaml --check-prefix=CHECK-YAML
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,instcombine,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -pass-remarks-output=%t2.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t2.opt.yaml --check-prefix=CHECK-YAML
;
; Test src:
;
; float test1() {
;   float A = 10;
;   float B = 11;
;   float C;
; #pragma omp target map(to: A) map(tofrom: B) map(from: C)
;   C = A + B;
;   return C;
; }
;
; void test2() {
;   int D, E, F;
; #pragma omp target map(from: D) map(to: E) map(tofrom: F)
; #pragma omp parallel private(D)
;     { D = 10; (void) E; (void) F; }
; }
;
; CHECK-HOST:       remark:{{.*}} MAP:TO clause for variable 'A' on 'target' construct can be changed to FIRSTPRIVATE to reduce mapping overhead
; CHECK-HOST:       remark:{{.*}} MAP:TOFROM clause for variable 'B' on 'target' construct can be changed to FIRSTPRIVATE to reduce mapping overhead
; CHECK-HOST:       remark:{{.*}} SHARED clause for variable 'E' on 'parallel' construct is redundant
; CHECK-HOST:       remark:{{.*}} SHARED clause for variable 'F' on 'parallel' construct is redundant
; CHECK-HOST:       remark:{{.*}} MAP:TO clause for variable 'E' on 'target' construct is redundant
; CHECK-HOST:       remark:{{.*}} MAP:TOFROM clause for variable 'F' on 'target' construct is redundant
; CHECK-HOST:       remark:{{.*}} MAP:FROM clause for variable 'D' on 'target' construct is redundant

; CHECK-DEVICE-NOT: remark:
;
; CHECK-YAML:      --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test1
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          'MAP:TO'
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          A
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          target
; CHECK-YAML-NEXT:   - String:          ''' construct can be changed to FIRSTPRIVATE to reduce mapping overhead'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test1
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          'MAP:TOFROM'
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          B
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          target
; CHECK-YAML-NEXT:   - String:          ''' construct can be changed to FIRSTPRIVATE to reduce mapping overhead'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          SHARED
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          E
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          parallel
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          SHARED
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          F
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          parallel
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          'MAP:TO'
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          E
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          target
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          'MAP:TOFROM'
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          F
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          target
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...
; CHECK-YAML-NEXT: --- !Analysis
; CHECK-YAML-NEXT: Pass:            openmp
; CHECK-YAML-NEXT: Name:            optimization note
; CHECK-YAML-NEXT: Function:        test2
; CHECK-YAML-NEXT: Args:
; CHECK-YAML-NEXT:   - String:          'MAP:FROM'
; CHECK-YAML-NEXT:   - String:          ' clause for variable '''
; CHECK-YAML-NEXT:   - String:          D
; CHECK-YAML-NEXT:   - String:          ''' on '''
; CHECK-YAML-NEXT:   - String:          target
; CHECK-YAML-NEXT:   - String:          ''' construct is redundant'
; CHECK-YAML-NEXT: ...

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

define dso_local float @test1() {
; CHECK-LABEL: @test1(
entry:
  %A = alloca float, align 4
  %B = alloca float, align 4
  %C = alloca float, align 4
  store float 1.000000e+01, ptr %A, align 4
  store float 1.100000e+01, ptr %B, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.FROM"(ptr %C, ptr %C, i64 4, i64 34, ptr null, ptr null), ; MAP type: 34 = 0x22 = TARGET_PARAM (0x20) | FROM (0x2)
    "QUAL.OMP.MAP.TO"(ptr %A, ptr %A, i64 4, i64 33, ptr null, ptr null), ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %B, ptr %B, i64 4, i64 35, ptr null, ptr null) ] ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)

  %1 = load float, ptr %A, align 4
  %2 = load float, ptr %B, align 4
  %add = fadd fast float %1, %2
  store float %add, ptr %C, align 4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  %3 = load float, ptr %C, align 4
  ret float %3
}

define dso_local void @test2() {
; CHECK-LABEL: @test2(
entry:
  %D = alloca i32, align 4
  %E = alloca i32, align 4
  %F = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 1),
    "QUAL.OMP.MAP.TO"(ptr %E, ptr %E, i64 4, i64 33, ptr null, ptr null), ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(ptr %F, ptr %F, i64 4, i64 35, ptr null, ptr null), ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)
    "QUAL.OMP.MAP.FROM"(ptr %D, ptr %D, i64 4, i64 2, ptr null, ptr null) ] ; MAP type: 2 = 0x2 = FROM (0x2)

  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %D, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %E, i32 0, i32 1),
    "QUAL.OMP.SHARED:TYPED"(ptr %F, i32 0, i32 1) ]

; CHECK:   call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"()
; CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1),
; CHECK-SAME: "QUAL.OMP.SHARED:TYPED"(ptr null, i32 0, i32 1),
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %E, i32 0, i32 1),
; CHECK-SAME: "QUAL.OMP.PRIVATE:TYPED"(ptr %F, i32 0, i32 1) ]

  store i32 10, ptr %D, align 4
  %2 = load i32, ptr %E, align 4
  %3 = load i32, ptr %F, align 4
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.PARALLEL"() ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0, !1}

!0 = !{i32 0, i32 66313, i32 50002634, !"_Z5test2", i32 12, i32 1, i32 0}
!1 = !{i32 0, i32 66313, i32 50002634, !"_Z5test1", i32 5, i32 0, i32 0}
