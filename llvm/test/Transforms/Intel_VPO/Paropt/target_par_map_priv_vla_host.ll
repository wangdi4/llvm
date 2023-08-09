; REQUIRES: asserts
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=WRegionUtils,vpo-paropt-transform -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s
; RUN: opt -aa-pipeline=basic-aa -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=WRegionUtils,vpo-paropt-transform -vpo-paropt-opt-scalar-fp=false -S %s 2>&1 | FileCheck %s

; Test src:

; #include <omp.h>
; #include <stdio.h>
; int main()
; {
;   int n_dims = 2;
;   int tmp[n_dims];
;   tmp[0] = 10;
;
; #pragma omp target map(tmp)
;   {
; #pragma omp parallel private(tmp)
;     {
;       printf("%d\n", tmp[0]);
;     }
;   }
;   printf("%d\n", tmp[1]);
; }

; This test is similar to target_par_priv_vla_tgt.ll, but instead of being
; marked FIRSTPRIVATE, the VLA is on the MAP clause on the outer target construct.

; Check that we capture the VLA size expression at both parallel and target constructs.
; CHECK: VPOParopt Transform: PARALLEL construct
; CHECK: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %fp.len '
; CHECK: captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'ptr [[SIZE_ADDR1:%[^ ]+]]'
; CHECK: VPOParopt Transform: TARGET construct
; CHECK: collectNonPointerValuesToBeUsedInOutlinedRegion: Non-pointer values to be passed into the outlined region: 'i64 %i1 '
; CHECK: captureAndAddCollectedNonPointerValuesToSharedClause: Added implicit shared/map(to)/firstprivate clause for: 'ptr [[I1_ADDR:%i1.addr]]'

; Ensure that the map-type for I1_ADDR (3rd) is 161 (PARAM|TO|PRIVATE)
; CHECK: @.offload_maptypes = private unnamed_addr constant [3 x i64] [i64 35, i64 161, i64 161]

; Check that the captured VLA size is passed in to the outlined function for the target region.
; CHECK: define dso_local i32 @main()
; CHECK: call void @__omp_offloading{{.*}}main{{.*}}(ptr %vla, ptr %omp.vla.tmp, ptr [[I1_ADDR]])

; Check that the captured VLA size is used in the parallel region for allocation of the private VLA.
; CHECK: define internal void @main.DIR.OMP.PARALLEL{{.*}}(ptr %{{.*}}, ptr %{{.*}}, ptr [[SIZE_ADDR1]], ptr %{{.*}})
; CHECK: [[SIZE_VAL1:%[^ ]+]] = load i64, ptr [[SIZE_ADDR1]], align 8
; CHECK: %{{.*}} = alloca i32, i64 [[SIZE_VAL1]], align 16

; Check that the address of %fp.len is passed into the outlined function for the parallel region.
; CHECK: define internal void @__omp_offloading{{.*}}main{{.*}}(ptr noalias %vla, ptr %omp.vla.tmp, ptr noalias [[I1_ADDR]])
; Check for the firstprivatet initialization of I1_ADDR
; CHECK: [[I1_ADDR]].fpriv = alloca i64, align 8
; CHECK: [[I1_VAL:%.+]] = load i64, ptr [[I1_ADDR]], align 8
; CHECK: store i64 [[I1_VAL]], ptr [[I1_ADDR]].fpriv, align 8
; Check that the captured VLA size %fp.len is passed in to the outlined function for the parallel region.
; CHECK: store i64 %fp.len, ptr [[SIZE_ADDR1]], align 8
; CHECK: call void (ptr, i32, ptr, ...) @__kmpc_fork_call(ptr @.kmpc_loc{{.*}}, i32 2, ptr @main.DIR.OMP.PARALLEL.{{.*}}, ptr [[SIZE_ADDR1]], ptr %{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

define dso_local i32 @main() #0 {
entry:
  %n_dims = alloca i32, align 4
  %saved_stack = alloca ptr, align 8
  %__vla_expr0 = alloca i64, align 8
  %omp.vla.tmp = alloca i64, align 8
  %omp.vla.tmp2 = alloca i64, align 8
  store i32 2, ptr %n_dims, align 4
  %i = load i32, ptr %n_dims, align 4
  %i1 = zext i32 %i to i64
  %i2 = call ptr @llvm.stacksave()
  store ptr %i2, ptr %saved_stack, align 8
  %vla = alloca i32, i64 %i1, align 16
  store i64 %i1, ptr %__vla_expr0, align 8
  %arrayidx = getelementptr inbounds i32, ptr %vla, i64 0
  store i32 10, ptr %arrayidx, align 16
  store i64 %i1, ptr %omp.vla.tmp, align 8

  %map.len = mul nuw i64 %i1, 4
  %arrayidx1 = getelementptr inbounds i32, ptr %vla, i64 0
  %i4 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(),
    "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %vla, ptr %arrayidx1, i64 %map.len, i64 3, ptr null, ptr null),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %omp.vla.tmp, i64 0, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %omp.vla.tmp2, i64 0, i32 1) ]

  %fp.len = load i64, ptr %omp.vla.tmp, align 8
  store i64 %fp.len, ptr %omp.vla.tmp2, align 8

  %i6 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %vla, i32 0, i64 %fp.len),
    "QUAL.OMP.SHARED:TYPED"(ptr %omp.vla.tmp2, i64 0, i32 1) ]

  %i7 = load i64, ptr %omp.vla.tmp2, align 8
  %arrayidx3 = getelementptr inbounds i32, ptr %vla, i64 0
  %i8 = load i32, ptr %arrayidx3, align 16
  %call = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %i8)

  call void @llvm.directive.region.exit(token %i6) [ "DIR.OMP.END.PARALLEL"() ]
  call void @llvm.directive.region.exit(token %i4) [ "DIR.OMP.END.TARGET"() ]

  %arrayidx4 = getelementptr inbounds i32, ptr %vla, i64 1
  %i9 = load i32, ptr %arrayidx4, align 4
  %call5 = call i32 (ptr, ...) @printf(ptr noundef @.str, i32 noundef %i9)
  %i10 = load ptr, ptr %saved_stack, align 8
  call void @llvm.stackrestore(ptr %i10)
  ret i32 0
}

declare ptr @llvm.stacksave()
declare void @llvm.stackrestore(i8*)
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
declare dso_local i32 @printf(ptr noundef, ...)

!omp_offload.info = !{!0}
!0 = !{i32 0, i32 66313, i32 186596698, !"_Z4main", i32 9, i32 0, i32 0}
