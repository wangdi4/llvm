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
; CHECK-NEXT:       %.vec4 = (<4 x i32>*)(%"main_$TEST_ARRAY")[0][0];
; CHECK-NEXT:       (<4 x i32>*)(%priv.mem)[0] = %.vec4 + 1;
; CHECK-NEXT:       %_ZGVbN4v_f_plus_one_ = @_ZGVbN4v_f_plus_one_(&((<4 x i32*>)(%priv.mem.bc)[<i32 0, i32 1, i32 2, i32 3>]));
; CHECK-NEXT:       (<4 x i32>*)(%"main_$TEST_ARRAY")[0][0] = %_ZGVbN4v_f_plus_one_;
; CHECK-NEXT:       %.vec4 = (<4 x i32>*)(%"main_$TEST_ARRAY")[0][4];
; CHECK-NEXT:       (<4 x i32>*)(%priv.mem)[0] = %.vec4 + 1;
; CHECK-NEXT:       %_ZGVbN4v_f_plus_one_ = @_ZGVbN4v_f_plus_one_(&((<4 x i32*>)(%priv.mem.bc)[<i32 0, i32 1, i32 2, i32 3>]));
; CHECK-NEXT:       (<4 x i32>*)(%"main_$TEST_ARRAY")[0][4] = %_ZGVbN4v_f_plus_one_;
; CHECK-NEXT: END REGION

; FIXME - GVN eliminates the second vector function call and stores the
;         result from the first call to the next 4 elements in the array.

; CHECK-LABEL: define void @MAIN__()
; CHECK:         region.24:                                        ; preds = %DIR.OMP.SIMD.1
; CHECK-NEXT:      %13 = bitcast <4 x i32>* %priv.mem to i32*
; CHECK-NEXT:      %14 = bitcast i32* %"(i32*)main_$TEST_ARRAY$" to <4 x i32>*
; CHECK-NEXT:      store <4 x i32> <i32 2, i32 3, i32 4, i32 5>, <4 x i32>* %priv.mem, align 4
; CHECK-NEXT:      %.splatinsert = insertelement <4 x i32*> poison, i32* %13, i32 0
; CHECK-NEXT:      %.splat = shufflevector <4 x i32*> %.splatinsert, <4 x i32*> poison, <4 x i32> zeroinitializer
; CHECK-NEXT:      %15 = getelementptr inbounds i32, <4 x i32*> %.splat, <4 x i64> <i64 0, i64 1, i64 2, i64 3>
; CHECK-NEXT:      %16 = call <4 x i32> @_ZGVbN4v_f_plus_one_(<4 x i32*> nonnull %15)
; CHECK-NEXT:      store <4 x i32> %16, <4 x i32>* %14, align 4
; CHECK-NEXT:      %17 = getelementptr inbounds [8 x i32], [8 x i32]* %"main_$TEST_ARRAY", i64 0, i64 4
; CHECK-NEXT:      %18 = bitcast i32* %17 to <4 x i32>*
; CHECK-NEXT:      store <4 x i32> <i32 6, i32 7, i32 8, i32 9>, <4 x i32>* %priv.mem, align 4
; CHECK-NEXT:      store <4 x i32> %16, <4 x i32>* %18, align 4

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@"var$3" = internal unnamed_addr constant [8 x i32] [i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10], align 16
@"var$7" = internal unnamed_addr constant [8 x i32] [i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8], align 16

; Function Attrs: argmemonly mustprogress nofree noinline norecurse nosync nounwind readonly willreturn uwtable
define i32 @f_plus_one_(i32* noalias nocapture readonly dereferenceable(4) %"f_plus_one_$X") local_unnamed_addr #1 {
alloca_1:
  %"f_plus_one_$X_fetch.28" = load i32, i32* %"f_plus_one_$X", align 1
  %add.39 = add nsw i32 %"f_plus_one_$X_fetch.28", 1
  ret i32 %add.39
}

; Function Attrs: nofree nosync nounwind readnone speculatable
declare i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8, i64, i64, i32*, i64) #3

; Function Attrs: argmemonly mustprogress nocallback nofree nounwind willreturn
declare void @llvm.memcpy.p0a8i32.p0a8i32.i64([8 x i32]* noalias nocapture writeonly align 16, [8 x i32]* noalias nocapture readonly align 16, i64, i1 immarg) #4

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #5

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #5

attributes #1 = { argmemonly mustprogress nofree noinline norecurse nosync nounwind readonly willreturn uwtable "denormal-fp-math"="preserve_sign" "frame-pointer"="none" "intel-lang"="fortran" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "pre_loopopt" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "vector-variants"="_ZGVbN4v_f_plus_one_" }

