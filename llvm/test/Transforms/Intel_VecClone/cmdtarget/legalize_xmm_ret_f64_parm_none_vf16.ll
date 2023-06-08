; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

; Test for vector variant arguments and return type legalization as per VFABI
; Return: double
; Arguments: none
; ISA class: XMM
; "vector-variants"="_ZGVxN16_func,_ZGVxM16_func"

; Need 8 registers to return <16 x double> logical type.
; Masked variant needs mask argument legalization

define dso_local nofpclass(nan inf) double @func() local_unnamed_addr #0 {
; CHECK-LABEL: define dso_local x86_regcallcc { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } @_ZGVxN16_func
; CHECK-SAME: () local_unnamed_addr #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <16 x double>, align 128
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RETVAL_GEP_0:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 0
; CHECK-NEXT:    [[VEC_RET_0:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_0:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } poison, <2 x double> [[VEC_RET_0]], 0
; CHECK-NEXT:    [[VEC_RETVAL_GEP_1:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 1
; CHECK-NEXT:    [[VEC_RET_1:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_1]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_1:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_0]], <2 x double> [[VEC_RET_1]], 1
; CHECK-NEXT:    [[VEC_RETVAL_GEP_2:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 2
; CHECK-NEXT:    [[VEC_RET_2:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_2]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_2:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_1]], <2 x double> [[VEC_RET_2]], 2
; CHECK-NEXT:    [[VEC_RETVAL_GEP_3:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 3
; CHECK-NEXT:    [[VEC_RET_3:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_3]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_3:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_2]], <2 x double> [[VEC_RET_3]], 3
; CHECK-NEXT:    [[VEC_RETVAL_GEP_4:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 4
; CHECK-NEXT:    [[VEC_RET_4:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_4]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_4:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_3]], <2 x double> [[VEC_RET_4]], 4
; CHECK-NEXT:    [[VEC_RETVAL_GEP_5:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 5
; CHECK-NEXT:    [[VEC_RET_5:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_5]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_5:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_4]], <2 x double> [[VEC_RET_5]], 5
; CHECK-NEXT:    [[VEC_RETVAL_GEP_6:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 6
; CHECK-NEXT:    [[VEC_RET_6:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_6]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_6:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_5]], <2 x double> [[VEC_RET_6]], 6
; CHECK-NEXT:    [[VEC_RETVAL_GEP_7:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 7
; CHECK-NEXT:    [[VEC_RET_7:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_7]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_7:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_6]], <2 x double> [[VEC_RET_7]], 7
; CHECK-NEXT:    ret { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_7]]
;
; CHECK-LABEL: define dso_local x86_regcallcc { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } @_ZGVxM16_func
; CHECK-SAME: (<2 x double> [[MASK_0:%.*]], <2 x double> [[MASK_1:%.*]], <2 x double> [[MASK_2:%.*]], <2 x double> [[MASK_3:%.*]], <2 x double> [[MASK_4:%.*]], <2 x double> [[MASK_5:%.*]], <2 x double> [[MASK_6:%.*]], <2 x double> [[MASK_7:%.*]]) local_unnamed_addr #[[ATTR0]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <16 x double>, align 128
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <16 x double>, align 128
; CHECK-NEXT:    [[VEC_MASK_GEP_0:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_MASK]], i32 0
; CHECK-NEXT:    store <2 x double> [[MASK_0]], ptr [[VEC_MASK_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_MASK_GEP_1:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_MASK]], i32 1
; CHECK-NEXT:    store <2 x double> [[MASK_1]], ptr [[VEC_MASK_GEP_1]], align 16
; CHECK-NEXT:    [[VEC_MASK_GEP_2:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_MASK]], i32 2
; CHECK-NEXT:    store <2 x double> [[MASK_2]], ptr [[VEC_MASK_GEP_2]], align 16
; CHECK-NEXT:    [[VEC_MASK_GEP_3:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_MASK]], i32 3
; CHECK-NEXT:    store <2 x double> [[MASK_3]], ptr [[VEC_MASK_GEP_3]], align 16
; CHECK-NEXT:    [[VEC_MASK_GEP_4:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_MASK]], i32 4
; CHECK-NEXT:    store <2 x double> [[MASK_4]], ptr [[VEC_MASK_GEP_4]], align 16
; CHECK-NEXT:    [[VEC_MASK_GEP_5:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_MASK]], i32 5
; CHECK-NEXT:    store <2 x double> [[MASK_5]], ptr [[VEC_MASK_GEP_5]], align 16
; CHECK-NEXT:    [[VEC_MASK_GEP_6:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_MASK]], i32 6
; CHECK-NEXT:    store <2 x double> [[MASK_6]], ptr [[VEC_MASK_GEP_6]], align 16
; CHECK-NEXT:    [[VEC_MASK_GEP_7:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_MASK]], i32 7
; CHECK-NEXT:    store <2 x double> [[MASK_7]], ptr [[VEC_MASK_GEP_7]], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RETVAL_GEP_0:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 0
; CHECK-NEXT:    [[VEC_RET_0:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_0:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } poison, <2 x double> [[VEC_RET_0]], 0
; CHECK-NEXT:    [[VEC_RETVAL_GEP_1:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 1
; CHECK-NEXT:    [[VEC_RET_1:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_1]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_1:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_0]], <2 x double> [[VEC_RET_1]], 1
; CHECK-NEXT:    [[VEC_RETVAL_GEP_2:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 2
; CHECK-NEXT:    [[VEC_RET_2:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_2]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_2:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_1]], <2 x double> [[VEC_RET_2]], 2
; CHECK-NEXT:    [[VEC_RETVAL_GEP_3:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 3
; CHECK-NEXT:    [[VEC_RET_3:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_3]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_3:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_2]], <2 x double> [[VEC_RET_3]], 3
; CHECK-NEXT:    [[VEC_RETVAL_GEP_4:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 4
; CHECK-NEXT:    [[VEC_RET_4:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_4]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_4:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_3]], <2 x double> [[VEC_RET_4]], 4
; CHECK-NEXT:    [[VEC_RETVAL_GEP_5:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 5
; CHECK-NEXT:    [[VEC_RET_5:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_5]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_5:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_4]], <2 x double> [[VEC_RET_5]], 5
; CHECK-NEXT:    [[VEC_RETVAL_GEP_6:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 6
; CHECK-NEXT:    [[VEC_RET_6:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_6]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_6:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_5]], <2 x double> [[VEC_RET_6]], 6
; CHECK-NEXT:    [[VEC_RETVAL_GEP_7:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 7
; CHECK-NEXT:    [[VEC_RET_7:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_7]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_7:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_6]], <2 x double> [[VEC_RET_7]], 7
; CHECK-NEXT:    ret { <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_7]]
;
entry:
  ret double 3.14
}

attributes #0 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(none) uwtable "vector-variants"="_ZGVxN16_func,_ZGVxM16_func" }
