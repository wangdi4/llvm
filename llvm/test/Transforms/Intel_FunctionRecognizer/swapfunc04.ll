; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,intel_feature_sw_dtrans,asserts
; RUN: opt < %s -enable-dtrans -passes='function(functionrecognizer)' -debug-only=functionrecognizer -S 2>&1 | FileCheck %s

; Test that @swapfunc is not recognized as a qsort swapfunc(), because the
; indexing GEPs do not have the right increments.

; CHECK-NOT: FUNCTION-RECOGNIZER: FOUND QSORT-SWAPFUNC swapfunc
; CHECK: define{{.*}}@swapfunc{{.*}}
; CHECK-NOT: attributes #0 = { "is-qsort-swapfunc" }

; Function Attrs: inlinehint nounwind uwtable
define internal void @swapfunc(i8* %a, i8* %b, i32 %n, i32 %swaptype_long, i32 %swaptype_int) #2 {
entry:
  %cmp = icmp sle i32 %swaptype_long, 1
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %conv = sext i32 %n to i64
  %div = udiv i64 %conv, 8
  %0 = bitcast i8* %a to i64*
  %1 = bitcast i8* %b to i64*
  br label %do.body

do.body:                                          ; preds = %do.body, %if.then
  %pj.0 = phi i64* [ %1, %if.then ], [ %incdec.ptr1, %do.body ]
  %pi.0 = phi i64* [ %0, %if.then ], [ %incdec.ptr, %do.body ]
  %i.0 = phi i64 [ %div, %if.then ], [ %dec, %do.body ]
  %2 = load i64, i64* %pi.0, align 8
  %3 = load i64, i64* %pj.0, align 8
  %incdec.ptr = getelementptr inbounds i64, i64* %pi.0, i32 -1
  store i64 %3, i64* %pi.0, align 8
  %incdec.ptr1 = getelementptr inbounds i64, i64* %pj.0, i32 -1
  store i64 %2, i64* %pj.0, align 8
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
  %4 = bitcast i8* %a to i32*
  %5 = bitcast i8* %b to i32*
  br label %do.body12

do.body12:                                        ; preds = %do.body12, %if.then6
  %i7.0 = phi i64 [ %div9, %if.then6 ], [ %dec17, %do.body12 ]
  %pi10.0 = phi i32* [ %4, %if.then6 ], [ %incdec.ptr14, %do.body12 ]
  %pj11.0 = phi i32* [ %5, %if.then6 ], [ %incdec.ptr15, %do.body12 ]
  %6 = load i32, i32* %pi10.0, align 4
  %7 = load i32, i32* %pj11.0, align 4
  %incdec.ptr14 = getelementptr inbounds i32, i32* %pi10.0, i32 2
  store i32 %7, i32* %pi10.0, align 4
  %incdec.ptr15 = getelementptr inbounds i32, i32* %pj11.0, i32 2
  store i32 %6, i32* %pj11.0, align 4
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
  %pi25.0 = phi i8* [ %a, %if.else21 ], [ %incdec.ptr29, %do.body27 ]
  %pj26.0 = phi i8* [ %b, %if.else21 ], [ %incdec.ptr30, %do.body27 ]
  %8 = load i8, i8* %pi25.0, align 1
  %9 = load i8, i8* %pj26.0, align 1
  %incdec.ptr29 = getelementptr inbounds i8, i8* %pi25.0, i32 1
  store i8 %9, i8* %pi25.0, align 1
  %incdec.ptr30 = getelementptr inbounds i8, i8* %pj26.0, i32 1
  store i8 %8, i8* %pj26.0, align 1
  %dec32 = add nsw i64 %i22.0, -1
  %cmp33 = icmp sgt i64 %dec32, 0
  br i1 %cmp33, label %do.body27, label %do.end35

do.end35:                                         ; preds = %do.body27
  br label %if.end36

if.end36:                                         ; preds = %do.end20, %do.end35, %do.end
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
