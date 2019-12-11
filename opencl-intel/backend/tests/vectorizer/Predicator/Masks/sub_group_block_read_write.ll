; RUN: %oclopt -predicate %s -S | FileCheck %s

; Function Attrs: convergent nounwind readnone
declare i64 @_Z13get_global_idj(i32 %0) local_unnamed_addr #3

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_group_idj(i32 %0) local_unnamed_addr #3

; Function Attrs: convergent nounwind readnone
declare i64 @_Z14get_local_sizej(i32 %0) #3

declare <2 x i32> @_Z27intel_sub_group_block_read2PU3AS1Kj(i32 addrspace(1)*)

; Function Attrs: nounwind readnone
declare i32 @fake.extract.element0(<2 x i32> %0, i32 %1) #2

; Function Attrs: nounwind readnone
declare i32 @fake.extract.element1(<2 x i32> %0, i32 %1) #2

; Function Attrs: convergent nounwind
define void @__Vectorized_.testKernel(i8 addrspace(1)* %input_data, i8 addrspace(1)* %results, i32 addrspace(1)* %preds) local_unnamed_addr {
entry:
  %call = tail call i64 @_Z13get_global_idj(i32 0) #4
  %call1 = tail call i64 @_Z12get_group_idj(i32 0) #4
  %call2 = tail call i64 @_Z14get_local_sizej(i32 0) #4
  %mul = mul i64 %call2, %call1
  %sub37 = add i64 %mul, %call
  %sub37.tr = trunc i64 %sub37 to i32
  %conv21 = shl i32 %sub37.tr, 2
  %reass.add.1 = add i64 %call2, %call
  %sub37.1 = add i64 %mul, %reass.add.1
  %sub37.tr.1 = trunc i64 %sub37.1 to i32
  %conv21.1 = shl i32 %sub37.tr.1, 2
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %preds, i64 %call
  %0 = load i32, i32 addrspace(1)* %arrayidx, align 4
  %tobool = icmp eq i32 %0, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %entry
  %mul.tr = trunc i64 %mul to i32
  %mul22 = shl i32 %mul.tr, 3
  %idx.ext = sext i32 %mul22 to i64
  %add.ptr = getelementptr inbounds i8, i8 addrspace(1)* %input_data, i64 %idx.ext
  %1 = bitcast i8 addrspace(1)* %add.ptr to i32 addrspace(1)*
  %call23_clone = tail call <2 x i32> @_Z27intel_sub_group_block_read2PU3AS1Kj(i32 addrspace(1)* %1) #5
; CHECK-NOT: @maskedf_0__Z27intel_sub_group_block_read2PU3AS1Kj
  %2 = call i32 @fake.extract.element0(<2 x i32> %call23_clone, i32 0) #3
  %3 = call i32 @fake.extract.element1(<2 x i32> %call23_clone, i32 1) #3
  br label %if.end

if.end:                                           ; preds = %if.then, %entry
  %read_result_vector.01 = phi i32 [ %2, %if.then ], [ 0, %entry ]
  %read_result_vector.02 = phi i32 [ %3, %if.then ], [ 0, %entry ]
  %astype = bitcast i32 %read_result_vector.01 to <4 x i8>
  %scalar = extractelement <4 x i8> %astype, i32 0
  %scalar3 = extractelement <4 x i8> %astype, i32 1
  %scalar4 = extractelement <4 x i8> %astype, i32 2
  %scalar5 = extractelement <4 x i8> %astype, i32 3
  %idxprom = zext i32 %conv21 to i64
  %arrayidx33 = getelementptr inbounds i8, i8 addrspace(1)* %results, i64 %idxprom
  store i8 %scalar, i8 addrspace(1)* %arrayidx33, align 1
  %add36 = or i32 %conv21, 1
  %idxprom37 = zext i32 %add36 to i64
  %arrayidx38 = getelementptr inbounds i8, i8 addrspace(1)* %results, i64 %idxprom37
  store i8 %scalar3, i8 addrspace(1)* %arrayidx38, align 1
  %add41 = or i32 %conv21, 2
  %idxprom42 = zext i32 %add41 to i64
  %arrayidx43 = getelementptr inbounds i8, i8 addrspace(1)* %results, i64 %idxprom42
  store i8 %scalar4, i8 addrspace(1)* %arrayidx43, align 1
  %add46 = or i32 %conv21, 3
  %idxprom47 = zext i32 %add46 to i64
  %arrayidx48 = getelementptr inbounds i8, i8 addrspace(1)* %results, i64 %idxprom47
  store i8 %scalar5, i8 addrspace(1)* %arrayidx48, align 1
  %astype.1 = bitcast i32 %read_result_vector.02 to <4 x i8>
  %scalar6 = extractelement <4 x i8> %astype.1, i32 0
  %scalar7 = extractelement <4 x i8> %astype.1, i32 1
  %scalar8 = extractelement <4 x i8> %astype.1, i32 2
  %scalar9 = extractelement <4 x i8> %astype.1, i32 3
  %idxprom.1 = zext i32 %conv21.1 to i64
  %arrayidx33.1 = getelementptr inbounds i8, i8 addrspace(1)* %results, i64 %idxprom.1
  store i8 %scalar6, i8 addrspace(1)* %arrayidx33.1, align 1
  %add36.1 = or i32 %conv21.1, 1
  %idxprom37.1 = zext i32 %add36.1 to i64
  %arrayidx38.1 = getelementptr inbounds i8, i8 addrspace(1)* %results, i64 %idxprom37.1
  store i8 %scalar7, i8 addrspace(1)* %arrayidx38.1, align 1
  %add41.1 = or i32 %conv21.1, 2
  %idxprom42.1 = zext i32 %add41.1 to i64
  %arrayidx43.1 = getelementptr inbounds i8, i8 addrspace(1)* %results, i64 %idxprom42.1
  store i8 %scalar8, i8 addrspace(1)* %arrayidx43.1, align 1
  %add46.1 = or i32 %conv21.1, 3
  %idxprom47.1 = zext i32 %add46.1 to i64
  %arrayidx48.1 = getelementptr inbounds i8, i8 addrspace(1)* %results, i64 %idxprom47.1
  store i8 %scalar9, i8 addrspace(1)* %arrayidx48.1, align 1
  ret void
}

attributes #2 = { nounwind readnone }
attributes #3 = { convergent nounwind readnone }
attributes #4 = { convergent nounwind }
