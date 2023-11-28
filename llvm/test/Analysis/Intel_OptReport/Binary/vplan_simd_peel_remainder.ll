; RUN: llc < %s -O3 -intel-opt-report=high -intel-opt-report-emitter=mir -opt-report-embed -enable-protobuf-opt-report=true -debug-only=opt-report-support-utils 2>&1 | FileCheck %s
; REQUIRES: asserts, proto_bor

; CHECK:      Global Mloop optimization report for : test_store
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN
; CHECK-NEXT: <Peeled loop for vectorization>
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN
; CHECK-NEXT:     remark #15301: SIMD LOOP WAS VECTORIZED
; CHECK-NEXT:     remark #15305: vectorization support: vector length 4
; CHECK-NEXT: LOOP END
; CHECK-EMPTY:
; CHECK-NEXT: LOOP BEGIN
; CHECK-NEXT: <Remainder loop for vectorization>
; CHECK-NEXT: LOOP END
; CHECK-NEXT: =================================================================

; CHECK-LABEL:    --- Start Protobuf Binary OptReport Printer ---
; CHECK-NEXT:     Version: 1.5
; CHECK-NEXT:     Property Message Map:
; CHECK-DAG:        C_LOOP_VEC_REMAINDER --> Remainder loop for vectorization
; CHECK-DAG:        C_LOOP_VEC_SIMD --> SIMD LOOP WAS VECTORIZED
; CHECK-DAG:        C_LOOP_VEC_VL --> vectorization support: vector length %s
; CHECK-DAG:        C_LOOP_VEC_PEEL --> Peeled loop for vectorization
; CHECK-NEXT:     Number of reports: 3

; CHECK-DAG:      === Loop Begin ===
; CHECK-DAG:      Anchor ID: 8a2af83a71e5136bad6207fa9f802242
; CHECK-DAG:      Number of remarks: 2
; CHECK-DAG:        Property: C_LOOP_VEC_SIMD, Remark ID: 15301, Remark Args:
; CHECK-DAG:        Property: C_LOOP_VEC_VL, Remark ID: 15305, Remark Args: 4
; CHECK-DAG:      ==== Loop End ====

; CHECK-DAG:      === Loop Begin ===
; CHECK-DAG:      Anchor ID: 9a5aada5429edd5a0977b70c87de0001
; CHECK-DAG:      Number of remarks: 1
; CHECK-DAG:        Property: C_LOOP_VEC_REMAINDER, Remark ID: 25519, Remark Args:
; CHECK-DAG:      ==== Loop End ====

; CHECK-DAG:      === Loop Begin ===
; CHECK-DAG:      Anchor ID: b752171342c9fd0372993138aa502b85
; CHECK-DAG:      Number of remarks: 1
; CHECK-DAG:        Property: C_LOOP_VEC_PEEL, Remark ID: 25518, Remark Args:
; CHECK-DAG:      ==== Loop End ====
; CHECK:          --- End Protobuf Binary OptReport Printer ---

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "x86_64-unknown-linux-gnu"

