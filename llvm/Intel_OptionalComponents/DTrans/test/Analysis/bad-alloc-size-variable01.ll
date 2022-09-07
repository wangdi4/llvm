; REQUIRES: asserts
; UNSUPPORTED: enable-opaque-pointers

; RUN: opt  < %s -whole-program-assume -dtransanalysis -dtrans-print-types -disable-output 2>&1 | FileCheck %s
; RUN: opt  < %s -whole-program-assume -passes='require<dtransanalysis>' -dtrans-print-types -disable-output 2>&1 | FileCheck %s

; This test checks that "bad alloc size" is not set for the types when the size
; used to allocate a new memory space is a multiple of the type size. This
; is the same test as bad-ptr-manupulation-related-types01.ll but it checks
; for "bad alloc size".

; CHECK: LLVMType: %class.OuterClass = type { %class.TestClass, %"class.std::vector.12" }
; CHECK-NOT: Safety data: {{.*}}Bad alloc size{{.*}}

$__clang_call_terminate = comdat any
$_ZTS2Ex = comdat any
$_ZTI2Ex = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS2Ex = linkonce_odr dso_local constant [4 x i8] c"2Ex\00", comdat, align 1
@_ZTI2Ex = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([4 x i8], [4 x i8]* @_ZTS2Ex, i32 0, i32 0) }, comdat, align 8

%class.OuterClass = type {%class.TestClass, %"class.std::vector.12"}
%"class.std::vector.12" = type { %"struct.std::_Vector_base.13" }
%"struct.std::_Vector_base.13" = type { %"struct.std::_Vector_base<OuterClass, std::allocator<OuterClass>>::_Vector_impl" }
%"struct.std::_Vector_base<OuterClass, std::allocator<OuterClass>>::_Vector_impl" = type { %class.OuterClass*, %class.OuterClass*, %class.OuterClass* }
%class.TestClass = type {i64, i64, i64}

define internal %class.OuterClass* @new_space(%class.OuterClass* nocapture %arg1) unnamed_addr align 2 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
bb:
  %tmp = getelementptr inbounds %class.OuterClass, %class.OuterClass* %arg1, i64 0, i32 1
  %tmp1 = bitcast %"class.std::vector.12"* %tmp to i64*
  %tmp2 = load i64, i64* %tmp1

  %tmp3 = getelementptr inbounds %class.OuterClass, %class.OuterClass* %arg1, i64 0, i32 1
  %tmp4 = getelementptr inbounds %"class.std::vector.12", %"class.std::vector.12"* %tmp3, i64 0, i32 0
  %tmp5 = getelementptr inbounds %"struct.std::_Vector_base.13", %"struct.std::_Vector_base.13"* %tmp4, i64 0, i32 0
  %tmp6 = getelementptr inbounds %"struct.std::_Vector_base<OuterClass, std::allocator<OuterClass>>::_Vector_impl", %"struct.std::_Vector_base<OuterClass, std::allocator<OuterClass>>::_Vector_impl"* %tmp5, i64 0, i32 1
  %tmp7 = bitcast %class.OuterClass** %tmp6 to i64*
  %tmp8 = load i64, i64* %tmp7

  %tmp9 = sub i64 %tmp8, %tmp2
  %tmp10 = sdiv exact i64 %tmp9, 48
  %tmp11 = icmp eq i64 %tmp10, 0
  br i1 %tmp11, label %bb5, label %bb2

bb2:
  %tmp12 = icmp ugt i64 %tmp10, 2167145685351216
  br i1 %tmp12, label %bb3, label %unreachable

bb3:
  %tmp13 = invoke noalias nonnull i8* @_Znwm(i64 %tmp9)
            to label %bb4 unwind label %lpad

bb4:
  %tmp14 = bitcast i8* %tmp13 to %class.OuterClass*
  br label %bb5

bb5:
  %tmp15 = phi %class.OuterClass* [%tmp14, %bb4] , [null, %bb]
  ret %class.OuterClass* %tmp15;

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
