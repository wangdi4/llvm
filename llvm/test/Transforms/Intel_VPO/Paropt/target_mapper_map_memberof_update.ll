; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefix=DBG
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefix=DBG

; The test was generating by modifying the CFE IR for the following test,
; by changing the FIRSTPRIVATE qual for "f" to "MAP.TOFROM".
; The test was disabled, because Paropt stopped supporting non-chain representation
; for maps. It cannot be supported with opaque pointers.
; UNSUPPORTED: asserts
;
; Test src:

; typedef struct {
; } a;
; #pragma omp declare mapper(a b) map(b)
; typedef struct {
;   a c;
; } d;
;
; fn1() {
;   d **e;
;   int f;
; #pragma omp target firstprivate(f) map(e[0][0])
;   e;
; }

; DBG: Map index of base of chain shifted from '1' to '2'.
; DBG: Updated MemberOf Flag from '2' to '3'.
; DBG: MapType changed from '562949953421315 (0x0002000000000003)' to '844424930131971 (0x0003000000000003)'.
; DBG: Map index of base of chain shifted from '1' to '2'.
; DBG: Updated MemberOf Flag from '2' to '3'.
; DBG: MapType changed from '562949953421315 (0x0002000000000003)' to '844424930131971 (0x0003000000000003)'.
; DBG: Map index of base of chain shifted from '1' to '2'.
; DBG: Updated MemberOf Flag from '2' to '3'.
; DBG: MapType changed from '562949953421827 (0x0002000000000203)' to '844424930132483 (0x0003000000000203)'.
; DBG: Map index of base of chain shifted from '1' to '2'.
; DBG: Updated MemberOf Flag from '2' to '3'.
; DBG: MapType changed from '562949953421843 (0x0002000000000213)' to '844424930132499 (0x0003000000000213)'.

; Check that the member-of value of the maps 4-7 point to the index 3.
; TFORM: @.offload_maptypes = private unnamed_addr constant [7 x i64] [i64 35, i64 35, i64 0, i64 844424930131971, i64 844424930131971, i64 844424930132483, i64 844424930132499]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64"
target device_triples = "x86_64"

%struct.d = type { %struct.a }
%struct.a = type { i8 }

; Function Attrs: convergent noinline nounwind optnone uwtable mustprogress
define hidden i32 @_Z3fn1v() #0 {
entry:
  %retval = alloca i32, align 4
  %e = alloca %struct.d**, align 8
  %f = alloca i32, align 4
  %e.map.ptr.tmp = alloca %struct.d**, align 8
  %0 = load %struct.d**, %struct.d*** %e, align 8
  %1 = load %struct.d**, %struct.d*** %e, align 8
  %2 = load %struct.d**, %struct.d*** %e, align 8
  %ptridx = getelementptr inbounds %struct.d*, %struct.d** %2, i64 0
  %3 = load %struct.d**, %struct.d*** %e, align 8
  %ptridx1 = getelementptr inbounds %struct.d*, %struct.d** %3, i64 0
  %4 = load %struct.d*, %struct.d** %ptridx1, align 8
  %ptridx2 = getelementptr inbounds %struct.d, %struct.d* %4, i64 0
  %5 = bitcast %struct.d* %ptridx2 to i8*
  %6 = getelementptr i8, i8* %5, i64 0
  %7 = load %struct.d**, %struct.d*** %e, align 8
  %ptridx3 = getelementptr inbounds %struct.d*, %struct.d** %7, i64 0
  %8 = load %struct.d*, %struct.d** %ptridx3, align 8
  %ptridx4 = getelementptr inbounds %struct.d, %struct.d* %8, i64 0
  %c = getelementptr inbounds %struct.d, %struct.d* %ptridx4, i32 0, i32 0
  %9 = bitcast %struct.d* %ptridx2 to i8*
  %10 = bitcast %struct.a* %c to i8*
  %11 = ptrtoint i8* %10 to i64
  %12 = ptrtoint i8* %9 to i64
  %13 = sub i64 %11, %12
  %14 = sdiv exact i64 %13, ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)
  %15 = getelementptr %struct.a, %struct.a* %c, i64 1
  %16 = bitcast %struct.a* %15 to i8*
  %17 = getelementptr i8, i8* %6, i64 1
  %18 = ptrtoint i8* %17 to i64
  %19 = ptrtoint i8* %16 to i64
  %20 = sub i64 %18, %19
  %21 = sdiv exact i64 %20, ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)
  %22 = load %struct.d**, %struct.d*** %e, align 8
  %23 = load %struct.d**, %struct.d*** %e, align 8
  %ptridx5 = getelementptr inbounds %struct.d*, %struct.d** %23, i64 0
  %24 = load %struct.d**, %struct.d*** %e, align 8
  %ptridx6 = getelementptr inbounds %struct.d*, %struct.d** %24, i64 0
  %25 = load %struct.d*, %struct.d** %ptridx6, align 8
  %ptridx7 = getelementptr inbounds %struct.d, %struct.d* %25, i64 0
  %c8 = getelementptr inbounds %struct.d, %struct.d* %ptridx7, i32 0, i32 0
  %26 = getelementptr %struct.d, %struct.d* %ptridx2, i32 1
  %27 = bitcast %struct.d* %ptridx2 to i8*
  %28 = bitcast %struct.d* %26 to i8*
  %29 = ptrtoint i8* %28 to i64
  %30 = ptrtoint i8* %27 to i64
  %31 = sub i64 %29, %30
  %32 = sdiv exact i64 %31, ptrtoint (i8* getelementptr (i8, i8* null, i32 1) to i64)

  %33 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.TOFROM"(i32* %f),
  "QUAL.OMP.MAP.TOFROM"(%struct.d** %1, %struct.d** %ptridx, i64 8, i64 35, i8* null, i8* null),
  "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.d** %ptridx, %struct.d* %ptridx2, i64 %32, i64 0, i8* null, i8* null),
  "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.d** %ptridx, %struct.d* %ptridx2, i64 %14, i64 562949953421315, i8* null, i8* null),
  "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.d** %ptridx, %struct.a* %15, i64 %21, i64 562949953421315, i8* null, i8* null),
  "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.d** %22, %struct.d** %ptridx5, i64 8, i64 562949953421827, i8* null, i8* null),
  "QUAL.OMP.MAP.TOFROM:CHAIN"(%struct.d** %ptridx5, %struct.a* %c8, i64 1, i64 562949953421843, i8* null, void (i8*, i8*, i8*, i64, i64, i8*)* @.omp_mapper._ZTS1a.default),
  "QUAL.OMP.PRIVATE"(%struct.d*** %e.map.ptr.tmp) ]

  store %struct.d** %1, %struct.d*** %e.map.ptr.tmp, align 8

  call void @llvm.directive.region.exit(token %33) [ "DIR.OMP.END.TARGET"() ]
  %34 = load i32, i32* %retval, align 4
  ret i32 %34
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: convergent noinline uwtable
declare void @.omp_mapper._ZTS1a.default(i8* %0, i8* %1, i8* %2, i64 %3, i64 %4, i8* %5) #2

; Function Attrs: nounwind
declare void @__tgt_push_mapper_component(i8*, i8*, i8*, i64, i64, i8*) #1

; Function Attrs: nounwind
declare i64 @__tgt_mapper_num_components(i8*) #1

attributes #0 = { convergent noinline nounwind optnone uwtable mustprogress "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { convergent noinline uwtable "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1, !2}

!0 = !{i32 0, i32 66309, i32 61215076, !"_Z3fn1v", i32 10, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
