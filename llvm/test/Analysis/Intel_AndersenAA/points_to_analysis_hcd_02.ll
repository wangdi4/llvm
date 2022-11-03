; CMPLRLLVM-30376 & CMPLRLLVM-30436: This test verifies that Andersens
; analysis shouldn't crash due to collapsing nodes in mapping (offline
; cycles) data that is collected by HCD.

; RUN: opt < %s  -passes='require<anders-aa>' -disable-output 2>/dev/null

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%class.cConfigOption = type { %class.cNoncopyableOwnedObject.base, i8, i8, i32, %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string" }
%class.cNoncopyableOwnedObject.base = type { %class.cOwnedObject.base }
%class.cOwnedObject.base = type <{ %class.cNamedObject.base, [4 x i8], %class.cObject*, i32 }>
%class.cNamedObject.base = type <{ %class.cObject, i8*, i16, i16 }>
%class.cObject = type { i32 (...)** }
%"class.std::__cxx11::basic_string" = type { %"struct.std::__cxx11::basic_string<char>::_Alloc_hider", i64, %union.anon }
%"struct.std::__cxx11::basic_string<char>::_Alloc_hider" = type { i8* }
%union.anon = type { i64, [8 x i8] }
%class.ExecuteOnStartup = type { void ()*, %class.ExecuteOnStartup* }
%class.PatternMatcher = type { %"class.std::vector.76", i8, %"class.std::__cxx11::basic_string" }
%"class.std::vector.76" = type { %"struct.std::_Vector_base.75" }
%"struct.std::_Vector_base.75" = type { %"struct.std::_Vector_base<PatternMatcher::Elem, std::allocator<PatternMatcher::Elem>>::_Vector_impl" }
%"struct.std::_Vector_base<PatternMatcher::Elem, std::allocator<PatternMatcher::Elem>>::_Vector_impl" = type { %"struct.std::_Vector_base<PatternMatcher::Elem, std::allocator<PatternMatcher::Elem>>::_Vector_impl_data" }
%"struct.std::_Vector_base<PatternMatcher::Elem, std::allocator<PatternMatcher::Elem>>::_Vector_impl_data" = type { %"struct.PatternMatcher::Elem"*, %"struct.PatternMatcher::Elem"*, %"struct.PatternMatcher::Elem"* }
%"struct.PatternMatcher::Elem" = type { i32, %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string", i64, i64 }
%"struct.std::vector<PatternMatcher::Elem>::_Temporary_value" = type { %"class.std::vector.76"*, %"union.std::aligned_storage<88, 8>::type" }
%"union.std::aligned_storage<88, 8>::type" = type { [88 x i8] }

@llvm.global_ctors = external global [123 x { i32, void ()*, i8* }]
@_ZTS24cFileOutputScalarManager = external dso_local constant [27 x i8], align 1
@_ZTS20cOutputScalarManager = external dso_local constant [23 x i8], align 1
@_ZTI20cOutputScalarManager = external dso_local constant { i8*, i8*, i8* }, align 8
@_ZTI24cFileOutputScalarManager = external dso_local constant { i8*, i8*, i8* }, align 8
@_ZStL8__ioinit = external dso_local global %"class.std::ios_base::Init", align 1
@CFGID_OUTPUT_SCALAR_FILE = external dso_local unnamed_addr global %class.cConfigOption*, align 8
@_ZN12_GLOBAL__N_118__onstartup_obj_42E = external dso_local global %class.ExecuteOnStartup, align 8
@.str = external dso_local unnamed_addr constant [19 x i8], align 1
@.str.2 = external dso_local unnamed_addr constant [44 x i8], align 1
@.str.5 = external dso_local unnamed_addr constant [24 x i8], align 1

declare i8* @_Znwm(i64) local_unnamed_addr

declare void @_ZdlPv() local_unnamed_addr

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

declare void @__cxa_throw() local_unnamed_addr

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.label(metadata) #0

define internal fastcc void @_Z17expressionyyerrorPKc() unnamed_addr personality i8* undef {
  ret void
}

define internal void @_ZN17opp_runtime_errorD2Ev() unnamed_addr align 2 personality i32 (...)* undef {
  ret void
}

define internal void @_ZN17opp_runtime_errorD0Ev() unnamed_addr align 2 personality i32 (...)* undef {
  ret void
}

define internal i8* @_ZNK17opp_runtime_error4whatEv() unnamed_addr align 2 personality i32 (...)* undef {
  ret i8* undef
}

define internal fastcc void @_ZN14PatternMatcher10setPatternEPKcbbb(%class.PatternMatcher* %0, i8* %1, i1 %2, i1 %3, i1 %4) unnamed_addr align 2 personality i8* undef {
  %6 = alloca %"struct.std::vector<PatternMatcher::Elem>::_Temporary_value", align 8
  %7 = getelementptr inbounds %class.PatternMatcher, %class.PatternMatcher* %0, i64 0, i32 0
  %8 = getelementptr inbounds %class.PatternMatcher, %class.PatternMatcher* %0, i64 0, i32 0, i32 0, i32 0, i32 0, i32 0
  %9 = load %"struct.PatternMatcher::Elem"*, %"struct.PatternMatcher::Elem"** %8, align 8
  %10 = getelementptr inbounds %class.PatternMatcher, %class.PatternMatcher* %0, i64 0, i32 0, i32 0, i32 0, i32 0, i32 1
  %11 = ptrtoint %"struct.PatternMatcher::Elem"* %9 to i64
  ret void

12:                                               ; No predecessors!
  store %"struct.PatternMatcher::Elem"* %9, %"struct.PatternMatcher::Elem"** %10, align 8
  ret void

13:                                               ; No predecessors!
  %14 = getelementptr inbounds %"struct.std::vector<PatternMatcher::Elem>::_Temporary_value", %"struct.std::vector<PatternMatcher::Elem>::_Temporary_value"* %6, i64 0, i32 0
  store %"class.std::vector.76"* %7, %"class.std::vector.76"** %14, align 8
  br label %15

15:                                               ; preds = %13
  %16 = load %"struct.PatternMatcher::Elem"*, %"struct.PatternMatcher::Elem"** %10, align 8
  %17 = getelementptr inbounds %"struct.PatternMatcher::Elem", %"struct.PatternMatcher::Elem"* %16, i64 0, i32 1
  %18 = getelementptr inbounds %"struct.PatternMatcher::Elem", %"struct.PatternMatcher::Elem"* %16, i64 -1, i32 1
  %19 = getelementptr inbounds %"struct.PatternMatcher::Elem", %"struct.PatternMatcher::Elem"* %16, i64 0, i32 1, i32 2
  %20 = bitcast %"class.std::__cxx11::basic_string"* %17 to %union.anon**
  store %union.anon* %19, %union.anon** %20, align 8
  %21 = getelementptr inbounds %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string"* %18, i64 0, i32 0, i32 0
  %22 = load i8*, i8** %21, align 8
  br i1 undef, label %23, label %25

23:                                               ; preds = %15
  %24 = bitcast %union.anon* %19 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %24, i8* %22, i64 16, i1 false)
  ret void

25:                                               ; preds = %15
  %26 = getelementptr inbounds %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string"* %17, i64 0, i32 0, i32 0
  store i8* %22, i8** %26, align 8
  ret void

27:                                               ; No predecessors!
  %28 = load %"struct.PatternMatcher::Elem"*, %"struct.PatternMatcher::Elem"** %10, align 8
  %29 = getelementptr inbounds %"struct.PatternMatcher::Elem", %"struct.PatternMatcher::Elem"* %28, i64 1
  store %"struct.PatternMatcher::Elem"* %29, %"struct.PatternMatcher::Elem"** %10, align 8
  %30 = getelementptr inbounds %"struct.PatternMatcher::Elem", %"struct.PatternMatcher::Elem"* %28, i64 -1
  br i1 undef, label %31, label %48

31:                                               ; preds = %27
  %32 = getelementptr inbounds %"struct.PatternMatcher::Elem", %"struct.PatternMatcher::Elem"* %30, i64 -1, i32 1
  %33 = getelementptr inbounds %"struct.PatternMatcher::Elem", %"struct.PatternMatcher::Elem"* %28, i64 -1, i32 1, i32 0, i32 0
  %34 = load i8*, i8** %33, align 8
  %35 = getelementptr inbounds %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string"* %32, i64 0, i32 0, i32 0
  %36 = load i8*, i8** %35, align 8
  ret void

37:                                               ; No predecessors!
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %34, i8* %36, i64 undef, i1 false)
  ret void

38:                                               ; No predecessors!
  store i8* %34, i8** %35, align 8
  br label %39

39:                                               ; preds = %38
  %40 = getelementptr inbounds %"struct.PatternMatcher::Elem", %"struct.PatternMatcher::Elem"* %30, i64 -1, i32 2
  %41 = getelementptr inbounds %"struct.PatternMatcher::Elem", %"struct.PatternMatcher::Elem"* %28, i64 -1, i32 2, i32 0, i32 0
  %42 = load i8*, i8** %41, align 8
  %43 = getelementptr inbounds %"class.std::__cxx11::basic_string", %"class.std::__cxx11::basic_string"* %40, i64 0, i32 0, i32 0
  %44 = load i8*, i8** %43, align 8
  br label %46

45:                                               ; No predecessors!
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %42, i8* %44, i64 undef, i1 false)
  ret void

