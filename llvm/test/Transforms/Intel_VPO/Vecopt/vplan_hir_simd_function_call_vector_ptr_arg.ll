; RUN: opt -passes="vec-clone,hir-ssa-deconstruction,hir-temp-cleanup,hir-vec-dir-insert,hir-vplan-vec,hir-cg,sroa,gvn" -vplan-force-vf=4 -disable-output -print-after=hir-vplan-vec -print-after=gvn -S < %s 2>&1 | FileCheck %s

; Test that shows the loop getting vectorized with VF=4 and unrolled by two
; with calls to a vector function with a vector of ptr arg. GVN ends up making 
; an illegal transformation by eliminating one of the calls and storing undef
; to private memory. Once the arg ptr type is widened to vector of pointers,
; analyses such as AA/ModRef and MemorySSA can no longer correctly reason
; about aliasing and MemoryDefs. E.g., they do checks on isPointerTy(), etc.
; Therefore, since many transformations rely on the accuracy of this
; information, many illegal transformations can occur.

; CHECK-LABEL: Function: MAIN__
; CHECK-EMPTY:
; CHECK:      BEGIN REGION { modified }
; CHECK:            %priv.mem.bc = &((i32*)(%priv.mem)[0]);
; CHECK-NEXT:       %.vec = (<4 x i32>*)(%TEST_ARRAY)[0];
; CHECK-NEXT:       (<4 x i32>*)(%priv.mem)[0] = %.vec;
; CHECK-NEXT:       %_ZGVbN4v_f_plus_one_ = @_ZGVbN4v_f_plus_one_(&((<4 x i32*>)(%priv.mem.bc)[<i32 0, i32 1, i32 2, i32 3>]));
; CHECK-NEXT:       (<4 x i32>*)(%TEST_ARRAY)[0] = %_ZGVbN4v_f_plus_one_;
; CHECK-NEXT:       %.vec = (<4 x i32>*)(%TEST_ARRAY)[4];
; CHECK-NEXT:       (<4 x i32>*)(%priv.mem)[0] = %.vec;
; CHECK-NEXT:       %_ZGVbN4v_f_plus_one_ = @_ZGVbN4v_f_plus_one_(&((<4 x i32*>)(%priv.mem.bc)[<i32 0, i32 1, i32 2, i32 3>]));
; CHECK-NEXT:       (<4 x i32>*)(%TEST_ARRAY)[4] = %_ZGVbN4v_f_plus_one_;
; CHECK-NEXT: END REGION

; FIXME - GVN eliminates the second vector function call and stores the
;         result from the first call to the next 4 elements in the array.

; CHECK-LABEL: define void @MAIN__(i32* %TEST_ARRAY)
; CHECK:         region.0:
; CHECK-NEXT:      %1 = bitcast <4 x i32>* %priv.mem to i32*
; CHECK-NEXT:      %2 = bitcast i32* %TEST_ARRAY to <4 x i32>*
; CHECK-NEXT:      %gepload = load <4 x i32>, <4 x i32>* %2, align 4
; CHECK-NEXT:      store <4 x i32> %gepload, <4 x i32>* %priv.mem, align 4
; CHECK-NEXT:      %.splatinsert = insertelement <4 x i32*> poison, i32* %1, i32 0
; CHECK-NEXT:      %.splat = shufflevector <4 x i32*> %.splatinsert, <4 x i32*> poison, <4 x i32> zeroinitializer
; CHECK-NEXT:      %3 = getelementptr inbounds i32, <4 x i32*> %.splat, <4 x i64> <i64 0, i64 1, i64 2, i64 3>
; CHECK-NEXT:      %4 = call <4 x i32> @_ZGVbN4v_f_plus_one_(<4 x i32*> nonnull %3)
; CHECK-NEXT:      store <4 x i32> %4, <4 x i32>* %2, align 4
; CHECK-NEXT:      %5 = getelementptr inbounds i32, i32* %TEST_ARRAY, i64 4
; CHECK-NEXT:      %6 = bitcast i32* %5 to <4 x i32>*
; CHECK-NEXT:      %gepload3 = load <4 x i32>, <4 x i32>* %6, align 4
; CHECK-NEXT:      store <4 x i32> %gepload3, <4 x i32>* %priv.mem, align 4
; CHECK-NEXT:      store <4 x i32> %4, <4 x i32>* %6, align 4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

declare i32 @f_plus_one_(i32*) #1
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #3
declare token @llvm.directive.region.entry() #5
declare void @llvm.directive.region.exit(token) #5

attributes #1 = { argmemonly mustprogress nofree noinline norecurse nosync nounwind readonly willreturn uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "vector-variants"="_ZGVbN4v_f_plus_one_" }

; Function Attrs: nounwind uwtable
define void @MAIN__(i32 *%TEST_ARRAY) local_unnamed_addr #0 {
alloca_0:
  %"var$9.priv" = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE"(i32* %"var$9.priv")]
  br label %omp.pdo.body22

omp.pdo.body22:                                   ; preds = %DIR.OMP.SIMD.1, %omp.pdo.body22
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.pdo.body22 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %index = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"TEST_ARRAY", i64 %indvars.iv.next)
  %val = load i32, i32* %index, align 4
  store i32 %val, i32* %"var$9.priv", align 4
  %func_result5 = call i32 @f_plus_one_(i32* nonnull %"var$9.priv")
  store i32 %func_result5, i32* %index, align 4
  %exitcond55.not = icmp eq i64 %indvars.iv.next, 8
  br i1 %exitcond55.not, label %DIR.OMP.END.SIMD.1, label %omp.pdo.body22

DIR.OMP.END.SIMD.1:                               ; preds = %omp.pdo.body22
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %bb14

bb14:                                             ; preds = %DIR.OMP.END.SIMD.159
  ret void
}
