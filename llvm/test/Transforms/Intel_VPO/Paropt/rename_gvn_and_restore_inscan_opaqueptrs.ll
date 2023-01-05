; RUN: opt -passes='function(vpo-rename-operands,gvn,vpo-restore-operands)' -S < %s 2>&1 | FileCheck %s

;; Check that GEPs on the UDS memory are not hoised outside of the guard regions.
;; This is the situation we'd like to avoid:

; %x = getelementptr inbounds %struct.dummy, ptr %red.red, i64 0, i32 0
;
; "DIR.OMP.SIMD"..."QUAL.OMP.REDUCTION.UDR:INSCAN.TYPED"(ptr %red.red, ...)
; "DIR.VPO.GUARD.MEM.MOTION"..."QUAL.OMP.LIVEIN"(ptr %red.red)
; ...
; %x1.i = getelementptr inbounds %struct.dummy, ptr %red.red, i64 0, i32 0
; %7 = load float, ptr %x1.i, align 4
; ...
; "DIR.VPO.GUARD.MEM.MOTION"
; "DIR.OMP.END.SIMD"
;
; =>
;
; %x = getelementptr inbounds %struct.dummy, ptr %red.red, i64 0, i32 0
;
; "DIR.OMP.SIMD"..."QUAL.OMP.REDUCTION.UDR:INSCAN.TYPED"(ptr %red.red, ...)
; "DIR.VPO.GUARD.MEM.MOTION"..."QUAL.OMP.LIVEIN"(ptr %red.red)
; ...
; %7 = load float, ptr %x, align 4
; ...
; "DIR.VPO.GUARD.MEM.MOTION"
; "DIR.OMP.END.SIMD"

;; Test source for reference:

; typedef struct Dummy {
;   float x;
;   float y;
; } Dummy;
;
; void my_add(Dummy &lhs, const Dummy &rhs) { lhs.x += rhs.x; lhs.y = rhs.y; }
; void my_init(Dummy &t) { t.x = 0; t.y = 0; }
;
; #pragma omp declare reduction(my_scan_add: Dummy : my_add(omp_out, omp_in)) initializer(my_init(omp_priv))
;
; float uds(float* m_scan_vec, float* m_out_vec, float* m_in, int c_size)
; {
;   Dummy red;
;     red.x = m_scan_vec[0];
;     red.y = m_scan_vec[0];
;     #pragma omp simd reduction(inscan, my_scan_add: red)
;     for (int i = 0; i < c_size; ++i) {
;         my_add(red, {m_in[i], m_in[i]});
;         #pragma omp scan inclusive(red)
;         m_out_vec[i] = red.x;
;     }
;     return red.x;
; }

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.Dummy = type { float, float }
; CHECK: [[STRUCT_DUMMY:%.*]] = type { float, float }
%struct.fast_red_t = type <{ %struct.Dummy }>

@"@tid.addr" = external global i32
@"@bid.addr" = external global i32

