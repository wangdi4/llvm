; RUN: opt -passes=vplan-vec -print-after=vplan-vec -vplan-force-vf=2 -vplan-print-after-vpentity-instrs -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR

; CHECK: VPlan after insertion of VPEntities instructions:
; CHECK:   ptr [[ALLPRIV:%.*]] = allocate-priv [4 x %struct.point2d], OrigAlign = 16
; CHECK:   private-nonpod-array-ctor ptr [[ALLPRIV]]
; CHECK:   ptr [[TMP3:%.*]] = getelementptr inbounds [4 x %struct.point2d], ptr [[ALLPRIV]] i64 0 i64 [[TMP2:%.*]]
; CHECK:   call ptr [[TMP3]] i32 [[IV:%.*]] ptr @_Z3bazP7point2di
; CHECK:   private-nonpod-array-dtor ptr [[ALLPRIV]]

; CHECK: *** IR Dump After vpo::VPlanDriverLLVMPass on _Z3foov ***
; Ctor related loops
; CHECK:      array.nonpod.private.outer.loop:                  ; preds = %array.nonpod.private.outer.loop.inc, %VPlannedBB1
; CHECK-NEXT:   %1 = phi i64 [ 0, %VPlannedBB1 ], [ %7, %array.nonpod.private.outer.loop.inc ]
; CHECK-NEXT:   %priv.extract = extractelement <2 x ptr> %myPoint.priv.insert.1, i64 %1
; CHECK-NEXT:   br label %array.nonpod.private.inner.loop
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.inner.loop:                  ; preds = %array.nonpod.private.inner.loop, %array.nonpod.private.outer.loop
; CHECK-NEXT:   %2 = phi i64 [ 0, %array.nonpod.private.outer.loop ], [ %5, %array.nonpod.private.inner.loop ]
; CHECK-NEXT:   %3 = getelementptr [4 x %struct.point2d], ptr %priv.extract, i64 0, i64 %2
; CHECK-NEXT:   %4 = call ptr @_ZTS7point2d.omp.def_constr(ptr %3)
; CHECK-NEXT:   %5 = add i64 %2, 1
; CHECK-NEXT:   %6 = icmp ult i64 %5, 4
; CHECK-NEXT:   br i1 %6, label %array.nonpod.private.inner.loop, label %array.nonpod.private.outer.loop.inc
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.outer.loop.inc:              ; preds = %array.nonpod.private.inner.loop
; CHECK-NEXT:   %7 = add i64 %1, 1
; CHECK-NEXT:   %8 = icmp ult i64 %1, 2
; CHECK-NEXT:   br i1 %8, label %array.nonpod.private.outer.loop, label %array.nonpod.private.loop.exit
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.loop.exit:                   ; preds = %array.nonpod.private.outer.loop.inc
; CHECK:        br label %vector.body

; Dtor related loops
; CHECK:      array.nonpod.private.outer.loop4:                 ; preds = %array.nonpod.private.outer.loop.inc6, %VPlannedBB3
; CHECK-NEXT:   %13 = phi i64 [ 0, %VPlannedBB3 ], [ %18, %array.nonpod.private.outer.loop.inc6 ]
; CHECK-NEXT:   %priv.extract8 = extractelement <2 x ptr> %myPoint.priv.insert.1, i64 %13
; CHECK-NEXT:   br label %array.nonpod.private.inner.loop5
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.inner.loop5:                 ; preds = %array.nonpod.private.inner.loop5, %array.nonpod.private.outer.loop4
; CHECK-NEXT:   %14 = phi i64 [ 0, %array.nonpod.private.outer.loop4 ], [ %16, %array.nonpod.private.inner.loop5 ]
; CHECK-NEXT:   %15 = getelementptr [4 x %struct.point2d], ptr %priv.extract8, i64 0, i64 %14
; CHECK-NEXT:   call void @_ZTS7point2d.omp.destr(ptr %15)
; CHECK-NEXT:   %16 = add i64 %14, 1
; CHECK-NEXT:   %17 = icmp ult i64 %16, 4
; CHECK-NEXT:   br i1 %17, label %array.nonpod.private.inner.loop5, label %array.nonpod.private.outer.loop.inc6
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.outer.loop.inc6:            ; preds = %array.nonpod.private.inner.loop5
; CHECK-NEXT:   %18 = add i64 %13, 1
; CHECK-NEXT:   %19 = icmp ult i64 %13, 2
; CHECK-NEXT:   br i1 %19, label %array.nonpod.private.outer.loop4, label %array.nonpod.private.loop.exit7
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.loop.exit7:                 ; preds = %array.nonpod.private.outer.loop.inc6
; CHECK:        br label %VPlannedBB9

; Incomming HIR
; <24>         BEGIN REGION { }
; <26>               %2 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.PRIVATE:NONPOD.TYPED(&((%myPoint.priv)[0])zeroinitializer4&((@_ZTS7point2d.omp.def_constr)[0])&((@_ZTS7point2d.omp.destr)[0])),  QUAL.OMP.NORMALIZED.IV:TYPED(null0),  QUAL.OMP.NORMALIZED.UB:TYPED(null0),  QUAL.OMP.LINEAR:IV.TYPED(&((%i.linear.iv)[0])011) ]
; <49>
; <49>               + DO i1 = 0, 999, 1   <DO_LOOP> <simd>
; <33>               |   @llvm.lifetime.start.p0(4,  &((i8*)(%i.linear.iv)[0]));
; <36>               |   @_Z3bazP7point2di(&((%myPoint.priv)[0][i1]),  i1);
; <37>               |   @llvm.lifetime.end.p0(4,  &((i8*)(%i.linear.iv)[0]));
; <49>               + END LOOP
; <49>
; <45>               @llvm.directive.region.exit(%2); [ DIR.OMP.END.SIMD() ]
; <24>         END REGION

