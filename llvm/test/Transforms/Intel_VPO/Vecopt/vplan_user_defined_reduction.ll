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

; RUN: opt -S -vplan-vec -vplan-entities-dump -vplan-print-legality -vplan-print-after-vpentity-instrs -vplan-force-vf=2  < %s 2>&1 | FileCheck %s  -check-prefixes=IR,CHECK
; RUN: opt -S -passes="vplan-vec" -vplan-entities-dump -vplan-print-legality -vplan-print-after-vpentity-instrs -vplan-force-vf=2 < %s 2>&1 | FileCheck %s -check-prefixes=IR,CHECK
; RUN: opt -disable-output -hir-ssa-deconstruction -hir-temp-cleanup -hir-vplan-vec -vplan-entities-dump -vplan-print-legality -vplan-print-after-vpentity-instrs -print-after=hir-vplan-vec -vplan-force-vf=2 < %s 2>&1 | FileCheck %s -check-prefixes=HIR,CHECK
; RUN: opt -disable-output -passes="hir-ssa-deconstruction,hir-temp-cleanup,hir-vplan-vec,print<hir>" -vplan-entities-dump -vplan-print-legality -vplan-print-after-vpentity-instrs -vplan-force-vf=2 < %s 2>&1 | FileCheck %s -check-prefixes=HIR,CHECK
; REQUIRES: asserts

; ------------------------------------------------------------------------------

; Check that UDRs are captured in legality lists
; CHECK-LABEL: {{VPO|HIR}}Legality UDRList:
; IR:          Ref:   %tmpcast.red = alloca %struct.point, align 4
; HIR:         Ref: &((%tmpcast.red)[0])
; CHECK:         UpdateInstructions:
; CHECK-NEXT:    none
; HIR-NEXT:      InitValue: %tmpcast.red
; CHECK-NEXT:    RedDescr: {Kind: call, IsSigned: 0}
; CHECK-NEXT:    RedDescrUDR: {Combiner: .omp_combiner., Initializer: .omp_initializer., Ctor: none, Dtor: none}

; IR:          Ref:   %tmpcast21.red = alloca %struct.point, align 4
; HIR:         Ref: &((%tmpcast21.red)[0])
; CHECK:         UpdateInstructions:
; CHECK-NEXT:    none
; HIR-NEXT:      InitValue: %tmpcast21.red
; CHECK-NEXT:    RedDescr: {Kind: call, IsSigned: 0}
; CHECK-NEXT:    RedDescrUDR: {Combiner: .omp_combiner..1, Initializer: .omp_initializer..2, Ctor: point.omp.def_constr, Dtor: point.omp.destr}

; ------------------------------------------------------------------------------

; Check that UDRs are imported as VPEntities and lowered to VPInstructions.
; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:       Reduction list

; CHECK-NEXT:   (UDR) Start: %struct.point* %tmpcast.red
; CHECK-NEXT:    Linked values: %struct.point* [[PVT1:%vp.*]],
; CHECK-NEXT:   udr Combiner: .omp_combiner., Initializer: .omp_initializer., Ctor: none, Dtor: none}
; CHECK-NEXT:   Memory: %struct.point* %tmpcast.red

; CHECK-NEXT:   (UDR) Start: %struct.point* %tmpcast21.red
; CHECK-NEXT:    Linked values: %struct.point* [[PVT2:%vp.*]],
; CHECK-NEXT:   udr Combiner: .omp_combiner..1, Initializer: .omp_initializer..2, Ctor: point.omp.def_constr, Dtor: point.omp.destr}
; CHECK-NEXT:   Memory: %struct.point* %tmpcast21.red

