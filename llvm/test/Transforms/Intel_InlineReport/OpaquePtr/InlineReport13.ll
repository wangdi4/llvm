; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; Inline report
; RUN: opt -opaque-pointers -new-double-callsite-inlining-heuristics=true -inline -inline-report=0xe807 -inline-threshold=2000< %s -S 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -new-double-callsite-inlining-heuristics=true -passes='cgscc(inline)' -inline-report=0xe807 -inline-threshold=2000< %s -S 2>&1 | FileCheck %s
; Inline report via metadata
; RUN: opt -opaque-pointers -inlinereportsetup -inline-report=0xe886 < %s -S | opt -inline -new-double-callsite-inlining-heuristics=true -inline-report=0xe886 -inline-threshold=2000 -S | opt -inlinereportemitter -inline-report=0xe886 -S 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -new-double-callsite-inlining-heuristics=true -inline-report=0xe886 -inline-threshold=2000 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s

; CHECK: Callee has double callsite and local linkage
; This LIT test checks the following worthy double internal callsite heuristic
;   (1) Must have exactly two calls to the function
;   (2) Call must be in a loop
;   (3) Called function must have loops
;   (4) Called function must have a large enough total number
;           of predecessors

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@p = common global ptr null, align 8

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %track_counts = alloca [10 x i32], align 16
  %s = alloca i32, align 4
  %fstate1 = alloca i32, align 4
  %fstate2 = alloca i32, align 4
  store i32 0, ptr %retval, align 4
  store i32 0, ptr %s, align 4
  br label %while.cond

while.cond:                                       ; preds = %while.body, %entry
  %i = load ptr, ptr @p, align 8
  %i1 = load i32, ptr %i, align 4
  %cmp = icmp ne i32 %i1, 0
  br i1 %cmp, label %while.body, label %while.end

while.body:                                       ; preds = %while.cond
  %arraydecay = getelementptr inbounds [10 x i32], ptr %track_counts, i32 0, i32 0
  %call = call i32 @core_state_transition_switch(ptr @p, ptr %arraydecay)
  store i32 %call, ptr %fstate1, align 4
  %arraydecay1 = getelementptr inbounds [10 x i32], ptr %track_counts, i32 0, i32 0
  %call2 = call i32 @core_state_transition_switch(ptr @p, ptr %arraydecay1)
  store i32 %call2, ptr %fstate2, align 4
  %i2 = load i32, ptr %fstate1, align 4
  %i3 = load i32, ptr %fstate2, align 4
  %add = add nsw i32 %i2, %i3
  %i4 = load i32, ptr %s, align 4
  %add3 = add nsw i32 %i4, %add
  store i32 %add3, ptr %s, align 4
  br label %while.cond

while.end:                                        ; preds = %while.cond
  %i5 = load i32, ptr %s, align 4
  ret i32 %i5
}

; Function Attrs: nounwind uwtable
define internal i32 @core_state_transition_switch(ptr %instr, ptr %transition_count) #0 {
entry:
  %instr.addr = alloca ptr, align 8
  %transition_count.addr = alloca ptr, align 8
  %str = alloca ptr, align 8
  %NEXT_SYMBOL = alloca i32, align 4
  %state = alloca i32, align 4
  store ptr %instr, ptr %instr.addr, align 8
  store ptr %transition_count, ptr %transition_count.addr, align 8
  %i = load ptr, ptr %instr.addr, align 8
  %i1 = load ptr, ptr %i, align 8
  store ptr %i1, ptr %str, align 8
  store i32 0, ptr %state, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %i2 = load ptr, ptr %str, align 8
  %i3 = load i32, ptr %i2, align 4
  %tobool = icmp ne i32 %i3, 0
  br i1 %tobool, label %land.rhs, label %land.end

land.rhs:                                         ; preds = %for.cond
  %i4 = load i32, ptr %state, align 4
  %cmp = icmp ne i32 %i4, 7
  br label %land.end

land.end:                                         ; preds = %land.rhs, %for.cond
  %i5 = phi i1 [ false, %for.cond ], [ %cmp, %land.rhs ]
  br i1 %i5, label %for.body, label %for.end

