; RUN: opt -S < %s -vplan-force-vf=2 -disable-output -vplan-print-after-plain-cfg -vplan-vec | FileCheck --allow-empty %s
; RUN: opt -S < %s -vplan-force-vf=2 -disable-output -vplan-print-after-plain-cfg -hir-framework -hir-vplan-vec | FileCheck --allow-empty %s

; Ensure VPlan wasn't built at all  due to unsupported "#pragma omp simd ordered"
; TODO: Implement support for it.
; CHECK-NOT: i32

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define void @var_tripcount(i32* %ip, i32 %n, i32* %x) local_unnamed_addr {
entry:
  %entry.region = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:
  %cmp5 = icmp sgt i32 %n, 0
  br i1 %cmp5, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:
  %wide.trip.count = zext i32 %n to i64
  br label %for.body

for.body:
  %indvars.iv = phi i64 [ %indvars.iv.next, %latch ], [ 0, %for.body.preheader ]
  %arrayidx = getelementptr inbounds i32, i32* %ip, i64 %indvars.iv
  br label %ordered.entry

ordered.entry:
  %tok.ordered = call token @llvm.directive.region.entry() [ "DIR.OMP.ORDERED"(), "QUAL.OMP.ORDERED.SIMD"() ]
  br label %ordered

ordered:
  %val = load i32, i32* %x
  store i32 %val, i32* %arrayidx, align 4
  br label %ordered.exit

ordered.exit:
  call void @llvm.directive.region.exit(token %tok.ordered) [ "DIR.OMP.END.ORDERED"() ]
  br label %latch

latch:
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond, label %for.end, label %for.body

for.end:
  br label %for.cond.cleanup

for.cond.cleanup:
  call void @llvm.directive.region.exit(token %entry.region) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:
  ret void
}

declare token @llvm.directive.region.entry() nounwind
declare void @llvm.directive.region.exit(token) nounwind
