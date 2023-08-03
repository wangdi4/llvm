; RUN: opt < %s -S -passes=optimize-dyn-casts -whole-program-assume -disable-whole-program-visibility | FileCheck %s

; Source code:
;
; class GrandParent {
;  public:
;   virtual int f() { return 30; };
; };
;
; class VirtualGrandParent {
;  public:
;   virtual void g(){};
; };
;
; class Parent1 : public GrandParent, public virtual VirtualGrandParent {};
;
; class Parent2 : public GrandParent, public virtual VirtualGrandParent {};
;
; class Derived1 : public Parent1, public Parent2 {
;  public:
;   int f() { return 60; }
; };
;
; class Parent3 {
;   public:
;     virtual int k() { return 100; }
; };
;
; class Derived2 : public Parent3{
;  public:
;   int k() { return 50; }
; };
;
; __attribute__((noinline))
; extern "C" int test1(Parent1 * p) {
;   Derived1 *res = dynamic_cast<Derived1 *>(p);
;   if (res != nullptr) {
;     return 1;
;   }
;   return 0;
; }
;
; __attribute__((noinline))
; extern "C" int test2(Parent1 * p) {
;   Parent2 *res = dynamic_cast<Parent2 *>(p);
;   if (res != nullptr) {
;     return 2;
;   }
;   return 0;
; }
;
; __attribute__((noinline))
; extern "C" int test3(VirtualGrandParent * p) {
;   Derived1 *res = dynamic_cast<Derived1 *>(p);
;   if (res != nullptr) {
;     return 3;
;   }
;   return 0;
; }
;
; __attribute__((noinline))
; extern "C" int test4(GrandParent * p) {
;   Derived1 *res = dynamic_cast<Derived1 *>(p);
;   if (res != nullptr) {
;     return 4;
;   }
;   return 0;
; }
;
; __attribute__((noinline))
; extern "C" int test5(Parent2 * p) {
;   Derived1 *res = dynamic_cast<Derived1 *>(p);
;   if (res != nullptr) {
;     return res->f();
;   }
;   return 0;
; }
;
; __attribute__((noinline))
; extern "C" int test6(VirtualGrandParent * p) {
;   Derived1 *res = dynamic_cast<Derived1 *>(p);
;   if (res != nullptr) {
;     return res->f();
;   }
;   return 0;
; }
;
; __attribute__((noinline))
; extern "C" int test7(GrandParent * p) {
;   Parent1 *res = dynamic_cast<Parent1 *>(p);
;   if (res != nullptr) {
;     return 7;
;   }
;   return 0;
; }
;
; int main() {
;   int res = 0;
;   Derived1 d;
;   Parent1 *p1 = &d;
;
;   res += test1(p1);
;   res += test2(p1);
;
;   VirtualGrandParent *vgp1 = &d;
;   res += test3(vgp1);
;
;   GrandParent *gp1 = p1;
;   res += test4(gp1);
;
;   Parent2 *p2 = &d;
;   res += test5(p2);
;
;   VirtualGrandParent *vgp2 = &d;
;   res += test6(vgp2);
;
;   Parent1 p;
;   GrandParent *gp2 = &p;
;   res += test7(gp2);
;
;   return res;
; }

; Classes hierarchy:
;
; +--------------------+          +---------------+
; |                    |          |               |
; | VirtualGrandParent |          |  GrandParent  |
; |                    |          |               |
; +------+-----------+-+          +-+---------+---+
;        |           |              |         |
;     Virtual        |              |         |
;        |      Virtual             |         |
;        |           |              |         |
;  +-----v-----+     |              |    +----v------+
;  |           |     |              |    |           |
;  |  Parent1  |     +-------------------> Parent2   |
;  |           | <------------------+    |           |
;  +------+----+                         +-----+-----+
;         |                                    |
;         |                                    |
;         |                                    |
;         |             +------------+         |
;         |             |            |         |
;         +------------->  Derived1 <----------+
;                       |            |
;                       +------------+

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.Derived1 = type { %class.Parent1.base, %class.Parent1.base, %class.GrandParent }
%class.Parent1.base = type { %class.GrandParent }
%class.GrandParent = type { ptr }
%class.Parent1 = type { %class.GrandParent, %class.GrandParent }

$_ZN11GrandParent1fEv = comdat any

$_ZN18VirtualGrandParent1gEv = comdat any

$_ZN8Derived11fEv = comdat any

$_ZThn8_N8Derived11fEv = comdat any

$_ZTI7Parent1 = comdat any

$_ZTI8Derived1 = comdat any

$_ZTS8Derived1 = comdat any

$_ZTI7Parent2 = comdat any

$_ZTS7Parent2 = comdat any

