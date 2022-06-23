; RUN: opt -enable-new-pm=0 -hir-ssa-deconstruction -hir-vec-dir-insert -hir-framework -hir-vplan-vec -vplan-force-vf=4 -vplan-enable-all-zero-bypass-non-loops -mtriple=x86_64-unknown-unknown -enable-intel-advanced-opts -mattr=+sse4.2 -print-after=hir-vplan-vec -disable-output < %s 2>&1 | FileCheck %s --check-prefixes=HIR-CG

define void @foo(i64* noalias %lp, i64* noalias %lp2, i64* noalias %lp1, i64 %n1, i64 %n2) {
; HIR-CG-LABEL:  Function: foo
; HIR-CG-EMPTY:
; HIR-CG-NEXT:        BEGIN REGION { modified }
; HIR-CG-NEXT:              + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; HIR-CG-NEXT:              |   [[DOTCOPY0:%.*]] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; HIR-CG-NEXT:              |   [[LIVEOUTCOPY0:%.*]] = &((<4 x i64*>)(%lp)[i1 + <i64 0, i64 1, i64 2, i64 3>]);
; HIR-CG-NEXT:              |   [[VEC0:%.*]] = (<4 x i64>*)(%lp)[i1];
; HIR-CG-NEXT:              |   [[VEC1:%.*]] = [[VEC0]] != 0;
; HIR-CG-NEXT:              |   [[VEC2:%.*]] = [[VEC1]]  ^  -1;
; HIR-CG-NEXT:              |   [[BITCAST:%.*]] = bitcast.<4 x i1>.i4([[VEC2]]);
; HIR-CG-NEXT:              |   [[CMP0:%.*]] = [[BITCAST]] == 0;
; HIR-CG-NEXT:              |   [[ALLZEROCHECK0:%.*]] = [[CMP0]];
; HIR-CG-NEXT:              |   if ([[CMP0]] != 1)
; HIR-CG-NEXT:              |   {
; HIR-CG-NEXT:              |      (<4 x i64>*)(%lp1)[i1] = %n1, Mask = @{[[VEC2]]};
; HIR-CG-NEXT:              |   }
; HIR-CG-NEXT:              |   [[CMP1:%.*]] = %n1 > %n2;
; HIR-CG-NEXT:              |   [[VEC3:%.*]] = [[VEC2]]  &  [[CMP1]];
; HIR-CG-NEXT:              |   if (%n1 > %n2)
; HIR-CG-NEXT:              |   {
; HIR-CG-NEXT:              |      [[BITCAST0:%.*]] = bitcast.<4 x i1>.i4([[VEC3]]);
; HIR-CG-NEXT:              |      [[CMP2:%.*]] = [[BITCAST0]] == 0;
; HIR-CG-NEXT:              |      [[ALLZEROCHECK1:%.*]] = [[CMP2]];
; HIR-CG-NEXT:              |      if ([[CMP2]] != 1)
; HIR-CG-NEXT:              |      {
; HIR-CG-NEXT:              |         [[VEC4:%.*]] = %n2  *  i1 + <i64 0, i64 1, i64 2, i64 3>;
; HIR-CG-NEXT:              |         [[SCAL0:%.*]] = %n2  *  i1;
; HIR-CG-NEXT:              |         (<4 x i64>*)(%lp)[i1] = [[VEC4]], Mask = @{[[VEC3]]};
; HIR-CG-NEXT:              |      }
; HIR-CG-NEXT:              |   }
; HIR-CG-NEXT:              |   [[BITCAST1:%.*]] = bitcast.<4 x i1>.i4([[VEC1]]);
; HIR-CG-NEXT:              |   [[CMP3:%.*]] = [[BITCAST1]] == 0;
; HIR-CG-NEXT:              |   [[ALLZEROCHECK2:%.*]] = [[CMP3]];
; HIR-CG-NEXT:              |   if ([[CMP3]] != 1)
; HIR-CG-NEXT:              |   {
; HIR-CG-NEXT:              |      (<4 x i64>*)(%lp1)[i1] = %n2, Mask = @{[[VEC1]]};
; HIR-CG-NEXT:              |   }
; HIR-CG-NEXT:              |   [[CMP4:%.*]] = %n1 > %n2;
; HIR-CG-NEXT:              |   [[VEC5:%.*]] = [[VEC1]]  &  [[CMP4]];
; HIR-CG-NEXT:              |   if (%n1 > %n2)
; HIR-CG-NEXT:              |   {
; HIR-CG-NEXT:              |      [[BITCAST2:%.*]] = bitcast.<4 x i1>.i4([[VEC5]]);
; HIR-CG-NEXT:              |      [[CMP5:%.*]] = [[BITCAST2]] == 0;
; HIR-CG-NEXT:              |      [[ALLZEROCHECK3:%.*]] = [[CMP5]];
; HIR-CG-NEXT:              |      if ([[CMP5]] != 1)
; HIR-CG-NEXT:              |      {
; HIR-CG-NEXT:              |         [[VEC6:%.*]] = %n1  *  i1 + <i64 0, i64 1, i64 2, i64 3>;
; HIR-CG-NEXT:              |         [[SCAL15:%.*]] = %n1  *  i1;
; HIR-CG-NEXT:              |         (<4 x i64>*)(%lp)[i1] = [[VEC6]], Mask = @{%.vec11};
; HIR-CG-NEXT:              |      }
; HIR-CG-NEXT:              |   }
; HIR-CG-NEXT:              |   (<4 x i64>*)(%lp2)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; HIR-CG-NEXT:               + END LOOP
; HIR-CG:                   %l1.011.out = extractelement [[DOTCOPY0]],  3;
; HIR-CG-NEXT:              %arrayidx = extractelement [[LIVEOUTCOPY0]],  3;
; HIR-CG-NEXT:          END REGION
;
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %l1.011 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds i64, i64* %lp, i64 %l1.011
  %0 = load i64, i64* %arrayidx, align 8
  %tobool.not = icmp ne i64 %0, 0
  br i1 %tobool.not, label %if.then1, label %if.else1

if.then1:
  %arrayidx1.1 = getelementptr inbounds i64, i64* %lp1, i64 %l1.011
  store i64 %n2, i64* %arrayidx1.1, align 8
  %cmp1 = icmp sgt i64 %n1, %n2
  br i1 %cmp1, label %if.then2, label %if3

if.then2:                                         ; preds = %for.body
  %l1.0111 = mul i64 %l1.011, %n1
  store i64 %l1.0111, i64* %arrayidx, align 8
  br label %if3

if.else1:
  %arrayidx1.2 = getelementptr inbounds i64, i64* %lp1, i64 %l1.011
  store i64 %n1, i64* %arrayidx1.2, align 8
  %cmp1.2 = icmp sgt i64 %n1, %n2
  br i1 %cmp1.2, label %if.then4, label %if3

if.then4:                                         ; preds = %for.body
  %l1.0112 = mul i64 %l1.011, %n2
  store i64 %l1.0112, i64* %arrayidx, align 8
  br label %if3

if.else4:
  %cmp1.3 = icmp sgt i64 %n1, %n2
  br label %if.then5

if.then5:                                         ; preds = %for.body
  store i64 %l1.011, i64* %arrayidx, align 8
  br label %if3

if3:
  %arrayidx2 = getelementptr inbounds i64, i64* %lp2, i64 %l1.011
  store i64 %l1.011, i64* %arrayidx2, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then2
  %inc = add nuw nsw i64 %l1.011, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}

