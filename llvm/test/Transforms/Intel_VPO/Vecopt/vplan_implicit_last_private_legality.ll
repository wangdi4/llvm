; RUN: opt %s -S -enable-new-pm=0 -vplan-vec -vplan-entities-dump -vplan-print-after-live-inout-list | FileCheck %s
; RUN: opt %s -S -passes='vplan-vec' -vplan-entities-dump -vplan-print-after-live-inout-list | FileCheck %s

define void @_ZGVeN32u__ZTSZ4mainEUlN4sycl3_V17nd_itemILi3EEEE_(i32 addrspace(1)* align 4 %_arg_result_device) {
; CHECK:       Private list
; CHECK:         Exit instr: i32 [[VP_RETVAL_1_I_I_I:%.*]] = phi  [ i32 [[SPEC_STORE_SELECT_I_I_I0:%.*]], [[BB0:BB[0-9]+]] ],  [ i32 [[TMP2:%.*]], [[BB1:BB[0-9]+]] ]
; CHECK-NEXT:    Linked values: i32 [[VP_RETVAL_1_I_I_I]], i32 [[VP_RETVAL_1_I_I_I_PRIV_FINAL:%.*]],
;
; CHECK:         [[BB5:BB[0-9]+]]: # preds: [[BB0]], [[BB1]]
; CHECK-NEXT:     i32 [[VP_RETVAL_1_I_I_I]] = phi  [ i32 [[SPEC_STORE_SELECT_I_I_I0]], [[BB0]] ],  [ i32 [[TMP2]], [[BB1]] ]
; CHECK-NEXT:     br [[BB4:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB4]]: # preds: [[BB5]]
; CHECK-NEXT:     i32 [[VP_INDVAR:%.*]] = add i32 [[VP_INDEX:%.*]] i32 [[VP_INDEX_IND_INIT_STEP:%.*]]
; CHECK-NEXT:     i1 [[VP_VECTOR_LOOP_EXITCOND:%.*]] = icmp ult i32 [[VP_INDVAR]] i32 [[VP_VECTOR_TRIP_COUNT:%.*]]
; CHECK-NEXT:     br i1 [[VP_VECTOR_LOOP_EXITCOND]], [[BB0]], [[BB6:BB[0-9]+]]
; CHECK-EMPTY:
; CHECK-NEXT:    [[BB6]]: # preds: [[BB4]]
; CHECK-NEXT:     i32 [[VP_INDEX_IND_FINAL:%.*]] = induction-final{add} i32 0 i32 1
; CHECK-NEXT:     i32 [[VP_RETVAL_1_I_I_I_PRIV_FINAL]] = private-final-uc i32 [[VP_RETVAL_1_I_I_I]]
; CHECK-NEXT:     br [[BB7:BB[0-9]+]]
;
; CHECK:       vector.body:
; CHECK-NEXT:    [[UNI_PHI0:%.*]] = phi i32 [ 0, [[VPLANNEDBB10:%.*]] ], [ [[TMP4:%.*]], [[VPLANNEDBB60:%.*]] ]
; CHECK-NEXT:    [[VEC_PHI0:%.*]] = phi <32 x i32> [ <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>, [[VPLANNEDBB10]] ], [ [[TMP3:%.*]], [[VPLANNEDBB60]] ]
; CHECK-NEXT:    br i1 [[CMP_I17_I_I0:%.*]], label [[VPLANNEDBB30:%.*]], label [[VPLANNEDBB40:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB4:
; CHECK-NEXT:    br label [[VPLANNEDBB30]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB3:
; CHECK-NEXT:    [[UNI_PHI50:%.*]] = phi i32 [ [[SPEC_STORE_SELECT_I_I_I0]], [[VECTOR_BODY0:%.*]] ], [ [[TMP2]], [[VPLANNEDBB40]] ]
; CHECK-NEXT:    [[BROADCAST_SPLATINSERT0:%.*]] = insertelement <32 x i32> poison, i32 [[UNI_PHI50]], i32 0
; CHECK-NEXT:    [[BROADCAST_SPLAT0:%.*]] = shufflevector <32 x i32> [[BROADCAST_SPLATINSERT0]], <32 x i32> poison, <32 x i32> zeroinitializer
; CHECK-NEXT:    br label [[VPLANNEDBB60]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB6:
; CHECK-NEXT:    [[TMP3]] = add nuw <32 x i32> [[VEC_PHI0]], <i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32, i32 32>
; CHECK-NEXT:    [[TMP4]] = add nuw i32 [[UNI_PHI0]], 32
; CHECK-NEXT:    [[TMP5:%.*]] = icmp ult i32 [[TMP4]], 32
; CHECK-NEXT:    br i1 false, label [[VECTOR_BODY0]], label [[VPLANNEDBB70:%.*]], !llvm.loop !0
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB7:
; CHECK-NEXT:    [[EXTRACTED_PRIV0:%.*]] = extractelement <32 x i32> [[BROADCAST_SPLAT0]], i64 31
; CHECK-NEXT:    br label [[VPLANNEDBB80:%.*]]
; CHECK-EMPTY:
; CHECK-NEXT:  VPlannedBB8:
; CHECK-NEXT:    br label [[FINAL_MERGE0:%.*]]
;
entry:
  %alloca._arg_result_device = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %_arg_result_device, i32 addrspace(1)** %alloca._arg_result_device, align 8
  br label %simd.begin.region

simd.begin.region:                                ; preds = %entry
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 32), "QUAL.OMP.UNIFORM"(i32 addrspace(1)** %alloca._arg_result_device) ]
  br label %simd.loop.preheader

simd.loop.preheader:                              ; preds = %simd.begin.region
  %load._arg_result_device = load i32 addrspace(1)*, i32 addrspace(1)** %alloca._arg_result_device, align 8
  %cmp.i6.inv.i.i = icmp ult i32 0, 8
  %spec.store.select.i.i.i = select i1 %cmp.i6.inv.i.i, i32 1, i32 8
  %conv.i.i.i = zext i32 %spec.store.select.i.i.i to i64
  %0 = add nsw i64 %conv.i.i.i, -1
  %rem.i.i.i = and i64 %0, 0
  %cmp.i17.i.i = icmp eq i64 %rem.i.i.i, 0
  %1 = select i1 %cmp.i6.inv.i.i, i64 0, i64 3
  %div.i18115.i.i = lshr i64 0, %1
  %conv7.i.i.i = trunc i64 %div.i18115.i.i to i32
  %mul.i.i.i = mul i32 %spec.store.select.i.i.i, %conv7.i.i.i
  %cmp11.not.i.i.i = icmp ugt i32 %mul.i.i.i, 59
  %conv16.i.i.i = sub i32 0, %mul.i.i.i
  %2 = select i1 %cmp11.not.i.i.i, i32 %spec.store.select.i.i.i, i32 %conv16.i.i.i
  br label %simd.loop.header

simd.loop.header:                                 ; preds = %simd.loop.latch, %simd.loop.preheader
  %index = phi i32 [ 0, %simd.loop.preheader ], [ %indvar, %simd.loop.latch ]
  br i1 %cmp.i17.i.i, label %_ZNK3Foo22get_local_linear_rangeEv.exit.i.i, label %if.end.i.i.i

if.end.i.i.i:                                     ; preds = %simd.loop.header
  br label %_ZNK3Foo22get_local_linear_rangeEv.exit.i.i

_ZNK3Foo22get_local_linear_rangeEv.exit.i.i:      ; preds = %if.end.i.i.i, %simd.loop.header
  %retval.1.i.i.i = phi i32 [ %spec.store.select.i.i.i, %simd.loop.header ], [ %2, %if.end.i.i.i ]
  br label %simd.loop.latch

simd.loop.latch:                                  ; preds = %_ZNK3Foo22get_local_linear_rangeEv.exit.i.i
  %indvar = add nuw i32 %index, 1
  %vl.cond = icmp ult i32 %indvar, 32
  br i1 %vl.cond, label %simd.loop.header, label %simd.end.region

simd.end.region:                                  ; preds = %simd.loop.latch
  %retval.1.i.i.i.lcssa = phi i32 [ %retval.1.i.i.i, %simd.loop.latch ]
  store i32 %retval.1.i.i.i.lcssa, i32 addrspace(1)* %load._arg_result_device, align 4
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %simd.end.region
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %return

return:                                           ; preds = %DIR.OMP.END.SIMD.1
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)
