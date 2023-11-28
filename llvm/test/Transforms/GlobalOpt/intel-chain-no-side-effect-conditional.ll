; Currently, trivial chain optimization doesn't work if the global is used in
; a terminator.
; RUN: opt -S -passes=globalopt -enable-intel-advanced-opts < %s | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; CHECK-NOT: @a
@a = internal global i32 1
; CHECK-NOT: @b
@b = internal global i32 1
; CHECK: @c
@c = internal global i32 1
; CHECK: @d
@d = internal global i32 1

define void @store_global_1(i32 %arg) #0 {
; CHECK-LABEL: define void @store_global_1(
; CHECK-NEXT:    store i32 %arg, ptr @c, align 4
; CHECK-NEXT:    ret void
;
  store i32 %arg, ptr @a
  store i32 %arg, ptr @c
  ret void
}

define void @store_global_2(i32 %arg) #0 {
; CHECK-LABEL: define void @store_global_2(
; CHECK-NEXT:    store i32 %arg, ptr @d, align 4
; CHECK-NEXT:    ret void
;
  store i32 %arg, ptr @b
  store i32 %arg, ptr @d
  ret void
}

define void @chain_no_conditional() #0 {
; CHECK-LABEL: define void @chain_no_conditional()
; CHECK-NEXT:    ret void
;
  %1 = load i32, ptr @a
  %2 = load i32, ptr @b
  %3 = and i32 %1, %2
  %4 = icmp eq i32 %3, %1
  %5 = select i1 %4, i32 0, i32 1
  store i32 %5, ptr @a
  ret void
}

define void @chain_conditional() #0 {
; CHECK-LABEL: define void @chain_conditional()
; CHECK-NEXT:    [[TMP1:%.*]] = load i32, ptr @c, align 4
; CHECK-NEXT:    [[TMP2:%.*]] = load i32, ptr @d, align 4
; CHECK-NEXT:    [[TMP3:%.*]] = and i32 [[TMP1]], [[TMP2]]
; CHECK-NEXT:    [[TMP4:%.*]] = icmp eq i32 [[TMP3]], [[TMP1]]
; CHECK-NEXT:    br i1 [[TMP4]], label %if.then, label %if.else
; CHECK:       if.then:
; CHECK-NEXT:    store i32 0, ptr @c, align 4
; CHECK-NEXT:    br label %func.return
; CHECK:       if.else:
; CHECK-NEXT:    store i32 1, ptr @c, align 4
; CHECK-NEXT:    br label %func.return
; CHECK:       return:
; CHECK-NEXT:    ret void
;
  %1 = load i32, ptr @c
  %2 = load i32, ptr @d
  %3 = and i32 %1, %2
  %4 = icmp eq i32 %3, %1
  br i1 %4, label %if.then, label %if.else

if.then:
  store i32 0, ptr @c
  br label %func.return

if.else:
  store i32 1, ptr @c
  br label %func.return

func.return:
  ret void
}

declare void @unknown_function()

attributes #0 = { "target-features"="+avx2" }
