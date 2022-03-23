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
;     ...
;     !$omp do lastprivate(collection)
;       ...
;     !$omp end do
; end program test

; Check that constructor/copy-assign/destructor functions are called on the
; privatized allocatable scalar "collection".
; CHECK:  %"do_test_$COLLECTION.lpriv" = alloca %"DO_TEST$.btOUTER_T5"*, align 8
; CHECK:  call void @"%DO_TEST$.btOUTER_T.omp.copy_ctor_deref1"(%"DO_TEST$.btOUTER_T5"** %"do_test_$COLLECTION.lpriv", %"DO_TEST$.btOUTER_T5"** @"do_test_$COLLECTION")
; CHECK:  call void @"%DO_TEST$.btOUTER_T.omp.copy_assign_deref1"(%"DO_TEST$.btOUTER_T5"** @"do_test_$COLLECTION", %"DO_TEST$.btOUTER_T5"** %"do_test_$COLLECTION.lpriv")
; CHECK:  call void @"%DO_TEST$.btOUTER_T.omp.dtor_deref1"(%"DO_TEST$.btOUTER_T5"** %"do_test_$COLLECTION.lpriv")

target triple = "x86_64-unknown-linux-gnu"

%"DO_TEST$.btOUTER_T5" = type { i32, %"QNCA_a0$%\22DO_TEST$.btINNER_T\22*$rank1$6" }
%"QNCA_a0$%\22DO_TEST$.btINNER_T\22*$rank1$6" = type { %"DO_TEST$.btINNER_T7"*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }
%"DO_TEST$.btINNER_T7" = type { i32, %"QNCA_a0$i32*$rank1$8" }
%"QNCA_a0$i32*$rank1$8" = type { i32*, i64, i64, i64, i64, i64, [1 x { i64, i64, i64 }] }

@"do_test_$COLLECTION" = external global %"DO_TEST$.btOUTER_T5"*, align 8

define void @MAIN__() #0 {
entry:
  %.omp.ub = alloca i32, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.LOOP"(),
    "QUAL.OMP.LASTPRIVATE:F90_NONPOD"(%"DO_TEST$.btOUTER_T5"** @"do_test_$COLLECTION", void (%"DO_TEST$.btOUTER_T5"**, %"DO_TEST$.btOUTER_T5"**)* @"%DO_TEST$.btOUTER_T.omp.copy_ctor_deref1", void (%"DO_TEST$.btOUTER_T5"**, %"DO_TEST$.btOUTER_T5"**)* @"%DO_TEST$.btOUTER_T.omp.copy_assign_deref1", void (%"DO_TEST$.btOUTER_T5"**)* @"%DO_TEST$.btOUTER_T.omp.dtor_deref1"),
    "QUAL.OMP.NORMALIZED.IV"(i32* undef),
    "QUAL.OMP.FIRSTPRIVATE"(i32* undef),
    "QUAL.OMP.NORMALIZED.UB"(i32* %.omp.ub) ]
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %omp.inner.for.body, %entry
  %1 = load i32, i32* undef, align 4
  %cmp = icmp sle i32 %1, undef
  br i1 %cmp, label %omp.inner.for.body, label %omp.loop.exit

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %2 = load i32, i32* undef, align 4
  %add2 = add nsw i32 %2, 1
  store i32 %add2, i32* undef, align 4
  br label %omp.inner.for.cond

omp.loop.exit:                                    ; preds = %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.LOOP"() ]
  ret void
}

declare void @"%DO_TEST$.btOUTER_T.omp.copy_ctor_deref1"(%"DO_TEST$.btOUTER_T5"**, %"DO_TEST$.btOUTER_T5"**) #0
declare void @"%DO_TEST$.btOUTER_T.omp.copy_assign_deref1"(%"DO_TEST$.btOUTER_T5"**, %"DO_TEST$.btOUTER_T5"**) #0
declare void @"%DO_TEST$.btOUTER_T.omp.dtor_deref1"(%"DO_TEST$.btOUTER_T5"**) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { "target-features"="+avx,+avx2,+bmi,+bmi2,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" }
attributes #1 = { nounwind }
; end INTEL_CUSTOMIZATION
