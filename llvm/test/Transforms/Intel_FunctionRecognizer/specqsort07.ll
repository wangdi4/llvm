; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced,intel_feature_sw_dtrans,asserts
; UNSUPPORTED: windows
; RUN: opt < %s -enable-dtrans -passes='function(functionrecognizer)' -debug-only=functionrecognizer,functionrecognizer-verbose -S 2>&1 | FileCheck %s

; Test that on Linux @spec_qsort is not recognized as a qsort spec_qsort,
; because the main whole loop is not indexed correctly with array pointers.

; CHECK: FXNREC: SPEC_QSORT: spec_qsort: Is not main while loop.
; CHECK-NOT: FUNCTION-RECOGNIZER: FOUND QSORT-SPEC_QSORT spec_qsort
; CHECK: define{{.*}}@spec_qsort{{.*}}
; CHECK-NOT: attributes #0 = { "is-qsort-spec_qsort" }

declare i8* @med3(i8* %a, i8* %b, i8* %c, i32 (i8*, i8*)* %cmp)

declare void @swapfunc(i8* %a, i8* %b, i32 %n, i32 %swaptype_long, i32 %swaptype_int)

define dso_local void @spec_qsort(i8* %a, i64 %n, i64 %es, i32 (i8*, i8*)* %cmp) #0 {
entry:
  br label %loop

loop:                                             ; preds = %if.then292, %entry
  %n.addr.0 = phi i64 [ %n, %entry ], [ %div295, %if.then292 ]
  %a.addr.0 = phi i8* [ %a, %entry ], [ %add.ptr294, %if.then292 ]
  %sub.ptr.lhs.cast = ptrtoint i8* %a.addr.0 to i64
  %sub.ptr.sub = sub i64 %sub.ptr.lhs.cast, 0
  %rem = urem i64 %sub.ptr.sub, 8
  %tobool = icmp ne i64 %rem, 0
  br i1 %tobool, label %cond.end, label %lor.lhs.false

lor.lhs.false:                                    ; preds = %loop
  %rem1 = urem i64 %es, 8
  %tobool2 = icmp ne i64 %rem1, 0
  br i1 %tobool2, label %cond.end, label %cond.false

cond.false:                                       ; preds = %lor.lhs.false
  %cmp3 = icmp eq i64 %es, 8
  %0 = zext i1 %cmp3 to i64
  %cond = select i1 %cmp3, i32 0, i32 1
  br label %cond.end

cond.end:                                         ; preds = %loop, %lor.lhs.false, %cond.false
  %cond4 = phi i32 [ %cond, %cond.false ], [ 2, %lor.lhs.false ], [ 2, %loop ]
  %sub.ptr.lhs.cast5 = ptrtoint i8* %a.addr.0 to i64
  %sub.ptr.sub6 = sub i64 %sub.ptr.lhs.cast5, 0
  %rem7 = urem i64 %sub.ptr.sub6, 4
  %tobool8 = icmp ne i64 %rem7, 0
  br i1 %tobool8, label %cond.end16, label %lor.lhs.false9

lor.lhs.false9:                                   ; preds = %cond.end
  %rem10 = urem i64 %es, 4
  %tobool11 = icmp ne i64 %rem10, 0
  br i1 %tobool11, label %cond.end16, label %cond.false13

cond.false13:                                     ; preds = %lor.lhs.false9
  %cmp14 = icmp eq i64 %es, 4
  %1 = zext i1 %cmp14 to i64
  %cond15 = select i1 %cmp14, i32 0, i32 1
  br label %cond.end16

cond.end16:                                       ; preds = %cond.end, %lor.lhs.false9, %cond.false13
  %cond17 = phi i32 [ %cond15, %cond.false13 ], [ 2, %lor.lhs.false9 ], [ 2, %cond.end ]
  %cmp18 = icmp ult i64 %n.addr.0, 7
  br i1 %cmp18, label %if.then, label %if.end48

