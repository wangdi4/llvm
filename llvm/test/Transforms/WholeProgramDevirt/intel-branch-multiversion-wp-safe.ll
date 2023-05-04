; INTEL_FEATURE_SW_DTRANS
; REQUIRES: intel_feature_sw_dtrans
; This test checks that the multiversioning based on virtual function pointers
; was done correctly when whole program safe is achieved. The goal is to check
; that the virtual call is removed correctly since all targets were found.
; The test case is pretty simple, this is how it looks like in C++:
;
; class Base {
;  virtual bool foo(int a) = 0;
; };
;
; class Derived : public Base {
;   bool foo(int a) {
;     return true;
;   }
; };
;
; class Derived2 : public Base {
;   bool foo(int a) {
;     return false;
;   }
; };
;
; Base *b;
; Derived d;
; Derived2 d2;
;
; int main(int argc, char* argv[]) {
;
;   if (argc < 2)
;     b = &d;
;   else
;     b = &d2;
;
;   bool c = b->foo(argc);
;   std::cout << c <<"\n";
;   return 0;
; }
; The devirtualization process should generate multiversioning similar to
; the following (in pseudo-code):
; if (address of b->foo == address of Derived::foo)
;   call Derived::foo
; else
;   call Derived2::foo

; RUN: opt < %s -enable-intel-advanced-opts=1 -mtriple=i686-- -mattr=+avx2 -passes=wholeprogramdevirt -wholeprogramdevirt-multiversion -wholeprogramdevirt-multiversion-verify -whole-program-assume -S 2>&1 | FileCheck %s

%"class.std::ios_base::Init" = type { i8 }
%class.Derived = type { %class.Base }
%class.Base = type { ptr }
%class.Derived2 = type { %class.Base }
%"class.std::basic_ostream" = type { ptr, %"class.std::basic_ios" }
%"class.std::basic_ios" = type { %"class.std::ios_base", ptr, i8, i8, ptr, ptr, ptr, ptr }
%"class.std::ios_base" = type { ptr, i64, i64, i32, i32, i32, ptr, %"struct.std::ios_base::_Words", [8 x %"struct.std::ios_base::_Words"], i32, ptr, %"class.std::locale" }
%"struct.std::ios_base::_Words" = type { ptr, i64 }
%"class.std::locale" = type { ptr }

$_ZTV7Derived = comdat any

$_ZTS7Derived = comdat any

$_ZTS4Base = comdat any

$_ZTI4Base = comdat any

$_ZTI7Derived = comdat any

$_ZTV8Derived2 = comdat any

$_ZTS8Derived2 = comdat any

$_ZTI8Derived2 = comdat any

@_ZSt8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@b = hidden local_unnamed_addr global ptr null, align 8
@d = hidden global %class.Derived zeroinitializer, align 8
@d2 = hidden global %class.Derived2 zeroinitializer, align 8
@_ZSt4cout = external dso_local global %"class.std::basic_ostream", align 8
@.str = private unnamed_addr constant [2 x i8] c"\0A\00", align 1
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
@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_driver2.cpp, ptr null }]

declare dso_local void @_ZNSt8ios_base4InitC1Ev(ptr) unnamed_addr #0

; Function Attrs: nounwind
declare dso_local void @_ZNSt8ios_base4InitD1Ev(ptr) unnamed_addr #1

; Function Attrs: nounwind
declare dso_local i32 @__cxa_atexit(ptr, ptr, ptr) local_unnamed_addr #2

; Function Attrs: norecurse uwtable
define hidden i32 @main(i32 %argc, ptr nocapture readnone %argv) local_unnamed_addr #3 {
entry:
  %cmp = icmp slt i32 %argc, 2
  %. = select i1 %cmp, ptr @d, ptr @d2
  %.4 = select i1 %cmp, ptr @d, ptr @d2
  store ptr %., ptr @b, align 8, !tbaa !8
  %vtable = load ptr, ptr %., align 8, !tbaa !12
  %tmp = bitcast ptr %vtable to ptr
  %tmp1 = tail call i1 @llvm.type.test(ptr %tmp, metadata !"_ZTS4Base")
  tail call void @llvm.assume(i1 %tmp1)
  %tmp2 = load ptr, ptr %vtable, align 8
  %call = tail call zeroext i1 %tmp2(ptr %.4, i32 %argc)
  %call.i = tail call dereferenceable(272) ptr @_ZNSo9_M_insertIbEERSoT_(ptr nonnull @_ZSt4cout, i1 zeroext %call)
  %call1.i = tail call dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull dereferenceable(272) %call.i, ptr nonnull @.str, i64 1)
  ret i32 0
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i1 @llvm.type.test(ptr, metadata) #4

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.assume(i1 noundef) #5