for.body:                                         ; preds = %land.end
  %i6 = load ptr, ptr %str, align 8
  %i7 = load i32, ptr %i6, align 4
  store i32 %i7, ptr %NEXT_SYMBOL, align 4
  %i8 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp1 = icmp eq i32 %i8, 44
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.body
  %i9 = load i32, ptr %state, align 4
  %idxprom = sext i32 %i9 to i64
  %i10 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx = getelementptr inbounds i32, ptr %i10, i64 %idxprom
  %i11 = load i32, ptr %arrayidx, align 4
  %inc = add nsw i32 %i11, 1
  store i32 %inc, ptr %arrayidx, align 4
  %i12 = load ptr, ptr %str, align 8
  %incdec.ptr = getelementptr inbounds i32, ptr %i12, i32 1
  store ptr %incdec.ptr, ptr %str, align 8
  br label %for.end

if.end:                                           ; preds = %for.body
  %i13 = load i32, ptr %state, align 4
  switch i32 %i13, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb16
    i32 2, label %sw.bb32
    i32 3, label %sw.bb45
    i32 4, label %sw.bb60
    i32 5, label %sw.bb71
    i32 6, label %sw.bb81
  ]

sw.bb:                                            ; preds = %if.end
  %i14 = load i32, ptr %NEXT_SYMBOL, align 4
  %call = call i32 (i32, ...) @ee_isdigit(i32 %i14)
  %tobool2 = icmp ne i32 %call, 0
  br i1 %tobool2, label %if.then3, label %if.else

if.then3:                                         ; preds = %sw.bb
  store i32 2, ptr %state, align 4
  br label %if.end13

if.else:                                          ; preds = %sw.bb
  %i15 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp4 = icmp eq i32 %i15, 43
  br i1 %cmp4, label %if.then6, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %if.else
  %i16 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp5 = icmp eq i32 %i16, 45
  br i1 %cmp5, label %if.then6, label %if.else7

if.then6:                                         ; preds = %lor.lhs.false, %if.else
  store i32 1, ptr %state, align 4
  br label %if.end12

if.else7:                                         ; preds = %lor.lhs.false
  %i17 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp8 = icmp eq i32 %i17, 46
  br i1 %cmp8, label %if.then9, label %if.else10

if.then9:                                         ; preds = %if.else7
  store i32 3, ptr %state, align 4
  br label %if.end11

if.else10:                                        ; preds = %if.else7
  store i32 7, ptr %state, align 4
  br label %if.end11

if.end11:                                         ; preds = %if.else10, %if.then9
  br label %if.end12

if.end12:                                         ; preds = %if.end11, %if.then6
  br label %if.end13

if.end13:                                         ; preds = %if.end12, %if.then3
  %i18 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx14 = getelementptr inbounds i32, ptr %i18, i64 0
  %i19 = load i32, ptr %arrayidx14, align 4
  %inc15 = add nsw i32 %i19, 1
  store i32 %inc15, ptr %arrayidx14, align 4
  br label %sw.epilog

sw.bb16:                                          ; preds = %if.end
  %i20 = load i32, ptr %NEXT_SYMBOL, align 4
  %call17 = call i32 (i32, ...) @ee_isdigit(i32 %i20)
  %tobool18 = icmp ne i32 %call17, 0
  br i1 %tobool18, label %if.then19, label %if.else22

if.then19:                                        ; preds = %sw.bb16
  store i32 2, ptr %state, align 4
  %i21 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx20 = getelementptr inbounds i32, ptr %i21, i64 1
  %i22 = load i32, ptr %arrayidx20, align 4
  %inc21 = add nsw i32 %i22, 1
  store i32 %inc21, ptr %arrayidx20, align 4
  br label %if.end31

if.else22:                                        ; preds = %sw.bb16
  %i23 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp23 = icmp eq i32 %i23, 46
  br i1 %cmp23, label %if.then24, label %if.else27

if.then24:                                        ; preds = %if.else22
  store i32 3, ptr %state, align 4
  %i24 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx25 = getelementptr inbounds i32, ptr %i24, i64 1
  %i25 = load i32, ptr %arrayidx25, align 4
  %inc26 = add nsw i32 %i25, 1
  store i32 %inc26, ptr %arrayidx25, align 4
  br label %if.end30

