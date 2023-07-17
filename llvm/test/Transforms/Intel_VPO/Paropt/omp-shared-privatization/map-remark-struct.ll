; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s -check-prefix=CHECK-HOST
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -S %s 2>&1 | FileCheck %s -check-prefix=CHECK-HOST
;
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -switch-to-offload -S %s 2>&1 | FileCheck %s -check-prefix=CHECK-DEVICE
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -switch-to-offload -S %s 2>&1 | FileCheck %s -check-prefix=CHECK-DEVICE
;
; RUN: opt -opaque-pointers=0 -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt-shared-privatization -pass-remarks-analysis=openmp -pass-remarks-output=%t1.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t1.opt.yaml --check-prefix=CHECK-YAML
; RUN: opt -opaque-pointers=0 -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring,vpo-paropt-shared-privatization)' -pass-remarks-analysis=openmp -pass-remarks-output=%t2.opt.yaml -S %s
; RUN: FileCheck %s -input-file=%t2.opt.yaml --check-prefix=CHECK-YAML
;
; Test src:
;
; typedef struct {
;   int N;
; } S;
;
; S test1() {
;   S A = {10};
;   S B = {11};
;   S C;
; #pragma omp target map(to: A) map(tofrom: B) map(from: C)
;   C.N = A.N + B.N;
;   return C;
; }
;
; CHECK-HOST:       remark:{{.*}} MAP:TO clause for variable 'A' on 'target' construct can be changed to FIRSTPRIVATE to reduce mapping overhead
; CHECK-HOST:       remark:{{.*}} MAP:TOFROM clause for variable 'B' on 'target' construct can be changed to FIRSTPRIVATE to reduce mapping overhead
;
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

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-pc-linux-gnu"

%struct.S = type { i32 }

@__const.test1.A = private unnamed_addr constant %struct.S { i32 10 }, align 4
@__const.test1.B = private unnamed_addr constant %struct.S { i32 11 }, align 4

define dso_local i32 @test1() {
entry:
  %retval = alloca %struct.S, align 4
  %A = alloca %struct.S, align 4
  %B = alloca %struct.S, align 4
  %0 = bitcast %struct.S* %A to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %0, i8* align 4 bitcast (%struct.S* @__const.test1.A to i8*), i64 4, i1 false)
  %1 = bitcast %struct.S* %B to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %1, i8* align 4 bitcast (%struct.S* @__const.test1.B to i8*), i64 4, i1 false)
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.FROM"(%struct.S* %retval, %struct.S* %retval, i64 4, i64 34, i8* null, i8* null), ; MAP type: 34 = 0x22 = TARGET_PARAM (0x20) | FROM (0x2)
    "QUAL.OMP.MAP.TO"(%struct.S* %A, %struct.S* %A, i64 4, i64 33, i8* null, i8* null), ; MAP type: 33 = 0x21 = TARGET_PARAM (0x20) | TO (0x1)
    "QUAL.OMP.MAP.TOFROM"(%struct.S* %B, %struct.S* %B, i64 4, i64 35, i8* null, i8* null) ] ; MAP type: 35 = 0x23 = TARGET_PARAM (0x20) | FROM (0x2) | TO (0x1)

  %N = getelementptr inbounds %struct.S, %struct.S* %A, i32 0, i32 0
  %3 = load i32, i32* %N, align 4
  %N1 = getelementptr inbounds %struct.S, %struct.S* %B, i32 0, i32 0
  %4 = load i32, i32* %N1, align 4
  %add = add nsw i32 %3, %4
  %N2 = getelementptr inbounds %struct.S, %struct.S* %retval, i32 0, i32 0
  store i32 %add, i32* %N2, align 4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]

  %coerce.dive = getelementptr inbounds %struct.S, %struct.S* %retval, i32 0, i32 0
  %5 = load i32, i32* %coerce.dive, align 4
  ret i32 %5
}

declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 52, i32 -683357548, !"_Z5test1", i32 5, i32 0, i32 0}
