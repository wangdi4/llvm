; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-special-opt-predicate' -disable-output -hir-cost-model-throttling=0 -print-after=hir-special-opt-predicate -hir-details < %s 2>&1 | FileCheck %s
; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-special-opt-predicate' -disable-output -hir-cost-model-throttling=0 -print-changed < %s 2>&1 | FileCheck %s --check-prefix=CHECK-CHANGED

; Check that we recognize the candidate for special opt predicate that looks like
;  for (v = (-height/2 to height/2)
;   for (u = (-width/2 to width/2)
;     if ((v*v+u*u) <= (ssize_t) ((width/2)*(height/2)))
;
; Check the transformation logic that will convert the predicate to 2 different loops - one
; for computing the bounds, and the other that executes the loop with new bounds.
; Also check for ztt for the lower bound of the second inner loop

; HIR
;      + DO i4 = 0, 2 * %sdiv, 1   <DO_LOOP>
;      |   %mul = i4 + -1 * %sdiv  *  i4 + -1 * %sdiv;
;      |   %sitofp120 = sitofp.i64.double(i4 + -1 * %sdiv);
;      |   %fadd = %phi103.out  +  %sitofp120;
;      |
;      |   + DO i5 = 0, 2 * %sdiv41, 1   <DO_LOOP>
;      |   |   %mul127 = i5 + -1 * %sdiv41  *  i5 + -1 * %sdiv41;
;      |   |   %add = %mul127  +  %mul;
;      |   |   if (%add <= ((%arg1 /u 2) * (%arg2 /u 2)))
;      |   |   {
;              ....



; CHECK: + DO i64 i4 = 0, 2 * %sdiv, 1   <DO_LOOP>
; CHECK: |   %mul = i4 + -1 * %sdiv  *  i4 + -1 * %sdiv;
; CHECK: |   %sitofp120 = sitofp.i64.double(i4 + -1 * %sdiv);
; CHECK: |   %fadd = %phi103.out  +  %sitofp120;
; CHECK: |   %optprd.lower = -1;
; CHECK: |   %optprd.upper = -1;
; CHECK: |
; CHECK: |   + DO i64 i5 = 0, 2 * %sdiv41, 1   <DO_MULTI_EXIT_LOOP>
; CHECK: |   |   %mul127 = i5 + -1 * %sdiv41  *  i5 + -1 * %sdiv41;
; CHECK: |   |   %add = %mul127  +  %mul;
; CHECK: |   |   if (%add <= ((%arg1 /u 2) * (%arg2 /u 2)))
; CHECK: |   |   {
; CHECK: |   |      %optprd.lower = i5;
; CHECK: |   |      %optprd.upper = -1 * i5 + 2 * %sdiv41;
; CHECK: |   |      goto loopexit.348;
; CHECK: |   |   }
; CHECK: |   + END LOOP
; CHECK: |
; CHECK: |   loopexit.348:
; CHECK: |

; CHECK:     + Ztt: if (%optprd.lower != -1)
; CHECK: |   + DO i64 i5 = %optprd.lower, %optprd.upper, 1   <DO_LOOP>


; Verify that pass is dumped with print-changed when it triggers.


; CHECK-CHANGED: Dump Before HIRTempCleanup
; CHECK-CHANGED: Dump After HIRSpecialOptPredicate

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.wombat = type { i32, i32, i32, double, i64, float, float, float, float, float }
%struct.barney = type { i16, i16, i16, i16 }
%struct.quux = type { i32, i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, ptr, %struct.barney, %struct.barney, %struct.barney, double, %struct.eggs, i32, ptr, i32, ptr, ptr, ptr, i64, double, double, %struct.widget.2, %struct.widget.2, %struct.widget.2, double, double, double, i32, i32, i32, i32, i32, i32, ptr, i64, i64, i64, i64, i64, i64, %struct.wombat.0, %struct.quux.1, ptr, ptr, ptr, ptr, ptr, ptr, [4096 x i8], [4096 x i8], [4096 x i8], i64, i64, %struct.widget, i32, i64, ptr, %struct.hoge, %struct.hoge, ptr, i64, i64, ptr, ptr, ptr, i32, i32, %struct.barney, ptr, %struct.widget.2, ptr, ptr, i32, i32, i64, i32, i64, i64, i32, i64 }
%struct.eggs = type { %struct.zot, %struct.zot, %struct.zot, %struct.zot }
%struct.zot = type { double, double, double }
%struct.wombat.0 = type { double, double, double }
%struct.quux.1 = type { %struct.bar, %struct.bar, i32, i64 }
%struct.bar = type { double, double, double }
%struct.widget = type { i32, i32, ptr, ptr, ptr, i32, ptr, i64 }
%struct.hoge = type { ptr, i64, ptr, i64 }
%struct.widget.2 = type { i64, i64, i64, i64 }

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #0

; Function Attrs: nounwind uwtable
define "intel_dtrans_func_index"="1" ptr @baz(ptr noundef "intel_dtrans_func_index"="2" %arg, i64 noundef %arg1, i64 noundef %arg2, ptr noundef "intel_dtrans_func_index"="3" %arg4) #1 {
bb:
  %alloca = alloca [4096 x i8], align 16
  %alloca5 = alloca %struct.wombat, align 8
  %alloca6 = alloca %struct.wombat, align 8
  %alloca7 = alloca %struct.barney, align 2
  %getelementptr = getelementptr inbounds %struct.quux, ptr %arg, i64 0, i32 59
  %load = load i32, ptr %getelementptr, align 8
  %icmp = icmp eq i32 %load, 0
  %getelementptr11 = getelementptr inbounds %struct.quux, ptr %arg, i64 0, i32 7
  %load12 = load i64, ptr %getelementptr11, align 8
  %getelementptr13 = getelementptr inbounds %struct.quux, ptr %arg, i64 0, i32 8
  %load14 = load i64, ptr %getelementptr13, align 8
  %call15 = tail call ptr null(ptr noundef nonnull %arg, i64 noundef %load12, i64 noundef %load14, i32 noundef 1, ptr noundef %arg4) #3
  %icmp16 = icmp eq ptr %call15, null
  %call18 = tail call i32 null(ptr noundef nonnull %call15, i32 noundef 1) #3
  %icmp19 = icmp eq i32 %call18, 0
  %call25 = tail call ptr null(ptr noundef nonnull %arg, ptr noundef %arg4) #3
  %call26 = tail call ptr null(ptr noundef nonnull %arg, ptr noundef %arg4) #3
  %call28 = tail call ptr null(ptr noundef nonnull %call15, ptr noundef %arg4) #3
  %getelementptr29 = getelementptr inbounds %struct.quux, ptr %call15, i64 0, i32 8
  %load30 = load i64, ptr %getelementptr29, align 8
  %icmp31 = icmp sgt i64 %load30, 0
  %getelementptr33 = getelementptr inbounds %struct.quux, ptr %call15, i64 0, i32 7
  %getelementptr34 = getelementptr i32, ptr %arg, i64 1
  %getelementptr35 = getelementptr inbounds %struct.wombat, ptr %alloca5, i64 0, i32 5
  %getelementptr36 = getelementptr inbounds %struct.wombat, ptr %alloca5, i64 0, i32 6
  %getelementptr37 = getelementptr inbounds %struct.wombat, ptr %alloca5, i64 0, i32 7
  %getelementptr38 = getelementptr inbounds %struct.wombat, ptr %alloca5, i64 0, i32 8
  %getelementptr39 = getelementptr inbounds %struct.wombat, ptr %alloca5, i64 0, i32 9
  %sdiv = sdiv i64 %arg2, 2
  %sub = sub nsw i64 0, %sdiv
  %icmp40 = icmp slt i64 %sdiv, 0
  %sdiv41 = sdiv i64 %arg1, 2
  %sub42 = sub nsw i64 0, %sdiv41
  %icmp43 = icmp slt i64 %sdiv41, 0
  %lshr = lshr i64 %arg1, 1
  %lshr44 = lshr i64 %arg2, 1
  %mul45 = mul i64 %lshr44, %lshr
  %getelementptr46 = getelementptr inbounds %struct.barney, ptr %alloca7, i64 0, i32 2
  %getelementptr47 = getelementptr inbounds %struct.barney, ptr %alloca7, i64 0, i32 1
  %getelementptr49 = getelementptr inbounds %struct.wombat, ptr %alloca6, i64 0, i32 5
  %getelementptr50 = getelementptr inbounds %struct.wombat, ptr %alloca6, i64 0, i32 6
  %getelementptr51 = getelementptr inbounds %struct.wombat, ptr %alloca6, i64 0, i32 7
  %getelementptr52 = getelementptr inbounds %struct.barney, ptr %alloca7, i64 0, i32 3
  %getelementptr53 = getelementptr inbounds %struct.wombat, ptr %alloca6, i64 0, i32 8
  %getelementptr54 = getelementptr inbounds %struct.quux, ptr %arg, i64 0, i32 47
  %getelementptr56 = getelementptr inbounds %struct.quux, ptr %arg, i64 0, i32 53, i64 0
  %getelementptr57 = getelementptr inbounds %struct.quux, ptr %arg, i64 0, i32 48
  %add58 = add nsw i64 %sdiv41, 1
  %add59 = add nsw i64 %sdiv, 1
  br label %bb60

bb60:                                             ; preds = %bb299, %bb
  %phi = phi i32 [ 1, %bb ], [ %phi301, %bb299 ]
  %phi61 = phi i64 [ 0, %bb ], [ %phi300, %bb299 ]
  %phi62 = phi i64 [ 0, %bb ], [ %add302, %bb299 ]
  %icmp63 = icmp eq i32 %phi, 0
  br i1 %icmp63, label %bb299, label %bb64

bb64:                                             ; preds = %bb60
  %load65 = load i64, ptr %getelementptr11, align 8
  %call66 = call ptr null(ptr noundef %call25, i64 noundef 0, i64 noundef %phi62, i64 noundef %load65, i64 noundef 1, ptr noundef %arg4) #4
  %load67 = load i64, ptr %getelementptr33, align 8
  %call68 = call ptr null(ptr noundef %call28, i64 noundef 0, i64 noundef %phi62, i64 noundef %load67, i64 noundef 1, ptr noundef %arg4) #4
  %icmp69 = icmp eq ptr %call66, null
  %icmp70 = icmp eq ptr %call68, null
  %select = select i1 %icmp69, i1 true, i1 %icmp70
  br i1 %select, label %bb299, label %bb71

bb71:                                             ; preds = %bb64
  %call72 = call ptr null(ptr noundef %call25) #3
  %load73 = load i64, ptr %getelementptr33, align 8
  %icmp74 = icmp sgt i64 %load73, 0
  br i1 %icmp74, label %bb75, label %bb283

bb75:                                             ; preds = %bb71
  %icmp76 = icmp ne ptr %call72, null
  %sitofp = sitofp i64 %phi62 to double
  br label %bb77

bb77:                                             ; preds = %bb244, %bb75
  %phi78 = phi i32 [ %phi, %bb75 ], [ %phi245, %bb244 ]
  %phi79 = phi i64 [ 0, %bb75 ], [ %add278, %bb244 ]
  %phi80 = phi ptr [ %call68, %bb75 ], [ %getelementptr277, %bb244 ]
  %phi81 = phi ptr [ %call66, %bb75 ], [ %getelementptr276, %bb244 ]
  call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %alloca5) #3
  call void null(ptr noundef %arg, ptr noundef nonnull %alloca5) #3
  %load82 = load i32, ptr %getelementptr34, align 4
  %getelementptr83 = getelementptr inbounds %struct.barney, ptr %phi81, i64 0, i32 2
  %load84 = load i16, ptr %getelementptr83, align 2
  %uitofp = uitofp i16 %load84 to float
  store float %uitofp, ptr %getelementptr35, align 8
  %getelementptr85 = getelementptr inbounds %struct.barney, ptr %phi81, i64 0, i32 1
  %load86 = load i16, ptr %getelementptr85, align 2
  %uitofp87 = uitofp i16 %load86 to float
  store float %uitofp87, ptr %getelementptr36, align 4
  %load88 = load i16, ptr %phi81, align 2
  %uitofp89 = uitofp i16 %load88 to float
  store float %uitofp89, ptr %getelementptr37, align 8
  %getelementptr90 = getelementptr inbounds %struct.barney, ptr %phi81, i64 0, i32 3
  %load91 = load i16, ptr %getelementptr90, align 2
  %uitofp92 = uitofp i16 %load91 to float
  store float %uitofp92, ptr %getelementptr38, align 4
  %icmp93 = icmp eq i32 %load82, 12
  %and = and i1 %icmp93, %icmp76
  br i1 %and, label %bb94, label %bb98

bb94:                                             ; preds = %bb77
  %getelementptr95 = getelementptr inbounds i16, ptr %call72, i64 %phi79
  %load96 = load i16, ptr %getelementptr95, align 2
  %uitofp97 = uitofp i16 %load96 to float
  store float %uitofp97, ptr %getelementptr39, align 8
  br label %bb98

bb98:                                             ; preds = %bb94, %bb77
  %sitofp99 = sitofp i64 %phi79 to double
  br label %bb100

bb100:                                            ; preds = %bb235, %bb98
  %phi101 = phi i64 [ 0, %bb98 ], [ %add236, %bb235 ]
  %phi102 = phi double [ %sitofp99, %bb98 ], [ %fmul193, %bb235 ]
  %phi103 = phi double [ %sitofp, %bb98 ], [ %fmul194, %bb235 ]
  %phi104 = phi i32 [ %phi78, %bb98 ], [ %phi188, %bb235 ]
  call void @llvm.lifetime.start.p0(i64 56, ptr nonnull %alloca6) #3
  call void null(ptr noundef %arg, ptr noundef nonnull %alloca6) #3
  %load105 = load float, ptr %getelementptr35, align 8
  %load106 = load float, ptr %getelementptr36, align 4
  %load107 = load float, ptr %getelementptr37, align 8
  br i1 %icmp40, label %bb187, label %bb108

bb108:                                            ; preds = %bb100
  %call109 = call fast double @llvm.rint.f64(double %phi102)
  %fptosi = fptosi double %call109 to i64
  %call110 = call fast double @llvm.rint.f64(double %phi103)
  %fptosi111 = fptosi double %call110 to i64
  br i1 %icmp43, label %bb187, label %bb112

bb112:                                            ; preds = %bb108
  br label %bb113

bb113:                                            ; preds = %bb175, %bb112
  %phi114 = phi i64 [ %add180, %bb175 ], [ %sub, %bb112 ]
  %phi115 = phi i64 [ %phi179, %bb175 ], [ 0, %bb112 ]
  %phi116 = phi double [ %phi178, %bb175 ], [ 0.000000e+00, %bb112 ]
  %phi117 = phi double [ %phi177, %bb175 ], [ 0.000000e+00, %bb112 ]
  %phi118 = phi i32 [ %phi176, %bb175 ], [ %phi104, %bb112 ]
  %mul = mul nsw i64 %phi114, %phi114
  %add119 = add i64 %phi114, %fptosi111
  %sitofp120 = sitofp i64 %phi114 to double
  %fadd = fadd fast double %phi103, %sitofp120
  br label %bb121

bb121:                                            ; preds = %bb168, %bb113
  %phi122 = phi i64 [ %sub42, %bb113 ], [ %add173, %bb168 ]
  %phi123 = phi i64 [ %phi115, %bb113 ], [ %phi172, %bb168 ]
  %phi124 = phi double [ %phi116, %bb113 ], [ %phi171, %bb168 ]
  %phi125 = phi double [ %phi117, %bb113 ], [ %phi170, %bb168 ]
  %phi126 = phi i32 [ %phi118, %bb113 ], [ %phi169, %bb168 ]
  %mul127 = mul nsw i64 %phi122, %phi122
  %add = add nuw nsw i64 %mul127, %mul
  %icmp128 = icmp sgt i64 %add, %mul45
  br i1 %icmp128, label %bb168, label %bb129

bb129:                                            ; preds = %bb121
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %alloca7) #3
  %add130 = add i64 %phi122, %fptosi
  %call131 = call i32 null(ptr noundef %call26, i64 noundef %add130, i64 noundef %add119, ptr noundef nonnull %alloca7, ptr noundef %arg4) #3
  %load132 = load float, ptr %getelementptr35, align 8
  %load133 = load i16, ptr %getelementptr46, align 2
  %uitofp134 = uitofp i16 %load133 to float
  %fsub = fsub fast float %load132, %uitofp134
  %fmul135 = fmul fast float %fsub, %fsub
  %load136 = load float, ptr %getelementptr36, align 4
  %load137 = load i16, ptr %getelementptr47, align 2
  %uitofp138 = uitofp i16 %load137 to float
  %fsub139 = fsub fast float %load136, %uitofp138
  %fmul140 = fmul fast float %fsub139, %fsub139
  %fadd141 = fadd fast float %fmul140, %fmul135
  %load142 = load float, ptr %getelementptr37, align 8
  %load143 = load i16, ptr %alloca7, align 2
  %uitofp144 = uitofp i16 %load143 to float
  %fsub145 = fsub fast float %load142, %uitofp144
  %fmul146 = fmul fast float %fsub145, %fsub145
  %fadd147 = fadd fast float %fadd141, %fmul146
  %fpext = fpext float %fadd147 to double
  %fcmp = fcmp fast ult double 0.000000e+00, %fpext
  br i1 %fcmp, label %bb164, label %bb148

bb148:                                            ; preds = %bb129
  %sitofp149 = sitofp i64 %phi122 to double
  %fadd150 = fadd fast double %phi124, %phi102
  %fadd151 = fadd fast double %fadd150, %sitofp149
  %fadd152 = fadd fast double %fadd, %phi125
  %load153 = load float, ptr %getelementptr49, align 8
  %fadd154 = fadd fast float %load153, %uitofp134
  store float %fadd154, ptr %getelementptr49, align 8
  %load155 = load float, ptr %getelementptr50, align 4
  %fadd156 = fadd fast float %load155, %uitofp138
  store float %fadd156, ptr %getelementptr50, align 4
  %load157 = load float, ptr %getelementptr51, align 8
  %fadd158 = fadd fast float %load157, %uitofp144
  store float %fadd158, ptr %getelementptr51, align 8
  %load159 = load i16, ptr %getelementptr52, align 2
  %uitofp160 = uitofp i16 %load159 to float
  %load161 = load float, ptr %getelementptr53, align 4
  %fadd162 = fadd fast float %load161, %uitofp160
  store float %fadd162, ptr %getelementptr53, align 4
  %add163 = add nsw i64 %phi123, 1
  br label %bb164

bb164:                                            ; preds = %bb148, %bb129
  %phi165 = phi double [ %fadd152, %bb148 ], [ %phi125, %bb129 ]
  %phi166 = phi double [ %fadd151, %bb148 ], [ %phi124, %bb129 ]
  %phi167 = phi i64 [ %add163, %bb148 ], [ %phi123, %bb129 ]
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %alloca7) #3
  br label %bb168