; Function Attrs: nounwind uwtable
define void @MAIN__() local_unnamed_addr #0 {
alloca_0:
  %"main_$I.linear.iv" = alloca i32, align 8
  %"var$9.priv" = alloca i32, align 4
  %"$io_ctx" = alloca [8 x i64], align 16
  %"main_$GOOD_ARRAY" = alloca [8 x i32], align 16
  %"main_$TEST_ARRAY" = alloca [8 x i32], align 16
  %"var$4" = alloca [8 x i32], align 16
  %"var$8" = alloca [8 x i32], align 16
  %"(&)val$" = alloca [4 x i8], align 1
  %argblock = alloca <{ i64, i8* }>, align 8
  %"(&)val$14" = alloca [4 x i8], align 1
  %argblock15 = alloca <{ i64, i8* }>, align 8
  call void @llvm.memcpy.p0a8i32.p0a8i32.i64([8 x i32]* noundef nonnull align 16 dereferenceable(32) %"var$4", [8 x i32]* noundef nonnull align 16 dereferenceable(32) @"var$3", i64 32, i1 false)
  %"(i32*)var$4$" = getelementptr inbounds [8 x i32], [8 x i32]* %"var$4", i64 0, i64 0
  %"(i32*)main_$GOOD_ARRAY$" = getelementptr inbounds [8 x i32], [8 x i32]* %"main_$GOOD_ARRAY", i64 0, i64 0
  br label %loop_body11

loop_body11:                                      ; preds = %alloca_0, %loop_body11
  %"$loop_ctr.051" = phi i64 [ 1, %alloca_0 ], [ %add.16, %loop_body11 ]
  %"var$4[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"(i32*)var$4$", i64 %"$loop_ctr.051")
  %"var$4[]_fetch.3" = load i32, i32* %"var$4[]", align 4
  %"main_$GOOD_ARRAY[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"(i32*)main_$GOOD_ARRAY$", i64 %"$loop_ctr.051")
  store i32 %"var$4[]_fetch.3", i32* %"main_$GOOD_ARRAY[]", align 4
  %add.16 = add nuw nsw i64 %"$loop_ctr.051", 1
  %exitcond.not = icmp eq i64 %add.16, 9
  br i1 %exitcond.not, label %loop_exit12, label %loop_body11

loop_exit12:                                      ; preds = %loop_body11
  call void @llvm.memcpy.p0a8i32.p0a8i32.i64([8 x i32]* noundef nonnull align 16 dereferenceable(32) %"var$8", [8 x i32]* noundef nonnull align 16 dereferenceable(32) @"var$7", i64 32, i1 false)
  %"(i32*)var$8$" = getelementptr inbounds [8 x i32], [8 x i32]* %"var$8", i64 0, i64 0
  %"(i32*)main_$TEST_ARRAY$" = getelementptr inbounds [8 x i32], [8 x i32]* %"main_$TEST_ARRAY", i64 0, i64 0
  br label %loop_body18

loop_body18:                                      ; preds = %loop_exit12, %loop_body18
  %"$loop_ctr3.052" = phi i64 [ 1, %loop_exit12 ], [ %add.32, %loop_body18 ]
  %"var$8[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"(i32*)var$8$", i64 %"$loop_ctr3.052")
  %"var$8[]_fetch.8" = load i32, i32* %"var$8[]", align 4
  %"main_$TEST_ARRAY[]" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"(i32*)main_$TEST_ARRAY$", i64 %"$loop_ctr3.052")
  store i32 %"var$8[]_fetch.8", i32* %"main_$TEST_ARRAY[]", align 4
  %add.32 = add nuw nsw i64 %"$loop_ctr3.052", 1
  %exitcond54.not = icmp eq i64 %add.32, 9
  br i1 %exitcond54.not, label %DIR.OMP.SIMD.1, label %loop_body18

omp.pdo.body22:                                   ; preds = %DIR.OMP.SIMD.1, %omp.pdo.body22
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.1 ], [ %indvars.iv.next, %omp.pdo.body22 ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %"main_$TEST_ARRAY[]11" = call i32* @llvm.intel.subscript.p0i32.i64.i64.p0i32.i64(i8 0, i64 1, i64 4, i32* nonnull elementtype(i32) %"(i32*)main_$TEST_ARRAY$", i64 %indvars.iv.next)
  %"main_$TEST_ARRAY[]_fetch.16" = load i32, i32* %"main_$TEST_ARRAY[]11", align 4
  %add.35 = add nsw i32 %"main_$TEST_ARRAY[]_fetch.16", 1
  store i32 %add.35, i32* %"var$9.priv", align 4
  %func_result5 = call i32 @f_plus_one_(i32* nonnull %"var$9.priv")
  store i32 %func_result5, i32* %"main_$TEST_ARRAY[]11", align 4
  %exitcond55.not = icmp eq i64 %indvars.iv.next, 8
  br i1 %exitcond55.not, label %DIR.OMP.END.SIMD.1, label %omp.pdo.body22

DIR.OMP.SIMD.1:                                   ; preds = %loop_body18
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %"main_$I.linear.iv", i32 1), "QUAL.OMP.PRIVATE"(i32* %"var$9.priv"), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LIVEIN"([8 x i32]* %"main_$TEST_ARRAY") ]
  br label %omp.pdo.body22

DIR.OMP.END.SIMD.1:                               ; preds = %omp.pdo.body22
  br label %DIR.OMP.END.SIMD.159

DIR.OMP.END.SIMD.159:                             ; preds = %DIR.OMP.END.SIMD.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %bb14

bb14:                                             ; preds = %DIR.OMP.END.SIMD.159
  ret void
}
