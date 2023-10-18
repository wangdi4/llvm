; Test to verify that VPlan framework imports and handles user-defined reduction
; entity.

; C code for reference:
; #pragma omp declare reduction(min : struct point : \
;         minproc(&omp_out, &omp_in)) \
;  initializer( omp_priv = { INT_MAX, INT_MAX } )
;
; #pragma omp declare reduction(max : struct point : \
;         maxproc(&omp_out, &omp_in)) \
;  initializer( omp_priv = { 0, 0 } )
;
; void find_enclosing_rectangle ( int n, struct point points[] )
; {
;   struct point minp = { INT_MAX, INT_MAX }, maxp = {0,0};
;   int i;
;
; #pragma omp simd reduction(min:minp) reduction(max:maxp)
;   for ( i = 0; i < n; i++ ) {
;      minproc(&minp, &points[i]);
;      maxproc(&maxp, &points[i]);
;   }
; }

; RUN: opt -S -passes="vplan-vec" -vplan-entities-dump -vplan-print-legality -vplan-print-after-vpentity-instrs -vplan-force-vf=2 < %s 2>&1 | FileCheck %s -check-prefixes=IR,CHECK
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vplan-vec,print<hir>" -vplan-entities-dump -vplan-print-legality -vplan-print-after-vpentity-instrs -vplan-force-vf=2 < %s 2>&1 | FileCheck %s -check-prefixes=HIR,CHECK
; RUN: opt -disable-output -passes=vplan-vec,intel-ir-optreport-emitter -vplan-force-vf=2 -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPT
; RUN: opt -disable-output -passes=hir-ssa-deconstruction,hir-temp-cleanup,hir-vplan-vec,hir-cg,simplifycfg,intel-ir-optreport-emitter -vplan-force-vf=2 -intel-opt-report=high < %s 2>&1 | FileCheck %s --check-prefix=OPTRPT
; REQUIRES: asserts

; ------------------------------------------------------------------------------

; Check that UDRs are captured in legality lists
; CHECK-LABEL: {{VPO|HIR}}Legality UDRList:
; IR:          Ref:   %tmpcast.red = alloca %struct.point, align 4
; HIR:         Ref: &((%tmpcast.red)[0])
; CHECK:         UpdateInstructions:
; CHECK-NEXT:    none
; HIR-NEXT:      InitValue: %tmpcast.red
; CHECK-NEXT:    RedDescr: {Kind: call, IsSigned: 0, IsComplex: 0}
; CHECK-NEXT:    RedDescrUDR: {Combiner: .omp_combiner., Initializer: .omp_initializer., Ctor: none, Dtor: none}

; IR:          Ref:   %tmpcast21.red = alloca %struct.point, align 4
; HIR:         Ref: &((%tmpcast21.red)[0])
; CHECK:         UpdateInstructions:
; CHECK-NEXT:    none
; HIR-NEXT:      InitValue: %tmpcast21.red
; CHECK-NEXT:    RedDescr: {Kind: call, IsSigned: 0, IsComplex: 0}
; CHECK-NEXT:    RedDescrUDR: {Combiner: .omp_combiner..1, Initializer: .omp_initializer..2, Ctor: point.omp.def_constr, Dtor: point.omp.destr}

; ------------------------------------------------------------------------------

; Check that UDRs are imported as VPEntities and lowered to VPInstructions.
; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:       Reduction list

; CHECK-NEXT:   (UDR) Start: ptr %tmpcast.red
; CHECK-NEXT:    Linked values: ptr [[PVT1:%vp.*]],
; CHECK-NEXT:   udr Combiner: .omp_combiner., Initializer: .omp_initializer., Ctor: none, Dtor: none}
; CHECK-NEXT:   Memory: ptr %tmpcast.red

; CHECK-NEXT:   (UDR) Start: ptr %tmpcast21.red
; CHECK-NEXT:    Linked values: ptr [[PVT2:%vp.*]],
; CHECK-NEXT:   udr Combiner: .omp_combiner..1, Initializer: .omp_initializer..2, Ctor: point.omp.def_constr, Dtor: point.omp.destr}
; CHECK-NEXT:   Memory: ptr %tmpcast21.red

; Initialization
; CHECK:        ptr [[PVT2]] = allocate-priv %struct.point
; CHECK:        ptr [[PVT1]] = allocate-priv %struct.point
; CHECK:        call ptr [[PVT1]] ptr %tmpcast.red ptr @.omp_initializer.
; CHECK-NEXT:   ptr {{.*}} = call ptr [[PVT2]] ptr @point.omp.def_constr
; CHECK-NEXT:   call ptr [[PVT2]] ptr %tmpcast21.red ptr @.omp_initializer..2

