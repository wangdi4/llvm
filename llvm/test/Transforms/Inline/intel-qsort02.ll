; RUN: opt < %s -passes='cgscc(inline)' -S 2>&1 | FileCheck %s

; Test that @spec-qsort loses its function recognizer attribute after
; inlining because @med3 was not recognized, but that the indirect calls
; through %cmp retain their attributes.

; CHECK-NOT: define{{.*}}@swapfunc
; CHECK-NOT: define{{.*}}@med3
; CHECK-NOT: define{{.*}}@spec_qsort{{.*}}#
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK: call{{.*}}%cmp({{.*}}) #0
; CHECK-NOT: attributes{{.*}}"is-qsort-spec_qsort"
; CHECK: attributes #0 = { "must-be-qsort-compare" }

define internal fastcc void @swapfunc(i8* %a, i8* %b, i32 %n, i32 %swaptype_long, i32 %swaptype_int) unnamed_addr #1 {
entry:
  %cmp = icmp slt i32 %swaptype_long, 2
  br i1 %cmp, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  %conv = sext i32 %n to i64
  %div = lshr i64 %conv, 3
  %0 = bitcast i8* %a to i64*
  %1 = bitcast i8* %b to i64*
  br label %do.body

do.body:                                          ; preds = %do.body, %if.then
  %pj.0 = phi i64* [ %1, %if.then ], [ %incdec.ptr1, %do.body ]
  %pi.0 = phi i64* [ %0, %if.then ], [ %incdec.ptr, %do.body ]
  %i.0 = phi i64 [ %div, %if.then ], [ %dec, %do.body ]
  %2 = load i64, i64* %pi.0, align 8
  %3 = load i64, i64* %pj.0, align 8
  %incdec.ptr = getelementptr inbounds i64, i64* %pi.0, i64 1
  store i64 %3, i64* %pi.0, align 8
  %incdec.ptr1 = getelementptr inbounds i64, i64* %pj.0, i64 1
  store i64 %2, i64* %pj.0, align 8
  %dec = add nsw i64 %i.0, -1
  %cmp2 = icmp sgt i64 %i.0, 1
  br i1 %cmp2, label %do.body, label %if.end36

if.else:                                          ; preds = %entry
  %cmp4 = icmp slt i32 %swaptype_int, 2
  %conv8 = sext i32 %n to i64
  br i1 %cmp4, label %if.then6, label %do.body27

if.then6:                                         ; preds = %if.else
  %div9 = lshr i64 %conv8, 2
  %4 = bitcast i8* %a to i32*
  %5 = bitcast i8* %b to i32*
  br label %do.body12

do.body12:                                        ; preds = %do.body12, %if.then6
  %i7.0 = phi i64 [ %div9, %if.then6 ], [ %dec17, %do.body12 ]
  %pi10.0 = phi i32* [ %4, %if.then6 ], [ %incdec.ptr14, %do.body12 ]
  %pj11.0 = phi i32* [ %5, %if.then6 ], [ %incdec.ptr15, %do.body12 ]
  %6 = load i32, i32* %pi10.0, align 4
  %7 = load i32, i32* %pj11.0, align 4
  %incdec.ptr14 = getelementptr inbounds i32, i32* %pi10.0, i64 1
  store i32 %7, i32* %pi10.0, align 4
  %incdec.ptr15 = getelementptr inbounds i32, i32* %pj11.0, i64 1
  store i32 %6, i32* %pj11.0, align 4
  %dec17 = add nsw i64 %i7.0, -1
  %cmp18 = icmp sgt i64 %i7.0, 1
  br i1 %cmp18, label %do.body12, label %if.end36

