; Test to verify that VPlan framework imports and handles user-defined reduction
; entity.

; C code for reference:
; struct point {
;   int x;
;   int y;
; };
;
; #pragma omp declare reduction(min : struct point : \
;         omp_out.x = omp_in.x > omp_out.x  ? omp_out.x : omp_in.x, \
;         omp_out.y = omp_in.y > omp_out.y  ? omp_out.y : omp_in.y ) \
;         initializer( omp_priv = { INT_MAX, INT_MAX } )
;
; #pragma omp declare reduction(max : struct point : \
;         omp_out.x = omp_in.x < omp_out.x  ? omp_out.x : omp_in.x,  \
;         omp_out.y = omp_in.y < omp_out.y  ? omp_out.y : omp_in.y ) \
;         initializer( omp_priv = { 0, 0 } )
;
; void find_enclosing_rectangle ( int n, struct point points[] )
; {
;   struct point minp = { INT_MAX, INT_MAX }, maxp = {0,0};
;   int i;
;
; #pragma omp simd reduction(min:minp) reduction(max:maxp)
;   for ( i = 0; i < n; i++ ) {
;     if ( points[i].x < minp.x ) minp.x = points[i].x;
;     if ( points[i].y < minp.y ) minp.y = points[i].y;
;     if ( points[i].x > maxp.x ) maxp.x = points[i].x;
;     if ( points[i].y > maxp.y ) maxp.y = points[i].y;
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
; IR:          Ref:   %tmpcast.red = alloca %struct.point, align 8
; HIR:         Ref: &((%tmpcast.red)[0])
; CHECK:         UpdateInstructions:
; CHECK-NEXT:    none
; HIR-NEXT:      InitValue: %tmpcast.red
; CHECK-NEXT:    RedDescr: {Kind: call, IsSigned: 0}
; CHECK-NEXT:    RedDescrUDR: {Combiner: .omp_combiner., Initializer: .omp_initializer., Ctor: none, Dtor: none}

; IR:          Ref:   %tmpcast68.red = alloca %struct.point, align 8
; HIR:         Ref: &((%tmpcast68.red)[0])
; CHECK:         UpdateInstructions:
; CHECK-NEXT:    none
; HIR-NEXT:      InitValue: %tmpcast68.red
; CHECK-NEXT:    RedDescr: {Kind: call, IsSigned: 0}
; CHECK-NEXT:    RedDescrUDR: {Combiner: .omp_combiner..1, Initializer: .omp_initializer..2, Ctor: none, Dtor: none}

; ------------------------------------------------------------------------------

; Check that UDRs are imported as VPEntities and lowered to VPInstructions.
; CHECK-LABEL: VPlan after insertion of VPEntities instructions:
; CHECK:       Reduction list

; CHECK-NEXT:   (UDR) Start: %struct.point* %tmpcast.red
; CHECK-NEXT:    Linked values: %struct.point* [[PVT1:%vp.*]],
; CHECK-NEXT:   udr Combiner: .omp_combiner., Initializer: .omp_initializer., Ctor: none, Dtor: none}
; CHECK-NEXT:   Memory: %struct.point* %tmpcast.red

; CHECK-NEXT:   (UDR) Start: %struct.point* %tmpcast68.red
; CHECK-NEXT:    Linked values: %struct.point* [[PVT2:%vp.*]],
; CHECK-NEXT:   udr Combiner: .omp_combiner..1, Initializer: .omp_initializer..2, Ctor: none, Dtor: none}
; CHECK-NEXT:   Memory: %struct.point* %tmpcast68.red

; Initialization
; CHECK:        %struct.point* [[PVT2]] = allocate-priv %struct.point*
; CHECK:        %struct.point* [[PVT1]] = allocate-priv %struct.point*
; CHECK:        call %struct.point* [[PVT1]] %struct.point* %tmpcast.red void (%struct.point*, %struct.point*)* @.omp_initializer.
; CHECK-NEXT:   call %struct.point* [[PVT2]] %struct.point* %tmpcast68.red void (%struct.point*, %struct.point*)* @.omp_initializer..2