; Initialization
; CHECK:        %struct.point* [[PVT2]] = allocate-priv %struct.point*
; CHECK:        %struct.point* [[PVT1]] = allocate-priv %struct.point*
; CHECK:        call %struct.point* [[PVT1]] %struct.point* %tmpcast.red void (%struct.point*, %struct.point*)* @.omp_initializer.
; CHECK-NEXT:   %struct.point* {{.*}} = call %struct.point* [[PVT2]] %struct.point* (%struct.point*)* @point.omp.def_constr
; CHECK-NEXT:   call %struct.point* [[PVT2]] %struct.point* %tmpcast21.red void (%struct.point*, %struct.point*)* @.omp_initializer..2

; In-loop instructions
; CHECK:        call %struct.point* [[PVT1]] %struct.point* {{.*}} void (%struct.point*, %struct.point*)* @minproc
; CHECK:        call %struct.point* [[PVT2]] %struct.point* {{.*}} void (%struct.point*, %struct.point*)* @maxproc

; Finalization
; CHECK:        reduction-final-udr %struct.point* [[PVT1]] %struct.point* %tmpcast.red, Combiner: .omp_combiner.
; CHECK-NEXT:   reduction-final-udr %struct.point* [[PVT2]] %struct.point* %tmpcast21.red, Combiner: .omp_combiner..1
; CHECK-NEXT:   call %struct.point* [[PVT2]] void (%struct.point*)* @point.omp.destr

; ------------------------------------------------------------------------------

; Check LLVM-IR CG of UDRs.
; IR-LABEL: define dso_local void @find_enclosing_rectangle
; IR:      entry:
; IR:        [[TMPCAST21_RED_VEC:%.*]] = alloca [2 x %struct.point], align 8
; IR-NEXT:   [[TMPCAST21_RED_VEC_BC:%.*]] = bitcast [2 x %struct.point]* [[TMPCAST21_RED_VEC]] to %struct.point*
; IR-NEXT:   [[TMPCAST21_RED_VEC_BASE_ADDR:%.*]] = getelementptr %struct.point, %struct.point* [[TMPCAST21_RED_VEC_BC]], <2 x i32> <i32 0, i32 1>
; IR-NEXT:   [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_1:%.*]] = extractelement <2 x %struct.point*> [[TMPCAST21_RED_VEC_BASE_ADDR]], i32 1
; IR-NEXT:   [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_0:%.*]] = extractelement <2 x %struct.point*> [[TMPCAST21_RED_VEC_BASE_ADDR]], i32 0
; IR-NEXT:   [[TMPCAST_RED_VEC:%.*]] = alloca [2 x %struct.point], align 8
; IR-NEXT:   [[TMPCAST_RED_VEC_BC:%.*]] = bitcast [2 x %struct.point]* [[TMPCAST_RED_VEC]] to %struct.point*
; IR-NEXT:   [[TMPCAST_RED_VEC_BASE_ADDR:%.*]] = getelementptr %struct.point, %struct.point* [[TMPCAST_RED_VEC_BC]], <2 x i32> <i32 0, i32 1>
; IR-NEXT:   [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_1:%.*]] = extractelement <2 x %struct.point*> [[TMPCAST_RED_VEC_BASE_ADDR]], i32 1
; IR-NEXT:   [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_0:%.*]] = extractelement <2 x %struct.point*> [[TMPCAST_RED_VEC_BASE_ADDR]], i32 0

; IR:      VPlannedBB2:                                      ; preds = %VPlannedBB1
; IR:        call void @.omp_initializer.(%struct.point* [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_0]], %struct.point* %tmpcast.red)
; IR-NEXT:   call void @.omp_initializer.(%struct.point* [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_1]], %struct.point* %tmpcast.red)
; IR-NEXT:   {{%.*}} = call %struct.point* @point.omp.def_constr(%struct.point* [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_0]])
; IR-NEXT:   {{%.*}} = call %struct.point* @point.omp.def_constr(%struct.point* [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_1]])
; IR-NEXT:   call void @.omp_initializer..2(%struct.point* [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_0]], %struct.point* %tmpcast21.red)
; IR-NEXT:   call void @.omp_initializer..2(%struct.point* [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_1]], %struct.point* %tmpcast21.red)