do.body27:                                        ; preds = %if.else, %do.body27
  %i22.0 = phi i64 [ %dec32, %do.body27 ], [ %conv8, %if.else ]
  %pi25.0 = phi i8* [ %incdec.ptr29, %do.body27 ], [ %a, %if.else ]
  %pj26.0 = phi i8* [ %incdec.ptr30, %do.body27 ], [ %b, %if.else ]
  %8 = load i8, i8* %pi25.0, align 1
  %9 = load i8, i8* %pj26.0, align 1
  %incdec.ptr29 = getelementptr inbounds i8, i8* %pi25.0, i64 1
  store i8 %9, i8* %pi25.0, align 1
  %incdec.ptr30 = getelementptr inbounds i8, i8* %pj26.0, i64 1
  store i8 %8, i8* %pj26.0, align 1
  %dec32 = add nsw i64 %i22.0, -1
  %cmp33 = icmp sgt i64 %i22.0, 1
  br i1 %cmp33, label %do.body27, label %if.end36

if.end36:                                         ; preds = %do.body27, %do.body12, %do.body
  ret void
}

; Function Attrs: inlinehint nounwind uwtable
define internal fastcc i8* @med3(i8* %a, i8* %b, i8* %c, i32 (i8*, i8*)* %cmp) unnamed_addr #2 {
entry:
  %call = call i32 %cmp(i8* %a, i8* %b) #3
  %cmp1 = icmp slt i32 %call, 0
  %call2 = call i32 %cmp(i8* %b, i8* %c) #3
  br i1 %cmp1, label %cond.true, label %cond.false11

cond.true:                                        ; preds = %entry
  %cmp3 = icmp slt i32 %call2, 0
  br i1 %cmp3, label %cond.end24, label %cond.false

cond.false:                                       ; preds = %cond.true
  %call5 = call i32 %cmp(i8* %a, i8* %c) #3
  %cmp6 = icmp slt i32 %call5, 0
  %0 = select i1 %cmp6, i8* %c, i8* %a
  br label %cond.end24

cond.false11:                                     ; preds = %entry
  %cmp13 = icmp sgt i32 %call2, 0
  br i1 %cmp13, label %cond.end24, label %cond.false15

cond.false15:                                     ; preds = %cond.false11
  %call16 = call i32 %cmp(i8* %a, i8* %c) #3
  %cmp17 = icmp slt i32 %call16, 0
  %1 = select i1 %cmp17, i8* %a, i8* %c
  br label %cond.end24

cond.end24:                                       ; preds = %cond.false11, %cond.true, %cond.false15, %cond.false
  %cond25 = phi i8* [ %0, %cond.false ], [ %1, %cond.false15 ], [ %b, %cond.true ], [ %b, %cond.false11 ]
  ret i8* %cond25
}

; Function Attrs: nounwind uwtable
define dso_local void @spec_qsort(i8* %a, i64 %n, i64 %es, i32 (i8*, i8*)* %cmp) local_unnamed_addr #0 {
entry:
  br label %loop

loop:                                             ; preds = %if.then292, %entry
  %n.addr.0 = phi i64 [ %n, %entry ], [ %div295, %if.then292 ]
  %a.addr.0 = phi i8* [ %a, %entry ], [ %add.ptr294, %if.then292 ]
  %sub.ptr.lhs.cast = ptrtoint i8* %a.addr.0 to i64
  %rem = and i64 %sub.ptr.lhs.cast, 7
  %tobool = icmp eq i64 %rem, 0
  %rem1 = and i64 %es, 7
  %tobool2 = icmp eq i64 %rem1, 0
  %or.cond = and i1 %tobool, %tobool2
  %cmp3 = icmp ne i64 %es, 8
  %cond = zext i1 %cmp3 to i32
  %0 = select i1 %or.cond, i32 %cond, i32 2
  %rem7 = and i64 %sub.ptr.lhs.cast, 3
  %tobool8 = icmp eq i64 %rem7, 0
  %rem10 = and i64 %es, 3
  %tobool11 = icmp eq i64 %rem10, 0
  %or.cond571 = and i1 %tobool8, %tobool11
  %cmp14 = icmp ne i64 %es, 4
  %cond15 = zext i1 %cmp14 to i32
  %1 = select i1 %or.cond571, i32 %cond15, i32 2
  %cmp18 = icmp ult i64 %n.addr.0, 7
  br i1 %cmp18, label %for.cond, label %if.end48

