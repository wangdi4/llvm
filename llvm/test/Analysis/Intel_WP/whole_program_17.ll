; REQUIRES: asserts

; This test case resembles the following C source code (simplified):

; #include<stdio.h>
;
; // Functions to be executed before and after main()
; void __attribute__((constructor)) calls_after_main();
; void __attribute__((destructor)) calls_after_main();
;
; int init_i();
; int init_k();
; void foo();
; void bar_before_main();
; void bar_after_main();
; void unused_bar();
;
; int k = init_k();
; int i = init_i();
;
; int main() {
;   foo();
;   return 0;
; }
;
; // This function will execute before main using __attribute__((constructor))
; void calls_before_main() {
;   bar_before_main();
; }
;
; // This function will execute after main using __attribute__((destructor))
; void calls_after_main() {
;   bar_after_main();
; }

; The main goal of this test case is to make sure that the functions traversal
; collects the correct functions:
;
;  call from main: foo
;  calls before main: calls_before_main, bar_before_main, init_i, init_k
;  calls after main: calls_after_main, bar_after_main
;  dead function (not collected): unused_bar
;

; RUN: opt < %s -disable-output -passes='require<wholeprogram>' -debug-only=whole-program-analysis 2>&1  | FileCheck %s

; CHECK:     WHOLE-PROGRAM-ANALYSIS
; CHECK:  LIBFUNCS NOT FOUND: 5
; CHECK:    foo
; CHECK:    bar_before_main
; CHECK:    bar_after_main
; CHECK:    init_k
; CHECK:    init_i
; CHECK:  VISIBLE OUTSIDE LTO: 2
; CHECK:    calls_before_main
; CHECK:    calls_after_main
; CHECK:  WHOLE PROGRAM RESULT:
; CHECK:    WHOLE PROGRAM SEEN:  NOT DETECTED
; CHECK-NOT: unused_bar

@k = dso_local global i32 0
@i = dso_local global i32 0
@_ZN1C1jE = dso_local global i32 0
@llvm.global_ctors = appending global [2 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @calls_before_main, i8* null }, { i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_example.cpp, i8* null }]
@llvm.global_dtors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @calls_after_main, i8* null }]

define internal void @__cxx_global_var_init() section ".text.startup" {
  %1 = call i32 @init_k()
  store i32 %1, i32* @k
  ret void
}

define internal void @__cxx_global_var_init.1() section ".text.startup" {
  %1 = call i32 @init_i()
  store i32 %1, i32* @i
  ret void
}

define dso_local i32 @main() {
  call void @foo()
  ret i32 0
}

define dso_local void @calls_before_main() {
  call void @bar_before_main()
  ret void
}

define dso_local void @calls_after_main() {
  call void @bar_after_main()
  ret void
}

define internal void @_GLOBAL__sub_I_example.cpp() section ".text.startup" {
  call void @__cxx_global_var_init()
  call void @__cxx_global_var_init.1()
  ret void
}

declare dso_local i32 @init_i()
declare dso_local i32 @init_k()
declare dso_local void @bar_before_main()
declare dso_local void @bar_after_main()
declare dso_local void @foo()
declare void @unused_bar()