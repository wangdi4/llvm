; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

; Test for vector variant arguments and return type legalization as per VFABI
; Return: <2 x double>
; Arguments: <2 x double>
; ISA class: XMM
; "vector-variants"="_ZGVxN4v_twice"

define <2 x double> @twice(<2 x double> %v) #0 {
; CHECK-LABEL: define x86_regcallcc { <2 x double>, <2 x double>, <2 x double>, <2 x double> } @_ZGVxN4v_twice
; CHECK-SAME: (<2 x double> [[V_0:%.*]], <2 x double> [[V_1:%.*]], <2 x double> [[V_2:%.*]], <2 x double> [[V_3:%.*]]) #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <8 x double>, align 64
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <8 x double>, align 64
; CHECK-NEXT:    [[VEC_V_GEP_0:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_V]], i32 0
; CHECK-NEXT:    store <2 x double> [[V_0]], ptr [[VEC_V_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_V_GEP_1:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_V]], i32 1
; CHECK-NEXT:    store <2 x double> [[V_1]], ptr [[VEC_V_GEP_1]], align 16
; CHECK-NEXT:    [[VEC_V_GEP_2:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_V]], i32 2
; CHECK-NEXT:    store <2 x double> [[V_2]], ptr [[VEC_V_GEP_2]], align 16
; CHECK-NEXT:    [[VEC_V_GEP_3:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_V]], i32 3
; CHECK-NEXT:    store <2 x double> [[V_3]], ptr [[VEC_V_GEP_3]], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
; CHECK:       simd.begin.region:
; CHECK-NEXT:    [[ENTRY_REGION:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.SIMDLEN"(i32 4) ]
; CHECK-NEXT:    br label [[SIMD_LOOP_PREHEADER:%.*]]
; CHECK:       simd.loop.preheader:
; CHECK-NEXT:    br label [[SIMD_LOOP_HEADER:%.*]]
; CHECK:       simd.loop.header:
; CHECK-NEXT:    [[INDEX:%.*]] = phi i32 [ 0, [[SIMD_LOOP_PREHEADER]] ], [ [[INDVAR:%.*]], [[SIMD_LOOP_LATCH:%.*]] ]
; CHECK-NEXT:    [[VEC_V_GEP:%.*]] = getelementptr <2 x double>, ptr [[VEC_V]], i32 [[INDEX]]
; CHECK-NEXT:    [[VEC_V_ELEM:%.*]] = load <2 x double>, ptr [[VEC_V_GEP]], align 16
; CHECK-NEXT:    [[VEC_V_GEP1:%.*]] = getelementptr <2 x double>, ptr [[VEC_V]], i32 [[INDEX]]
; CHECK-NEXT:    [[VEC_V_ELEM2:%.*]] = load <2 x double>, ptr [[VEC_V_GEP1]], align 16
; CHECK-NEXT:    [[RES:%.*]] = fadd fast <2 x double> [[VEC_V_ELEM2]], [[VEC_V_ELEM]]
; CHECK-NEXT:    [[VEC_RETVAL_GEP:%.*]] = getelementptr <2 x double>, ptr [[VEC_RETVAL]], i32 [[INDEX]]
; CHECK-NEXT:    store <2 x double> [[RES]], ptr [[VEC_RETVAL_GEP]], align 16
; CHECK-NEXT:    br label [[SIMD_LOOP_LATCH]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RETVAL_GEP_0:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 0
; CHECK-NEXT:    [[VEC_RET_0:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_0:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double> } poison, <2 x double> [[VEC_RET_0]], 0
; CHECK-NEXT:    [[VEC_RETVAL_GEP_1:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 1
; CHECK-NEXT:    [[VEC_RET_1:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_1]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_1:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_0]], <2 x double> [[VEC_RET_1]], 1
; CHECK-NEXT:    [[VEC_RETVAL_GEP_2:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 2
; CHECK-NEXT:    [[VEC_RET_2:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_2]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_2:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_1]], <2 x double> [[VEC_RET_2]], 2
; CHECK-NEXT:    [[VEC_RETVAL_GEP_3:%.*]] = getelementptr inbounds <2 x double>, ptr [[VEC_RETVAL]], i32 3
; CHECK-NEXT:    [[VEC_RET_3:%.*]] = load <2 x double>, ptr [[VEC_RETVAL_GEP_3]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_3:%.*]] = insertvalue { <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_2]], <2 x double> [[VEC_RET_3]], 3
; CHECK-NEXT:    ret { <2 x double>, <2 x double>, <2 x double>, <2 x double> } [[VEC_RETVAL_INS_3]]
;
entry:
  %res = fadd fast <2 x double> %v, %v
  ret <2 x double> %res
}

attributes #0 = { "vector-variants"="_ZGVxN4v_twice" }
