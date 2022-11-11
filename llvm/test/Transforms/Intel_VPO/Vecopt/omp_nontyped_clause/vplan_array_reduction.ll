; Test to verify that VPlan vectorizer handles array reduction idioms
; identified in incoming IR.

; REQUIRES: asserts
; RUN: opt -vplan-vec -vplan-force-vf=2 -debug-only=vplan-vec -debug-only=vpo-ir-loop-vectorize-legality -vplan-print-after-vpentity-instrs -vplan-entities-dump -print-after=vplan-vec -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -debug-only=HIRLegality -debug-only=vplan-vec -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefix=HIR

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define i32 @test1(i32* nocapture readonly %A, i64 %N, i32 %init) {
; Checks for LLVM-IR vectorizer.
; CHECK: VPlan LLVM-IR Driver for Function: test1
; CHECK: VPlan after insertion of VPEntities instructions:
; CHECK: Reduction list
; CHECK:  (+) Start: [8 x i32]* %sum
; CHECK:   Linked values: [8 x i32]* [[VPSUMALLOCA:%.*]],
; CHECK:  Memory: [8 x i32]* %sum
; CHECK:  (SIntMin) Start: [9 x i32]* %min
; CHECK:   Linked values: [9 x i32]* [[VPMINALLOCA:%.*]],
; CHECK:  Memory: [9 x i32]* %min

; CHECK: [9 x i32]* [[VPMINALLOCA]] = allocate-priv [9 x i32]*, OrigAlign = 4
; CHECK: [8 x i32]* [[VPSUMALLOCA]] = allocate-priv [8 x i32]*, OrigAlign = 4
; CHECK: reduction-init-arr i32 0 [8 x i32]* [[VPSUMALLOCA]]
; CHECK: reduction-init-arr i32 2147483647 [9 x i32]* [[VPMINALLOCA]]
; CHECK: [8 x i32] [[VPSUMFIN:%.*]] = reduction-final-arr{add} [8 x i32]* [[VPSUMALLOCA]] [8 x i32]* %sum
; CHECK: [9 x i32] [[VPMINFIN:%.*]] = reduction-final-arr{smin} [9 x i32]* [[VPMINALLOCA]] [9 x i32]* %min

; CHECK-LABEL: @test1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[SUM:%.*]] = alloca [8 x i32], align 4
; CHECK-NEXT:    [[MIN:%.*]] = alloca [9 x i32], align 4
; CHECK-NEXT:    [[MIN_VEC:%.*]] = alloca [2 x [9 x i32]], align 4
; CHECK-NEXT:    [[MIN_VEC_BC:%.*]] = bitcast [2 x [9 x i32]]* [[MIN_VEC]] to [9 x i32]*
; CHECK-NEXT:    [[MIN_VEC_BASE_ADDR:%.*]] = getelementptr [9 x i32], [9 x i32]* [[MIN_VEC_BC]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[MIN_VEC_BASE_ADDR_EXTRACT_1_:%.*]] = extractelement <2 x [9 x i32]*> [[MIN_VEC_BASE_ADDR]], i32 1
; CHECK-NEXT:    [[MIN_VEC_BASE_ADDR_EXTRACT_0_:%.*]] = extractelement <2 x [9 x i32]*> [[MIN_VEC_BASE_ADDR]], i32 0
; CHECK-NEXT:    [[SUM_VEC:%.*]] = alloca [2 x [8 x i32]], align 4
; CHECK-NEXT:    [[SUM_VEC_BC:%.*]] = bitcast [2 x [8 x i32]]* [[SUM_VEC]] to [8 x i32]*
; CHECK-NEXT:    [[SUM_VEC_BASE_ADDR:%.*]] = getelementptr [8 x i32], [8 x i32]* [[SUM_VEC_BC]], <2 x i32> <i32 0, i32 1>
; CHECK-NEXT:    [[SUM_VEC_BASE_ADDR_EXTRACT_1_:%.*]] = extractelement <2 x [8 x i32]*> [[SUM_VEC_BASE_ADDR]], i32 1
; CHECK-NEXT:    [[SUM_VEC_BASE_ADDR_EXTRACT_0_:%.*]] = extractelement <2 x [8 x i32]*> [[SUM_VEC_BASE_ADDR]], i32 0
; CHECK-NEXT:    br label [[FILL_SUM:%.*]]