if.then:                                          ; preds = %cond.end16
  %add.ptr = getelementptr inbounds i8, i8* %a.addr.0, i64 %es
  br label %for.cond

for.cond:                                         ; preds = %for.inc45, %if.then
  %pm.0 = phi i8* [ %add.ptr, %if.then ], [ %add.ptr46, %for.inc45 ]
  %mul = mul i64 %n.addr.0, %es
  %add.ptr19 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul
  %cmp20 = icmp ult i8* %pm.0, %add.ptr19
  br i1 %cmp20, label %for.body, label %for.end47

for.body:                                         ; preds = %for.cond
  br label %for.cond21

for.cond21:                                       ; preds = %for.inc, %for.body
  %pl.0 = phi i8* [ %pm.0, %for.body ], [ %add.ptr44, %for.inc ]
  %cmp22 = icmp ugt i8* %pl.0, %a.addr.0
  br i1 %cmp22, label %land.rhs, label %for.inc45

land.rhs:                                         ; preds = %for.cond21
  %idx.neg = sub i64 0, %es
  %add.ptr23 = getelementptr inbounds i8, i8* %pl.0, i64 %idx.neg
  %call = call i32 %cmp(i8* %add.ptr23, i8* %pl.0)
  %cmp24 = icmp sgt i32 %call, 0
  br i1 %cmp24, label %for.body25, label %for.inc45

for.body25:                                       ; preds = %land.rhs
  %cmp26 = icmp eq i32 %cond4, 0
  br i1 %cmp26, label %if.then27, label %if.else

if.then27:                                        ; preds = %for.body25
  %2 = bitcast i8* %pl.0 to i64*
  %3 = load i64, i64* %2, align 8
  %idx.neg28 = sub i64 0, %es
  %add.ptr29 = getelementptr inbounds i8, i8* %pl.0, i64 %idx.neg28
  %4 = bitcast i8* %add.ptr29 to i64*
  %5 = load i64, i64* %4, align 8
  %6 = bitcast i8* %pl.0 to i64*
  store i64 %5, i64* %6, align 8
  %idx.neg30 = sub i64 0, %es
  %add.ptr31 = getelementptr inbounds i8, i8* %pl.0, i64 %idx.neg30
  %7 = bitcast i8* %add.ptr31 to i64*
  store i64 %3, i64* %7, align 8
  br label %for.inc

if.else:                                          ; preds = %for.body25
  %cmp32 = icmp eq i32 %cond17, 0
  br i1 %cmp32, label %if.then33, label %if.else39

if.then33:                                        ; preds = %if.else
  %8 = bitcast i8* %pl.0 to i32*
  %9 = load i32, i32* %8, align 4
  %idx.neg35 = sub i64 0, %es
  %add.ptr36 = getelementptr inbounds i8, i8* %pl.0, i64 %idx.neg35
  %10 = bitcast i8* %add.ptr36 to i32*
  %11 = load i32, i32* %10, align 4
  %12 = bitcast i8* %pl.0 to i32*
  store i32 %11, i32* %12, align 4
  %idx.neg37 = sub i64 0, %es
  %add.ptr38 = getelementptr inbounds i8, i8* %pl.0, i64 %idx.neg37
  %13 = bitcast i8* %add.ptr38 to i32*
  store i32 %9, i32* %13, align 4
  br label %for.inc

if.else39:                                        ; preds = %if.else
  %idx.neg40 = sub i64 0, %es
  %add.ptr41 = getelementptr inbounds i8, i8* %pl.0, i64 %idx.neg40
  %conv = trunc i64 %es to i32
  call void @swapfunc(i8* %pl.0, i8* %add.ptr41, i32 %conv, i32 %cond4, i32 %cond17)
  br label %for.inc

for.inc:                                          ; preds = %if.then27, %if.else39, %if.then33
  %idx.neg43 = sub i64 0, %es
  %add.ptr44 = getelementptr inbounds i8, i8* %pl.0, i64 %idx.neg43
  br label %for.cond21

