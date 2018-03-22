; RUN: opt -VPlanDriver -vplan-print-after-simplify-cfg -S < %s 2>&1 | FileCheck %s

; Test check plain dump of a VPlan

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

@N = common local_unnamed_addr global i32 0, align 4
@a = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16
@b = common local_unnamed_addr global [1024 x i32] zeroinitializer, align 16

; Function Attrs: noinline norecurse nounwind uwtable
define i32 @foo() local_unnamed_addr {
; CHECK: Print after simplify plain CFG
; CHECK-NEXT:  REGION: [[region_1:region[0-9]+]] (BP: NULL)
; CHECK-NEXT:  [[BB_7:BB[0-9]+]] (BP: NULL) :
; CHECK-NEXT:   <Empty Block>
; CHECK-NEXT:  SUCCESSORS(1):[[BB_5:BB[0-9]+]]

; CHECK:       [[BB_5]] (BP: NULL) :
; CHECK-NEXT:   [[vp_864:%vp[0-9]+]] = load i32* @N
; CHECK-NEXT:   [[vp_320:%vp[0-9]+]] = sext [[vp_864]]
; CHECK-NEXT:  SUCCESSORS(1):[[BB_2:BB[0-9]+]]

; CHECK:       [[BB_2]] (BP: NULL) :
; CHECK-NEXT:   [[vp_52896:%vp[0-9]+]] = phi i64 0 [[vp_54720:%vp[0-9]+]]
; CHECK-NEXT:   [[vp_54896:%vp[0-9]+]] = getelementptr [1024 x i32]* @a i64 0 [[vp_52896]]
; CHECK-NEXT:   [[vp_55232:%vp[0-9]+]] = load [[vp_54896]]
; CHECK-NEXT:   [[vp_55408:%vp[0-9]+]] = getelementptr [1024 x i32]* @b i64 0 [[vp_52896]]
; CHECK-NEXT:   [[vp_55792:%vp[0-9]+]] = load [[vp_55408]]
; CHECK-NEXT:   [[vp_55968:%vp[0-9]+]] = icmp [[vp_55232]] [[vp_55792]]
; CHECK-NEXT:  SUCCESSORS(1):[[BB_9:BB[0-9]+]]

; CHECK:       [[BB_9]] (BP: NULL) :
; CHECK-NEXT:   <Empty Block>
; CHECK-NEXT:   Condition([[BB_2]]): [[vp_55968]] = icmp [[vp_55232]] [[vp_55792]]
; CHECK-NEXT:  SUCCESSORS(2):[[BB_3:BB[0-9]+]]([[vp_55968]]), [[BB_4:BB[0-9]+]](![[vp_55968]])

; CHECK:        [[BB_3]] (BP: NULL) :
; CHECK-NEXT:     [[vp_61632:%vp[0-9]+]] = icmp [[vp_55232]] i32 16
; CHECK-NEXT:     [[vp_61920:%vp[0-9]+]] = mul [[vp_55792]] [[vp_55232]]
; CHECK-NEXT:     [[vp_62128:%vp[0-9]+]] = add [[vp_55792]] [[vp_55232]]
; CHECK-NEXT:     [[vp_62368:%vp[0-9]+]] = select [[vp_61632]] [[vp_61920]] i32 1
; CHECK-NEXT:     [[vp_62672:%vp[0-9]+]] = select [[vp_61632]] [[vp_62128]] i32 1
; CHECK-NEXT:     [[vp_62976:%vp[0-9]+]] = mul [[vp_55792]] [[vp_55792]]
; CHECK-NEXT:     [[vp_63216:%vp[0-9]+]] = mul [[vp_55232]] [[vp_55232]]
; CHECK-NEXT:    SUCCESSORS(1):[[BB_4]]

; CHECK:       [[BB_4]] (BP: NULL) :
; CHECK-NEXT:   [[vp_63392:%vp[0-9]+]] = phi [[vp_62976]] i32 0
; CHECK-NEXT:   [[vp_63648:%vp[0-9]+]] = phi [[vp_63216]] i32 0
; CHECK-NEXT:   [[vp_63904:%vp[0-9]+]] = phi [[vp_62368]] i32 1
; CHECK-NEXT:   [[vp_64160:%vp[0-9]+]] = phi [[vp_62672]] i32 1
; CHECK-NEXT:   [[vp_64416:%vp[0-9]+]] = mul [[vp_63904]] [[vp_63392]]
; CHECK-NEXT:   [[vp_64592:%vp[0-9]+]] = add [[vp_64416]] [[vp_55792]]
; CHECK-NEXT:   store [[vp_64592]] [[vp_55408]]
; CHECK-NEXT:   [[vp_64944:%vp[0-9]+]] = mul [[vp_64160]] [[vp_63648]]
; CHECK-NEXT:   [[vp_65120:%vp[0-9]+]] = add [[vp_64944]] [[vp_55232]]
; CHECK-NEXT:   store [[vp_65120]] [[vp_54896]]
; CHECK-NEXT:   [[vp_54720]] = add [[vp_52896]] i64 1
; CHECK-NEXT:   [[vp_112:%vp[0-9]+]] = icmp [[vp_54720]] [[vp_320]]
; CHECK-NEXT:  SUCCESSORS(1):[[BB_10:BB[0-9]+]]

; CHECK:      [[BB_10]] (BP: NULL) :
; CHECK-NEXT:   <Empty Block>
; CHECK-NEXT:   Condition([[BB_4]]): [[vp_112]] = icmp [[vp_54720]] [[vp_320]]
; CHECK-NEXT:  SUCCESSORS(2):[[BB_2]]([[vp_112]]), [[BB_6:BB[0-9]+]](![[vp_112]])

; CHECK:      [[BB_6]] (BP: NULL) :
; CHECK-NEXT:   <Empty Block>
; CHECK-NEXT:  SUCCESSORS(1):[[BB_8:BB[0-9]+]]

; CHECK:      [[BB_8]] (BP: NULL) :
; CHECK-NEXT:   <Empty Block>
; CHECK-NEXT:  END Block - no SUCCESSORS
; CHECK-NEXT:  END Region([[region_1]])

entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
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
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.END.SIMD.1
  ret i32 0
}

declare void @llvm.intel.directive(metadata)