$_ZTI11GrandParent = comdat any

$_ZTI18VirtualGrandParent = comdat any

$_ZTS18VirtualGrandParent = comdat any

$_ZTS11GrandParent = comdat any

$_ZTS7Parent1 = comdat any

$_ZTV8Derived1 = comdat any

$_ZTV7Parent1 = comdat any

@_ZTI7Parent1 = internal constant { ptr, ptr, i32, i32, ptr, i64, ptr, i64 } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2), ptr @_ZTS7Parent1, i32 0, i32 2, ptr @_ZTI11GrandParent, i64 2, ptr @_ZTI18VirtualGrandParent, i64 -6141 }, comdat
@_ZTI8Derived1 = internal constant { ptr, ptr, i32, i32, ptr, i64, ptr, i64 } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2), ptr @_ZTS8Derived1, i32 3, i32 2, ptr @_ZTI7Parent1, i64 2, ptr @_ZTI7Parent2, i64 2050 }, comdat
@_ZTVN10__cxxabiv121__vmi_class_type_infoE = external global ptr
@_ZTS8Derived1 = internal constant [10 x i8] c"8Derived1\00", comdat
@_ZTI7Parent2 = internal constant { ptr, ptr, i32, i32, ptr, i64, ptr, i64 } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv121__vmi_class_type_infoE, i64 2), ptr @_ZTS7Parent2, i32 0, i32 2, ptr @_ZTI11GrandParent, i64 2, ptr @_ZTI18VirtualGrandParent, i64 -6141 }, comdat
@_ZTS7Parent2 = internal constant [9 x i8] c"7Parent2\00", comdat
@_ZTI11GrandParent = internal constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS11GrandParent }, comdat
@_ZTI18VirtualGrandParent = internal constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr @_ZTS18VirtualGrandParent }, comdat
@_ZTVN10__cxxabiv117__class_type_infoE = external global ptr
@_ZTS18VirtualGrandParent = internal constant [21 x i8] c"18VirtualGrandParent\00", comdat
@_ZTS11GrandParent = internal constant [14 x i8] c"11GrandParent\00", comdat
@_ZTS7Parent1 = internal constant [9 x i8] c"7Parent1\00", comdat
@_ZTV8Derived1 = internal unnamed_addr constant { [4 x ptr], [4 x ptr], [4 x ptr] } { [4 x ptr] [ptr inttoptr (i64 16 to ptr), ptr null, ptr @_ZTI8Derived1, ptr @_ZN8Derived11fEv], [4 x ptr] [ptr inttoptr (i64 8 to ptr), ptr inttoptr (i64 -8 to ptr), ptr @_ZTI8Derived1, ptr @_ZThn8_N8Derived11fEv], [4 x ptr] [ptr null, ptr inttoptr (i64 -16 to ptr), ptr @_ZTI8Derived1, ptr @_ZN18VirtualGrandParent1gEv] }, comdat, align 8, !type !0, !type !1, !type !2, !type !3, !type !4, !type !5
@_ZTV7Parent1 = internal unnamed_addr constant { [4 x ptr], [4 x ptr] } { [4 x ptr] [ptr inttoptr (i64 8 to ptr), ptr null, ptr @_ZTI7Parent1, ptr @_ZN11GrandParent1fEv], [4 x ptr] [ptr null, ptr inttoptr (i64 -8 to ptr), ptr @_ZTI7Parent1, ptr @_ZN18VirtualGrandParent1gEv] }, comdat, align 8, !type !0, !type !6, !type !3

; Derived1 class is final.
; Hint >= 0 so we can optmimize this dynamic_cast.
; Function Attrs: noinline nounwind readonly uwtable
define internal i32 @test1(ptr readonly %p) #0 {
; CHECK-LABEL: @test1(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = icmp eq ptr [[P:%.*]], null
; CHECK-NEXT:    br i1 [[TMP0]], label [[IF_END:%.*]], label [[DYNAMIC_CAST_NOTNULL:%.*]]
; CHECK:       dynamic_cast.notnull:
; CHECK-NEXT:    [[TMP3:%.*]] = load ptr, ptr [[P]]
; CHECK-NEXT:    [[TMP4:%.*]] = getelementptr ptr, ptr [[TMP3]], i32 -1
; CHECK-NEXT:    [[TMP5:%.*]] = load ptr, ptr [[TMP4]]
; CHECK-NEXT:    [[TMP6:%.*]] = icmp eq ptr [[TMP5]], @_ZTI8Derived1
; CHECK-NEXT:    [[TMP7:%.*]] = select i1 [[TMP6]], ptr [[P]], ptr null
; CHECK-NEXT:    [[TMP8:%.*]] = tail call ptr @__dynamic_cast(ptr [[P]], ptr @_ZTI7Parent1, ptr @_ZTI8Derived1, i64 0) #6
; CHECK-NEXT:    [[PHITMP:%.*]] = icmp eq ptr [[TMP7]], null
; CHECK-NEXT:    br i1 [[PHITMP]], label [[IF_END]], label [[CLEANUP:%.*]]
; CHECK:       if.end:
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ 0, [[IF_END]] ], [ 1, [[DYNAMIC_CAST_NOTNULL]] ]
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;
entry:
  %i = icmp eq ptr %p, null
  br i1 %i, label %if.end, label %dynamic_cast.notnull