; In-loop instructions
; IR:        call ptr [[PVT1]] ptr {{.*}} ptr @minproc
; IR:        call ptr [[PVT2]] ptr {{.*}} ptr @maxproc
; HIR:       ptr [[PVT1SUB:%.*]] = subscript inbounds ptr [[PVT1]]
; HIR:       call ptr [[PVT1SUB]] ptr {{.*}} ptr @minproc
; HIR:       ptr [[PVT2SUB:%.*]] = subscript inbounds ptr [[PVT2]]
; HIR:       call ptr [[PVT2SUB]] ptr {{.*}} ptr @maxproc

; Finalization
; CHECK:        reduction-final-udr ptr [[PVT1]] ptr %tmpcast.red, Combiner: .omp_combiner.
; CHECK-NEXT:   reduction-final-udr ptr [[PVT2]] ptr %tmpcast21.red, Combiner: .omp_combiner..1
; CHECK-NEXT:   call ptr [[PVT2]] ptr @point.omp.destr

; ------------------------------------------------------------------------------

; Check LLVM-IR CG of UDRs.
; IR-LABEL: define dso_local void @find_enclosing_rectangle
; IR:      entry:
; IR:        [[TMPCAST21_RED_VEC:%.*]] = alloca [2 x %struct.point], align 8
; IR-NEXT:   [[TMPCAST21_RED_VEC_BASE_ADDR:%.*]] = getelementptr %struct.point, ptr [[TMPCAST21_RED_VEC]], <2 x i32> <i32 0, i32 1>
; IR-NEXT:   [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_1:%.*]] = extractelement <2 x ptr> [[TMPCAST21_RED_VEC_BASE_ADDR]], i32 1
; IR-NEXT:   [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_0:%.*]] = extractelement <2 x ptr> [[TMPCAST21_RED_VEC_BASE_ADDR]], i32 0
; IR-NEXT:   [[TMPCAST_RED_VEC:%.*]] = alloca [2 x %struct.point], align 8
; IR-NEXT:   [[TMPCAST_RED_VEC_BASE_ADDR:%.*]] = getelementptr %struct.point, ptr [[TMPCAST_RED_VEC]], <2 x i32> <i32 0, i32 1>
; IR-NEXT:   [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_1:%.*]] = extractelement <2 x ptr> [[TMPCAST_RED_VEC_BASE_ADDR]], i32 1
; IR-NEXT:   [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_0:%.*]] = extractelement <2 x ptr> [[TMPCAST_RED_VEC_BASE_ADDR]], i32 0

; IR:      VPlannedBB1:                                      ; preds = %VPlannedBB
; IR:        call void @.omp_initializer.(ptr [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_0]], ptr %tmpcast.red)
; IR-NEXT:   call void @.omp_initializer.(ptr [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_1]], ptr %tmpcast.red)
; IR-NEXT:   {{%.*}} = call ptr @point.omp.def_constr(ptr [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_0]])
; IR-NEXT:   {{%.*}} = call ptr @point.omp.def_constr(ptr [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_1]])
; IR-NEXT:   call void @.omp_initializer..2(ptr [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_0]], ptr %tmpcast21.red)
; IR-NEXT:   call void @.omp_initializer..2(ptr [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_1]], ptr %tmpcast21.red)

; IR:      VPlannedBB8:                                      ; preds = %VPlannedBB7
; IR:        call void @.omp_combiner.(ptr %tmpcast.red, ptr [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_0]])
; IR-NEXT:   call void @.omp_combiner.(ptr %tmpcast.red, ptr [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_1]])
; IR-NEXT:   call void @.omp_combiner..1(ptr %tmpcast21.red, ptr [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_0]])
; IR-NEXT:   call void @.omp_combiner..1(ptr %tmpcast21.red, ptr [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_1]])
; IR-NEXT:   call void @point.omp.destr(ptr [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_0]])
; IR-NEXT:   call void @point.omp.destr(ptr [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_1]])

; ------------------------------------------------------------------------------

; HIR before VPlan
; BEGIN REGION { }
;       %0 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.UDR(&((%tmpcast.red)[0])nullnull&((@.omp_combiner.)[0])&((@.omp_initializer.)[0])),  QUAL.OMP.REDUCTION.UDR(&((%tmpcast21.red)[0])&((@point.omp.def_constr)[0])&((@point.omp.destr)[0])&((@.omp_combiner..1)[0])&((@.omp_initializer..2)[0])) ]
;
;       + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <simd>
;       |   %guard.start = @llvm.directive.region.entry(); [ DIR.VPO.GUARD.MEM.MOTION(),  QUAL.OMP.LIVEIN(&((%tmpcast.red)[0])),  QUAL.OMP.LIVEIN(&((%tmpcast21.red)[0])) ]
;       |   @minproc(&((%tmpcast.red)[0]),  &((%points)[i1]));
;       |   @maxproc(&((%tmpcast21.red)[0]),  &((%points)[i1]));
;       |   @llvm.directive.region.exit(%guard.start); [ DIR.VPO.END.GUARD.MEM.MOTION() ]
;       + END LOOP
;
;       @llvm.directive.region.exit(%0); [ DIR.OMP.END.SIMD() ]
; END REGION


