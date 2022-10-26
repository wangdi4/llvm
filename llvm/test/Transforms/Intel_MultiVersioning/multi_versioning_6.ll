; Test multi-versioning with various combinations of def-use chains in
; bool closure.
; This test is similar to multi_versioning_1.ll, but includes freeze
; instructions
;
; RUN: opt < %s -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -passes=multiversioning -multiversioning-threshold=2 -S 2>&1 | FileCheck %s

%struct.S = type { i8 }

;                           / Branch
;  Arg - GEP - Load - Cmp NE
;                           \ Select
;
; CHECK: br i1 true
; CHECK: select i1 true
; CHECK: br i1 false
; CHECK: select i1 false

define i32 @foo1(%struct.S* %Arg) local_unnamed_addr {
entry:
  %i = getelementptr inbounds %struct.S, %struct.S* %Arg, i64 0, i32 0
  %i1 = load i8, i8* %i, align 1
  %i11 = freeze i8 %i1
  %i2 = icmp ne i8 %i11, 0
  br i1 %i2, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %i3 = select i1 %i2, i32 33, i32 22
  ret i32 %i3
}

;                   / Cmp NE - Branch
;  Arg - GEP - Load
;                   \ Cmp EQ - Select
;
; CHECK: br i1 true
; CHECK: select i1 false
; CHECK: br i1 false
; CHECK: select i1 true

define i32 @foo2(%struct.S* %Arg) local_unnamed_addr {
entry:
  %i = getelementptr inbounds %struct.S, %struct.S* %Arg, i64 0, i32 0
  %i1 = load i8, i8* %i, align 1
  %i11 = freeze i8 %i1
  %i2 = icmp ne i8 %i11, 0
  br i1 %i2, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %i3 = icmp eq i8 %i11, 0
  %i4 = select i1 %i3, i32 33, i32 22
  ret i32 %i4
}

;           / Load - Cmp NE - Branch
;  Arg - GEP
;           \ Load - Cmp EQ - Select
;
; CHECK: br i1 true
; CHECK: select i1 false
; CHECK: br i1 false
; CHECK: select i1 true

define i32 @foo3(%struct.S* %Arg) local_unnamed_addr {
entry:
  %i = getelementptr inbounds %struct.S, %struct.S* %Arg, i64 0, i32 0
  %i1 = load i8, i8* %i, align 1
  %i11 = freeze i8 %i1
  %i2 = icmp ne i8 %i11, 0
  br i1 %i2, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %i3 = load i8, i8* %i, align 1
  %i4 = icmp eq i8 %i3, 0
  %i5 = select i1 %i4, i32 33, i32 22
  ret i32 %i5
}

;     / GEP - Load - Cmp NE - Branch
;  Arg
;     \ GEP - Load - Cmp EQ - Select
;
; CHECK: br i1 true
; CHECK: select i1 false
; CHECK: br i1 false
; CHECK: select i1 true

define i32 @foo4(%struct.S* %Arg) local_unnamed_addr {
entry:
  %i = getelementptr inbounds %struct.S, %struct.S* %Arg, i64 0, i32 0
  %i1 = load i8, i8* %i, align 1
  %i11 = freeze i8 %i1
  %i2 = icmp ne i8 %i11, 0
  br i1 %i2, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  br label %if.end

if.else:                                          ; preds = %entry
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %i3 = getelementptr inbounds %struct.S, %struct.S* %Arg, i64 0, i32 0
  %i4 = load i8, i8* %i3, align 1
  %i5 = icmp eq i8 %i4, 0
  %i6 = select i1 %i5, i32 33, i32 22
  ret i32 %i6
}