; CHECK:       VPlannedBB2:
; CHECK:         [[ARR_RED_BASE_ADDR_BC:%.*]] = bitcast [8 x i32]* [[SUM_VEC_BASE_ADDR_EXTRACT_0_]] to i32*
; CHECK:         br label [[ARRAY_REDN_INIT_LOOP:%.*]]
; CHECK:       array.redn.init.loop:
; CHECK-NEXT:    [[CUR_ELEM_IDX:%.*]] = phi i64 [ 0, [[VPLANNEDBB2:%.*]] ], [ [[NEXT_ELEM_IDX:%.*]], [[ARRAY_REDN_INIT_LOOP]] ]
; CHECK-NEXT:    [[CUR_ELEM_PTR:%.*]] = getelementptr i32, i32* [[ARR_RED_BASE_ADDR_BC]], i64 [[CUR_ELEM_IDX]]
; CHECK-NEXT:    store i32 0, i32* [[CUR_ELEM_PTR]], align 4
; CHECK-NEXT:    [[NEXT_ELEM_IDX]] = add i64 [[CUR_ELEM_IDX]], 1
; CHECK-NEXT:    [[INITLOOP_DONE:%.*]] = icmp ult i64 [[NEXT_ELEM_IDX]], 16
; CHECK-NEXT:    br i1 [[INITLOOP_DONE]], label [[ARRAY_REDN_INIT_LOOP]], label [[ARRAY_REDN_INIT_LOOPEXIT:%.*]]
; CHECK:       array.redn.init.loopexit:
; CHECK-NEXT:    [[ARR_RED_BASE_ADDR_BC3:%.*]] = bitcast [9 x i32]* [[MIN_VEC_BASE_ADDR_EXTRACT_0_]] to i32*
; CHECK-NEXT:    br label [[ARRAY_REDN_INIT_LOOP4:%.*]]
; CHECK:       array.redn.init.loop4:
; CHECK-NEXT:    [[CUR_ELEM_IDX6:%.*]] = phi i64 [ 0, [[ARRAY_REDN_INIT_LOOPEXIT]] ], [ [[NEXT_ELEM_IDX8:%.*]], [[ARRAY_REDN_INIT_LOOP4]] ]
; CHECK-NEXT:    [[CUR_ELEM_PTR7:%.*]] = getelementptr i32, i32* [[ARR_RED_BASE_ADDR_BC3]], i64 [[CUR_ELEM_IDX6]]
; CHECK-NEXT:    store i32 2147483647, i32* [[CUR_ELEM_PTR7]], align 4
; CHECK-NEXT:    [[NEXT_ELEM_IDX8]] = add i64 [[CUR_ELEM_IDX6]], 1
; CHECK-NEXT:    [[INITLOOP_DONE9:%.*]] = icmp ult i64 [[NEXT_ELEM_IDX8]], 18
; CHECK-NEXT:    br i1 [[INITLOOP_DONE9]], label [[ARRAY_REDN_INIT_LOOP4]], label [[ARRAY_REDN_INIT_LOOPEXIT5:%.*]]

