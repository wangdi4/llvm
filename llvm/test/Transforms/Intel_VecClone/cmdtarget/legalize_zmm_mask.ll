; RUN: opt -mtriple=x86_64 -mcpu=skylake-avx512 -passes=vec-clone -S < %s  | FileCheck %s

define double @func0(double %v) #0 {
; CHECK-LABEL: define x86_regcallcc { <8 x double>, <8 x double> } @_ZGVZM16v_func0
; CHECK-SAME: (<8 x double> [[V_0:%.*]], <8 x double> [[V_1:%.*]], i32 [[MASK_0:%.*]], i32 [[MASK_1:%.*]]) #[[ATTR0:[0-9]+]] {
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <16 x double>, align 128
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <16 x double>, align 128
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <16 x double>, align 128
; CHECK-NEXT:    [[VEC_V_GEP_0:%.*]] = getelementptr inbounds <8 x double>, ptr [[VEC_V]], i32 0
; CHECK-NEXT:    store <8 x double> [[V_0]], ptr [[VEC_V_GEP_0]], align 64
; CHECK-NEXT:    [[VEC_V_GEP_1:%.*]] = getelementptr inbounds <8 x double>, ptr [[VEC_V]], i32 1
; CHECK-NEXT:    store <8 x double> [[V_1]], ptr [[VEC_V_GEP_1]], align 64
; CHECK-NEXT:    [[MASK_0_TRUNC:%.*]] = trunc i32 [[MASK_0]] to i8
; CHECK-NEXT:    [[MASK_0_VEC:%.*]] = bitcast i8 [[MASK_0_TRUNC]] to <8 x i1>
; CHECK-NEXT:    [[MASK_0_VEC_SEXT:%.*]] = sext <8 x i1> [[MASK_0_VEC]] to <8 x i64>
; CHECK-NEXT:    [[MASK_0_VEC_CAST:%.*]] = bitcast <8 x i64> [[MASK_0_VEC_SEXT]] to <8 x double>
; CHECK-NEXT:    [[VEC_MASK_GEP_0:%.*]] = getelementptr inbounds <8 x double>, ptr [[VEC_MASK]], i32 0
; CHECK-NEXT:    store <8 x double> [[MASK_0_VEC_CAST]], ptr [[VEC_MASK_GEP_0]], align 64
; CHECK-NEXT:    [[MASK_1_TRUNC:%.*]] = trunc i32 [[MASK_1]] to i8
; CHECK-NEXT:    [[MASK_1_VEC:%.*]] = bitcast i8 [[MASK_1_TRUNC]] to <8 x i1>
; CHECK-NEXT:    [[MASK_1_VEC_SEXT:%.*]] = sext <8 x i1> [[MASK_1_VEC]] to <8 x i64>
; CHECK-NEXT:    [[MASK_1_VEC_CAST:%.*]] = bitcast <8 x i64> [[MASK_1_VEC_SEXT]] to <8 x double>
; CHECK-NEXT:    [[VEC_MASK_GEP_1:%.*]] = getelementptr inbounds <8 x double>, ptr [[VEC_MASK]], i32 1
; CHECK-NEXT:    store <8 x double> [[MASK_1_VEC_CAST]], ptr [[VEC_MASK_GEP_1]], align 64
;
; CHECK-LABEL: define x86_regcallcc <8 x double> @_ZGVZM8v_func0
; CHECK-SAME: (<8 x double> [[V:%.*]], i32 [[MASK:%.*]]) #[[ATTR0]] {
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <8 x double>, align 64
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <8 x double>, align 64
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <8 x double>, align 64
; CHECK-NEXT:    store <8 x double> [[V]], ptr [[VEC_V]], align 64
; CHECK-NEXT:    [[MASK_TRUNC:%.*]] = trunc i32 [[MASK]] to i8
; CHECK-NEXT:    [[MASK_VEC:%.*]] = bitcast i8 [[MASK_TRUNC]] to <8 x i1>
; CHECK-NEXT:    [[MASK_VEC_SEXT:%.*]] = sext <8 x i1> [[MASK_VEC]] to <8 x i64>
; CHECK-NEXT:    [[MASK_VEC_CAST:%.*]] = bitcast <8 x i64> [[MASK_VEC_SEXT]] to <8 x double>
; CHECK-NEXT:    store <8 x double> [[MASK_VEC_CAST]], ptr [[VEC_MASK]], align 64
;
  %add = fadd fast double %v, %v
  ret double %add
}