dynamic_cast.notnull:                             ; preds = %entry
  %i2 = tail call ptr @__dynamic_cast(ptr %p, ptr @_ZTI7Parent1, ptr @_ZTI8Derived1, i64 0) #6
  %phitmp = icmp eq ptr %i2, null
  br i1 %phitmp, label %if.end, label %cleanup

if.end:                                           ; preds = %dynamic_cast.notnull, %entry
  br label %cleanup

cleanup:                                          ; preds = %if.end, %dynamic_cast.notnull
  %retval.0 = phi i32 [ 0, %if.end ], [ 1, %dynamic_cast.notnull ]
  ret i32 %retval.0
}

; Function Attrs: nounwind readonly
declare ptr @__dynamic_cast(ptr, ptr, ptr, i64) local_unnamed_addr #1

; Parent2 is not final, optimization will not be performed.
; Function Attrs: noinline nounwind readonly uwtable
define internal i32 @test2(ptr readonly %p) #0 {
; CHECK-LABEL: @test2(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = icmp eq ptr [[P:%.*]], null
; CHECK-NEXT:    br i1 [[TMP0]], label [[IF_END:%.*]], label [[DYNAMIC_CAST_NOTNULL:%.*]]
; CHECK:       dynamic_cast.notnull:
; CHECK-NEXT:    [[TMP2:%.*]] = tail call ptr @__dynamic_cast(ptr [[P]], ptr @_ZTI7Parent1, ptr @_ZTI7Parent2, i64 -2) #6
; CHECK-NEXT:    [[PHITMP:%.*]] = icmp eq ptr [[TMP2]], null
; CHECK-NEXT:    br i1 [[PHITMP]], label [[IF_END]], label [[CLEANUP:%.*]]
; CHECK:       if.end:
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ 0, [[IF_END]] ], [ 2, [[DYNAMIC_CAST_NOTNULL]] ]
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;

entry:
  %i = icmp eq ptr %p, null
  br i1 %i, label %if.end, label %dynamic_cast.notnull

dynamic_cast.notnull:                             ; preds = %entry
  %i2 = tail call ptr @__dynamic_cast(ptr %p, ptr @_ZTI7Parent1, ptr @_ZTI7Parent2, i64 -2) #6
  %phitmp = icmp eq ptr %i2, null
  br i1 %phitmp, label %if.end, label %cleanup

if.end:                                           ; preds = %dynamic_cast.notnull, %entry
  br label %cleanup

cleanup:                                          ; preds = %if.end, %dynamic_cast.notnull
  %retval.0 = phi i32 [ 0, %if.end ], [ 2, %dynamic_cast.notnull ]
  ret i32 %retval.0
}

; Derived1 class is final. Hint < 0 so we are not able to easily calculate
; casted pointer ourselves. That is why we need to check that we don't need
; the result of dynamic_cast, but we only need to know if it is successful or
; not. Here this condition is satisfied, that is why call will be optimized.
; Function Attrs: noinline nounwind readonly uwtable
define internal i32 @test3(ptr readonly %p) #0 {
; CHECK-LABEL: @test3(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = icmp eq ptr [[P:%.*]], null
; CHECK-NEXT:    br i1 [[TMP0]], label [[IF_END:%.*]], label [[DYNAMIC_CAST_NOTNULL:%.*]]
; CHECK:       dynamic_cast.notnull:
; CHECK-NEXT:    [[TMP3:%.*]] = load ptr, ptr [[P]]
; CHECK-NEXT:    [[TMP4:%.*]] = getelementptr ptr, ptr [[TMP3]], i32 -1
; CHECK-NEXT:    [[TMP5:%.*]] = load ptr, ptr [[TMP4]]
; CHECK-NEXT:    [[TMP6:%.*]] = icmp ne ptr [[TMP5]], @_ZTI8Derived1
; CHECK-NEXT:    [[TMP7:%.*]] = tail call ptr @__dynamic_cast(ptr [[P]], ptr @_ZTI18VirtualGrandParent, ptr @_ZTI8Derived1, i64 -1) #6
; CHECK-NEXT:    [[PHITMP:%.*]] = icmp eq ptr [[TMP7]], null
; CHECK-NEXT:    br i1 [[TMP6]], label [[IF_END]], label [[CLEANUP:%.*]]
; CHECK:       if.end:
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ 0, [[IF_END]] ], [ 3, [[DYNAMIC_CAST_NOTNULL]] ]
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;
entry:
  %i = icmp eq ptr %p, null
  br i1 %i, label %if.end, label %dynamic_cast.notnull