; CHECK:       array.redn.final.main.loop:
; CHECK-NEXT:    [[MAIN_ELEM_IDX:%.*]] = phi i64 [ 0, [[VPLANNEDBB13:%.*]] ], [ [[NEXT_MAIN_ELEM_IDX:%.*]], [[ARRAY_REDN_FINAL_MAIN_LOOP:%.*]] ]
; CHECK-NEXT:    [[ORIG_ARR_GEP:%.*]] = getelementptr [8 x i32], [8 x i32]* [[SUM]], i64 0, i64 [[MAIN_ELEM_IDX]]
; CHECK-NEXT:    [[ORIG_ARR_BC:%.*]] = bitcast i32* [[ORIG_ARR_GEP]] to <4 x i32>*
; CHECK-NEXT:    [[ORIG_ARR_LD:%.*]] = load <4 x i32>, <4 x i32>* [[ORIG_ARR_BC]], align 16
; CHECK-NEXT:    [[PRIV_ARR_GEP_LANE0:%.*]] = getelementptr [8 x i32], [8 x i32]* [[SUM_VEC_BASE_ADDR_EXTRACT_0_]], i64 0, i64 [[MAIN_ELEM_IDX]]
; CHECK-NEXT:    [[PRIV_ARR_BC_LANE0:%.*]] = bitcast i32* [[PRIV_ARR_GEP_LANE0]] to <4 x i32>*
; CHECK-NEXT:    [[PRIV_ARR_LD_LANE0:%.*]] = load <4 x i32>, <4 x i32>* [[PRIV_ARR_BC_LANE0]], align 16
; CHECK-NEXT:    [[ARR_FIN_RED:%.*]] = add <4 x i32> [[ORIG_ARR_LD]], [[PRIV_ARR_LD_LANE0]]
; CHECK-NEXT:    [[PRIV_ARR_GEP_LANE1:%.*]] = getelementptr [8 x i32], [8 x i32]* [[SUM_VEC_BASE_ADDR_EXTRACT_1_]], i64 0, i64 [[MAIN_ELEM_IDX]]
; CHECK-NEXT:    [[PRIV_ARR_BC_LANE1:%.*]] = bitcast i32* [[PRIV_ARR_GEP_LANE1]] to <4 x i32>*
; CHECK-NEXT:    [[PRIV_ARR_LD_LANE1:%.*]] = load <4 x i32>, <4 x i32>* [[PRIV_ARR_BC_LANE1]], align 16
; CHECK-NEXT:    [[ARR_FIN_RED14:%.*]] = add <4 x i32> [[ARR_FIN_RED]], [[PRIV_ARR_LD_LANE1]]
; CHECK-NEXT:    store <4 x i32> [[ARR_FIN_RED14]], <4 x i32>* [[ORIG_ARR_BC]], align 16
; CHECK-NEXT:    [[NEXT_MAIN_ELEM_IDX]] = add i64 [[MAIN_ELEM_IDX]], 4
; CHECK-NEXT:    [[FINAL_MAINLOOP_COND:%.*]] = icmp ult i64 [[NEXT_MAIN_ELEM_IDX]], 8
; CHECK-NEXT:    br i1 [[FINAL_MAINLOOP_COND]], label [[ARRAY_REDN_FINAL_MAIN_LOOP]], label [[ARRAY_REDN_FINAL_REM_LOOP:%.*]]
; CHECK:       array.redn.final.rem.loop:
; CHECK-NEXT:    br label [[ARRAY_REDN_FINAL_EXIT:%.*]]
; CHECK:       array.redn.final.exit:
; CHECK-NEXT:    br label [[ARRAY_REDN_FINAL_MAIN_LOOP15:%.*]]
; CHECK:       array.redn.final.main.loop15:
; CHECK-NEXT:    [[MAIN_ELEM_IDX18:%.*]] = phi i64 [ 0, [[ARRAY_REDN_FINAL_EXIT]] ], [ [[NEXT_MAIN_ELEM_IDX30:%.*]], [[ARRAY_REDN_FINAL_MAIN_LOOP15]] ]
; CHECK-NEXT:    [[ORIG_ARR_GEP19:%.*]] = getelementptr [9 x i32], [9 x i32]* [[MIN]], i64 0, i64 [[MAIN_ELEM_IDX18]]
; CHECK-NEXT:    [[ORIG_ARR_BC20:%.*]] = bitcast i32* [[ORIG_ARR_GEP19]] to <4 x i32>*
; CHECK-NEXT:    [[ORIG_ARR_LD21:%.*]] = load <4 x i32>, <4 x i32>* [[ORIG_ARR_BC20]], align 16
; CHECK-NEXT:    [[PRIV_ARR_GEP_LANE022:%.*]] = getelementptr [9 x i32], [9 x i32]* [[MIN_VEC_BASE_ADDR_EXTRACT_0_]], i64 0, i64 [[MAIN_ELEM_IDX18]]
; CHECK-NEXT:    [[PRIV_ARR_BC_LANE023:%.*]] = bitcast i32* [[PRIV_ARR_GEP_LANE022]] to <4 x i32>*
; CHECK-NEXT:    [[PRIV_ARR_LD_LANE024:%.*]] = load <4 x i32>, <4 x i32>* [[PRIV_ARR_BC_LANE023]], align 16
; CHECK-NEXT:    [[ARR_FIN_RED25:%.*]] = call <4 x i32> @llvm.smin.v4i32(<4 x i32> [[ORIG_ARR_LD21]], <4 x i32> [[PRIV_ARR_LD_LANE024]])
; CHECK-NEXT:    [[PRIV_ARR_GEP_LANE126:%.*]] = getelementptr [9 x i32], [9 x i32]* [[MIN_VEC_BASE_ADDR_EXTRACT_1_]], i64 0, i64 [[MAIN_ELEM_IDX18]]
; CHECK-NEXT:    [[PRIV_ARR_BC_LANE127:%.*]] = bitcast i32* [[PRIV_ARR_GEP_LANE126]] to <4 x i32>*
; CHECK-NEXT:    [[PRIV_ARR_LD_LANE128:%.*]] = load <4 x i32>, <4 x i32>* [[PRIV_ARR_BC_LANE127]], align 16
; CHECK-NEXT:    [[ARR_FIN_RED29:%.*]] = call <4 x i32> @llvm.smin.v4i32(<4 x i32> [[ARR_FIN_RED25]], <4 x i32> [[PRIV_ARR_LD_LANE128]])
; CHECK-NEXT:    store <4 x i32> [[ARR_FIN_RED29]], <4 x i32>* [[ORIG_ARR_BC20]], align 16
; CHECK-NEXT:    [[NEXT_MAIN_ELEM_IDX30]] = add i64 [[MAIN_ELEM_IDX18]], 4
; CHECK-NEXT:    [[FINAL_MAINLOOP_COND31:%.*]] = icmp ult i64 [[NEXT_MAIN_ELEM_IDX30]], 8
; CHECK-NEXT:    br i1 [[FINAL_MAINLOOP_COND31]], label [[ARRAY_REDN_FINAL_MAIN_LOOP15]], label [[ARRAY_REDN_FINAL_REM_LOOP16:%.*]]
; CHECK:       array.redn.final.rem.loop16:
; CHECK-NEXT:    [[REM_ELEM_IDX:%.*]] = phi i64 [ 8, [[ARRAY_REDN_FINAL_MAIN_LOOP15]] ], [ [[NEXT_REM_ELEM_IDX:%.*]], [[ARRAY_REDN_FINAL_REM_LOOP16]] ]
; CHECK-NEXT:    [[ORIG_ARR_GEP32:%.*]] = getelementptr [9 x i32], [9 x i32]* [[MIN]], i64 0, i64 [[REM_ELEM_IDX]]
; CHECK-NEXT:    [[ORIG_ARR_LD33:%.*]] = load i32, i32* [[ORIG_ARR_GEP32]], align 4
; CHECK-NEXT:    [[PRIV_ARR_GEP_LANE034:%.*]] = getelementptr [9 x i32], [9 x i32]* [[MIN_VEC_BASE_ADDR_EXTRACT_0_]], i64 0, i64 [[REM_ELEM_IDX]]
; CHECK-NEXT:    [[PRIV_ARR_LD_LANE035:%.*]] = load i32, i32* [[PRIV_ARR_GEP_LANE034]], align 4
; CHECK-NEXT:    [[ARR_FIN_RED36:%.*]] = call i32 @llvm.smin.i32(i32 [[ORIG_ARR_LD33]], i32 [[PRIV_ARR_LD_LANE035]])
; CHECK-NEXT:    [[PRIV_ARR_GEP_LANE137:%.*]] = getelementptr [9 x i32], [9 x i32]* [[MIN_VEC_BASE_ADDR_EXTRACT_1_]], i64 0, i64 [[REM_ELEM_IDX]]
; CHECK-NEXT:    [[PRIV_ARR_LD_LANE138:%.*]] = load i32, i32* [[PRIV_ARR_GEP_LANE137]], align 4
; CHECK-NEXT:    [[ARR_FIN_RED39:%.*]] = call i32 @llvm.smin.i32(i32 [[ARR_FIN_RED36]], i32 [[PRIV_ARR_LD_LANE138]])
; CHECK-NEXT:    store i32 [[ARR_FIN_RED39]], i32* [[ORIG_ARR_GEP32]], align 4
; CHECK-NEXT:    [[NEXT_REM_ELEM_IDX]] = add i64 [[REM_ELEM_IDX]], 1
; CHECK-NEXT:    [[FINAL_REMLOOP_COND:%.*]] = icmp ult i64 [[NEXT_REM_ELEM_IDX]], 9
; CHECK-NEXT:    br i1 [[FINAL_REMLOOP_COND]], label [[ARRAY_REDN_FINAL_REM_LOOP16]], label [[ARRAY_REDN_FINAL_EXIT17:%.*]]
;
; Checks for HIR vectorizer
; Prototype support added only for LLVM-IR vectorizer.
; HIR: VPlan HIR Driver for Function: test1
; HIR: Cannot handle array reductions.
; HIR: VD: Not vectorizing: Cannot prove legality.
; HIR: Function: test1
;
entry:
  %sum = alloca [8 x i32], align 4
  %min = alloca [9 x i32], align 4
  br label %fill.sum