define void @test_store(i64* nocapture %ary, i32 %c) {
entry:
  br label %peel.checkz23

peel.checkz23:                                    ; preds = %entry
  %broadcast.splatinsert = insertelement <4 x i64*> poison, i64* %ary, i32 0
  %broadcast.splat = shufflevector <4 x i64*> %broadcast.splatinsert, <4 x i64*> poison, <4 x i32> zeroinitializer
  %0 = ptrtoint <4 x i64*> %broadcast.splat to <4 x i64>
  %.extract.0. = extractelement <4 x i64> %0, i32 0
  %1 = udiv i64 %.extract.0., 8
  %2 = mul i64 %1, 3
  %3 = urem i64 %2, 4
  %4 = icmp eq i64 0, %3
  br i1 %4, label %merge.blk21, label %peel.checkv24

peel.checkv24:                                    ; preds = %peel.checkz23
  %5 = add i64 %3, 4
  %6 = icmp ugt i64 %5, 1026
  br i1 %6, label %merge.blk19, label %PeelBlk13

PeelBlk13:                                        ; preds = %peel.checkv24
  br label %for.body.sl.clone

VPlannedBB:                                       ; preds = %for.body.sl.clone
  br label %merge.blk21

merge.blk21:                                      ; preds = %VPlannedBB, %peel.checkz23
  %uni.phi = phi i64 [ 0, %peel.checkz23 ], [ %indvars.iv.next.sl.clone, %VPlannedBB ]
  br label %VPlannedBB1

VPlannedBB1:                                      ; preds = %merge.blk21
  %7 = add i64 %3, 4
  %8 = icmp ugt i64 %7, 1026
  br i1 %8, label %merge.blk19, label %VPlannedBB2

VPlannedBB2:                                      ; preds = %VPlannedBB1
  %broadcast.splatinsert6 = insertelement <4 x i32> poison, i32 %c, i32 0
  %broadcast.splat7 = shufflevector <4 x i32> %broadcast.splatinsert6, <4 x i32> poison, <4 x i32> zeroinitializer
  br label %VPlannedBB3

VPlannedBB3:                                      ; preds = %VPlannedBB2
  %uni.phiind.start.bcast.splatinsert = insertelement <4 x i64> poison, i64 %uni.phi, i32 0
  %uni.phiind.start.bcast.splat = shufflevector <4 x i64> %uni.phiind.start.bcast.splatinsert, <4 x i64> poison, <4 x i32> zeroinitializer
  %9 = add <4 x i64> %uni.phiind.start.bcast.splat, <i64 0, i64 1, i64 2, i64 3>
  %n.adjst = sub nuw nsw i64 1026, %3
  %n.mod.vf = urem i64 %n.adjst, 4
  %n.vec = sub nuw nsw i64 1026, %n.mod.vf
  br label %vector.body

vector.body:                                      ; preds = %vector.body, %VPlannedBB3
  %uni.phi5 = phi i64 [ %uni.phi, %VPlannedBB3 ], [ %14, %vector.body ]
  %vec.phi = phi <4 x i64> [ %9, %VPlannedBB3 ], [ %13, %vector.body ]
  %scalar.gep = getelementptr inbounds i64, i64* %ary, i64 %uni.phi5
  %10 = sext <4 x i32> %broadcast.splat7 to <4 x i64>
  %11 = add <4 x i64> %10, %vec.phi
  %12 = bitcast i64* %scalar.gep to <4 x i64>*
  store <4 x i64> %11, <4 x i64>* %12, align 8, !intel.preferred_alignment !0
  %13 = add nuw nsw <4 x i64> %vec.phi, <i64 4, i64 4, i64 4, i64 4>
  %14 = add nuw nsw i64 %uni.phi5, 4
  %15 = icmp ult i64 %14, %n.vec
  br i1 %15, label %vector.body, label %VPlannedBB8, !llvm.loop !1

VPlannedBB8:                                      ; preds = %vector.body
  %16 = mul i64 1, %n.vec
  %17 = add i64 0, %16
  br label %VPlannedBB9

VPlannedBB9:                                      ; preds = %VPlannedBB8
  br label %VPlannedBB10

VPlannedBB10:                                     ; preds = %VPlannedBB9
  %18 = icmp eq i64 1026, %n.vec
  br i1 %18, label %final.merge, label %merge.blk19

merge.blk19:                                      ; preds = %VPlannedBB10, %VPlannedBB1, %peel.checkv24
  %uni.phi11 = phi i64 [ %17, %VPlannedBB10 ], [ 0, %peel.checkv24 ], [ %uni.phi, %VPlannedBB1 ]
  br label %RemBlk15

RemBlk15:                                         ; preds = %merge.blk19
  br label %for.body

VPlannedBB12:                                     ; preds = %for.body
  br label %final.merge

final.merge:                                      ; preds = %VPlannedBB12, %VPlannedBB10
  %uni.phi13 = phi i64 [ %indvars.iv.next, %VPlannedBB12 ], [ %17, %VPlannedBB10 ]
  br label %for.end

for.body:                                         ; preds = %RemBlk15, %for.body
  %indvars.iv = phi i64 [ %uni.phi11, %RemBlk15 ], [ %indvars.iv.next, %for.body ]
  %ptr = getelementptr inbounds i64, i64* %ary, i64 %indvars.iv
  %cc = sext i32 %c to i64
  %add = add i64 %cc, %indvars.iv
  store i64 %add, i64* %ptr, align 8
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %cmp = icmp ult i64 %indvars.iv.next, 1026
  br i1 %cmp, label %for.body, label %VPlannedBB12, !llvm.loop !8

for.body.sl.clone:                                ; preds = %PeelBlk13, %for.body.sl.clone
  %indvars.iv.sl.clone = phi i64 [ 0, %PeelBlk13 ], [ %indvars.iv.next.sl.clone, %for.body.sl.clone ]
  %ptr.sl.clone = getelementptr inbounds i64, i64* %ary, i64 %indvars.iv.sl.clone
  %cc.sl.clone = sext i32 %c to i64
  %add.sl.clone = add i64 %cc.sl.clone, %indvars.iv.sl.clone
  store i64 %add.sl.clone, i64* %ptr.sl.clone, align 8
  %indvars.iv.next.sl.clone = add nuw nsw i64 %indvars.iv.sl.clone, 1
  %cmp.sl.clone = icmp ult i64 %indvars.iv.next.sl.clone, %3
  br i1 %cmp.sl.clone, label %for.body.sl.clone, label %VPlannedBB, !llvm.loop !14

for.end:                                          ; preds = %final.merge
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

attributes #0 = { nounwind }

!0 = !{i32 32}
!1 = distinct !{!1, !2, !7}
!2 = distinct !{!"intel.optreport", !4}
!4 = !{!"intel.optreport.remarks", !5, !6}
!5 = !{!"intel.optreport.remark", i32 15301}
!6 = !{!"intel.optreport.remark", i32 15305, !"4"}
!7 = !{!"llvm.loop.isvectorized", i32 1}
!8 = distinct !{!8, !9, !10, !7}
!9 = !{!"llvm.loop.vectorize.enable", i32 1}
!10 = distinct !{!"intel.optreport", !12}
!12 = !{!"intel.optreport.origin", !13}
!13 = !{!"intel.optreport.remark", i32 25519}
!14 = distinct !{!14, !9, !15}
!15 = distinct !{!"intel.optreport", !17}
!17 = !{!"intel.optreport.origin", !18}
!18 = !{!"intel.optreport.remark", i32 25518}
