; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -vpo-paropt-use-mapper-api=false -S %s | FileCheck %s

; Test for F90_DV operands on both map and use_device_ptr clause on a target data construct.
; It's similar to Intel_fort_dv_target_data_map_use_dev_ptr.ll, but the operand is not an F90_DV.

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
;       integer :: B(10)
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

; Function Attrs: nounwind uwtable
define void @foo_(i32* %"foo_$B") #0 {
alloca_0:
  %"foo_$B_entry" = bitcast i32* %"foo_$B" to [10 x i32]*

  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET.DATA"(), "QUAL.OMP.USE_DEVICE_PTR"([10 x i32]* %"foo_$B_entry"), "QUAL.OMP.MAP.TOFROM"([10 x i32]* %"foo_$B_entry", [10 x i32]* %"foo_$B_entry", i64 40, i64 3, i8* null, i8* null) ]

; Check that only one map is created, and map-type for %"foo_$B_entry" is
; RETURN_PARAM | TO_FROM, i.e. 67 (0x23).
; CHECK: @.offload_maptypes = private unnamed_addr constant [1 x i64] [i64 67]

; Check that the value in the baseptrs struct after the tgt_data call is
; used inside the region as the updated value of the pointer %"foo_$B_entry".
; CHECK: [[GEP:%[^ ]+]] = getelementptr inbounds [1 x i8*], [1 x i8*]* %.offload_baseptrs, i32 0, i32 0
; CHECK: call void @__tgt_target_data_begin({{.+}})
; CHECK: [[GEP_CAST:%[^ ]+]] = bitcast i8** [[GEP]] to [10 x i32]**
; CHECK: %"foo_$B_entry.updated.val" = load [10 x i32]*, [10 x i32]** [[GEP_CAST]]

; Check that call to outlined function for target data uses %array_device.new
; CHECK: call void @foo_.DIR.OMP.TARGET.DATA{{[^ ]+}}([10 x i32]* %"foo_$B_entry.updated.val")

  %ptr_cast = bitcast [10 x i32]* %"foo_$B_entry" to i32*
  %"foo_$B_entry[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* elementtype(i32) %ptr_cast, i64 2)
  store i32 20, i32* %"foo_$B_entry[]", align 1

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET.DATA"() ]

  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { nounwind uwtable "intel-lang"="fortran" "min-legal-vector-width"="0" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }

!omp_offload.info = !{}
; end INTEL_CUSTOMIZATION