46:                                               ; preds = %39
  br label %47

47:                                               ; preds = %46
  store i8* %42, i8** %43, align 8
  ret void

48:                                               ; preds = %27
  %49 = getelementptr inbounds %"struct.std::vector<PatternMatcher::Elem>::_Temporary_value", %"struct.std::vector<PatternMatcher::Elem>::_Temporary_value"* %6, i64 0, i32 1, i32 0, i64 8
  %50 = getelementptr inbounds %"struct.PatternMatcher::Elem", %"struct.PatternMatcher::Elem"* undef, i64 0, i32 1, i32 0, i32 0
  %51 = load i8*, i8** %50, align 8
  %52 = bitcast i8* %49 to i8**
  %53 = load i8*, i8** %52, align 8
  %54 = getelementptr inbounds %"struct.std::vector<PatternMatcher::Elem>::_Temporary_value", %"struct.std::vector<PatternMatcher::Elem>::_Temporary_value"* %6, i64 0, i32 1, i32 0, i64 24
  br label %56

55:                                               ; No predecessors!
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* %51, i8* %53, i64 undef, i1 false)
  ret void

56:                                               ; preds = %48
  store i8* %53, i8** %50, align 8
  br i1 undef, label %58, label %57

57:                                               ; preds = %56
  store i8* %51, i8** %52, align 8
  ret void

58:                                               ; preds = %56
  %59 = bitcast i8* %49 to i8**
  store i8* %54, i8** %59, align 8
  ret void
}

define internal fastcc void @_ZN15MatchExpressionC2EPKcbbb() unnamed_addr align 2 personality i8* undef {
  %1 = invoke i8* @_Znwm(i64 64)
          to label %2 unwind label %5

2:                                                ; preds = %0
  %3 = bitcast i8* %1 to %class.PatternMatcher*
  invoke fastcc void @_ZN14PatternMatcher10setPatternEPKcbbb(%class.PatternMatcher* %3, i8* undef, i1 false, i1 true, i1 true)
          to label %4 unwind label %5

4:                                                ; preds = %2
  ret void

5:                                                ; preds = %2, %0
  %6 = landingpad { i8*, i32 }
          cleanup
  ret void
}

define internal void @_ZN13cClassFactoryD2Ev() unnamed_addr align 2 personality i32 (...)* undef {
  ret void
}