bb168:                                            ; preds = %bb164, %bb121
  %phi169 = phi i32 [ %call131, %bb164 ], [ %phi126, %bb121 ]
  %phi170 = phi double [ %phi165, %bb164 ], [ %phi125, %bb121 ]
  %phi171 = phi double [ %phi166, %bb164 ], [ %phi124, %bb121 ]
  %phi172 = phi i64 [ %phi167, %bb164 ], [ %phi123, %bb121 ]
  %add173 = add i64 %phi122, 1
  %icmp174 = icmp eq i64 %add173, %add58
  br i1 %icmp174, label %bb175, label %bb121

bb175:                                            ; preds = %bb168
  %phi176 = phi i32 [ %phi169, %bb168 ]
  %phi177 = phi double [ %phi170, %bb168 ]
  %phi178 = phi double [ %phi171, %bb168 ]
  %phi179 = phi i64 [ %phi172, %bb168 ]
  %add180 = add i64 %phi114, 1
  %icmp181 = icmp eq i64 %add180, %add59
  br i1 %icmp181, label %bb182, label %bb113

bb182:                                            ; preds = %bb175
  %phi183 = phi i32 [ %phi176, %bb175 ]
  %phi184 = phi double [ %phi177, %bb175 ]
  %phi185 = phi double [ %phi178, %bb175 ]
  %phi186 = phi i64 [ %phi179, %bb175 ]
  br label %bb187