define i64 @func1(i64 %v) #1 {
; CHECK-LABEL: define x86_regcallcc { <8 x i64>, <8 x i64> } @_ZGVZM16v_func1
; CHECK-SAME: (<8 x i64> [[V_0:%.*]], <8 x i64> [[V_1:%.*]], i32 [[MASK_0:%.*]], i32 [[MASK_1:%.*]]) #[[ATTR0]] {
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <16 x i64>, align 128
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <16 x i64>, align 128
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <16 x i64>, align 128
; CHECK-NEXT:    [[VEC_V_GEP_0:%.*]] = getelementptr inbounds <8 x i64>, ptr [[VEC_V]], i32 0
; CHECK-NEXT:    store <8 x i64> [[V_0]], ptr [[VEC_V_GEP_0]], align 64
; CHECK-NEXT:    [[VEC_V_GEP_1:%.*]] = getelementptr inbounds <8 x i64>, ptr [[VEC_V]], i32 1
; CHECK-NEXT:    store <8 x i64> [[V_1]], ptr [[VEC_V_GEP_1]], align 64
; CHECK-NEXT:    [[MASK_0_TRUNC:%.*]] = trunc i32 [[MASK_0]] to i8
; CHECK-NEXT:    [[MASK_0_VEC:%.*]] = bitcast i8 [[MASK_0_TRUNC]] to <8 x i1>
; CHECK-NEXT:    [[MASK_0_VEC_SEXT:%.*]] = sext <8 x i1> [[MASK_0_VEC]] to <8 x i64>
; CHECK-NEXT:    [[VEC_MASK_GEP_0:%.*]] = getelementptr inbounds <8 x i64>, ptr [[VEC_MASK]], i32 0
; CHECK-NEXT:    store <8 x i64> [[MASK_0_VEC_SEXT]], ptr [[VEC_MASK_GEP_0]], align 64
; CHECK-NEXT:    [[MASK_1_TRUNC:%.*]] = trunc i32 [[MASK_1]] to i8
; CHECK-NEXT:    [[MASK_1_VEC:%.*]] = bitcast i8 [[MASK_1_TRUNC]] to <8 x i1>
; CHECK-NEXT:    [[MASK_1_VEC_SEXT:%.*]] = sext <8 x i1> [[MASK_1_VEC]] to <8 x i64>
; CHECK-NEXT:    [[VEC_MASK_GEP_1:%.*]] = getelementptr inbounds <8 x i64>, ptr [[VEC_MASK]], i32 1
; CHECK-NEXT:    store <8 x i64> [[MASK_1_VEC_SEXT]], ptr [[VEC_MASK_GEP_1]], align 64
;
; CHECK-LABEL: define x86_regcallcc <8 x i64> @_ZGVZM8v_func1
; CHECK-SAME: (<8 x i64> [[V:%.*]], i32 [[MASK:%.*]]) #[[ATTR0]] {
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <8 x i64>, align 64
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <8 x i64>, align 64
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <8 x i64>, align 64
; CHECK-NEXT:    store <8 x i64> [[V]], ptr [[VEC_V]], align 64
; CHECK-NEXT:    [[MASK_TRUNC:%.*]] = trunc i32 [[MASK]] to i8
; CHECK-NEXT:    [[MASK_VEC:%.*]] = bitcast i8 [[MASK_TRUNC]] to <8 x i1>
; CHECK-NEXT:    [[MASK_VEC_SEXT:%.*]] = sext <8 x i1> [[MASK_VEC]] to <8 x i64>
; CHECK-NEXT:    store <8 x i64> [[MASK_VEC_SEXT]], ptr [[VEC_MASK]], align 64
;
  %add = add i64 %v, %v
  ret i64 %add
}