for.inc45:                                        ; preds = %land.rhs, %for.cond21
  %add.ptr46 = getelementptr inbounds i8, i8* %pm.0, i64 %es
  br label %for.cond

for.end47:                                        ; preds = %for.cond
  br label %cleanup

if.end48:                                         ; preds = %cond.end16
  %div = udiv i64 %n.addr.0, 2
  %mul49 = mul i64 %div, %es
  %add.ptr50 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul49
  %cmp51 = icmp ugt i64 %n.addr.0, 7
  br i1 %cmp51, label %if.then53, label %if.end77

if.then53:                                        ; preds = %if.end48
  %sub = sub i64 %n.addr.0, 1
  %mul54 = mul i64 %sub, %es
  %add.ptr55 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul54
  %cmp56 = icmp ugt i64 %n.addr.0, 40
  br i1 %cmp56, label %if.then58, label %if.end75

if.then58:                                        ; preds = %if.then53
  %div59 = udiv i64 %n.addr.0, 8
  %mul60 = mul i64 %div59, %es
  %add.ptr61 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul60
  %mul62 = mul i64 2, %mul60
  %add.ptr63 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul62
  %call64 = call i8* @med3(i8* %a.addr.0, i8* %add.ptr61, i8* %add.ptr63, i32 (i8*, i8*)* %cmp)
  %idx.neg65 = sub i64 0, %mul60
  %add.ptr66 = getelementptr inbounds i8, i8* %add.ptr50, i64 %idx.neg65
  %add.ptr67 = getelementptr inbounds i8, i8* %add.ptr50, i64 %mul60
  %call68 = call i8* @med3(i8* %add.ptr66, i8* %add.ptr50, i8* %add.ptr67, i32 (i8*, i8*)* %cmp)
  %mul69 = mul i64 2, %mul60
  %idx.neg70 = sub i64 0, %mul69
  %add.ptr71 = getelementptr inbounds i8, i8* %add.ptr55, i64 %idx.neg70
  %idx.neg72 = sub i64 0, %mul60
  %add.ptr73 = getelementptr inbounds i8, i8* %add.ptr55, i64 %idx.neg72
  %call74 = call i8* @med3(i8* %add.ptr71, i8* %add.ptr73, i8* %add.ptr55, i32 (i8*, i8*)* %cmp)
  br label %if.end75

if.end75:                                         ; preds = %if.then58, %if.then53
  %pn.0 = phi i8* [ %call74, %if.then58 ], [ %add.ptr55, %if.then53 ]
  %pm.1 = phi i8* [ %call68, %if.then58 ], [ %add.ptr50, %if.then53 ]
  %pl.1 = phi i8* [ %call64, %if.then58 ], [ %a.addr.0, %if.then53 ]
  %call76 = call i8* @med3(i8* %pl.1, i8* %pm.1, i8* %pn.0, i32 (i8*, i8*)* %cmp)
  br label %if.end77

if.end77:                                         ; preds = %if.end75, %if.end48
  %pm.2 = phi i8* [ %call76, %if.end75 ], [ %add.ptr50, %if.end48 ]
  %cmp78 = icmp eq i32 %cond4, 0
  br i1 %cmp78, label %if.then80, label %if.else82

if.then80:                                        ; preds = %if.end77
  %14 = bitcast i8* %a.addr.0 to i64*
  %15 = load i64, i64* %14, align 8
  %16 = bitcast i8* %pm.2 to i64*
  %17 = load i64, i64* %16, align 8
  %18 = bitcast i8* %a.addr.0 to i64*
  store i64 %17, i64* %18, align 8
  %19 = bitcast i8* %pm.2 to i64*
  store i64 %15, i64* %19, align 8
  br label %if.end90

if.else82:                                        ; preds = %if.end77
  %cmp83 = icmp eq i32 %cond17, 0
  br i1 %cmp83, label %if.then85, label %if.else87

