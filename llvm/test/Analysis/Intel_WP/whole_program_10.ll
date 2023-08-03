; REQUIRES: asserts
; Test that checks if whole program not seen was identified correctly and the output
; is printed correctly when whole-program-trace-symbols is used. This is the same test
; case as whole_program_9.ll but it checks only the visibility trace. The goal of this
; test is to catch an indirect call that has some use in the program.

; RUN: llvm-as < %s >%t1
; RUN: llvm-lto -exported-symbol=main -debug-only=whole-program-analysis -whole-program-trace-visibility -o %t2 %t1 2>&1 | FileCheck %s

; CHECK: WHOLE-PROGRAM-ANALYSIS: EXTERNAL FUNCTIONS TRACE
; CHECK:      VISIBLE OUTSIDE LTO: 1
; CHECK-NEXT:    _ZN8Derived23fooEv

; Create the Base, Derived and Derived2 classes
%class.Base = type { ptr }
%class.Derived = type { %class.Base }
%class.Derived2 = type { %class.Derived }

; Create the global variables:
; Base b*;
; Derived d;
; Derived d2;

@b = dso_local local_unnamed_addr global ptr null, align 8
@d = dso_local global %class.Derived { %class.Base { ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV7Derived, i32 0, inrange i32 0, i32 2) } }, align 8
@d2 = dso_local global %class.Derived2 { %class.Derived { %class.Base { ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV8Derived2, i32 0, inrange i32 0, i32 2) } } }, align 8

; Setup the virtual tables
@_ZTV7Derived = linkonce_odr dso_local unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @_ZTI7Derived, ptr @_ZN7Derived3fooEv] }, align 8, !type !0, !type !1, !type !2, !type !3
@_ZTVN10__cxxabiv120__si_class_type_infoE = external dso_local global ptr
@_ZTS7Derived = linkonce_odr dso_local constant [9 x i8] c"7Derived\00"
@_ZTVN10__cxxabiv117__class_type_infoE = external dso_local global ptr
@_ZTS4Base = linkonce_odr dso_local constant [6 x i8] c"4Base\00"
@_ZTI4Base = linkonce_odr dso_local constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS4Base }
@_ZTI7Derived = linkonce_odr dso_local constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS7Derived, ptr @_ZTI4Base }
@_ZTV8Derived2 = available_externally dso_local unnamed_addr constant { [3 x ptr] } { [3 x ptr] [ptr null, ptr @_ZTI8Derived2, ptr @_ZN8Derived23fooEv] }, align 8
@_ZTI8Derived2 = external dso_local constant ptr
@llvm.used = appending global [1 x ptr] [ptr @_ZN8Derived23fooEv]

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
define i32 @main(i32 %argc, ptr nocapture readnone %argv) {
entry:
  %cmp = icmp slt i32 %argc, 2
  %. = select i1 %cmp, ptr @d, ptr @d2
  %.1 = select i1 %cmp, ptr @d, ptr @d2
  store ptr %., ptr @b, align 8, !tbaa !4
  %vtable = load ptr, ptr %., align 8, !tbaa !8
  %i = load ptr, ptr %vtable, align 8
  %call = tail call zeroext i1 %i(ptr %.1)
  ret i32 0
}


; Create Derived::foo() { return true; }

; Function Attrs: noinline
define linkonce_odr dso_local zeroext i1 @_ZN7Derived3fooEv(ptr %this) #0{
entry:
  ret i1 true
}

; Declare Derived2::foo();
define weak_odr zeroext i1 @_ZN8Derived23fooEv(ptr %this) #0 {
entry:
  ret i1 true
}

attributes #0 = { noinline }

!0 = !{i64 16, !"_ZTS4Base"}
!1 = !{i64 16, !"_ZTSM4BaseFbvE.virtual"}
!2 = !{i64 16, !"_ZTS7Derived"}
!3 = !{i64 16, !"_ZTSM7DerivedFbvE.virtual"}
!4 = !{!5, !5, i64 0}
!5 = !{!"unspecified pointer", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C++ TBAA"}
!8 = !{!9, !9, i64 0}
!9 = !{!"vtable pointer", !7, i64 0}
