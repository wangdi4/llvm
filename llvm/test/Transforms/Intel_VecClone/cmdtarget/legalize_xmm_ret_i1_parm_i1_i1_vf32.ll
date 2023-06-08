; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

; Return: i1
; Arguments: i1, i1
; ISA class: XMM
; TODO -- Add masked variant. Blocked by CMPLRLLVM-48459.
; "vector-variants"="_ZGVxN32vv_func"
;
; Make sure legalization for i1 arguments and return type works right.

define i1 @func(i1 %f1, i1 %f2) #0 {
; CHECK-LABEL: define x86_regcallcc { <16 x i8>, <16 x i8> } @_ZGVxN32vv_func
; CHECK-SAME: (<16 x i8> [[F1_0:%.*]], <16 x i8> [[F1_1:%.*]], <16 x i8> [[F2_0:%.*]], <16 x i8> [[F2_1:%.*]]) #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VEC_F1:%.*]] = alloca <32 x i8>, align 32
; CHECK-NEXT:    [[VEC_F2:%.*]] = alloca <32 x i8>, align 32
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <32 x i8>, align 32
; CHECK-NEXT:    [[VEC_F1_GEP_0:%.*]] = getelementptr inbounds <16 x i8>, ptr [[VEC_F1]], i32 0
; CHECK-NEXT:    store <16 x i8> [[F1_0]], ptr [[VEC_F1_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_F1_GEP_1:%.*]] = getelementptr inbounds <16 x i8>, ptr [[VEC_F1]], i32 1
; CHECK-NEXT:    store <16 x i8> [[F1_1]], ptr [[VEC_F1_GEP_1]], align 16
; CHECK-NEXT:    [[VEC_F2_GEP_0:%.*]] = getelementptr inbounds <16 x i8>, ptr [[VEC_F2]], i32 0
; CHECK-NEXT:    store <16 x i8> [[F2_0]], ptr [[VEC_F2_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_F2_GEP_1:%.*]] = getelementptr inbounds <16 x i8>, ptr [[VEC_F2]], i32 1
; CHECK-NEXT:    store <16 x i8> [[F2_1]], ptr [[VEC_F2_GEP_1]], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RETVAL_GEP_0:%.*]] = getelementptr inbounds <16 x i8>, ptr [[VEC_RETVAL]], i32 0
; CHECK-NEXT:    [[VEC_RET_0:%.*]] = load <16 x i8>, ptr [[VEC_RETVAL_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_0:%.*]] = insertvalue { <16 x i8>, <16 x i8> } poison, <16 x i8> [[VEC_RET_0]], 0
; CHECK-NEXT:    [[VEC_RETVAL_GEP_1:%.*]] = getelementptr inbounds <16 x i8>, ptr [[VEC_RETVAL]], i32 1
; CHECK-NEXT:    [[VEC_RET_1:%.*]] = load <16 x i8>, ptr [[VEC_RETVAL_GEP_1]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_1:%.*]] = insertvalue { <16 x i8>, <16 x i8> } [[VEC_RETVAL_INS_0]], <16 x i8> [[VEC_RET_1]], 1
; CHECK-NEXT:    ret { <16 x i8>, <16 x i8> } [[VEC_RETVAL_INS_1]]
;
entry:
  %or = or i1 %f1, %f2
  ret i1 %or
}

attributes #0 = { "vector-variants"="_ZGVxN32vv_func" }
