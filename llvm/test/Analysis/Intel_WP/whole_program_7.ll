; REQUIRES: assert
; Test that checks if whole program seen was identified correctly.The test case
; is simple. It contains a Base class, Derived class and Derived2 class. The
; function foo is declared as virtual in Base and the definition can be found
; in Derived and Derived2. Since the definition is seen in both derived classes
; then it should be whole program seen.

; RUN: llvm-as < %s >%t1
; RUN: llvm-lto -exported-symbol=main -debug-only=whole-program-analysis -whole-program-assume-hidden -o %t2 %t1 2>&1 | FileCheck %s

; CHECK:     WHOLE-PROGRAM-ANALYSIS: SIMPLE ANALYSIS
; CHECK:      UNRESOLVED CALLSITES: 0
; CHECK:      VISIBLE OUTSIDE LTO: 0
; CHECK:     WHOLE PROGRAM DETECTED
; CHECK-NOT: whole program not seen;

; Create the Base, Derived and Derived2 classes
%class.Base = type { i32 (...)** }
%class.Derived = type { %class.Base }
%class.Derived2 = type { %class.Derived }

; Create the global variables:
; Base b*;
; Derived d;
; Derived d2;

@b = dso_local local_unnamed_addr global %class.Base* null, align 8
@d = dso_local global %class.Derived { %class.Base { i32 (...)** bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTV7Derived, i32 0, inrange i32 0, i32 2) to i32 (...)**) } }, align 8
@d2 = dso_local global %class.Derived2 { %class.Derived { %class.Base { i32 (...)** bitcast (i8** getelementptr inbounds ({ [3 x i8*] }, { [3 x i8*] }* @_ZTV8Derived2, i32 0, inrange i32 0, i32 2) to i32 (...)**) } } }, align 8

; Setup the virtual tables
@_ZTV7Derived = linkonce_odr dso_local unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast ({ i8*, i8*, i8* }* @_ZTI7Derived to i8*), i8* bitcast (i1 (%class.Derived*)* @_ZN7Derived3fooEv to i8*)] }, align 8, !type !0, !type !1, !type !2, !type !3
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global i8*
@_ZTS7Derived = linkonce_odr dso_local constant [9 x i8] c"7Derived\00"
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global i8*
@_ZTS4Base = linkonce_odr dso_local constant [6 x i8] c"4Base\00"
@_ZTI4Base = linkonce_odr dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([6 x i8], [6 x i8]* @_ZTS4Base, i32 0, i32 0) }
@_ZTI7Derived = linkonce_odr dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @_ZTS7Derived, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI4Base to i8*) }
@_ZTV8Derived2 = available_externally dso_local unnamed_addr constant { [3 x i8*] } { [3 x i8*] [i8* null, i8* bitcast (i8** @_ZTI8Derived2 to i8*), i8* bitcast (i1 (%class.Derived2*)* @_ZN8Derived23fooEv to i8*)] }, align 8
@_ZTI8Derived2 = external dso_local constant i8*

; Create main
; int main(int argc, char* argv* []) {
;  if (argc < 2)
;    b = &d;
;  else
;    b = &d2;
;  b->foo()
;  return 0;
; }

; Function Attrs: norecurse uwtable
define i32 @main(i32 %argc, i8** nocapture readnone %argv) {
entry:
  %cmp = icmp slt i32 %argc, 2
  %. = select i1 %cmp, i1 (%class.Base*)*** bitcast (%class.Derived* @d to i1 (%class.Base*)***), i1 (%class.Base*)*** bitcast (%class.Derived2* @d2 to i1 (%class.Base*)***)
  %.1 = select i1 %cmp, %class.Base* getelementptr inbounds (%class.Derived, %class.Derived* @d, i64 0, i32 0), %class.Base* getelementptr inbounds (%class.Derived2, %class.Derived2* @d2, i64 0, i32 0, i32 0)
  store i1 (%class.Base*)*** %., i1 (%class.Base*)**** bitcast (%class.Base** @b to i1 (%class.Base*)****), align 8, !tbaa !6
  %vtable = load i1 (%class.Base*)**, i1 (%class.Base*)*** %., align 8, !tbaa !10
  %0 = load i1 (%class.Base*)*, i1 (%class.Base*)** %vtable, align 8
  %call = tail call zeroext i1 %0(%class.Base* %.1)
  ret i32 0
}

; Create Derived::foo() { return true; }

; Function Attrs: noinline
define linkonce_odr dso_local zeroext i1 @_ZN7Derived3fooEv(%class.Derived* %this) #0{
entry:
  ret i1 true
}

; Create Derived2::foo() { return false; }

; Function Attrs: noinline
define linkonce_odr dso_local zeroext i1 @_ZN8Derived23fooEv(%class.Derived2* %this) #0{
entry:
  ret i1 false
}

attributes #0 = { noinline }

; Metadata used by the vtables to identify the classes
!0 = !{i64 16, !"_ZTS4Base"}
!1 = !{i64 16, !"_ZTSM4BaseFbvE.virtual"}
!2 = !{i64 16, !"_ZTS7Derived"}
!3 = !{i64 16, !"_ZTSM7DerivedFbvE.virtual"}

!6 = !{!7, !7, i64 0}
!7 = !{!"unspecified pointer", !8, i64 0}
!8 = !{!"omnipotent char", !9, i64 0}
!9 = !{!"Simple C++ TBAA"}
!10 = !{!11, !11, i64 0}
!11 = !{!"vtable pointer", !9, i64 0}