fill.sum:
  %arr.begin = getelementptr inbounds [8 x i32], [8 x i32]* %sum, i32 0, i32 0
  %arr.end = getelementptr i32, i32* %arr.begin, i32 8
  %red.init.isempty = icmp eq i32* %arr.begin, %arr.end
  br i1 %red.init.isempty, label %fill.min, label %red.init.body

red.init.body:
  %red.curr.ptr = phi i32* [ %arr.begin, %fill.sum ], [ %red.next.ptr, %red.init.body ]
  store i32 %init, i32* %red.curr.ptr, align 4
  %red.next.ptr = getelementptr inbounds i32, i32* %red.curr.ptr, i32 1
  %red.init.done = icmp eq i32* %red.next.ptr, %arr.end
  br i1 %red.init.done, label %begin.simd.1, label %red.init.body

fill.min:
  %arr2.begin = getelementptr inbounds [9 x i32], [9 x i32]* %min, i32 0, i32 0
  %arr2.end = getelementptr i32, i32* %arr2.begin, i32 9
  %red2.init.isempty = icmp eq i32* %arr2.begin, %arr2.end
  br i1 %red2.init.isempty, label %begin.simd.1, label %red2.init.body

red2.init.body:
  %red2.curr.ptr = phi i32* [ %arr2.begin, %fill.min ], [ %red2.next.ptr, %red2.init.body ]
  store i32 %init, i32* %red2.curr.ptr, align 4
  %red2.next.ptr = getelementptr inbounds i32, i32* %red2.curr.ptr, i32 1
  %red2.init.done = icmp eq i32* %red2.next.ptr, %arr2.end
  br i1 %red2.init.done, label %begin.simd.1, label %red2.init.body