for.cond:                                         ; preds = %loop, %for.inc45
  %a.addr.0.pn = phi i8* [ %pm.0, %for.inc45 ], [ %a.addr.0, %loop ]
  %pm.0 = getelementptr inbounds i8, i8* %a.addr.0.pn, i64 %es
  %mul = mul i64 %n.addr.0, %es
  %add.ptr19 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul
  %cmp20 = icmp ult i8* %pm.0, %add.ptr19
  br i1 %cmp20, label %for.cond21, label %cleanup

for.cond21:                                       ; preds = %for.cond, %for.inc
  %pl.0 = phi i8* [ %add.ptr23, %for.inc ], [ %pm.0, %for.cond ]
  %cmp22 = icmp ugt i8* %pl.0, %a.addr.0
  br i1 %cmp22, label %land.rhs, label %for.inc45

land.rhs:                                         ; preds = %for.cond21
  %idx.neg = sub i64 0, %es
  %add.ptr23 = getelementptr inbounds i8, i8* %pl.0, i64 %idx.neg
  %call = call i32 %cmp(i8* %add.ptr23, i8* %pl.0) #3
  %cmp24 = icmp sgt i32 %call, 0
  br i1 %cmp24, label %for.body25, label %for.inc45

for.body25:                                       ; preds = %land.rhs
  %cmp26 = icmp eq i32 %0, 0
  br i1 %cmp26, label %if.then27, label %if.else

if.then27:                                        ; preds = %for.body25
  %2 = bitcast i8* %pl.0 to i64*
  %3 = load i64, i64* %2, align 8
  %4 = bitcast i8* %add.ptr23 to i64*
  %5 = load i64, i64* %4, align 8
  store i64 %5, i64* %2, align 8
  store i64 %3, i64* %4, align 8
  br label %for.inc

if.else:                                          ; preds = %for.body25
  %cmp32 = icmp eq i32 %1, 0
  br i1 %cmp32, label %if.then33, label %if.else39

if.then33:                                        ; preds = %if.else
  %6 = bitcast i8* %pl.0 to i32*
  %7 = load i32, i32* %6, align 4
  %8 = bitcast i8* %add.ptr23 to i32*
  %9 = load i32, i32* %8, align 4
  store i32 %9, i32* %6, align 4
  store i32 %7, i32* %8, align 4
  br label %for.inc

if.else39:                                        ; preds = %if.else
  %conv = trunc i64 %es to i32
  call fastcc void @swapfunc(i8* %pl.0, i8* %add.ptr23, i32 %conv, i32 %0, i32 %1)
  br label %for.inc

for.inc:                                          ; preds = %if.then27, %if.else39, %if.then33
  br label %for.cond21

for.inc45:                                        ; preds = %land.rhs, %for.cond21
  br label %for.cond

if.end48:                                         ; preds = %loop
  %div = lshr i64 %n.addr.0, 1
  %mul49 = mul i64 %div, %es
  %add.ptr50 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul49
  %cmp51 = icmp eq i64 %n.addr.0, 7
  br i1 %cmp51, label %if.end77, label %if.then53

if.then53:                                        ; preds = %if.end48
  %sub = add i64 %n.addr.0, -1
  %mul54 = mul i64 %sub, %es
  %add.ptr55 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul54
  %cmp56 = icmp ugt i64 %n.addr.0, 40
  br i1 %cmp56, label %if.then58, label %if.end75

