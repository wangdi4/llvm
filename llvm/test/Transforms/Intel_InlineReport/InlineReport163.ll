; RUN: opt -passes=inline -disable-output -inline-report=0xf847 < %s 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup,inline,inlinereportemitter' -disable-output -inline-report=0xf8c6 < %s 2>&1 | FileCheck %s

; Check that when the inliner replaces the indirect call to %__pf with a call
; to @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_ that the call
; to @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_ appears in
; the inline report.

; CHECK: COMPILE FUNC: _ZN12cctki_piraha8find_valENS_9smart_ptrINS_5GroupEEENSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES8_
; CHECK: INLINE: _ZNSolsEPFRSoS_E {{.*}}Callee is single basic block
; CHECK: INLINE: _ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_ {{.*}}Callee is single basic block

%"class._ZTSN12cctki_piraha9smart_ptrINS_5ValueEEE.cctki_piraha::smart_ptr" = type { ptr }
$_ZN12cctki_piraha9smart_ptrINS_5ValueEED2Ev = comdat any

declare dso_local i32 @__gxx_personality_v0(...)

define available_externally dso_local void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED2Ev(ptr noundef align 8 dereferenceable(32) %this) unnamed_addr #7 align 2 {
  ret void
}

define available_externally dso_local void @_ZNSt7__cxx1119basic_ostringstreamIcSt11char_traitsIcESaIcEED1Ev(ptr noundef align 8 dereferenceable(112) %this) unnamed_addr #7 align 2 {
  ret void
}

define linkonce_odr dso_local void @_ZN12cctki_piraha9smart_ptrINS_5ValueEED2Ev(ptr noundef align 8 dereferenceable(8) %this) unnamed_addr #7 comdat align 2 {
  ret void
}

define available_externally dso_local noundef align 8 ptr @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_(ptr noundef nonnull align 8 dereferenceable(8) %__os) #21 {
  ret ptr null
}

define dso_local void @_ZN12cctki_piraha8find_valENS_9smart_ptrINS_5GroupEEENSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEEES8_(ptr noalias sret(%"class._ZTSN12cctki_piraha9smart_ptrINS_5ValueEEE.cctki_piraha::smart_ptr") align 8 %agg.result, ptr noundef %gr, ptr noundef %thorn, ptr noundef %name) local_unnamed_addr #0 personality ptr @__gxx_personality_v0 {
entry:
  %call70 = invoke noundef nonnull align 8 dereferenceable(8) ptr @_ZNSolsEPFRSoS_E(ptr noundef nonnull align 8 dereferenceable(8) null, ptr noundef nonnull @_ZSt4endlIcSt11char_traitsIcEERSt13basic_ostreamIT_T0_ES6_)
          to label %invoke.cont85 unwind label %lpad64

invoke.cont85:                                    ; preds = %invoke.cont82
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED2Ev(ptr noundef align 8 dereferenceable(32) null) #18
  call void @_ZNSt7__cxx1112basic_stringIcSt11char_traitsIcESaIcEED2Ev(ptr noundef align 8 dereferenceable(32) null) #18
  call void @_ZNSt7__cxx1119basic_ostringstreamIcSt11char_traitsIcESaIcEED1Ev(ptr noundef align 8 dereferenceable(112) null) #18
  br label %nrvo.skipdtor

lpad64:                                           ; preds = %invoke.cont67, %invoke.cont65, %invoke.cont63
  %t40 = landingpad { ptr, i32 }
          cleanup
  br label %ehcleanup90

ehcleanup90:                                      ; preds = %ehcleanup89, %lpad64
  call void @_ZNSt7__cxx1119basic_ostringstreamIcSt11char_traitsIcESaIcEED1Ev(ptr noundef nonnull align 8 dereferenceable(112) null) #18
  br label %ehcleanup91

ehcleanup91:                                      ; preds = %ehcleanup90, %lpad62
  br label %ehcleanup92

ehcleanup92:                                      ; preds = %ehcleanup91, %lpad57, %ehcleanup40, %lpad6
  call void @_ZN12cctki_piraha9smart_ptrINS_5ValueEED2Ev(ptr noundef nonnull align 8 dereferenceable(8) null) #18
  br label %eh.resume

nrvo.skipdtor:                                    ; preds = %sw.bb, %sw.bb44, %sw.bb48, %invoke.cont85, %sw.bb53
  ret void

eh.resume:                                        ; preds = %ehcleanup, %cleanup.action, %ehcleanup92
  resume { ptr, i32 } %t40
}

; Function Attrs: mustprogress uwtable
define available_externally dso_local noundef nonnull align 8 dereferenceable(8) ptr @_ZNSolsEPFRSoS_E(ptr noundef nonnull align 8 dereferenceable(8) %this, ptr noundef %__pf) local_unnamed_addr #14 align 2 {
entry:
  %call = call noundef nonnull align 8 dereferenceable(8) ptr %__pf(ptr noundef nonnull align 8 dereferenceable(8) %this)
  ret ptr %call
}
