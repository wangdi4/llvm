; This test is to check that the devirtualizer marks a devirtualized call that
; involves bitcasts with metadata for the DTrans analysis. This case checks the
; path where there are multiple direct calls produced.

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"

; RUN: opt < %s -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=wholeprogramdevirt -whole-program-visibility -wholeprogramdevirt-multiversion -S 2>&1 | FileCheck %s

; Define the types and global variables required for the devirtualizer.
%class.Base = type { ptr }
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

@b = hidden local_unnamed_addr global ptr null, align 8
@d = hidden global %class.Derived zeroinitializer, align 8
@d2 = hidden global %class.Derived2 zeroinitializer, align 8

@_ZTV7Derived = linkonce_odr hidden unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @_ZTI7Derived, ptr @_ZN7Derived3fooEi] }, comdat, align 8, !type !0, !type !1, !type !2, !type !3
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global ptr
@_ZTS7Derived = linkonce_odr hidden constant [9 x i8] c"7Derived\00", comdat
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTS4Base = linkonce_odr hidden constant [6 x i8] c"4Base\00", comdat
@_ZTI4Base = linkonce_odr hidden constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS4Base }, comdat
@_ZTI7Derived = linkonce_odr hidden constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS7Derived, ptr @_ZTI4Base }, comdat
@_ZTV8Derived2 = linkonce_odr hidden unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @_ZTI8Derived2, ptr @_ZN8Derived23fooEi] }, comdat, align 8, !type !0, !type !1, !type !4, !type !5
@_ZTS8Derived2 = linkonce_odr hidden constant [10 x i8] c"8Derived2\00", comdat
@_ZTI8Derived2 = linkonce_odr hidden constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS8Derived2, ptr @_ZTI4Base }, comdat

; This function contains the indirect call that will be converted.
define i32 @main(i32 %argc, ptr %argv) {
; CHECK-LABEL: define i32 @main

  %vtable_addr = select i1 undef, ptr @d, ptr @d2
  %obj = select i1 undef, ptr @d, ptr @d2
  %vtable = load ptr, ptr %vtable_addr, align 8
  %vtable_i8 = bitcast ptr %vtable to ptr
  %type_test_result = tail call i1 @llvm.type.test(ptr %vtable_i8, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %type_test_result)
  %fptr = load ptr, ptr %vtable, align 8

  ; This indirect call should be converted to a test for which function should
  ; be called, followed by a direct call to the appropriate function for the
  ; object type, and marked with metadata for DTrans.
  %call = tail call zeroext i1 %fptr(ptr %obj, i32 %argc)
; CHECK-LABEL: BBDevirt__ZN7Derived3fooEi
; CHECK:  tail call zeroext i1 @_ZN7Derived3fooEi(ptr %obj, i32 %argc), !_Intel.Devirt.Call
; CHECK-LABEL: BBDevirt__ZN8Derived23fooEi
; CHECK: call zeroext i1 @_ZN8Derived23fooEi(ptr %obj, i32 %argc), !_Intel.Devirt.Call

  ret i32 0
}

declare i1 @llvm.type.test(ptr, metadata)
declare void @llvm.assume(i1)

define linkonce_odr hidden zeroext i1 @_ZN7Derived3fooEi(ptr %this, i32 %a) {
  ret i1 true
}

define linkonce_odr hidden zeroext i1 @_ZN8Derived23fooEi(ptr %this, i32 %a){
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
