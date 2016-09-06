; RUN: opt < %s -hir-ssa-deconstruction | opt -analyze -hir-parser | FileCheck %s

; Check parsing output for instructions assigning to plane.2.lcssa.i. The rval should be successfully parsed as an 'address of' GEP DDRef.

; CHECK: %plane.2.lcssa.i = &((%add.ptr)[%step * i1])
; CHECK: %plane.2.lcssa.i = &((%add.ptr)[%step * i1 + %step1])


; ModuleID = 'bugpoint-reduced-simplified.bc'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @padramp_freq_4u(i32 %np3, float* %add.ptr, i64 %step, i64 %step1) #0 {
entry:
  br i1 undef, label %if.then, label %for.cond176.preheader

for.cond176.preheader:                            ; preds = %entry
  br i1 undef, label %for.body179.lr.ph, label %if.end

for.body179.lr.ph:                                ; preds = %for.cond176.preheader
  br label %for.body179

if.then:                                          ; preds = %entry
  br i1 undef, label %for.body.lr.ph, label %for.cond40.preheader

for.body.lr.ph:                                   ; preds = %if.then
  br label %for.body

for.cond40.preheader.loopexit:                    ; preds = %padramp_freq_3u_work.exit
  br label %for.cond40.preheader

for.cond40.preheader:                             ; preds = %for.cond40.preheader.loopexit, %if.then
  br i1 undef, label %for.body43.lr.ph, label %for.cond105.preheader

for.body43.lr.ph:                                 ; preds = %for.cond40.preheader
  br label %for.body43

for.body:                                         ; preds = %padramp_freq_3u_work.exit, %for.body.lr.ph
  %vol_out.0374 = phi float* [ %add.ptr, %for.body.lr.ph ], [ %add.ptr31, %padramp_freq_3u_work.exit ]
  %i4.0373 = phi i32 [ undef, %for.body.lr.ph ], [ %dec, %padramp_freq_3u_work.exit ]
  br i1 undef, label %if.then.i, label %for.cond102.preheader.i

for.cond102.preheader.i:                          ; preds = %for.body
  br i1 undef, label %for.body105.i.preheader, label %padramp_freq_3u_work.exit

for.body105.i.preheader:                          ; preds = %for.cond102.preheader.i
  br label %for.body105.i

if.then.i:                                        ; preds = %for.body
  br i1 undef, label %for.body.i.preheader, label %for.end.i

for.body.i.preheader:                             ; preds = %if.then.i
  br label %for.body.i

for.body.i:                                       ; preds = %for.body.i, %for.body.i.preheader
  %i3.0236.i = phi i32 [ %dec.i, %for.body.i ], [ undef, %for.body.i.preheader ]
  %dec.i = add nsw i32 %i3.0236.i, -1
  %cmp20.i = icmp sgt i32 %dec.i, undef
  br i1 %cmp20.i, label %for.body.i, label %for.end.loopexit.i

for.end.loopexit.i:                               ; preds = %for.body.i
  br label %for.end.i

for.end.i:                                        ; preds = %for.end.loopexit.i, %if.then.i
  br i1 undef, label %for.body42.lr.ph.i, label %for.cond61.preheader.i

for.body42.lr.ph.i:                               ; preds = %for.end.i
  br label %for.body42.i

for.cond61.preheader.i.loopexit:                  ; preds = %scale_2s_reverse_order.exit.i
  br label %for.cond61.preheader.i

for.cond61.preheader.i:                           ; preds = %for.cond61.preheader.i.loopexit, %for.end.i
  br i1 undef, label %for.end74.i, label %for.body64.i.preheader

for.body64.i.preheader:                           ; preds = %for.cond61.preheader.i
  br label %for.body64.i

for.body42.i:                                     ; preds = %scale_2s_reverse_order.exit.i, %for.body42.lr.ph.i
  %i3.1234.in.i = phi i32 [ %np3, %for.body42.lr.ph.i ], [ %i3.1234.i, %scale_2s_reverse_order.exit.i ]
  %i3.1234.i = add nsw i32 %i3.1234.in.i, -1
  br i1 undef, label %for.cond10.preheader.i.i.preheader, label %for.cond25.preheader.i.i

for.cond10.preheader.i.i.preheader:               ; preds = %for.body42.i
  br label %for.cond10.preheader.i.i

for.cond10.preheader.i.i:                         ; preds = %for.end.i.i, %for.cond10.preheader.i.i.preheader
  %i2.066.i.i = phi i32 [ undef, %for.end.i.i ], [ undef, %for.cond10.preheader.i.i.preheader ]
  br i1 undef, label %for.body13.i.i.preheader, label %for.end.i.i