; *** IR Dump After vpo::VPlanDriverHIRPass ***
; HIR:         	    BEGIN REGION { modified }
; HIR-NEXT:               %priv.mem.bc = &(([4 x %struct.point2d]*)(%priv.mem)[0]);
; HIR:                    + DO i1 = 0, 1, 1   <DO_LOOP>
; HIR-NEXT:               |   %priv.extract = extractelement &((<2 x ptr>)(%priv.mem.bc)[<i32 0, i32 1>]),  i1;
; HIR-NEXT:               |
; HIR-NEXT:               |   + DO i2 = 0, 3, 1   <DO_LOOP>
; HIR-NEXT:               |   |   %call = @_ZTS7point2d.omp.def_constr(&((%struct.point2d*)(%priv.extract)[i2]));
; HIR-NEXT:               |   + END LOOP
; HIR-NEXT:               + END LOOP
; HIR:                    + DO i1 = 0, 999, 2   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR:                    |   @_Z3bazP7point2di(%extract{{.*}},  i1);
; HIR:                    |   @_Z3bazP7point2di(%extract{{.*}},  %extract{{.*}});
; HIR:                    + END LOOP
; HIR:                    + DO i1 = 0, 1, 1   <DO_LOOP>
; HIR-NEXT:               |   %priv.extract[[NUM:.*]] = extractelement &((<2 x ptr>)(%priv.mem.bc)[<i32 0, i32 1>]),  i1;
; HIR-NEXT:               |
; HIR-NEXT:               |   + DO i2 = 0, 3, 1   <DO_LOOP>
; HIR-NEXT:               |   |   @_ZTS7point2d.omp.destr(&((%struct.point2d*)(%priv.extract[[NUM]])[i2]));
; HIR-NEXT:               |   + END LOOP
; HIR-NEXT:               + END LOOP
; HIR-NEXT:         END REGION

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.point2d = type { i32, i32 }

; Function Attrs: nounwind uwtable
define void @_Z3foov() {
entry.split:
  %i.linear.iv = alloca i32, align 4
  %myPoint.priv = alloca [4 x %struct.point2d], align 16
  %myPoint = alloca [4 x %struct.point2d], align 16
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %myPoint) #2
  %0 = getelementptr inbounds [4 x %struct.point2d], ptr %myPoint.priv, i64 0, i64 4
  br label %priv.constr.body

priv.constr.body:                                 ; preds = %priv.constr.body, %entry.split
  %priv.cpy.dest.ptr = phi ptr [ %myPoint.priv, %entry.split ], [ %priv.cpy.dest.inc, %priv.constr.body ]
  store i32 0, ptr %priv.cpy.dest.ptr, align 4
  %y.i.i = getelementptr inbounds %struct.point2d, ptr %priv.cpy.dest.ptr, i64 0, i32 1
  store i32 0, ptr %y.i.i, align 4
  %priv.cpy.dest.inc = getelementptr %struct.point2d, ptr %priv.cpy.dest.ptr, i64 1
  %priv.cpy.done = icmp eq ptr %priv.cpy.dest.inc, %0
  br i1 %priv.cpy.done, label %arrayctor.loop.preheader, label %priv.constr.body

arrayctor.loop.preheader:                         ; preds = %priv.constr.body
  %arrayctor.end = getelementptr inbounds [4 x %struct.point2d], ptr %myPoint, i64 0, i64 4
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %arrayctor.loop.preheader
  %arrayctor.cur = phi ptr [ %arrayctor.next, %arrayctor.loop ], [ %myPoint, %arrayctor.loop.preheader ]
  store i32 0, ptr %arrayctor.cur, align 4
  %y.i = getelementptr inbounds %struct.point2d, ptr %arrayctor.cur, i64 0, i32 1
  store i32 0, ptr %y.i, align 4
  %arrayctor.next = getelementptr inbounds %struct.point2d, ptr %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq ptr %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %DIR.OMP.SIMD.1, label %arrayctor.loop

DIR.OMP.SIMD.1:                                   ; preds = %arrayctor.loop
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD.TYPED"(ptr %myPoint.priv, %struct.point2d zeroinitializer, i64 4, ptr @_ZTS7point2d.omp.def_constr, ptr @_ZTS7point2d.omp.destr), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.119

DIR.OMP.SIMD.119:                                 ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.119
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.119 ], [ %indvars.iv.next, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i.linear.iv)
  %2 = trunc i64 %indvars.iv to i32
  %arrayidx = getelementptr inbounds [4 x %struct.point2d], ptr %myPoint.priv, i64 0, i64 %indvars.iv
  call void @_Z3bazP7point2di(ptr noundef nonnull %arrayidx, i32 noundef %2) #0
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.linear.iv)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.220

DIR.OMP.END.SIMD.220:                             ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.lifetime.end.p0(i64 32, ptr nonnull %myPoint)
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable
declare ptr @_ZTS7point2d.omp.def_constr(ptr noundef returned writeonly)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare void @_ZTS7point2d.omp.destr(ptr nocapture readnone)

declare void @_Z3bazP7point2di(ptr noundef, i32 noundef)

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

attributes #0 = { nounwind }