; Function Attrs: noinline norecurse nounwind uwtable
define internal zeroext i1 @_ZN7Derived3fooEi(ptr %this, i32 %a) unnamed_addr #6 {
entry:
  ret i1 true
}

; Function Attrs: noinline norecurse nounwind uwtable
define internal zeroext i1 @_ZN8Derived23fooEi(ptr %this, i32 %a) unnamed_addr #6 {
entry:
  ret i1 false
}

declare dso_local dereferenceable(272) ptr @_ZNSo9_M_insertIbEERSoT_(ptr, i1 zeroext) local_unnamed_addr #0

declare dso_local dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr dereferenceable(272), ptr, i64) local_unnamed_addr #0

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_driver2.cpp() #7 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(ptr nonnull @_ZSt8__ioinit)
  %i = tail call i32 @__cxa_atexit(ptr @_ZNSt8ios_base4InitD1Ev, ptr @_ZSt8__ioinit, ptr nonnull @__dso_handle) #2
  store ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV7Derived, i64 0, inrange i32 0, i64 2), ptr @d, align 8, !tbaa !12
  store ptr getelementptr inbounds ({ [3 x ptr] }, ptr @_ZTV8Derived2, i64 0, inrange i32 0, i64 2), ptr @d2, align 8, !tbaa !12
  ret void
}

attributes #0 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind }
attributes #3 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #5 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #6 = { noinline norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #7 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!6}
!llvm.ident = !{!7}

!0 = !{i64 16, !"_ZTS4Base"}
!1 = !{i64 16, !"_ZTSM4BaseFbiE.virtual"}
!2 = !{i64 16, !"_ZTS7Derived"}
!3 = !{i64 16, !"_ZTSM7DerivedFbiE.virtual"}
!4 = !{i64 16, !"_ZTS8Derived2"}
!5 = !{i64 16, !"_ZTSM8Derived2FbiE.virtual"}
!6 = !{i32 1, !"wchar_size", i32 4}
!7 = !{!"clang version 8.0.0"}
!8 = !{!9, !9, i64 0}
!9 = !{!"unspecified pointer", !10, i64 0}
!10 = !{!"omnipotent char", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
!12 = !{!13, !13, i64 0}
!13 = !{!"vtable pointer", !11, i64 0}


; Check that the transformation was applied correctly in function main.
; CHECK:       define hidden i32 @main(i32 %argc, ptr nocapture readnone %argv) local_unnamed_addr #3 {

; CHECK:       [[T:%.*]] = bitcast ptr %vtable to ptr
; CHECK-NEXT:  [[T1:%.*]] = tail call i1 @llvm.type.test(ptr [[T]], metadata !"_ZTS4Base")

; This part checks if the address of the virtual function is the same as Derived::foo
; CHECK:       %0 = bitcast ptr %tmp2 to ptr
; CHECK-NEXT:  %1 = bitcast ptr @_ZN7Derived3fooEi to ptr
; CHECK-NEXT:  %2 = icmp eq ptr %0, %1
; CHECK-NEXT:  br i1 %2, label %BBDevirt__ZN7Derived3fooEi, label %BBDevirt__ZN8Derived23fooEi

; If the address is the same, then call Derived::foo
; CHECK-LABEL: BBDevirt__ZN7Derived3fooEi:
; CHECK:        %3 = tail call zeroext i1 @_ZN7Derived3fooEi(ptr %.4, i32 %argc)
; CHECK-NEXT:   br label %MergeBB

; If the address is the same as Derived2::foo, then call it
; CHECK-LABEL: BBDevirt__ZN8Derived23fooEi:
; CHECK:        %4 = tail call zeroext i1 @_ZN8Derived23fooEi(ptr %.4, i32 %argc)
; CHECK-NEXT:   br label %MergeBB

; We need to collect back the result and generate the PhiNode
; CHECK-LABEL: MergeBB:
; CHECK-NEXT:   %5 = phi i1 [ %3, %BBDevirt__ZN7Derived3fooEi ], [ %4, %BBDevirt__ZN8Derived23fooEi ]
; CHECK-NEXT:   br label %6

; Now check that the users were replaced correctly
; CHECK-LABEL: 6:
; CHECK-NEXT:   %call.i = tail call dereferenceable(272) ptr @_ZNSo9_M_insertIbEERSoT_(ptr nonnull @_ZSt4cout, i1 zeroext %5)
; CHECK-NEXT:   %call1.i = tail call dereferenceable(272) ptr @_ZSt16__ostream_insertIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_PKS3_l(ptr nonnull dereferenceable(272) %call.i, ptr nonnull @.str, i64 1)
; CHECK-NEXT:   ret i32 0
; CHECK-NEXT: }

; end INTEL_FEATURE_SW_DTRANS
