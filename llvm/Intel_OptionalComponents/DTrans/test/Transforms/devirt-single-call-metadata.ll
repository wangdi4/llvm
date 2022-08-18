; This test is to check that the devirtualizer marks a devirtualized call that
; involves bitcasts with metadata for the DTrans analysis. This case checks the
; path where there is only a single devirtualization target for the call.

; RUN: opt < %s -passes=wholeprogramdevirt -whole-program-visibility -S 2>&1 | FileCheck %s

; Define the types and global variables required for the devirtualizer.
%class.Base = type { i32 (...)** }
%class.Derived = type { %class.Base }

$_ZTV7Derived = comdat any
$_ZTS7Derived = comdat any
$_ZTI7Derived = comdat any

$_ZTS4Base = comdat any
$_ZTI4Base = comdat any

@b = hidden local_unnamed_addr global %class.Base* null, align 8
@d = hidden global %class.Derived zeroinitializer, align 8
@_ZTV7Derived = linkonce_odr hidden unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI7Derived to i8*), i8* bitcast (i1 (%class.Derived*, i32)* @_ZN7Derived3fooEi to i8*)] }, comdat, align 8, !type !0, !type !1, !type !2, !type !3
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTS7Derived = linkonce_odr hidden constant [9 x i8] c"7Derived\00", comdat
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS4Base = linkonce_odr hidden constant [6 x i8] c"4Base\00", comdat
@_ZTI4Base = linkonce_odr hidden constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @_ZTS4Base, i32 0, i32 0) }, comdat
@_ZTI7Derived = linkonce_odr hidden constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @_ZTS7Derived, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }, comdat

; This function contains the indirect call that will be converted.
define i32 @main(i32 %argc, i8** %argv) {
; CHECK-LABEL: define i32 @main

  %vtable_addr = bitcast %class.Derived* @d to i1 (%class.Base*, i32)***
  %obj = getelementptr %class.Derived, %class.Derived* @d, i64 0, i32 0
  %vtable = load i1 (%class.Base*, i32)**, i1 (%class.Base*, i32)*** %vtable_addr
  %vtable_i8 = bitcast i1 (%class.Base*, i32)** %vtable to i8*
  %type_test_result = tail call i1 @llvm.type.test(i8* %vtable_i8, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %type_test_result)
  %fptr = load i1 (%class.Base*, i32)*, i1 (%class.Base*, i32)** %vtable

  ; This indirect call should be converted to a direct call to
  ; _ZN7Derived3fooEi, and marked with metadata for DTrans.
  %call = tail call zeroext i1 %fptr(%class.Base* %obj, i32 %argc)
; CHECK: %call = tail call zeroext i1 bitcast (i1 (%class.Derived*, i32)* @_ZN7Derived3fooEi to i1 (%class.Base*, i32)*)(%class.Base* %obj, i32 %argc), !_Intel.Devirt.Call

  ret i32 0
}

declare i1 @llvm.type.test(i8*, metadata)
declare void @llvm.assume(i1)

define internal default zeroext i1 @_ZN7Derived3fooEi(%class.Derived* %this, i32 %a) {
  ret i1 true
}

!llvm.module.flags = !{!4}

!0 = !{i64 16, !"_ZTS4Base"}
!1 = !{i64 16, !"_ZTSM4BaseFbiE.virtual"}
!2 = !{i64 16, !"_ZTS7Derived"}
!3 = !{i64 16, !"_ZTSM7DerivedFbiE.virtual"}
!4 = !{i32 1, !"wchar_size", i32 4}