dynamic_cast.notnull:                             ; preds = %entry
  %i2 = tail call ptr @__dynamic_cast(ptr %p, ptr @_ZTI18VirtualGrandParent, ptr @_ZTI8Derived1, i64 -1) #6
  %phitmp = icmp eq ptr %i2, null
  br i1 %phitmp, label %if.end, label %cleanup

if.end:                                           ; preds = %dynamic_cast.notnull, %entry
  br label %cleanup

cleanup:                                          ; preds = %if.end, %dynamic_cast.notnull
  %retval.0 = phi i32 [ 0, %if.end ], [ 3, %dynamic_cast.notnull ]
  ret i32 %retval.0
}

; Derived1 class is final. Hint < 0 so we are not able to easily calculate
; casted pointer ourselves. That is why we need to check that we don't need
; the result of dynamic_cast, but we only need to know if it is successful or
; not. Here this condition is satisfied, that is why call will be optimized.
; Function Attrs: noinline nounwind readonly uwtable
; Function Attrs: noinline nounwind readonly uwtable
define internal i32 @test4(ptr readonly %p) #0 {
; CHECK-LABEL: @test4(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = icmp eq ptr [[P:%.*]], null
; CHECK-NEXT:    br i1 [[TMP0]], label [[IF_END:%.*]], label [[DYNAMIC_CAST_NOTNULL:%.*]]
; CHECK:       dynamic_cast.notnull:
; CHECK-NEXT:    [[TMP3:%.*]] = load ptr, ptr  [[P]]
; CHECK-NEXT:    [[TMP4:%.*]] = getelementptr ptr, ptr [[TMP3]], i32 -1
; CHECK-NEXT:    [[TMP5:%.*]] = load ptr, ptr [[TMP4]]
; CHECK-NEXT:    [[TMP6:%.*]] = icmp ne ptr [[TMP5]], @_ZTI8Derived1
; CHECK-NEXT:    [[TMP7:%.*]] = tail call ptr @__dynamic_cast(ptr [[P]], ptr @_ZTI11GrandParent, ptr @_ZTI8Derived1, i64 -3) #6
; CHECK-NEXT:    [[PHITMP:%.*]] = icmp eq ptr [[TMP7]], null
; CHECK-NEXT:    br i1 [[TMP6]], label [[IF_END]], label [[CLEANUP:%.*]]
; CHECK:       if.end:
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ 0, [[IF_END]] ], [ 4, [[DYNAMIC_CAST_NOTNULL]] ]
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;
entry:
  %i = icmp eq ptr %p, null
  br i1 %i, label %if.end, label %dynamic_cast.notnull

dynamic_cast.notnull:                             ; preds = %entry
  %i2 = tail call ptr @__dynamic_cast(ptr %p, ptr @_ZTI11GrandParent, ptr @_ZTI8Derived1, i64 -3) #6
  %phitmp = icmp eq ptr %i2, null
  br i1 %phitmp, label %if.end, label %cleanup

if.end:                                           ; preds = %dynamic_cast.notnull, %entry
  br label %cleanup

cleanup:                                          ; preds = %if.end, %dynamic_cast.notnull
  %retval.0 = phi i32 [ 0, %if.end ], [ 4, %dynamic_cast.notnull ]
  ret i32 %retval.0
}

