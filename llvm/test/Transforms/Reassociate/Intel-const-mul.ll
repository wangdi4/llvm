; RUN: opt < %s -passes='reassociate' -S | FileCheck %s

;; Check that we reassociate this sum which is later multiplied by a constant.
;; The intuition is that the constant multiplication makes no-wrap flags less
;; valuable and makes simplifications enabled by reassociation more likely to be
;; valuable.

define void @a(double* %b) {
; CHECK-LABEL: if.else:
; CHECK-NEXT:    [[Q:%.*]] = mul nsw i32 [[I_0:%.*]], 100
; CHECK-NEXT:    [[R:%.*]] = mul nsw i32 [[D_0:%.*]], 10000
; CHECK-NEXT:    [[AL:%.*]] = add i32 [[Q]], [[R]]
; CHECK-NEXT:    [[S:%.*]] = add i32 [[AL]], [[O_0:%.*]]
;
c:
  br label %for.as

for.as:
  %d.0 = phi i32 [ -2, %c ], [ %e, %for.f ]
  %at = icmp slt i32 %d.0, 2
  br i1 %at, label %for.g, label %for.h

for.g:
  %i.0 = phi i32 [ %aa, %for.j ], [ 0, %for.as ]
  %k = icmp ult i32 %i.0, 100
  br i1 %k, label %for.l, label %for.f

for.l:
  %o.0 = phi i32 [ %ab, %for.ab ], [ 0, %for.g ]
  %m = icmp ult i32 %o.0, 100
  br i1 %m, label %for.n, label %for.j

for.n:
  br label %if.else

if.else:
  %q = mul nsw i32 %i.0, 100
  %al = add nsw i32 %o.0, %q
  %r = mul nsw i32 %d.0, 10000
  %s = add nuw nsw i32 %al, %r
  %am = mul nuw i32 %s, 20
  %an = add nuw i32 %am, 9
  %ao = zext i32 %an to i64
  %ap = getelementptr double, double* %b, i64 %ao
  %bc = bitcast double* %ap to i32*
  %aq = or i32 3, 2
  store i32 %aq, i32* %bc, align 4
  br label %for.ab

for.ab:
  %ab = add i32 %o.0, 1
  br label %for.l

for.j:
  %aa = add i32 %i.0, 1
  br label %for.g

for.f:
  %e = add i32 %d.0, 1
  br label %for.as

for.h:
  ret void
}