define float @_Z3udsPfS_S_i(ptr %m_scan_vec, ptr %m_out_vec, ptr %m_in, i32 %c_size) {
; CHECK: call void @.omp_initializer.(ptr [[RED_RED:%.*]], {{.*}})
; CHECK: "DIR.OMP.SIMD"{{.*}}"QUAL.OMP.REDUCTION.UDR:INSCAN.TYPED"(ptr [[RED_RED]], {{.*}})
; CHECK: "DIR.VPO.GUARD.MEM.MOTION"(), "QUAL.OMP.LIVEIN"(ptr [[RED_RED]])
; CHECK: [[X1_I:%.*]] = getelementptr inbounds [[STRUCT_DUMMY]], ptr [[RED_RED]], i64 0, i32 0
; CHECK: [[Y2_I:%.*]] = getelementptr inbounds [[STRUCT_DUMMY]], ptr [[RED_RED]], i64 0, i32 1
; CHECK: "DIR.VPO.END.GUARD.MEM.MOTION"
; CHECK: "DIR.OMP.END.SIMD"
;
entry:
  %red.red = alloca %struct.Dummy, align 4
  %fast_red_struct = alloca %struct.fast_red_t, align 8
  %i.linear.iv = alloca i32, align 4
  %ref.tmp.priv = alloca %struct.Dummy, align 4
  %red = alloca %struct.Dummy, align 4
  %.omp.iv = alloca i32, align 4
  %.omp.ub = alloca i32, align 4
  %i = alloca i32, align 4
  %ref.tmp = alloca %struct.Dummy, align 4
  %0 = load float, ptr %m_scan_vec, align 4
  %x = getelementptr inbounds %struct.Dummy, ptr %red, i64 0, i32 0
  store float %0, ptr %x, align 4
  %y = getelementptr inbounds %struct.Dummy, ptr %red, i64 0, i32 1
  store float %0, ptr %y, align 4
  %cmp = icmp sgt i32 %c_size, 0
  br i1 %cmp, label %DIR.OMP.SIMD.226, label %omp.precond.end

DIR.OMP.SIMD.226:                                 ; preds = %entry
  %sub2 = add nsw i32 %c_size, -1
  store i32 %sub2, ptr %.omp.ub, align 4
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.SIMD.226
  br label %DIR.OMP.SIMD.1.split35

DIR.OMP.SIMD.1.split35:                           ; preds = %DIR.OMP.SIMD.1
  br label %DIR.OMP.SIMD.1.split35.split36

DIR.OMP.SIMD.1.split35.split36:                   ; preds = %DIR.OMP.SIMD.1.split35
  %red.fast_red = getelementptr inbounds %struct.fast_red_t, ptr %fast_red_struct, i32 0, i32 0
  br label %DIR.OMP.SIMD.1.split35.split

DIR.OMP.SIMD.1.split35.split:                     ; preds = %DIR.OMP.SIMD.1.split35.split36
  call void @.omp_initializer.(ptr %red.red, ptr %red)
  br label %DIR.OMP.SIMD.1.split

DIR.OMP.SIMD.1.split:                             ; preds = %DIR.OMP.SIMD.1.split35.split
  %1 = load i32, ptr %.omp.ub, align 4
  br label %DIR.OMP.SIMD.229

DIR.OMP.SIMD.229:                                 ; preds = %DIR.OMP.SIMD.1.split
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.229
  %cmp4.not33 = icmp sgt i32 0, %1
  br i1 %cmp4.not33, label %DIR.OMP.END.SIMD.7.loopexit, label %DIR.OMP.SCAN.427.lr.ph

DIR.OMP.SCAN.427.lr.ph:                           ; preds = %DIR.OMP.SIMD.2
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(),
 "QUAL.OMP.REDUCTION.UDR:INSCAN.TYPED"(ptr %red.red, %struct.Dummy zeroinitializer, i32 1, ptr null, ptr null, ptr @.omp_combiner., ptr @.omp_initializer., i64 1),
 "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0),
 "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0),
 "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1),
 "QUAL.OMP.PRIVATE:TYPED"(ptr %ref.tmp.priv, %struct.Dummy zeroinitializer, i32 1) ]

  br label %DIR.OMP.SCAN.427

DIR.OMP.SCAN.427:                                 ; preds = %DIR.OMP.SCAN.427.lr.ph, %DIR.VPO.END.GUARD.MEM.MOTION.4
  %.omp.iv.local.034 = phi i32 [ 0, %DIR.OMP.SCAN.427.lr.ph ], [ %add14, %DIR.VPO.END.GUARD.MEM.MOTION.4 ]
  br label %DIR.VPO.GUARD.MEM.MOTION.1

DIR.VPO.GUARD.MEM.MOTION.1:                       ; preds = %DIR.OMP.SCAN.427
  %guard.start = call token @llvm.directive.region.entry() [ "DIR.VPO.GUARD.MEM.MOTION"(),
 "QUAL.OMP.LIVEIN"(ptr %red.red) ]

  br label %DIR.VPO.GUARD.MEM.MOTION.2

DIR.VPO.GUARD.MEM.MOTION.2:                       ; preds = %DIR.VPO.GUARD.MEM.MOTION.1
  store i32 %.omp.iv.local.034, ptr %i.linear.iv, align 4
  %x6 = getelementptr inbounds %struct.Dummy, ptr %ref.tmp.priv, i64 0, i32 0
  %idxprom = sext i32 %.omp.iv.local.034 to i64
  %arrayidx7 = getelementptr inbounds float, ptr %m_in, i64 %idxprom
  %3 = load float, ptr %arrayidx7, align 4
  store float %3, ptr %x6, align 4
  %y8 = getelementptr inbounds %struct.Dummy, ptr %ref.tmp.priv, i64 0, i32 1
  %4 = load i32, ptr %i.linear.iv, align 4
  %idxprom9 = sext i32 %4 to i64
  %arrayidx10 = getelementptr inbounds float, ptr %m_in, i64 %idxprom9
  %5 = load float, ptr %arrayidx10, align 4
  store float %5, ptr %y8, align 4
  %x.i = getelementptr inbounds %struct.Dummy, ptr %ref.tmp.priv, i64 0, i32 0
  %6 = load float, ptr %x.i, align 4
  %x1.i = getelementptr inbounds %struct.Dummy, ptr %red.red, i64 0, i32 0
  %7 = load float, ptr %x1.i, align 4
  %add.i = fadd fast float %7, %6
  store float %add.i, ptr %x1.i, align 4
  %y.i = getelementptr inbounds %struct.Dummy, ptr %ref.tmp.priv, i64 0, i32 1
  %8 = load float, ptr %y.i, align 4
  %y2.i = getelementptr inbounds %struct.Dummy, ptr %red.red, i64 0, i32 1
  store float %8, ptr %y2.i, align 4
  br label %DIR.OMP.SCAN.3