; Finalization
; CHECK:        reduction-final-udr %struct.point* [[PVT1]] %struct.point* %tmpcast.red, Combiner: .omp_combiner.
; CHECK-NEXT:   reduction-final-udr %struct.point* [[PVT2]] %struct.point* %tmpcast68.red, Combiner: .omp_combiner..1

; ------------------------------------------------------------------------------

; Check LLVM-IR CG of UDRs.
; IR-LABEL: define dso_local void @find_enclosing_rectangle
; IR:      entry:
; IR:        [[TMPCAST68_RED_VEC:%.*]] = alloca [2 x %struct.point], align 8
; IR-NEXT:   [[TMPCAST68_RED_VEC_BC:%.*]] = bitcast [2 x %struct.point]* [[TMPCAST68_RED_VEC]] to %struct.point*
; IR-NEXT:   [[TMPCAST68_RED_VEC_BASE_ADDR:%.*]] = getelementptr %struct.point, %struct.point* [[TMPCAST68_RED_VEC_BC]], <2 x i32> <i32 0, i32 1>
; IR-NEXT:   [[TMPCAST68_RED_VEC_BASE_ADDR_EXTRACT_1:%.*]] = extractelement <2 x %struct.point*> [[TMPCAST68_RED_VEC_BASE_ADDR]], i32 1
; IR-NEXT:   [[TMPCAST68_RED_VEC_BASE_ADDR_EXTRACT_0:%.*]] = extractelement <2 x %struct.point*> [[TMPCAST68_RED_VEC_BASE_ADDR]], i32 0
; IR-NEXT:   [[TMPCAST_RED_VEC:%.*]] = alloca [2 x %struct.point], align 8
; IR-NEXT:   [[TMPCAST_RED_VEC_BC:%.*]] = bitcast [2 x %struct.point]* [[TMPCAST_RED_VEC]] to %struct.point*
; IR-NEXT:   [[TMPCAST_RED_VEC_BASE_ADDR:%.*]] = getelementptr %struct.point, %struct.point* [[TMPCAST_RED_VEC_BC]], <2 x i32> <i32 0, i32 1>
; IR-NEXT:   [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_1:%.*]] = extractelement <2 x %struct.point*> [[TMPCAST_RED_VEC_BASE_ADDR]], i32 1
; IR-NEXT:   [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_0:%.*]] = extractelement <2 x %struct.point*> [[TMPCAST_RED_VEC_BASE_ADDR]], i32 0

; IR:      VPlannedBB1:                                      ; preds = %VPlannedBB
; IR:        call void @.omp_initializer.(%struct.point* [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_0]], %struct.point* %tmpcast.red)
; IR-NEXT:   call void @.omp_initializer.(%struct.point* [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_1]], %struct.point* %tmpcast.red)
; IR-NEXT:   call void @.omp_initializer..2(%struct.point* [[TMPCAST68_RED_VEC_BASE_ADDR_EXTRACT_0]], %struct.point* %tmpcast68.red)
; IR-NEXT:   call void @.omp_initializer..2(%struct.point* [[TMPCAST68_RED_VEC_BASE_ADDR_EXTRACT_1]], %struct.point* %tmpcast68.red)

; IR:      VPlannedBB36:                                      ; preds = %VPlannedBB35
; IR:        call void @.omp_combiner.(%struct.point* %tmpcast.red, %struct.point* [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_0]])
; IR-NEXT:   call void @.omp_combiner.(%struct.point* %tmpcast.red, %struct.point* [[TMPCAST_RED_VEC_BASE_ADDR_EXTRACT_1]])
; IR-NEXT:   call void @.omp_combiner..1(%struct.point* %tmpcast68.red, %struct.point* [[TMPCAST68_RED_VEC_BASE_ADDR_EXTRACT_0]])
; IR-NEXT:   call void @.omp_combiner..1(%struct.point* %tmpcast68.red, %struct.point* [[TMPCAST68_RED_VEC_BASE_ADDR_EXTRACT_1]])

