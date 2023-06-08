; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

; Test for vector variant arguments and return type legalization as per VFABI
; Return: double
; Arguments: double
; ISA class: YMM2
; "vector-variants"="_ZGVYN8v_foo,_ZGVYM8v_foo"

define dso_local nofpclass(nan inf) double @foo(double noundef nofpclass(nan inf) %x) local_unnamed_addr #0 {
; CHECK:  define dso_local x86_regcallcc { <4 x double>, <4 x double> } @_ZGVYN8v_foo(<4 x double> noundef nofpclass(nan inf) [[X_00:%.*]], <4 x double> noundef nofpclass(nan inf) [[X_10:%.*]]) local_unnamed_addr #1 {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VEC_X0:%.*]] = alloca <8 x double>, align 64
; CHECK-NEXT:    [[VEC_RETVAL0:%.*]] = alloca <8 x double>, align 64
; CHECK-NEXT:    [[VEC_X_GEP_00:%.*]] = getelementptr inbounds <4 x double>, ptr [[VEC_X0]], i32 0
; CHECK-NEXT:    store <4 x double> [[X_00]], ptr [[VEC_X_GEP_00]], align 32
; CHECK-NEXT:    [[VEC_X_GEP_10:%.*]] = getelementptr inbounds <4 x double>, ptr [[VEC_X0]], i32 1
; CHECK-NEXT:    store <4 x double> [[X_10]], ptr [[VEC_X_GEP_10]], align 32
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION0:%.*]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RETVAL_GEP_00:%.*]] = getelementptr inbounds <4 x double>, ptr [[VEC_RETVAL0]], i32 0
; CHECK-NEXT:    [[VEC_RET_00:%.*]] = load <4 x double>, ptr [[VEC_RETVAL_GEP_00]], align 32
; CHECK-NEXT:    [[VEC_RETVAL_INS_00:%.*]] = insertvalue { <4 x double>, <4 x double> } poison, <4 x double> [[VEC_RET_00]], 0
; CHECK-NEXT:    [[VEC_RETVAL_GEP_10:%.*]] = getelementptr inbounds <4 x double>, ptr [[VEC_RETVAL0]], i32 1
; CHECK-NEXT:    [[VEC_RET_10:%.*]] = load <4 x double>, ptr [[VEC_RETVAL_GEP_10]], align 32
; CHECK-NEXT:    [[VEC_RETVAL_INS_10:%.*]] = insertvalue { <4 x double>, <4 x double> } [[VEC_RETVAL_INS_00]], <4 x double> [[VEC_RET_10]], 1
; CHECK-NEXT:    ret { <4 x double>, <4 x double> } [[VEC_RETVAL_INS_10]]
; CHECK-NEXT:  }
;
; CHECK:  define dso_local x86_regcallcc { <4 x double>, <4 x double> } @_ZGVYM8v_foo(<4 x double> noundef nofpclass(nan inf) [[X_00]], <4 x double> noundef nofpclass(nan inf) [[X_10]], <4 x double> [[MASK_00:%.*]], <4 x double> [[MASK_10:%.*]]) local_unnamed_addr #1 {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VEC_X0]] = alloca <8 x double>, align 64
; CHECK-NEXT:    [[VEC_MASK0:%.*]] = alloca <8 x double>, align 64
; CHECK-NEXT:    [[VEC_RETVAL0]] = alloca <8 x double>, align 64
; CHECK-NEXT:    [[VEC_X_GEP_00]] = getelementptr inbounds <4 x double>, ptr [[VEC_X0]], i32 0
; CHECK-NEXT:    store <4 x double> [[X_00]], ptr [[VEC_X_GEP_00]], align 32
; CHECK-NEXT:    [[VEC_X_GEP_10]] = getelementptr inbounds <4 x double>, ptr [[VEC_X0]], i32 1
; CHECK-NEXT:    store <4 x double> [[X_10]], ptr [[VEC_X_GEP_10]], align 32
; CHECK-NEXT:    [[VEC_MASK_GEP_00:%.*]] = getelementptr inbounds <4 x double>, ptr [[VEC_MASK0]], i32 0
; CHECK-NEXT:    store <4 x double> [[MASK_00]], ptr [[VEC_MASK_GEP_00]], align 32
; CHECK-NEXT:    [[VEC_MASK_GEP_10:%.*]] = getelementptr inbounds <4 x double>, ptr [[VEC_MASK0]], i32 1
; CHECK-NEXT:    store <4 x double> [[MASK_10]], ptr [[VEC_MASK_GEP_10]], align 32
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION0]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RETVAL_GEP_00]] = getelementptr inbounds <4 x double>, ptr [[VEC_RETVAL0]], i32 0
; CHECK-NEXT:    [[VEC_RET_00]] = load <4 x double>, ptr [[VEC_RETVAL_GEP_00]], align 32
; CHECK-NEXT:    [[VEC_RETVAL_INS_00]] = insertvalue { <4 x double>, <4 x double> } poison, <4 x double> [[VEC_RET_00]], 0
; CHECK-NEXT:    [[VEC_RETVAL_GEP_10]] = getelementptr inbounds <4 x double>, ptr [[VEC_RETVAL0]], i32 1
; CHECK-NEXT:    [[VEC_RET_10]] = load <4 x double>, ptr [[VEC_RETVAL_GEP_10]], align 32
; CHECK-NEXT:    [[VEC_RETVAL_INS_10]] = insertvalue { <4 x double>, <4 x double> } [[VEC_RETVAL_INS_00]], <4 x double> [[VEC_RET_10]], 1
; CHECK-NEXT:    ret { <4 x double>, <4 x double> } [[VEC_RETVAL_INS_10]]
; CHECK-NEXT:  }
;
entry:
  %mul = fmul fast double 512.0, %x
  ret double %mul
}

attributes #0 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(none) uwtable "vector-variants"="_ZGVYN8v_foo,_ZGVYM8v_foo" }