; Derived1 class is final.
; We can notice here that result of dynamic_cast res5 is needed because
; res5->Foo() call is made using it. Hint > 0 so we will be able to easily
; calculate result of dynamic_cast = p2 - hint. That is why optimization
; could be performed.
; Function Attrs: noinline uwtable
define internal i32 @test5(ptr %p) #2 {
; CHECK-LABEL: @test5(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = icmp eq ptr [[P:%.*]], null
; CHECK-NEXT:    br i1 [[TMP0]], label [[CLEANUP:%.*]], label [[DYNAMIC_CAST_NOTNULL:%.*]]
; CHECK:       dynamic_cast.notnull:
; CHECK-NEXT:    [[TMP3:%.*]] = load ptr, ptr [[P]]
; CHECK-NEXT:    [[TMP4:%.*]] = getelementptr ptr, ptr [[TMP3]], i32 -1
; CHECK-NEXT:    [[TMP5:%.*]] = load ptr, ptr [[TMP4]]
; CHECK-NEXT:    [[TMP6:%.*]] = icmp eq ptr [[TMP5]], @_ZTI8Derived1
; CHECK-NEXT:    [[TMP7:%.*]] = getelementptr i8, ptr [[P]], i64 -8
; CHECK-NEXT:    [[TMP8:%.*]] = select i1 [[TMP6]], ptr [[TMP7]], ptr null
; CHECK-NEXT:    [[TMP9:%.*]] = tail call ptr @__dynamic_cast(ptr [[P]], ptr @_ZTI7Parent2, ptr @_ZTI8Derived1, i64 8) #6
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq ptr [[TMP8]], null
; CHECK-NEXT:    br i1 [[CMP]], label [[CLEANUP]], label [[IF_THEN:%.*]]
; CHECK:       if.then:
; CHECK-NEXT:    [[VTABLE:%.*]] = load ptr, ptr [[TMP8]], align 8, !tbaa !9
; CHECK-NEXT:    [[TMP12:%.*]] = load ptr, ptr [[VTABLE]], align 8
; CHECK-NEXT:    [[CALL:%.*]] = tail call i32 [[TMP12]](ptr nonnull [[TMP8]])
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ [[CALL]], [[IF_THEN]] ], [ 0, [[ENTRY:%.*]] ], [ 0, [[DYNAMIC_CAST_NOTNULL]] ]
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;
entry:
  %i = icmp eq ptr %p, null
  br i1 %i, label %cleanup, label %dynamic_cast.notnull

dynamic_cast.notnull:                             ; preds = %entry
  %i2 = tail call ptr @__dynamic_cast(ptr %p, ptr @_ZTI7Parent2, ptr @_ZTI8Derived1, i64 8) #6
  %cmp = icmp eq ptr %i2, null
  br i1 %cmp, label %cleanup, label %if.then

if.then:                                          ; preds = %dynamic_cast.notnull
  %vtable = load ptr, ptr %i2, align 8, !tbaa !9
  %i5 = load ptr, ptr %vtable, align 8
  %call = tail call i32 %i5(ptr nonnull %i2)
  br label %cleanup

cleanup:                                          ; preds = %if.then, %dynamic_cast.notnull, %entry
  %retval.0 = phi i32 [ %call, %if.then ], [ 0, %entry ], [ 0, %dynamic_cast.notnull ]
  ret i32 %retval.0
}

; Negative hint and there are other uses than comparison with null that is
; why dynamic_cast will not be optimized.
; Function Attrs: noinline uwtable
define internal i32 @test6(ptr %p) #2 {
; CHECK-LABEL: @test6(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = icmp eq ptr [[P:%.*]], null
; CHECK-NEXT:    br i1 [[TMP0]], label [[CLEANUP:%.*]], label [[DYNAMIC_CAST_NOTNULL:%.*]]
; CHECK:       dynamic_cast.notnull:
; CHECK-NEXT:    [[TMP2:%.*]] = tail call ptr @__dynamic_cast(ptr [[P]], ptr @_ZTI18VirtualGrandParent, ptr @_ZTI8Derived1, i64 -1) #6
; CHECK-NEXT:    [[CMP:%.*]] = icmp eq ptr [[TMP2]], null
; CHECK-NEXT:    br i1 [[CMP]], label [[CLEANUP]], label [[IF_THEN:%.*]]
; CHECK:       if.then:
; CHECK-NEXT:    [[VTABLE:%.*]] = load ptr, ptr [[TMP2]], align 8, !tbaa !9
; CHECK-NEXT:    [[TMP5:%.*]] = load ptr, ptr  [[VTABLE]], align 8
; CHECK-NEXT:    [[CALL:%.*]] = tail call i32 [[TMP5]](ptr nonnull [[TMP2]])
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ [[CALL]], [[IF_THEN]] ], [ 0, [[ENTRY:%.*]] ], [ 0, [[DYNAMIC_CAST_NOTNULL]] ]
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;
entry:
  %i = icmp eq ptr %p, null
  br i1 %i, label %cleanup, label %dynamic_cast.notnull

dynamic_cast.notnull:                             ; preds = %entry
  %i2 = tail call ptr @__dynamic_cast(ptr %p, ptr @_ZTI18VirtualGrandParent, ptr @_ZTI8Derived1, i64 -1) #6
  %cmp = icmp eq ptr %i2, null
  br i1 %cmp, label %cleanup, label %if.then

if.then:                                          ; preds = %dynamic_cast.notnull
  %vtable = load ptr, ptr %i2, align 8, !tbaa !9
  %i5 = load ptr, ptr %vtable, align 8
  %call = tail call i32 %i5(ptr nonnull %i2)
  br label %cleanup