if.else27:                                        ; preds = %if.else22
  store i32 7, ptr %state, align 4
  %i26 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx28 = getelementptr inbounds i32, ptr %i26, i64 1
  %i27 = load i32, ptr %arrayidx28, align 4
  %inc29 = add nsw i32 %i27, 1
  store i32 %inc29, ptr %arrayidx28, align 4
  br label %if.end30

if.end30:                                         ; preds = %if.else27, %if.then24
  br label %if.end31

if.end31:                                         ; preds = %if.end30, %if.then19
  br label %sw.epilog

sw.bb32:                                          ; preds = %if.end
  %i28 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp33 = icmp eq i32 %i28, 46
  br i1 %cmp33, label %if.then34, label %if.else37

if.then34:                                        ; preds = %sw.bb32
  store i32 3, ptr %state, align 4
  %i29 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx35 = getelementptr inbounds i32, ptr %i29, i64 2
  %i30 = load i32, ptr %arrayidx35, align 4
  %inc36 = add nsw i32 %i30, 1
  store i32 %inc36, ptr %arrayidx35, align 4
  br label %if.end44

if.else37:                                        ; preds = %sw.bb32
  %i31 = load i32, ptr %NEXT_SYMBOL, align 4
  %call38 = call i32 (i32, ...) @ee_isdigit(i32 %i31)
  %tobool39 = icmp ne i32 %call38, 0
  br i1 %tobool39, label %if.end43, label %if.then40

if.then40:                                        ; preds = %if.else37
  store i32 7, ptr %state, align 4
  %i32 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx41 = getelementptr inbounds i32, ptr %i32, i64 2
  %i33 = load i32, ptr %arrayidx41, align 4
  %inc42 = add nsw i32 %i33, 1
  store i32 %inc42, ptr %arrayidx41, align 4
  br label %if.end43

if.end43:                                         ; preds = %if.then40, %if.else37
  br label %if.end44

if.end44:                                         ; preds = %if.end43, %if.then34
  br label %sw.epilog

sw.bb45:                                          ; preds = %if.end
  %i34 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp46 = icmp eq i32 %i34, 69
  br i1 %cmp46, label %if.then49, label %lor.lhs.false47

lor.lhs.false47:                                  ; preds = %sw.bb45
  %i35 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp48 = icmp eq i32 %i35, 101
  br i1 %cmp48, label %if.then49, label %if.else52

if.then49:                                        ; preds = %lor.lhs.false47, %sw.bb45
  store i32 4, ptr %state, align 4
  %i36 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx50 = getelementptr inbounds i32, ptr %i36, i64 3
  %i37 = load i32, ptr %arrayidx50, align 4
  %inc51 = add nsw i32 %i37, 1
  store i32 %inc51, ptr %arrayidx50, align 4
  br label %if.end59

if.else52:                                        ; preds = %lor.lhs.false47
  %i38 = load i32, ptr %NEXT_SYMBOL, align 4
  %call53 = call i32 (i32, ...) @ee_isdigit(i32 %i38)
  %tobool54 = icmp ne i32 %call53, 0
  br i1 %tobool54, label %if.end58, label %if.then55

if.then55:                                        ; preds = %if.else52
  store i32 7, ptr %state, align 4
  %i39 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx56 = getelementptr inbounds i32, ptr %i39, i64 3
  %i40 = load i32, ptr %arrayidx56, align 4
  %inc57 = add nsw i32 %i40, 1
  store i32 %inc57, ptr %arrayidx56, align 4
  br label %if.end58

if.end58:                                         ; preds = %if.then55, %if.else52
  br label %if.end59

if.end59:                                         ; preds = %if.end58, %if.then49
  br label %sw.bb60

sw.bb60:                                          ; preds = %if.end59, %if.end
  %i41 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp61 = icmp eq i32 %i41, 43
  br i1 %cmp61, label %if.then64, label %lor.lhs.false62

lor.lhs.false62:                                  ; preds = %sw.bb60
  %i42 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp63 = icmp eq i32 %i42, 45
  br i1 %cmp63, label %if.then64, label %if.else67

if.then64:                                        ; preds = %lor.lhs.false62, %sw.bb60
  store i32 5, ptr %state, align 4
  %i43 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx65 = getelementptr inbounds i32, ptr %i43, i64 4
  %i44 = load i32, ptr %arrayidx65, align 4
  %inc66 = add nsw i32 %i44, 1
  store i32 %inc66, ptr %arrayidx65, align 4
  br label %if.end70

