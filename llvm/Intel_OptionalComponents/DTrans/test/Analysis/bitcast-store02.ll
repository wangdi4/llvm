; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that bad casting for related types is set for the types
; when the data is being moved to a new space with the same type as the
; source.

; CHECK: Safety data: {{.*}}Bad casting (related types){{.*}}

$__clang_call_terminate = comdat any
$_ZTS2Ex = comdat any
$_ZTI2Ex = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS2Ex = linkonce_odr dso_local constant [4 x i8] c"2Ex\00", comdat, align 1
@_ZTI2Ex = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @_ZTS2Ex, i32 0, i32 0) }, comdat, align 8

%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" }
%"struct.std::_Vector_base<TestClass, std::allocator<TestClass>>::_Vector_impl.67" = type { %class.TestClass*, %class.TestClass*, %class.TestClass* }
%class.TestClass = type {i64, i64, i64}


define internal i8* @copy(%"class.std::vector.12"* nocapture %arg) unnamed_addr align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
bb:
  %tmp = invoke noalias nonnull dereferenceable(24) i8* @_Znwm(i64 24)
          to label %bb2 unwind label %lpad

bb2:
  %tmp1 = bitcast i8* %tmp to %"class.std::vector.12"*
  br label %bb3

bb3:
  %tmp4 = getelementptr inbounds i8, i8* %tmp, i64 0
  %tmp5 = bitcast i8* %tmp4 to i64*

  %tmp6 = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %arg, i64 0, i32 0
  %tmp7 = bitcast %"struct.std::_Vector_base.13"* %tmp6 to i64*
  %tmp8 = load i64, i64* %tmp7

  store i64 %tmp8, i64* %tmp5
  ret i8* %tmp

lpad:
  %lp1 = landingpad { i8*, i32 }
          catch i8* null
  %ext1 = extractvalue { i8*, i32 } %lp1, 0
  %call1 = tail call i8* @__cxa_begin_catch(i8* %ext1)
  %exception = tail call i8* @__cxa_allocate_exception(i64 1)
  invoke void @__cxa_throw(i8* nonnull %exception, i8* bitcast ({ i8*, i8* }* @_ZTI2Ex to i8*), i8* null)
          to label %unreachable unwind label %lpad2

lpad2:
  %lp2 = landingpad { i8*, i32 }
          cleanup
  invoke void @__cxa_end_catch()
          to label %invoke.cont3 unwind label %terminate.lpad

invoke.cont3:
  %ext2 = extractvalue { i8*, i32 } %lp2, 0
  call void @_Unwind_Resume(i8* %ext2)
  unreachable

terminate.lpad:
  %lp3 = landingpad { i8*, i32 }
          catch i8* null
  %ext3 = extractvalue { i8*, i32 } %lp3, 0
  tail call void @__clang_call_terminate(i8* %ext3)
  unreachable

unreachable:
  unreachable
}

declare dso_local noalias nonnull i8* @_Znwm(i64) local_unnamed_addr

declare dso_local i32 @__gxx_personality_v0(...)

declare dso_local i8* @__cxa_begin_catch(i8*) local_unnamed_addr

declare dso_local noalias nonnull i8* @__cxa_allocate_exception(i64) local_unnamed_addr

declare dso_local void @__cxa_throw(i8*, i8*, i8*) local_unnamed_addr

declare dso_local void @__cxa_end_catch() local_unnamed_addr

declare dso_local void @_ZSt9terminatev() local_unnamed_addr

define linkonce_odr hidden void @__clang_call_terminate(i8* %ptr) local_unnamed_addr comdat {
  %call = tail call i8* @__cxa_begin_catch(i8* %ptr)
  tail call void @_ZSt9terminatev()
  unreachable
}

declare void @_Unwind_Resume(i8*)
