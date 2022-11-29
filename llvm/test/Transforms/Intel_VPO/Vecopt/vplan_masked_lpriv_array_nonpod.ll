; RUN: opt -vplan-vec-scenario="n0;v2;m2" -vplan-enable-masked-variant -vplan-vec -print-after=vplan-vec -vplan-force-vf=2 -vplan-print-after-create-masked-vplan -disable-output %s 2>&1 | FileCheck %s --check-prefixes=CHECK,LLVM
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-print-after-create-masked-vplan -vplan-force-vf=2 -vplan-vec-scenario="n0;v2;m2" -disable-output %s 2>&1 | FileCheck %s --check-prefixes=CHECK,HIR

; Incomming HIR
; <24>   BEGIN REGION { }
; <26>         %2 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LASTPRIVATE:NONPOD.TYPED(&((%myPoint.lpriv)[0])zeroinitializer4&((@_ZTS7point2d.omp.def_constr)[0])&((@_ZTS7point2d.omp.copy_assign)[0])&((@_ZTS7point2d.omp.destr)[0])),  QUAL.OMP.NORMALIZED.IV:TYPED(null0),  QUAL.OMP.NORMALIZED.UB:TYPED(null0),  QUAL.OMP.LINEAR:IV.TYPED(&((%i.linear.iv)[0])011) ]
; <75>
; <75>         + DO i1 = 0, 999, 1   <DO_LOOP> <simd>
; <33>         |   @llvm.lifetime.start.p0i8(4,  &((i8*)(%i.linear.iv)[0]));
; <35>         |   %rem = i1  %u  3;
; <37>         |   if (%rem != 0)
; <37>         |   {
; <42>         |      @_Z3bazP7point2di(&((%myPoint.lpriv)[0][i1]),  i1);
; <37>         |   }
; <45>         |   @llvm.lifetime.end.p0i8(4,  &((i8*)(%i.linear.iv)[0]));
; <75>         + END LOOP
; <75>
; <53>         @llvm.directive.region.exit(%2); [ DIR.OMP.END.SIMD() ]
; <24>   END REGION

; CHECK: [DA: Div] [4 x %struct.point2d]* [[PRIV_ALLOCA:%.*]] = allocate-priv [4 x %struct.point2d]*
; CHECK: [DA: Uni] private-nonpod-array-ctor [4 x %struct.point2d]* [[PRIV_ALLOCA]]
; CHECK: [DA: Uni] private-last-value-nonpod-array-masked [4 x %struct.point2d]* [[PRIV_ALLOCA]] [4 x %struct.point2d]* %myPoint.lpriv i1 [[MASK:%.*]]
; CHECK: [DA: Uni] private-nonpod-array-dtor [4 x %struct.point2d]* [[PRIV_ALLOCA]]

; LLVM:      *** IR Dump After vpo::VPlanDriverPass on _Z3foov ***
; Ctor related loops
; LLVM:       array.nonpod.private.outer.loop24:
; LLVM-NEXT:    [[TMP37:%.*]] = phi i64 [ 0, [[VPLANNEDBB21:%.*]] ], [ [[TMP43:%.*]], [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP_INC26:%.*]] ]
; LLVM-NEXT:    [[PRIV_EXTRACT28:%.*]] = extractelement <2 x [4 x %struct.point2d]*> [[DOTINSERT_1:%.*]], i64 [[TMP37]]
; LLVM-NEXT:    br label [[ARRAY_NONPOD_PRIVATE_INNER_LOOP25:%.*]]
; LLVM:       array.nonpod.private.inner.loop25:
; LLVM-NEXT:    [[TMP38:%.*]] = phi i64 [ 0, [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP24:%.*]] ], [ [[TMP41:%.*]], [[ARRAY_NONPOD_PRIVATE_INNER_LOOP25]] ]
; LLVM-NEXT:    [[TMP39:%.*]] = getelementptr [4 x %struct.point2d], [4 x %struct.point2d]* [[PRIV_EXTRACT28]], i64 0, i64 [[TMP38]]
; LLVM-NEXT:    [[TMP40:%.*]] = call %struct.point2d* @_ZTS7point2d.omp.def_constr(%struct.point2d* [[TMP39]])
; LLVM-NEXT:    [[TMP41]] = add i64 [[TMP38]], 1
; LLVM-NEXT:    [[TMP42:%.*]] = icmp ult i64 [[TMP41]], 4
; LLVM-NEXT:    br i1 [[TMP42]], label [[ARRAY_NONPOD_PRIVATE_INNER_LOOP25]], label [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP_INC26]]
; LLVM:       array.nonpod.private.outer.loop.inc26:
; LLVM-NEXT:    [[TMP43]] = add i64 [[TMP37]], 1
; LLVM-NEXT:    [[TMP44:%.*]] = icmp ult i64 [[TMP37]], 2
; LLVM-NEXT:    br i1 [[TMP44]], label [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP24]], label [[ARRAY_NONPOD_PRIVATE_LOOP_EXIT27:%.*]]
; LLVM:       array.nonpod.private.loop.exit27:
; LLVM:         br label [[VPLANNEDBB29:%.*]]

