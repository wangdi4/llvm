; RUN: llvm-extract %libdir/../clbltfnshared.rtl -rfunc sub_group_broadcastDv16_iDv16_j -S -o - | FileCheck %s

; CHECK: define {{.*}}<16 x i32> @_Z19sub_group_broadcastDv16_iDv16_jS0_(<16 x i32> noundef [[X:%.*]], <16 x i32> noundef [[LIDS:%.*]], <16 x i32> noundef [[MASK:%.*]])
; CHECK-NEXT: entry:
; CHECK-NEXT:   [[TO_BOOL:%.*]] = trunc <16 x i32> [[MASK]] to <16 x i1>
; CHECK-NEXT:   [[TO_INT:%.*]] = bitcast <16 x i1> [[TO_BOOL]] to i16
; CHECK-NEXT:   [[TRAIL_ZERO:%.*]] = tail call i16 @llvm.cttz.i16(i16 [[TO_INT]], i1 true)
; CHECK-NEXT:   [[LEADER:%.*]] = zext nneg i16 [[TRAIL_ZERO]] to i32
; CHECK-NEXT:   [[VECEXT:%.*]] = extractelement <16 x i32> [[LIDS]], i32 [[LEADER]]
; CHECK-NEXT:   [[ELEM:%.*]] = extractelement <16 x i32> [[X]], i32 [[VECEXT]]
; CHECK-NEXT:   [[SPLAT:%.*]] = insertelement <16 x i32> {{.*}}, i32 [[ELEM]], i64 0
; CHECK-NEXT:   [[BROADCAST:%.*]] = shufflevector <16 x i32> [[SPLAT]], <16 x i32> {{.*}}, <16 x i32> zeroinitializer
; CHECK-NEXT:   ret <16 x i32> [[BROADCAST]]