if.else67:                                        ; preds = %lor.lhs.false62
  store i32 7, ptr %state, align 4
  %i45 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx68 = getelementptr inbounds i32, ptr %i45, i64 4
  %i46 = load i32, ptr %arrayidx68, align 4
  %inc69 = add nsw i32 %i46, 1
  store i32 %inc69, ptr %arrayidx68, align 4
  br label %if.end70

if.end70:                                         ; preds = %if.else67, %if.then64
  br label %sw.epilog

sw.bb71:                                          ; preds = %if.end
  %i47 = load i32, ptr %NEXT_SYMBOL, align 4
  %call72 = call i32 (i32, ...) @ee_isdigit(i32 %i47)
  %tobool73 = icmp ne i32 %call72, 0
  br i1 %tobool73, label %if.then74, label %if.else77

if.then74:                                        ; preds = %sw.bb71
  store i32 6, ptr %state, align 4
  %i48 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx75 = getelementptr inbounds i32, ptr %i48, i64 5
  %i49 = load i32, ptr %arrayidx75, align 4
  %inc76 = add nsw i32 %i49, 1
  store i32 %inc76, ptr %arrayidx75, align 4
  br label %if.end80

if.else77:                                        ; preds = %sw.bb71
  store i32 7, ptr %state, align 4
  %i50 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx78 = getelementptr inbounds i32, ptr %i50, i64 5
  %i51 = load i32, ptr %arrayidx78, align 4
  %inc79 = add nsw i32 %i51, 1
  store i32 %inc79, ptr %arrayidx78, align 4
  br label %if.end80

if.end80:                                         ; preds = %if.else77, %if.then74
  br label %sw.epilog

sw.bb81:                                          ; preds = %if.end
  %i52 = load i32, ptr %NEXT_SYMBOL, align 4
  %call82 = call i32 (i32, ...) @ee_isdigit(i32 %i52)
  %tobool83 = icmp ne i32 %call82, 0
  br i1 %tobool83, label %if.end85, label %if.then84

if.then84:                                        ; preds = %sw.bb81
  store i32 7, ptr %state, align 4
  br label %if.end85

if.end85:                                         ; preds = %if.then84, %sw.bb81
  br label %sw.epilog

sw.default:                                       ; preds = %if.end
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %if.end85, %if.end80, %if.end70, %if.end44, %if.end31, %if.end13
  br label %for.inc

for.inc:                                          ; preds = %sw.epilog
  %i53 = load ptr, ptr %str, align 8
  %incdec.ptr86 = getelementptr inbounds i32, ptr %i53, i32 1
  store ptr %incdec.ptr86, ptr %str, align 8
  br label %for.cond

for.end:                                          ; preds = %if.then, %land.end
  %i54 = load i32, ptr %state, align 4
  %cmp87 = icmp eq i32 %i54, 7
  br i1 %cmp87, label %if.then88, label %if.end91

if.then88:                                        ; preds = %for.end
  %i55 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx89 = getelementptr inbounds i32, ptr %i55, i64 7
  %i56 = load i32, ptr %arrayidx89, align 4
  %inc90 = add nsw i32 %i56, 1
  store i32 %inc90, ptr %arrayidx89, align 4
  br label %if.end91

if.end91:                                         ; preds = %if.then88, %for.end
  br label %for.cond92

for.cond92:                                       ; preds = %for.inc195, %if.end91
  %i57 = load ptr, ptr %str, align 8
  %i58 = load i32, ptr %i57, align 4
  %tobool93 = icmp ne i32 %i58, 0
  br i1 %tobool93, label %land.rhs94, label %land.end96

land.rhs94:                                       ; preds = %for.cond92
  %i59 = load i32, ptr %state, align 4
  %cmp95 = icmp ne i32 %i59, 7
  br label %land.end96

land.end96:                                       ; preds = %land.rhs94, %for.cond92
  %i60 = phi i1 [ false, %for.cond92 ], [ %cmp95, %land.rhs94 ]
  br i1 %i60, label %for.body97, label %for.end197

for.body97:                                       ; preds = %land.end96
  %i61 = load ptr, ptr %str, align 8
  %i62 = load i32, ptr %i61, align 4
  store i32 %i62, ptr %NEXT_SYMBOL, align 4
  %i63 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp98 = icmp eq i32 %i63, 44
  br i1 %cmp98, label %if.then99, label %if.end104

