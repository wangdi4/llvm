; RUN: opt -instcombine -S < %s 2>&1 | FileCheck %s
; RUN: opt -passes=instcombine -S < %s 2>&1 | FileCheck %s
;
; IR for this test was derrived from the target code of the following source
;
; #include <complex>
;
; void foo(std::complex<float> *A, std::complex<float> *B, std::complex<float> *C, int N) {
;   auto lambda = [=](int J) {
;     std::complex<float> X = A[J];
;     std::complex<float> Y = B[J];
;
;     for (int K = 0; K < 100; ++K)
;       Y *= X;
;
;     C[J] = Y;
;   };
;
; #pragma omp target teams distribute parallel for
;   for (int I = 0; I < N; ++I)
;     lambda(I);
; }
;
; Check that instcombine removes redundant llvm.stacksave/llvm.stackrestore calls.
;
; CHECK-NOT: call{{.*}} @llvm.stacksave
; CHECK-NOT: call{{.*}} @llvm.stackrestore

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"

%class._ZTSZ3fooPSt7complexIfES1_S1_iEUliE_ = type { %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* }
%"struct.std::complex" = type { { float, float } }

define weak dso_local spir_kernel void @__omp_offloading_36_d68c2cca__Z3fooPSt7complexIfES1_S1_i_l14(%class._ZTSZ3fooPSt7complexIfES1_S1_iEUliE_ addrspace(1)* noalias %lambda.ascast, i64 %.omp.lb.ascast.val63.zext, i64 %.omp.ub.ascast.val.zext, i64 %.capture_expr.0.ascast.val.zext) {
DIR.OMP.TEAMS.5:
  %.capture_expr.0.ascast.val.zext.trunc = trunc i64 %.capture_expr.0.ascast.val.zext to i32
  %.omp.ub.ascast.val.zext.trunc = trunc i64 %.omp.ub.ascast.val.zext to i32
  %.omp.lb.ascast.val63.zext.trunc = trunc i64 %.omp.lb.ascast.val63.zext to i32
  %cmp = icmp sle i32 %.capture_expr.0.ascast.val.zext.trunc, 0
  %.omp.lb.ascast.val.zext = zext i32 %.omp.lb.ascast.val63.zext.trunc to i64
  %cmp3.not55 = icmp sgt i32 %.omp.lb.ascast.val63.zext.trunc, %.omp.ub.ascast.val.zext.trunc
  %or.cond = or i1 %cmp, %cmp3.not55
  br i1 %or.cond, label %DIR.OMP.END.TARGET.1354.exitStub, label %omp.inner.for.body.lr.ph

DIR.OMP.END.TARGET.1354.exitStub:                 ; preds = %_ZZ3fooPSt7complexIfES1_S1_iENKUliE_clEi.exit, %3, %DIR.OMP.TEAMS.5
  ret void

omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.TEAMS.5
  %0 = call spir_func i64 @_Z13get_global_idj(i32 0)
  %1 = trunc i64 %0 to i32
  %2 = icmp slt i32 %1, %.omp.ub.ascast.val.zext.trunc
  br i1 %2, label %omp.inner.for.body.preheader, label %3

3:                                                ; preds = %omp.inner.for.body.lr.ph
  %4 = icmp sle i32 %1, %.omp.ub.ascast.val.zext.trunc
  %5 = icmp eq i32 %.omp.ub.ascast.val.zext.trunc, %.omp.ub.ascast.val.zext.trunc
  %6 = and i1 %4, %5
  br i1 %4, label %omp.inner.for.body.preheader, label %DIR.OMP.END.TARGET.1354.exitStub

omp.inner.for.body.preheader:                     ; preds = %3, %omp.inner.for.body.lr.ph
  %loop0.upper.bnd.019.ph = phi i32 [ %1, %omp.inner.for.body.lr.ph ], [ %.omp.ub.ascast.val.zext.trunc, %3 ]
  %7 = getelementptr inbounds %class._ZTSZ3fooPSt7complexIfES1_S1_iEUliE_, %class._ZTSZ3fooPSt7complexIfES1_S1_iEUliE_ addrspace(1)* %lambda.ascast, i64 0, i32 2
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %_ZZ3fooPSt7complexIfES1_S1_iENKUliE_clEi.exit, %omp.inner.for.body.preheader
  %.omp.iv.ascast.local.056 = phi i32 [ %add5, %_ZZ3fooPSt7complexIfES1_S1_iENKUliE_clEi.exit ], [ %1, %omp.inner.for.body.preheader ]
  %savedstack = call i8* @llvm.stacksave()
  %8 = getelementptr inbounds %class._ZTSZ3fooPSt7complexIfES1_S1_iEUliE_, %class._ZTSZ3fooPSt7complexIfES1_S1_iEUliE_ addrspace(1)* %lambda.ascast, i64 0, i32 0
  %9 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(1)* %8, align 8
  %idxprom.i = sext i32 %.omp.iv.ascast.local.056 to i64
  %ptridx.i = getelementptr inbounds %"struct.std::complex", %"struct.std::complex" addrspace(4)* %9, i64 %idxprom.i
  %.sroa_cast10 = bitcast %"struct.std::complex" addrspace(4)* %ptridx.i to i32 addrspace(4)*
  %10 = load i32, i32 addrspace(4)* %.sroa_cast10, align 4
  %.sroa_idx11 = getelementptr inbounds %"struct.std::complex", %"struct.std::complex" addrspace(4)* %ptridx.i, i64 0, i32 0, i32 1
  %.sroa_cast12 = bitcast float addrspace(4)* %.sroa_idx11 to i32 addrspace(4)*
  %11 = load i32, i32 addrspace(4)* %.sroa_cast12, align 4
  %12 = getelementptr inbounds %class._ZTSZ3fooPSt7complexIfES1_S1_iEUliE_, %class._ZTSZ3fooPSt7complexIfES1_S1_iEUliE_ addrspace(1)* %lambda.ascast, i64 0, i32 1
  %13 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(1)* %12, align 8
  %ptridx3.i = getelementptr inbounds %"struct.std::complex", %"struct.std::complex" addrspace(4)* %13, i64 %idxprom.i
  %.sroa_cast3 = bitcast %"struct.std::complex" addrspace(4)* %ptridx3.i to i32 addrspace(4)*
  %14 = load i32, i32 addrspace(4)* %.sroa_cast3, align 4
  %.sroa_idx5 = getelementptr inbounds %"struct.std::complex", %"struct.std::complex" addrspace(4)* %ptridx3.i, i64 0, i32 0, i32 1
  %.sroa_cast6 = bitcast float addrspace(4)* %.sroa_idx5 to i32 addrspace(4)*
  %15 = load i32, i32 addrspace(4)* %.sroa_cast6, align 4
  br label %for.cond.i

for.cond.i:                                       ; preds = %for.body.i, %omp.inner.for.body
  %Y.i.sroa.4.0 = phi i32 [ %15, %omp.inner.for.body ], [ %21, %for.body.i ]
  %Y.i.sroa.0.0 = phi i32 [ %14, %omp.inner.for.body ], [ %20, %for.body.i ]
  %K.0.i = phi i32 [ 0, %omp.inner.for.body ], [ %inc.i, %for.body.i ]
  %cmp.i = icmp ult i32 %K.0.i, 100
  br i1 %cmp.i, label %for.body.i, label %_ZZ3fooPSt7complexIfES1_S1_iENKUliE_clEi.exit

for.body.i:                                       ; preds = %for.cond.i
  %16 = bitcast i32 %10 to float
  %17 = bitcast i32 %11 to float
  %18 = bitcast i32 %Y.i.sroa.0.0 to float
  %19 = bitcast i32 %Y.i.sroa.4.0 to float
  %mul_ad.i.i = fmul fast float %18, %17
  %mul_bc.i.i = fmul fast float %19, %16
  %mul_i.i.i = fadd fast float %mul_ad.i.i, %mul_bc.i.i
  %mul_ac.i.i = fmul fast float %18, %16
  %mul_bd.i.i = fmul fast float %19, %17
  %mul_r.i.i = fsub fast float %mul_ac.i.i, %mul_bd.i.i
  %20 = bitcast float %mul_r.i.i to i32
  %21 = bitcast float %mul_i.i.i to i32
  %inc.i = add nuw nsw i32 %K.0.i, 1
  br label %for.cond.i

_ZZ3fooPSt7complexIfES1_S1_iENKUliE_clEi.exit:    ; preds = %for.cond.i
  %Y.i.sroa.4.0.lcssa = phi i32 [ %Y.i.sroa.4.0, %for.cond.i ]
  %Y.i.sroa.0.0.lcssa = phi i32 [ %Y.i.sroa.0.0, %for.cond.i ]
  %22 = load %"struct.std::complex" addrspace(4)*, %"struct.std::complex" addrspace(4)* addrspace(1)* %7, align 8
  %ptridx5.i = getelementptr inbounds %"struct.std::complex", %"struct.std::complex" addrspace(4)* %22, i64 %idxprom.i
  %.sroa_cast = bitcast %"struct.std::complex" addrspace(4)* %ptridx5.i to i32 addrspace(4)*
  store i32 %Y.i.sroa.0.0.lcssa, i32 addrspace(4)* %.sroa_cast, align 4
  %.sroa_idx = getelementptr inbounds %"struct.std::complex", %"struct.std::complex" addrspace(4)* %ptridx5.i, i64 0, i32 0, i32 1
  %.sroa_cast2 = bitcast float addrspace(4)* %.sroa_idx to i32 addrspace(4)*
  store i32 %Y.i.sroa.4.0.lcssa, i32 addrspace(4)* %.sroa_cast2, align 4
  call void @llvm.stackrestore(i8* %savedstack)
  %add5 = add nsw i32 %.omp.iv.ascast.local.056, 1
  %cmp3.not = icmp sle i32 %add5, %loop0.upper.bnd.019.ph
  br i1 %cmp3.not, label %omp.inner.for.body, label %DIR.OMP.END.TARGET.1354.exitStub
}

declare i8* @llvm.stacksave()

declare void @llvm.stackrestore(i8*)

declare spir_func i64 @_Z13get_global_idj(i32)
