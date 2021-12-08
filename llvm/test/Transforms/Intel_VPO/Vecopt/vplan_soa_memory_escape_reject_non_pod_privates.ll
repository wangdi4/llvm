;; Check that we bail out for non-POD privates.

; RUN: opt -S -vplan-vec -vplan-force-vf=2 -vplan-enable-masked-variant=0 -vplan-enable-soa -vplan-dump-soa-info -disable-vplan-codegen %s 2>&1 | FileCheck %s

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -vplan-enable-masked-variant=0 -vplan-enable-soa-hir -vplan-dump-soa-info\
; RUN: -disable-output  -disable-vplan-codegen %s 2>&1 | FileCheck %s

; REQUIRES: asserts

; CHECK: SOA profitability:
; CHECK: SOAUnsafe = value.priv

%struct.ClassA = type { i32 }

define dso_local void @_Z4funcPiS_i(i32* nocapture %dst, i32* nocapture readonly %src, i32 %n) local_unnamed_addr #2 {
entry:
  %value.priv = alloca %struct.ClassA, align 4
  %value.priv.constr = call %struct.ClassA* @_ZTS6ClassA.omp.def_constr(%struct.ClassA* %value.priv)
  %cmp3.not20 = icmp slt i32 %n, 1
  br i1 %cmp3.not20, label %omp.precond.end, label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD"(%struct.ClassA* %value.priv, %struct.ClassA* (%struct.ClassA*)* @_ZTS6ClassA.omp.def_constr, void (%struct.ClassA*)* @_ZTS6ClassA.omp.destr) ]
  br label %DIR.OMP.SIMD.128

DIR.OMP.SIMD.128:                                 ; preds = %DIR.OMP.SIMD.1
  %m_value = getelementptr inbounds %struct.ClassA, %struct.ClassA* %value.priv, i64 0, i32 0
  %wide.trip.count27 = zext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.128, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.128 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %iv.trunc = trunc i64 %indvars.iv to i32
  call void @_ZN6ClassA3incEi(%struct.ClassA* nonnull dereferenceable(4) %value.priv, i32 %iv.trunc) #4
  %ptridx = getelementptr inbounds i32, i32* %src, i64 %indvars.iv
  %src.ld = load i32, i32* %ptridx, align 4
  %m_value.ld = load i32, i32* %m_value, align 4
  %add5 = add nsw i32 %src.ld, %m_value.ld
  %ptridx7 = getelementptr inbounds i32, i32* %dst, i64 %indvars.iv
  %dst.ld = load i32, i32* %ptridx7, align 4
  %add8 = add nsw i32 %add5, %dst.ld
  store i32 %add8, i32* %ptridx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count27
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.226, label %omp.inner.for.body

DIR.OMP.END.SIMD.226:                             ; preds = %omp.inner.for.body
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.226
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @_ZTS6ClassA.omp.destr(%struct.ClassA* nonnull %value.priv)
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

declare token @llvm.directive.region.entry() #4

declare void @llvm.directive.region.exit(token) #4

declare %struct.ClassA* @_ZTS6ClassA.omp.def_constr(%struct.ClassA* nonnull returned %0) #5

declare void @_ZTS6ClassA.omp.destr(%struct.ClassA* nocapture readnone %0) #5

declare void @_ZN6ClassA3incEi(%struct.ClassA* nocapture nonnull dereferenceable(4) %this, i32 %a) #1