; IR:      VPlannedBB4:                                      ; preds = %vector.body
; IR:        call void @.omp_combiner.(%struct.point* %tmpcast.red, %struct.point* [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_0]])
; IR-NEXT:   call void @.omp_combiner.(%struct.point* %tmpcast.red, %struct.point* [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_1]])
; IR-NEXT:   call void @.omp_combiner..1(%struct.point* %tmpcast21.red, %struct.point* [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_0]])
; IR-NEXT:   call void @.omp_combiner..1(%struct.point* %tmpcast21.red, %struct.point* [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_1]])
; IR-NEXT:   call void @point.omp.destr(%struct.point* [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_0]])
; IR-NEXT:   call void @point.omp.destr(%struct.point* [[TMPCAST21_RED_VEC_BASE_ADDR_EXTRACT_1]])

; ------------------------------------------------------------------------------

; Check HIR CG of UDRs.
; HIR-LABEL: Function: find_enclosing_rectangle
; HIR:              [[PRIV_MEM_BC:%.*]] = &((%struct.point*)([[PRIV_MEM:%.*]])[0]);
; HIR-NEXT:         [[PRIV_MEM_BC3:%.*]] = &((%struct.point*)([[PRIV_MEM2:%.*]])[0]);
; HIR:              @.omp_initializer.(&((%struct.point*)([[PRIV_MEM2]])[0]),  %tmpcast.red);
; HIR-NEXT:         [[PRIV_MEM_BC3_EXTRACT_1:%.*]] = extractelement &((<2 x %struct.point*>)([[PRIV_MEM_BC3]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_initializer.([[PRIV_MEM_BC3_EXTRACT_1]],  %tmpcast.red);
; HIR-NEXT:         [[CTOR_SERIAL_TEMP:%.*]] = undef;
; HIR-NEXT:         [[CTOR_0:%.*]] = @point.omp.def_constr(&((%struct.point*)([[PRIV_MEM]])[0]));
; HIR-NEXT:         [[CTOR_SERIAL_TEMP]] = insertelement [[CTOR_SERIAL_TEMP]],  [[CTOR_0]],  0;
; HIR-NEXT:         [[CTOR_ARG_EXTRACT:%.*]] = extractelement &((<2 x %struct.point*>)([[PRIV_MEM_BC]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         [[CTOR_1:%.*]] = @point.omp.def_constr([[CTOR_ARG_EXTRACT]]);
; HIR-NEXT:         [[CTOR_SERIAL_TEMP]] = insertelement [[CTOR_SERIAL_TEMP]],  [[CTOR_1]],  1;
; HIR-NEXT:         @.omp_initializer..2(&((%struct.point*)([[PRIV_MEM]])[0]),  %tmpcast21.red);
; HIR-NEXT:         [[PRIV_MEM_BC_EXTRACT_1:%.*]] = extractelement &((<2 x %struct.point*>)([[PRIV_MEM_BC]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_initializer..2([[PRIV_MEM_BC_EXTRACT_1]],  %tmpcast21.red);

; HIR:              + DO i1 = 0
; HIR:              + END LOOP

; HIR:              @.omp_combiner.(%tmpcast.red,  &((%struct.point*)([[PRIV_MEM2]])[0]));
; HIR-NEXT:         [[PRIV_MEM_BC3_EXTRACT_1:%.*]] = extractelement &((<2 x %struct.point*>)([[PRIV_MEM_BC3]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_combiner.(%tmpcast.red,  [[PRIV_MEM_BC3_EXTRACT_1]]);
; HIR-NEXT:         @.omp_combiner..1(%tmpcast21.red,  &((%struct.point*)([[PRIV_MEM]])[0]));
; HIR-NEXT:         [[PRIV_MEM_BC_EXTRACT_1:%.*]] = extractelement &((<2 x %struct.point*>)([[PRIV_MEM_BC]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_combiner..1(%tmpcast21.red,  [[PRIV_MEM_BC_EXTRACT_1]]);
; HIR-NEXT:         @point.omp.destr(&((%struct.point*)([[PRIV_MEM]])[0]));
; HIR-NEXT:         [[DTOR_ARG_EXTRACT:%.*]] = extractelement &((<2 x %struct.point*>)([[PRIV_MEM_BC]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @point.omp.destr([[DTOR_ARG_EXTRACT]]);

; ------------------------------------------------------------------------------


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.point = type { i32, i32 }

declare dso_local void @minproc(%struct.point* nocapture noundef %out, %struct.point* nocapture noundef readonly %in) local_unnamed_addr

declare dso_local void @maxproc(%struct.point* nocapture noundef %out, %struct.point* nocapture noundef readonly %in) local_unnamed_addr

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @.omp_combiner.(%struct.point* noalias nocapture noundef %0, %struct.point* noalias nocapture noundef readonly %1)

declare void @.omp_initializer.(%struct.point* noalias nocapture noundef writeonly %0, %struct.point* noalias nocapture noundef readnone %1)

declare void @.omp_combiner..1(%struct.point* noalias nocapture noundef %0, %struct.point* noalias nocapture noundef readonly %1)

declare void @.omp_initializer..2(%struct.point* noalias nocapture noundef writeonly %0, %struct.point* noalias nocapture noundef readnone %1)

declare %struct.point* @point.omp.def_constr(%struct.point* noundef returned writeonly %0)

declare void @point.omp.destr(%struct.point* nocapture readnone %0)


define dso_local void @find_enclosing_rectangle(i32 noundef %n, %struct.point* nocapture noundef readonly %points) local_unnamed_addr {
entry:
  %tmpcast21.red = alloca %struct.point, align 4
  %tmpcast.red = alloca %struct.point, align 4
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %x.i = getelementptr inbounds %struct.point, %struct.point* %tmpcast21.red, i64 0, i32 0
  store i32 0, i32* %x.i, align 4
  %y.i = getelementptr inbounds %struct.point, %struct.point* %tmpcast21.red, i64 0, i32 1
  store i32 0, i32* %y.i, align 4
  %x.i40 = getelementptr inbounds %struct.point, %struct.point* %tmpcast.red, i64 0, i32 0
  store i32 2147483647, i32* %x.i40, align 4
  %y.i41 = getelementptr inbounds %struct.point, %struct.point* %tmpcast.red, i64 0, i32 1
  store i32 2147483647, i32* %y.i41, align 4
  br label %DIR.OMP.SIMD.164

DIR.OMP.SIMD.164:                                 ; preds = %DIR.OMP.SIMD.1
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.UDR:TYPED"(%struct.point* %tmpcast.red, %struct.point zeroinitializer, i32 1, i8* null, i8* null, void (%struct.point*, %struct.point*)* @.omp_combiner., void (%struct.point*, %struct.point*)* @.omp_initializer.), "QUAL.OMP.REDUCTION.UDR:TYPED"(%struct.point* %tmpcast21.red, %struct.point zeroinitializer, i32 1, %struct.point* (%struct.point*)* @point.omp.def_constr, void (%struct.point*)* @point.omp.destr, void (%struct.point*, %struct.point*)* @.omp_combiner..1, void (%struct.point*, %struct.point*)* @.omp_initializer..2) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.164
  %wide.trip.count63 = zext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.2, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %arrayidx = getelementptr inbounds %struct.point, %struct.point* %points, i64 %indvars.iv
  call void @minproc(%struct.point* noundef nonnull %tmpcast.red, %struct.point* noundef %arrayidx) #0
  call void @maxproc(%struct.point* noundef nonnull %tmpcast21.red, %struct.point* noundef %arrayidx) #0
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count63
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:                               ; preds = %omp.inner.for.body
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  %.fca.0.load = load i32, i32* %x.i40, align 4
  %.fca.1.load = load i32, i32* %y.i41, align 4
  %.fca.0.load58 = load i32, i32* %x.i, align 4
  %.fca.1.load61 = load i32, i32* %y.i, align 4
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