if.then85:                                        ; preds = %if.else82
  %20 = bitcast i8* %a.addr.0 to i32*
  %21 = load i32, i32* %20, align 4
  %22 = bitcast i8* %pm.2 to i32*
  %23 = load i32, i32* %22, align 4
  %24 = bitcast i8* %a.addr.0 to i32*
  store i32 %23, i32* %24, align 4
  %25 = bitcast i8* %pm.2 to i32*
  store i32 %21, i32* %25, align 4
  br label %if.end90

if.else87:                                        ; preds = %if.else82
  %conv88 = trunc i64 %es to i32
  call void @swapfunc(i8* %a.addr.0, i8* %pm.2, i32 %conv88, i32 %cond4, i32 %cond17)
  br label %if.end90

if.end90:                                         ; preds = %if.then85, %if.else87, %if.then80
  %add.ptr91 = getelementptr inbounds i8, i8* %a.addr.0, i64 %es
  %sub92 = sub i64 %n.addr.0, 1
  %mul93 = mul i64 %sub92, %es
  %add.ptr94 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul93
  br label %for.cond95

for.cond95:                                       ; preds = %if.end169, %if.end90
  %swap_cnt.0 = phi i32 [ 0, %if.end90 ], [ 1, %if.end169 ]
  %pd.0 = phi i8* [ %add.ptr91, %if.end90 ], [ %pd.1, %if.end169 ]
  %pc.0 = phi i8* [ %add.ptr91, %if.end90 ], [ %add.ptr172, %if.end169 ]
  %pb.0 = phi i8* [ %add.ptr94, %if.end90 ], [ %add.ptr170, %if.end169 ]
  %pa.0 = phi i8* [ %add.ptr94, %if.end90 ], [ %pa.1, %if.end169 ]
  br label %while.cond

while.cond:                                       ; preds = %if.end120, %for.cond95
  %swap_cnt.1 = phi i32 [ %swap_cnt.0, %for.cond95 ], [ %swap_cnt.2, %if.end120 ]
  %pb.1 = phi i8* [ %pb.0, %for.cond95 ], [ %add.ptr121, %if.end120 ]
  %pa.1 = phi i8* [ %pa.0, %for.cond95 ], [ %pa.2, %if.end120 ]
  %cmp96 = icmp ule i8* %pb.1, %pc.0
  br i1 %cmp96, label %land.rhs98, label %while.end

land.rhs98:                                       ; preds = %while.cond
  %call99 = call i32 %cmp(i8* %pb.1, i8* %a.addr.0)
  %cmp100 = icmp sle i32 %call99, 0
  br i1 %cmp100, label %while.body, label %while.end

while.body:                                       ; preds = %land.rhs98
  %cmp103 = icmp eq i32 %call99, 0
  br i1 %cmp103, label %if.then105, label %if.end120

if.then105:                                       ; preds = %while.body
  %cmp106 = icmp eq i32 %cond4, 0
  br i1 %cmp106, label %if.then108, label %if.else110

if.then108:                                       ; preds = %if.then105
  %26 = bitcast i8* %pa.1 to i64*
  %27 = load i64, i64* %26, align 8
  %28 = bitcast i8* %pb.1 to i64*
  %29 = load i64, i64* %28, align 8
  %30 = bitcast i8* %pa.1 to i64*
  store i64 %29, i64* %30, align 8
  %31 = bitcast i8* %pb.1 to i64*
  store i64 %27, i64* %31, align 8
  br label %if.end118

if.else110:                                       ; preds = %if.then105
  %cmp111 = icmp eq i32 %cond17, 0
  br i1 %cmp111, label %if.then113, label %if.else115

if.then113:                                       ; preds = %if.else110
  %32 = bitcast i8* %pa.1 to i32*
  %33 = load i32, i32* %32, align 4
  %34 = bitcast i8* %pb.1 to i32*
  %35 = load i32, i32* %34, align 4
  %36 = bitcast i8* %pa.1 to i32*
  store i32 %35, i32* %36, align 4
  %37 = bitcast i8* %pb.1 to i32*
  store i32 %33, i32* %37, align 4
  br label %if.end118

