; RUN: opt < %s -S -passes=optimize-dyn-casts | FileCheck %s

; Test case with dlopen that loads dynamic library in runtime.
; It is expected that whole porgram will not be detected for this
; program and dynamic_cast optimization will not be performed.

; Source code:
; #include <iostream>
; #include <dlfcn.h>
;
; using namespace std;
;
; struct Parent {
;   virtual void f() {}
; };
;
; struct Derived : Parent {
;   virtual void g() {}
; };
;
;
; __attribute__((noinline))
; extern "C" int test(Parent *p) {
;   Derived *res = dynamic_cast<Derived *>(p);
;   if (res != nullptr) {
;     return 0;
;   }
;   return -1;
; }
;
; typedef Parent* (*FuncType)();
;
; int main(void) {
;   void *h = dlopen("./lib.so", RTLD_NOW);
;   auto func = (FuncType)dlsym(h, "func");
;   const char *dlsym_error = dlerror();
;   if (dlsym_error) {
;       dlclose(h);
;       return 1;
;   }
;
;   auto result = func();
;   int res = test(result);
;   dlclose(h);
;
;   return res;
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }

$_ZTI6Parent = comdat any

$_ZTI7Derived = comdat any

$_ZTS7Derived = comdat any

$_ZTS6Parent = comdat any

@llvm.global_ctors = appending global [1 x { i32, ptr, ptr }] [{ i32, ptr, ptr } { i32 65535, ptr @_GLOBAL__sub_I_test.cpp, ptr null }]
@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external hidden global i8
@_ZTI6Parent = internal constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS6Parent }, comdat
@_ZTI7Derived = internal constant { ptr, ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2), ptr @_ZTS7Derived, ptr @_ZTI6Parent }, comdat
@_ZTVN10__cxxabiv120__si_class_type_infoE = external global ptr
@_ZTS7Derived = internal constant [9 x i8] c"7Derived\00", comdat
@_ZTVN10__cxxabiv117__class_type_infoE = external global ptr
@_ZTS6Parent = internal constant [8 x i8] c"6Parent\00", comdat
@.str = private unnamed_addr constant [9 x i8] c"./lib.so\00", align 1
@.str.1 = private unnamed_addr constant [5 x i8] c"func\00", align 1

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_test.cpp() #0 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(ptr nonnull @_ZStL8__ioinit)
  %i = tail call i32 @__cxa_atexit(ptr @_ZNSt8ios_base4InitD1Ev, ptr @_ZStL8__ioinit, ptr nonnull @__dso_handle) #3
  ret void
}

declare void @_ZNSt8ios_base4InitC1Ev(ptr) unnamed_addr #1

; Function Attrs: nounwind
declare void @_ZNSt8ios_base4InitD1Ev(ptr) unnamed_addr #2

; Function Attrs: nounwind
declare i32 @__cxa_atexit(ptr, ptr, ptr) local_unnamed_addr #3

; Function Attrs: noinline nounwind readonly uwtable
define internal i32 @test(ptr readonly %p) #4 {
; CHECK-LABEL: @test(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = icmp eq ptr [[P:%.*]], null
; CHECK-NEXT:    br i1 [[TMP0]], label [[IF_END:%.*]], label [[DYNAMIC_CAST_NOTNULL:%.*]]
; CHECK:       dynamic_cast.notnull:
; CHECK-NEXT:    [[TMP2:%.*]] = tail call ptr @__dynamic_cast(ptr [[P]], ptr @_ZTI6Parent, ptr @_ZTI7Derived, i64 0) #3
; CHECK-NEXT:    [[PHITMP:%.*]] = icmp eq ptr [[TMP2]], null
; CHECK-NEXT:    br i1 [[PHITMP]], label [[IF_END]], label [[CLEANUP:%.*]]
; CHECK:       if.end:
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ -1, [[IF_END]] ], [ 0, [[DYNAMIC_CAST_NOTNULL]] ]
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;
entry:
  %i = icmp eq ptr %p, null
  br i1 %i, label %if.end, label %dynamic_cast.notnull

dynamic_cast.notnull:                             ; preds = %entry
  %i2 = tail call ptr @__dynamic_cast(ptr %p, ptr @_ZTI6Parent, ptr @_ZTI7Derived, i64 0) #3
  %phitmp = icmp eq ptr %i2, null
  br i1 %phitmp, label %if.end, label %cleanup

if.end:                                           ; preds = %dynamic_cast.notnull, %entry
  br label %cleanup

cleanup:                                          ; preds = %if.end, %dynamic_cast.notnull
  %retval.0 = phi i32 [ -1, %if.end ], [ 0, %dynamic_cast.notnull ]
  ret i32 %retval.0
}

; Function Attrs: nounwind readonly
declare ptr @__dynamic_cast(ptr, ptr, ptr, i64) local_unnamed_addr #5

; Function Attrs: norecurse uwtable
define dso_local i32 @main() #6 {
entry:
  %call = tail call ptr @dlopen(ptr @.str, i32 2) #3
  %call1 = tail call ptr @dlsym(ptr %call, ptr @.str.1) #3
  %call2 = tail call ptr @dlerror() #3
  %tobool = icmp eq ptr %call2, null
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %call3 = tail call i32 @dlclose(ptr %call) #3
  br label %cleanup

if.end:                                           ; preds = %entry
  %call4 = tail call ptr %call1()
  %call5 = tail call i32 @test(ptr %call4)
  %call6 = tail call i32 @dlclose(ptr %call) #3
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ 1, %if.then ], [ %call5, %if.end ]
  ret i32 %retval.0
}

; Function Attrs: nounwind
declare ptr @dlopen(ptr, i32) local_unnamed_addr #2

; Function Attrs: nounwind
declare ptr @dlsym(ptr, ptr) local_unnamed_addr #2

; Function Attrs: nounwind
declare ptr @dlerror() local_unnamed_addr #2

; Function Attrs: nounwind
declare i32 @dlclose(ptr) local_unnamed_addr #2

attributes #0 = { uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { nounwind }
attributes #4 = { noinline nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { nounwind readonly }
attributes #6 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}
!llvm.module.flags = !{!1}

!0 = !{!"clang version 6.0.0"}
!1 = !{i32 1, !"wchar_size", i32 4}
