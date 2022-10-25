; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,intel_feature_sw_dtrans,asserts
; UNSUPPORTED: linux
; RUN: opt < %s -opaque-pointers -enable-dtrans -passes='function(functionrecognizer)' -debug-only=functionrecognizer -S 2>&1 | FileCheck %s

; Test that the Windows form of @swapfunc is recognized as a qsort swapfunc().

; CHECK: FUNCTION-RECOGNIZER: FOUND QSORT-SWAPFUNC swapfunc
; CHECK: define{{.*}}@swapfunc{{.*}} #0
; CHECK: attributes #0 = { "is-qsort-swapfunc" }

define internal void @swapfunc(ptr %a, ptr %b, i32 %n, i32 %swaptype_long, i32 %swaptype_int) {
entry:
  %cmp = icmp sle i32 %swaptype_long, 1
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %conv = sext i32 %n to i64
  %div = udiv i64 %conv, 4
  %conv1 = trunc i64 %div to i32
  br label %do.body

do.body:                                          ; preds = %do.body, %if.then
  %pj.0 = phi ptr [ %b, %if.then ], [ %incdec.ptr2, %do.body ]
  %pi.0 = phi ptr [ %a, %if.then ], [ %incdec.ptr, %do.body ]
  %i.0 = phi i32 [ %conv1, %if.then ], [ %dec, %do.body ]
  %i2 = load i32, ptr %pi.0, align 4
  %i3 = load i32, ptr %pj.0, align 4
  %incdec.ptr = getelementptr inbounds i32, ptr %pi.0, i32 1
  store i32 %i3, ptr %pi.0, align 4
  %incdec.ptr2 = getelementptr inbounds i32, ptr %pj.0, i32 1
  store i32 %i2, ptr %pj.0, align 4
  %dec = add nsw i32 %i.0, -1
  %cmp3 = icmp sgt i32 %dec, 0
  br i1 %cmp3, label %do.body, label %do.end

do.end:                                           ; preds = %do.body
  br label %if.end39

if.else:                                          ; preds = %entry
  %cmp5 = icmp sle i32 %swaptype_int, 1
  br i1 %cmp5, label %if.then7, label %if.else23

if.then7:                                         ; preds = %if.else
  %conv9 = sext i32 %n to i64
  %div10 = udiv i64 %conv9, 4
  %conv11 = trunc i64 %div10 to i32
  br label %do.body14

do.body14:                                        ; preds = %do.body14, %if.then7
  %i8.0 = phi i32 [ %conv11, %if.then7 ], [ %dec19, %do.body14 ]
  %pi12.0 = phi ptr [ %a, %if.then7 ], [ %incdec.ptr16, %do.body14 ]
  %pj13.0 = phi ptr [ %b, %if.then7 ], [ %incdec.ptr17, %do.body14 ]
  %i6 = load i32, ptr %pi12.0, align 4
  %i7 = load i32, ptr %pj13.0, align 4
  %incdec.ptr16 = getelementptr inbounds i32, ptr %pi12.0, i32 1
  store i32 %i7, ptr %pi12.0, align 4
  %incdec.ptr17 = getelementptr inbounds i32, ptr %pj13.0, i32 1
  store i32 %i6, ptr %pj13.0, align 4
  %dec19 = add nsw i32 %i8.0, -1
  %cmp20 = icmp sgt i32 %dec19, 0
  br i1 %cmp20, label %do.body14, label %do.end22

do.end22:                                         ; preds = %do.body14
  br label %if.end39

if.else23:                                        ; preds = %if.else
  %conv25 = sext i32 %n to i64
  %div26 = udiv i64 %conv25, 1
  %conv27 = trunc i64 %div26 to i32
  br label %do.body30

do.body30:                                        ; preds = %do.body30, %if.else23
  %i24.0 = phi i32 [ %conv27, %if.else23 ], [ %dec35, %do.body30 ]
  %pi28.0 = phi ptr [ %a, %if.else23 ], [ %incdec.ptr32, %do.body30 ]
  %pj29.0 = phi ptr [ %b, %if.else23 ], [ %incdec.ptr33, %do.body30 ]
  %i8 = load i8, ptr %pi28.0, align 1
  %i9 = load i8, ptr %pj29.0, align 1
  %incdec.ptr32 = getelementptr inbounds i8, ptr %pi28.0, i32 1
  store i8 %i9, ptr %pi28.0, align 1
  %incdec.ptr33 = getelementptr inbounds i8, ptr %pj29.0, i32 1
  store i8 %i8, ptr %pj29.0, align 1
  %dec35 = add nsw i32 %i24.0, -1
  %cmp36 = icmp sgt i32 %dec35, 0
  br i1 %cmp36, label %do.body30, label %do.end38

do.end38:                                         ; preds = %do.body30
  br label %if.end39

if.end39:                                         ; preds = %do.end38, %do.end22, %do.end
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