if.then58:                                        ; preds = %if.then53
  %div59 = lshr i64 %n.addr.0, 3
  %mul60 = mul i64 %div59, %es
  %add.ptr61 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul60
  %mul62 = shl i64 %mul60, 1
  %add.ptr63 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul62
  %call64 = call fastcc i8* @med3(i8* %a.addr.0, i8* %add.ptr61, i8* %add.ptr63, i32 (i8*, i8*)* %cmp)
  %idx.neg65 = sub i64 0, %mul60
  %add.ptr66 = getelementptr inbounds i8, i8* %add.ptr50, i64 %idx.neg65
  %add.ptr67 = getelementptr inbounds i8, i8* %add.ptr50, i64 %mul60
  %call68 = call fastcc i8* @med3(i8* %add.ptr66, i8* %add.ptr50, i8* %add.ptr67, i32 (i8*, i8*)* %cmp)
  %idx.neg70 = sub i64 0, %mul62
  %add.ptr71 = getelementptr inbounds i8, i8* %add.ptr55, i64 %idx.neg70
  %add.ptr73 = getelementptr inbounds i8, i8* %add.ptr55, i64 %idx.neg65
  %call74 = call fastcc i8* @med3(i8* %add.ptr71, i8* %add.ptr73, i8* %add.ptr55, i32 (i8*, i8*)* %cmp)
  br label %if.end75

if.end75:                                         ; preds = %if.then58, %if.then53
  %pn.0 = phi i8* [ %call74, %if.then58 ], [ %add.ptr55, %if.then53 ]
  %pm.1 = phi i8* [ %call68, %if.then58 ], [ %add.ptr50, %if.then53 ]
  %pl.1 = phi i8* [ %call64, %if.then58 ], [ %a.addr.0, %if.then53 ]
  %call76 = call fastcc i8* @med3(i8* %pl.1, i8* %pm.1, i8* %pn.0, i32 (i8*, i8*)* %cmp)
  br label %if.end77

if.end77:                                         ; preds = %if.end48, %if.end75
  %pm.2 = phi i8* [ %call76, %if.end75 ], [ %add.ptr50, %if.end48 ]
  %cmp78 = icmp eq i32 %0, 0
  br i1 %cmp78, label %if.then80, label %if.else82

if.then80:                                        ; preds = %if.end77
  %10 = bitcast i8* %a.addr.0 to i64*
  %11 = load i64, i64* %10, align 8
  %12 = bitcast i8* %pm.2 to i64*
  %13 = load i64, i64* %12, align 8
  store i64 %13, i64* %10, align 8
  store i64 %11, i64* %12, align 8
  br label %if.end90

if.else82:                                        ; preds = %if.end77
  %cmp83 = icmp eq i32 %1, 0
  br i1 %cmp83, label %if.then85, label %if.else87

if.then85:                                        ; preds = %if.else82
  %14 = bitcast i8* %a.addr.0 to i32*
  %15 = load i32, i32* %14, align 4
  %16 = bitcast i8* %pm.2 to i32*
  %17 = load i32, i32* %16, align 4
  store i32 %17, i32* %14, align 4
  store i32 %15, i32* %16, align 4
  br label %if.end90

if.else87:                                        ; preds = %if.else82
  %conv88 = trunc i64 %es to i32
  call fastcc void @swapfunc(i8* %a.addr.0, i8* %pm.2, i32 %conv88, i32 %0, i32 %1)
  br label %if.end90

if.end90:                                         ; preds = %if.then85, %if.else87, %if.then80
  %add.ptr91 = getelementptr inbounds i8, i8* %a.addr.0, i64 %es
  %sub92 = add i64 %n.addr.0, -1
  %mul93 = mul i64 %sub92, %es
  %add.ptr94 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul93
  br label %for.cond95

