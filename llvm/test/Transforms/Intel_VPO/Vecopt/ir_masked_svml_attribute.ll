; Check that extra attributes (e.g. nofpclass) are not incorrectly attached to mask argument when vectorizing math library calls with SVML
; RUN: opt -passes=vplan-vec -vector-library=SVML -vplan-force-vf=16 -S < %s 2>&1 | FileCheck --check-prefixes=CHECK %s

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK-LABEL: @f
; CHECK: call fast svml_cc nofpclass(nan inf) <16 x double> @__svml_pow16_mask(<16 x double> undef, <16 x i1> {{.*}}, <16 x double> noundef nofpclass(nan inf) {{.*}}, <16 x double> noundef nofpclass(nan inf) {{.*}})
; CHECK: declare nofpclass(nan inf) <16 x double> @__svml_pow16_mask(<16 x double>, <16 x i1>, <16 x double>, <16 x double>)

define dso_local void @f(ptr nocapture noundef writeonly %table, double noundef %gamma_val) #0 {
DIR.OMP.SIMD.1:
  %i.linear.iv = alloca i32, align 4
  br label %DIR.OMP.SIMD.110

DIR.OMP.SIMD.110:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.NORMALIZED.IV:TYPED"(ptr null, i32 0), "QUAL.OMP.NORMALIZED.UB:TYPED"(ptr null, i32 0), "QUAL.OMP.LINEAR:IV.TYPED"(ptr %i.linear.iv, i32 0, i32 1, i32 1) ]
  br label %omp.inner.for.body

omp.inner.for.body:
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.110 ], [ %indvars.iv.next, %loop.exit ]
  %.omp.iv.local.08 = phi i32 [ 0, %DIR.OMP.SIMD.110 ], [ %add1, %loop.exit ]
  %cmp.i = icmp ult i64 %indvars.iv, 192
  br i1 %cmp.i, label %if.then.i, label %if.end.i

if.then.i:
  %1 = trunc i64 %indvars.iv to i32
  %conv.i = uitofp i32 %1 to double
  %call.i = call fast nofpclass(nan inf) double @pow(double noundef nofpclass(nan inf) %conv.i, double noundef nofpclass(nan inf) %gamma_val) #2
  %conv2.i = fptoui double %call.i to i8
  br label %loop.exit

if.end.i:
  %conv3.i = trunc i32 %.omp.iv.local.08 to i8
  br label %loop.exit

loop.exit:
  %retval.0.i = phi i8 [ %conv2.i, %if.then.i ], [ %conv3.i, %if.end.i ]
  %arrayidx = getelementptr inbounds i8, ptr %table, i64 %indvars.iv
  store i8 %retval.0.i, ptr %arrayidx, align 1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %add1 = add nuw nsw i32 %.omp.iv.local.08, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, 256
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2, label %omp.inner.for.body

DIR.OMP.END.SIMD.2:
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:
  ret void
}

declare dso_local nofpclass(nan inf) double @pow(double noundef nofpclass(nan inf), double noundef nofpclass(nan inf)) local_unnamed_addr #1
declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { nounwind uwtable }
attributes #1 = { nounwind }
attributes #2 = { nobuiltin nounwind }
