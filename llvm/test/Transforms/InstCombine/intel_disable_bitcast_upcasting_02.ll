; RUN: opt -instcombine -disable-combine-upcasting=true < %s -S 2>&1 | FileCheck %s --check-prefix=CHECK-TRUE
; RUN: opt -instcombine -disable-combine-upcasting=false < %s -S 2>&1 | FileCheck %s --check-prefix=CHECK-FALSE

; This test checks that the load instruction at %tmp5 is
; not converted as a bitcast instruction since it can produce
; an upcasting. This test case checks when the enclosed structure
; is a base structure, but the casting use a padded structure.

%class.Inner1 = type <{ [ 4 x i32 ], i32, [4 x i8] }>

%class.Inner1.base = type <{ [ 4 x i32 ], i32 }>
%class.Inner2 = type <{ [ 8 x i32 ], i32 }>
%class.Outer2 = type { %class.Inner1.base, %class.Inner2 }
%class.Outer1 = type { %class.Outer2* }

declare void @bar(%class.Inner1* %0)
declare void @baz(%class.Outer2* %0)

define void @foo(%class.Outer1* %0, i32 %1) {
  %tmp2 = getelementptr inbounds %class.Outer1, %class.Outer1* %0, i64 0, i32 0
  %tmp3 = bitcast %class.Outer1* %0 to %class.Inner1**
  %tmp4 = load %class.Inner1*, %class.Inner1** %tmp3
  %tmp5 = load %class.Outer2*, %class.Outer2** %tmp2
  call void @bar(%class.Inner1* %tmp4)
  call void @baz(%class.Outer2* %tmp5)
  ret void
}

; Check that the bitcast wasn't created since -disable-combine-upcasting
; is enabled
; CHECK-TRUE: %tmp5 = load %class.Outer2*, %class.Outer2** %tmp2

; Check that the bitcast was generated since -disable-combine-upcasting is
; disabled
; CHECK-FALSE: %tmp5.cast = bitcast %class.Inner1* %tmp4 to %class.Outer2*
