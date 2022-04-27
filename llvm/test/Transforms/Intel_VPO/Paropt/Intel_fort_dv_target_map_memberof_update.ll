; INTEL_CUSTOMIZATION
; REQUIRES: asserts
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s -check-prefix=TFORM
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefix=DBG
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -debug-only=vpo-paropt-target -S %s 2>&1 | FileCheck %s -check-prefix=DBG

; Until CMPLRLLVM-27668 is fixed, Paropt unconditionally updates the member-of
; flags for Fortran.

; Test src:

; subroutine foo(a, b, c, d, e)
;     integer:: a
;     integer:: b
;     integer:: c(*)
;     integer:: d(:)
;     integer:: e(:)
;
;     !$omp target map(a) firstprivate(b)  map(to:c(1:10), d, e)
;     !$omp end target
; end subroutine

; DBG: Updated MemberOf Flag from '1' to '2'.
; DBG: MapType changed from '281474976710673 (0x0001000000000011)' to '562949953421329 (0x0002000000000011)'.
; DBG: Updated MemberOf Flag from '1' to '2'.
; DBG: MapType changed from '281474976710657 (0x0001000000000001)' to '562949953421313 (0x0002000000000001)'.
; DBG: Updated MemberOf Flag from '1' to '5'.
; DBG: MapType changed from '281474976710673 (0x0001000000000011)' to '1407374883553297 (0x0005000000000011)'.
; DBG: Updated MemberOf Flag from '1' to '5'.
; DBG: MapType changed from '281474976710657 (0x0001000000000001)' to '1407374883553281 (0x0005000000000001)'.

; TFORM: @.offload_maptypes = private unnamed_addr constant [9 x i64] [i64 33, i64 32, i64 562949953421329, i64 562949953421313, i64 32, i64 1407374883553297, i64 1407374883553281, i64 35, i64 288]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64"

%"QNCA_a0$i32*$rank1$" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Function Attrs: noinline nounwind uwtable
define void @foo_(i32* dereferenceable(4) %"foo_$A", i32* dereferenceable(4) %"foo_$B", i32* dereferenceable(4) %"foo_$C", %"QNCA_a0$i32*$rank1$"* dereferenceable(72) "assumed_shape" "ptrnoalias" %"foo_$D", %"QNCA_a0$i32*$rank1$"* dereferenceable(72) "assumed_shape" "ptrnoalias" %"foo_$E") #0 {
alloca_0:
  %"$io_ctx" = alloca [8 x i64], align 8
  %"([0 x i32]*)foo_$C$" = bitcast i32* %"foo_$C" to [0 x i32]*
  %0 = getelementptr [0 x i32], [0 x i32]* %"([0 x i32]*)foo_$C$", i64 0, i64 0
  %"foo_$D.addr_a0$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %"foo_$D", i32 0, i32 0
  %"foo_$D.addr_a0$_fetch" = load i32*, i32** %"foo_$D.addr_a0$", align 1
  %"foo_$D.addr_length$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %"foo_$D", i32 0, i32 1
  %"foo_$D.addr_length$2" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %"foo_$D", i32 0, i32 1
  %"foo_$D.addr_length$2_fetch" = load i64, i64* %"foo_$D.addr_length$2", align 1
  %"foo_$D.dim_info$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %"foo_$D", i32 0, i32 6, i32 0
  %"foo_$D.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$D.dim_info$", i32 0, i32 0
  %"foo_$D.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$D.dim_info$.extent$", i32 0)
  %"foo_$D.dim_info$.extent$[]_fetch" = load i64, i64* %"foo_$D.dim_info$.extent$[]", align 1
  %mul.2 = mul nsw i64 %"foo_$D.addr_length$2_fetch", %"foo_$D.dim_info$.extent$[]_fetch"
  %"foo_$E.addr_a0$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %"foo_$E", i32 0, i32 0
  %"foo_$E.addr_a0$_fetch" = load i32*, i32** %"foo_$E.addr_a0$", align 1
  %"foo_$E.addr_length$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %"foo_$E", i32 0, i32 1
  %"foo_$E.addr_length$4" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %"foo_$E", i32 0, i32 1
  %"foo_$E.addr_length$4_fetch" = load i64, i64* %"foo_$E.addr_length$4", align 1
  %"foo_$E.dim_info$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", %"QNCA_a0$i32*$rank1$"* %"foo_$E", i32 0, i32 6, i32 0
  %"foo_$E.dim_info$.extent$" = getelementptr inbounds { i64, i64, i64 }, { i64, i64, i64 }* %"foo_$E.dim_info$", i32 0, i32 0
  %"foo_$E.dim_info$.extent$[]" = call i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 0, i64 0, i32 24, i64* elementtype(i64) %"foo_$E.dim_info$.extent$", i32 0)
  %"foo_$E.dim_info$.extent$[]_fetch" = load i64, i64* %"foo_$E.dim_info$.extent$[]", align 1
  %mul.3 = mul nsw i64 %"foo_$E.addr_length$4_fetch", %"foo_$E.dim_info$.extent$[]_fetch"
  br label %bb2

bb2:                                              ; preds = %alloca_0
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0),
  "QUAL.OMP.MAP.TO"(i32* %"foo_$C", i32* %0, i64 40, i64 33),

  "QUAL.OMP.MAP.TO"(%"QNCA_a0$i32*$rank1$"* %"foo_$D", %"QNCA_a0$i32*$rank1$"* %"foo_$D", i64 72, i64 32),
  "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i32*$rank1$"* %"foo_$D", i32* %"foo_$D.addr_a0$_fetch", i64 %mul.2, i64 281474976710673),
  "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i32*$rank1$"* %"foo_$D", i64* %"foo_$D.addr_length$", i64 64, i64 281474976710657),

  "QUAL.OMP.MAP.TO"(%"QNCA_a0$i32*$rank1$"* %"foo_$E", %"QNCA_a0$i32*$rank1$"* %"foo_$E", i64 72, i64 32),
  "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i32*$rank1$"* %"foo_$E", i32* %"foo_$E.addr_a0$_fetch", i64 %mul.3, i64 281474976710673),
  "QUAL.OMP.MAP.TO:CHAIN"(%"QNCA_a0$i32*$rank1$"* %"foo_$E", i64* %"foo_$E.addr_length$", i64 64, i64 281474976710657),

  "QUAL.OMP.MAP.TOFROM"(i32* %"foo_$A", i32* %"foo_$A", i64 4, i64 35, i8* null, i8* null),
  "QUAL.OMP.FIRSTPRIVATE"(i32* %"foo_$B") ]
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  ret void
}

; Function Attrs: nounwind readnone speculatable
declare i64* @llvm.intel.subscript.p0i64.i64.i32.p0i64.i32(i8 %0, i64 %1, i32 %2, i64* elementtype(i64) %3, i32 %4) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token %0) #2

attributes #0 = { noinline nounwind uwtable "frame-pointer"="all" "intel-lang"="fortran" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { nounwind }

!omp_offload.info = !{!0}

!0 = !{i32 0, i32 66309, i32 47065241, !"foo_", i32 8, i32 0, i32 0}
; end INTEL_CUSTOMIZATION
