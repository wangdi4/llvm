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



source_filename = "ld-temp.o"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%"class.std::ios_base::Init" = type { i8 }
%struct.Parent = type { i32 (...)** }

$_ZTI6Parent = comdat any

$_ZTI7Derived = comdat any

$_ZTS7Derived = comdat any

$_ZTS6Parent = comdat any

@llvm.global_ctors = appending global [1 x { i32, void ()*, i8* }] [{ i32, void ()*, i8* } { i32 65535, void ()* @_GLOBAL__sub_I_test.cpp, i8* null }]
@_ZStL8__ioinit = internal global %"class.std::ios_base::Init" zeroinitializer, align 1
@__dso_handle = external dso_local hidden global i8
@_ZTI6Parent = internal dso_local constant { i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv117__class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([8 x i8], [8 x i8]* @_ZTS6Parent, i32 0, i32 0) }, comdat
@_ZTI7Derived = internal dso_local constant { i8*, i8*, i8* } { i8* bitcast (i8** getelementptr inbounds (i8*, i8** @_ZTVN10__cxxabiv120__si_class_type_infoE, i64 2) to i8*), i8* getelementptr inbounds ([9 x i8], [9 x i8]* @_ZTS7Derived, i32 0, i32 0), i8* bitcast ({ i8*, i8* }* @_ZTI6Parent to i8*) }, comdat
@_ZTVN10__cxxabiv120__si_class_type_infoE = external global i8*
@_ZTS7Derived = internal dso_local constant [9 x i8] c"7Derived\00", comdat
@_ZTVN10__cxxabiv117__class_type_infoE = external global i8*
@_ZTS6Parent = internal dso_local constant [8 x i8] c"6Parent\00", comdat
@.str = private unnamed_addr constant [9 x i8] c"./lib.so\00", align 1
@.str.1 = private unnamed_addr constant [5 x i8] c"func\00", align 1

; Function Attrs: uwtable
define internal void @_GLOBAL__sub_I_test.cpp() #0 section ".text.startup" {
entry:
  tail call void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"* nonnull @_ZStL8__ioinit)
  %0 = tail call i32 @__cxa_atexit(void (i8*)* bitcast (void (%"class.std::ios_base::Init"*)* @_ZNSt8ios_base4InitD1Ev to void (i8*)*), i8* getelementptr inbounds (%"class.std::ios_base::Init", %"class.std::ios_base::Init"* @_ZStL8__ioinit, i64 0, i32 0), i8* nonnull @__dso_handle) #3
  ret void
}

declare void @_ZNSt8ios_base4InitC1Ev(%"class.std::ios_base::Init"*) unnamed_addr #1

; Function Attrs: nounwind
declare void @_ZNSt8ios_base4InitD1Ev(%"class.std::ios_base::Init"*) unnamed_addr #2

; Function Attrs: nounwind
declare i32 @__cxa_atexit(void (i8*)*, i8*, i8*) local_unnamed_addr #3

; Function Attrs: noinline nounwind readonly uwtable
define internal dso_local i32 @test(%struct.Parent* readonly %p) #4 {
; CHECK-LABEL: @test(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = icmp eq %struct.Parent* [[P:%.*]], null
; CHECK-NEXT:    br i1 [[TMP0]], label [[IF_END:%.*]], label [[DYNAMIC_CAST_NOTNULL:%.*]]
; CHECK:       dynamic_cast.notnull:
; CHECK-NEXT:    [[TMP1:%.*]] = bitcast %struct.Parent* [[P]] to i8*
; CHECK-NEXT:    [[TMP2:%.*]] = tail call i8* @__dynamic_cast(i8* [[TMP1]], i8* bitcast ({ i8*, i8* }* @_ZTI6Parent to i8*), i8* bitcast ({ i8*, i8*, i8* }* @_ZTI7Derived to i8*), i64 0) #3
; CHECK-NEXT:    [[PHITMP:%.*]] = icmp eq i8* [[TMP2]], null
; CHECK-NEXT:    br i1 [[PHITMP]], label [[IF_END]], label [[CLEANUP:%.*]]
; CHECK:       if.end:
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ -1, [[IF_END]] ], [ 0, [[DYNAMIC_CAST_NOTNULL]] ]
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;
entry:
  %0 = icmp eq %struct.Parent* %p, null
  br i1 %0, label %if.end, label %dynamic_cast.notnull

dynamic_cast.notnull:                             ; preds = %entry
  %1 = bitcast %struct.Parent* %p to i8*
  %2 = tail call i8* @__dynamic_cast(i8* %1, i8* bitcast ({ i8*, i8* }* @_ZTI6Parent to i8*), i8* bitcast ({ i8*, i8*, i8* }* @_ZTI7Derived to i8*), i64 0) #3
  %phitmp = icmp eq i8* %2, null
  br i1 %phitmp, label %if.end, label %cleanup

if.end:                                           ; preds = %dynamic_cast.notnull, %entry
  br label %cleanup

cleanup:                                          ; preds = %if.end, %dynamic_cast.notnull
  %retval.0 = phi i32 [ -1, %if.end ], [ 0, %dynamic_cast.notnull ]
  ret i32 %retval.0
}

; Function Attrs: nounwind readonly
declare i8* @__dynamic_cast(i8*, i8*, i8*, i64) local_unnamed_addr #5

; Function Attrs: norecurse uwtable
define dso_local i32 @main() #6 {
entry:
  %call = tail call i8* @dlopen(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i64 0, i64 0), i32 2) #3
  %call1 = tail call i8* @dlsym(i8* %call, i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.1, i64 0, i64 0)) #3
  %call2 = tail call i8* @dlerror() #3
  %tobool = icmp eq i8* %call2, null
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %call3 = tail call i32 @dlclose(i8* %call) #3
  br label %cleanup

if.end:                                           ; preds = %entry
  %0 = bitcast i8* %call1 to %struct.Parent* ()*
  %call4 = tail call %struct.Parent* %0()
  %call5 = tail call i32 @test(%struct.Parent* %call4)
  %call6 = tail call i32 @dlclose(i8* %call) #3
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ 1, %if.then ], [ %call5, %if.end ]
  ret i32 %retval.0
}

; Function Attrs: nounwind
declare i8* @dlopen(i8*, i32) local_unnamed_addr #2

; Function Attrs: nounwind
declare i8* @dlsym(i8*, i8*) local_unnamed_addr #2

; Function Attrs: nounwind
declare i8* @dlerror() local_unnamed_addr #2

; Function Attrs: nounwind
declare i32 @dlclose(i8*) local_unnamed_addr #2

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
