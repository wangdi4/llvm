; RUN: opt -passes="vplan-vec" -vplan-vec-scenario="n0;v2;m2" -vplan-enable-masked-variant -print-after=vplan-vec -vplan-force-vf=2 -disable-output %s 2>&1 | FileCheck %s --check-prefixes=LLVM
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>" -vplan-force-vf=2 -vplan-vec-scenario="n0;v2;m2" -disable-output %s 2>&1 | FileCheck %s --check-prefixes=HIR

; Incomming HIR
; <24>   BEGIN REGION { }
; <26>         %2 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.LASTPRIVATE:NONPOD.TYPED(&((%myPoint.lpriv)[0])zeroinitializer4&((@_ZTS7point2d.omp.def_constr)[0])&((@_ZTS7point2d.omp.copy_assign)[0])&((@_ZTS7point2d.omp.destr)[0])),  QUAL.OMP.NORMALIZED.IV:TYPED(null0),  QUAL.OMP.NORMALIZED.UB:TYPED(null0),  QUAL.OMP.LINEAR:IV.TYPED(&((%i.linear.iv)[0])011) ]
; <75>
; <75>         + DO i1 = 0, 999, 1   <DO_LOOP> <simd>
; <33>         |   @llvm.lifetime.start.p0(4,  &((i8*)(%i.linear.iv)[0]));
; <35>         |   %rem = i1  %u  3;
; <37>         |   if (%rem != 0)
; <37>         |   {
; <42>         |      @_Z3bazP7point2di(&((%myPoint.lpriv)[0][i1]),  i1);
; <37>         |   }
; <45>         |   @llvm.lifetime.end.p0(4,  &((i8*)(%i.linear.iv)[0]));
; <75>         + END LOOP
; <75>
; <53>         @llvm.directive.region.exit(%2); [ DIR.OMP.END.SIMD() ]
; <24>   END REGION

; LLVM:  define dso_local void @_Z3foov() {
; Ctor related loops
; LLVM:       array.nonpod.private.outer.loop[[OUTER:[0-9]*]]:
; LLVM-NEXT:    [[TMP37:%.*]] = phi i64 [ 0, [[VPLANNEDBB210:%.*]] ], [ [[TMP43:%.*]], [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP_INC260:%.*]] ]
; LLVM-NEXT:    [[PRIV_EXTRACT280:%.*]] = extractelement <2 x ptr> [[DOTINSERT_10:%.*]], i64 [[TMP37]]
; LLVM-NEXT:    br label [[ARRAY_NONPOD_PRIVATE_INNER_LOOP250:%.*]]
; LLVM:       array.nonpod.private.inner.loop[[INNER:[0-9]*]]:
; LLVM-NEXT:    [[TMP38:%.*]] = phi i64 [ 0, %array.nonpod.private.outer.loop[[OUTER]] ], [ [[TMP41:%.*]], %array.nonpod.private.inner.loop[[INNER]] ]
; LLVM-NEXT:    [[TMP39:%.*]] = getelementptr [4 x %struct.point2d], ptr [[PRIV_EXTRACT280]], i64 0, i64 [[TMP38]]
; LLVM-NEXT:    [[TMP40:%.*]] = call ptr @_ZTS7point2d.omp.def_constr(ptr [[TMP39]])
; LLVM-NEXT:    [[TMP41]] = add i64 [[TMP38]], 1
; LLVM-NEXT:    [[TMP42:%.*]] = icmp ult i64 [[TMP41]], 4
; LLVM-NEXT:    br i1 [[TMP42]], label %array.nonpod.private.inner.loop[[INNER]], label %array.nonpod.private.outer.loop.inc[[LOOPINC:[0-9]*]]
; LLVM:       array.nonpod.private.outer.loop.inc[[LOOPINC]]:
; LLVM-NEXT:    [[TMP43]] = add i64 [[TMP37]], 1
; LLVM-NEXT:    [[TMP44:%.*]] = icmp ult i64 [[TMP37]], 2
; LLVM-NEXT:    br i1 [[TMP44]], label %array.nonpod.private.outer.loop[[OUTER]], label %array.nonpod.private.loop.exit[[LOOPEXIT:[0-9]*]]
; LLVM:       array.nonpod.private.loop.exit[[LOOPEXIT]]:
; LLVM:         [[TMP45:%.*]] = sub i64 1000, [[UNI_PHI190:%.*]]
; LLVM:         br label [[VPLANNEDBB290:%.*]]