for.body13.i.i.preheader:                         ; preds = %for.cond10.preheader.i.i
  br label %for.body13.i.i

for.cond25.preheader.loopexit.i.i:                ; preds = %for.end.i.i
  br label %for.cond25.preheader.i.i

for.cond25.preheader.i.i:                         ; preds = %for.cond25.preheader.loopexit.i.i, %for.body42.i
  br i1 undef, label %for.body28.i.i.preheader, label %scale_2s_reverse_order.exit.i

for.body28.i.i.preheader:                         ; preds = %for.cond25.preheader.i.i
  br label %for.body28.i.i

for.body13.i.i:                                   ; preds = %for.body13.i.i, %for.body13.i.i.preheader
  %indvars.iv69.in.i.i = phi i64 [ undef, %for.body13.i.i ], [ undef, %for.body13.i.i.preheader ]
  %cmp11.i.i = icmp sgt i64 %indvars.iv69.in.i.i, 1
  br i1 %cmp11.i.i, label %for.body13.i.i, label %for.end.i.i.loopexit

for.end.i.i.loopexit:                             ; preds = %for.body13.i.i
  br label %for.end.i.i

for.end.i.i:                                      ; preds = %for.end.i.i.loopexit, %for.cond10.preheader.i.i
  %cmp.i.i = icmp sgt i32 %i2.066.i.i, 1
  br i1 %cmp.i.i, label %for.cond10.preheader.i.i, label %for.cond25.preheader.loopexit.i.i

for.body28.i.i:                                   ; preds = %for.body28.i.i, %for.body28.i.i.preheader
  %indvars.iv.in.i.i = phi i64 [ undef, %for.body28.i.i ], [ undef, %for.body28.i.i.preheader ]
  %cmp26.i.i = icmp sgt i64 %indvars.iv.in.i.i, 1
  br i1 %cmp26.i.i, label %for.body28.i.i, label %scale_2s_reverse_order.exit.i.loopexit

scale_2s_reverse_order.exit.i.loopexit:           ; preds = %for.body28.i.i
  br label %scale_2s_reverse_order.exit.i

scale_2s_reverse_order.exit.i:                    ; preds = %scale_2s_reverse_order.exit.i.loopexit, %for.cond25.preheader.i.i
  %cmp40.i = icmp sgt i32 %i3.1234.i, undef
  br i1 %cmp40.i, label %for.body42.i, label %for.cond61.preheader.i.loopexit

for.body64.i:                                     ; preds = %for.body64.i, %for.body64.i.preheader
  %plane.2230.i = phi float* [ %add.ptr68.i, %for.body64.i ], [ %vol_out.0374, %for.body64.i.preheader ]
  %i3.2228.i = phi i32 [ undef, %for.body64.i ], [ 0, %for.body64.i.preheader ]
  %add.ptr68.i = getelementptr inbounds float, float* %plane.2230.i, i64 %step1
  %cmp62.i = icmp slt i32 %i3.2228.i, undef
  br i1 %cmp62.i, label %for.body64.i, label %for.end74.i.loopexit

for.end74.i.loopexit:                             ; preds = %for.body64.i
  br label %for.end74.i

for.end74.i:                                      ; preds = %for.end74.i.loopexit, %for.cond61.preheader.i
  %plane.2.lcssa.i = phi float* [ %vol_out.0374, %for.cond61.preheader.i ], [ %add.ptr68.i, %for.end74.i.loopexit ]
  br i1 undef, label %for.body85.i.preheader, label %padramp_freq_3u_work.exit

for.body85.i.preheader:                           ; preds = %for.end74.i
  br label %for.body85.i

for.body85.i:                                     ; preds = %scale_2s.exit.i, %for.body85.i.preheader
  %plane.3226.i = phi float* [ %add.ptr97.i, %scale_2s.exit.i ], [ %plane.2.lcssa.i, %for.body85.i.preheader ]
  %i3.3225.i = phi i32 [ %inc99.i, %scale_2s.exit.i ], [ 0, %for.body85.i.preheader ]
  br i1 undef, label %for.cond1.preheader.i.i.preheader, label %scale_2s.exit.i

for.cond1.preheader.i.i.preheader:                ; preds = %for.body85.i
  br label %for.cond1.preheader.i.i