if.then99:                                        ; preds = %for.body97
  %i64 = load i32, ptr %state, align 4
  %idxprom100 = sext i32 %i64 to i64
  %i65 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx101 = getelementptr inbounds i32, ptr %i65, i64 %idxprom100
  %i66 = load i32, ptr %arrayidx101, align 4
  %inc102 = add nsw i32 %i66, 1
  store i32 %inc102, ptr %arrayidx101, align 4
  %i67 = load ptr, ptr %str, align 8
  %incdec.ptr103 = getelementptr inbounds i32, ptr %i67, i32 1
  store ptr %incdec.ptr103, ptr %str, align 8
  br label %for.end197

if.end104:                                        ; preds = %for.body97
  %i68 = load i32, ptr %state, align 4
  switch i32 %i68, label %sw.default193 [
    i32 0, label %sw.bb105
    i32 1, label %sw.bb123
    i32 2, label %sw.bb139
    i32 3, label %sw.bb152
    i32 4, label %sw.bb167
    i32 5, label %sw.bb178
    i32 6, label %sw.bb188
  ]

sw.bb105:                                         ; preds = %if.end104
  %i69 = load i32, ptr %NEXT_SYMBOL, align 4
  %call106 = call i32 (i32, ...) @ee_isdigit(i32 %i69)
  %tobool107 = icmp ne i32 %call106, 0
  br i1 %tobool107, label %if.then108, label %if.else109

if.then108:                                       ; preds = %sw.bb105
  store i32 2, ptr %state, align 4
  br label %if.end120

if.else109:                                       ; preds = %sw.bb105
  %i70 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp110 = icmp eq i32 %i70, 43
  br i1 %cmp110, label %if.then113, label %lor.lhs.false111

lor.lhs.false111:                                 ; preds = %if.else109
  %i71 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp112 = icmp eq i32 %i71, 45
  br i1 %cmp112, label %if.then113, label %if.else114

if.then113:                                       ; preds = %lor.lhs.false111, %if.else109
  store i32 1, ptr %state, align 4
  br label %if.end119

if.else114:                                       ; preds = %lor.lhs.false111
  %i72 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp115 = icmp eq i32 %i72, 46
  br i1 %cmp115, label %if.then116, label %if.else117

if.then116:                                       ; preds = %if.else114
  store i32 3, ptr %state, align 4
  br label %if.end118

if.else117:                                       ; preds = %if.else114
  store i32 7, ptr %state, align 4
  br label %if.end118

if.end118:                                        ; preds = %if.else117, %if.then116
  br label %if.end119

if.end119:                                        ; preds = %if.end118, %if.then113
  br label %if.end120

if.end120:                                        ; preds = %if.end119, %if.then108
  %i73 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx121 = getelementptr inbounds i32, ptr %i73, i64 0
  %i74 = load i32, ptr %arrayidx121, align 4
  %inc122 = add nsw i32 %i74, 1
  store i32 %inc122, ptr %arrayidx121, align 4
  br label %sw.epilog194

sw.bb123:                                         ; preds = %if.end104
  %i75 = load i32, ptr %NEXT_SYMBOL, align 4
  %call124 = call i32 (i32, ...) @ee_isdigit(i32 %i75)
  %tobool125 = icmp ne i32 %call124, 0
  br i1 %tobool125, label %if.then126, label %if.else129

if.then126:                                       ; preds = %sw.bb123
  store i32 2, ptr %state, align 4
  %i76 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx127 = getelementptr inbounds i32, ptr %i76, i64 1
  %i77 = load i32, ptr %arrayidx127, align 4
  %inc128 = add nsw i32 %i77, 1
  store i32 %inc128, ptr %arrayidx127, align 4
  br label %if.end138

if.else129:                                       ; preds = %sw.bb123
  %i78 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp130 = icmp eq i32 %i78, 46
  br i1 %cmp130, label %if.then131, label %if.else134

if.then131:                                       ; preds = %if.else129
  store i32 3, ptr %state, align 4
  %i79 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx132 = getelementptr inbounds i32, ptr %i79, i64 1
  %i80 = load i32, ptr %arrayidx132, align 4
  %inc133 = add nsw i32 %i80, 1
  store i32 %inc133, ptr %arrayidx132, align 4
  br label %if.end137

