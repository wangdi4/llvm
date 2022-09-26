; Check HIR vectorizer codegen support for simple non-POD privates which are not
; liveout. HIR vector CG creates a new widened alloca for the private memory
; and all calls appropriate constructor/destructor to initialize and deallocate
; the memory.

; RUN: opt -hir-ssa-deconstruction -hir-framework -hir-vplan-vec -vplan-force-vf=2 -print-after=hir-vplan-vec -hir-details -disable-output < %s 2>&1 | FileCheck %s

; Incoming HIR
;    BEGIN REGION { }
;          %tok = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.PRIVATE:NONPOD(&((%value.priv)[0])@_ZTS6ClassA.omp.def_constr@_ZTS6ClassA.omp.destr) ]
;
;          + DO i1 = 0, zext.i32.i64(%n) + -1, 1   <DO_LOOP>  <MAX_TC_EST = 2147483647>   <LEGAL_MAX_TC = 2147483647> <simd>
;          |   @_ZN6ClassA3incEi(&((%value.priv)[0]),  i1);
;          |   %src.ld = (%src)[i1];
;          |   %m_value.ld = (%value.priv)[0].0;
;          |   %dst.ld = (%dst)[i1];
;          |   (%dst)[i1] = %src.ld + %m_value.ld + %dst.ld;
;          + END LOOP
;
;          @llvm.directive.region.exit(%tok); [ DIR.OMP.END.SIMD() ]
;    END REGION


%struct.ClassA = type { i32 }

; Function Attrs: nounwind uwtable mustprogress
define dso_local void @ctor_dtor(i32* nocapture %dst, i32* nocapture readonly %src, i32 %n) local_unnamed_addr #2 {
; CHECK-LABEL: Function: ctor_dtor
; CHECK:         BEGIN REGION { modified }
; CHECK:            %priv.mem.bc = &((%struct.ClassA*)(%priv.mem)[0]);
; CHECK:            %serial.temp = undef;
; CHECK:            %_ZTS6ClassA.omp.def_constr = @_ZTS6ClassA.omp.def_constr(&((%struct.ClassA*)(%priv.mem)[0]));
; CHECK:            %serial.temp = insertelement %serial.temp,  %_ZTS6ClassA.omp.def_constr,  0;
; CHECK:            %[[extract1:.*]] = extractelement &((<2 x %struct.ClassA*>)(%priv.mem.bc)[<i32 0, i32 1>]),  1;
; CHECK:            %_ZTS6ClassA.omp.def_constr2 = @_ZTS6ClassA.omp.def_constr(%[[extract1]]);
; CHECK:            %serial.temp = insertelement %serial.temp,  %_ZTS6ClassA.omp.def_constr2,  1;

; CHECK:            + DO i64 i1 = 0, {{.*}}, 2   <DO_LOOP>
; CHECK:            + END LOOP

; CHECK:            @_ZTS6ClassA.omp.destr(&((%struct.ClassA*)(%priv.mem)[0]));
; CHECK:            %[[extract19:.*]] = extractelement &((<2 x %struct.ClassA*>)(%priv.mem.bc)[<i32 0, i32 1>]),  1;
; CHECK:            @_ZTS6ClassA.omp.destr(%[[extract19]]);
; CHECK:         END REGION
;

entry:
  %value.priv = alloca %struct.ClassA, align 4
  %value.priv.constr = call %struct.ClassA* @_ZTS6ClassA.omp.def_constr(%struct.ClassA* %value.priv)
  %cmp3.not20 = icmp slt i32 %n, 1
  br i1 %cmp3.not20, label %omp.precond.end, label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:NONPOD.TYPED"(%struct.ClassA* %value.priv, %struct.ClassA zeroinitializer, i32 1, %struct.ClassA* (%struct.ClassA*)* @_ZTS6ClassA.omp.def_constr, void (%struct.ClassA*)* @_ZTS6ClassA.omp.destr) ]
  br label %DIR.OMP.SIMD.128

DIR.OMP.SIMD.128:                                 ; preds = %DIR.OMP.SIMD.1
  %m_value = getelementptr inbounds %struct.ClassA, %struct.ClassA* %value.priv, i64 0, i32 0
  %wide.trip.count27 = zext i32 %n to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %DIR.OMP.SIMD.128, %omp.inner.for.body
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.128 ], [ %indvars.iv.next, %omp.inner.for.body ]
  %iv.trunc = trunc i64 %indvars.iv to i32
  call void @_ZN6ClassA3incEi(%struct.ClassA* nonnull dereferenceable(4) %value.priv, i32 %iv.trunc) #4
  %ptridx = getelementptr inbounds i32, i32* %src, i64 %indvars.iv
  %src.ld = load i32, i32* %ptridx, align 4
  %m_value.ld = load i32, i32* %m_value, align 4
  %add5 = add nsw i32 %src.ld, %m_value.ld
  %ptridx7 = getelementptr inbounds i32, i32* %dst, i64 %indvars.iv
  %dst.ld = load i32, i32* %ptridx7, align 4
  %add8 = add nsw i32 %add5, %dst.ld
  store i32 %add8, i32* %ptridx7, align 4
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count27
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.226, label %omp.inner.for.body