DIR.OMP.SCAN.3:                                   ; preds = %DIR.VPO.GUARD.MEM.MOTION.2
  %9 = call token @llvm.directive.region.entry() [ "DIR.OMP.SCAN"(),
 "QUAL.OMP.INCLUSIVE"(ptr %red.red, i64 1) ]

  br label %DIR.OMP.SCAN.430

DIR.OMP.SCAN.430:                                 ; preds = %DIR.OMP.SCAN.3
  br label %DIR.OMP.SCAN.4

DIR.OMP.SCAN.4:                                   ; preds = %DIR.OMP.SCAN.430
  fence acq_rel
  br label %DIR.OMP.END.SCAN.6

DIR.OMP.END.SCAN.6:                               ; preds = %DIR.OMP.SCAN.4
  br label %DIR.OMP.END.SCAN.5

DIR.OMP.END.SCAN.5:                               ; preds = %DIR.OMP.END.SCAN.6
  call void @llvm.directive.region.exit(token %9) [ "DIR.OMP.END.SCAN"() ]
  br label %DIR.OMP.END.SCAN.631

DIR.OMP.END.SCAN.631:                             ; preds = %DIR.OMP.END.SCAN.5
  %x11 = getelementptr inbounds %struct.Dummy, ptr %red.red, i64 0, i32 0
  %10 = load float, ptr %x11, align 4
  %11 = load i32, ptr %i.linear.iv, align 4
  %idxprom12 = sext i32 %11 to i64
  %arrayidx13 = getelementptr inbounds float, ptr %m_out_vec, i64 %idxprom12
  store float %10, ptr %arrayidx13, align 4
  %add14 = add nsw i32 %.omp.iv.local.034, 1
  %12 = add i32 %1, 1
  br label %DIR.VPO.END.GUARD.MEM.MOTION.3

DIR.VPO.END.GUARD.MEM.MOTION.3:                   ; preds = %DIR.OMP.END.SCAN.631
  call void @llvm.directive.region.exit(token %guard.start) [ "DIR.VPO.END.GUARD.MEM.MOTION"() ]
  br label %DIR.VPO.END.GUARD.MEM.MOTION.4

DIR.VPO.END.GUARD.MEM.MOTION.4:                   ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.3
  %cmp4.not = icmp sgt i32 %12, %add14
  br i1 %cmp4.not, label %DIR.OMP.SCAN.427, label %omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge

omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge: ; preds = %DIR.VPO.END.GUARD.MEM.MOTION.4
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.1

DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge
  %13 = load %struct.Dummy, ptr %red.red, align 4
  store %struct.Dummy %13, ptr %red.fast_red, align 4
  br label %omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge.split37

omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge.split37: ; preds = %DIR.OMP.END.SIMD.1
  br label %omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge.split

omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge.split: ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge.split37
  br label %omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge.split.split

omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge.split.split: ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge.split
  call void @.omp_combiner.(ptr %red, ptr %red.fast_red)
  br label %DIR.OMP.END.SIMD.7.loopexit

DIR.OMP.END.SIMD.7.loopexit:                      ; preds = %omp.inner.for.cond.DIR.OMP.END.SIMD.7.loopexit_crit_edge.split.split, %DIR.OMP.SIMD.2
  br label %DIR.OMP.END.SIMD.7

DIR.OMP.END.SIMD.7:                               ; preds = %DIR.OMP.END.SIMD.7.loopexit
  br label %DIR.OMP.END.SIMD.732

DIR.OMP.END.SIMD.732:                             ; preds = %DIR.OMP.END.SIMD.7
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.732, %entry
  %14 = load float, ptr %x, align 4
  ret float %14
}

declare void @_Z6my_addR5DummyRKS_(ptr %lhs, ptr %rhs)

declare void @_Z7my_initR5Dummy(ptr %t)

declare token @llvm.directive.region.entry()

declare void @llvm.directive.region.exit(token)

declare void @.omp_combiner.(ptr %0, ptr %1)

declare void @.omp_initializer.(ptr %0, ptr %1)