for.cond95:                                       ; preds = %if.end169, %if.end90
  %swap_cnt.0 = phi i32 [ 0, %if.end90 ], [ 1, %if.end169 ]
  %pd.0 = phi i8* [ %add.ptr94, %if.end90 ], [ %pd.1, %if.end169 ]
  %pc.0 = phi i8* [ %add.ptr94, %if.end90 ], [ %add.ptr172, %if.end169 ]
  %pb.0 = phi i8* [ %add.ptr91, %if.end90 ], [ %add.ptr170, %if.end169 ]
  %pa.0 = phi i8* [ %add.ptr91, %if.end90 ], [ %pa.1, %if.end169 ]
  br label %while.cond

while.cond:                                       ; preds = %if.end120, %for.cond95
  %swap_cnt.1 = phi i32 [ %swap_cnt.0, %for.cond95 ], [ %swap_cnt.2, %if.end120 ]
  %pb.1 = phi i8* [ %pb.0, %for.cond95 ], [ %add.ptr121, %if.end120 ]
  %pa.1 = phi i8* [ %pa.0, %for.cond95 ], [ %pa.2, %if.end120 ]
  %cmp96 = icmp ugt i8* %pb.1, %pc.0
  br i1 %cmp96, label %while.end, label %land.rhs98

land.rhs98:                                       ; preds = %while.cond
  %call99 = call i32 %cmp(i8* %pb.1, i8* %a.addr.0) #3
  %cmp100 = icmp slt i32 %call99, 1
  br i1 %cmp100, label %while.body, label %while.end

while.body:                                       ; preds = %land.rhs98
  %cmp103 = icmp eq i32 %call99, 0
  br i1 %cmp103, label %if.then105, label %if.end120

if.then105:                                       ; preds = %while.body
  br i1 %cmp78, label %if.then108, label %if.else110

if.then108:                                       ; preds = %if.then105
  %18 = bitcast i8* %pa.1 to i64*
  %19 = load i64, i64* %18, align 8
  %20 = bitcast i8* %pb.1 to i64*
  %21 = load i64, i64* %20, align 8
  store i64 %21, i64* %18, align 8
  store i64 %19, i64* %20, align 8
  br label %if.end118

if.else110:                                       ; preds = %if.then105
  %cmp111 = icmp eq i32 %1, 0
  br i1 %cmp111, label %if.then113, label %if.else115

if.then113:                                       ; preds = %if.else110
  %22 = bitcast i8* %pa.1 to i32*
  %23 = load i32, i32* %22, align 4
  %24 = bitcast i8* %pb.1 to i32*
  %25 = load i32, i32* %24, align 4
  store i32 %25, i32* %22, align 4
  store i32 %23, i32* %24, align 4
  br label %if.end118

if.else115:                                       ; preds = %if.else110
  %conv116 = trunc i64 %es to i32
  call fastcc void @swapfunc(i8* %pa.1, i8* %pb.1, i32 %conv116, i32 %0, i32 %1)
  br label %if.end118

if.end118:                                        ; preds = %if.then113, %if.else115, %if.then108
  %add.ptr119 = getelementptr inbounds i8, i8* %pa.1, i64 %es
  br label %if.end120

if.end120:                                        ; preds = %if.end118, %while.body
  %swap_cnt.2 = phi i32 [ 1, %if.end118 ], [ %swap_cnt.1, %while.body ]
  %pa.2 = phi i8* [ %add.ptr119, %if.end118 ], [ %pa.1, %while.body ]
  %add.ptr121 = getelementptr inbounds i8, i8* %pb.1, i64 %es
  br label %while.cond

while.end:                                        ; preds = %while.cond, %land.rhs98
  br label %while.cond122

while.cond122:                                    ; preds = %if.end149, %while.end
  %swap_cnt.3 = phi i32 [ %swap_cnt.1, %while.end ], [ %swap_cnt.4, %if.end149 ]
  %pd.1 = phi i8* [ %pd.0, %while.end ], [ %pd.2, %if.end149 ]
  %pc.1 = phi i8* [ %pc.0, %while.end ], [ %add.ptr151, %if.end149 ]
  %cmp123 = icmp ugt i8* %pb.1, %pc.1
  br i1 %cmp123, label %while.end152, label %land.rhs125