begin.simd.1:
  br label %begin.simd

begin.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD"([8 x i32]* %sum), "QUAL.OMP.REDUCTION.MIN"([9 x i32]* %min) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %begin.simd ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %A.i = load i32, i32* %arrayidx, align 4
  %sum.gep = getelementptr inbounds [8 x i32], [8 x i32]* %sum, i64 0, i64 %indvars.iv
  %sum.ld = load i32, i32* %sum.gep, align 4
  %add = add nsw i32 %A.i, %sum.ld
  store i32 %add, i32* %sum.gep, align 4
  %min.gep = getelementptr inbounds [9 x i32], [9 x i32]* %min, i64 0, i64 %indvars.iv
  %min.ld = load i32, i32* %min.gep, align 4
  %cmp = icmp slt i32 %A.i, %min.ld
  %select = select i1 %cmp, i32 %A.i, i32 %min.ld
  store i32 %select, i32* %min.gep, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp slt i64 %indvars.iv.next, %N
  br i1 %exitcond, label %for.body, label %for.cond.cleanup.loopexit

for.cond.cleanup.loopexit:                             ; preds = %for.body
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  %fin.gep = getelementptr inbounds [8 x i32], [8 x i32]* %sum, i32 0, i32 8
  %fin = load i32, i32* %fin.gep, align 4
  %fin2.gep = getelementptr inbounds [9 x i32], [9 x i32]* %min, i32 0, i32 9
  %fin2 = load i32, i32* %fin2.gep, align 4
  %fin.add = add i32 %fin, %fin2
  ret i32 %fin.add

}

