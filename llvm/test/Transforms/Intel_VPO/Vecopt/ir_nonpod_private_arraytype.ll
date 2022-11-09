; RUN: opt -vplan-vec -print-after=vplan-vec -vplan-force-vf=2 -vplan-print-after-vpentity-instrs -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR

; CHECK: VPlan after insertion of VPEntities instructions:
; CHECK:   [4 x %struct.point2d]* [[ALLPRIV:%.*]] = allocate-priv [4 x %struct.point2d]*, OrigAlign = 16
; CHECK:   private-nonpod-array-ctor [4 x %struct.point2d]* [[ALLPRIV]]
; CHECK:   %struct.point2d* [[TMP3:%.*]] = getelementptr inbounds [4 x %struct.point2d]* [[ALLPRIV]] i64 0 i64 [[TMP2:%.*]]
; CHECK:   call %struct.point2d* [[TMP3]] i32 [[IV:%.*]] void (%struct.point2d*, i32)* @_Z3bazP7point2di
; CHECK:   private-nonpod-array-dtor [4 x %struct.point2d]* [[ALLPRIV]]

; CHECK: *** IR Dump After vpo::VPlanDriverPass on _Z3foov ***
; Ctor related loops
; CHECK:      array.nonpod.private.outer.loop:                  ; preds = %array.nonpod.private.outer.loop.inc, %VPlannedBB1
; CHECK-NEXT:   %4 = phi i64 [ 0, %VPlannedBB1 ], [ %10, %array.nonpod.private.outer.loop.inc ]
; CHECK-NEXT:   %priv.extract = extractelement <2 x [4 x %struct.point2d]*> %myPoint.priv.insert.1, i64 %4
; CHECK-NEXT:   br label %array.nonpod.private.inner.loop
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.inner.loop:                  ; preds = %array.nonpod.private.inner.loop, %array.nonpod.private.outer.loop
; CHECK-NEXT:   %5 = phi i64 [ 0, %array.nonpod.private.outer.loop ], [ %8, %array.nonpod.private.inner.loop ]
; CHECK-NEXT:   %6 = getelementptr [4 x %struct.point2d], [4 x %struct.point2d]* %priv.extract, i64 0, i64 %5
; CHECK-NEXT:   %7 = call %struct.point2d* @_ZTS7point2d.omp.def_constr(%struct.point2d* %6)
; CHECK-NEXT:   %8 = add i64 %5, 1
; CHECK-NEXT:   %9 = icmp ult i64 %8, 4
; CHECK-NEXT:   br i1 %9, label %array.nonpod.private.inner.loop, label %array.nonpod.private.outer.loop.inc
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.outer.loop.inc:              ; preds = %array.nonpod.private.inner.loop
; CHECK-NEXT:   %10 = add i64 %4, 1
; CHECK-NEXT:   %11 = icmp ult i64 %4, 2
; CHECK-NEXT:   br i1 %11, label %array.nonpod.private.outer.loop, label %array.nonpod.private.loop.exit
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.loop.exit:                   ; preds = %array.nonpod.private.outer.loop.inc
; CHECK:        br label %vector.body

; Dtor related loops
; CHECK:      array.nonpod.private.outer.loop6:                 ; preds = %array.nonpod.private.outer.loop.inc8, %VPlannedBB5
; CHECK-NEXT:   %16 = phi i64 [ 0, %VPlannedBB5 ], [ %21, %array.nonpod.private.outer.loop.inc8 ]
; CHECK-NEXT:   %priv.extract10 = extractelement <2 x [4 x %struct.point2d]*> %myPoint.priv.insert.1, i64 %16
; CHECK-NEXT:   br label %array.nonpod.private.inner.loop7
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.inner.loop7:                 ; preds = %array.nonpod.private.inner.loop7, %array.nonpod.private.outer.loop6
; CHECK-NEXT:   %17 = phi i64 [ 0, %array.nonpod.private.outer.loop6 ], [ %19, %array.nonpod.private.inner.loop7 ]
; CHECK-NEXT:   %18 = getelementptr [4 x %struct.point2d], [4 x %struct.point2d]* %priv.extract10, i64 0, i64 %17
; CHECK-NEXT:   call void @_ZTS7point2d.omp.destr(%struct.point2d* %18)
; CHECK-NEXT:   %19 = add i64 %17, 1
; CHECK-NEXT:   %20 = icmp ult i64 %19, 4
; CHECK-NEXT:   br i1 %20, label %array.nonpod.private.inner.loop7, label %array.nonpod.private.outer.loop.inc8
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.outer.loop.inc8:            ; preds = %array.nonpod.private.inner.loop7
; CHECK-NEXT:   %21 = add i64 %16, 1
; CHECK-NEXT:   %22 = icmp ult i64 %16, 2
; CHECK-NEXT:   br i1 %22, label %array.nonpod.private.outer.loop6, label %array.nonpod.private.loop.exit9
; CHECK-EMPTY:
; CHECK-NEXT: array.nonpod.private.loop.exit9:                 ; preds = %array.nonpod.private.outer.loop.inc8
; CHECK:        br label %VPlannedBB13