define i32 @func2(i32 %v) #2 {
; CHECK-LABEL: define x86_regcallcc { <16 x i32>, <16 x i32> } @_ZGVZM32v_func2
; CHECK-SAME: (<16 x i32> [[V_0:%.*]], <16 x i32> [[V_1:%.*]], i32 [[MASK_0:%.*]], i32 [[MASK_1:%.*]]) #[[ATTR0]] {
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <32 x i32>, align 128
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <32 x i32>, align 128
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <32 x i32>, align 128
; CHECK-NEXT:    [[VEC_V_GEP_0:%.*]] = getelementptr inbounds <16 x i32>, ptr [[VEC_V]], i32 0
; CHECK-NEXT:    store <16 x i32> [[V_0]], ptr [[VEC_V_GEP_0]], align 64
; CHECK-NEXT:    [[VEC_V_GEP_1:%.*]] = getelementptr inbounds <16 x i32>, ptr [[VEC_V]], i32 1
; CHECK-NEXT:    store <16 x i32> [[V_1]], ptr [[VEC_V_GEP_1]], align 64
; CHECK-NEXT:    [[MASK_0_TRUNC:%.*]] = trunc i32 [[MASK_0]] to i16
; CHECK-NEXT:    [[MASK_0_VEC:%.*]] = bitcast i16 [[MASK_0_TRUNC]] to <16 x i1>
; CHECK-NEXT:    [[MASK_0_VEC_SEXT:%.*]] = sext <16 x i1> [[MASK_0_VEC]] to <16 x i32>
; CHECK-NEXT:    [[VEC_MASK_GEP_0:%.*]] = getelementptr inbounds <16 x i32>, ptr [[VEC_MASK]], i32 0
; CHECK-NEXT:    store <16 x i32> [[MASK_0_VEC_SEXT]], ptr [[VEC_MASK_GEP_0]], align 64
; CHECK-NEXT:    [[MASK_1_TRUNC:%.*]] = trunc i32 [[MASK_1]] to i16
; CHECK-NEXT:    [[MASK_1_VEC:%.*]] = bitcast i16 [[MASK_1_TRUNC]] to <16 x i1>
; CHECK-NEXT:    [[MASK_1_VEC_SEXT:%.*]] = sext <16 x i1> [[MASK_1_VEC]] to <16 x i32>
; CHECK-NEXT:    [[VEC_MASK_GEP_1:%.*]] = getelementptr inbounds <16 x i32>, ptr [[VEC_MASK]], i32 1
; CHECK-NEXT:    store <16 x i32> [[MASK_1_VEC_SEXT]], ptr [[VEC_MASK_GEP_1]], align 64
;
 %add = add i32 %v, %v
  ret i32 %add
}

define i8 @func3(i8 %v) #3 {
; CHECK-LABEL: define x86_regcallcc <64 x i8> @_ZGVZM64v_func3
; CHECK-SAME: (<64 x i8> [[V:%.*]], i64 [[MASK:%.*]]) #[[ATTR0]] {
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <64 x i8>, align 64
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <64 x i8>, align 64
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <64 x i8>, align 64
; CHECK-NEXT:    store <64 x i8> [[V]], ptr [[VEC_V]], align 64
; CHECK-NEXT:    [[MASK_VEC:%.*]] = bitcast i64 [[MASK]] to <64 x i1>
; CHECK-NEXT:    [[MASK_VEC_SEXT:%.*]] = sext <64 x i1> [[MASK_VEC]] to <64 x i8>
; CHECK-NEXT:    store <64 x i8> [[MASK_VEC_SEXT]], ptr [[VEC_MASK]], align 64
;
; CHECK-LABEL: define x86_regcallcc { <64 x i8>, <64 x i8> } @_ZGVZM128v_func3
; CHECK-SAME: (<64 x i8> [[V_0:%.*]], <64 x i8> [[V_1:%.*]], i64 [[MASK_0:%.*]], i64 [[MASK_1:%.*]]) #[[ATTR0]] {
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <128 x i8>, align 128
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <128 x i8>, align 128
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <128 x i8>, align 128
; CHECK-NEXT:    [[VEC_V_GEP_0:%.*]] = getelementptr inbounds <64 x i8>, ptr [[VEC_V]], i32 0
; CHECK-NEXT:    store <64 x i8> [[V_0]], ptr [[VEC_V_GEP_0]], align 64
; CHECK-NEXT:    [[VEC_V_GEP_1:%.*]] = getelementptr inbounds <64 x i8>, ptr [[VEC_V]], i32 1
; CHECK-NEXT:    store <64 x i8> [[V_1]], ptr [[VEC_V_GEP_1]], align 64
; CHECK-NEXT:    [[MASK_0_VEC:%.*]] = bitcast i64 [[MASK_0]] to <64 x i1>
; CHECK-NEXT:    [[MASK_0_VEC_SEXT:%.*]] = sext <64 x i1> [[MASK_0_VEC]] to <64 x i8>
; CHECK-NEXT:    [[VEC_MASK_GEP_0:%.*]] = getelementptr inbounds <64 x i8>, ptr [[VEC_MASK]], i32 0
; CHECK-NEXT:    store <64 x i8> [[MASK_0_VEC_SEXT]], ptr [[VEC_MASK_GEP_0]], align 64
; CHECK-NEXT:    [[MASK_1_VEC:%.*]] = bitcast i64 [[MASK_1]] to <64 x i1>
; CHECK-NEXT:    [[MASK_1_VEC_SEXT:%.*]] = sext <64 x i1> [[MASK_1_VEC]] to <64 x i8>
; CHECK-NEXT:    [[VEC_MASK_GEP_1:%.*]] = getelementptr inbounds <64 x i8>, ptr [[VEC_MASK]], i32 1
; CHECK-NEXT:    store <64 x i8> [[MASK_1_VEC_SEXT]], ptr [[VEC_MASK_GEP_1]], align 64
;
  %add = add i8 %v, %v
  ret i8 %add
}