land.rhs125:                                      ; preds = %while.cond122
  %call126 = call i32 %cmp(i8* %pc.1, i8* %a.addr.0) #3
  %cmp127 = icmp sgt i32 %call126, -1
  br i1 %cmp127, label %while.body130, label %while.end152

while.body130:                                    ; preds = %land.rhs125
  %cmp131 = icmp eq i32 %call126, 0
  br i1 %cmp131, label %if.then133, label %if.end149

if.then133:                                       ; preds = %while.body130
  br i1 %cmp78, label %if.then136, label %if.else138

if.then136:                                       ; preds = %if.then133
  %26 = bitcast i8* %pc.1 to i64*
  %27 = load i64, i64* %26, align 8
  %28 = bitcast i8* %pd.1 to i64*
  %29 = load i64, i64* %28, align 8
  store i64 %29, i64* %26, align 8
  store i64 %27, i64* %28, align 8
  br label %if.end146

if.else138:                                       ; preds = %if.then133
  %cmp139 = icmp eq i32 %1, 0
  br i1 %cmp139, label %if.then141, label %if.else143

if.then141:                                       ; preds = %if.else138
  %30 = bitcast i8* %pc.1 to i32*
  %31 = load i32, i32* %30, align 4
  %32 = bitcast i8* %pd.1 to i32*
  %33 = load i32, i32* %32, align 4
  store i32 %33, i32* %30, align 4
  store i32 %31, i32* %32, align 4
  br label %if.end146

if.else143:                                       ; preds = %if.else138
  %conv144 = trunc i64 %es to i32
  call fastcc void @swapfunc(i8* %pc.1, i8* %pd.1, i32 %conv144, i32 %0, i32 %1)
  br label %if.end146

if.end146:                                        ; preds = %if.then141, %if.else143, %if.then136
  %idx.neg147 = sub i64 0, %es
  %add.ptr148 = getelementptr inbounds i8, i8* %pd.1, i64 %idx.neg147
  br label %if.end149

if.end149:                                        ; preds = %if.end146, %while.body130
  %swap_cnt.4 = phi i32 [ 1, %if.end146 ], [ %swap_cnt.3, %while.body130 ]
  %pd.2 = phi i8* [ %add.ptr148, %if.end146 ], [ %pd.1, %while.body130 ]
  %idx.neg150 = sub i64 0, %es
  %add.ptr151 = getelementptr inbounds i8, i8* %pc.1, i64 %idx.neg150
  br label %while.cond122

while.end152:                                     ; preds = %while.cond122, %land.rhs125
  %cmp153 = icmp ugt i8* %pb.1, %pc.1
  br i1 %cmp153, label %for.end173, label %if.end156

if.end156:                                        ; preds = %while.end152
  br i1 %cmp78, label %if.then159, label %if.else161

if.then159:                                       ; preds = %if.end156
  %34 = bitcast i8* %pb.1 to i64*
  %35 = load i64, i64* %34, align 8
  %36 = bitcast i8* %pc.1 to i64*
  %37 = load i64, i64* %36, align 8
  store i64 %37, i64* %34, align 8
  store i64 %35, i64* %36, align 8
  br label %if.end169

if.else161:                                       ; preds = %if.end156
  %cmp162 = icmp eq i32 %1, 0
  br i1 %cmp162, label %if.then164, label %if.else166

if.then164:                                       ; preds = %if.else161
  %38 = bitcast i8* %pb.1 to i32*
  %39 = load i32, i32* %38, align 4
  %40 = bitcast i8* %pc.1 to i32*
  %41 = load i32, i32* %40, align 4
  store i32 %41, i32* %38, align 4
  store i32 %39, i32* %40, align 4
  br label %if.end169