; Check HIR CG of UDRs.
; HIR-LABEL: Function: find_enclosing_rectangle
; HIR:              [[PRIV_MEM_BC:%.*]] = &((%struct.point*)([[PRIV_MEM:%.*]])[0]);
; HIR-NEXT:         [[PRIV_MEM_BC3:%.*]] = &((%struct.point*)([[PRIV_MEM2:%.*]])[0]);
; HIR:              @.omp_initializer.(&((%struct.point*)([[PRIV_MEM2]])[0]),  %tmpcast.red);
; HIR-NEXT:         [[PRIV_MEM_BC3_EXTRACT_1:%.*]] = extractelement &((<2 x ptr>)([[PRIV_MEM_BC3]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_initializer.([[PRIV_MEM_BC3_EXTRACT_1]],  %tmpcast.red);
; HIR-NEXT:         [[CTOR_SERIAL_TEMP:%.*]] = undef;
; HIR-NEXT:         [[CTOR_0:%.*]] = @point.omp.def_constr(&((%struct.point*)([[PRIV_MEM]])[0]));
; HIR-NEXT:         [[CTOR_SERIAL_TEMP]] = insertelement [[CTOR_SERIAL_TEMP]],  [[CTOR_0]],  0;
; HIR-NEXT:         [[CTOR_ARG_EXTRACT:%.*]] = extractelement &((<2 x ptr>)([[PRIV_MEM_BC]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         [[CTOR_1:%.*]] = @point.omp.def_constr([[CTOR_ARG_EXTRACT]]);
; HIR-NEXT:         [[CTOR_SERIAL_TEMP]] = insertelement [[CTOR_SERIAL_TEMP]],  [[CTOR_1]],  1;
; HIR-NEXT:         @.omp_initializer..2(&((%struct.point*)([[PRIV_MEM]])[0]),  %tmpcast21.red);
; HIR-NEXT:         [[PRIV_MEM_BC_EXTRACT_1:%.*]] = extractelement &((<2 x ptr>)([[PRIV_MEM_BC]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_initializer..2([[PRIV_MEM_BC_EXTRACT_1]],  %tmpcast21.red);

; HIR:              + DO i1 = 0
; HIR:              + END LOOP

; HIR:              @.omp_combiner.(%tmpcast.red,  &((%struct.point*)([[PRIV_MEM2]])[0]));
; HIR-NEXT:         [[PRIV_MEM_BC3_EXTRACT_1:%.*]] = extractelement &((<2 x ptr>)([[PRIV_MEM_BC3]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_combiner.(%tmpcast.red,  [[PRIV_MEM_BC3_EXTRACT_1]]);
; HIR-NEXT:         @.omp_combiner..1(%tmpcast21.red,  &((%struct.point*)([[PRIV_MEM]])[0]));
; HIR-NEXT:         [[PRIV_MEM_BC_EXTRACT_1:%.*]] = extractelement &((<2 x ptr>)([[PRIV_MEM_BC]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_combiner..1(%tmpcast21.red,  [[PRIV_MEM_BC_EXTRACT_1]]);
; HIR-NEXT:         @point.omp.destr(&((%struct.point*)([[PRIV_MEM]])[0]));
; HIR-NEXT:         [[DTOR_ARG_EXTRACT:%.*]] = extractelement &((<2 x ptr>)([[PRIV_MEM_BC]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @point.omp.destr([[DTOR_ARG_EXTRACT]]);

; ------------------------------------------------------------------------------
; Check opt report reduction remarks
;
; OPTRPT: remark #25588: Loop has SIMD reduction
; OPTRPT-NEXT: remark #15590: vectorization support: user-defined reduction with value type structure of 2 elements
; OPTRPT-NEXT: remark #15590: vectorization support: user-defined reduction with value type structure of 2 elements
; ------------------------------------------------------------------------------


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.point = type { i32, i32 }

declare dso_local void @minproc(ptr nocapture noundef %out, ptr nocapture noundef readonly %in) local_unnamed_addr

declare dso_local void @maxproc(ptr nocapture noundef %out, ptr nocapture noundef readonly %in) local_unnamed_addr

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @.omp_combiner.(ptr noalias nocapture noundef %0, ptr noalias nocapture noundef readonly %1)

declare void @.omp_initializer.(ptr noalias nocapture noundef writeonly %0, ptr noalias nocapture noundef readnone %1)

declare void @.omp_combiner..1(ptr noalias nocapture noundef %0, ptr noalias nocapture noundef readonly %1)

declare void @.omp_initializer..2(ptr noalias nocapture noundef writeonly %0, ptr noalias nocapture noundef readnone %1)

declare ptr @point.omp.def_constr(ptr noundef returned writeonly %0)

declare void @point.omp.destr(ptr nocapture readnone %0)


define dso_local void @find_enclosing_rectangle(i32 noundef %n, ptr nocapture noundef readonly %points) local_unnamed_addr {
entry:
  %tmpcast21.red = alloca %struct.point, align 4
  %tmpcast.red = alloca %struct.point, align 4
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  store i32 0, ptr %tmpcast21.red, align 4
  %y.i = getelementptr inbounds %struct.point, ptr %tmpcast21.red, i64 0, i32 1
  store i32 0, ptr %y.i, align 4
  store i32 2147483647, ptr %tmpcast.red, align 4
  %y.i41 = getelementptr inbounds %struct.point, ptr %tmpcast.red, i64 0, i32 1
  store i32 2147483647, ptr %y.i41, align 4
  br label %DIR.OMP.SIMD.164

DIR.OMP.SIMD.164:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr %tmpcast.red, %struct.point zeroinitializer, i32 1, ptr null, ptr null, ptr @.omp_combiner., ptr @.omp_initializer.), "QUAL.OMP.REDUCTION.UDR:TYPED"(ptr %tmpcast21.red, %struct.point zeroinitializer, i32 1, ptr @point.omp.def_constr, ptr @point.omp.destr, ptr @.omp_combiner..1, ptr @.omp_initializer..2) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.164
  %wide.trip.count63 = zext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %DIR.VPO.END.GUARD.MEM.MOTION.4 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.1

DIR.VPO.GUARD.MEM.MOTION.1:                       ; preds = %omp.inner.for.body
  br label %DIR.VPO.GUARD.MEM.MOTION.1.split

DIR.VPO.GUARD.MEM.MOTION.1.split:                 ; preds = %DIR.VPO.GUARD.MEM.MOTION.1
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr %tmpcast.red), "QUAL.OMP.LIVEIN"(ptr %tmpcast21.red) ]
  br label %DIR.VPO.GUARD.MEM.MOTION.2

DIR.VPO.GUARD.MEM.MOTION.2:                       ; preds = %DIR.VPO.GUARD.MEM.MOTION.1.split
  %arrayidx = getelementptr inbounds %struct.point, ptr %points, i64 %indvars.iv
  call void @minproc(ptr noundef nonnull %tmpcast.red, ptr noundef %arrayidx) #0
  call void @maxproc(ptr noundef nonnull %tmpcast21.red, ptr noundef %arrayidx) #0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.3

DIR.VPO.END.GUARD.MEM.MOTION.3:                   ; preds = %DIR.VPO.GUARD.MEM.MOTION.2
  call void @llvm.directive.region.exit(token %guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.4

DIR.VPO.END.GUARD.MEM.MOTION.4:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.3
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count63
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  %.fca.0.load = load i32, ptr %tmpcast.red, align 4
  %.fca.1.load = load i32, ptr %y.i41, align 4
  %.fca.0.load58 = load i32, ptr %tmpcast21.red, align 4
  %.fca.1.load61 = load i32, ptr %y.i, align 4
  %cmp.i.i = icmp sgt i32 %.fca.0.load58, 0
  %1 = select i1 %cmp.i.i, i32 %.fca.0.load58, i32 0
  %cmp5.i.i = icmp sgt i32 %.fca.1.load61, 0
  %2 = select i1 %cmp5.i.i, i32 %.fca.1.load61, i32 0
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.4, %entry
  %maxp.sroa.0.2 = phi i32 [ 0, %entry ], [ %1, %DIR.OMP.END.SIMD.4 ]
  %maxp.sroa.6.2 = phi i32 [ 0, %entry ], [ %2, %DIR.OMP.END.SIMD.4 ]
  %minp.sroa.0.2 = phi i32 [ 2147483647, %entry ], [ %.fca.0.load, %DIR.OMP.END.SIMD.4 ]
  %minp.sroa.6.2 = phi i32 [ 2147483647, %entry ], [ %.fca.1.load, %DIR.OMP.END.SIMD.4 ]
  ret void
}

attributes #0 = { nounwind }