define i16 @func4(i16 %v) #4 {
; CHECK-LABEL: define x86_regcallcc <32 x i16> @_ZGVZM32v_func4
; CHECK-SAME: (<32 x i16> [[V:%.*]], i32 [[MASK:%.*]]) #[[ATTR0]] {
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <32 x i16>, align 64
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <32 x i16>, align 64
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <32 x i16>, align 64
; CHECK-NEXT:    store <32 x i16> [[V]], ptr [[VEC_V]], align 64
; CHECK-NEXT:    [[MASK_VEC:%.*]] = bitcast i32 [[MASK]] to <32 x i1>
; CHECK-NEXT:    [[MASK_VEC_SEXT:%.*]] = sext <32 x i1> [[MASK_VEC]] to <32 x i16>
; CHECK-NEXT:    store <32 x i16> [[MASK_VEC_SEXT]], ptr [[VEC_MASK]], align 64
;
 %add = add i16 %v, %v
  ret i16 %add
}

define i1 @func5(i1 %v) #5 {
; CHECK-LABEL: define x86_regcallcc <16 x i8> @_ZGVZM16v_func5
; CHECK-SAME: (<16 x i8> [[V:%.*]], i32 [[MASK:%.*]]) #[[ATTR0]] {
; CHECK-NEXT:    [[VEC_V:%.*]] = alloca <16 x i8>, align 16
; CHECK-NEXT:    [[VEC_MASK:%.*]] = alloca <16 x i8>, align 16
; CHECK-NEXT:    [[VEC_RETVAL:%.*]] = alloca <16 x i8>, align 16
; CHECK-NEXT:    store <16 x i8> [[V]], ptr [[VEC_V]], align 16
; CHECK-NEXT:    [[MASK_TRUNC:%.*]] = trunc i32 [[MASK]] to i16
; CHECK-NEXT:    [[MASK_VEC:%.*]] = bitcast i16 [[MASK_TRUNC]] to <16 x i1>
; CHECK-NEXT:    [[MASK_VEC_SEXT:%.*]] = sext <16 x i1> [[MASK_VEC]] to <16 x i8>
; CHECK-NEXT:    store <16 x i8> [[MASK_VEC_SEXT]], ptr [[VEC_MASK]], align 16
;
  %add = and i1 %v, 1
  ret i1 %add
}

attributes #0 = { "vector-variants"="_ZGVZM16v_func0,_ZGVZM8v_func0" }
attributes #1 = { "vector-variants"="_ZGVZM16v_func1,_ZGVZM8v_func1" }
attributes #2 = { "vector-variants"="_ZGVZM32v_func2" }
attributes #3 = { "vector-variants"="_ZGVZM64v_func3,_ZGVZM128v_func3" }
attributes #4 = { "vector-variants"="_ZGVZM32v_func4" }
attributes #5 = { "vector-variants"="_ZGVZM16v_func5" }
