; Verify that IV predicate optimization attaches UB blob ddref if a self-blob UB becomes regular ddref.
; RUN: opt -hir-ssa-deconstruction -disable-output -hir-opt-var-predicate -print-after=hir-opt-var-predicate < %s 2>&1 | FileCheck %s

; HIR:
; BEGIN REGION { }
;      + DO i1 = 0, zext.i32.i64(%1) + -1, 1   <DO_LOOP>
;      |   %3 = (%2)[i1];
;      |   %4 = (%3)[0].0;
;      |
;      |      %cvert.051 = %cvert.051 + 1  +  %4;
;      |   + DO i2 = 0, %4, 1   <DO_LOOP>
;      |   |   if (i2 != 0)
;      |   |   {
;      |   |      (%medge.142)[0].0 = i2 + %cvert.051 + -1;
;      |   |      (%medge.142)[0].1 = i2 + %cvert.051;
;      |   |      (%medge.142)[0].4 = 162;
;      |   |      %medge.142 = &((%medge.142)[1]);
;      |   |   }
;      |   + END LOOP
;      + END LOOP
; END REGION

; CHECK: + DO i2 = 0, %4 + -1, 1

;Module Before HIR; ModuleID = 'main.c'
source_filename = "main.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.ModifierData = type { %struct.ParticleSystem* }
%struct.ParticleSystem = type { %struct.ParticleSettings*, i32, i32, %struct.ParticleCacheKey** }
%struct.ParticleSettings = type opaque
%struct.ParticleCacheKey = type { i32 }
%struct.MEdge = type { i32, i32, i8, i8, i16 }

; Function Attrs: norecurse nounwind uwtable
define i32 @aaaa(%struct.ModifierData* nocapture readonly %md) local_unnamed_addr #0 {
entry:
  %psys1 = getelementptr inbounds %struct.ModifierData, %struct.ModifierData* %md, i64 0, i32 0
  %0 = load %struct.ParticleSystem*, %struct.ParticleSystem** %psys1, align 8
  %totcached = getelementptr inbounds %struct.ParticleSystem, %struct.ParticleSystem* %0, i64 0, i32 1
  %1 = load i32, i32* %totcached, align 8
  %pathcache = getelementptr inbounds %struct.ParticleSystem, %struct.ParticleSystem* %0, i64 0, i32 3
  %2 = load %struct.ParticleCacheKey**, %struct.ParticleCacheKey*** %pathcache, align 8
  %cmp47 = icmp sgt i32 %1, 0
  br i1 %cmp47, label %for.body.preheader, label %for.end11

for.body.preheader:                               ; preds = %entry
  %wide.trip.count = zext i32 %1 to i64
  br label %for.body

for.body:                                         ; preds = %for.body.preheader, %for.inc9
  %indvars.iv = phi i64 [ %indvars.iv.next, %for.inc9 ], [ 0, %for.body.preheader ]
  %cvert.051 = phi i32 [ %cvert.1.lcssa, %for.inc9 ], [ 0, %for.body.preheader ]
  %medge.049 = phi %struct.MEdge* [ %medge.1.lcssa, %for.inc9 ], [ undef, %for.body.preheader ]
  %arrayidx = getelementptr inbounds %struct.ParticleCacheKey*, %struct.ParticleCacheKey** %2, i64 %indvars.iv
  %3 = load %struct.ParticleCacheKey*, %struct.ParticleCacheKey** %arrayidx, align 8
  %steps = getelementptr inbounds %struct.ParticleCacheKey, %struct.ParticleCacheKey* %3, i64 0, i32 0
  %4 = load i32, i32* %steps, align 4
  %cmp440 = icmp slt i32 %4, 0
  br i1 %cmp440, label %for.inc9, label %for.body5.preheader

for.body5.preheader:                              ; preds = %for.body
  %5 = add i32 %cvert.051, 1
  %6 = add i32 %5, %4
  br label %for.body5

for.body5:                                        ; preds = %for.inc, %for.body5.preheader
  %cvert.144 = phi i32 [ %inc7, %for.inc ], [ %cvert.051, %for.body5.preheader ]
  %medge.142 = phi %struct.MEdge* [ %medge.2, %for.inc ], [ %medge.049, %for.body5.preheader ]
  %k.041 = phi i32 [ %inc, %for.inc ], [ 0, %for.body5.preheader ]
  %tobool = icmp eq i32 %k.041, 0
  br i1 %tobool, label %for.inc, label %if.then

if.then:                                          ; preds = %for.body5
  %sub = add nsw i32 %cvert.144, -1
  %v1 = getelementptr inbounds %struct.MEdge, %struct.MEdge* %medge.142, i64 0, i32 0
  store i32 %sub, i32* %v1, align 4
  %v2 = getelementptr inbounds %struct.MEdge, %struct.MEdge* %medge.142, i64 0, i32 1
  store i32 %cvert.144, i32* %v2, align 4
  %flag = getelementptr inbounds %struct.MEdge, %struct.MEdge* %medge.142, i64 0, i32 4
  store i16 162, i16* %flag, align 2
  %incdec.ptr = getelementptr inbounds %struct.MEdge, %struct.MEdge* %medge.142, i64 1
  br label %for.inc

for.inc:                                          ; preds = %for.body5, %if.then
  %medge.2 = phi %struct.MEdge* [ %incdec.ptr, %if.then ], [ %medge.142, %for.body5 ]
  %inc = add nuw nsw i32 %k.041, 1
  %inc7 = add nsw i32 %cvert.144, 1
  %exitcond = icmp eq i32 %k.041, %4
  br i1 %exitcond, label %for.inc9.loopexit, label %for.body5

for.inc9.loopexit:                                ; preds = %for.inc
  %medge.2.lcssa = phi %struct.MEdge* [ %medge.2, %for.inc ]
  br label %for.inc9

for.inc9:                                         ; preds = %for.inc9.loopexit, %for.body
  %medge.1.lcssa = phi %struct.MEdge* [ %medge.049, %for.body ], [ %medge.2.lcssa, %for.inc9.loopexit ]
  %cvert.1.lcssa = phi i32 [ %cvert.051, %for.body ], [ %6, %for.inc9.loopexit ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond53 = icmp eq i64 %indvars.iv.next, %wide.trip.count
  br i1 %exitcond53, label %for.end11.loopexit, label %for.body

for.end11.loopexit:                               ; preds = %for.inc9
  br label %for.end11

for.end11:                                        ; preds = %for.end11.loopexit, %entry
  ret i32 1
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