; Copy-assign related loop
; LLVM:       VPlannedBB47:
; LLVM-NEXT:    [[TMP71:%.*]] = bitcast <2 x i1> [[TMP46:%.*]] to i2
; LLVM-NEXT:    [[CTLZ:%.*]] = call i2 @llvm.ctlz.i2(i2 [[TMP71]], i1 true)
; LLVM-NEXT:    [[TMP72:%.*]] = sub i2 1, [[CTLZ]]
; LLVM-NEXT:    [[PRIV_EXTRACT48:%.*]] = extractelement <2 x [4 x %struct.point2d]*> [[DOTINSERT_1]], i2 [[TMP72]]
; LLVM-NEXT:    br label [[ARRAY_NONPOD_LAST_PRIVATE_LOOP49:%.*]]
; LLVM:       array.nonpod.last.private.loop49:
; LLVM-NEXT:    [[TMP73:%.*]] = phi i64 [ 0, [[VPLANNEDBB47:%.*]] ], [ [[TMP76:%.*]], [[ARRAY_NONPOD_LAST_PRIVATE_LOOP49]] ]
; LLVM-NEXT:    [[TMP74:%.*]] = getelementptr [4 x %struct.point2d], [4 x %struct.point2d]* [[PRIV_EXTRACT48]], i64 0, i64 [[TMP73]]
; LLVM-NEXT:    [[TMP75:%.*]] = getelementptr [4 x %struct.point2d], [4 x %struct.point2d]* [[MYPOINT_LPRIV:%.*]], i64 0, i64 [[TMP73]]
; LLVM-NEXT:    call void @_ZTS7point2d.omp.copy_assign(%struct.point2d* [[TMP75]], %struct.point2d* [[TMP74]])
; LLVM-NEXT:    [[TMP76]] = add i64 [[TMP73]], 1
; LLVM-NEXT:    [[TMP77:%.*]] = icmp ult i64 [[TMP76]], 4
; LLVM-NEXT:    br i1 [[TMP77]], label [[ARRAY_NONPOD_LAST_PRIVATE_LOOP49]], label [[ARRAY_NONPOD_LAST_PRIVATE_LOOP_EXIT50:%.*]]
; LLVM:       array.nonpod.last.private.loop.exit50:
; LLVM-NEXT:    br label [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP51:%.*]]