define i32 @test2(i32* nocapture readonly %A, i64 %N, i32 %init) {
; test2 cannot be vectorized since it contains non-single element alloca.
; Checks for LLVM-IR vectorizer
; CHECK: VPlan LLVM-IR Driver for Function: test2
; CHECK: Cannot handle array reductions.
; CHECK: VD: Not vectorizing: Cannot prove legality.

; CHECK: define i32 @test2
; CHECK: %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD"(i32* %sum) ]
;
; Checks for HIR vectorizer
; HIR: VPlan HIR Driver for Function: test2
; HIR: Cannot handle array reductions.
; HIR: VD: Not vectorizing: Cannot prove legality.
; HIR: Function: test2
;
entry:
  %sum = alloca i32, i32 42, align 4
  br label %fill.sum

fill.sum:
  %arr.begin = getelementptr inbounds i32, i32* %sum, i32 0
  %arr.end = getelementptr i32, i32* %arr.begin, i32 42
  %red.init.isempty = icmp eq i32* %arr.begin, %arr.end
  br i1 %red.init.isempty, label %begin.simd.1, label %red.init.body

red.init.body:
  %red.curr.ptr = phi i32* [ %arr.begin, %fill.sum ], [ %red.next.ptr, %red.init.body ]
  store i32 %init, i32* %red.curr.ptr, align 4
  %red.next.ptr = getelementptr inbounds i32, i32* %red.curr.ptr, i32 1
  %red.init.done = icmp eq i32* %red.next.ptr, %arr.end
  br i1 %red.init.done, label %begin.simd.1, label %red.init.body

begin.simd.1:
  br label %begin.simd

begin.simd:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD"(i32* %sum) ]
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.body ], [ 0, %begin.simd ]
  %arrayidx = getelementptr inbounds i32, i32* %A, i64 %indvars.iv
  %A.i = load i32, i32* %arrayidx, align 4
  %sum.gep = getelementptr inbounds i32, i32* %sum, i64 %indvars.iv
  %sum.ld = load i32, i32* %sum.gep, align 4
  %add = add nsw i32 %A.i, %sum.ld
  store i32 %add, i32* %sum.gep, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp slt i64 %indvars.iv.next, %N
  br i1 %exitcond, label %for.body, label %for.cond.cleanup.loopexit

for.cond.cleanup.loopexit:                             ; preds = %for.body
  br label %end.simd

end.simd:
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  %fin.gep = getelementptr inbounds i32, i32* %sum, i32 42
  %fin = load i32, i32* %fin.gep, align 4
  ret i32 %fin

}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