if.else115:                                       ; preds = %if.else110
  %conv116 = trunc i64 %es to i32
  call void @swapfunc(i8* %pa.1, i8* %pb.1, i32 %conv116, i32 %cond4, i32 %cond17)
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
  %cmp123 = icmp ule i8* %pb.1, %pc.1
  br i1 %cmp123, label %land.rhs125, label %while.end152

land.rhs125:                                      ; preds = %while.cond122
  %call126 = call i32 %cmp(i8* %pc.1, i8* %a.addr.0)
  %cmp127 = icmp sge i32 %call126, 0
  br i1 %cmp127, label %while.body130, label %while.end152

while.body130:                                    ; preds = %land.rhs125
  %cmp131 = icmp eq i32 %call126, 0
  br i1 %cmp131, label %if.then133, label %if.end149

if.then133:                                       ; preds = %while.body130
  %cmp134 = icmp eq i32 %cond4, 0
  br i1 %cmp134, label %if.then136, label %if.else138

if.then136:                                       ; preds = %if.then133
  %38 = bitcast i8* %pc.1 to i64*
  %39 = load i64, i64* %38, align 8
  %40 = bitcast i8* %pd.1 to i64*
  %41 = load i64, i64* %40, align 8
  %42 = bitcast i8* %pc.1 to i64*
  store i64 %41, i64* %42, align 8
  %43 = bitcast i8* %pd.1 to i64*
  store i64 %39, i64* %43, align 8
  br label %if.end146

if.else138:                                       ; preds = %if.then133
  %cmp139 = icmp eq i32 %cond17, 0
  br i1 %cmp139, label %if.then141, label %if.else143

if.then141:                                       ; preds = %if.else138
  %44 = bitcast i8* %pc.1 to i32*
  %45 = load i32, i32* %44, align 4
  %46 = bitcast i8* %pd.1 to i32*
  %47 = load i32, i32* %46, align 4
  %48 = bitcast i8* %pc.1 to i32*
  store i32 %47, i32* %48, align 4
  %49 = bitcast i8* %pd.1 to i32*
  store i32 %45, i32* %49, align 4
  br label %if.end146

if.else143:                                       ; preds = %if.else138
  %conv144 = trunc i64 %es to i32
  call void @swapfunc(i8* %pc.1, i8* %pd.1, i32 %conv144, i32 %cond4, i32 %cond17)
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
  %cmp157 = icmp eq i32 %cond4, 0
  br i1 %cmp157, label %if.then159, label %if.else161

if.then159:                                       ; preds = %if.end156
  %50 = bitcast i8* %pb.1 to i64*
  %51 = load i64, i64* %50, align 8
  %52 = bitcast i8* %pc.1 to i64*
  %53 = load i64, i64* %52, align 8
  %54 = bitcast i8* %pb.1 to i64*
  store i64 %53, i64* %54, align 8
  %55 = bitcast i8* %pc.1 to i64*
  store i64 %51, i64* %55, align 8
  br label %if.end169

if.else161:                                       ; preds = %if.end156
  %cmp162 = icmp eq i32 %cond17, 0
  br i1 %cmp162, label %if.then164, label %if.else166

if.then164:                                       ; preds = %if.else161
  %56 = bitcast i8* %pb.1 to i32*
  %57 = load i32, i32* %56, align 4
  %58 = bitcast i8* %pc.1 to i32*
  %59 = load i32, i32* %58, align 4
  %60 = bitcast i8* %pb.1 to i32*
  store i32 %59, i32* %60, align 4
  %61 = bitcast i8* %pc.1 to i32*
  store i32 %57, i32* %61, align 4
  br label %if.end169

if.else166:                                       ; preds = %if.else161
  %conv167 = trunc i64 %es to i32
  call void @swapfunc(i8* %pb.1, i8* %pc.1, i32 %conv167, i32 %cond4, i32 %cond17)
  br label %if.end169

