; RUN: opt -passes='hir-ssa-deconstruction,hir-temp-cleanup,hir-special-opt-predicate' -disable-output -debug-only=hir-special-opt-predicate -print-after=hir-special-opt-predicate -hir-details < %s 2>&1 | FileCheck %s

; Check that we recognize the candidate for special opt predicate that looks like
;  for (v = (-height/2 to height/2)
;   for (u = (-width/2 to width/2)
;     if ((v*v+u*u) <= (ssize_t) ((width/2)*(height/2)))
;
; Check the transformation logic that will convert the predicate to 2 different loops - one
; for computing the bounds, and the other that executes the loop with new bounds.
; Also check for ztt for the lower bound of the second inner loop

; HIR
;    + DO i4 = 0, 2 * %inst63, 1   <DO_LOOP>
;    |   %inst160 = i4 + -1 * %inst63  *  i4 + -1 * %inst63;
;    |   %inst162 = sitofp.i64.double(i4 + -1 * %inst63);
;    |   %inst163 = %inst142.out  +  %inst162;
;    |   %inst165 = i4 + -1 * %inst63 + %inst148 < 0;
;    |   + DO i5 = 0, 2 * %inst66, 1   <DO_LOOP>
;    |   |   %inst177 = i5 + -1 * %inst66  *  i5 + -1 * %inst66;
;    |   |   %inst178 = %inst177  +  %inst160;
;    |   |   if (%inst178 <= ((%arg1 /u 2) * (%arg2 /u 2)))
;    |   |   {
;              ....

; CHECK: Found Candidate!
; CHECK:      + DO i64 i4 = 0, 2 * %inst63, 1   <DO_LOOP>
; CHECK:      |   %inst160 = i4 + -1 * %inst63  *  i4 + -1 * %inst63;
; CHECK:      |   %inst162 = sitofp.i64.double(i4 + -1 * %inst63);
; CHECK:      |   %inst163 = %inst142.out  +  %inst162;
; CHECK:      |   %inst165 = i4 + -1 * %inst63 + %inst148 < 0;

; CHECK:      |   %optprd.lower = -1;
; CHECK:      |   %optprd.upper = -1;
; CHECK:      |
; CHECK:      |   + DO i64 i5 = 0, 2 * %inst66, 1   <DO_MULTI_EXIT_LOOP>
; CHECK:      |   |   %inst177 = i5 + -1 * %inst66  *  i5 + -1 * %inst66;
; CHECK:      |   |   %inst178 = %inst177  +  %inst160;
; CHECK:      |   |   if (%inst178 <= ((%arg1 /u 2) * (%arg2 /u 2)))
; CHECK:      |   |   {
; CHECK:      |   |      %optprd.upper = -1 * i5 + 2 * %inst66;
; CHECK:      |   |      %optprd.lower = i5;
; CHECK:      |   |      goto loopexit.248;
; CHECK:      |   |   }
; CHECK:      |   + END LOOP
; CHECK:      |
; CHECK:      |   loopexit.248:
; CHECK:      |
; CHECK:          + Ztt: if (%optprd.lower != -1)
; CHECK:      |   + DO i64 i5 = %optprd.lower, %optprd.upper, 1   <DO_LOOP>


target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.wibble = type { i32, i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, ptr, %struct.quux, %struct.quux, %struct.quux, double, %struct.barney, i32, ptr, i32, ptr, ptr, ptr, i64, double, double, %struct.blam, %struct.blam, %struct.blam, double, double, double, i32, i32, i32, i32, i32, i32, ptr, i64, i64, i64, i64, i64, i64, %struct.hoge, %struct.foo, ptr, ptr, ptr, ptr, ptr, ptr, [4096 x i8], [4096 x i8], [4096 x i8], i64, i64, %struct.bar, i32, i64, ptr, %struct.wobble, %struct.wobble, ptr, i64, i64, ptr, ptr, ptr, i32, i32, %struct.quux, ptr, %struct.blam, ptr, ptr, i32, i32, i64, i32, i64, i64, i32, i64 }
%struct.barney = type { %struct.baz, %struct.baz, %struct.baz, %struct.baz }
%struct.baz = type { double, double, double }
%struct.hoge = type { double, double, double }
%struct.foo = type { %struct.baz.0, %struct.baz.0, i32, i64 }
%struct.baz.0 = type { double, double, double }
%struct.bar = type { i32, i32, ptr, ptr, ptr, i32, ptr, i64 }
%struct.wobble = type { ptr, i64, ptr, i64 }
%struct.quux = type { i16, i16, i16, i16 }
%struct.blam = type { i64, i64, i64, i64 }
%struct.eggs = type { i64, i64, ptr, ptr, ptr, ptr, i64 }
%struct.spam = type { ptr, ptr }
%struct.snork = type { ptr, i32, i64, ptr, i32, i64 }
%struct.zot = type { i32, %struct.blam, i64, ptr, ptr, i32, ptr, i64 }
%struct.pluto = type { i32, i32, i64, i32, i32, i32, i64, i64, i64, i64, i32, %struct.baz.1, i64, ptr, ptr, ptr, i32, i32, [4096 x i8], [4096 x i8], %struct.bar.2, ptr, i64, ptr, i32, i32, i32, i64, ptr, ptr, i64, i64 }
%struct.baz.1 = type { i32, i32, i32, double, i64, float, float, float, float, float }
%struct.bar.2 = type { ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr }

declare fastcc i32 @bar(ptr)

declare fastcc i32 @ham(ptr)

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare double @llvm.rint.f64(double) #0

define fastcc ptr @ham.1(ptr %arg, i64 %arg1, i64 %arg2, double %arg3, ptr %arg4) #1 {
bb:
  %inst = alloca [4096 x i8], i32 0, align 16
  %inst5 = getelementptr %struct.wibble, ptr %arg, i64 0, i32 7
  %inst6 = load i64, ptr null, align 8
  %inst7 = getelementptr %struct.wibble, ptr %arg, i64 0, i32 8
  %inst8 = load i64, ptr null, align 8
  %inst9 = tail call fastcc ptr null(ptr null, i64 0, i64 0, ptr null)
  %inst10 = icmp eq ptr null, null
  br label %bb11

bb11:                                             ; preds = %bb
  %inst12 = getelementptr %struct.wibble, ptr %inst9, i64 0, i32 0
  %inst13 = getelementptr %struct.wibble, ptr %inst9, i64 0, i32 58
  %inst14 = tail call fastcc ptr null(ptr null, i32 0, ptr null)
  %inst15 = icmp eq ptr null, null
  br label %bb51

bb16:                                             ; No predecessors!
  %inst17 = getelementptr %struct.wibble, ptr %inst9, i64 0, i32 58, i32 4
  %inst18 = load ptr, ptr null, align 8
  %inst19 = icmp eq ptr null, null
  br label %bb50

bb20:                                             ; No predecessors!
  %inst21 = getelementptr %struct.eggs, ptr %inst18, i64 0, i32 2
  %inst22 = load ptr, ptr null, align 8
  %inst23 = getelementptr %struct.eggs, ptr %inst18, i64 0, i32 4
  %inst24 = icmp eq ptr null, null
  br i1 false, label %bb50, label %bb25

bb25:                                             ; preds = %bb20
  %inst26 = getelementptr %struct.spam, ptr %inst22, i64 0, i32 0
  %inst27 = load ptr, ptr null, align 8
  %inst28 = getelementptr %struct.spam, ptr %inst22, i64 0, i32 1
  %inst29 = load ptr, ptr null, align 8
  %inst30 = icmp eq ptr null, null
  br label %bb50

bb31:                                             ; No predecessors!
  br label %bb32

bb32:                                             ; preds = %bb43, %bb31
  %inst33 = phi ptr [ null, %bb43 ], [ null, %bb31 ]
  %inst34 = load i32, ptr null, align 8
  %inst35 = getelementptr %struct.bar, ptr %inst33, i64 0, i32 2
  %inst36 = load ptr, ptr null, align 8
  %inst37 = getelementptr %struct.bar, ptr %inst33, i64 0, i32 3
  %inst38 = load ptr, ptr null, align 8
  %inst39 = load ptr, ptr null, align 8
  %inst40 = getelementptr %struct.eggs, ptr %inst39, i64 0, i32 4
  %inst41 = load ptr, ptr null, align 8
  %inst42 = icmp eq ptr null, null
  br label %bb49

bb43:                                             ; No predecessors!
  %inst44 = getelementptr %struct.spam, ptr %inst41, i64 0, i32 0
  %inst45 = load ptr, ptr null, align 8
  %inst46 = getelementptr %struct.spam, ptr %inst41, i64 0, i32 1
  %inst47 = load ptr, ptr null, align 8
  %inst48 = icmp eq ptr null, null
  br i1 false, label %bb49, label %bb32

bb49:                                             ; preds = %bb43, %bb32
  br label %bb50

bb50:                                             ; preds = %bb49, %bb25, %bb20, %bb16
  br label %bb209

bb51:                                             ; preds = %bb11
  %inst52 = tail call fastcc ptr null(ptr null)
  %inst53 = tail call fastcc ptr null(ptr null)
  %inst54 = tail call fastcc ptr null(ptr null)
  %inst55 = getelementptr %struct.snork, ptr %inst54, i64 0, i32 0
  %inst56 = load ptr, ptr null, align 8
  %inst57 = tail call fastcc ptr null(ptr null, i32 0, ptr null)
  %inst58 = getelementptr %struct.wibble, ptr %inst9, i64 0, i32 8
  %inst59 = load i64, ptr null, align 8
  %inst60 = icmp sgt i64 0, 0
  br label %bb61

bb61:                                             ; preds = %bb51
  %inst62 = getelementptr %struct.wibble, ptr %inst9, i64 0, i32 7
  %inst63 = sdiv i64 %arg1, 0
  %inst64 = sub i64 0, %inst63
  %inst65 = icmp slt i64 0, 0
  %inst66 = sdiv i64 %arg1, 0
  %inst67 = sub i64 0, %inst66
  %inst68 = icmp slt i64 0, 0
  %inst69 = lshr i64 %arg1, 1
  %inst70 = lshr i64 %arg2, 1
  %inst71 = mul i64 %inst70, %inst69
  %inst72 = fmul double 0.000000e+00, 0.000000e+00
  %inst73 = getelementptr %struct.wibble, ptr %arg, i64 0, i32 47
  %inst74 = getelementptr [4096 x i8], ptr %inst, i64 0, i64 0
  %inst75 = getelementptr %struct.wibble, ptr %arg, i64 0, i32 53, i64 0
  %inst76 = getelementptr %struct.wibble, ptr %arg, i64 0, i32 48
  %inst77 = add i64 %inst66, 1
  %inst78 = add i64 %inst63, 1
  %inst79 = getelementptr %struct.snork, ptr %inst54, i64 0, i32 3
  %inst80 = getelementptr i32, ptr %inst53, i64 2
  %inst81 = getelementptr i8, ptr %inst53, i64 24
  br label %bb82

bb82:                                             ; preds = %bb206, %bb61
  %inst83 = phi i32 [ 0, %bb61 ], [ 0, %bb206 ]
  %inst84 = phi i64 [ 0, %bb61 ], [ 0, %bb206 ]
  %inst85 = phi i64 [ 0, %bb61 ], [ 0, %bb206 ]
  %inst86 = icmp eq i32 0, 0
  br i1 false, label %bb206, label %bb87

bb87:                                             ; preds = %bb82
  %inst88 = load i64, ptr null, align 8
  %inst89 = call fastcc ptr null(ptr null, i64 0, i64 0, i64 0, i64 0, ptr null)
  %inst90 = load i64, ptr null, align 8
  %inst91 = load ptr, ptr null, align 8
  %inst92 = load ptr, ptr null, align 8
  %inst93 = load ptr, ptr null, align 8
  %inst94 = call fastcc ptr null(ptr null, i64 0, i64 0, i64 0, i64 0, i32 0, ptr null, ptr null)
  %inst95 = icmp eq ptr null, null
  br label %bb96

bb96:                                             ; preds = %bb87
  %inst97 = getelementptr %struct.wibble, ptr %inst91, i64 0, i32 49
  %inst98 = load ptr, ptr null, align 8
  %inst99 = getelementptr %struct.zot, ptr %inst93, i64 0, i32 5
  %inst100 = load i32, ptr null, align 8
  %inst101 = icmp eq i32 0, 0
  br label %bb102

bb102:                                            ; preds = %bb96
  %inst103 = call fastcc i32 @bar(ptr null)
  %inst104 = icmp eq i32 0, 0
  br i1 false, label %bb206, label %bb105

bb105:                                            ; preds = %bb102
  %inst106 = getelementptr %struct.pluto, ptr %inst98, i64 0, i32 16
  %inst107 = load i32, ptr null, align 8
  %inst108 = icmp eq i32 0, 0
  br label %bb109

bb109:                                            ; preds = %bb105
  %inst110 = call fastcc i32 @ham(ptr null)
  %inst111 = icmp eq i32 0, 0
  %inst112 = icmp eq ptr null, null
  %inst113 = select i1 false, i1 false, i1 false
  br label %bb116

bb114:                                            ; No predecessors!
  %inst115 = icmp eq ptr null, null
  br label %bb206

bb116:                                            ; preds = %bb109
  %inst117 = load i64, ptr null, align 8
  %inst118 = icmp sgt i64 0, 0
  br label %bb120

bb119:                                            ; No predecessors!
  unreachable

bb120:                                            ; preds = %bb116
  %inst121 = sitofp i64 0 to double
  br label %bb122

bb122:                                            ; preds = %bb203, %bb120
  %inst123 = phi i32 [ 0, %bb120 ], [ 0, %bb203 ]
  %inst124 = phi i64 [ 0, %bb120 ], [ 0, %bb203 ]
  %inst125 = phi ptr [ null, %bb120 ], [ null, %bb203 ]
  %inst126 = phi ptr [ null, %bb120 ], [ null, %bb203 ]
  %inst127 = getelementptr %struct.quux, ptr %inst126, i64 0, i32 2
  %inst128 = load i16, ptr null, align 2
  %inst129 = uitofp i16 0 to float
  %inst130 = getelementptr %struct.quux, ptr %inst126, i64 0, i32 1
  %inst131 = load i16, ptr null, align 2
  %inst132 = uitofp i16 0 to float
  %inst133 = load i16, ptr null, align 2
  %inst134 = uitofp i16 0 to float
  %inst135 = sitofp i64 0 to double
  br label %bb136

bb136:                                            ; preds = %bb200, %bb122
  %inst137 = phi float [ 0.000000e+00, %bb122 ], [ 0.000000e+00, %bb200 ]
  %inst138 = phi float [ 0.000000e+00, %bb122 ], [ 0.000000e+00, %bb200 ]
  %inst139 = phi float [ 0.000000e+00, %bb122 ], [ 0.000000e+00, %bb200 ]
  %inst140 = phi i64 [ 0, %bb122 ], [ 0, %bb200 ]
  %inst141 = phi double [ 0.000000e+00, %bb122 ], [ 0.000000e+00, %bb200 ]
  %inst142 = phi double [ 0.000000e+00, %bb122 ], [ %inst201, %bb200 ]
  %inst143 = phi i32 [ 0, %bb122 ], [ 0, %bb200 ]
  br i1 false, label %bb200, label %bb144

bb144:                                            ; preds = %bb136
  %inst145 = call double @llvm.rint.f64(double 0.000000e+00)
  %inst146 = fptosi double 0.000000e+00 to i64
  %inst147 = call double @llvm.rint.f64(double 0.000000e+00)
  %inst148 = fptosi double 0.000000e+00 to i64
  br label %bb149

bb149:                                            ; preds = %bb144
  br label %bb150

bb150:                                            ; preds = %bb196, %bb149
  %inst151 = phi float [ 0.000000e+00, %bb196 ], [ 0.000000e+00, %bb149 ]
  %inst152 = phi float [ 0.000000e+00, %bb196 ], [ 0.000000e+00, %bb149 ]
  %inst153 = phi float [ 0.000000e+00, %bb196 ], [ 0.000000e+00, %bb149 ]
  %inst154 = phi float [ 0.000000e+00, %bb196 ], [ 0.000000e+00, %bb149 ]
  %inst155 = phi i64 [ %inst197, %bb196 ], [ %inst64, %bb149 ]
  %inst156 = phi i64 [ 0, %bb196 ], [ 0, %bb149 ]
  %inst157 = phi double [ 0.000000e+00, %bb196 ], [ 0.000000e+00, %bb149 ]
  %inst158 = phi double [ %inst192, %bb196 ], [ 0.000000e+00, %bb149 ]
  %inst159 = phi i32 [ 0, %bb196 ], [ 0, %bb149 ]
  %inst160 = mul i64 %inst155, %inst155
  %inst161 = add i64 %inst155, %inst148
  %inst162 = sitofp i64 %inst155 to double
  %inst163 = fadd double %inst142, %inst162
  %inst164 = icmp sgt i64 0, 0
  %inst165 = icmp slt i64 %inst161, 0
  %inst166 = add i64 0, 0
  br label %bb167

bb167:                                            ; preds = %bb187, %bb150
  %inst168 = phi float [ 0.000000e+00, %bb150 ], [ %inst188, %bb187 ]
  %inst169 = phi float [ 0.000000e+00, %bb150 ], [ %inst189, %bb187 ]
  %inst170 = phi float [ 0.000000e+00, %bb150 ], [ %inst190, %bb187 ]
  %inst171 = phi float [ 0.000000e+00, %bb150 ], [ %inst191, %bb187 ]
  %inst172 = phi i64 [ %inst67, %bb150 ], [ %inst194, %bb187 ]
  %inst173 = phi i64 [ 0, %bb150 ], [ 0, %bb187 ]
  %inst174 = phi double [ 0.000000e+00, %bb150 ], [ %inst193, %bb187 ]
  %inst175 = phi double [ %inst158, %bb150 ], [ %inst175, %bb187 ]
  %inst176 = phi i32 [ 0, %bb150 ], [ 0, %bb187 ]
  %inst177 = mul i64 %inst172, %inst172
  %inst178 = add i64 %inst177, %inst160
  %inst179 = icmp sgt i64 %inst178, %inst71
  br i1 %inst179, label %bb187, label %bb180

bb180:                                            ; preds = %bb167
  %inst181 = or i1 false, %inst165
  br i1 %inst181, label %bb185, label %bb182

bb182:                                            ; preds = %bb180
  %inst183 = call fastcc i32 @bar(ptr %arg)
  %inst184 = call fastcc i32 @ham(ptr %arg)
  br label %bb185

bb185:                                            ; preds = %bb182, %bb180
  %inst186 = fadd double %inst163, 0.000000e+00
  br label %bb187

bb187:                                            ; preds = %bb185, %bb167
  %inst188 = phi float [ %inst168, %bb167 ], [ 0.000000e+00, %bb185 ]
  %inst189 = phi float [ %inst169, %bb167 ], [ 0.000000e+00, %bb185 ]
  %inst190 = phi float [ %inst170, %bb167 ], [ 0.000000e+00, %bb185 ]
  %inst191 = phi float [ %inst171, %bb167 ], [ 0.000000e+00, %bb185 ]
  %inst192 = phi double [ %inst175, %bb167 ], [ %inst186, %bb185 ]
  %inst193 = phi double [ %inst174, %bb167 ], [ 0.000000e+00, %bb185 ]
  %inst194 = add i64 %inst172, 1
  %inst195 = icmp eq i64 %inst194, %inst77
  br i1 %inst195, label %bb196, label %bb167

bb196:                                            ; preds = %bb187
  %inst197 = add i64 %inst155, 1
  %inst198 = icmp eq i64 %inst197, %inst78
  br i1 %inst198, label %bb199, label %bb150

bb199:                                            ; preds = %bb196
  br label %bb200

bb200:                                            ; preds = %bb199, %bb136
  %inst201 = phi double [ 0.000000e+00, %bb136 ], [ %inst192, %bb199 ]
  %inst202 = icmp eq i64 0, 0
  br i1 %inst202, label %bb203, label %bb136

bb203:                                            ; preds = %bb200
  %inst204 = icmp slt i64 0, 0
  br i1 %inst204, label %bb122, label %bb205

bb205:                                            ; preds = %bb203
  br label %bb206

bb206:                                            ; preds = %bb205, %bb114, %bb102, %bb82
  %inst207 = icmp slt i64 0, 0
  br i1 %inst207, label %bb82, label %bb208

bb208:                                            ; preds = %bb206
  br label %bb209

bb209:                                            ; preds = %bb208, %bb50
  ret ptr null
}

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { "prefer-function-level-region" }
