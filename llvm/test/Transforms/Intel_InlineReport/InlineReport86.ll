; RUN: opt < %s -inline -ip-cloning -ip-cloning-after-inl -ip-cloning-force-heuristics-off -inline-report=7 -S 2>&1 | FileCheck %s
; RUN: opt < %s -passes='cgscc(inline),module(post-inline-ip-cloning)' -ip-cloning-force-heuristics-off -inline-report=7 -S 2>&1 | FileCheck %s

; Check that the classic inlining report includes the results of cloning
; after inlining. Also check that it does NOT include 'Newly created callsite',
; 'Not tested for inlining', or 'DELETE' messages.

; CHECK: COMPILE FUNC: myadd
; CHECK: COMPILE FUNC: mysub
; CHECK: COMPILE FUNC: main
; CHECK: myadd.2{{.*}}Callee has noinline attribute{{.*}}
; CHECK: myadd.1{{.*}}Callee has noinline attribute{{.*}}
; CHECK: mysub.4{{.*}}Callee has noinline attribute{{.*}}
; CHECK: mysub.4{{.*}}Callee has noinline attribute{{.*}}
; CHECK: mysub.3{{.*}}Callee has noinline attribute{{.*}}
; CHECK-NOT: Newly created callsite
; CHECK-NOT: Not tested for inlining
; CHECK-NOT: DELETE
; CHECK: COMPILE FUNC: myadd.1
; CHECK: COMPILE FUNC: myadd.2
; CHECK: COMPILE FUNC: mysub.3
; CHECK: COMPILE FUNC: mysub.4

define dso_local i32 @main() local_unnamed_addr {
entry:
  %call = tail call i32 @myadd(i32 1, i32 2)
  %call1 = tail call i32 @myadd(i32 1, i32 3)
  %add1 = add nsw i32 %call1, %call
  %call2 = tail call i32 @mysub(i32 4, i32 5)
  %call3 = tail call i32 @mysub(i32 4, i32 5)
  %add2 = add nsw i32 %call1, %call3
  %add3 = add i32 %add1, %add2
  %call4 = tail call i32 @mysub(i32 6, i32 7)
  %add4 = add i32 %add3, %call4
  ret i32 %add4
}

define internal i32 @myadd(i32 %a, i32 %b) local_unnamed_addr #0 {
entry:
  %add = add nsw i32 %b, %a
  ret i32 %add
}

define internal i32 @mysub(i32 %a, i32 %b) local_unnamed_addr #0 {
entry:
  %sub = sub nsw i32 %a, %b
  ret i32 %sub
}

attributes #0 = { noinline }