if.end169:                                        ; preds = %if.then164, %if.else166, %if.then159
  %add.ptr170 = getelementptr inbounds i8, i8* %pb.1, i64 %es
  %idx.neg171 = sub i64 0, %es
  %add.ptr172 = getelementptr inbounds i8, i8* %pc.1, i64 %idx.neg171
  br label %for.cond95

for.end173:                                       ; preds = %while.end152
  %cmp174 = icmp eq i32 %swap_cnt.3, 0
  br i1 %cmp174, label %if.then176, label %if.end225

if.then176:                                       ; preds = %for.end173
  %add.ptr177 = getelementptr inbounds i8, i8* %a.addr.0, i64 %es
  br label %for.cond178

for.cond178:                                      ; preds = %for.inc222, %if.then176
  %pm.3 = phi i8* [ %add.ptr177, %if.then176 ], [ %add.ptr223, %for.inc222 ]
  %mul179 = mul i64 %n.addr.0, %es
  %add.ptr180 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul179
  %cmp181 = icmp ult i8* %pm.3, %add.ptr180
  br i1 %cmp181, label %for.body183, label %for.end224

for.body183:                                      ; preds = %for.cond178
  br label %for.cond184

for.cond184:                                      ; preds = %for.inc218, %for.body183
  %pl.2 = phi i8* [ %pm.3, %for.body183 ], [ %add.ptr220, %for.inc218 ]
  %cmp185 = icmp ugt i8* %pl.2, %a.addr.0
  br i1 %cmp185, label %land.rhs187, label %for.inc222

land.rhs187:                                      ; preds = %for.cond184
  %idx.neg188 = sub i64 0, %es
  %add.ptr189 = getelementptr inbounds i8, i8* %pl.2, i64 %idx.neg188
  %call190 = call i32 %cmp(i8* %add.ptr189, i8* %pl.2)
  %cmp191 = icmp sgt i32 %call190, 0
  br i1 %cmp191, label %for.body194, label %for.inc222

for.body194:                                      ; preds = %land.rhs187
  %cmp195 = icmp eq i32 %cond4, 0
  br i1 %cmp195, label %if.then197, label %if.else203

if.then197:                                       ; preds = %for.body194
  %62 = bitcast i8* %pl.2 to i64*
  %63 = load i64, i64* %62, align 8
  %idx.neg199 = sub i64 0, %es
  %add.ptr200 = getelementptr inbounds i8, i8* %pl.2, i64 %idx.neg199
  %64 = bitcast i8* %add.ptr200 to i64*
  %65 = load i64, i64* %64, align 8
  %66 = bitcast i8* %pl.2 to i64*
  store i64 %65, i64* %66, align 8
  %idx.neg201 = sub i64 0, %es
  %add.ptr202 = getelementptr inbounds i8, i8* %pl.2, i64 %idx.neg201
  %67 = bitcast i8* %add.ptr202 to i64*
  store i64 %63, i64* %67, align 8
  br label %for.inc218

if.else203:                                       ; preds = %for.body194
  %cmp204 = icmp eq i32 %cond17, 0
  br i1 %cmp204, label %if.then206, label %if.else212

if.then206:                                       ; preds = %if.else203
  %68 = bitcast i8* %pl.2 to i32*
  %69 = load i32, i32* %68, align 4
  %idx.neg208 = sub i64 0, %es
  %add.ptr209 = getelementptr inbounds i8, i8* %pl.2, i64 %idx.neg208
  %70 = bitcast i8* %add.ptr209 to i32*
  %71 = load i32, i32* %70, align 4
  %72 = bitcast i8* %pl.2 to i32*
  store i32 %71, i32* %72, align 4
  %idx.neg210 = sub i64 0, %es
  %add.ptr211 = getelementptr inbounds i8, i8* %pl.2, i64 %idx.neg210
  %73 = bitcast i8* %add.ptr211 to i32*
  store i32 %69, i32* %73, align 4
  br label %for.inc218

