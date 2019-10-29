; RUN: opt -VPlanDriver -vplan-print-after-simplify-cfg -S -disable-output -vplan-force-vf=2 < %s 2>&1 | FileCheck %s

; Test to check that the incoming blocks for a VPPHINode are correctly updated after simplifyPlainCFG. The incoming blocks are strictly predecessors of the VPPHINode's parent VPBasicBlock.
; The test checks for the block entires of a VPPHINode based on the HCFG structure which is also verified.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

@N = common local_unnamed_addr global i32 0, align 4
@a = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@b = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr {
; CHECK-LABEL:  Print after simplify plain CFG
; CHECK-NEXT:    REGION: [[REGION0:region[0-9]+]] (BP: NULL)
; CHECK-NEXT:    [[BB0:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[BB1:BB[0-9]+]]
; CHECK:         [[BB1]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[BB2:BB[0-9]+]]
; CHECK:         [[BB2]] (BP: NULL) :
; CHECK-NEXT:     i64 [[VP2:%.*]] = phi [ i64 0, [[BB1]] ], [ i64 {{%vp.*}}, [[BB6:BB[0-9]+]] ]
; CHECK:         SUCCESSORS(1):[[BB3:BB[0-9]+]]
; CHECK:         [[BB3]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:     Condition([[BB2]]): [DA: Divergent] i1 [[VPBP1:.*]] = icmp
; CHECK-NEXT:    SUCCESSORS(2):[[BB4:BB[0-9]+]](i1 [[VPBP1]]), [[BB5:BB[0-9]+]](!i1 [[VPBP1]])
; CHECK:           [[BB4]] (BP: NULL) :
; CHECK:           SUCCESSORS(1):[[BB5]]
; CHECK:         [[BB5]] (BP: NULL) :
; CHECK-NEXT:     [[VP16:%.*]] = phi [ i32 {{%vp.*}}, [[BB4]] ], [ i32 0, [[BB3]] ]
; CHECK-NEXT:     [[VP17:%.*]] = phi [ i32 {{%vp.*}}, [[BB4]] ], [ i32 0, [[BB3]] ]
; CHECK-NEXT:     [[VP18:%.*]] = phi [ i32 {{%vp.*}}, [[BB4]] ], [ i32 1, [[BB3]] ]
; CHECK-NEXT:     [[VP19:%.*]] = phi [ i32 {{%vp.*}}, [[BB4]] ], [ i32 1, [[BB3]] ]
; CHECK:         SUCCESSORS(1):[[BB6]]
; CHECK:         [[BB6]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:     Condition([[BB5]]): [DA: Uniform] i1 [[VPBP2:.*]] = icmp
; CHECK-NEXT:    SUCCESSORS(2):[[BB2]](i1 [[VPBP2]]), [[BB7:BB[0-9]+]](!i1 [[VPBP2]])
; CHECK:         [[BB7]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    SUCCESSORS(1):[[BB8:BB[0-9]+]]
; CHECK:         [[BB8]] (BP: NULL) :
; CHECK-NEXT:     <Empty Block>
; CHECK-NEXT:    no SUCCESSORS
; CHECK-NEXT:    PREDECESSORS(1): BB6
; CHECK-EMPTY:
; CHECK-NEXT:    END Region([[REGION0]])
;
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %L1

L1:
  %0 = load i32, i32* @N, align 4
  %cmp54 = icmp sgt i32 %0, 0
  br i1 %cmp54, label %for.body.lr.ph, label %DIR.OMP.END.SIMD.1

for.body.lr.ph:                                   ; preds = %L1
  %1 = load i32, i32* @N, align 4
  %2 = sext i32 %1 to i64
  br label %for.body

for.body:                                         ; preds = %for.body.lr.ph, %if.end26
  %indvars.iv = phi i64 [ 0, %for.body.lr.ph ], [ %indvars.iv.next, %if.end26 ]
  %arrayidx = getelementptr inbounds [1024 x i32], [1024 x i32]* @a, i64 0, i64 %indvars.iv
  %3 = load i32, i32* %arrayidx, align 4
  %arrayidx2 = getelementptr inbounds [1024 x i32], [1024 x i32]* @b, i64 0, i64 %indvars.iv
  %4 = load i32, i32* %arrayidx2, align 4
  %cmp3 = icmp sgt i32 %3, %4
  br i1 %cmp3, label %if.then, label %if.end26

if.then:                                          ; preds = %for.body
  %cmp6 = icmp eq i32 %3, 16
  %mul = mul nsw i32 %4, %3
  %add = add nsw i32 %4, %3
  %5 = select i1 %cmp6, i32 %mul, i32 1
  %6 = select i1 %cmp6, i32 %add, i32 1
  %mul20 = mul nsw i32 %4, %4
  %mul25 = mul nsw i32 %3, %3
  br label %if.end26

if.end26:                                         ; preds = %if.then, %for.body
  %mb.0 = phi i32 [ %mul20, %if.then ], [ 0, %for.body ]
  %ma.0 = phi i32 [ %mul25, %if.then ], [ 0, %for.body ]
  %mc.1 = phi i32 [ %5, %if.then ], [ 1, %for.body ]
  %md.1 = phi i32 [ %6, %if.then ], [ 1, %for.body ]
  %mul27 = mul nsw i32 %mc.1, %mb.0
  %add30 = add nsw i32 %mul27, %4
  store i32 %add30, i32* %arrayidx2, align 4
  %mul31 = mul nsw i32 %md.1, %ma.0
  %add34 = add nsw i32 %mul31, %3
  store i32 %add34, i32* %arrayidx, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp slt i64 %indvars.iv.next, %2
  br i1 %cmp, label %for.body, label %loop.exit

loop.exit:                                        ; preds = %if.end26
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = loop.exit
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.END.SIMD.1
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry()

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token)
