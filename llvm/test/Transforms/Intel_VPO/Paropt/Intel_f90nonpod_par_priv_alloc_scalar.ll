; INTEL_CUSTOMIZATION
; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
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
; CHECK:  %"do_test_$COLLECTION.priv" = alloca %"DO_TEST$.btOUTER_T5"*, align 8
; CHECK:  call void @"%DO_TEST$.btOUTER_T.omp.copy_ctor_deref1"(%"DO_TEST$.btOUTER_T5"** %"do_test_$COLLECTION.priv", %"DO_TEST$.btOUTER_T5"** @"do_test_$COLLECTION")
; CHECK:  call void @"%DO_TEST$.btOUTER_T.omp.dtor_deref1"(%"DO_TEST$.btOUTER_T5"** %"do_test_$COLLECTION.priv")

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"DO_TEST$.btOUTER_T5" = type { i32, %"QNCA_a0$%\22DO_TEST$.btINNER_T\22*$rank1$6" }
%"QNCA_a0$%\22DO_TEST$.btINNER_T\22*$rank1$6" = type { %"DO_TEST$.btINNER_T7"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"DO_TEST$.btINNER_T7" = type { i32, %"QNCA_a0$i32*$rank1$8" }
%"QNCA_a0$i32*$rank1$8" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@"do_test_$COLLECTION" = external global %"DO_TEST$.btOUTER_T5"*, align 8

define void @MAIN__() #0 {
bb4:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE:F90_NONPOD"(%"DO_TEST$.btOUTER_T5"** @"do_test_$COLLECTION", void (%"DO_TEST$.btOUTER_T5"**, %"DO_TEST$.btOUTER_T5"**)* @"%DO_TEST$.btOUTER_T.omp.copy_ctor_deref1", void (%"DO_TEST$.btOUTER_T5"**)* @"%DO_TEST$.btOUTER_T.omp.dtor_deref1") ]
  br label %bb23

bb23:                                             ; preds = %bb4
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ]
  ret void
}


declare void @"%DO_TEST$.btOUTER_T.omp.copy_ctor_deref1"(%"DO_TEST$.btOUTER_T5"**, %"DO_TEST$.btOUTER_T5"**) #0

declare void @"%DO_TEST$.btOUTER_T.omp.dtor_deref1"(%"DO_TEST$.btOUTER_T5"**) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind }
; end INTEL_CUSTOMIZATION