; Copy-assign related loop
; LLVM:       VPlannedBB41:
; LLVM:         [[TMP70:%.*]] = bitcast <2 x i1> [[TMP47:%.*]] to i2
; LLVM:         [[TMP71:%.*]] = icmp eq i2 [[TMP70]], 0
; LLVM:         br i1 [[TMP71]], label [[VPLANNEDBB480:%.*]], label %[[VPLANNEDBB490:.*]]
; LLVM:       [[VPLANNEDBB490]]:
; LLVM-NEXT:    [[TMP72:%.*]] = bitcast <2 x i1> [[TMP47]] to i2
; LLVM-NEXT:    [[CTLZ0:%.*]] = call i2 @llvm.ctlz.i2(i2 [[TMP72]], i1 true)
; LLVM-NEXT:    [[TMP73:%.*]] = sub i2 1, [[CTLZ0]]
; LLVM-NEXT:    [[PRIV_EXTRACT500:%.*]] = extractelement <2 x ptr> %.insert.1, i2 [[TMP73]]
; LLVM-NEXT:    br label %[[ARRAY_NONPOD_LAST_PRIVATE_LOOP510:.*]]
; LLVM:       [[ARRAY_NONPOD_LAST_PRIVATE_LOOP510]]:
; LLVM-NEXT:    [[TMP74:%.*]] = phi i64 [ 0, %[[VPLANNEDBB490]] ], [ [[TMP77:%.*]], %[[ARRAY_NONPOD_LAST_PRIVATE_LOOP510]] ]
; LLVM-NEXT:    [[TMP75:%.*]] = getelementptr [4 x %struct.point2d], ptr [[PRIV_EXTRACT500]], i64 0, i64 [[TMP74]]
; LLVM-NEXT:    [[TMP76:%.*]] = getelementptr [4 x %struct.point2d], ptr [[MYPOINT_LPRIV0:%.*]], i64 0, i64 [[TMP74]]
; LLVM-NEXT:    call void @_ZTS7point2d.omp.copy_assign(ptr [[TMP76]], ptr [[TMP75]])
; LLVM-NEXT:    [[TMP77]] = add i64 [[TMP74]], 1
; LLVM-NEXT:    [[TMP78:%.*]] = icmp ult i64 [[TMP77]], 4
; LLVM-NEXT:    br i1 [[TMP78]], label %[[ARRAY_NONPOD_LAST_PRIVATE_LOOP510]], label %[[ARRAY_NONPOD_LAST_PRIVATE_LOOP_EXIT520:.*]]
; LLVM:       [[ARRAY_NONPOD_LAST_PRIVATE_LOOP_EXIT520]]:
; LLVM-NEXT:    br label %[[ARRAY_NONPOD_PRIVATE_OUTER_LOOP530:.*]]