if.else134:                                       ; preds = %if.else129
  store i32 7, ptr %state, align 4
  %i81 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx135 = getelementptr inbounds i32, ptr %i81, i64 1
  %i82 = load i32, ptr %arrayidx135, align 4
  %inc136 = add nsw i32 %i82, 1
  store i32 %inc136, ptr %arrayidx135, align 4
  br label %if.end137

if.end137:                                        ; preds = %if.else134, %if.then131
  br label %if.end138

if.end138:                                        ; preds = %if.end137, %if.then126
  br label %sw.epilog194

sw.bb139:                                         ; preds = %if.end104
  %i83 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp140 = icmp eq i32 %i83, 46
  br i1 %cmp140, label %if.then141, label %if.else144

if.then141:                                       ; preds = %sw.bb139
  store i32 3, ptr %state, align 4
  %i84 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx142 = getelementptr inbounds i32, ptr %i84, i64 2
  %i85 = load i32, ptr %arrayidx142, align 4
  %inc143 = add nsw i32 %i85, 1
  store i32 %inc143, ptr %arrayidx142, align 4
  br label %if.end151

if.else144:                                       ; preds = %sw.bb139
  %i86 = load i32, ptr %NEXT_SYMBOL, align 4
  %call145 = call i32 (i32, ...) @ee_isdigit(i32 %i86)
  %tobool146 = icmp ne i32 %call145, 0
  br i1 %tobool146, label %if.end150, label %if.then147

if.then147:                                       ; preds = %if.else144
  store i32 7, ptr %state, align 4
  %i87 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx148 = getelementptr inbounds i32, ptr %i87, i64 2
  %i88 = load i32, ptr %arrayidx148, align 4
  %inc149 = add nsw i32 %i88, 1
  store i32 %inc149, ptr %arrayidx148, align 4
  br label %if.end150

if.end150:                                        ; preds = %if.then147, %if.else144
  br label %if.end151

if.end151:                                        ; preds = %if.end150, %if.then141
  br label %sw.epilog194

sw.bb152:                                         ; preds = %if.end104
  %i89 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp153 = icmp eq i32 %i89, 69
  br i1 %cmp153, label %if.then156, label %lor.lhs.false154

lor.lhs.false154:                                 ; preds = %sw.bb152
  %i90 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp155 = icmp eq i32 %i90, 101
  br i1 %cmp155, label %if.then156, label %if.else159

if.then156:                                       ; preds = %lor.lhs.false154, %sw.bb152
  store i32 4, ptr %state, align 4
  %i91 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx157 = getelementptr inbounds i32, ptr %i91, i64 3
  %i92 = load i32, ptr %arrayidx157, align 4
  %inc158 = add nsw i32 %i92, 1
  store i32 %inc158, ptr %arrayidx157, align 4
  br label %if.end166

if.else159:                                       ; preds = %lor.lhs.false154
  %i93 = load i32, ptr %NEXT_SYMBOL, align 4
  %call160 = call i32 (i32, ...) @ee_isdigit(i32 %i93)
  %tobool161 = icmp ne i32 %call160, 0
  br i1 %tobool161, label %if.end165, label %if.then162

if.then162:                                       ; preds = %if.else159
  store i32 7, ptr %state, align 4
  %i94 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx163 = getelementptr inbounds i32, ptr %i94, i64 3
  %i95 = load i32, ptr %arrayidx163, align 4
  %inc164 = add nsw i32 %i95, 1
  store i32 %inc164, ptr %arrayidx163, align 4
  br label %if.end165

if.end165:                                        ; preds = %if.then162, %if.else159
  br label %if.end166

if.end166:                                        ; preds = %if.end165, %if.then156
  br label %sw.bb167

sw.bb167:                                         ; preds = %if.end166, %if.end104
  %i96 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp168 = icmp eq i32 %i96, 43
  br i1 %cmp168, label %if.then171, label %lor.lhs.false169

lor.lhs.false169:                                 ; preds = %sw.bb167
  %i97 = load i32, ptr %NEXT_SYMBOL, align 4
  %cmp170 = icmp eq i32 %i97, 45
  br i1 %cmp170, label %if.then171, label %if.else174

