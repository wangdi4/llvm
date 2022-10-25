; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,intel_feature_sw_dtrans,asserts
; UNSUPPORTED: windows
; RUN: opt < %s -opaque-pointers -enable-dtrans -passes='function(functionrecognizer)' -debug-only=functionrecognizer -S 2>&1 | FileCheck %s

; Test that the Linux form of @swapfunc is recognized as a qsort swapfunc().

; CHECK: FUNCTION-RECOGNIZER: FOUND QSORT-SWAPFUNC swapfunc
; CHECK: define{{.*}}@swapfunc{{.*}} #0
; CHECK: attributes #0 = { "is-qsort-swapfunc" }

define internal void @swapfunc(ptr %a, ptr %b, i32 %n, i32 %swaptype_long, i32 %swaptype_int) {
entry:
  %cmp = icmp sle i32 %swaptype_long, 1
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %conv = sext i32 %n to i64
  %div = udiv i64 %conv, 8
  br label %do.body

do.body:                                          ; preds = %do.body, %if.then
  %pj.0 = phi ptr [ %b, %if.then ], [ %incdec.ptr1, %do.body ]
  %pi.0 = phi ptr [ %a, %if.then ], [ %incdec.ptr, %do.body ]
  %i.0 = phi i64 [ %div, %if.then ], [ %dec, %do.body ]
  %i2 = load i64, ptr %pi.0, align 8
  %i3 = load i64, ptr %pj.0, align 8
  %incdec.ptr = getelementptr inbounds i64, ptr %pi.0, i32 1
  store i64 %i3, ptr %pi.0, align 8
  %incdec.ptr1 = getelementptr inbounds i64, ptr %pj.0, i32 1
  store i64 %i2, ptr %pj.0, align 8
  %dec = add nsw i64 %i.0, -1
  %cmp2 = icmp sgt i64 %dec, 0
  br i1 %cmp2, label %do.body, label %do.end

do.end:                                           ; preds = %do.body
  br label %if.end36

if.else:                                          ; preds = %entry
  %cmp4 = icmp sle i32 %swaptype_int, 1
  br i1 %cmp4, label %if.then6, label %if.else21

if.then6:                                         ; preds = %if.else
  %conv8 = sext i32 %n to i64
  %div9 = udiv i64 %conv8, 4
  br label %do.body12

do.body12:                                        ; preds = %do.body12, %if.then6
  %i7.0 = phi i64 [ %div9, %if.then6 ], [ %dec17, %do.body12 ]
  %pi10.0 = phi ptr [ %a, %if.then6 ], [ %incdec.ptr14, %do.body12 ]
  %pj11.0 = phi ptr [ %b, %if.then6 ], [ %incdec.ptr15, %do.body12 ]
  %i6 = load i32, ptr %pi10.0, align 4
  %i7 = load i32, ptr %pj11.0, align 4
  %incdec.ptr14 = getelementptr inbounds i32, ptr %pi10.0, i32 1
  store i32 %i7, ptr %pi10.0, align 4
  %incdec.ptr15 = getelementptr inbounds i32, ptr %pj11.0, i32 1
  store i32 %i6, ptr %pj11.0, align 4
  %dec17 = add nsw i64 %i7.0, -1
  %cmp18 = icmp sgt i64 %dec17, 0
  br i1 %cmp18, label %do.body12, label %do.end20

do.end20:                                         ; preds = %do.body12
  br label %if.end36

if.else21:                                        ; preds = %if.else
  %conv23 = sext i32 %n to i64
  %div24 = udiv i64 %conv23, 1
  br label %do.body27

do.body27:                                        ; preds = %do.body27, %if.else21
  %i22.0 = phi i64 [ %div24, %if.else21 ], [ %dec32, %do.body27 ]
  %pi25.0 = phi ptr [ %a, %if.else21 ], [ %incdec.ptr29, %do.body27 ]
  %pj26.0 = phi ptr [ %b, %if.else21 ], [ %incdec.ptr30, %do.body27 ]
  %i8 = load i8, ptr %pi25.0, align 1
  %i9 = load i8, ptr %pj26.0, align 1
  %incdec.ptr29 = getelementptr inbounds i8, ptr %pi25.0, i32 1
  store i8 %i9, ptr %pi25.0, align 1
  %incdec.ptr30 = getelementptr inbounds i8, ptr %pj26.0, i32 1
  store i8 %i8, ptr %pj26.0, align 1
  %dec32 = add nsw i64 %i22.0, -1
  %cmp33 = icmp sgt i64 %dec32, 0
  br i1 %cmp33, label %do.body27, label %do.end35

do.end35:                                         ; preds = %do.body27
  br label %if.end36

if.end36:                                         ; preds = %do.end35, %do.end20, %do.end
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