if.else166:                                       ; preds = %if.else161
  %conv167 = trunc i64 %es to i32
  call fastcc void @swapfunc(i8* %pb.1, i8* %pc.1, i32 %conv167, i32 %0, i32 %1)
  br label %if.end169

if.end169:                                        ; preds = %if.then164, %if.else166, %if.then159
  %add.ptr170 = getelementptr inbounds i8, i8* %pb.1, i64 %es
  %idx.neg171 = sub i64 0, %es
  %add.ptr172 = getelementptr inbounds i8, i8* %pc.1, i64 %idx.neg171
  br label %for.cond95

for.end173:                                       ; preds = %while.end152
  %cmp174 = icmp eq i32 %swap_cnt.3, 0
  br i1 %cmp174, label %for.cond178, label %if.end225

for.cond178:                                      ; preds = %for.end173, %for.inc222
  %pm.3 = phi i8* [ %add.ptr223, %for.inc222 ], [ %add.ptr91, %for.end173 ]
  %mul179 = mul i64 %n.addr.0, %es
  %add.ptr180 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul179
  %cmp181 = icmp ult i8* %pm.3, %add.ptr180
  br i1 %cmp181, label %for.cond184, label %cleanup

for.cond184:                                      ; preds = %for.cond178, %for.inc218
  %pl.2 = phi i8* [ %add.ptr189, %for.inc218 ], [ %pm.3, %for.cond178 ]
  %cmp185 = icmp ugt i8* %pl.2, %a.addr.0
  br i1 %cmp185, label %land.rhs187, label %for.inc222

land.rhs187:                                      ; preds = %for.cond184
  %idx.neg188 = sub i64 0, %es
  %add.ptr189 = getelementptr inbounds i8, i8* %pl.2, i64 %idx.neg188
  %call190 = call i32 %cmp(i8* %add.ptr189, i8* %pl.2) #3
  %cmp191 = icmp sgt i32 %call190, 0
  br i1 %cmp191, label %for.body194, label %for.inc222

for.body194:                                      ; preds = %land.rhs187
  br i1 %cmp78, label %if.then197, label %if.else203

if.then197:                                       ; preds = %for.body194
  %42 = bitcast i8* %pl.2 to i64*
  %43 = load i64, i64* %42, align 8
  %44 = bitcast i8* %add.ptr189 to i64*
  %45 = load i64, i64* %44, align 8
  store i64 %45, i64* %42, align 8
  store i64 %43, i64* %44, align 8
  br label %for.inc218

if.else203:                                       ; preds = %for.body194
  %cmp204 = icmp eq i32 %1, 0
  br i1 %cmp204, label %if.then206, label %if.else212

if.then206:                                       ; preds = %if.else203
  %46 = bitcast i8* %pl.2 to i32*
  %47 = load i32, i32* %46, align 4
  %48 = bitcast i8* %add.ptr189 to i32*
  %49 = load i32, i32* %48, align 4
  store i32 %49, i32* %46, align 4
  store i32 %47, i32* %48, align 4
  br label %for.inc218

if.else212:                                       ; preds = %if.else203
  %conv215 = trunc i64 %es to i32
  call fastcc void @swapfunc(i8* %pl.2, i8* %add.ptr189, i32 %conv215, i32 %0, i32 %1)
  br label %for.inc218

for.inc218:                                       ; preds = %if.then197, %if.else212, %if.then206
  br label %for.cond184

for.inc222:                                       ; preds = %land.rhs187, %for.cond184
  %add.ptr223 = getelementptr inbounds i8, i8* %pm.3, i64 %es
  br label %for.cond178