bb187:                                            ; preds = %bb182, %bb108, %bb100
  %phi188 = phi i32 [ %phi104, %bb100 ], [ %phi104, %bb108 ], [ %phi183, %bb182 ]
  %phi189 = phi double [ 0.000000e+00, %bb100 ], [ 0.000000e+00, %bb108 ], [ %phi184, %bb182 ]
  %phi190 = phi double [ 0.000000e+00, %bb100 ], [ 0.000000e+00, %bb108 ], [ %phi185, %bb182 ]
  %phi191 = phi i64 [ 0, %bb100 ], [ 0, %bb108 ], [ %phi186, %bb182 ]
  %sitofp192 = sitofp i64 %phi191 to double
  %fdiv = fdiv fast double 1.000000e+00, %sitofp192
  %fmul193 = fmul fast double %fdiv, %phi190
  %fmul194 = fmul fast double %fdiv, %phi189
  %load195 = load float, ptr %getelementptr49, align 8
  %fpext196 = fpext float %load195 to double
  %fmul197 = fmul fast double %fdiv, %fpext196
  %fptrunc = fptrunc double %fmul197 to float
  store float %fptrunc, ptr %getelementptr35, align 8
  %load198 = load float, ptr %getelementptr50, align 4
  %fpext199 = fpext float %load198 to double
  %fmul200 = fmul fast double %fdiv, %fpext199
  %fptrunc201 = fptrunc double %fmul200 to float
  store float %fptrunc201, ptr %getelementptr36, align 4
  %load202 = load float, ptr %getelementptr51, align 8
  %fpext203 = fpext float %load202 to double
  %fmul204 = fmul fast double %fdiv, %fpext203
  %fptrunc205 = fptrunc double %fmul204 to float
  store float %fptrunc205, ptr %getelementptr37, align 8
  %load206 = load float, ptr %getelementptr53, align 4
  %fpext207 = fpext float %load206 to double
  %fmul208 = fmul fast double %fdiv, %fpext207
  %fptrunc209 = fptrunc double %fmul208 to float
  store float %fptrunc209, ptr %getelementptr38, align 4
  %fsub210 = fsub fast double %fmul193, %phi102
  %fmul211 = fmul fast double %fsub210, %fsub210
  %fsub212 = fsub fast double %fmul194, %phi103
  %fmul213 = fmul fast double %fsub212, %fsub212
  %fsub214 = fsub fast float %fptrunc, %load105
  %fpext215 = fpext float %fsub214 to double
  %fmul216 = fmul fast double %fpext215, %fpext215
  %fsub217 = fsub fast float %fptrunc201, %load106
  %fpext218 = fpext float %fsub217 to double
  %fmul219 = fmul fast double %fpext218, %fpext218
  %fsub220 = fsub fast float %fptrunc205, %load107
  %fpext221 = fpext float %fsub220 to double
  %fmul222 = fmul fast double %fpext221, %fpext221
  %fadd223 = fadd fast double %fmul219, %fmul216
  %fadd224 = fadd fast double %fadd223, %fmul222
  %fmul225 = fmul fast double %fadd224, 0x3EEFC05F809F40DF
  %fadd226 = fadd fast double %fmul211, %fmul213
  %fadd227 = fadd fast double %fadd226, %fmul225
  %fcmp228 = fcmp fast ugt double %fadd227, 3.000000e+00
  br i1 %fcmp228, label %bb235, label %bb229

