; CMPLRLLVM-30376 & CMPLRLLVM-30436: This test verifies that Andersens
; analysis shouldn't crash due to collapsing nodes in mapping (offline
; cycles) data that is collected by HCD.

; RUN: opt < %s  -passes='require<anders-aa>' -disable-output 2>/dev/null

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%class.ExecuteOnStartup = type { ptr, ptr }
%"struct.std::vector<PatternMatcher::Elem>::_Temporary_value" = type { ptr, %"union.std::aligned_storage<88, 8>::type" }
%"union.std::aligned_storage<88, 8>::type" = type { [88 x i8] }
%class.PatternMatcher = type { %"class.std::vector.76", i8, %"class.std::__cxx11::basic_string" }
%"class.std::vector.76" = type { %"struct.std::_Vector_base.75" }
%"struct.std::_Vector_base.75" = type { %"struct.std::_Vector_base<PatternMatcher::Elem, std::allocator<PatternMatcher::Elem>>::_Vector_impl" }
%"struct.std::_Vector_base<PatternMatcher::Elem, std::allocator<PatternMatcher::Elem>>::_Vector_impl" = type { %"struct.std::_Vector_base<PatternMatcher::Elem, std::allocator<PatternMatcher::Elem>>::_Vector_impl_data" }
%"struct.std::_Vector_base<PatternMatcher::Elem, std::allocator<PatternMatcher::Elem>>::_Vector_impl_data" = type { ptr, ptr, ptr }
%"class.std::__cxx11::basic_string" = type { %"struct.std::__cxx11::basic_string<char>::_Alloc_hider", i64, %union.anon }
%"struct.std::__cxx11::basic_string<char>::_Alloc_hider" = type { ptr }
%union.anon = type { i64, [8 x i8] }
%"struct.PatternMatcher::Elem" = type { i32, %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string", i64, i64 }

@llvm.global_ctors = external global [123 x { i32, ptr, ptr }]
@_ZTS24cFileOutputScalarManager = external dso_local constant [27 x i8], align 1
@_ZTS20cOutputScalarManager = external dso_local constant [23 x i8], align 1
@_ZTI20cOutputScalarManager = external dso_local constant { ptr, ptr, ptr }, align 8
@_ZTI24cFileOutputScalarManager = external dso_local constant { ptr, ptr, ptr }, align 8
@_ZStL8__ioinit = external dso_local global %"class.std::ios_base::Init", align 1
@CFGID_OUTPUT_SCALAR_FILE = external dso_local unnamed_addr global ptr, align 8
@_ZN12_GLOBAL__N_118__onstartup_obj_42E = external dso_local global %class.ExecuteOnStartup, align 8
@.str = external dso_local unnamed_addr constant [19 x i8], align 1
@.str.2 = external dso_local unnamed_addr constant [44 x i8], align 1
@.str.5 = external dso_local unnamed_addr constant [24 x i8], align 1

declare ptr @_Znwm(i64) local_unnamed_addr

declare void @_ZdlPv() local_unnamed_addr

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.value(metadata, metadata, metadata) #0

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

declare void @__cxa_throw() local_unnamed_addr

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.label(metadata) #0

define internal fastcc void @_Z17expressionyyerrorPKc() unnamed_addr personality ptr undef {
bb:
  ret void
}

define internal void @_ZN17opp_runtime_errorD2Ev() unnamed_addr align 2 personality ptr undef {
bb:
  ret void
}

define internal void @_ZN17opp_runtime_errorD0Ev() unnamed_addr align 2 personality ptr undef {
bb:
  ret void
}

define internal ptr @_ZNK17opp_runtime_error4whatEv() unnamed_addr align 2 personality ptr undef {
bb:
  ret ptr undef
}

define internal fastcc void @_ZN14PatternMatcher10setPatternEPKcbbb(ptr %arg, ptr %arg1, i1 %arg2, i1 %arg3, i1 %arg4) unnamed_addr align 2 personality ptr undef {
bb:
  %i = alloca %"struct.std::vector<PatternMatcher::Elem>::_Temporary_value", align 8
  %i5 = getelementptr inbounds %class.PatternMatcher, ptr %arg, i64 0, i32 0
  %i6 = getelementptr inbounds %class.PatternMatcher, ptr %arg, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %i7 = load ptr, ptr %i6, align 8
  %i8 = getelementptr inbounds %class.PatternMatcher, ptr %arg, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %i9 = ptrtoint ptr %i7 to i64
  ret void

bb10:                                             ; No predecessors!
  store ptr %i7, ptr %i8, align 8
  ret void

bb11:                                             ; No predecessors!
  %i12 = getelementptr inbounds %"struct.std::vector<PatternMatcher::Elem>::_Temporary_value", ptr %i, i64 0, i32 0
  store ptr %i5, ptr %i12, align 8
  br label %bb13

bb13:                                             ; preds = %bb11
  %i14 = load ptr, ptr %i8, align 8
  %i15 = getelementptr inbounds %"struct.PatternMatcher::Elem", ptr %i14, i64 0, i32 1
  %i16 = getelementptr inbounds %"struct.PatternMatcher::Elem", ptr %i14, i64 -1, i32 1
  %i17 = getelementptr inbounds %"struct.PatternMatcher::Elem", ptr %i14, i64 0, i32 1, i32 2
  %i18 = bitcast ptr %i15 to ptr
  store ptr %i17, ptr %i18, align 8
  %i19 = getelementptr inbounds %"class.std::__cxx11::basic_string", ptr %i16, i64 0, i32 0, i32 0
  %i20 = load ptr, ptr %i19, align 8
  br i1 undef, label %bb21, label %bb23

bb21:                                             ; preds = %bb13
  %i22 = bitcast ptr %i17 to ptr
  call void @llvm.memcpy.p0.p0.i64(ptr %i22, ptr %i20, i64 16, i1 false)
  ret void

bb23:                                             ; preds = %bb13
  %i24 = getelementptr inbounds %"class.std::__cxx11::basic_string", ptr %i15, i64 0, i32 0, i32 0
  store ptr %i20, ptr %i24, align 8
  ret void

bb25:                                             ; No predecessors!
  %i26 = load ptr, ptr %i8, align 8
  %i27 = getelementptr inbounds %"struct.PatternMatcher::Elem", ptr %i26, i64 1
  store ptr %i27, ptr %i8, align 8
  %i28 = getelementptr inbounds %"struct.PatternMatcher::Elem", ptr %i26, i64 -1
  br i1 undef, label %bb29, label %bb46

bb29:                                             ; preds = %bb25
  %i30 = getelementptr inbounds %"struct.PatternMatcher::Elem", ptr %i28, i64 -1, i32 1
  %i31 = getelementptr inbounds %"struct.PatternMatcher::Elem", ptr %i26, i64 -1, i32 1, i32 0, i32 0
  %i32 = load ptr, ptr %i31, align 8
  %i33 = getelementptr inbounds %"class.std::__cxx11::basic_string", ptr %i30, i64 0, i32 0, i32 0
  %i34 = load ptr, ptr %i33, align 8
  ret void

bb35:                                             ; No predecessors!
  call void @llvm.memcpy.p0.p0.i64(ptr %i32, ptr %i34, i64 undef, i1 false)
  ret void

bb36:                                             ; No predecessors!
  store ptr %i32, ptr %i33, align 8
  br label %bb37

bb37:                                             ; preds = %bb36
  %i38 = getelementptr inbounds %"struct.PatternMatcher::Elem", ptr %i28, i64 -1, i32 2
  %i39 = getelementptr inbounds %"struct.PatternMatcher::Elem", ptr %i26, i64 -1, i32 2, i32 0, i32 0
  %i40 = load ptr, ptr %i39, align 8
  %i41 = getelementptr inbounds %"class.std::__cxx11::basic_string", ptr %i38, i64 0, i32 0, i32 0
  %i42 = load ptr, ptr %i41, align 8
  br label %bb44

bb43:                                             ; No predecessors!
  call void @llvm.memcpy.p0.p0.i64(ptr %i40, ptr %i42, i64 undef, i1 false)
  ret void

bb44:                                             ; preds = %bb37
  br label %bb45

bb45:                                             ; preds = %bb44
  store ptr %i40, ptr %i41, align 8
  ret void

bb46:                                             ; preds = %bb25
  %i47 = getelementptr inbounds %"struct.std::vector<PatternMatcher::Elem>::_Temporary_value", ptr %i, i64 0, i32 1, i32 0, i64 8
  %i48 = getelementptr inbounds %"struct.PatternMatcher::Elem", ptr undef, i64 0, i32 1, i32 0, i32 0
  %i49 = load ptr, ptr %i48, align 8
  %i50 = bitcast ptr %i47 to ptr
  %i51 = load ptr, ptr %i50, align 8
  %i52 = getelementptr inbounds %"struct.std::vector<PatternMatcher::Elem>::_Temporary_value", ptr %i, i64 0, i32 1, i32 0, i64 24
  br label %bb54

bb53:                                             ; No predecessors!
  call void @llvm.memcpy.p0.p0.i64(ptr %i49, ptr %i51, i64 undef, i1 false)
  ret void

bb54:                                             ; preds = %bb46
  store ptr %i51, ptr %i48, align 8
  br i1 undef, label %bb56, label %bb55

bb55:                                             ; preds = %bb54
  store ptr %i49, ptr %i50, align 8
  ret void

bb56:                                             ; preds = %bb54
  %i57 = bitcast ptr %i47 to ptr
  store ptr %i52, ptr %i57, align 8
  ret void
}

define internal fastcc void @_ZN15MatchExpressionC2EPKcbbb() unnamed_addr align 2 personality ptr undef {
bb:
  %i = invoke ptr @_Znwm(i64 64)
          to label %bb1 unwind label %bb4

bb1:                                              ; preds = %bb
  %i2 = bitcast ptr %i to ptr
  invoke fastcc void @_ZN14PatternMatcher10setPatternEPKcbbb(ptr %i2, ptr undef, i1 false, i1 true, i1 true)
          to label %bb3 unwind label %bb4

bb3:                                              ; preds = %bb1
  ret void

bb4:                                              ; preds = %bb1, %bb
  %i5 = landingpad { ptr, i32 }
          cleanup
  ret void
}

define internal void @_ZN13cClassFactoryD2Ev() unnamed_addr align 2 personality ptr undef {
bb:
  ret void
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { nocallback nofree nounwind willreturn memory(argmem: readwrite) }