cleanup:                                          ; preds = %if.then, %dynamic_cast.notnull, %entry
  %retval.0 = phi i32 [ %call, %if.then ], [ 0, %entry ], [ 0, %dynamic_cast.notnull ]
  ret i32 %retval.0
}

; Parent1 is not final class. Optimization will not be performed.
; Function Attrs: noinline nounwind readonly uwtable
define internal i32 @test7(ptr readonly %p) #0 {
; CHECK-LABEL: @test7(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = icmp eq ptr [[P:%.*]], null
; CHECK-NEXT:    br i1 [[TMP0]], label [[IF_END:%.*]], label [[DYNAMIC_CAST_NOTNULL:%.*]]
; CHECK:       dynamic_cast.notnull:
; CHECK-NEXT:    [[TMP2:%.*]] = tail call ptr @__dynamic_cast(ptr [[P]], ptr @_ZTI11GrandParent, ptr @_ZTI7Parent1, i64 0) #6
; CHECK-NEXT:    [[PHITMP:%.*]] = icmp eq ptr [[TMP2]], null
; CHECK-NEXT:    br i1 [[PHITMP]], label [[IF_END]], label [[CLEANUP:%.*]]
; CHECK:       if.end:
; CHECK-NEXT:    br label [[CLEANUP]]
; CHECK:       cleanup:
; CHECK-NEXT:    [[RETVAL_0:%.*]] = phi i32 [ 0, [[IF_END]] ], [ 7, [[DYNAMIC_CAST_NOTNULL]] ]
; CHECK-NEXT:    ret i32 [[RETVAL_0]]
;
entry:
  %i = icmp eq ptr %p, null
  br i1 %i, label %if.end, label %dynamic_cast.notnull

dynamic_cast.notnull:                             ; preds = %entry
  %i2 = tail call ptr @__dynamic_cast(ptr %p, ptr @_ZTI11GrandParent, ptr @_ZTI7Parent1, i64 0) #6
  %phitmp = icmp eq ptr %i2, null
  br i1 %phitmp, label %if.end, label %cleanup

if.end:                                           ; preds = %dynamic_cast.notnull, %entry
  br label %cleanup

cleanup:                                          ; preds = %if.end, %dynamic_cast.notnull
  %retval.0 = phi i32 [ 0, %if.end ], [ 7, %dynamic_cast.notnull ]
  ret i32 %retval.0
}