bb229:                                            ; preds = %bb187
  %phi230 = phi i32 [ %phi188, %bb187 ]
  %phi231 = phi float [ %fptrunc, %bb187 ]
  %phi232 = phi float [ %fptrunc201, %bb187 ]
  %phi233 = phi float [ %fptrunc205, %bb187 ]
  %phi234 = phi float [ %fptrunc209, %bb187 ]
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %alloca6) #3
  br label %bb244

bb235:                                            ; preds = %bb187
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %alloca6) #3
  %add236 = add nuw nsw i64 %phi101, 1
  %icmp237 = icmp eq i64 %add236, 100
  br i1 %icmp237, label %bb238, label %bb100

bb238:                                            ; preds = %bb235
  %phi239 = phi i32 [ %phi188, %bb235 ]
  %phi240 = phi float [ %fptrunc, %bb235 ]
  %phi241 = phi float [ %fptrunc201, %bb235 ]
  %phi242 = phi float [ %fptrunc205, %bb235 ]
  %phi243 = phi float [ %fptrunc209, %bb235 ]
  br label %bb244

bb244:                                            ; preds = %bb238, %bb229
  %phi245 = phi i32 [ %phi239, %bb238 ], [ %phi230, %bb229 ]
  %phi246 = phi float [ %phi240, %bb238 ], [ %phi231, %bb229 ]
  %phi247 = phi float [ %phi241, %bb238 ], [ %phi232, %bb229 ]
  %phi248 = phi float [ %phi242, %bb238 ], [ %phi233, %bb229 ]
  %phi249 = phi float [ %phi243, %bb238 ], [ %phi234, %bb229 ]
  %fcmp250 = fcmp fast ugt float %phi246, 0.000000e+00
  %fcmp251 = fcmp fast ult float %phi246, 6.553500e+04
  %fadd252 = fadd fast float %phi246, 5.000000e-01
  %fptoui = fptoui float %fadd252 to i16
  %select253 = select i1 %fcmp251, i16 %fptoui, i16 -1
  %select254 = select i1 %fcmp250, i16 %select253, i16 0
  %getelementptr255 = getelementptr inbounds %struct.barney, ptr %phi80, i64 0, i32 2
  store i16 %select254, ptr %getelementptr255, align 2
  %fcmp256 = fcmp fast ugt float %phi247, 0.000000e+00
  %fcmp257 = fcmp fast ult float %phi247, 6.553500e+04
  %fadd258 = fadd fast float %phi247, 5.000000e-01
  %fptoui259 = fptoui float %fadd258 to i16
  %select260 = select i1 %fcmp257, i16 %fptoui259, i16 -1
  %select261 = select i1 %fcmp256, i16 %select260, i16 0
  %getelementptr262 = getelementptr inbounds %struct.barney, ptr %phi80, i64 0, i32 1
  store i16 %select261, ptr %getelementptr262, align 2
  %fcmp263 = fcmp fast ugt float %phi248, 0.000000e+00
  %fcmp264 = fcmp fast ult float %phi248, 6.553500e+04
  %fadd265 = fadd fast float %phi248, 5.000000e-01
  %fptoui266 = fptoui float %fadd265 to i16
  %select267 = select i1 %fcmp264, i16 %fptoui266, i16 -1
  %select268 = select i1 %fcmp263, i16 %select267, i16 0
  store i16 %select268, ptr %phi80, align 2
  %fcmp269 = fcmp fast ugt float %phi249, 0.000000e+00
  %fcmp270 = fcmp fast ult float %phi249, 6.553500e+04
  %fadd271 = fadd fast float %phi249, 5.000000e-01
  %fptoui272 = fptoui float %fadd271 to i16
  %select273 = select i1 %fcmp270, i16 %fptoui272, i16 -1
  %select274 = select i1 %fcmp269, i16 %select273, i16 0
  %getelementptr275 = getelementptr inbounds %struct.barney, ptr %phi80, i64 0, i32 3
  store i16 %select274, ptr %getelementptr275, align 2
  %getelementptr276 = getelementptr inbounds %struct.barney, ptr %phi81, i64 1
  %getelementptr277 = getelementptr inbounds %struct.barney, ptr %phi80, i64 1
  call void @llvm.lifetime.end.p0(i64 56, ptr nonnull %alloca5) #3
  %add278 = add nuw nsw i64 %phi79, 1
  %load279 = load i64, ptr %getelementptr33, align 8
  %icmp280 = icmp slt i64 %add278, %load279
  br i1 %icmp280, label %bb77, label %bb281