define void @foo1(i64* noalias %lp, i64* noalias %lp2, i64* noalias %lp1, i1* %n1.ptr, i1* %n2.ptr) {
; HIR-CG-LABEL: Function: foo1
; HIR-CG-EMPTY:
; HIR-CG-NEXT          BEGIN REGION { modified }
; HIR-CG-NEXT               + DO i1 = 0, 99, 4   <DO_LOOP> <auto-vectorized> <novectorize>
; HIR-CG-NEXT               |   [[VEC0:%.*]] = (<4 x i64>*)(%lp)[i1];
; HIR-CG-NEXT               |   [[VEC1:%.*]] = [[VEC0]] != 0;
; HIR-CG-NEXT               |   [[VEC2:%.*]] = [[VEC1]]  ^  -1;
; HIR-CG-NEXT               |   [[BITCAST:%.*]] = bitcast.<4 x i1>.i4([[VEC2]]);
; HIR-CG-NEXT               |   [[CMP0:%.*]] = [[BITCAST]] != 0;
; HIR-CG-NEXT               |   [[UNIFLOAD:%.*]] = undef;
; HIR-CG-NEXT               |   if ([[CMP0]] == 1)
; HIR-CG-NEXT               |   {
; HIR-CG-NEXT               |      [[UNIFLOAD]] = ([[UNIFORM_PTR1:%.*]])[0];
; HIR-CG-NEXT               |   }
; HIR-CG-NEXT               |   [[VEC3:%.*]] = [[UNIFLOAD]] != 0;
; HIR-CG-NEXT               |   [[VEC4:%.*]] = [[VEC2]]  &  [[VEC3]];
; HIR-CG-NEXT               |   (<4 x i64>*)(%lp)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>, Mask = @{[[VEC4]]};
; HIR-CG-NEXT               |   [[BITCAST1:%.*]] = bitcast.<4 x i1>.i4([[VEC1]]);
; HIR-CG-NEXT               |   [[CMP1:%.*]] = [[BITCAST1]] != 0;
; HIR-CG-NEXT               |   [[UNIFLOAD1:%.*]] = undef;
; HIR-CG-NEXT               |   if ([[CMP1]] == 1)
; HIR-CG-NEXT               |   {
; HIR-CG-NEXT               |      [[UNIFLOAD1]] = ([[UNIFORM_PTR0:%.*]])[0];
; HIR-CG-NEXT               |   }
; HIR-CG-NEXT               |   [[VEC8:%.*]] = [[UNIFLOAD1]] != 0;
; HIR-CG-NEXT               |   [[VEC9:%.*]] = [[VEC1]]  &  [[VEC8]];
; HIR-CG-NEXT               |   (<4 x i64>*)(%lp)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>, Mask = @{[[VEC9]]};
; HIR-CG-NEXT               |   (<4 x i64>*)(%lp2)[i1] = i1 + <i64 0, i64 1, i64 2, i64 3>;
; HIR-CG-NEXT               + END LOOP
; HIR-CG-NEXT          END REGION
;
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.inc
  %l1.011 = phi i64 [ 0, %entry ], [ %inc, %for.inc ]
  %arrayidx = getelementptr inbounds i64, i64* %lp, i64 %l1.011
  %0 = load i64, i64* %arrayidx, align 8
  %tobool.not = icmp ne i64 %0, 0
  br i1 %tobool.not, label %if.then1, label %if.else1

if.then1:
  %arrayidx1.1 = getelementptr inbounds i64, i64* %lp1, i64 %l1.011
  %n1 = load i1, i1 *%n1.ptr
  br i1 %n1, label %if.then2, label %if3

if.then2:                                         ; preds = %for.body
  store i64 %l1.011, i64* %arrayidx, align 8
  br label %if3

if.else1:
  %arrayidx1.2 = getelementptr inbounds i64, i64* %lp1, i64 %l1.011
  %n2 = load i1, i1 *%n2.ptr
  br i1 %n2, label %if.then4, label %if3

if.then4:                                         ; preds = %for.body
  store i64 %l1.011, i64* %arrayidx, align 8
  br label %if3

if3:
  %arrayidx2 = getelementptr inbounds i64, i64* %lp2, i64 %l1.011
  store i64 %l1.011, i64* %arrayidx2, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body, %if.then2
  %inc = add nuw nsw i64 %l1.011, 1
  %exitcond.not = icmp eq i64 %inc, 100
  br i1 %exitcond.not, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  ret void
}
