;; Check that we bail out for non-POD privates.

; RUN: opt -S -passes=vplan-vec -vplan-force-vf=2 -vplan-enable-masked-variant=0 -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen %s 2>&1 | FileCheck %s

; RUN: opt -passes='hir-ssa-deconstruction,hir-vplan-vec,print<hir>' -vplan-force-vf=2 -vplan-enable-masked-variant=0 -vplan-enable-soa-hir -vplan-dump-soa-info\
; RUN: -disable-output  -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES: asserts

; CHECK: SOA profitability:
; CHECK: SOAUnsafe = [[VP_VALUE_PRIV:%.*]] (value.priv)

%struct.ClassA = type { i32 }

define dso_local void @_Z4funcPiS_i(ptr nocapture %dst, ptr nocapture readonly %src, i32 %n) local_unnamed_addr #2 {
entry:
  %value.priv = alloca %struct.ClassA, align 4
  %value.priv.constr = call ptr @_ZTS6ClassA.omp.def_constr(ptr %value.priv)
  %cmp3.not20 = icmp slt i32 %n, 1
  br i1 %cmp3.not20, label %omp.precond.end, label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr %value.priv, %struct.ClassA zeroinitializer, i32 1, ptr @_ZTS6ClassA.omp.def_constr, ptr @_ZTS6ClassA.omp.destr) ]
  br label %DIR.OMP.SIMD.128

DIR.OMP.SIMD.128:                                 ; preds = %DIR.OMP.SIMD.1
  %wide.trip.count27 = zext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.128, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.128 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %iv.trunc = trunc i64 %indvars.iv to i32
  call void @_ZN6ClassA3incEi(ptr nonnull dereferenceable(4) %value.priv, i32 %iv.trunc) #4
  %ptridx = getelementptr inbounds i32, ptr %src, i64 %indvars.iv
  %src.ld = load i32, ptr %ptridx, align 4
  %m_value.ld = load i32, ptr %value.priv, align 4
  %add5 = add nsw i32 %src.ld, %m_value.ld
  %ptridx7 = getelementptr inbounds i32, ptr %dst, i64 %indvars.iv
  %dst.ld = load i32, ptr %ptridx7, align 4
  %add8 = add nsw i32 %add5, %dst.ld
  store i32 %add8, ptr %ptridx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count27
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.226, label %omp.inner.for.body

DIR.OMP.END.SIMD.226:                             ; preds = %omp.inner.for.body
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.226
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @_ZTS6ClassA.omp.destr(ptr nonnull %value.priv)
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

declare token @llvm.directive.region.entry() #4

declare void @llvm.directive.region.exit(token) #4

declare ptr @_ZTS6ClassA.omp.def_constr(ptr nonnull returned %0) #5

declare void @_ZTS6ClassA.omp.destr(ptr nocapture readnone %0) #5

declare void @_ZN6ClassA3incEi(ptr nocapture nonnull dereferenceable(4) %this, i32 %a) #1