; Incomming HIR
; <24>         BEGIN REGION { }
; <26>               %2 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.PRIVATE:NONPOD.TYPED(&((%myPoint.priv)[0])zeroinitializer4&((@_ZTS7point2d.omp.def_constr)[0])&((@_ZTS7point2d.omp.destr)[0])),  QUAL.OMP.NORMALIZED.IV:TYPED(null0),  QUAL.OMP.NORMALIZED.UB:TYPED(null0),  QUAL.OMP.LINEAR:IV.TYPED(&((%i.linear.iv)[0])011) ]
; <49>
; <49>               + DO i1 = 0, 999, 1   <DO_LOOP> <simd>
; <33>               |   @llvm.lifetime.start.p0i8(4,  &((i8*)(%i.linear.iv)[0]));
; <36>               |   @_Z3bazP7point2di(&((%myPoint.priv)[0][i1]),  i1);
; <37>               |   @llvm.lifetime.end.p0i8(4,  &((i8*)(%i.linear.iv)[0]));
; <49>               + END LOOP
; <49>
; <45>               @llvm.directive.region.exit(%2); [ DIR.OMP.END.SIMD() ]
; <24>         END REGION

; *** IR Dump After vpo::VPlanDriverHIRPass ***
; HIR:         	    BEGIN REGION { modified }
; HIR-NEXT:               %priv.mem.bc = &(([4 x %struct.point2d]*)(%priv.mem)[0]);
; HIR:                    + DO i1 = 0, 2, 1   <DO_LOOP>
; HIR-NEXT:               |   %priv.extract = extractelement &((<2 x [4 x %struct.point2d]*>)(%priv.mem.bc)[<i32 0, i32 1>]),  i1;
; HIR-NEXT:               |
; HIR-NEXT:               |   + DO i2 = 0, 4, 1   <DO_LOOP>
; HIR-NEXT:               |   |   %call = @_ZTS7point2d.omp.def_constr(&((%struct.point2d*)(%priv.extract)[i2]));
; HIR-NEXT:               |   + END LOOP
; HIR-NEXT:               + END LOOP
; HIR:                    + DO i1 = 0, 999, 2   <DO_LOOP> <simd-vectorized> <novectorize>
; HIR:                    |   @_Z3bazP7point2di(%extract.0.,  i1);
; HIR:                    |   @_Z3bazP7point2di(%extract.1.,  %extract.1.2);
; HIR:                    + END LOOP
; HIR:                    + DO i1 = 0, 2, 1   <DO_LOOP>
; HIR-NEXT:               |   %priv.extract4 = extractelement &((<2 x [4 x %struct.point2d]*>)(%priv.mem.bc)[<i32 0, i32 1>]),  i1;
; HIR-NEXT:               |
; HIR-NEXT:               |   + DO i2 = 0, 4, 1   <DO_LOOP>
; HIR-NEXT:               |   |   @_ZTS7point2d.omp.destr(&((%struct.point2d*)(%priv.extract4)[i2]));
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
  %myPoint.priv.gep = getelementptr inbounds [4 x %struct.point2d], [4 x %struct.point2d]* %myPoint.priv, i64 0, i64 0
  %myPoint = alloca [4 x %struct.point2d], align 16
  %0 = bitcast [4 x %struct.point2d]* %myPoint to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %0) #2
  %array.begin = getelementptr inbounds [4 x %struct.point2d], [4 x %struct.point2d]* %myPoint, i64 0, i64 0
  %1 = getelementptr inbounds [4 x %struct.point2d], [4 x %struct.point2d]* %myPoint.priv, i64 0, i64 4
  br label %priv.constr.body