; Function Attrs: norecurse uwtable
define dso_local i32 @main() #3 {
; CHECK-LABEL: @main(
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[D:%.*]] = alloca [[CLASS_DERIVED1:%.*]], align 8
; CHECK-NEXT:    [[P:%.*]] = alloca [[CLASS_PARENT1:%.*]], align 8
; CHECK-NEXT:    call void @llvm.lifetime.start.p0(i64 24, ptr nonnull [[D]]) #6
; CHECK-NEXT:    [[TMP1:%.*]] = getelementptr inbounds [[CLASS_DERIVED1]], ptr [[D]], i64 0, i32 2, i32 0
; CHECK-NEXT:    [[TMP2:%.*]] = getelementptr inbounds [[CLASS_DERIVED1]], ptr [[D]], i64 0, i32 0, i32 0, i32 0
; CHECK-NEXT:    [[TMP3:%.*]] = getelementptr inbounds [[CLASS_DERIVED1]], ptr [[D]], i64 0, i32 1, i32 0, i32 0
; CHECK-NEXT:    store ptr getelementptr inbounds ({ [4 x ptr], [4 x ptr], [4 x ptr] }, ptr @_ZTV8Derived1, i64 0, inrange i32 0, i64 3), ptr [[TMP2]], align 8, !tbaa !9
; CHECK-NEXT:    store ptr getelementptr inbounds ({ [4 x ptr], [4 x ptr], [4 x ptr] }, ptr @_ZTV8Derived1, i64 0, inrange i32 2, i64 3), ptr [[TMP1]], align 8, !tbaa !9
; CHECK-NEXT:    store ptr getelementptr inbounds ({ [4 x ptr], [4 x ptr], [4 x ptr] }, ptr @_ZTV8Derived1, i64 0, inrange i32 1, i64 3), ptr [[TMP3]], align 8, !tbaa !9
; CHECK-NEXT:    [[CALL:%.*]] = call i32 @test1(ptr nonnull [[D]])
; CHECK-NEXT:    [[CALL1:%.*]] = call i32 @test2(ptr nonnull [[D]])
; CHECK-NEXT:    [[ADD2:%.*]] = add nsw i32 [[CALL1]], [[CALL]]
; CHECK-NEXT:    [[ADD_PTR:%.*]] = getelementptr inbounds [[CLASS_DERIVED1]], ptr [[D]], i64 0, i32 2
; CHECK-NEXT:    [[CALL3:%.*]] = call i32 @test3(ptr nonnull [[ADD_PTR]])
; CHECK-NEXT:    [[ADD4:%.*]] = add nsw i32 [[ADD2]], [[CALL3]]
; CHECK-NEXT:    [[TMP6:%.*]] = getelementptr inbounds [[CLASS_DERIVED1]], ptr [[D]], i64 0, i32 0, i32 0
; CHECK-NEXT:    [[CALL5:%.*]] = call i32 @test4(ptr nonnull [[TMP6]])
; CHECK-NEXT:    [[ADD6:%.*]] = add nsw i32 [[ADD4]], [[CALL5]]
; CHECK-NEXT:    [[ADD_PTR8:%.*]] = getelementptr inbounds [[CLASS_DERIVED1]], ptr [[D]], i64 0, i32 1
; CHECK-NEXT:    [[CALL11:%.*]] = call i32 @test5(ptr nonnull [[ADD_PTR8]])
; CHECK-NEXT:    [[ADD12:%.*]] = add nsw i32 [[ADD6]], [[CALL11]]
; CHECK-NEXT:    [[VTABLE14:%.*]] = load ptr, ptr [[D]], align 8, !tbaa !9
; CHECK-NEXT:    [[VBASE_OFFSET_PTR15:%.*]] = getelementptr i8, ptr [[VTABLE14]], i64 -24
; CHECK-NEXT:    [[VBASE_OFFSET16:%.*]] = load i64, ptr [[VBASE_OFFSET_PTR15]], align 8
; CHECK-NEXT:    [[ADD_PTR17:%.*]] = getelementptr inbounds i8, ptr [[D]], i64 [[VBASE_OFFSET16]]
; CHECK-NEXT:    [[CALL20:%.*]] = call i32 @test6(ptr [[ADD_PTR17]])
; CHECK-NEXT:    [[ADD21:%.*]] = add nsw i32 [[ADD12]], [[CALL20]]
; CHECK-NEXT:    call void @llvm.lifetime.start.p0(i64 16, ptr nonnull [[P]]) #6
; CHECK-NEXT:    [[TMP11:%.*]] = getelementptr inbounds [[CLASS_PARENT1]], ptr [[P]], i64 0, i32 1, i32 0
; CHECK-NEXT:    [[TMP12:%.*]] = getelementptr inbounds [[CLASS_PARENT1]], ptr [[P]], i64 0, i32 0, i32 0
; CHECK-NEXT:    store ptr getelementptr inbounds ({ [4 x ptr], [4 x ptr] }, ptr @_ZTV7Parent1, i64 0, inrange i32 0, i64 3), ptr [[TMP12]], align 8, !tbaa !9
; CHECK-NEXT:    store ptr getelementptr inbounds ({ [4 x ptr], [4 x ptr] }, ptr @_ZTV7Parent1, i64 0, inrange i32 1, i64 3), ptr [[TMP11]], align 8, !tbaa !9
; CHECK-NEXT:    [[TMP13:%.*]] = getelementptr inbounds [[CLASS_PARENT1]], ptr [[P]], i64 0, i32 0
; CHECK-NEXT:    [[CALL22:%.*]] = call i32 @test7(ptr nonnull [[TMP13]])
; CHECK-NEXT:    [[ADD23:%.*]] = add nsw i32 [[ADD21]], [[CALL22]]
; CHECK-NEXT:    call void @llvm.lifetime.end.p0(i64 16, ptr nonnull [[P]]) #6
; CHECK-NEXT:    call void @llvm.lifetime.end.p0(i64 24, ptr nonnull [[D]]) #6
; CHECK-NEXT:    ret i32 [[ADD23]]
entry:
  %d = alloca %class.Derived1, align 8
  %p = alloca %class.Parent1, align 8
  call void @llvm.lifetime.start.p0(i64 24, ptr nonnull %d) #6
  %i1 = getelementptr inbounds %class.Derived1, ptr %d, i64 0, i32 2, i32 0
  %i2 = getelementptr inbounds %class.Derived1, ptr %d, i64 0, i32 0, i32 0, i32 0
  %i3 = getelementptr inbounds %class.Derived1, ptr %d, i64 0, i32 1, i32 0, i32 0
  store ptr getelementptr inbounds ({ [4 x ptr], [4 x ptr], [4 x ptr] }, ptr @_ZTV8Derived1, i64 0, inrange i32 0, i64 3), ptr %i2, align 8, !tbaa !9
  store ptr getelementptr inbounds ({ [4 x ptr], [4 x ptr], [4 x ptr] }, ptr @_ZTV8Derived1, i64 0, inrange i32 2, i64 3), ptr %i1, align 8, !tbaa !9
  store ptr getelementptr inbounds ({ [4 x ptr], [4 x ptr], [4 x ptr] }, ptr @_ZTV8Derived1, i64 0, inrange i32 1, i64 3), ptr %i3, align 8, !tbaa !9
  %call = call i32 @test1(ptr nonnull %d)
  %call1 = call i32 @test2(ptr nonnull %d)
  %add2 = add nsw i32 %call1, %call
  %add.ptr = getelementptr inbounds %class.Derived1, ptr %d, i64 0, i32 2
  %call3 = call i32 @test3(ptr nonnull %add.ptr)
  %add4 = add nsw i32 %add2, %call3
  %i6 = getelementptr inbounds %class.Derived1, ptr %d, i64 0, i32 0, i32 0
  %call5 = call i32 @test4(ptr nonnull %i6)
  %add6 = add nsw i32 %add4, %call5
  %add.ptr8 = getelementptr inbounds %class.Derived1, ptr %d, i64 0, i32 1
  %call11 = call i32 @test5(ptr nonnull %add.ptr8)
  %add12 = add nsw i32 %add6, %call11
  %vtable14 = load ptr, ptr %d, align 8, !tbaa !9
  %vbase.offset.ptr15 = getelementptr i8, ptr %vtable14, i64 -24
  %vbase.offset16 = load i64, ptr %vbase.offset.ptr15, align 8
  %add.ptr17 = getelementptr inbounds i8, ptr %d, i64 %vbase.offset16
  %call20 = call i32 @test6(ptr %add.ptr17)
  %add21 = add nsw i32 %add12, %call20
  call void @llvm.lifetime.start.p0(i64 16, ptr nonnull %p) #6
  %i11 = getelementptr inbounds %class.Parent1, ptr %p, i64 0, i32 1, i32 0
  %i12 = getelementptr inbounds %class.Parent1, ptr %p, i64 0, i32 0, i32 0
  store ptr getelementptr inbounds ({ [4 x ptr], [4 x ptr] }, ptr @_ZTV7Parent1, i64 0, inrange i32 0, i64 3), ptr %i12, align 8, !tbaa !9
  store ptr getelementptr inbounds ({ [4 x ptr], [4 x ptr] }, ptr @_ZTV7Parent1, i64 0, inrange i32 1, i64 3), ptr %i11, align 8, !tbaa !9
  %i13 = getelementptr inbounds %class.Parent1, ptr %p, i64 0, i32 0
  %call22 = call i32 @test7(ptr nonnull %i13)
  %add23 = add nsw i32 %add21, %call22
  call void @llvm.lifetime.end.p0(i64 16, ptr nonnull %p) #6
  call void @llvm.lifetime.end.p0(i64 24, ptr nonnull %d) #6
  ret i32 %add23
}

