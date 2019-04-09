; RUN: opt -S -jump-threading < %s | FileCheck %s

; Test that the block "test1" is jump-threaded over. The compare is always
; false if we come from the block "if.then". We can connect "if.then" directly
; to "test1.if.end3", skipping over "test1".
;
; CHECK: test1: {{.*}} preds = %entry
; CHECK: test1.if.end3: {{.*}} preds = %entry, %test1
; CHECK: test1.ret: {{.*}} preds = %test1.if.end3, %test1

define i32 @test1(i32 %j, i32 %k, i32* %p, i32* %q) local_unnamed_addr {
entry:
  %0 = load i32, i32* %q, align 4
  %cmp = icmp slt i32 %0, %j
  br i1 %cmp, label %if.then, label %test1

if.then:                                          ; preds = %entry
  %1 = load i32, i32* %p, align 4
  %inc = add nsw i32 %0, 1
  br label %test1

test1:                                            ; preds = %if.then, %entry
  %i.0 = phi i32 [ %inc, %if.then ], [ %0, %entry ]
  %r.0 = phi i32 [ %1, %if.then ], [ 0, %entry ]
  %cmp1 = icmp slt i32 %i.0, %j ;; always false incoming from "%if.then"
  br i1 %cmp1, label %test1.ret, label %test1.if.end3

test1.if.end3:                                    ; preds = %test1
  %2 = load i32, i32* %p, align 4
  %add = add nsw i32 %2, 1
  br label %test1.ret

test1.ret:                                     ; preds = %test1.if.end3, %test1
  %r.1 = phi i32 [ %r.0, %test1 ], [ %add, %test1.if.end3 ]
  ret i32 %r.1
}

; Test that the block "test2" is jump-threaded over. The compare in "test2"
; is always true if we come from the block "test2.if.end3". This case is more
; complex, because there are multiple reaching definitions of %i.1 which is
; the value of %i.0 in "test2.if.end3". All reaching definitions must have
; been compared with %j, in order for us to conclude that the comparison is
; always true in block "test2.if.end3".

; CHECK: test2.if.end3: {{.*}} preds = %if.else, %if.then
; CHECK: test2: {{.*}} preds = %test2.if.end3
; CHECK: test2.if.then8: {{.*}} preds = %test2.if.end3, %test2
; CHECK: test2.if.end10: {{.*}} preds = %test2.if.then8, %test2

define i32 @test2(i32 %j, i32 %k, i32* %p, i32* %q) local_unnamed_addr {
entry:
  %0 = load i32, i32* %q, align 4
  %cmp = icmp slt i32 %0, %j
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i32 0, i32* %p, align 4
  br label %test2.if.end3

if.else:                                          ; preds = %entry
  %inc = add nsw i32 %0, 1
  %cmp1 = icmp slt i32 %inc, %j
  br i1 %cmp1, label %test2.if.end3, label %elsewhere

elsewhere: ; preds = %if.else
  ret i32 0

test2.if.end3:                                     ; preds = %if.else, %if.then
  %i.0 = phi i32 [ %0, %if.then ], [ %inc, %if.else ]
              ; %0 and %inc have both been compared with %j
  %tobool = icmp eq i32 %k, 0
  br i1 %tobool, label %test2, label %if.then4

if.then4:                                         ; preds = %test2.if.end3
  store i32 2, i32* %p, align 4
  %dec = add nsw i32 %i.0, -1
  br label %test2

test2:                                            ; preds = %if.then4, %test2.if.end3
  %i.1 = phi i32 [ %dec, %if.then4 ], [ %i.0, %test2.if.end3 ]
  %cmp7 = icmp slt i32 %i.1, %j  ; always true incoming from %test2.if.end3
  br i1 %cmp7, label %test2.if.then8, label %test2.if.end10

test2.if.then8:                                    ; preds = %test2
  store i32 1, i32* %p, align 4
  br label %test2.if.end10

test2.if.end10:                                    ; preds = %test2.if.then8, %test2
  ret i32 0
}

; Negative test. The same compare reaches a block with both true and false
; values. We should not jump thread in this case as the value is not known.

; CHECK: test3.if.end3: {{.*}} preds = %if.then2, %if.else, %if.then
; CHECK: test3: {{.*}} preds = %if.then4, %test3.if.end3
; CHECK: test3.if.then8: {{.*}} preds = %test3
; CHECK: test3.if.end10: {{.*}} preds = %test3.if.then8, %test3

define i32 @test3_neg(i32 %j, i32 %k, i32* %p, i32* %q) local_unnamed_addr {
entry:
  %0 = load i32, i32* %q, align 4
  %cmp = icmp slt i32 %0, %j
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  store i32 0, i32* %p, align 4
  br label %test3.if.end3

if.else:                                          ; preds = %entry
  %inc = add nsw i32 %0, 1
  %cmp1 = icmp slt i32 %inc, %j
  br i1 %cmp1, label %if.then2, label %test3.if.end3

if.then2:                                         ; preds = %if.else
  store i32 3, i32* %p, align 4
  br label %test3.if.end3

test3.if.end3:                                     ; preds = %if.then2, %if.else, %if.then
  %i.0 = phi i32 [ %0, %if.then ], [ %inc, %if.then2 ], [ %inc, %if.else ]
  %tobool = icmp eq i32 %k, 0   ;; inc is both true and false on the 2 incoming edges
  br i1 %tobool, label %test3, label %if.then4

if.then4:                                         ; preds = %test3.if.end3
  store i32 2, i32* %p, align 4
  %dec = add nsw i32 %i.0, -1
  br label %test3

test3:                                            ; preds = %if.then4 %test3.if.end3
  %i.1 = phi i32 [ %dec, %if.then4 ], [ %i.0, %test3.if.end3 ]
  %cmp7 = icmp slt i32 %i.1, %j  ;; always true incoming from %if.then4
  br i1 %cmp7, label %test3.if.then8, label %test3.if.end10

test3.if.then8:                                    ; preds = %test3
  store i32 1, i32* %p, align 4
  br label %test3.if.end10

test3.if.end10:                                    ; preds = %test3.if.then8, %test3
  ret i32 0
}