if.then171:                                       ; preds = %lor.lhs.false169, %sw.bb167
  store i32 5, ptr %state, align 4
  %i98 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx172 = getelementptr inbounds i32, ptr %i98, i64 4
  %i99 = load i32, ptr %arrayidx172, align 4
  %inc173 = add nsw i32 %i99, 1
  store i32 %inc173, ptr %arrayidx172, align 4
  br label %if.end177

if.else174:                                       ; preds = %lor.lhs.false169
  store i32 7, ptr %state, align 4
  %i100 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx175 = getelementptr inbounds i32, ptr %i100, i64 4
  %i101 = load i32, ptr %arrayidx175, align 4
  %inc176 = add nsw i32 %i101, 1
  store i32 %inc176, ptr %arrayidx175, align 4
  br label %if.end177

if.end177:                                        ; preds = %if.else174, %if.then171
  br label %sw.epilog194

sw.bb178:                                         ; preds = %if.end104
  %i102 = load i32, ptr %NEXT_SYMBOL, align 4
  %call179 = call i32 (i32, ...) @ee_isdigit(i32 %i102)
  %tobool180 = icmp ne i32 %call179, 0
  br i1 %tobool180, label %if.then181, label %if.else184

if.then181:                                       ; preds = %sw.bb178
  store i32 6, ptr %state, align 4
  %i103 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx182 = getelementptr inbounds i32, ptr %i103, i64 5
  %i104 = load i32, ptr %arrayidx182, align 4
  %inc183 = add nsw i32 %i104, 1
  store i32 %inc183, ptr %arrayidx182, align 4
  br label %if.end187

if.else184:                                       ; preds = %sw.bb178
  store i32 7, ptr %state, align 4
  %i105 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx185 = getelementptr inbounds i32, ptr %i105, i64 5
  %i106 = load i32, ptr %arrayidx185, align 4
  %inc186 = add nsw i32 %i106, 1
  store i32 %inc186, ptr %arrayidx185, align 4
  br label %if.end187

if.end187:                                        ; preds = %if.else184, %if.then181
  br label %sw.epilog194

sw.bb188:                                         ; preds = %if.end104
  %i107 = load i32, ptr %NEXT_SYMBOL, align 4
  %call189 = call i32 (i32, ...) @ee_isdigit(i32 %i107)
  %tobool190 = icmp ne i32 %call189, 0
  br i1 %tobool190, label %if.end192, label %if.then191

if.then191:                                       ; preds = %sw.bb188
  store i32 7, ptr %state, align 4
  br label %if.end192

if.end192:                                        ; preds = %if.then191, %sw.bb188
  br label %sw.epilog194

sw.default193:                                    ; preds = %if.end104
  br label %sw.epilog194

sw.epilog194:                                     ; preds = %sw.default193, %if.end192, %if.end187, %if.end177, %if.end151, %if.end138, %if.end120
  br label %for.inc195

for.inc195:                                       ; preds = %sw.epilog194
  %i108 = load ptr, ptr %str, align 8
  %incdec.ptr196 = getelementptr inbounds i32, ptr %i108, i32 1
  store ptr %incdec.ptr196, ptr %str, align 8
  br label %for.cond92

for.end197:                                       ; preds = %if.then99, %land.end96
  %i109 = load i32, ptr %state, align 4
  %cmp198 = icmp eq i32 %i109, 7
  br i1 %cmp198, label %if.then199, label %if.end202

if.then199:                                       ; preds = %for.end197
  %i110 = load ptr, ptr %transition_count.addr, align 8
  %arrayidx200 = getelementptr inbounds i32, ptr %i110, i64 7
  %i111 = load i32, ptr %arrayidx200, align 4
  %inc201 = add nsw i32 %i111, 1
  store i32 %inc201, ptr %arrayidx200, align 4
  br label %if.end202

if.end202:                                        ; preds = %if.then199, %for.end197
  %i112 = load ptr, ptr %str, align 8
  %i113 = load ptr, ptr %instr.addr, align 8
  store ptr %i112, ptr %i113, align 8
  %i114 = load i32, ptr %state, align 4
  ret i32 %i114
}

declare i32 @ee_isdigit(...) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (trunk 20640) (llvm/branches/ltoprof 20717)"}
; end INTEL_FEATURE_SW_ADVANCED
