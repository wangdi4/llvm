; INTEL_CUSTOMIZATION
; RUN: opt -passes='function(vpo-cfg-restructuring,loop-simplify,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s

; Test src:
;
; The test IR was reduced/hand modified from an original test containing
; something like:
;
; program test
;     type inner_t
;         integer                 :: inner_id
;         integer, allocatable    :: inner_stuff(:)
;     end type inner_t
;
;     type outer_t
;         integer                    :: outer_id
;         type(inner_t), allocatable :: stuff(:)
;     end type outer_t
;
;     type(outer_t), allocatable      :: collection
;
;     !$omp parallel private(collection)
;       ...
;     !$omp end parallel
; end program test

; Check that constructor/destructor functions are called on the privatized
; allocatable scalar "collection".

; CHECK:  %"test_$COLLECTION.priv" = alloca ptr, align 8
; CHECK:  call void @"TEST$.btOUTER_T.omp.mold_ctor_deref"(ptr %"test_$COLLECTION.priv", ptr %"test_$COLLECTION")
; CHECK:  call void @"TEST$.btOUTER_T.omp.dtor_deref"(ptr %"test_$COLLECTION.priv")

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @MAIN__() {
alloca_0:
  %"test_$COLLECTION" = alloca ptr, align 8
  br label %bb_new5

bb_new5:                                          ; preds = %alloca_0
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(),
    "QUAL.OMP.PRIVATE:F90_NONPOD.TYPED"(ptr %"test_$COLLECTION", ptr null, i64 1, ptr @"TEST$.btOUTER_T.omp.mold_ctor_deref", ptr @"TEST$.btOUTER_T.omp.dtor_deref") ]

  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]

  ret void
}

declare token @llvm.directive.region.entry()

declare void @"TEST$.btOUTER_T.omp.mold_ctor_deref"(ptr %dst, ptr %src)

declare void @"TEST$.btOUTER_T.omp.dtor_deref"(ptr %old)

declare void @llvm.directive.region.exit(token)
; end INTEL_CUSTOMIZATION