if.else212:                                       ; preds = %if.else203
  %idx.neg213 = sub i64 0, %es
  %add.ptr214 = getelementptr inbounds i8, i8* %pl.2, i64 %idx.neg213
  %conv215 = trunc i64 %es to i32
  call void @swapfunc(i8* %pl.2, i8* %add.ptr214, i32 %conv215, i32 %cond4, i32 %cond17)
  br label %for.inc218

for.inc218:                                       ; preds = %if.then197, %if.else212, %if.then206
  %idx.neg219 = sub i64 0, %es
  %add.ptr220 = getelementptr inbounds i8, i8* %pl.2, i64 %idx.neg219
  br label %for.cond184

for.inc222:                                       ; preds = %land.rhs187, %for.cond184
  %add.ptr223 = getelementptr inbounds i8, i8* %pm.3, i64 %es
  br label %for.cond178

for.end224:                                       ; preds = %for.cond178
  br label %cleanup

if.end225:                                        ; preds = %for.end173
  %mul226 = mul i64 %n.addr.0, %es
  %add.ptr227 = getelementptr inbounds i8, i8* %a.addr.0, i64 %mul226
  %sub.ptr.lhs.cast228 = ptrtoint i8* %pa.1 to i64
  %sub.ptr.rhs.cast = ptrtoint i8* %a.addr.0 to i64
  %sub.ptr.sub229 = sub i64 %sub.ptr.lhs.cast228, %sub.ptr.rhs.cast
  %sub.ptr.lhs.cast230 = ptrtoint i8* %pb.1 to i64
  %sub.ptr.rhs.cast231 = ptrtoint i8* %pa.1 to i64
  %sub.ptr.sub232 = sub i64 %sub.ptr.lhs.cast230, %sub.ptr.rhs.cast231
  %cmp233 = icmp slt i64 %sub.ptr.sub229, %sub.ptr.sub232
  br i1 %cmp233, label %cond.true235, label %cond.false239

cond.true235:                                     ; preds = %if.end225
  %sub.ptr.lhs.cast236 = ptrtoint i8* %pa.1 to i64
  %sub.ptr.rhs.cast237 = ptrtoint i8* %a.addr.0 to i64
  %sub.ptr.sub238 = sub i64 %sub.ptr.lhs.cast236, %sub.ptr.rhs.cast237
  br label %cond.end243

cond.false239:                                    ; preds = %if.end225
  %sub.ptr.lhs.cast240 = ptrtoint i8* %pb.1 to i64
  %sub.ptr.rhs.cast241 = ptrtoint i8* %pa.1 to i64
  %sub.ptr.sub242 = sub i64 %sub.ptr.lhs.cast240, %sub.ptr.rhs.cast241
  br label %cond.end243

cond.end243:                                      ; preds = %cond.false239, %cond.true235
  %cond244 = phi i64 [ %sub.ptr.sub238, %cond.true235 ], [ %sub.ptr.sub242, %cond.false239 ]
  %cmp245 = icmp ugt i64 %cond244, 0
  br i1 %cmp245, label %if.then247, label %if.end251

if.then247:                                       ; preds = %cond.end243
  %idx.neg248 = sub i64 0, %cond244
  %add.ptr249 = getelementptr inbounds i8, i8* %pb.1, i64 %idx.neg248
  %conv250 = trunc i64 %cond244 to i32
  call void @swapfunc(i8* %a.addr.0, i8* %add.ptr249, i32 %conv250, i32 %cond4, i32 %cond17)
  br label %if.end251

