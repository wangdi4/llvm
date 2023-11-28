; INTEL_CUSTOMIZATION
; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; Test for F90_DV operands on both map and use_device_ptr clause on a target data construct.
; Test src:

; !program main
; !    implicit none
; !    integer :: A(10)
; !    A(2) = 10
; !    print *, A(2)
; !    call foo(A)
; !    print *, A(2)
;
; !contains
;     subroutine foo(B)
;       integer :: B(:)
;
;       !$omp target data map(tofrom:B) use_device_ptr(B)
; !      !$omp target is_device_ptr(B)
; !        print *, B(2)
;         B(2) = 20
; !        print *, B(2)
; !      !$omp end target
;       !$omp end target data
;
;     end subroutine
; !end program main

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

%"QNCA_a0$i32*$rank1$" = type { ptr, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

; Check for the map type struct for target data region. It should have
; 3 entries for the map clause and one (64) for use_device_ptr.

; CHECK: [[MAP_TYS:@.offload_maptypes[^ ]*]] = private unnamed_addr constant [4 x i64] [i64 32, i64 281474976710675, i64 281474976710657, i64 64]

; Function Attrs: nounwind uwtable
define void @foo_(ptr dereferenceable(72) "ptrnoalias" %"foo_$B") #0 {
alloca_0:
  %"foo_$B.field_6$.extent$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"foo_$B", i64 0, i32 6, i64 0, i32 0
  %"foo_$B.field_6$.extent$[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$B.field_6$.extent$", i32 0)
  %"foo_$B.field_6$.extent$[]_fetch" = load i64, ptr %"foo_$B.field_6$.extent$[]", align 1
  %mul = shl nsw i64 %"foo_$B.field_6$.extent$[]_fetch", 2
  %"foo_$B.addr_a0$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"foo_$B", i64 0, i32 0
  %"foo_$B.addr_a0$_fetch" = load ptr, ptr %"foo_$B.addr_a0$", align 1
  %"foo_$B.addr_length$" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"foo_$B", i64 0, i32 1

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(),
    "QUAL.OMP.USE_DEVICE_PTR:F90_DV.TYPED"(ptr %"foo_$B", %"QNCA_a0$i32*$rank1$" zeroinitializer, i32 0),
    "QUAL.OMP.MAP.TOFROM"(ptr %"foo_$B", ptr %"foo_$B", i64 72, i64 32),
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %"foo_$B", ptr %"foo_$B.addr_a0$_fetch", i64 %mul, i64 281474976710675),
    "QUAL.OMP.MAP.TOFROM:CHAIN"(ptr %"foo_$B", ptr %"foo_$B.addr_length$", i64 64, i64 281474976710657) ]

; Check that a local dope vector is allocated for the USE_DEVICE_PTR:F90_DV operand.
; CHECK: %"foo_$B.new" = alloca %"QNCA_a0$i32*$rank1$", align 8

; Check that B.addr[0] is used in the map clause for use_device_ptr to get the corresponding device pointer.
; CHECK: [[B_ADDR0_GEP:%[^ ]+]] = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"foo_$B", i32 0, i32 0
; CHECK: [[B_ADDR0:%[^ ]+]] = load ptr, ptr [[B_ADDR0_GEP]], align 8
; CHECK: [[GEP:%[^ ]+]] = getelementptr inbounds [4 x ptr], ptr %.offload_baseptrs, i32 0, i32 3
; CHECK: store ptr [[B_ADDR0]], ptr [[GEP]], align 8

; CHECK: call void @__tgt_target_data_begin({{.*}}[[MAP_TYS]]{{.*}})
; CHECK: [[B_ADDR0_UPDATED:%[^ ]+]] = load ptr, ptr [[GEP]], align 8

; Check that the local dope vector is initialized using the original dope vector %"foo_$B".
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 8 %"foo_$B.new", ptr align 8 %"foo_$B", i64 72, i1 false)

; Check that the addr0 field of the new dope vector is initialized using the updated pointer from the tgt_data call.
; CHECK: [[B_NEW_ADDR0_GEP:%[^ ]+]] = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"foo_$B.new", i32 0, i32 0
; CHECK: store ptr [[B_ADDR0_UPDATED]], ptr [[B_NEW_ADDR0_GEP]], align 8

; Check that call to outlined function for target data uses %"foo_$B.new"
; CHECK: call void @foo_.DIR.OMP.TARGET.DATA{{.*}}(ptr %"foo_$B.new")


  %"foo_$B.addr_a0$_clone" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"foo_$B", i64 0, i32 0
  %"foo_$B.field_6$.spacing$_clone" = getelementptr inbounds %"QNCA_a0$i32*$rank1$", ptr %"foo_$B", i64 0, i32 6, i64 0, i32 1
  %"foo_$B.addr_a0$_clone_fetch" = load ptr, ptr %"foo_$B.addr_a0$_clone", align 1
  %"foo_$B.field_6$.spacing$_clone[]" = call ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8 0, i64 0, i32 24, ptr nonnull elementtype(i64) %"foo_$B.field_6$.spacing$_clone", i32 0)
  %"foo_$B.field_6$.spacing$_clone[]_fetch" = load i64, ptr %"foo_$B.field_6$.spacing$_clone[]", align 1
  %"foo_$B.addr_a0$_clone_fetch[]" = call ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8 0, i64 1, i64 %"foo_$B.field_6$.spacing$_clone[]_fetch", ptr elementtype(i32) %"foo_$B.addr_a0$_clone_fetch", i64 2)
  store i32 20, ptr %"foo_$B.addr_a0$_clone_fetch[]", align 1

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.DATA"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i32.p0.i32(i8, i64, i32, ptr, i32) #2

; Function Attrs: nounwind readnone speculatable
declare ptr @llvm.intel.subscript.p0.i64.i64.p0.i64(i8, i64, i64, ptr, i64) #2

attributes #0 = { nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }

!omp_offload.info = !{}
; end INTEL_CUSTOMIZATION