DIR.OMP.END.SIMD.226:                             ; preds = %omp.inner.for.body
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.226
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2
  call void @_ZTS6ClassA.omp.destr(%struct.ClassA* nonnull %value.priv)
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.3, %entry
  ret void
}

; Incoming HIR
;    BEGIN REGION { }
;          + DO i1 = 0, 119, 1   <DO_LOOP>
;          |   %2 = @llvm.directive.region.entry(); [ DIR.OMP.SIMD(),  QUAL.OMP.NORMALIZED.IV(null),  QUAL.OMP.NORMALIZED.UB(null),  QUAL.OMP.LINEAR:IV(&((%j.i.linear.iv)[0])1),  QUAL.OMP.PRIVATE(&((%ref.tmp.i.priv)[0])) ]
;          |
;          |   + DO i2 = 0, 119, 1   <DO_LOOP> <simd>
;          |   |   (%ref.tmp.i.priv)[0].0.0 = 0x400A666660000000;
;          |   |   (%ref.tmp.i.priv)[0].0.1 = 0x40119999A0000000;
;          |   |   %call2.i = @_Z5pointRKSt7complexIfE(&((%ref.tmp.i.priv)[0]));
;          |   + END LOOP
;          |
;          |   @llvm.directive.region.exit(%2); [ DIR.OMP.END.SIMD() ]
;          + END LOOP
;    END REGION

%"class.std::complex" = type { { float, float } }

; Function Attrs: norecurse
define dso_local i32 @nested_loops() local_unnamed_addr #4 {
; CHECK-LABEL: Function: nested_loops
; CHECK:    BEGIN REGION { modified }
; CHECK:          + DO i32 i1 = 0, 119, 1   <DO_LOOP>
; CHECK:          |   %priv.mem.bc = &((%"class.std::complex"*)(%priv.mem)[0]);
; CHECK:          |
; CHECK:          |   + DO i32 i2 = 0, 119, 2   <DO_LOOP> <simd-vectorized> <novectorize>
; CHECK:          |   |   %nsbgepcopy = &((<2 x %"class.std::complex"*>)(%priv.mem.bc)[<i32 0, i32 1>]);
; CHECK:          |   |   <RVAL-REG> &((<2 x %"class.std::complex"*>)(LINEAR %"class.std::complex"* %priv.mem.bc{def@1})[<2 x i32> <i32 0, i32 1>]) inbounds
; CHECK:          |   |   %nsbgepcopy3 = &((%"class.std::complex"*)(%priv.mem)[0]);
; CHECK:          |   |   (<2 x float>*)(%nsbgepcopy)[0].0.0 = 0x400A666660000000;
; CHECK:          |   |   %nsbgepcopy4 = &((<2 x %"class.std::complex"*>)(%priv.mem.bc)[<i32 0, i32 1>]);
; CHECK:          |   |   %nsbgepcopy5 = &((%"class.std::complex"*)(%priv.mem)[0]);
; CHECK:          |   |   (<2 x float>*)(%nsbgepcopy4)[0].0.1 = 0x40119999A0000000;
; CHECK:          |   |   %serial.temp = undef;
; CHECK:          |   |   %_Z5pointRKSt7complexIfE = @_Z5pointRKSt7complexIfE(&((%"class.std::complex"*)(%priv.mem)[0]));
; CHECK:          |   |   %serial.temp = insertelement %serial.temp,  %_Z5pointRKSt7complexIfE,  0;
; CHECK:          |   |   %extract.1. = extractelement &((<2 x %"class.std::complex"*>)(%priv.mem.bc)[<i32 0, i32 1>]),  1;
; CHECK:          |   |   %_Z5pointRKSt7complexIfE6 = @_Z5pointRKSt7complexIfE(%extract.1.);
; CHECK:          |   |   %serial.temp = insertelement %serial.temp,  %_Z5pointRKSt7complexIfE6,  1;
; CHECK:          |   + END LOOP
; CHECK:          + END LOOP
; CHECK:    END REGION