; Dtor related loops
; LLVM:       [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP530]]:
; LLVM-NEXT:    [[TMP79:%.*]] = phi i64 [ 0, %[[ARRAY_NONPOD_LAST_PRIVATE_LOOP_EXIT520]] ], [ [[TMP84:%.*]], %[[ARRAY_NONPOD_PRIVATE_OUTER_LOOP_INC550:.*]] ]
; LLVM-NEXT:    [[PRIV_EXTRACT570:%.*]] = extractelement <2 x ptr> %.insert.1, i64 [[TMP79]]
; LLVM-NEXT:    br label %[[ARRAY_NONPOD_PRIVATE_INNER_LOOP540:.*]]
; LLVM:       [[ARRAY_NONPOD_PRIVATE_INNER_LOOP540]]:
; LLVM-NEXT:    [[TMP80:%.*]] = phi i64 [ 0, %[[ARRAY_NONPOD_PRIVATE_OUTER_LOOP530]] ], [ [[TMP82:%.*]], %[[ARRAY_NONPOD_PRIVATE_INNER_LOOP540]] ]
; LLVM-NEXT:    [[TMP81:%.*]] = getelementptr [4 x %struct.point2d], ptr [[PRIV_EXTRACT570]], i64 0, i64 [[TMP80]]
; LLVM-NEXT:    call void @_ZTS7point2d.omp.destr(ptr [[TMP81]])
; LLVM-NEXT:    [[TMP82]] = add i64 [[TMP80]], 1
; LLVM-NEXT:    [[TMP83:%.*]] = icmp ult i64 [[TMP82]], 4
; LLVM-NEXT:    br i1 [[TMP83]], label %[[ARRAY_NONPOD_PRIVATE_INNER_LOOP540]], label %[[ARRAY_NONPOD_PRIVATE_OUTER_LOOP_INC550]]
; LLVM:       [[ARRAY_NONPOD_PRIVATE_OUTER_LOOP_INC550]]:
; LLVM-NEXT:    [[TMP84]] = add i64 [[TMP79]], 1
; LLVM-NEXT:    [[TMP85:%.*]] = icmp ult i64 [[TMP79]], 2
; LLVM-NEXT:    br i1 [[TMP85]], label %[[ARRAY_NONPOD_PRIVATE_OUTER_LOOP530]], label %[[ARRAY_NONPOD_PRIVATE_LOOP_EXIT560:.*]]
; LLVM:       [[ARRAY_NONPOD_PRIVATE_LOOP_EXIT560]]:
;
; HIR:  + DO i1 = 0, 1, 1   <DO_LOOP>
; HIR:  |   %priv.extract[[EXTR:[0-9]*]] = extractelement &((<2 x ptr>)(%priv.mem.bc)[<i32 0, i32 1>]),  i1;
; HIR:  |
; HIR:  |   + DO i2 = 0, 3, 1   <DO_LOOP>
; HIR:  |   |   %call{{[0-9]*}} = @_ZTS7point2d.omp.def_constr(&((%struct.point2d*)(%priv.extract[[EXTR]])[i2]));
; HIR:  |   + END LOOP
; HIR:  + END LOOP

; HIR:  %bsfintmask = bitcast.<2 x i1>.i2(%.vec{{[0-9]*}});
; HIR:  %bsf = @llvm.ctlz.i2(%bsfintmask,  1);
; HIR:  %ext.lane = 1  -  %bsf;
; HIR:  %priv.extract[[EXTR2:[0-9]*]] = extractelement &((<2 x ptr>)(%priv.mem.bc{{[0-9]*}})[<i32 0, i32 1>]),  %ext.lane;

; HIR:  + DO i1 = 0, 3, 1   <DO_LOOP>
; HIR:  |   @_ZTS7point2d.omp.copy_assign(&((%struct.point2d*)(%myPoint.lpriv)[i1]),  &((%struct.point2d*)(%priv.extract[[EXTR2]])[i1]));
; HIR:  + END LOOP

; HIR:  + DO i1 = 0, 1, 1   <DO_LOOP>
; HIR:  |   %priv.extract[[EXTR3:[0-9]*]] = extractelement &((<2 x ptr>)(%priv.mem.bc{{[0-9]*}})[<i32 0, i32 1>]),  i1;
; HIR:  |
; HIR:  |   + DO i2 = 0, 3, 1   <DO_LOOP>
; HIR:  |   |   @_ZTS7point2d.omp.destr(&((%struct.point2d*)(%priv.extract[[EXTR3]])[i2]));
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
  call void @llvm.lifetime.start.p0(i64 32, ptr nonnull %myPoint)
  %0 = getelementptr inbounds [4 x %struct.point2d], ptr %myPoint.lpriv, i64 0, i64 4
  br label %priv.constr.body

