; RUN: opt -passes='hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>' -vplan-force-vf=2 -S -disable-output -vplan-vec-scenario="n0;v2;m2" < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.str = type { i32 }

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @_Z3foov() local_unnamed_addr {
; CHECK:               %1 = bitcast.<2 x i1>.i2([[VEC:%.*]]);
; CHECK-NEXT:          [[CMP:%.*]] = %1 == 0;
; CHECK-NEXT:          [[ALL_ZERO_CHECK:%all.zero.*]] = [[CMP]];
; CHECK-NEXT:          if ([[CMP]] == 1)
; CHECK-NEXT:          {
; CHECK-NEXT:             goto [[BB19:BB.*]];
; CHECK-NEXT:          }
; CHECK-NEXT:          %bsfintmask = bitcast.<2 x i1>.i2([[VEC]]);
; CHECK-NEXT:          %bsf = @llvm.ctlz.i2(%bsfintmask,  1);
; CHECK-NEXT:          %ext.lane = 1  -  %bsf;
; CHECK-NEXT:          %priv.extract = extractelement &((<2 x ptr>)([[PRIV_MEM1:%priv.mem.*]])[<i32 0, i32 1>]),  %ext.lane;
; CHECK-NEXT:          @_ZTS3str.omp.copy_assign(%x.lpriv,  %priv.extract);
; CHECK-NEXT:          @_ZTS3str.omp.destr(&((%struct.str*)([[PRIV_MEM2:%priv.mem.*]])[0]));
; CHECK-NEXT:          [[EXTRACT1:%.*]] = extractelement &((<2 x ptr>)([[PRIV_MEM1]])[<i32 0, i32 1>]),  1;
; CHECK-NEXT:          @_ZTS3str.omp.destr([[EXTRACT1]]);
; CHECK-NEXT:          [[BB19]]:
; CHECK-NEXT:          [[PHI:%phi.*]] = 9999;
; CHECK-NEXT:          [[FINAL_MERGE:final.merge.*]]:
;
DIR.OMP.SIMD.116:
  %x.lpriv = alloca %struct.str, align 4
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.116
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:NONPOD.TYPED"(ptr %x.lpriv, %struct.str zeroinitializer, i32 1, ptr @_ZTS3str.omp.def_constr, ptr @_ZTS3str.omp.copy_assign, ptr @_ZTS3str.omp.destr), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %.omp.iv.local.010 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i.linear.iv)
  %add = add nuw nsw i32 %.omp.iv.local.010, 1
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.linear.iv)
  %exitcond.not = icmp eq i32 %add, 9999
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.217, label %omp.inner.for.body

DIR.OMP.END.SIMD.217:                             ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.217
  ret i32 9999
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare ptr @_ZTS3str.omp.def_constr(ptr noundef readnone returned %0)

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn uwtable
declare void @_ZTS3str.omp.copy_assign(ptr nocapture noundef writeonly %0, ptr nocapture noundef readonly %i.linear.iv)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare void @_ZTS3str.omp.destr(ptr nocapture noundef readnone %0)

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)