; Function Attrs: norecurse nounwind uwtable
define internal i32 @_ZN11GrandParent1fEv(ptr %this) unnamed_addr #4 comdat align 2 {
entry:
  ret i32 30
}

; Function Attrs: norecurse nounwind uwtable
define internal void @_ZN18VirtualGrandParent1gEv(ptr %this) unnamed_addr #4 comdat align 2 {
entry:
  ret void
}

; Function Attrs: norecurse nounwind uwtable
define internal i32 @_ZN8Derived11fEv(ptr %this) unnamed_addr #4 comdat align 2 {
entry:
  ret i32 60
}

; Function Attrs: norecurse uwtable
define internal i32 @_ZThn8_N8Derived11fEv(ptr %this) unnamed_addr #3 comdat align 2 {
entry:
  ret i32 60
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #5

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #5

attributes #0 = { noinline nounwind readonly uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readonly }
attributes #2 = { noinline uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { norecurse uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #5 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #6 = { nounwind }

!llvm.ident = !{!7}
!llvm.module.flags = !{!8}

!0 = !{i64 24, !"_ZTS11GrandParent"}
!1 = !{i64 56, !"_ZTS11GrandParent"}
!2 = !{i64 88, !"_ZTS18VirtualGrandParent"}
!3 = !{i64 24, !"_ZTS7Parent1"}
!4 = !{i64 56, !"_ZTS7Parent2"}
!5 = !{i64 24, !"_ZTS8Derived1"}
!6 = !{i64 56, !"_ZTS18VirtualGrandParent"}
!7 = !{!"clang version 6.0.0"}
!8 = !{i32 1, !"wchar_size", i32 4}
!9 = !{!10, !10, i64 0}
!10 = !{!"vtable pointer", !11, i64 0}
!11 = !{!"Simple C++ TBAA"}