priv.constr.body:                                 ; preds = %priv.constr.body, %entry.split
  %priv.cpy.dest.ptr = phi ptr [ %myPoint.lpriv, %entry.split ], [ %priv.cpy.dest.inc, %priv.constr.body ]
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
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.LASTPRIVATE:NONPOD.TYPED"(ptr %myPoint.lpriv, %struct.point2d zeroinitializer, i64 4, ptr @_ZTS7point2d.omp.def_constr, ptr @_ZTS7point2d.omp.copy_assign, ptr @_ZTS7point2d.omp.destr), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %DIR.OMP.SIMD.124

DIR.OMP.SIMD.124:                                 ; preds = %DIR.OMP.SIMD.1
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %omp.body.continue, %DIR.OMP.SIMD.124
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.124 ], [ %indvars.iv.next, %omp.body.continue ]
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %i.linear.iv)
  %2 = trunc i64 %indvars.iv to i32
  %rem = urem i32 %2, 3
  %tobool.not = icmp eq i32 %rem, 0
  br i1 %tobool.not, label %omp.body.continue, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx = getelementptr inbounds [4 x %struct.point2d], ptr %myPoint.lpriv, i64 0, i64 %indvars.iv
  call void @_Z3bazP7point2di(ptr noundef nonnull %arrayidx, i32 noundef %2) #0
  br label %omp.body.continue

omp.body.continue:                                ; preds = %if.then, %omp.inner.for.body
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i.linear.iv)
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %omp.inner.for.body

DIR.OMP.END.SIMD.1:                               ; preds = %omp.body.continue
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.SIMD"() ]
  br label %priv.cpyassn.body

priv.cpyassn.body:                                ; preds = %priv.cpyassn.body, %DIR.OMP.END.SIMD.1
  %priv.cpy.dest.ptr14 = phi ptr [ %myPoint, %DIR.OMP.END.SIMD.1 ], [ %priv.cpy.dest.inc15, %priv.cpyassn.body ]
  %priv.cpy.src.ptr = phi ptr [ %myPoint.lpriv, %DIR.OMP.END.SIMD.1 ], [ %priv.cpy.src.inc, %priv.cpyassn.body ]
  %3 = load i32, ptr %priv.cpy.src.ptr, align 4
  store i32 %3, ptr %priv.cpy.dest.ptr14, align 4
  %4 = getelementptr inbounds %struct.point2d, ptr %priv.cpy.src.ptr, i64 0, i32 1
  %5 = getelementptr inbounds %struct.point2d, ptr %priv.cpy.dest.ptr14, i64 0, i32 1
  %6 = load i32, ptr %4, align 4
  store i32 %6, ptr %5, align 4
  %priv.cpy.dest.inc15 = getelementptr %struct.point2d, ptr %priv.cpy.dest.ptr14, i64 1
  %priv.cpy.src.inc = getelementptr %struct.point2d, ptr %priv.cpy.src.ptr, i64 1
  %priv.cpy.done16 = icmp eq ptr %priv.cpy.dest.inc15, %arrayctor.end
  br i1 %priv.cpy.done16, label %priv.destr.body.preheader, label %priv.cpyassn.body

priv.destr.body.preheader:                        ; preds = %priv.cpyassn.body
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
declare hidden noundef ptr @_ZTS7point2d.omp.def_constr(ptr noundef returned writeonly)

; Function Attrs: argmemonly mustprogress nofree norecurse nosync nounwind willreturn uwtable
declare hidden void @_ZTS7point2d.omp.copy_assign(ptr nocapture noundef writeonly, ptr nocapture noundef readonly)

; Function Attrs: mustprogress nofree norecurse nosync nounwind readnone willreturn uwtable
declare hidden void @_ZTS7point2d.omp.destr(ptr nocapture readnone)

declare dso_local void @_Z3bazP7point2di(ptr noundef, i32 noundef)

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture)

attributes #0 = { nounwind }