if.end225:                                        ; preds = %for.end173
  %mul226 = mul i64 %n.addr.0, %es
  %add.ptr227 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul226
  %sub.ptr.lhs.cast228 = ptrtoint i8* %pa.1 to i64
  %sub.ptr.sub229 = sub i64 %sub.ptr.lhs.cast228, %sub.ptr.lhs.cast
  %sub.ptr.lhs.cast230 = ptrtoint i8* %pb.1 to i64
  %sub.ptr.sub232 = sub i64 %sub.ptr.lhs.cast230, %sub.ptr.lhs.cast228
  %cmp233 = icmp slt i64 %sub.ptr.sub229, %sub.ptr.sub232
  %spec.select = select i1 %cmp233, i64 %sub.ptr.sub229, i64 %sub.ptr.sub232
  %cmp245 = icmp eq i64 %spec.select, 0
  br i1 %cmp245, label %if.end251, label %if.then247

if.then247:                                       ; preds = %if.end225
  %idx.neg248 = sub i64 0, %spec.select
  %add.ptr249 = getelementptr inbounds i8, i8* %pb.1, i64 %idx.neg248
  %conv250 = trunc i64 %spec.select to i32
  call fastcc void @swapfunc(i8* %a.addr.0, i8* %add.ptr249, i32 %conv250, i32 %0, i32 %1)
  br label %if.end251

if.end251:                                        ; preds = %if.end225, %if.then247
  %sub.ptr.lhs.cast252 = ptrtoint i8* %pd.1 to i64
  %sub.ptr.rhs.cast253 = ptrtoint i8* %pc.1 to i64
  %sub.ptr.sub254 = sub i64 %sub.ptr.lhs.cast252, %sub.ptr.rhs.cast253
  %sub.ptr.lhs.cast255 = ptrtoint i8* %add.ptr227 to i64
  %sub.ptr.sub257 = sub i64 %sub.ptr.lhs.cast255, %sub.ptr.lhs.cast252
  %sub258 = sub nsw i64 %sub.ptr.sub257, %es
  %cmp259 = icmp slt i64 %sub.ptr.sub254, %sub258
  %spec.select572 = select i1 %cmp259, i64 %sub.ptr.sub254, i64 %sub258
  %cmp272 = icmp eq i64 %spec.select572, 0
  br i1 %cmp272, label %if.end278, label %if.then274

if.then274:                                       ; preds = %if.end251
  %idx.neg275 = sub i64 0, %spec.select572
  %add.ptr276 = getelementptr inbounds i8, i8* %add.ptr227, i64 %idx.neg275
  %conv277 = trunc i64 %spec.select572 to i32
  call fastcc void @swapfunc(i8* %pb.1, i8* %add.ptr276, i32 %conv277, i32 %0, i32 %1)
  br label %if.end278

if.end278:                                        ; preds = %if.end251, %if.then274
  %cmp282 = icmp ugt i64 %sub.ptr.sub232, %es
  br i1 %cmp282, label %if.then284, label %if.end286

if.then284:                                       ; preds = %if.end278
  %div285 = udiv i64 %sub.ptr.sub232, %es
  call void @spec_qsort(i8* %a.addr.0, i64 %div285, i64 %es, i32 (i8*, i8*)* %cmp)
  br label %if.end286

if.end286:                                        ; preds = %if.then284, %if.end278
  %cmp290 = icmp ugt i64 %sub.ptr.sub254, %es
  br i1 %cmp290, label %if.then292, label %cleanup

if.then292:                                       ; preds = %if.end286
  %idx.neg293 = sub i64 0, %sub.ptr.sub254
  %add.ptr294 = getelementptr inbounds i8, i8* %add.ptr227, i64 %idx.neg293
  %div295 = udiv i64 %sub.ptr.sub254, %es
  br label %loop

cleanup:                                          ; preds = %if.end286, %for.cond178, %for.cond
  ret void
}

attributes #0 = { "is-qsort-spec_qsort" }
attributes #1 = { "must-be-qsort-swapfunc" "is-qsort-swapfunc" }
attributes #2 = { "must-be-qsort-med3" }
attributes #3 = { "must-be-qsort-compare" }