; Dtor related loops
; LLVM:       array.nonpod.private.outer.loop51:
; LLVM-NEXT:    [[TMP78:%.*]] = phi i64 [ 0, [[ARRAY_NONPOD_LAST_PRIVATE_LOOP_EXIT50]] ], [ [[TMP83:%.*]], [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP_INC53:%.*]] ]
; LLVM-NEXT:    [[PRIV_EXTRACT55:%.*]] = extractelement <2 x [4 x %struct.point2d]*> [[DOTINSERT_1]], i64 [[TMP78]]
; LLVM-NEXT:    br label [[ARRAY_NONPOD_PRIVATE_INNER_LOOP52:%.*]]
; LLVM:       array.nonpod.private.inner.loop52:
; LLVM-NEXT:    [[TMP79:%.*]] = phi i64 [ 0, [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP51]] ], [ [[TMP81:%.*]], [[ARRAY_NONPOD_PRIVATE_INNER_LOOP52]] ]
; LLVM-NEXT:    [[TMP80:%.*]] = getelementptr [4 x %struct.point2d], [4 x %struct.point2d]* [[PRIV_EXTRACT55]], i64 0, i64 [[TMP79]]
; LLVM-NEXT:    call void @_ZTS7point2d.omp.destr(%struct.point2d* [[TMP80]])
; LLVM-NEXT:    [[TMP81]] = add i64 [[TMP79]], 1
; LLVM-NEXT:    [[TMP82:%.*]] = icmp ult i64 [[TMP81]], 4
; LLVM-NEXT:    br i1 [[TMP82]], label [[ARRAY_NONPOD_PRIVATE_INNER_LOOP52]], label [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP_INC53]]
; LLVM:       array.nonpod.private.outer.loop.inc53:
; LLVM-NEXT:    [[TMP83]] = add i64 [[TMP78]], 1
; LLVM-NEXT:    [[TMP84:%.*]] = icmp ult i64 [[TMP78]], 2
; LLVM-NEXT:    br i1 [[TMP84]], label [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP51]], label [[ARRAY_NONPOD_PRIVATE_LOOP_EXIT54:%.*]]
; LLVM:       array.nonpod.private.loop.exit54:
; LLVM-NEXT:    br label [[VPLANNEDBB46:%.*]]

; HIR:  + DO i1 = 0, 1, 1   <DO_LOOP>
; HIR:  |   %priv.extract45 = extractelement &((<2 x [4 x %struct.point2d]*>)(%priv.mem.bc44)[<i32 0, i32 1>]),  i1;
; HIR:  |
; HIR:  |   + DO i2 = 0, 3, 1   <DO_LOOP>
; HIR:  |   |   %call46 = @_ZTS7point2d.omp.def_constr(&((%struct.point2d*)(%priv.extract45)[i2]));
; HIR:  |   + END LOOP
; HIR:  + END LOOP

; HIR:  %bsfintmask = bitcast.<2 x i1>.i2(%.vec47);
; HIR:  %bsf = @llvm.ctlz.i2(%bsfintmask,  1);
; HIR:  %ext.lane = 1  -  %bsf;
; HIR:  %priv.extract64 = extractelement &((<2 x [4 x %struct.point2d]*>)(%priv.mem.bc44)[<i32 0, i32 1>]),  %ext.lane;

; HIR:  + DO i1 = 0, 3, 1   <DO_LOOP>
; HIR:  |   @_ZTS7point2d.omp.copy_assign(&((%struct.point2d*)(%myPoint.lpriv)[i1]),  &((%struct.point2d*)(%priv.extract64)[i1]));
; HIR:  + END LOOP

; HIR:  + DO i1 = 0, 1, 1   <DO_LOOP>
; HIR:  |   %priv.extract65 = extractelement &((<2 x [4 x %struct.point2d]*>)(%priv.mem.bc44)[<i32 0, i32 1>]),  i1;
; HIR:  |
; HIR:  |   + DO i2 = 0, 3, 1   <DO_LOOP>
; HIR:  |   |   @_ZTS7point2d.omp.destr(&((%struct.point2d*)(%priv.extract65)[i2]));
; HIR:  |   + END LOOP
; HIR:  + END LOOP

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.point2d = type { i32, i32 }

; Function Attrs: nounwind uwtable
define dso_local void @_Z3foov() {
entry.split:
  %myPoint.lpriv = alloca [4 x %struct.point2d], align 16
  %i.linear.iv = alloca i32, align 4
  %myPoint = alloca [4 x %struct.point2d], align 16
  %0 = bitcast [4 x %struct.point2d]* %myPoint to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* nonnull %0)
  %array.begin = getelementptr inbounds [4 x %struct.point2d], [4 x %struct.point2d]* %myPoint, i64 0, i64 0
  %array.begin11 = getelementptr inbounds [4 x %struct.point2d], [4 x %struct.point2d]* %myPoint.lpriv, i64 0, i64 0
  %1 = getelementptr inbounds [4 x %struct.point2d], [4 x %struct.point2d]* %myPoint.lpriv, i64 0, i64 4
  br label %priv.constr.body

