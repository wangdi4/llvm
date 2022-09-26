; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -print-after=hir-vplan-vec -vplan-force-vf=2 -S -disable-output -vplan-vec-scenario="n0;v2;m2" < %s 2>&1 | FileCheck %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.str = type { i32 }

; Function Attrs: nounwind uwtable
define dso_local noundef i32 @_Z3foov() local_unnamed_addr {
; CHECK:               %2 = bitcast.<2 x i1>.i2(%.vec21);
; CHECK-NEXT:          %cmp25 = %2 == 0;
; CHECK-NEXT:          %all.zero.check26 = %cmp25;
; CHECK-NEXT:          if (%cmp25 == 1)
; CHECK-NEXT:          {
; CHECK-NEXT:             goto BB19.91;
; CHECK-NEXT:          }
; CHECK-NEXT:          %bsfintmask = bitcast.<2 x i1>.i2(%.vec21);
; CHECK-NEXT:          %bsf = @llvm.ctlz.i2(%bsfintmask,  1);
; CHECK-NEXT:          %ext.lane = 1  -  %bsf;
; CHECK-NEXT:          %priv.extract = extractelement &((<2 x %struct.str*>)(%priv.mem.bc14)[<i32 0, i32 1>]),  %ext.lane;
; CHECK-NEXT:          @_ZTS3str.omp.copy_assign(%x.lpriv,  %priv.extract);
; CHECK-NEXT:          @_ZTS3str.omp.destr(&((%struct.str*)(%priv.mem13)[0]));
; CHECK-NEXT:          %extract.1.27 = extractelement &((<2 x %struct.str*>)(%priv.mem.bc14)[<i32 0, i32 1>]),  1;
; CHECK-NEXT:          @_ZTS3str.omp.destr(%extract.1.27);
; CHECK-NEXT:          BB19.91:
; CHECK-NEXT:          %phi.temp10 = 9999;
; CHECK-NEXT:          final.merge.58:
;
DIR.OMP.SIMD.116:
  %x.lpriv = alloca %struct.str, align 4
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.116
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:NONPOD"(%struct.str* %x.lpriv, %struct.str* (%struct.str*)* @_ZTS3str.omp.def_constr, void (%struct.str*, %struct.str*)* @_ZTS3str.omp.copy_assign, void (%struct.str*)* @_ZTS3str.omp.destr), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %i.linear.iv, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1
  %1 = bitcast i32* %i.linear.iv to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %.omp.iv.local.010 = phi i32 [ 0, %DIR.OMP.SIMD.2 ], [ %add, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %1)
  %add = add nuw nsw i32 %.omp.iv.local.010, 1
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %1)
  %exitcond.not = icmp eq i32 %add, 9999
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.217, label %omp.inner.for.body

DIR.OMP.END.SIMD.217:                             ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.217
  ret i32 9999
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare %struct.str* @_ZTS3str.omp.def_constr(%struct.str* noundef readnone returned %0)

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn uwtable
declare void @_ZTS3str.omp.copy_assign(%struct.str* nocapture noundef writeonly %0, %struct.str* nocapture noundef readonly %1)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare void @_ZTS3str.omp.destr(%struct.str* nocapture noundef readnone %0)

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