; ------------------------------------------------------------------------------
; HIR before VPlan
; BEGIN REGION { }
;       %6 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.REDUCTION.UDR:TYPED(&((%tmpcast.red)[0])zeroinitializer1nullnull&((@.omp_combiner.)[0])&((@.omp_initializer.)[0])),  QUAL.OMP.REDUCTION.UDR:TYPED(&((%tmpcast68.red)[0])zeroinitializer1nullnull&((@.omp_combiner..1)[0])&((@.omp_initializer..2)[0])),  QUAL.OMP.LINEAR:IV.TYPED(&((%i.linear.iv)[0])011) ]
;
;       + DO i1 = 0, smax(1, (1 + %5)) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>  <LEGAL_MAX_TC = 2147483647> <simd>
;       |   %guard.start = @llvm.directive.region.entry(); [ DIR.VPO.GUARD.MEM.MOTION(),  QUAL.OMP.LIVEIN:TYPED(&((%tmpcast.red)[0])zeroinitializer1),  QUAL.OMP.LIVEIN:TYPED(&((%tmpcast68.red)[0])zeroinitializer1) ]
;       |   (%i.linear.iv)[0] = i1;
;       |   %7 = (%points)[i1].0;
;       |   %8 = (%tmpcast.red)[0].0;
;       |   if (%7 < %8)
;       |   {
;       |      (%tmpcast.red)[0].0 = %7;
;       |   }
;       |   %9 = (%i.linear.iv)[0];
;       |   %10 = (%points)[%9].1;
;       |   %11 = (%tmpcast.red)[0].1;
;       |   if (%10 < %11)
;       |   {
;       |      (%tmpcast.red)[0].1 = %10;
;       |   }
;       |   %12 = (%i.linear.iv)[0];
;       |   %13 = (%points)[%12].0;
;       |   %14 = (%tmpcast68.red)[0].0;
;       |   if (%13 > %14)
;       |   {
;       |      (%tmpcast68.red)[0].0 = %13;
;       |   }
;       |   %15 = (%i.linear.iv)[0];
;       |   %16 = (%points)[%15].1;
;       |   %17 = (%tmpcast68.red)[0].1;
;       |   if (%16 > %17)
;       |   {
;       |      (%tmpcast68.red)[0].1 = %16;
;       |   }
;       |   %18 = (%i.linear.iv)[0];
;       |   (%i.linear.iv)[0] = %18 + 1;
;       |   @llvm.directive.region.exit(%guard.start); [ DIR.VPO.END.GUARD.MEM.MOTION() ]
;       + END LOOP
;
;       @llvm.directive.region.exit(%6); [ DIR.OMP.END.SIMD() ]
; END REGION


