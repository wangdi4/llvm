; RUN: opt -passes=vec-clone -mtriple=x86_64 -vec-clone-legalize-enabled -S < %s  | FileCheck %s

; Return: i32
; Arguments: float, float
; ISA class: YMM1, YMM2
; "vector-variants"="_ZGVyM8vv_func,_ZGVYM8vv_func"
;
; YMM1: both mask and return value need to be legalized, i.e. need two registers to pass <8 x i32> logical type.
; YMM2: legalization not required since all the logical arguments fit nicely physical registers.

define dso_local i32 @func(float noundef nofpclass(nan inf) %x, float noundef nofpclass(nan inf) %y) local_unnamed_addr #0 {
; CHECK:  define dso_local x86_regcallcc { <4 x i32>, <4 x i32> } @_ZGVyM8vv_func(<8 x float> noundef nofpclass(nan inf) [[X0:%.*]], <8 x float> noundef nofpclass(nan inf) [[Y0:%.*]], <4 x i32> [[MASK_00:%.*]], <4 x i32> [[MASK_10:%.*]]) local_unnamed_addr #{{.*}} {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VEC_X0:%.*]] = alloca <8 x float>, align 32
; CHECK-NEXT:    [[VEC_Y0:%.*]] = alloca <8 x float>, align 32
; CHECK-NEXT:    [[VEC_MASK0:%.*]] = alloca <8 x i32>, align 32
; CHECK-NEXT:    [[VEC_RETVAL0:%.*]] = alloca <8 x i32>, align 32
; CHECK-NEXT:    store <8 x float> [[X0]], ptr [[VEC_X0]], align 32
; CHECK-NEXT:    store <8 x float> [[Y0]], ptr [[VEC_Y0]], align 32
; CHECK-NEXT:    [[VEC_MASK_GEP_00:%.*]] = getelementptr inbounds <4 x i32>, ptr [[VEC_MASK0]], i32 0
; CHECK-NEXT:    store <4 x i32> [[MASK_00]], ptr [[VEC_MASK_GEP_00]], align 16
; CHECK-NEXT:    [[VEC_MASK_GEP_10:%.*]] = getelementptr inbounds <4 x i32>, ptr [[VEC_MASK0]], i32 1
; CHECK-NEXT:    store <4 x i32> [[MASK_10]], ptr [[VEC_MASK_GEP_10]], align 16
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION0:%.*]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RETVAL_GEP_00:%.*]] = getelementptr inbounds <4 x i32>, ptr [[VEC_RETVAL0]], i32 0
; CHECK-NEXT:    [[VEC_RET_00:%.*]] = load <4 x i32>, ptr [[VEC_RETVAL_GEP_00]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_00:%.*]] = insertvalue { <4 x i32>, <4 x i32> } poison, <4 x i32> [[VEC_RET_00]], 0
; CHECK-NEXT:    [[VEC_RETVAL_GEP_10:%.*]] = getelementptr inbounds <4 x i32>, ptr [[VEC_RETVAL0]], i32 1
; CHECK-NEXT:    [[VEC_RET_10:%.*]] = load <4 x i32>, ptr [[VEC_RETVAL_GEP_10]], align 16
; CHECK-NEXT:    [[VEC_RETVAL_INS_10:%.*]] = insertvalue { <4 x i32>, <4 x i32> } [[VEC_RETVAL_INS_00]], <4 x i32> [[VEC_RET_10]], 1
; CHECK-NEXT:    ret { <4 x i32>, <4 x i32> } [[VEC_RETVAL_INS_10]]
; CHECK-NEXT:  }
;
; CHECK:  define dso_local x86_regcallcc <8 x i32> @_ZGVYM8vv_func(<8 x float> noundef nofpclass(nan inf) [[X:%.*]], <8 x float> noundef nofpclass(nan inf) [[Y:%.*]], <8 x i32> [[MASK:%.*]]) local_unnamed_addr #{{.*}} {
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[VEC_X:%.*]] = alloca <8 x float>, align 32
; CHECK-NEXT:    [[VEC_Y:%.*]] = alloca <8 x float>, align 32
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <8 x i32>, align 32
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <8 x i32>, align 32
; CHECK-NEXT:    store <8 x float> [[X]], ptr [[VEC_X]], align 32
; CHECK-NEXT:    store <8 x float> [[Y]], ptr [[VEC_Y]], align 32
; CHECK-NEXT:    store <8 x i32> [[MASK]], ptr [[VEC_MASK]], align 32
; CHECK-NEXT:    br label [[SIMD_BEGIN_REGION:%.*]]
;    ...skip...
; CHECK:       return:
; CHECK-NEXT:    [[VEC_RET:%.*]] = load <8 x i32>, ptr [[VEC_RETVAL]], align 32
; CHECK-NEXT:    ret <8 x i32> [[VEC_RET]]
; CHECK-NEXT:  }
;
entry:
  %add = fadd fast float %x, %y
  %res = fptosi float %add to i32
  ret i32 %res
}

attributes #0 = { mustprogress nofree noinline norecurse nosync nounwind willreturn memory(none) uwtable "vector-variants"="_ZGVyM8vv_func,_ZGVYM8vv_func" }
