; This test is to check that the devirtualizer marks a devirtualized invoke that
; involves bitcasts with metadata for the DTrans analysis. This case checks the
; path where there are multiple direct calls produced.

; RUN: opt < %s -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=wholeprogramdevirt -whole-program-visibility -wholeprogramdevirt-multiversion -S 2>&1 | FileCheck %s

; Define the types and global variables required for the devirtualizer.
%class.Base = type { i32 (...)** }
%class.Derived = type { %class.Base }
%class.Derived2 = type { %class.Base }

$_ZN7Derived3fooEi = comdat any
$_ZN8Derived23fooEi = comdat any
$_ZTV7Derived = comdat any
$_ZTS7Derived = comdat any
$_ZTS4Base = comdat any
$_ZTI4Base = comdat any
$_ZTI7Derived = comdat any
$_ZTV8Derived2 = comdat any
$_ZTS8Derived2 = comdat any
$_ZTI8Derived2 = comdat any

@b = hidden local_unnamed_addr global %class.Base* null, align 8
@d = hidden global %class.Derived zeroinitializer, align 8
@d2 = hidden global %class.Derived2 zeroinitializer, align 8
@_ZTIPKc = external dso_local constant i8*
@_ZTV7Derived = linkonce_odr hidden unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI7Derived to i8*), i8* bitcast (i1 (%class.Derived*, i32)* @_ZN7Derived3fooEi to i8*)] }, comdat, align 8, !type !0, !type !1, !type !2, !type !3
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTS7Derived = linkonce_odr hidden constant [9 x i8] c"7Derived\00", comdat
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS4Base = linkonce_odr hidden constant [6 x i8] c"4Base\00", comdat
@_ZTI4Base = linkonce_odr hidden constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @_ZTS4Base, i32 0, i32 0) }, comdat
@_ZTI7Derived = linkonce_odr hidden constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @_ZTS7Derived, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, comdat
@_ZTV8Derived2 = linkonce_odr hidden unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI8Derived2 to i8*), i8* bitcast (i1 (%class.Derived2*, i32)* @_ZN8Derived23fooEi to i8*)] }, comdat, align 8, !type !0, !type !1, !type !4, !type !5
@_ZTS8Derived2 = linkonce_odr hidden constant [10 x i8] c"8Derived2\00", comdat
@_ZTI8Derived2 = linkonce_odr hidden constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([10 x i8], [10 x i8]* @_ZTS8Derived2, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, comdat

; This function contains the indirect invoke that will be converted.
define i32 @main(i32 %argc, i8** %argv) local_unnamed_addr #3 personality i8* bitcast (i32 (...)* @__gxx_personality_v0 to i8*) {
; CHECK-LABEL: define i32 @main

  %vtable_addr = select i1 undef, i1 (%class.Base*, i32)*** bitcast (%class.Derived* @d to i1 (%class.Base*, i32)***), i1 (%class.Base*, i32)*** bitcast (%class.Derived2* @d2 to i1 (%class.Base*, i32)***)
  %obj = select i1 undef, %class.Base* getelementptr inbounds (%class.Derived, %class.Derived* @d, i64 0, i32 0), %class.Base* getelementptr inbounds (%class.Derived2, %class.Derived2* @d2, i64 0, i32 0)
  %vtable = load i1 (%class.Base*, i32)**, i1 (%class.Base*, i32)*** %vtable_addr
  %vtable_i8 = bitcast i1 (%class.Base*, i32)** %vtable to i8*
  %type_test_result = tail call i1 @llvm.type.test(i8* %vtable_i8, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %type_test_result)
  %fptr = load i1 (%class.Base*, i32)*, i1 (%class.Base*, i32)** %vtable

  ; This indirect invoke should be converted to a test for which function should
  ; be called, followed by a direct call to the appropriate function for the
  ; object type, and marked with metadata for DTrans.
  %call = invoke zeroext i1 %fptr(%class.Base* %obj, i32 %argc)
          to label %invoke.cont unwind label %lpad

; CHECK-LABEL: BBDevirt__ZN7Derived3fooEi:
; CHECK: invoke zeroext i1 bitcast (i1 (%class.Derived*, i32)* @_ZN7Derived3fooEi to i1 (%class.Base*, i32)*)(%class.Base* %obj, i32 %argc)
; CHECK:  to label %MergeBB unwind label %lpad, !_Intel.Devirt.Call

; CHECK-LABEL: BBDevirt__ZN8Derived23fooEi:
; CHECK: invoke zeroext i1 bitcast (i1 (%class.Derived2*, i32)* @_ZN8Derived23fooEi to i1 (%class.Base*, i32)*)(%class.Base* %obj, i32 %argc)
; CHECK: to label %MergeBB unwind label %lpad, !_Intel.Devirt.Call

invoke.cont:
  ret i32 0

lpad:
  %lp = landingpad { i8*, i32 }
  cleanup
  catch i8* bitcast (i8** @_ZTIPKc to i8*)
  resume { i8*, i32 } %lp
}

declare i1 @llvm.type.test(i8*, metadata)
declare void @llvm.assume(i1)
declare dso_local i32 @__gxx_personality_v0(...)
declare i32 @llvm.eh.typeid.for(i8*)

define linkonce_odr hidden zeroext i1 @_ZN7Derived3fooEi(%class.Derived* %this, i32 %a) {
  ret i1 true
}

define linkonce_odr hidden zeroext i1 @_ZN8Derived23fooEi(%class.Derived2* %this, i32 %a) {
  ret i1 false
}

!llvm.module.flags = !{!6}

!0 = !{i64 16, !"_ZTS4Base"}
!1 = !{i64 16, !"_ZTSM4BaseFbiE.virtual"}
!2 = !{i64 16, !"_ZTS7Derived"}
!3 = !{i64 16, !"_ZTSM7DerivedFbiE.virtual"}
!4 = !{i64 16, !"_ZTS8Derived2"}
!5 = !{i64 16, !"_ZTSM8Derived2FbiE.virtual"}
!6 = !{i32 1, !"wchar_size", i32 4}