for.cond1.preheader.i.i:                          ; preds = %for.end.i222.i, %for.cond1.preheader.i.i.preheader
  %i2.025.i.i = phi i32 [ %inc9.i.i, %for.end.i222.i ], [ 0, %for.cond1.preheader.i.i.preheader ]
  %des.addr.024.i.i = phi float* [ %add.ptr.i221.i, %for.end.i222.i ], [ %plane.3226.i, %for.cond1.preheader.i.i.preheader ]
  br i1 undef, label %for.body3.i.i.preheader, label %for.end.i222.i

for.body3.i.i.preheader:                          ; preds = %for.cond1.preheader.i.i
  br label %for.body3.i.i

for.body3.i.i:                                    ; preds = %for.body3.i.i, %for.body3.i.i.preheader
  %indvars.iv.i218.i = phi i64 [ %indvars.iv.next.i.i, %for.body3.i.i ], [ 0, %for.body3.i.i.preheader ]
  %arrayidx5.i.i = getelementptr inbounds float, float* %des.addr.024.i.i, i64 %indvars.iv.i218.i
  store float undef, float* %arrayidx5.i.i, align 4
  %indvars.iv.next.i.i = add nuw nsw i64 %indvars.iv.i218.i, 1
  %lftr.wideiv396 = trunc i64 %indvars.iv.next.i.i to i32
  %exitcond397 = icmp eq i32 %lftr.wideiv396, undef
  br i1 %exitcond397, label %for.end.i222.i.loopexit, label %for.body3.i.i

for.end.i222.i.loopexit:                          ; preds = %for.body3.i.i
  br label %for.end.i222.i

for.end.i222.i:                                   ; preds = %for.end.i222.i.loopexit, %for.cond1.preheader.i.i
  %add.ptr.i221.i = getelementptr inbounds float, float* %des.addr.024.i.i, i64 undef
  %inc9.i.i = add nuw nsw i32 %i2.025.i.i, 1
  %exitcond26.i.i = icmp eq i32 %inc9.i.i, undef
  br i1 %exitcond26.i.i, label %scale_2s.exit.i.loopexit, label %for.cond1.preheader.i.i

scale_2s.exit.i.loopexit:                         ; preds = %for.end.i222.i
  br label %scale_2s.exit.i

scale_2s.exit.i:                                  ; preds = %scale_2s.exit.i.loopexit, %for.body85.i
  %add.ptr97.i = getelementptr inbounds float, float* %plane.3226.i, i64 undef
  %inc99.i = add nuw nsw i32 %i3.3225.i, 1
  %exitcond245.i = icmp eq i32 %inc99.i, undef
  br i1 %exitcond245.i, label %padramp_freq_3u_work.exit.loopexit, label %for.body85.i

for.body105.i:                                    ; preds = %for.body105.i, %for.body105.i.preheader
  %i3.4241.i = phi i32 [ undef, %for.body105.i ], [ undef, %for.body105.i.preheader ]
  %cmp103.i = icmp sgt i32 %i3.4241.i, 1
  br i1 %cmp103.i, label %for.body105.i, label %padramp_freq_3u_work.exit.loopexit400

padramp_freq_3u_work.exit.loopexit:               ; preds = %scale_2s.exit.i
  br label %padramp_freq_3u_work.exit

padramp_freq_3u_work.exit.loopexit400:            ; preds = %for.body105.i
  br label %padramp_freq_3u_work.exit

padramp_freq_3u_work.exit:                        ; preds = %padramp_freq_3u_work.exit.loopexit400, %padramp_freq_3u_work.exit.loopexit, %for.end74.i, %for.cond102.preheader.i
  %add.ptr31 = getelementptr inbounds float, float* %vol_out.0374, i64 %step
  %dec = add nsw i32 %i4.0373, -1
  %cmp24 = icmp sgt i32 %dec, undef
  br i1 %cmp24, label %for.body, label %for.cond40.preheader.loopexit

for.cond40.loopexit:                              ; preds = %scale_2s_reverse_order.exit
  br i1 undef, label %for.body43, label %for.cond105.preheader.loopexit

for.cond105.preheader.loopexit:                   ; preds = %for.cond40.loopexit
  br label %for.cond105.preheader

for.cond105.preheader:                            ; preds = %for.cond105.preheader.loopexit, %for.cond40.preheader
  br i1 undef, label %for.end116, label %for.body108.preheader

for.body108.preheader:                            ; preds = %for.cond105.preheader
  br label %for.body108

for.body43:                                       ; preds = %for.cond40.loopexit, %for.body43.lr.ph
  br label %for.cond79

for.cond79:                                       ; preds = %for.body82, %for.body43
  br i1 undef, label %for.cond10.preheader.lr.ph.i, label %for.cond25.preheader.i