bb281:                                            ; preds = %bb244
  %phi282 = phi i32 [ %phi245, %bb244 ]
  br label %bb283

bb283:                                            ; preds = %bb281, %bb71
  %phi284 = phi i32 [ %phi, %bb71 ], [ %phi282, %bb281 ]
  %call285 = call i32 null(ptr noundef %call28, ptr noundef %arg4) #4
  %icmp286 = icmp eq i32 %call285, 0
  %select287 = select i1 %icmp286, i32 0, i32 %phi284
  %load288 = load ptr, ptr %getelementptr54, align 8
  %icmp289 = icmp eq ptr %load288, null
  br i1 %icmp289, label %bb299, label %bb290

bb290:                                            ; preds = %bb283
  %add291 = add nsw i64 %phi61, 1
  %load292 = load i64, ptr %getelementptr13, align 8
  call void @llvm.lifetime.start.p0(i64 4096, ptr nonnull %alloca) #3
  %call293 = call i64 (ptr, i64, ptr, ...) null(ptr noundef nonnull %alloca, i64 noundef 4096, ptr noundef nonnull null, ptr noundef nonnull null, ptr noundef nonnull %getelementptr56) #3
  %load294 = load ptr, ptr %getelementptr54, align 8
  %load295 = load ptr, ptr %getelementptr57, align 8
  %call296 = call i32 %load294(ptr noundef nonnull %alloca, i64 noundef %phi61, i64 noundef %load292, ptr noundef %load295) #3
  call void @llvm.lifetime.end.p0(i64 4096, ptr nonnull %alloca) #3
  %icmp297 = icmp eq i32 %call296, 0
  %select298 = select i1 %icmp297, i32 0, i32 %select287
  br label %bb299

bb299:                                            ; preds = %bb290, %bb283, %bb64, %bb60
  %phi300 = phi i64 [ %phi61, %bb60 ], [ %phi61, %bb64 ], [ %phi61, %bb283 ], [ %add291, %bb290 ]
  %phi301 = phi i32 [ 0, %bb60 ], [ 0, %bb64 ], [ %select287, %bb283 ], [ %select298, %bb290 ]
  %add302 = add nuw nsw i64 %phi62, 1
  %load303 = load i64, ptr %getelementptr29, align 8
  %icmp304 = icmp slt i64 %add302, %load303
  br i1 %icmp304, label %bb60, label %bb305

bb305:                                            ; preds = %bb299
  ret ptr null
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.rint.f64(double) #2

; uselistorder directives
uselistorder ptr @llvm.rint.f64, { 1, 0 }

attributes #0 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #1 = { nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "loopopt-pipeline"="full" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #2 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #3 = { nounwind }
attributes #4 = { hot nounwind }
