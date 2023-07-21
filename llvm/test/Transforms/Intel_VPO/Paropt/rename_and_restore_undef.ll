; RUN: opt -bugpoint-enable-legacy-pm -vpo-restore-operands -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -passes='function(vpo-restore-operands)' -S %s | FileCheck %s -check-prefix=RESTR
; RUN: opt -bugpoint-enable-legacy-pm -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s -check-prefix=TFORM

; Test src:

; class A {
; public:
;   A();
;   ~A();
;   int a;
; };
; void foo() {
;   A *b;
; #pragma omp target
;   b->a = 10;
; }

; The tese IR is obtained using -vpo-cfg-restructuring -vpo-paropt-prepare -sroa -instcombine -simplifycfg.
; The test is to ensure that we can handle cases where there is no store to the ADDR
; operand of an "OPERAND.ADDR" pair (such as "%.addr").

; This can happen when an optimization replaces the first operand with undef, and thus deletes the store.

; Check that after restore operands, uses of %1 are replaced with undef.
; RESTR: store ptr undef, ptr %b.map.ptr.tmp{{.*}}
; RESTR: %a = getelementptr inbounds %class.A, ptr undef, i64 0, i32 0

; Check that paropt-transform doesn't comp-fail when compiling the output of restore-operands.
; TFORM: call i32 @__tgt_target(i64 %{{.*}}, ptr @__omp_offloading{{[^ ,]*}}foo{{[^ ,]*}}, i32 0, ptr null, ptr null, ptr null, ptr null)

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

%class.A = type { i32 }

@"@tid.addr" = external global i32

define protected void @_Z3foov() {
entry:
  %b.map.ptr.tmp = alloca ptr, align 8
  %b.map.ptr.tmp.addr = alloca ptr, align 8
  %.addr = alloca ptr, align 8
  store ptr %b.map.ptr.tmp, ptr %b.map.ptr.tmp.addr, align 8
  %end.dir.temp = alloca i1, align 1

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr undef, ptr undef, i64 0, i64 544, ptr null, ptr null), ; MAP type: 544 = 0x220 = IMPLICIT (0x200) | TARGET_PARAM (0x20)
    "QUAL.OMP.PRIVATE:TYPED"(ptr %b.map.ptr.tmp, ptr null, i32 1),
    "QUAL.OMP.OPERAND.ADDR"(ptr %b.map.ptr.tmp, ptr %b.map.ptr.tmp.addr),
    "QUAL.OMP.OPERAND.ADDR"(ptr undef, ptr %.addr),
    "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]

  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.TARGET.4.split, label %DIR.OMP.TARGET.3

DIR.OMP.TARGET.3:                                 ; preds = %entry
  %b.map.ptr.tmp1 = load volatile ptr, ptr %b.map.ptr.tmp.addr, align 8

  %1 = load volatile ptr, ptr %.addr, align 8
  store ptr %1, ptr %b.map.ptr.tmp1, align 8
  %a = getelementptr inbounds %class.A, ptr %1, i64 0, i32 0
  store i32 10, ptr %a, align 4
  br label %DIR.OMP.END.TARGET.4.split

DIR.OMP.END.TARGET.4.split:                       ; preds = %entry, %DIR.OMP.TARGET.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]

  ret void
}

declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66306, i32 40906352, !"_Z3foov", i32 9, i32 0, i32 0, i32 0}