if.end251:                                        ; preds = %if.then247, %cond.end243
  %sub.ptr.lhs.cast252 = ptrtoint i8* %pd.1 to i64
  %sub.ptr.rhs.cast253 = ptrtoint i8* %pc.1 to i64
  %sub.ptr.sub254 = sub i64 %sub.ptr.lhs.cast252, %sub.ptr.rhs.cast253
  %sub.ptr.lhs.cast255 = ptrtoint i8* %add.ptr227 to i64
  %sub.ptr.rhs.cast256 = ptrtoint i8* %pd.1 to i64
  %sub.ptr.sub257 = sub i64 %sub.ptr.lhs.cast255, %sub.ptr.rhs.cast256
  %sub258 = sub nsw i64 %sub.ptr.sub257, %es
  %cmp259 = icmp slt i64 %sub.ptr.sub254, %sub258
  br i1 %cmp259, label %cond.true261, label %cond.false265

cond.true261:                                     ; preds = %if.end251
  %sub.ptr.lhs.cast262 = ptrtoint i8* %pd.1 to i64
  %sub.ptr.rhs.cast263 = ptrtoint i8* %pc.1 to i64
  %sub.ptr.sub264 = sub i64 %sub.ptr.lhs.cast262, %sub.ptr.rhs.cast263
  br label %cond.end270

cond.false265:                                    ; preds = %if.end251
  %sub.ptr.lhs.cast266 = ptrtoint i8* %add.ptr227 to i64
  %sub.ptr.rhs.cast267 = ptrtoint i8* %pd.1 to i64
  %sub.ptr.sub268 = sub i64 %sub.ptr.lhs.cast266, %sub.ptr.rhs.cast267
  %sub269 = sub nsw i64 %sub.ptr.sub268, %es
  br label %cond.end270

cond.end270:                                      ; preds = %cond.false265, %cond.true261
  %cond271 = phi i64 [ %sub.ptr.sub264, %cond.true261 ], [ %sub269, %cond.false265 ]
  %cmp272 = icmp ugt i64 %cond271, 0
  br i1 %cmp272, label %if.then274, label %if.end278

if.then274:                                       ; preds = %cond.end270
  %idx.neg275 = sub i64 0, %cond271
  %add.ptr276 = getelementptr inbounds i8, i8* %add.ptr227, i64 %idx.neg275
  %conv277 = trunc i64 %cond271 to i32
  call void @swapfunc(i8* %pb.1, i8* %add.ptr276, i32 %conv277, i32 %cond4, i32 %cond17)
  br label %if.end278

if.end278:                                        ; preds = %if.then274, %cond.end270
  %sub.ptr.lhs.cast279 = ptrtoint i8* %pb.1 to i64
  %sub.ptr.rhs.cast280 = ptrtoint i8* %pa.1 to i64
  %sub.ptr.sub281 = sub i64 %sub.ptr.lhs.cast279, %sub.ptr.rhs.cast280
  %cmp282 = icmp ugt i64 %sub.ptr.sub281, %es
  br i1 %cmp282, label %if.then284, label %if.end286

if.then284:                                       ; preds = %if.end278
  %div285 = udiv i64 %sub.ptr.sub281, %es
  call void @spec_qsort(i8* %a.addr.0, i64 %div285, i64 %es, i32 (i8*, i8*)* %cmp)
  br label %if.end286

if.end286:                                        ; preds = %if.then284, %if.end278
  %sub.ptr.lhs.cast287 = ptrtoint i8* %pd.1 to i64
  %sub.ptr.rhs.cast288 = ptrtoint i8* %pc.1 to i64
  %sub.ptr.sub289 = sub i64 %sub.ptr.lhs.cast287, %sub.ptr.rhs.cast288
  %cmp290 = icmp ugt i64 %sub.ptr.sub289, %es
  br i1 %cmp290, label %if.then292, label %if.end296

if.then292:                                       ; preds = %if.end286
  %idx.neg293 = sub i64 0, %sub.ptr.sub289
  %add.ptr294 = getelementptr inbounds i8, i8* %add.ptr227, i64 %idx.neg293
  %div295 = udiv i64 %sub.ptr.sub289, %es
  br label %loop

if.end296:                                        ; preds = %if.end286
  br label %cleanup

cleanup:                                          ; preds = %if.end296, %for.end224, %for.end47
  ret void
}
; end INTEL_FEATURE_SW_ADVANCED