priv.constr.body:                                 ; preds = %priv.constr.body, %entry.split
  %priv.cpy.dest.ptr = phi %struct.point2d* [ %array.begin11, %entry.split ], [ %priv.cpy.dest.inc, %priv.constr.body ]
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
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:NONPOD.TYPED"([4 x %struct.point2d]* %myPoint.lpriv, %struct.point2d zeroinitializer, i64 4, %struct.point2d* (%struct.point2d*)* @_ZTS7point2d.omp.def_constr, void (%struct.point2d*, %struct.point2d*)* @_ZTS7point2d.omp.copy_assign, void (%struct.point2d*)* @_ZTS7point2d.omp.destr), "QUAL.OMP.NORMALIZED.IV:TYPED"(i8* null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(i8* null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.124

DIR.OMP.SIMD.124:                                 ; preds = %DIR.OMP.SIMD.1
  %3 = bitcast i32* %i.linear.iv to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.body.continue, %DIR.OMP.SIMD.124
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.124 ], [ %indvars.iv.next, %omp.body.continue ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3)
  %4 = trunc i64 %indvars.iv to i32
  %rem = urem i32 %4, 3
  %tobool.not = icmp eq i32 %rem, 0
  br i1 %tobool.not, label %omp.body.continue, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx = getelementptr inbounds [4 x %struct.point2d], [4 x %struct.point2d]* %myPoint.lpriv, i64 0, i64 %indvars.iv
  call void @_Z3bazP7point2di(%struct.point2d* noundef nonnull %arrayidx, i32 noundef %4) #0
  br label %omp.body.continue

omp.body.continue:                                ; preds = %if.then, %omp.inner.for.body
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %omp.inner.for.body

DIR.OMP.END.SIMD.1:                               ; preds = %omp.body.continue
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %priv.cpyassn.body

priv.cpyassn.body:                                ; preds = %priv.cpyassn.body, %DIR.OMP.END.SIMD.1
  %priv.cpy.dest.ptr14 = phi %struct.point2d* [ %array.begin, %DIR.OMP.END.SIMD.1 ], [ %priv.cpy.dest.inc15, %priv.cpyassn.body ]
  %priv.cpy.src.ptr = phi %struct.point2d* [ %array.begin11, %DIR.OMP.END.SIMD.1 ], [ %priv.cpy.src.inc, %priv.cpyassn.body ]
  %5 = getelementptr inbounds %struct.point2d, %struct.point2d* %priv.cpy.src.ptr, i64 0, i32 0
  %6 = getelementptr inbounds %struct.point2d, %struct.point2d* %priv.cpy.dest.ptr14, i64 0, i32 0
  %7 = load i32, i32* %5, align 4
  store i32 %7, i32* %6, align 4
  %8 = getelementptr inbounds %struct.point2d, %struct.point2d* %priv.cpy.src.ptr, i64 0, i32 1
  %9 = getelementptr inbounds %struct.point2d, %struct.point2d* %priv.cpy.dest.ptr14, i64 0, i32 1
  %10 = load i32, i32* %8, align 4
  store i32 %10, i32* %9, align 4
  %priv.cpy.dest.inc15 = getelementptr %struct.point2d, %struct.point2d* %priv.cpy.dest.ptr14, i64 1
  %priv.cpy.src.inc = getelementptr %struct.point2d, %struct.point2d* %priv.cpy.src.ptr, i64 1
  %priv.cpy.done16 = icmp eq %struct.point2d* %priv.cpy.dest.inc15, %arrayctor.end
  br i1 %priv.cpy.done16, label %priv.destr.body.preheader, label %priv.cpyassn.body

priv.destr.body.preheader:                        ; preds = %priv.cpyassn.body
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
declare hidden noundef %struct.point2d* @_ZTS7point2d.omp.def_constr(%struct.point2d* noundef returned writeonly)

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn uwtable
declare hidden void @_ZTS7point2d.omp.copy_assign(%struct.point2d* nocapture noundef writeonly, %struct.point2d* nocapture noundef readonly)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare hidden void @_ZTS7point2d.omp.destr(%struct.point2d* nocapture readnone)

declare dso_local void @_Z3bazP7point2di(%struct.point2d* noundef, i32 noundef)

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)

attributes #0 = { nounwind }