for.cond10.preheader.lr.ph.i:                     ; preds = %for.cond79
  br label %for.cond10.preheader.i

for.cond10.preheader.i:                           ; preds = %for.end.i341, %for.cond10.preheader.lr.ph.i
  br i1 undef, label %for.body13.i.preheader, label %for.end.i341

for.body13.i.preheader:                           ; preds = %for.cond10.preheader.i
  br label %for.body13.i

for.cond25.preheader.loopexit.i:                  ; preds = %for.end.i341
  br label %for.cond25.preheader.i

for.cond25.preheader.i:                           ; preds = %for.cond25.preheader.loopexit.i, %for.cond79
  br i1 undef, label %for.body28.i.preheader, label %scale_2s_reverse_order.exit

for.body28.i.preheader:                           ; preds = %for.cond25.preheader.i
  br label %for.body28.i

for.body13.i:                                     ; preds = %for.body13.i, %for.body13.i.preheader
  br i1 undef, label %for.body13.i, label %for.end.i341.loopexit

for.end.i341.loopexit:                            ; preds = %for.body13.i
  br label %for.end.i341

for.end.i341:                                     ; preds = %for.end.i341.loopexit, %for.cond10.preheader.i
  br i1 undef, label %for.cond10.preheader.i, label %for.cond25.preheader.loopexit.i

for.body28.i:                                     ; preds = %for.body28.i, %for.body28.i.preheader
  br i1 undef, label %for.body28.i, label %scale_2s_reverse_order.exit.loopexit

scale_2s_reverse_order.exit.loopexit:             ; preds = %for.body28.i
  br label %scale_2s_reverse_order.exit

scale_2s_reverse_order.exit:                      ; preds = %scale_2s_reverse_order.exit.loopexit, %for.cond25.preheader.i
  br i1 undef, label %for.body82, label %for.cond40.loopexit

for.body82:                                       ; preds = %scale_2s_reverse_order.exit
  br label %for.cond79

for.body108:                                      ; preds = %for.body108, %for.body108.preheader
  br i1 undef, label %for.body108, label %for.end116.loopexit

for.end116.loopexit:                              ; preds = %for.body108
  br label %for.end116

for.end116:                                       ; preds = %for.end116.loopexit, %for.cond105.preheader
  br i1 undef, label %for.body129.lr.ph, label %if.end

for.body129.lr.ph:                                ; preds = %for.end116
  br label %for.body129

for.body129:                                      ; preds = %for.end155, %for.body129.lr.ph
  br i1 undef, label %for.body141.preheader, label %for.end155

for.body141.preheader:                            ; preds = %for.body129
  br label %for.body141

for.body141:                                      ; preds = %scale_2s.exit, %for.body141.preheader
  br i1 undef, label %for.cond1.preheader.i.preheader, label %scale_2s.exit

for.cond1.preheader.i.preheader:                  ; preds = %for.body141
  br label %for.cond1.preheader.i

for.cond1.preheader.i:                            ; preds = %for.end.i348, %for.cond1.preheader.i.preheader
  br i1 undef, label %for.body3.i.preheader, label %for.end.i348

for.body3.i.preheader:                            ; preds = %for.cond1.preheader.i
  br label %for.body3.i

for.body3.i:                                      ; preds = %for.body3.i, %for.body3.i.preheader
  br i1 undef, label %for.end.i348.loopexit, label %for.body3.i

for.end.i348.loopexit:                            ; preds = %for.body3.i
  br label %for.end.i348

for.end.i348:                                     ; preds = %for.end.i348.loopexit, %for.cond1.preheader.i
  br i1 undef, label %scale_2s.exit.loopexit, label %for.cond1.preheader.i

scale_2s.exit.loopexit:                           ; preds = %for.end.i348
  br label %scale_2s.exit

scale_2s.exit:                                    ; preds = %scale_2s.exit.loopexit, %for.body141
  br i1 undef, label %for.end155.loopexit, label %for.body141

for.end155.loopexit:                              ; preds = %scale_2s.exit
  br label %for.end155

for.end155:                                       ; preds = %for.end155.loopexit, %for.body129
  br i1 undef, label %if.end.loopexit, label %for.body129

for.body179:                                      ; preds = %for.body179, %for.body179.lr.ph
  br i1 undef, label %for.body179, label %if.end.loopexit401

if.end.loopexit:                                  ; preds = %for.end155
  br label %if.end

if.end.loopexit401:                               ; preds = %for.body179
  br label %if.end

if.end:                                           ; preds = %if.end.loopexit401, %if.end.loopexit, %for.end116, %for.cond176.preheader
  ret void
}

