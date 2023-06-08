; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

; Test for vector variant arguments/return type legalization as per VFABI.
; Check that byval arguments are not re-allocated by VecClone pass.
; Return: i32
; Arguments: struct byval
; ISA class: XMM
; "vector-variants"="_ZGVxN8u_foo"
; Origin: byval.ll


%struct.pair = type { i32, i32 }

define i32 @foo(ptr byval(%struct.pair) %x) #0 {
; CHECK-LABEL: define x86_regcallcc { <4 x i32>, <4 x i32> } @_ZGVxN8u_foo
; CHECK-SAME: (ptr byval([[STRUCT_PAIR:%.*]]) [[X:%.*]]) #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <8 x i32>, align 32
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RETVAL_GEP_0:%.*]] = getelementptr inbounds <4 x i32>, ptr [[VEC_RETVAL]], i32 0
; CHECK-NEXT:    [[VEC_RET_0:%.*]] = load <4 x i32>, ptr [[VEC_RETVAL_GEP_0]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_0:%.*]] = insertvalue { <4 x i32>, <4 x i32> } poison, <4 x i32> [[VEC_RET_0]], 0
; CHECK-NEXT:    [[VEC_RETVAL_GEP_1:%.*]] = getelementptr inbounds <4 x i32>, ptr [[VEC_RETVAL]], i32 1
; CHECK-NEXT:    [[VEC_RET_1:%.*]] = load <4 x i32>, ptr [[VEC_RETVAL_GEP_1]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_1:%.*]] = insertvalue { <4 x i32>, <4 x i32> } [[VEC_RETVAL_INS_0]], <4 x i32> [[VEC_RET_1]], 1
; CHECK-NEXT:    ret { <4 x i32>, <4 x i32> } [[VEC_RETVAL_INS_1]]
;
  %fst.p = getelementptr inbounds %struct.pair, ptr %x, i32 0, i32 0
  %snd.p = getelementptr inbounds %struct.pair, ptr %x, i32 0, i32 1
  %fst = load i32, ptr %fst.p, align 4
  %snd = load i32, ptr %snd.p, align 4
  %sum = add i32 %fst, %snd
  ret i32 %sum
}

attributes #0 = { nounwind uwtable "vector-variants"="_ZGVxN8u_foo" }