entry:
  %j.i.linear.iv = alloca i32, align 4
  %ref.tmp.i.priv = alloca %"class.std::complex", align 4
  %0 = bitcast i32* %j.i.linear.iv to i8*
  %1 = bitcast %"class.std::complex"* %ref.tmp.i.priv to i8*
  %_M_value.realp.i.i = getelementptr inbounds %"class.std::complex", %"class.std::complex"* %ref.tmp.i.priv, i32 0, i32 0, i32 0
  %_M_value.imagp.i.i = getelementptr inbounds %"class.std::complex", %"class.std::complex"* %ref.tmp.i.priv, i32 0, i32 0, i32 1
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %DIR.OMP.END.SIMD.2, %entry
  %i.0.i13 = phi i32 [ 0, %entry ], [ %inc.i, %DIR.OMP.END.SIMD.2 ]
  br label %DIR.OMP.SIMD.115

DIR.OMP.SIMD.115:                                 ; preds = %DIR.OMP.SIMD.1
  %2 = call token @llvm.directive.region.entry() #5 [ "DIR.OMP.SIMD"(), "QUAL.OMP.PRIVATE:TYPED"(%"class.std::complex"* %ref.tmp.i.priv, %"class.std::complex" zeroinitializer, i32 1) ]
  br label %omp.inner.for.body.i

omp.inner.for.body.i:                             ; preds = %DIR.OMP.SIMD.115, %omp.inner.for.body.i
  %.omp.iv.i.local.09 = phi i32 [ 0, %DIR.OMP.SIMD.115 ], [ %add3.i, %omp.inner.for.body.i ]
  store float 0x400A666660000000, float* %_M_value.realp.i.i, align 4
  store float 0x40119999A0000000, float* %_M_value.imagp.i.i, align 4
  %call2.i = call i32 @_Z5pointRKSt7complexIfE(%"class.std::complex"* nonnull align 4 dereferenceable(8) %ref.tmp.i.priv) #5
  %add3.i = add nuw nsw i32 %.omp.iv.i.local.09, 1
  %exitcond.not = icmp eq i32 %add3.i, 120
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.1, label %omp.inner.for.body.i

DIR.OMP.END.SIMD.1:                               ; preds = %omp.inner.for.body.i
  call void @llvm.directive.region.exit(token %2) #5 [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.2

DIR.OMP.END.SIMD.2:                               ; preds = %DIR.OMP.END.SIMD.1
  %inc.i = add nuw nsw i32 %i.0.i13, 1
  %exitcond14.not = icmp eq i32 %inc.i, 120
  br i1 %exitcond14.not, label %_ZL9MandelOMPv.exit, label %DIR.OMP.SIMD.1

_ZL9MandelOMPv.exit:                              ; preds = %DIR.OMP.END.SIMD.2
  %ref.tmp.i4 = alloca %"class.std::complex", align 4
  %3 = bitcast %"class.std::complex"* %ref.tmp.i4 to i8*
  %_M_value.realp.i.i5 = getelementptr inbounds %"class.std::complex", %"class.std::complex"* %ref.tmp.i4, i32 0, i32 0, i32 0
  store float 0x3FF19999A0000000, float* %_M_value.realp.i.i5, align 4
  %_M_value.imagp.i.i6 = getelementptr inbounds %"class.std::complex", %"class.std::complex"* %ref.tmp.i4, i32 0, i32 0, i32 1
  store float 0x40019999A0000000, float* %_M_value.imagp.i.i6, align 4
  %call1.i = call i32 @_Z5pointRKSt7complexIfE(%"class.std::complex"* nonnull align 4 dereferenceable(8) %ref.tmp.i4)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #4

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #4

; Function Attrs: nofree norecurse nounwind uwtable willreturn
declare %struct.ClassA* @_ZTS6ClassA.omp.def_constr(%struct.ClassA* nonnull returned %0) #5

; Function Attrs: nofree norecurse nounwind uwtable willreturn
declare void @_ZTS6ClassA.omp.destr(%struct.ClassA* nocapture readnone %0) #5

; Function Attrs: nofree noinline norecurse nounwind uwtable willreturn mustprogress
declare void @_ZN6ClassA3incEi(%struct.ClassA* nocapture nonnull dereferenceable(4) %this, i32 %a) #1

; Function Attrs: mustprogress nofree noinline nosync nounwind willreturn
declare dso_local i32 @_Z5pointRKSt7complexIfE(%"class.std::complex"* nocapture nonnull readonly align 4 dereferenceable(8) %c) #2

attributes #1 = { nofree noinline norecurse nounwind uwtable willreturn mustprogress }
attributes #2 = { mustprogress nofree noinline nosync nounwind willreturn }