; Check HIR CG of UDRs.
; HIR-LABEL: Function: find_enclosing_rectangle
; HIR:              [[PRIV_MEM_BC:%.*]] = &((%struct.point*)([[PRIV_MEM:%.*]])[0]);
; HIR-NEXT:         [[PRIV_MEM_BC3:%.*]] = &((%struct.point*)([[PRIV_MEM2:%.*]])[0]);
; HIR:              @.omp_initializer.(&((%struct.point*)([[PRIV_MEM2]])[0]),  %tmpcast.red);
; HIR-NEXT:         [[PRIV_MEM_BC3_EXTRACT_1:%.*]] = extractelement &((<2 x %struct.point*>)([[PRIV_MEM_BC3]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_initializer.([[PRIV_MEM_BC3_EXTRACT_1]],  %tmpcast.red);
; HIR-NEXT:         @.omp_initializer..2(&((%struct.point*)([[PRIV_MEM]])[0]),  %tmpcast68.red);
; HIR-NEXT:         [[PRIV_MEM_BC_EXTRACT_1:%.*]] = extractelement &((<2 x %struct.point*>)([[PRIV_MEM_BC]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_initializer..2([[PRIV_MEM_BC_EXTRACT_1]],  %tmpcast68.red);

; HIR:              + DO i1 = 0
; HIR:              + END LOOP

; HIR:              @.omp_combiner.(%tmpcast.red,  &((%struct.point*)([[PRIV_MEM2]])[0]));
; HIR-NEXT:         [[PRIV_MEM_BC3_EXTRACT_1:%.*]] = extractelement &((<2 x %struct.point*>)([[PRIV_MEM_BC3]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_combiner.(%tmpcast.red,  [[PRIV_MEM_BC3_EXTRACT_1]]);
; HIR-NEXT:         @.omp_combiner..1(%tmpcast68.red,  &((%struct.point*)([[PRIV_MEM]])[0]));
; HIR-NEXT:         [[PRIV_MEM_BC_EXTRACT_1:%.*]] = extractelement &((<2 x %struct.point*>)([[PRIV_MEM_BC]])[<i32 0, i32 1>]),  1;
; HIR-NEXT:         @.omp_combiner..1(%tmpcast68.red,  [[PRIV_MEM_BC_EXTRACT_1]]);

; ------------------------------------------------------------------------------


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.point = type { i32, i32 }
%struct.fast_red_t = type <{ %struct.point, %struct.point }>

; Function Attrs: nounwind uwtable
define dso_local void @find_enclosing_rectangle(i32 noundef %n, %struct.point* noundef %points) local_unnamed_addr #0 {
entry:
  %tmpcast68.red = alloca %struct.point, align 8
  %tmpcast.red = alloca %struct.point, align 8
  %fast_red_struct = alloca %struct.fast_red_t, align 4
  %i.linear.iv = alloca i32, align 4
  %minp = alloca i64, align 8
  %tmpcast = bitcast i64* %minp to %struct.point*
  %maxp = alloca i64, align 8
  %tmpcast68 = bitcast i64* %maxp to %struct.point*
  %i = alloca i32, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %0 = bitcast i64* %minp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %0) #2
  store i64 9223372034707292159, i64* %minp, align 8
  %1 = bitcast i64* %maxp to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %1) #2
  store i64 0, i64* %maxp, align 8
  %2 = bitcast i32* %i to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #2
  %cmp = icmp sgt i32 %n, 0
  br i1 %cmp, label %DIR.OMP.SIMD.267, label %omp.precond.end

DIR.OMP.SIMD.267:                                 ; preds = %entry
  %sub1 = add nsw i32 %n, -1
  %3 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3) #2
  %4 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %4) #2
  store i32 %sub1, i32* %.omp.ub, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.267
  br label %DIR.OMP.SIMD.1.split73

DIR.OMP.SIMD.1.split73:                           ; preds = %DIR.OMP.SIMD.1
  br label %DIR.OMP.SIMD.1.split73.split77

DIR.OMP.SIMD.1.split73.split77:                   ; preds = %DIR.OMP.SIMD.1.split73
  %tmpcast68.fast_red = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 1
  br label %DIR.OMP.SIMD.1.split73.split76

DIR.OMP.SIMD.1.split73.split76:                   ; preds = %DIR.OMP.SIMD.1.split73.split77
  call void @.omp_initializer..2(%struct.point* %tmpcast68.red, %struct.point* %tmpcast68)
  br label %DIR.OMP.SIMD.1.split73.split74

DIR.OMP.SIMD.1.split73.split74:                   ; preds = %DIR.OMP.SIMD.1.split73.split76
  %tmpcast.fast_red = getelementptr inbounds %struct.fast_red_t, %struct.fast_red_t* %fast_red_struct, i32 0, i32 0
  br label %DIR.OMP.SIMD.1.split73.split

DIR.OMP.SIMD.1.split73.split:                     ; preds = %DIR.OMP.SIMD.1.split73.split74
  call void @.omp_initializer.(%struct.point* %tmpcast.red, %struct.point* %tmpcast)
  br label %DIR.OMP.SIMD.1.split

DIR.OMP.SIMD.1.split:                             ; preds = %DIR.OMP.SIMD.1.split73.split
  %5 = load i32, i32* %.omp.ub, align 4
  br label %DIR.OMP.SIMD.269

DIR.OMP.SIMD.269:                                 ; preds = %DIR.OMP.SIMD.1.split
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.269
  %cmp3.not71 = icmp sgt i32 0, %5
  br i1 %cmp3.not71, label %DIR.OMP.END.SIMD.3.loopexit, label %omp.inner.for.body.lr.ph

omp.inner.for.body.lr.ph:                         ; preds = %DIR.OMP.SIMD.2
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
 "QUAL.OMP.REDUCTION.UDR:TYPED"(%struct.point* %tmpcast.red, %struct.point zeroinitializer, i32 1, i8* null, i8* null, void (%struct.point*, %struct.point*)* @.omp_combiner., void (%struct.point*, %struct.point*)* @.omp_initializer.),
 "QUAL.OMP.REDUCTION.UDR:TYPED"(%struct.point* %tmpcast68.red, %struct.point zeroinitializer, i32 1, i8* null, i8* null, void (%struct.point*, %struct.point*)* @.omp_combiner..1, void (%struct.point*, %struct.point*)* @.omp_initializer..2),
 "QUAL.OMP.LINEAR:IV.TYPED"(i32* %i.linear.iv, i32 0, i32 1, i32 1) ]

  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4, %omp.inner.for.body.lr.ph
  %.omp.iv.local.072 = phi i32 [ 0, %omp.inner.for.body.lr.ph ], [ %add43, %DIR.VPO.END.GUARD.MEM.MOTION.4 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.1

DIR.VPO.GUARD.MEM.MOTION.1:                       ; preds = %omp.inner.for.body
  br label %DIR.VPO.GUARD.MEM.MOTION.1.split

DIR.VPO.GUARD.MEM.MOTION.1.split:                 ; preds = %DIR.VPO.GUARD.MEM.MOTION.1
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN:TYPED"(%struct.point* %tmpcast.red, %struct.point zeroinitializer, i32 1), "QUAL.OMP.LIVEIN:TYPED"(%struct.point* %tmpcast68.red, %struct.point zeroinitializer, i32 1) ]
  br label %DIR.VPO.GUARD.MEM.MOTION.2

DIR.VPO.GUARD.MEM.MOTION.2:                       ; preds = %DIR.VPO.GUARD.MEM.MOTION.1.split
  store i32 %.omp.iv.local.072, i32* %i.linear.iv, align 4
  %idxprom = sext i32 %.omp.iv.local.072 to i64
  %x = getelementptr inbounds %struct.point, %struct.point* %points, i64 %idxprom, i32 0
  %7 = load i32, i32* %x, align 4
  %x5 = getelementptr inbounds %struct.point, %struct.point* %tmpcast.red, i64 0, i32 0
  %8 = load i32, i32* %x5, align 4
  %cmp6 = icmp slt i32 %7, %8
  br i1 %cmp6, label %if.then, label %if.end

if.then:                                          ; preds = %DIR.VPO.GUARD.MEM.MOTION.2
  store i32 %7, i32* %x5, align 4
  br label %if.end

if.end:                                           ; preds = %if.then, %DIR.VPO.GUARD.MEM.MOTION.2
  %9 = load i32, i32* %i.linear.iv, align 4
  %idxprom11 = sext i32 %9 to i64
  %y = getelementptr inbounds %struct.point, %struct.point* %points, i64 %idxprom11, i32 1
  %10 = load i32, i32* %y, align 4
  %y13 = getelementptr inbounds %struct.point, %struct.point* %tmpcast.red, i64 0, i32 1
  %11 = load i32, i32* %y13, align 4
  %cmp14 = icmp slt i32 %10, %11
  br i1 %cmp14, label %if.then15, label %if.end20

if.then15:                                        ; preds = %if.end
  store i32 %10, i32* %y13, align 4
  br label %if.end20

if.end20:                                         ; preds = %if.then15, %if.end
  %12 = load i32, i32* %i.linear.iv, align 4
  %idxprom21 = sext i32 %12 to i64
  %x23 = getelementptr inbounds %struct.point, %struct.point* %points, i64 %idxprom21, i32 0
  %13 = load i32, i32* %x23, align 4
  %x24 = getelementptr inbounds %struct.point, %struct.point* %tmpcast68.red, i64 0, i32 0
  %14 = load i32, i32* %x24, align 4
  %cmp25 = icmp sgt i32 %13, %14
  br i1 %cmp25, label %if.then26, label %if.end31

if.then26:                                        ; preds = %if.end20
  store i32 %13, i32* %x24, align 4
  br label %if.end31

if.end31:                                         ; preds = %if.then26, %if.end20
  %15 = load i32, i32* %i.linear.iv, align 4
  %idxprom32 = sext i32 %15 to i64
  %y34 = getelementptr inbounds %struct.point, %struct.point* %points, i64 %idxprom32, i32 1
  %16 = load i32, i32* %y34, align 4
  %y35 = getelementptr inbounds %struct.point, %struct.point* %tmpcast68.red, i64 0, i32 1
  %17 = load i32, i32* %y35, align 4
  %cmp36 = icmp sgt i32 %16, %17
  br i1 %cmp36, label %if.then37, label %omp.inner.for.inc

if.then37:                                        ; preds = %if.end31
  store i32 %16, i32* %y35, align 4
  br label %omp.inner.for.inc

omp.inner.for.inc:                                ; preds = %if.then37, %if.end31
  %add43 = add nsw i32 %.omp.iv.local.072, 1
  %18 = load i32, i32* %i.linear.iv, align 4
  %add44 = add nsw i32 %18, 1
  store i32 %add44, i32* %i.linear.iv, align 4
  %19 = add i32 %5, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.3

DIR.VPO.END.GUARD.MEM.MOTION.3:                   ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.4

DIR.VPO.END.GUARD.MEM.MOTION.4:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.3
  %cmp3.not = icmp sgt i32 %19, %add43
  br i1 %cmp3.not, label %omp.inner.for.body, label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge: ; preds = %omp.inner.for.inc
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge
  %20 = load %struct.point, %struct.point* %tmpcast.red, align 4
  store %struct.point %20, %struct.point* %tmpcast.fast_red, align 4
  br label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split75

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split75: ; preds = %DIR.OMP.END.SIMD.1
  %21 = load %struct.point, %struct.point* %tmpcast68.red, align 4
  store %struct.point %21, %struct.point* %tmpcast68.fast_red, align 4
  br label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split75.split

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split75.split: ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split75
  %22 = load i32, i32* %i.linear.iv, align 4
  store i32 %22, i32* %i, align 4
  br label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split: ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split75.split
  br label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split.split

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split.split: ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split
  call void @.omp_combiner.(%struct.point* %tmpcast, %struct.point* %tmpcast.fast_red)
  br label %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split.split.split

omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split.split.split: ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split.split
  call void @.omp_combiner..1(%struct.point* %tmpcast68, %struct.point* %tmpcast68.fast_red)
  br label %DIR.OMP.END.SIMD.3.loopexit

DIR.OMP.END.SIMD.3.loopexit:                      ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.3.loopexit_crit_edge.split.split.split, %DIR.OMP.SIMD.2
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.3.loopexit
  br label %DIR.OMP.END.SIMD.370

DIR.OMP.END.SIMD.370:                             ; preds = %DIR.OMP.END.SIMD.3
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.370, %entry
  %23 = bitcast i32* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %23) #2
  %24 = bitcast i32* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %24) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #2
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %1) #2
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %0) #2
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: alwaysinline nounwind uwtable
declare hidden void @.omp_combiner.(%struct.point* noalias noundef, %struct.point* noalias noundef) #3

; Function Attrs: alwaysinline nounwind uwtable
declare hidden void @.omp_initializer.(%struct.point* noalias noundef, %struct.point* noalias) #3

; Function Attrs: alwaysinline nounwind uwtable
declare hidden void @.omp_combiner..1(%struct.point* noalias noundef, %struct.point* noalias noundef) #3

; Function Attrs: alwaysinline nounwind uwtable
declare hidden void @.omp_initializer..2(%struct.point* noalias noundef, %struct.point* noalias) #3

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1