priv.constr.body:                                 ; preds = %priv.constr.body, %entry.split
  %priv.cpy.dest.ptr = phi %struct.point2d* [ %myPoint.priv.gep, %entry.split ], [ %priv.cpy.dest.inc, %priv.constr.body ]
  %x.i.i = getelementptr inbounds %struct.point2d, %struct.point2d* %priv.cpy.dest.ptr, i64 0, i32 0
  store i32 0, i32* %x.i.i, align 4
  %y.i.i = getelementptr inbounds %struct.point2d, %struct.point2d* %priv.cpy.dest.ptr, i64 0, i32 1
  store i32 0, i32* %y.i.i, align 4
  %priv.cpy.dest.inc = getelementptr %struct.point2d, %struct.point2d* %priv.cpy.dest.ptr, i64 1
  %priv.cpy.done = icmp eq %struct.point2d* %priv.cpy.dest.inc, %1
  br i1 %priv.cpy.done, label %arrayctor.loop.preheader, label %priv.constr.body

arrayctor.loop.preheader:                         ; preds = %priv.constr.body
  %arrayctor.end = getelementptr inbounds [4 x %struct.point2d], [4 x %struct.point2d]* %myPoint, i64 0, i64 4
  br label %arrayctor.loop

arrayctor.loop:                                   ; preds = %arrayctor.loop, %arrayctor.loop.preheader
  %arrayctor.cur = phi %struct.point2d* [ %arrayctor.next, %arrayctor.loop ], [ %array.begin, %arrayctor.loop.preheader ]
  %x.i = getelementptr inbounds %struct.point2d, %struct.point2d* %arrayctor.cur, i64 0, i32 0
  store i32 0, i32* %x.i, align 4
  %y.i = getelementptr inbounds %struct.point2d, %struct.point2d* %arrayctor.cur, i64 0, i32 1
  store i32 0, i32* %y.i, align 4
  %arrayctor.next = getelementptr inbounds %struct.point2d, %struct.point2d* %arrayctor.cur, i64 1
  %arrayctor.done = icmp eq %struct.point2d* %arrayctor.next, %arrayctor.end
  br i1 %arrayctor.done, label %DIR.OMP.SIMD.1, label %arrayctor.loop

DIR.OMP.SIMD.1:                                   ; preds = %arrayctor.loop
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD.TYPED"([4 x %struct.point2d]* %myPoint.priv, %struct.point2d zeroinitializer, i64 4, %struct.point2d* (%struct.point2d*)* @_ZTS7point2d.omp.def_constr, void (%struct.point2d*)* @_ZTS7point2d.omp.destr), "QUAL.OMP.NORMALIZED.IV:TYPED"(i8* null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(i8* null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.119

DIR.OMP.SIMD.119:                                 ; preds = %DIR.OMP.SIMD.1
  %3 = bitcast i32* %i.linear.iv to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.inner.for.body, %DIR.OMP.SIMD.119
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.119 ], [ %indvars.iv.next, %omp.inner.for.body ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3)
  %4 = trunc i64 %indvars.iv to i32
  %arrayidx = getelementptr inbounds [4 x %struct.point2d], [4 x %struct.point2d]* %myPoint.priv, i64 0, i64 %indvars.iv
  call void @_Z3bazP7point2di(%struct.point2d* noundef nonnull %arrayidx, i32 noundef %4) #0
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.220

DIR.OMP.END.SIMD.220:                             ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.lifetime.end.p0i8(i64 32, i8* nonnull %0)
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn writeonly uwtable
declare %struct.point2d* @_ZTS7point2d.omp.def_constr(%struct.point2d* noundef returned writeonly)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare void @_ZTS7point2d.omp.destr(%struct.point2d* nocapture readnone)

declare void @_Z3bazP7point2di(%struct.point2d* noundef, i32 noundef)

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

attributes #0 = { nounwind }
