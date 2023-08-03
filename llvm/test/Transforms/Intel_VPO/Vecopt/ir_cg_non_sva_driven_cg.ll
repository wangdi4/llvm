; Test to check stability of VPlan LLVM-IR vector CG for
; sequence of instructions where some opcodes are SVA-driven.

; RUN: opt -passes=vplan-vec -vplan-force-vf=2 -vplan-print-scalvec-results -disable-output < %s | FileCheck %s --check-prefix=VPLAN-IR
; RUN: opt -S -passes=vplan-vec -vplan-force-vf=2 < %s | FileCheck %s --check-prefix=LLVM-IR

define void @test1(ptr nocapture %arr, ptr %dest, i32 %uni) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]
  %idx = getelementptr inbounds float, ptr %arr, i64 %iv
  %ld = load float, ptr %idx
; VPLAN-IR:        [DA: Div, SVA: ( V )] float [[VP_LD:%.*]] = load ptr [[VP_IDX:%.*]] (SVAOpBits 0->F )
; LLVM-IR:         [[WIDE_LOAD:%.*]] = load <2 x float>, ptr [[TMP0:%.*]], align 4

  ; Bitcast is scalarized using SVA and only last lane value
  ; is preserved.
  %bc = bitcast float %ld to i32
; VPLAN-IR:        [DA: Div, SVA: (  L)] i32 [[VP_BC:%.*]] = bitcast float [[VP_LD]] (SVAOpBits 0->L )
; LLVM-IR:         [[WIDE_LOAD_EXTRACT_1_:%.*]] = extractelement <2 x float> [[WIDE_LOAD]], i32 1
; LLVM-IR-NEXT:    [[TMP1:%.*]] = bitcast float [[WIDE_LOAD_EXTRACT_1_]] to i32

  ; CG for add is not SVA-driven so we need to bcast %bc from
  ; last lane to emit vector add.
; VPLAN-IR:        [DA: Div, SVA: (  L)] i32 [[VP_RES:%.*]] = add i32 [[VP_BC]] i32 [[UNI0:%.*]] (SVAOpBits 0->L 1->L )
; LLVM-IR-NEXT:    [[BROADCAST_SPLATINSERT:%.*]] = insertelement <2 x i32> poison, i32 [[TMP1]], i64 0
; LLVM-IR-NEXT:    [[BROADCAST_SPLAT:%.*]] = shufflevector <2 x i32> [[BROADCAST_SPLATINSERT]], <2 x i32> poison, <2 x i32> zeroinitializer
; LLVM-IR-NEXT:    [[TMP2:%.*]] = add <2 x i32> [[BROADCAST_SPLAT]], [[BROADCAST_SPLAT4:%.*]]
  %res = add i32 %bc, %uni

; VPLAN-IR:        [DA: Uni, SVA: (  L)] store i32 [[VP_RES]] ptr [[DEST0:%.*]] (SVAOpBits 0->L 1->F )
; LLVM-IR-NEXT:    [[DOTEXTRACT_1_:%.*]] = extractelement <2 x i32> [[TMP2]], i32 1
; LLVM-IR-NEXT:    store i32 [[DOTEXTRACT_1_]], ptr [[DEST:%.*]], align 4
  store i32 %res, ptr %dest
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define void @test2(ptr nocapture %arr, ptr %dest, i32 %uni) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %merge ]
  %idx = getelementptr inbounds i32, ptr %arr, i64 %iv
  %ld = load i32, ptr %idx
  %cmp = icmp eq i32 %uni, 42
  br i1 %cmp, label %if.then, label %merge

if.then:
  %v1 = add i32 %uni, 1
  br label %merge

merge:
  ; CG for phi is not SVA-driven.
  %phi.v = phi i32 [ %v1, %if.then ], [ 0, %header ]
; VPLAN-IR:        [DA: Uni, SVA: (F  )] i32 [[PHI:%.*]] = phi  [ i32 {{%.*}}, {{BB.*}} ],  [ i32 0, {{BB.*}} ] (SVAOpBits 0->F 1->F )
; LLVM-IR:         [[SCAL_PHI:%.*]] = phi i32 [ [[TMP2:%.*]], %VPlannedBB3 ], [ 0, %vector.body ]

  ; While getting vector value of %phi.v do not rely on its SVA results.
  %res = add i32 %phi.v, %ld
; VPLAN-IR:        [DA: Div, SVA: (  L)] i32 [[ADD:%.*]] = add i32 [[PHI]] i32 [[VP_LOAD:%.*]] (SVAOpBits 0->L 1->L )
; LLVM-IR:         [[BROADCAST_SPLATINSERT:%.*]] = insertelement <2 x i32> poison, i32 [[SCAL_PHI]], i64 0
; LLVM-IR-NEXT:    [[BROADCAST_SPLAT]] = shufflevector <2 x i32> [[BROADCAST_SPLATINSERT]], <2 x i32> poison, <2 x i32> zeroinitializer
; LLVM-IR-NEXT:    [[VEC_ADD:%.*]] = add <2 x i32> [[BROADCAST_SPLAT]], [[WIDE_LOAD:%.*]]

  store i32 %res, ptr %dest
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

define void @test3(ptr nocapture %arr, ptr %dest) {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %header

header:
  %iv = phi i64 [ 0, %entry ], [ %iv.next, %header ]
  ; GEP will be needed in both first and last scalar context. This is
  ; unsupported in getVectorValue, so emit vector version too.
  %idx = getelementptr inbounds i64, ptr %arr, i64 %iv
; VPLAN-IR:         [DA: Div, SVA: (F L)] ptr [[GEP:%.*]] = getelementptr inbounds i64, ptr %arr i64 [[IV:%.*]] (SVAOpBits 0->FL 1->FL )
; LLVM-IR:          [[SCAL_GEP_FSCAL:%.*]] = getelementptr inbounds i64, ptr %arr, i64 {{%.*}}
; LLVM-IR-NEXT:     [[SCAL_GEP_LSCAL:%.*]] = getelementptr inbounds i64, ptr %arr, i64 {{%.*}}
; LLVM-IR-NEXT:     [[VEC_GEP:%.*]] = getelementptr inbounds i64, ptr %arr, <2 x i64> {{%.*}}

  store i64 %iv, ptr %idx

  ; ptrtoint is not uplifted yet, so we need GEP in vector context here.
  %pti = ptrtoint ptr %idx to i64
; VPLAN-IR:         [DA: Div, SVA: (  L)] i64 [[PTRTOINT:%.*]] = ptrtoint ptr [[GEP]] to i64 (SVAOpBits 0->L )
; LLVM-IR:          [[VEC_PTI:%.*]] = ptrtoint <2 x ptr> [[VEC_GEP:%.*]] to <2 x i64>

  store i64 %pti, ptr %dest
  %iv.next = add nuw nsw i64 %iv, 1
  %exitcond = icmp eq i64 %iv.next, 300
  br i1 %exitcond, label %loop.exit, label %header

loop.exit:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
