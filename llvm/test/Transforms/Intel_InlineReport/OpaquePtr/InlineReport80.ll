; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt -opaque-pointers < %s -S -passes='cgscc(inline)' -pre-lto-inline-cost -inline-report=0xe807 2>&1 | FileCheck %s
; RUN: opt -opaque-pointers -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -pre-lto-inline-cost -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-MD

; Check results for type #1 double external callsite inlining heuristic

; Check IR and inlining report when classic inlining report is specified

; CHECK: define void @add_bond_forces
; CHECK: define i32 @mc_polymer_iteration
; CHECK: call token @llvm.directive.region.entry
; CHECK-NOT: call void @trial_move_smc
; CHECK: call void @llvm.directive.region.exit
; CHECK: define i32 @mc_set_iteration
; CHECK: call token @llvm.directive.region.entry
; CHECK-NOT: call void @trial_move_smc
; CHECK: call void @llvm.directive.region.exit

; CHECK: COMPILE FUNC: add_bond_forces
; CHECK: COMPILE FUNC: trial_move_smc
; CHECK: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage
; CHECK: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage

; CHECK: COMPILE FUNC: mc_polymer_iteration
; CHECK: llvm.directive.region.entry{{.*}}Callee is intrinsic
; CHECK: INLINE: trial_move_smc{{.*}}Callee has double callsite without local linkage
; CHECK: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage
; CHECK: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage
; CHECK: llvm.directive.region.exit{{.*}}Callee is intrinsic
; CHECK: COMPILE FUNC: mc_set_iteration
; CHECK: llvm.directive.region.entry{{.*}}Callee is intrinsic
; CHECK: INLINE: trial_move_smc{{.*}}Callee has double callsite without local linkage
; CHECK: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage
; CHECK: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage
; CHECK: llvm.directive.region.exit{{.*}}Callee is intrinsic

; Check IR and inlining report when metadata inlining report is specified

; CHECK-MD: COMPILE FUNC: add_bond_forces
; CHECK-MD: COMPILE FUNC: mc_polymer_iteration
; CHECK-MD: llvm.directive.region.entry{{.*}}Callee is intrinsic
; CHECK-MD: INLINE: trial_move_smc{{.*}}Callee has double callsite without local linkage
; CHECK-MD: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage
; CHECK-MD: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage
; CHECK-MD: llvm.directive.region.exit{{.*}}Callee is intrinsic
; CHECK-MD: COMPILE FUNC: mc_set_iteration
; CHECK-MD: llvm.directive.region.entry{{.*}}Callee is intrinsic
; CHECK-MD: INLINE: trial_move_smc{{.*}}Callee has double callsite without local linkage
; CHECK-MD: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage
; CHECK-MD: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage
; CHECK-MD: llvm.directive.region.exit{{.*}}Callee is intrinsic
; CHECK-MD: COMPILE FUNC: trial_move_smc
; CHECK-MD: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage
; CHECK-MD: INLINE: add_bond_forces{{.*}}Callee has double callsite without local linkage

; CHECK-MD: define void @add_bond_forces
; CHECK-MD: define i32 @mc_polymer_iteration
; CHECK-MD: call token @llvm.directive.region.entry
; CHECK-MD-NOT: call void @trial_move_smc
; CHECK-MD: call void @llvm.directive.region.exit
; CHECK-MD: define i32 @mc_set_iteration
; CHECK-MD: call token @llvm.directive.region.entry
; CHECK-NOT: call void @trial_move_smc
; CHECK-MD: call void @llvm.directive.region.exit
; CHECK-MD: define void @trial_move_smc
; CHECK-MD-NOT: call void @add_bond_forces

%struct.Phase = type { i32, double, double, i32, ptr, i64, i64, i64, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, ptr, i32, i32, ptr, i32, i64, i64, i32, i32, i32, i64, double, double, double, double, double, double, i64, i64, double, ptr, %struct.Info_MPI, i32, ptr, i32, ptr, %struct.som_args, i64, i32, i8, double, i32, ptr, i32, i32, %struct.Autotuner, %struct.Autotuner, double, i32, ptr }
%struct.Info_MPI = type { i32, i32, i32, i32, %struct.MPI_Status, double, i32 }
%struct.MPI_Status = type { i32, i32, i32, i32, i32 }
%struct.som_args = type { ptr, ptr, i32, ptr, ptr, i32, ptr, ptr, i32, ptr, ptr, double, ptr, ptr, i32, ptr, ptr, i32, ptr, ptr, i32, ptr, ptr, i32, ptr, i32, ptr, ptr, i32, ptr, ptr, i32, ptr, i32, ptr, ptr, ptr, ptr, ptr, i32, ptr, ptr, i32, ptr, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.Autotuner = type { [12 x i32], [12 x double], double, i8, i32, i32, i8, i32 }
%struct.Polymer = type { i64, i64, i32, %struct.RNG_STATE, ptr, ptr }
%struct.RNG_STATE = type { %struct.PCG_STATE, ptr, ptr }
%struct.PCG_STATE = type { i64, i64 }
%struct.Allocator = type { %struct.anon, %struct.anon.0, %struct.anon.1, %struct.anon.2, %struct.anon.3, %struct.anon.4 }
%struct.anon = type { i64, i64, ptr, ptr }
%struct.anon.0 = type { i64, i64, ptr, ptr }
%struct.anon.1 = type { i64, i64, ptr, ptr }
%struct.anon.2 = type { i64, i64, ptr, ptr }
%struct.anon.3 = type { i64, i64, ptr, ptr }
%struct.anon.4 = type { i64, i64, ptr, ptr }
%struct.Monomer = type { double, double, double }
%struct.IndependetSets = type { i32, i32, ptr, ptr }

@stderr = external dso_local global ptr, align 8
@.str = private unnamed_addr constant [46 x i8] c"ERROR: %s:%d stiff bond not yet implemented.\0A\00", align 1
@.str.1 = private unnamed_addr constant [5 x i8] c"mc.c\00", align 1
@.str.2 = private unnamed_addr constant [43 x i8] c"ERROR: %s:%d unknow bond type appeared %d\0A\00", align 1

declare dso_local i32 @get_bondlist_offset(i32)

declare dso_local i32 @get_end(i32)

declare dso_local i32 @get_bond_type(i32)

declare dso_local i32 @get_offset(i32)

declare dso_local i32 @fprintf(ptr, ptr, ...)

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #0

declare dso_local i32 @soma_rng_uint(ptr, i32)

declare dso_local i32 @get_particle_type(i32)

declare void @trial_move(ptr, i64, i32, ptr, ptr, ptr, i32, i32, ptr)

declare i32 @possible_move_area51(ptr, double, double, double, double, double, double, i32)

declare double @calc_delta_energy(ptr, i64, ptr, i32, double, double, double, i32)

declare zeroext i1 @som_accept(ptr, i32, double)

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #0

declare dso_local void @soma_normal_vector(ptr, i32, ptr, ptr, ptr)

define void @add_bond_forces(ptr %p, i64 %ipoly, i32 %ibead, double %x, double %y, double %z, ptr %fx, ptr %fy, ptr %fz) local_unnamed_addr {
entry:
  %poly_arch = getelementptr inbounds %struct.Phase, ptr %p, i64 0, i32 45
  %i = load ptr, ptr %poly_arch, align 8
  %poly_type_offset = getelementptr inbounds %struct.Phase, ptr %p, i64 0, i32 43
  %i1 = load ptr, ptr %poly_type_offset, align 8
  %polymers = getelementptr inbounds %struct.Phase, ptr %p, i64 0, i32 8
  %i2 = load ptr, ptr %polymers, align 8
  %type = getelementptr inbounds %struct.Polymer, ptr %i2, i64 %ipoly, i32 2
  %i3 = load i32, ptr %type, align 8
  %idxprom = zext i32 %i3 to i64
  %ptridx1 = getelementptr inbounds i32, ptr %i1, i64 %idxprom
  %i4 = load i32, ptr %ptridx1, align 4
  %add = add i32 %i4, %ibead
  %add2 = add i32 %add, 1
  %idxprom3 = zext i32 %add2 to i64
  %ptridx4 = getelementptr inbounds i32, ptr %i, i64 %idxprom3
  %i5 = load i32, ptr %ptridx4, align 4
  %call = call i32 @get_bondlist_offset(i32 %i5)
  %cmp = icmp sgt i32 %call, 0
  br i1 %cmp, label %do.body, label %if.end

do.body:                                          ; preds = %sw.epilog, %entry
  %i.0 = phi i32 [ %inc, %sw.epilog ], [ %call, %entry ]
  %v1z.0 = phi double [ %v1z.1, %sw.epilog ], [ 0.000000e+00, %entry ]
  %v1y.0 = phi double [ %v1y.1, %sw.epilog ], [ 0.000000e+00, %entry ]
  %v1x.0 = phi double [ %v1x.1, %sw.epilog ], [ 0.000000e+00, %entry ]
  %i6 = load ptr, ptr %poly_arch, align 8
  %inc = add nsw i32 %i.0, 1
  %idxprom6 = sext i32 %i.0 to i64
  %ptridx7 = getelementptr inbounds i32, ptr %i6, i64 %idxprom6
  %i7 = load i32, ptr %ptridx7, align 4
  %call8 = call i32 @get_end(i32 %i7)
  %call9 = call i32 @get_bond_type(i32 %i7)
  %call10 = call i32 @get_offset(i32 %i7)
  %add11 = add i32 %call10, %ibead
  switch i32 %call9, label %sw.default [
    i32 2, label %sw.bb
    i32 0, label %sw.bb12
    i32 1, label %sw.bb53
  ]

sw.bb:                                            ; preds = %do.body
  %harmonic_normb_variable_scale = getelementptr inbounds %struct.Phase, ptr %p, i64 0, i32 2
  %i8 = load double, ptr %harmonic_normb_variable_scale, align 8
  br label %sw.bb12

sw.bb12:                                          ; preds = %sw.bb, %do.body
  %scale.0 = phi double [ 1.000000e+00, %do.body ], [ %i8, %sw.bb ]
  %allocator = getelementptr inbounds %struct.Phase, ptr %p, i64 0, i32 59
  %i9 = load ptr, ptr %allocator, align 8
  %device_buf = getelementptr inbounds %struct.Allocator, ptr %i9, i64 0, i32 0, i32 3
  %i10 = load ptr, ptr %device_buf, align 8
  %i11 = load ptr, ptr %polymers, align 8
  %beads = getelementptr inbounds %struct.Polymer, ptr %i11, i64 %ipoly, i32 0
  %i12 = load i64, ptr %beads, align 8
  %conv = zext i32 %add11 to i64
  %add15 = add i64 %i12, %conv
  %x17 = getelementptr inbounds %struct.Monomer, ptr %i10, i64 %add15, i32 0
  %i13 = load double, ptr %x17, align 8
  %sub = fsub fast double %i13, %x
  %mul = fmul fast double %sub, 2.000000e+00
  %harmonic_normb = getelementptr inbounds %struct.Phase, ptr %p, i64 0, i32 1
  %i14 = load double, ptr %harmonic_normb, align 8
  %mul18 = fmul fast double %mul, %i14
  %mul19 = fmul fast double %mul18, %scale.0
  %add20 = fadd fast double %v1x.0, %mul19
  %y30 = getelementptr inbounds %struct.Monomer, ptr %i10, i64 %add15, i32 1
  %i15 = load double, ptr %y30, align 8
  %sub31 = fsub fast double %i15, %y
  %mul32 = fmul fast double %sub31, 2.000000e+00
  %mul34 = fmul fast double %mul32, %i14
  %mul35 = fmul fast double %mul34, %scale.0
  %add36 = fadd fast double %v1y.0, %mul35
  %z46 = getelementptr inbounds %struct.Monomer, ptr %i10, i64 %add15, i32 2
  %i16 = load double, ptr %z46, align 8
  %sub47 = fsub fast double %i16, %z
  %mul48 = fmul fast double %sub47, 2.000000e+00
  %mul50 = fmul fast double %mul48, %i14
  %mul51 = fmul fast double %mul50, %scale.0
  %add52 = fadd fast double %v1z.0, %mul51
  br label %sw.epilog

sw.bb53:                                          ; preds = %do.body
  %i17 = load ptr, ptr @stderr, align 8
  %call54 = call i32 (ptr, ptr, ...) @fprintf(ptr %i17, ptr @.str, ptr @.str.1, i32 748)
  br label %sw.epilog

sw.default:                                       ; preds = %do.body
  %i18 = load ptr, ptr @stderr, align 8
  %call55 = call i32 (ptr, ptr, ...) @fprintf(ptr %i18, ptr @.str.2, ptr @.str.1, i32 758, i32 %call9)
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb53, %sw.bb12
  %v1z.1 = phi double [ %v1z.0, %sw.default ], [ %v1z.0, %sw.bb53 ], [ %add52, %sw.bb12 ]
  %v1y.1 = phi double [ %v1y.0, %sw.default ], [ %v1y.0, %sw.bb53 ], [ %add36, %sw.bb12 ]
  %v1x.1 = phi double [ %v1x.0, %sw.default ], [ %v1x.0, %sw.bb53 ], [ %add20, %sw.bb12 ]
  %cmp56 = icmp eq i32 %call8, 0
  br i1 %cmp56, label %do.body, label %if.end

if.end:                                           ; preds = %sw.epilog, %entry
  %v1z.2 = phi double [ 0.000000e+00, %entry ], [ %v1z.1, %sw.epilog ]
  %v1y.2 = phi double [ 0.000000e+00, %entry ], [ %v1y.1, %sw.epilog ]
  %v1x.2 = phi double [ 0.000000e+00, %entry ], [ %v1x.1, %sw.epilog ]
  %i19 = load double, ptr %fx, align 8
  %add58 = fadd fast double %i19, %v1x.2
  store double %add58, ptr %fx, align 8
  %i20 = load double, ptr %fy, align 8
  %add59 = fadd fast double %i20, %v1y.2
  store double %add59, ptr %fy, align 8
  %i21 = load double, ptr %fz, align 8
  %add60 = fadd fast double %i21, %v1z.2
  store double %add60, ptr %fz, align 8
  ret void
}

define i32 @mc_polymer_iteration(ptr %p, i32 %nsteps, i32 %tuning_parameter) local_unnamed_addr #1 {
entry:
  %p.addr = alloca ptr, align 8
  %n_accepts = alloca i32, align 4
  %.omp.iv = alloca i64, align 8
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %npoly = alloca i64, align 8
  %accepted_moves_loc = alloca i32, align 4
  %mypoly = alloca ptr, align 8
  %myN = alloca i32, align 4
  %myrngstate = alloca ptr, align 8
  %arg_rng_type = alloca i32, align 4
  %nmc = alloca i32, align 4
  %dx = alloca double, align 8
  %dy = alloca double, align 8
  %dz = alloca double, align 8
  %delta_energy = alloca double, align 8
  %ibead = alloca i32, align 4
  %iwtype = alloca i32, align 4
  %mybead = alloca %struct.Monomer, align 8
  %mybead_ptr = alloca ptr, align 8
  %smc_deltaE = alloca double, align 8
  %newx = alloca double, align 8
  %newy = alloca double, align 8
  %newz = alloca double, align 8
  %move_allowed = alloca i32, align 4
  store ptr %p, ptr %p.addr, align 8
  %npoly.addr = alloca ptr, align 8
  %accepted_moves_loc.addr = alloca ptr, align 8
  %mypoly.addr = alloca ptr, align 8
  %myN.addr = alloca ptr, align 8
  %myrngstate.addr = alloca ptr, align 8
  %arg_rng_type.addr = alloca ptr, align 8
  %nmc.addr = alloca ptr, align 8
  %delta_energy.addr = alloca ptr, align 8
  %dx.addr = alloca ptr, align 8
  %dy.addr = alloca ptr, align 8
  %dz.addr = alloca ptr, align 8
  %ibead.addr = alloca ptr, align 8
  %iwtype.addr = alloca ptr, align 8
  %mybead.addr = alloca ptr, align 8
  %mybead_ptr.addr = alloca ptr, align 8
  %smc_deltaE.addr = alloca ptr, align 8
  %newx.addr = alloca ptr, align 8
  %newy.addr = alloca ptr, align 8
  %newz.addr = alloca ptr, align 8
  %move_allowed.addr = alloca ptr, align 8
  %.omp.lb.addr = alloca ptr, align 8
  %p.addr.addr = alloca ptr, align 8
  %n_accepts.addr = alloca ptr, align 8
  br label %for.cond

for.cond:                                         ; preds = %omp.precond.end, %entry
  %step.0 = phi i32 [ 0, %entry ], [ %inc64, %omp.precond.end ]
  %cmp = icmp ult i32 %step.0, %nsteps
  br i1 %cmp, label %for.body, label %for.end65

for.body:                                         ; preds = %for.cond
  %i = load ptr, ptr %p.addr, align 8
  %n_polymers1 = getelementptr inbounds %struct.Phase, ptr %i, i64 0, i32 5
  %i1 = load i64, ptr %n_polymers1, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %n_accepts)
  store i32 0, ptr %n_accepts, align 4
  %cmp4 = icmp eq i64 %i1, 0
  br i1 %cmp4, label %omp.precond.end, label %DIR.OMP.PARALLEL.LOOP.397

DIR.OMP.PARALLEL.LOOP.397:                        ; preds = %for.body
  %sub2 = add i64 %i1, -1
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %.omp.iv)
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %.omp.lb)
  store i64 0, ptr %.omp.lb, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %.omp.ub)
  store volatile i64 %sub2, ptr %.omp.ub, align 8
  store ptr %npoly, ptr %npoly.addr, align 8
  store ptr %accepted_moves_loc, ptr %accepted_moves_loc.addr, align 8
  store ptr %mypoly, ptr %mypoly.addr, align 8
  store ptr %myN, ptr %myN.addr, align 8
  store ptr %myrngstate, ptr %myrngstate.addr, align 8
  store ptr %arg_rng_type, ptr %arg_rng_type.addr, align 8
  store ptr %nmc, ptr %nmc.addr, align 8
  store ptr %delta_energy, ptr %delta_energy.addr, align 8
  store ptr %dx, ptr %dx.addr, align 8
  store ptr %dy, ptr %dy.addr, align 8
  store ptr %dz, ptr %dz.addr, align 8
  store ptr %ibead, ptr %ibead.addr, align 8
  store ptr %iwtype, ptr %iwtype.addr, align 8
  store ptr %mybead, ptr %mybead.addr, align 8
  store ptr %mybead_ptr, ptr %mybead_ptr.addr, align 8
  store ptr %smc_deltaE, ptr %smc_deltaE.addr, align 8
  store ptr %newx, ptr %newx.addr, align 8
  store ptr %newy, ptr %newy.addr, align 8
  store ptr %newz, ptr %newz.addr, align 8
  store ptr %move_allowed, ptr %move_allowed.addr, align 8
  store ptr %.omp.lb, ptr %.omp.lb.addr, align 8
  store ptr %p.addr, ptr %p.addr.addr, align 8
  store ptr %n_accepts, ptr %n_accepts.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %i6 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(ptr %n_accepts), "QUAL.OMP.SHARED"(ptr %p.addr), "QUAL.OMP.NORMALIZED.IV"(ptr %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(ptr %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(ptr %.omp.ub), "QUAL.OMP.PRIVATE"(ptr %npoly), "QUAL.OMP.PRIVATE"(ptr %accepted_moves_loc), "QUAL.OMP.PRIVATE"(ptr %mypoly), "QUAL.OMP.PRIVATE"(ptr %myN), "QUAL.OMP.PRIVATE"(ptr %myrngstate), "QUAL.OMP.PRIVATE"(ptr %arg_rng_type), "QUAL.OMP.PRIVATE"(ptr %nmc), "QUAL.OMP.PRIVATE"(ptr %delta_energy), "QUAL.OMP.PRIVATE"(ptr %dx), "QUAL.OMP.PRIVATE"(ptr %dy), "QUAL.OMP.PRIVATE"(ptr %dz), "QUAL.OMP.PRIVATE"(ptr %ibead), "QUAL.OMP.PRIVATE"(ptr %iwtype), "QUAL.OMP.PRIVATE"(ptr %mybead), "QUAL.OMP.PRIVATE"(ptr %mybead_ptr), "QUAL.OMP.PRIVATE"(ptr %smc_deltaE), "QUAL.OMP.PRIVATE"(ptr %newx), "QUAL.OMP.PRIVATE"(ptr %newy), "QUAL.OMP.PRIVATE"(ptr %newz), "QUAL.OMP.PRIVATE"(ptr %move_allowed), "QUAL.OMP.OPERAND.ADDR"(ptr %npoly, ptr %npoly.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %accepted_moves_loc, ptr %accepted_moves_loc.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %mypoly, ptr %mypoly.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %myN, ptr %myN.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %myrngstate, ptr %myrngstate.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %arg_rng_type, ptr %arg_rng_type.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %nmc, ptr %nmc.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %delta_energy, ptr %delta_energy.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %dx, ptr %dx.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %dy, ptr %dy.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %dz, ptr %dz.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %ibead, ptr %ibead.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %iwtype, ptr %iwtype.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %mybead, ptr %mybead.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %mybead_ptr, ptr %mybead_ptr.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %smc_deltaE, ptr %smc_deltaE.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %newx, ptr %newx.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %newy, ptr %newy.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %newz, ptr %newz.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %move_allowed, ptr %move_allowed.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb, ptr %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %p.addr, ptr %p.addr.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %n_accepts, ptr %n_accepts.addr), "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.PARALLEL.LOOP.4, label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.397
  %npoly67 = load volatile ptr, ptr %npoly.addr, align 8
  %accepted_moves_loc68 = load volatile ptr, ptr %accepted_moves_loc.addr, align 8
  %mypoly69 = load volatile ptr, ptr %mypoly.addr, align 8
  %myN70 = load volatile ptr, ptr %myN.addr, align 8
  %myrngstate71 = load volatile ptr, ptr %myrngstate.addr, align 8
  %arg_rng_type72 = load volatile ptr, ptr %arg_rng_type.addr, align 8
  %nmc73 = load volatile ptr, ptr %nmc.addr, align 8
  %delta_energy74 = load volatile ptr, ptr %delta_energy.addr, align 8
  %dx75 = load volatile ptr, ptr %dx.addr, align 8
  %dy76 = load volatile ptr, ptr %dy.addr, align 8
  %dz77 = load volatile ptr, ptr %dz.addr, align 8
  %ibead78 = load volatile ptr, ptr %ibead.addr, align 8
  %iwtype79 = load volatile ptr, ptr %iwtype.addr, align 8
  %mybead80 = load volatile ptr, ptr %mybead.addr, align 8
  %mybead_ptr81 = load volatile ptr, ptr %mybead_ptr.addr, align 8
  %smc_deltaE82 = load volatile ptr, ptr %smc_deltaE.addr, align 8
  %newx83 = load volatile ptr, ptr %newx.addr, align 8
  %newy84 = load volatile ptr, ptr %newy.addr, align 8
  %newz85 = load volatile ptr, ptr %newz.addr, align 8
  %move_allowed86 = load volatile ptr, ptr %move_allowed.addr, align 8
  %.omp.lb87 = load volatile ptr, ptr %.omp.lb.addr, align 8
  %p.addr88 = load volatile ptr, ptr %p.addr.addr, align 8
  %n_accepts89 = load volatile ptr, ptr %n_accepts.addr, align 8
  %i7 = load i64, ptr %.omp.lb87, align 8
  store volatile i64 %i7, ptr %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %for.cond.cleanup, %DIR.OMP.PARALLEL.LOOP.3
  %i8 = load volatile i64, ptr %.omp.iv, align 8
  %i9 = load volatile i64, ptr %.omp.ub, align 8
  %add5 = add i64 %i9, 1
  %cmp6 = icmp ult i64 %i8, %add5
  br i1 %cmp6, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.4

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 8, ptr %npoly67)
  %i11 = load volatile i64, ptr %.omp.iv, align 8
  store i64 %i11, ptr %npoly67, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %accepted_moves_loc68)
  store i32 0, ptr %accepted_moves_loc68, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr %mypoly69)
  %i14 = load ptr, ptr %p.addr88, align 8
  %polymers = getelementptr inbounds %struct.Phase, ptr %i14, i64 0, i32 8
  %i15 = load ptr, ptr %polymers, align 8
  %i16 = load i64, ptr %npoly67, align 8
  %ptridx = getelementptr inbounds %struct.Polymer, ptr %i15, i64 %i16
  store ptr %ptridx, ptr %mypoly69, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %myN70)
  %i18 = load ptr, ptr %p.addr88, align 8
  %poly_arch = getelementptr inbounds %struct.Phase, ptr %i18, i64 0, i32 45
  %i19 = load ptr, ptr %poly_arch, align 8
  %poly_type_offset = getelementptr inbounds %struct.Phase, ptr %i18, i64 0, i32 43
  %i20 = load ptr, ptr %poly_type_offset, align 8
  %i21 = load ptr, ptr %mypoly69, align 8
  %type = getelementptr inbounds %struct.Polymer, ptr %i21, i64 0, i32 2
  %i22 = load i32, ptr %type, align 8
  %idxprom = zext i32 %i22 to i64
  %ptridx8 = getelementptr inbounds i32, ptr %i20, i64 %idxprom
  %i23 = load i32, ptr %ptridx8, align 4
  %idxprom9 = sext i32 %i23 to i64
  %ptridx10 = getelementptr inbounds i32, ptr %i19, i64 %idxprom9
  %i24 = load i32, ptr %ptridx10, align 4
  store i32 %i24, ptr %myN70, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr %myrngstate71)
  %i26 = load ptr, ptr %mypoly69, align 8
  %poly_state = getelementptr inbounds %struct.Polymer, ptr %i26, i64 0, i32 3
  store ptr %poly_state, ptr %myrngstate71, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %arg_rng_type72)
  %i28 = load ptr, ptr %p.addr88, align 8
  %pseudo_random_number_generator_arg = getelementptr inbounds %struct.Phase, ptr %i28, i64 0, i32 46, i32 17
  %i29 = load i32, ptr %pseudo_random_number_generator_arg, align 8
  store i32 %i29, ptr %arg_rng_type72, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %nmc73)
  br label %for.cond11

for.cond11:                                       ; preds = %if.end55, %omp.inner.for.body
  %storemerge = phi i32 [ 0, %omp.inner.for.body ], [ %inc, %if.end55 ]
  store i32 %storemerge, ptr %nmc73, align 4
  %i31 = load i32, ptr %myN70, align 4
  %cmp12 = icmp ult i32 %storemerge, %i31
  br i1 %cmp12, label %for.body13, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond11
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %nmc73)
  %i32 = load i32, ptr %accepted_moves_loc68, align 4
  %i33 = load i32, ptr %n_accepts89, align 4
  %add56 = add i32 %i33, %i32
  store i32 %add56, ptr %n_accepts89, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %arg_rng_type72)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %myrngstate71)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %myN70)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %mypoly69)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %accepted_moves_loc68)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %npoly67)
  %i34 = load volatile i64, ptr %.omp.iv, align 8
  %add57 = add nuw i64 %i34, 1
  store volatile i64 %add57, ptr %.omp.iv, align 8
  br label %omp.inner.for.cond

for.body13:                                       ; preds = %for.cond11
  call void @llvm.lifetime.start.p0(i64 8, ptr %dx75)
  store double 0.000000e+00, ptr %dx75, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %dy76)
  store double 0.000000e+00, ptr %dy76, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %dz77)
  store double 0.000000e+00, ptr %dz77, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %delta_energy74)
  store double 0.000000e+00, ptr %delta_energy74, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %ibead78)
  %i40 = load ptr, ptr %myrngstate71, align 8
  %i41 = load i32, ptr %arg_rng_type72, align 4
  %call = call i32 @soma_rng_uint(ptr %i40, i32 %i41)
  %i42 = load i32, ptr %myN70, align 4
  %rem = urem i32 %call, %i42
  store i32 %rem, ptr %ibead78, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %iwtype79)
  %i44 = load ptr, ptr %p.addr88, align 8
  %poly_arch14 = getelementptr inbounds %struct.Phase, ptr %i44, i64 0, i32 45
  %i45 = load ptr, ptr %poly_arch14, align 8
  %poly_type_offset15 = getelementptr inbounds %struct.Phase, ptr %i44, i64 0, i32 43
  %i46 = load ptr, ptr %poly_type_offset15, align 8
  %i47 = load ptr, ptr %mypoly69, align 8
  %type16 = getelementptr inbounds %struct.Polymer, ptr %i47, i64 0, i32 2
  %i48 = load i32, ptr %type16, align 8
  %idxprom17 = zext i32 %i48 to i64
  %ptridx18 = getelementptr inbounds i32, ptr %i46, i64 %idxprom17
  %i49 = load i32, ptr %ptridx18, align 4
  %add19 = add nsw i32 %i49, 1
  %i50 = load i32, ptr %ibead78, align 4
  %add20 = add i32 %add19, %i50
  %idxprom21 = zext i32 %add20 to i64
  %ptridx22 = getelementptr inbounds i32, ptr %i45, i64 %idxprom21
  %i51 = load i32, ptr %ptridx22, align 4
  %call23 = call i32 @get_particle_type(i32 %i51)
  store i32 %call23, ptr %iwtype79, align 4
  call void @llvm.lifetime.start.p0(i64 24, ptr %mybead80)
  %i53 = load ptr, ptr %p.addr88, align 8
  %allocator = getelementptr inbounds %struct.Phase, ptr %i53, i64 0, i32 59
  %i54 = load ptr, ptr %allocator, align 8
  %device_buf = getelementptr inbounds %struct.Allocator, ptr %i54, i64 0, i32 0, i32 3
  %i55 = load ptr, ptr %device_buf, align 8
  %i56 = load ptr, ptr %mypoly69, align 8
  %beads = getelementptr inbounds %struct.Polymer, ptr %i56, i64 0, i32 0
  %i57 = load i64, ptr %beads, align 8
  %i58 = load i32, ptr %ibead78, align 4
  %conv = zext i32 %i58 to i64
  %add24 = add i64 %i57, %conv
  %ptridx25 = getelementptr inbounds %struct.Monomer, ptr %i55, i64 %add24
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 dereferenceable(24) %mybead80, ptr nonnull align 8 dereferenceable(24) %ptridx25, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 8, ptr %mybead_ptr81)
  %i61 = load ptr, ptr %p.addr88, align 8
  %allocator26 = getelementptr inbounds %struct.Phase, ptr %i61, i64 0, i32 59
  %i62 = load ptr, ptr %allocator26, align 8
  %device_buf28 = getelementptr inbounds %struct.Allocator, ptr %i62, i64 0, i32 0, i32 3
  %i63 = load ptr, ptr %device_buf28, align 8
  %i64 = load ptr, ptr %mypoly69, align 8
  %beads29 = getelementptr inbounds %struct.Polymer, ptr %i64, i64 0, i32 0
  %i65 = load i64, ptr %beads29, align 8
  %i66 = load i32, ptr %ibead78, align 4
  %conv30 = zext i32 %i66 to i64
  %add31 = add i64 %i65, %conv30
  %ptridx32 = getelementptr inbounds %struct.Monomer, ptr %i63, i64 %add31
  store ptr %ptridx32, ptr %mybead_ptr81, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %smc_deltaE82)
  %i68 = load ptr, ptr %p.addr88, align 8
  %move_type_arg = getelementptr inbounds %struct.Phase, ptr %i68, i64 0, i32 46, i32 25
  %i69 = load i32, ptr %move_type_arg, align 8
  switch i32 %i69, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb34
  ]

sw.bb:                                            ; preds = %for.body13
  %i70 = load i32, ptr %ibead78, align 4
  %i71 = load i32, ptr %iwtype79, align 4
  %i72 = load i32, ptr %arg_rng_type72, align 4
  %i73 = load ptr, ptr %myrngstate71, align 8
  call void @trial_move(ptr %i68, i64 undef, i32 %i70, ptr nonnull %dx75, ptr nonnull %dy76, ptr nonnull %dz77, i32 %i71, i32 %i72, ptr %i73)
  store double 0.000000e+00, ptr %smc_deltaE82, align 8
  br label %sw.epilog

sw.bb34:                                          ; preds = %for.body13
  %i74 = load i64, ptr %npoly67, align 8
  %i75 = load i32, ptr %ibead78, align 4
  %i76 = load ptr, ptr %myrngstate71, align 8
  %i77 = load i32, ptr %arg_rng_type72, align 4
  %i78 = load i32, ptr %iwtype79, align 4
  call void @trial_move_smc(ptr %i68, i64 %i74, i32 %i75, ptr nonnull %dx75, ptr nonnull %dy76, ptr nonnull %dz77, ptr %smc_deltaE82, ptr %mybead80, ptr %i76, i32 %i77, i32 %i78)
  br label %sw.epilog

sw.default:                                       ; preds = %for.body13
  store double 0.000000e+00, ptr %smc_deltaE82, align 8
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb34, %sw.bb
  call void @llvm.lifetime.start.p0(i64 8, ptr %newx83)
  %x = getelementptr inbounds %struct.Monomer, ptr %mybead80, i64 0, i32 0
  %i80 = load double, ptr %x, align 8
  %i81 = load double, ptr %dx75, align 8
  %add36 = fadd fast double %i80, %i81
  store double %add36, ptr %newx83, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %newy84)
  %y = getelementptr inbounds %struct.Monomer, ptr %mybead80, i64 0, i32 1
  %i83 = load double, ptr %y, align 8
  %i84 = load double, ptr %dy76, align 8
  %add37 = fadd fast double %i83, %i84
  store double %add37, ptr %newy84, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %newz85)
  %z = getelementptr inbounds %struct.Monomer, ptr %mybead80, i64 0, i32 2
  %i86 = load double, ptr %z, align 8
  %i87 = load double, ptr %dz77, align 8
  %add38 = fadd fast double %i86, %i87
  store double %add38, ptr %newz85, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %move_allowed86)
  %i89 = load ptr, ptr %p.addr88, align 8
  %i90 = load double, ptr %x, align 8
  %i91 = load double, ptr %y, align 8
  %i92 = load double, ptr %z, align 8
  %i93 = load double, ptr %dx75, align 8
  %i94 = load double, ptr %dy76, align 8
  %i95 = load double, ptr %dz77, align 8
  %nonexact_area51_flag = getelementptr inbounds %struct.Phase, ptr %i89, i64 0, i32 46, i32 23
  %i96 = load i32, ptr %nonexact_area51_flag, align 8
  %call43 = call i32 @possible_move_area51(ptr %i89, double %i90, double %i91, double %i92, double %i93, double %i94, double %i95, i32 %i96)
  store i32 %call43, ptr %move_allowed86, align 4
  %tobool = icmp eq i32 %call43, 0
  br i1 %tobool, label %if.end55, label %if.then

if.then:                                          ; preds = %sw.epilog
  %i97 = load ptr, ptr %p.addr88, align 8
  %i98 = load i64, ptr %npoly67, align 8
  %i99 = load i32, ptr %ibead78, align 4
  %i100 = load double, ptr %dx75, align 8
  %i101 = load double, ptr %dy76, align 8
  %i102 = load double, ptr %dz77, align 8
  %i103 = load i32, ptr %iwtype79, align 4
  %call44 = call fast double @calc_delta_energy(ptr %i97, i64 %i98, ptr %mybead80, i32 %i99, double %i100, double %i101, double %i102, i32 %i103)
  store double %call44, ptr %delta_energy74, align 8
  %i104 = load double, ptr %smc_deltaE82, align 8
  %add45 = fadd fast double %call44, %i104
  store double %add45, ptr %delta_energy74, align 8
  %i105 = load ptr, ptr %myrngstate71, align 8
  %i106 = load i32, ptr %arg_rng_type72, align 4
  %call46 = call zeroext i1 @som_accept(ptr %i105, i32 %i106, double %add45)
  br i1 %call46, label %if.then50, label %if.end55

if.then50:                                        ; preds = %if.then
  %i107 = load double, ptr %newx83, align 8
  %i108 = load ptr, ptr %mybead_ptr81, align 8
  %x51 = getelementptr inbounds %struct.Monomer, ptr %i108, i64 0, i32 0
  store double %i107, ptr %x51, align 8
  %i109 = load double, ptr %newy84, align 8
  %i110 = load ptr, ptr %mybead_ptr81, align 8
  %y52 = getelementptr inbounds %struct.Monomer, ptr %i110, i64 0, i32 1
  store double %i109, ptr %y52, align 8
  %i111 = load double, ptr %newz85, align 8
  %i112 = load ptr, ptr %mybead_ptr81, align 8
  %z53 = getelementptr inbounds %struct.Monomer, ptr %i112, i64 0, i32 2
  store double %i111, ptr %z53, align 8
  %i113 = load i32, ptr %accepted_moves_loc68, align 4
  %add54 = add i32 %i113, 1
  store i32 %add54, ptr %accepted_moves_loc68, align 4
  br label %if.end55

if.end55:                                         ; preds = %if.then50, %if.then, %sw.epilog
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %move_allowed86)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %newz85)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %newy84)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %newx83)
  call void @llvm.lifetime.end.p0(i64 8, ptr %smc_deltaE82)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %mybead_ptr81)
  call void @llvm.lifetime.end.p0(i64 24, ptr %mybead80)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %iwtype79)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %ibead78)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %delta_energy74)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %dz77)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %dy76)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %dx75)
  %i114 = load i32, ptr %nmc73, align 4
  %inc = add i32 %i114, 1
  br label %for.cond11

DIR.OMP.END.PARALLEL.LOOP.4:                      ; preds = %omp.inner.for.cond, %DIR.OMP.PARALLEL.LOOP.397
  call void @llvm.directive.region.exit(token %i6) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.PARALLEL.LOOP.4, %for.body
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %.omp.ub)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %.omp.lb)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %.omp.iv)
  %i118 = load ptr, ptr %p.addr, align 8
  %time = getelementptr inbounds %struct.Phase, ptr %i118, i64 0, i32 24
  %i119 = load i32, ptr %time, align 8
  %add58 = add i32 %i119, 1
  store i32 %add58, ptr %time, align 8
  %num_all_beads_local = getelementptr inbounds %struct.Phase, ptr %i118, i64 0, i32 26
  %i120 = load i64, ptr %num_all_beads_local, align 8
  %n_moves = getelementptr inbounds %struct.Phase, ptr %i118, i64 0, i32 37
  %i121 = load i64, ptr %n_moves, align 8
  %add59 = add i64 %i121, %i120
  store i64 %add59, ptr %n_moves, align 8
  %i122 = load i32, ptr %n_accepts, align 4
  %conv60 = zext i32 %i122 to i64
  %i123 = load ptr, ptr %p.addr, align 8
  %n_accepts61 = getelementptr inbounds %struct.Phase, ptr %i123, i64 0, i32 38
  %i124 = load i64, ptr %n_accepts61, align 8
  %add62 = add i64 %i124, %conv60
  store i64 %add62, ptr %n_accepts61, align 8
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %n_accepts)
  %inc64 = add i32 %step.0, 1
  br label %for.cond

for.end65:                                        ; preds = %for.cond
  ret i32 0
}

define i32 @mc_set_iteration(ptr %p, i32 %nsteps, i32 %tuning_parameter) local_unnamed_addr #1 {
entry:
  %p.addr = alloca ptr, align 8
  %my_rng_type = alloca i32, align 4
  %nonexact_area51 = alloca i32, align 4
  %cleanup.dest.slot = alloca i32, align 4
  %n_accepts = alloca i32, align 4
  %.omp.iv = alloca i64, align 8
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %npoly = alloca i64, align 8
  %accepted_moves_poly = alloca i32, align 4
  %poly_arch = alloca ptr, align 8
  %poly_type_offset = alloca ptr, align 8
  %mypoly = alloca ptr, align 8
  %poly_type = alloca i32, align 4
  %myN = alloca i32, align 4
  %mySets = alloca %struct.IndependetSets, align 8
  %n_sets = alloca i32, align 4
  %set_length = alloca ptr, align 8
  %sets26 = alloca ptr, align 8
  %max_member = alloca i32, align 4
  %set_states = alloca ptr, align 8
  %set_permutation = alloca ptr, align 8
  %i = alloca i32, align 4
  %d = alloca i32, align 4
  %iSet = alloca i32, align 4
  %accepted_moves_set = alloca i32, align 4
  %set_id = alloca i32, align 4
  %len = alloca i32, align 4
  %iP = alloca i32, align 4
  %ibead = alloca i32, align 4
  %iwtype = alloca i32, align 4
  %my_state = alloca %struct.RNG_STATE, align 8
  %mybead = alloca %struct.Monomer, align 8
  %dx = alloca %struct.Monomer, align 8
  %smc_deltaE = alloca double, align 8
  %move_allowed = alloca i32, align 4
  %delta_energy = alloca double, align 8
  %newx = alloca %struct.Monomer, align 8
  store ptr %p, ptr %p.addr, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %my_rng_type)
  %pseudo_random_number_generator_arg = getelementptr inbounds %struct.Phase, ptr %p, i64 0, i32 46, i32 17
  %i2 = load i32, ptr %pseudo_random_number_generator_arg, align 8
  store i32 %i2, ptr %my_rng_type, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %nonexact_area51)
  %i4 = load ptr, ptr %p.addr, align 8
  %nonexact_area51_flag = getelementptr inbounds %struct.Phase, ptr %i4, i64 0, i32 46, i32 23
  %i5 = load i32, ptr %nonexact_area51_flag, align 8
  store i32 %i5, ptr %nonexact_area51, align 4
  %npoly.addr = alloca ptr, align 8
  %accepted_moves_poly.addr = alloca ptr, align 8
  %poly_arch.addr = alloca ptr, align 8
  %poly_type_offset.addr = alloca ptr, align 8
  %mypoly.addr = alloca ptr, align 8
  %poly_type.addr = alloca ptr, align 8
  %myN.addr = alloca ptr, align 8
  %mySets.addr = alloca ptr, align 8
  %n_sets.addr = alloca ptr, align 8
  %set_length.addr = alloca ptr, align 8
  %sets26.addr = alloca ptr, align 8
  %max_member.addr = alloca ptr, align 8
  %set_states.addr = alloca ptr, align 8
  %set_permutation.addr = alloca ptr, align 8
  %i.addr = alloca ptr, align 8
  %d.addr = alloca ptr, align 8
  %iSet.addr = alloca ptr, align 8
  %accepted_moves_set.addr = alloca ptr, align 8
  %set_id.addr = alloca ptr, align 8
  %len.addr = alloca ptr, align 8
  %iP.addr = alloca ptr, align 8
  %ibead.addr = alloca ptr, align 8
  %iwtype.addr = alloca ptr, align 8
  %my_state.addr = alloca ptr, align 8
  %mybead.addr = alloca ptr, align 8
  %dx.addr = alloca ptr, align 8
  %smc_deltaE.addr = alloca ptr, align 8
  %move_allowed.addr = alloca ptr, align 8
  %delta_energy.addr = alloca ptr, align 8
  %newx.addr = alloca ptr, align 8
  %cleanup.dest.slot.addr = alloca ptr, align 8
  %.omp.lb.addr = alloca ptr, align 8
  %p.addr.addr = alloca ptr, align 8
  %my_rng_type.addr = alloca ptr, align 8
  %nonexact_area51.addr = alloca ptr, align 8
  %n_accepts.addr = alloca ptr, align 8
  br label %for.cond

for.cond:                                         ; preds = %omp.precond.end, %entry
  %step.0 = phi i32 [ 0, %entry ], [ %inc134, %omp.precond.end ]
  %cmp = icmp ult i32 %step.0, %nsteps
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  store i32 1, ptr %cleanup.dest.slot, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %nonexact_area51)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %my_rng_type)
  ret i32 0

for.body:                                         ; preds = %for.cond
  %i6 = load ptr, ptr %p.addr, align 8
  %n_polymers2 = getelementptr inbounds %struct.Phase, ptr %i6, i64 0, i32 5
  %i7 = load i64, ptr %n_polymers2, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr nonnull %n_accepts)
  store i32 0, ptr %n_accepts, align 4
  %cmp6 = icmp eq i64 %i7, 0
  br i1 %cmp6, label %omp.precond.end, label %DIR.OMP.PARALLEL.LOOP.3181

DIR.OMP.PARALLEL.LOOP.3181:                       ; preds = %for.body
  %sub3 = add i64 %i7, -1
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %.omp.iv)
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %.omp.lb)
  store i64 0, ptr %.omp.lb, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %.omp.ub)
  store volatile i64 %sub3, ptr %.omp.ub, align 8
  store ptr %npoly, ptr %npoly.addr, align 8
  store ptr %accepted_moves_poly, ptr %accepted_moves_poly.addr, align 8
  store ptr %poly_arch, ptr %poly_arch.addr, align 8
  store ptr %poly_type_offset, ptr %poly_type_offset.addr, align 8
  store ptr %mypoly, ptr %mypoly.addr, align 8
  store ptr %poly_type, ptr %poly_type.addr, align 8
  store ptr %myN, ptr %myN.addr, align 8
  store ptr %mySets, ptr %mySets.addr, align 8
  store ptr %n_sets, ptr %n_sets.addr, align 8
  store ptr %set_length, ptr %set_length.addr, align 8
  store ptr %sets26, ptr %sets26.addr, align 8
  store ptr %max_member, ptr %max_member.addr, align 8
  store ptr %set_states, ptr %set_states.addr, align 8
  store ptr %set_permutation, ptr %set_permutation.addr, align 8
  store ptr %i, ptr %i.addr, align 8
  store ptr %d, ptr %d.addr, align 8
  store ptr %iSet, ptr %iSet.addr, align 8
  store ptr %accepted_moves_set, ptr %accepted_moves_set.addr, align 8
  store ptr %set_id, ptr %set_id.addr, align 8
  store ptr %len, ptr %len.addr, align 8
  store ptr %iP, ptr %iP.addr, align 8
  store ptr %ibead, ptr %ibead.addr, align 8
  store ptr %iwtype, ptr %iwtype.addr, align 8
  store ptr %my_state, ptr %my_state.addr, align 8
  store ptr %mybead, ptr %mybead.addr, align 8
  store ptr %dx, ptr %dx.addr, align 8
  store ptr %smc_deltaE, ptr %smc_deltaE.addr, align 8
  store ptr %move_allowed, ptr %move_allowed.addr, align 8
  store ptr %delta_energy, ptr %delta_energy.addr, align 8
  store ptr %newx, ptr %newx.addr, align 8
  store ptr %cleanup.dest.slot, ptr %cleanup.dest.slot.addr, align 8
  store ptr %.omp.lb, ptr %.omp.lb.addr, align 8
  store ptr %p.addr, ptr %p.addr.addr, align 8
  store ptr %my_rng_type, ptr %my_rng_type.addr, align 8
  store ptr %nonexact_area51, ptr %nonexact_area51.addr, align 8
  store ptr %n_accepts, ptr %n_accepts.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %i12 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(ptr %n_accepts), "QUAL.OMP.SHARED"(ptr %p.addr), "QUAL.OMP.SHARED"(ptr %my_rng_type), "QUAL.OMP.SHARED"(ptr %nonexact_area51), "QUAL.OMP.NORMALIZED.IV"(ptr %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(ptr %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(ptr %.omp.ub), "QUAL.OMP.PRIVATE"(ptr %npoly), "QUAL.OMP.PRIVATE"(ptr %accepted_moves_poly), "QUAL.OMP.PRIVATE"(ptr %poly_arch), "QUAL.OMP.PRIVATE"(ptr %poly_type_offset), "QUAL.OMP.PRIVATE"(ptr %mypoly), "QUAL.OMP.PRIVATE"(ptr %poly_type), "QUAL.OMP.PRIVATE"(ptr %myN), "QUAL.OMP.PRIVATE"(ptr %mySets), "QUAL.OMP.PRIVATE"(ptr %n_sets), "QUAL.OMP.PRIVATE"(ptr %set_length), "QUAL.OMP.PRIVATE"(ptr %sets26), "QUAL.OMP.PRIVATE"(ptr %max_member), "QUAL.OMP.PRIVATE"(ptr %set_states), "QUAL.OMP.PRIVATE"(ptr %set_permutation), "QUAL.OMP.PRIVATE"(ptr %i), "QUAL.OMP.PRIVATE"(ptr %d), "QUAL.OMP.PRIVATE"(ptr %iSet), "QUAL.OMP.PRIVATE"(ptr %accepted_moves_set), "QUAL.OMP.PRIVATE"(ptr %set_id), "QUAL.OMP.PRIVATE"(ptr %len), "QUAL.OMP.PRIVATE"(ptr %iP), "QUAL.OMP.PRIVATE"(ptr %ibead), "QUAL.OMP.PRIVATE"(ptr %iwtype), "QUAL.OMP.PRIVATE"(ptr %my_state), "QUAL.OMP.PRIVATE"(ptr %mybead), "QUAL.OMP.PRIVATE"(ptr %dx), "QUAL.OMP.PRIVATE"(ptr %smc_deltaE), "QUAL.OMP.PRIVATE"(ptr %move_allowed), "QUAL.OMP.PRIVATE"(ptr %delta_energy), "QUAL.OMP.PRIVATE"(ptr %newx), "QUAL.OMP.PRIVATE"(ptr %cleanup.dest.slot), "QUAL.OMP.OPERAND.ADDR"(ptr %npoly, ptr %npoly.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %accepted_moves_poly, ptr %accepted_moves_poly.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %poly_arch, ptr %poly_arch.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %poly_type_offset, ptr %poly_type_offset.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %mypoly, ptr %mypoly.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %poly_type, ptr %poly_type.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %myN, ptr %myN.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %mySets, ptr %mySets.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %n_sets, ptr %n_sets.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %set_length, ptr %set_length.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %sets26, ptr %sets26.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %max_member, ptr %max_member.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %set_states, ptr %set_states.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %set_permutation, ptr %set_permutation.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %i, ptr %i.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %d, ptr %d.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %iSet, ptr %iSet.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %accepted_moves_set, ptr %accepted_moves_set.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %set_id, ptr %set_id.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %len, ptr %len.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %iP, ptr %iP.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %ibead, ptr %ibead.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %iwtype, ptr %iwtype.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %my_state, ptr %my_state.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %mybead, ptr %mybead.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %dx, ptr %dx.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %smc_deltaE, ptr %smc_deltaE.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %move_allowed, ptr %move_allowed.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %delta_energy, ptr %delta_energy.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %newx, ptr %newx.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %cleanup.dest.slot, ptr %cleanup.dest.slot.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %.omp.lb, ptr %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %p.addr, ptr %p.addr.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %my_rng_type, ptr %my_rng_type.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %nonexact_area51, ptr %nonexact_area51.addr), "QUAL.OMP.OPERAND.ADDR"(ptr %n_accepts, ptr %n_accepts.addr), "QUAL.OMP.JUMP.TO.END.IF"(ptr %end.dir.temp) ]
  %temp.load = load volatile i1, ptr %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.PARALLEL.LOOP.4, label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.3181
  %npoly137 = load volatile ptr, ptr %npoly.addr, align 8
  %accepted_moves_poly138 = load volatile ptr, ptr %accepted_moves_poly.addr, align 8
  %poly_arch139 = load volatile ptr, ptr %poly_arch.addr, align 8
  %poly_type_offset140 = load volatile ptr, ptr %poly_type_offset.addr, align 8
  %mypoly141 = load volatile ptr, ptr %mypoly.addr, align 8
  %poly_type142 = load volatile ptr, ptr %poly_type.addr, align 8
  %myN143 = load volatile ptr, ptr %myN.addr, align 8
  %mySets144 = load volatile ptr, ptr %mySets.addr, align 8
  %n_sets145 = load volatile ptr, ptr %n_sets.addr, align 8
  %set_length146 = load volatile ptr, ptr %set_length.addr, align 8
  %sets26147 = load volatile ptr, ptr %sets26.addr, align 8
  %max_member148 = load volatile ptr, ptr %max_member.addr, align 8
  %set_states149 = load volatile ptr, ptr %set_states.addr, align 8
  %set_permutation150 = load volatile ptr, ptr %set_permutation.addr, align 8
  %i151 = load volatile ptr, ptr %i.addr, align 8
  %d152 = load volatile ptr, ptr %d.addr, align 8
  %iSet153 = load volatile ptr, ptr %iSet.addr, align 8
  %accepted_moves_set154 = load volatile ptr, ptr %accepted_moves_set.addr, align 8
  %set_id155 = load volatile ptr, ptr %set_id.addr, align 8
  %len156 = load volatile ptr, ptr %len.addr, align 8
  %iP157 = load volatile ptr, ptr %iP.addr, align 8
  %ibead158 = load volatile ptr, ptr %ibead.addr, align 8
  %iwtype159 = load volatile ptr, ptr %iwtype.addr, align 8
  %my_state160 = load volatile ptr, ptr %my_state.addr, align 8
  %mybead161 = load volatile ptr, ptr %mybead.addr, align 8
  %dx162 = load volatile ptr, ptr %dx.addr, align 8
  %smc_deltaE163 = load volatile ptr, ptr %smc_deltaE.addr, align 8
  %move_allowed164 = load volatile ptr, ptr %move_allowed.addr, align 8
  %delta_energy165 = load volatile ptr, ptr %delta_energy.addr, align 8
  %newx166 = load volatile ptr, ptr %newx.addr, align 8
  %cleanup.dest.slot167 = load volatile ptr, ptr %cleanup.dest.slot.addr, align 8
  %.omp.lb168 = load volatile ptr, ptr %.omp.lb.addr, align 8
  %p.addr169 = load volatile ptr, ptr %p.addr.addr, align 8
  %my_rng_type170 = load volatile ptr, ptr %my_rng_type.addr, align 8
  %nonexact_area51171 = load volatile ptr, ptr %nonexact_area51.addr, align 8
  %n_accepts172 = load volatile ptr, ptr %n_accepts.addr, align 8
  %i13 = load i64, ptr %.omp.lb168, align 8
  store volatile i64 %i13, ptr %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %for.cond.cleanup44, %DIR.OMP.PARALLEL.LOOP.3
  %i14 = load volatile i64, ptr %.omp.iv, align 8
  %i15 = load volatile i64, ptr %.omp.ub, align 8
  %add7 = add i64 %i15, 1
  %cmp8 = icmp ult i64 %i14, %add7
  br i1 %cmp8, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.4

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  call void @llvm.lifetime.start.p0(i64 8, ptr %npoly137)
  %i17 = load volatile i64, ptr %.omp.iv, align 8
  store i64 %i17, ptr %npoly137, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %accepted_moves_poly138)
  store i32 0, ptr %accepted_moves_poly138, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr %poly_arch139)
  %i20 = load ptr, ptr %p.addr169, align 8
  %poly_arch11 = getelementptr inbounds %struct.Phase, ptr %i20, i64 0, i32 45
  %i21 = load ptr, ptr %poly_arch11, align 8
  store ptr %i21, ptr %poly_arch139, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %poly_type_offset140)
  %i23 = load ptr, ptr %p.addr169, align 8
  %poly_type_offset12 = getelementptr inbounds %struct.Phase, ptr %i23, i64 0, i32 43
  %i24 = load ptr, ptr %poly_type_offset12, align 8
  store ptr %i24, ptr %poly_type_offset140, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %mypoly141)
  %i26 = load ptr, ptr %p.addr169, align 8
  %polymers = getelementptr inbounds %struct.Phase, ptr %i26, i64 0, i32 8
  %i27 = load ptr, ptr %polymers, align 8
  %i28 = load i64, ptr %npoly137, align 8
  %ptridx = getelementptr inbounds %struct.Polymer, ptr %i27, i64 %i28
  store ptr %ptridx, ptr %mypoly141, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %poly_type142)
  %i30 = load ptr, ptr %mypoly141, align 8
  %type = getelementptr inbounds %struct.Polymer, ptr %i30, i64 0, i32 2
  %i31 = load i32, ptr %type, align 8
  store i32 %i31, ptr %poly_type142, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %myN143)
  %i33 = load ptr, ptr %p.addr169, align 8
  %poly_arch13 = getelementptr inbounds %struct.Phase, ptr %i33, i64 0, i32 45
  %i34 = load ptr, ptr %poly_arch13, align 8
  %poly_type_offset14 = getelementptr inbounds %struct.Phase, ptr %i33, i64 0, i32 43
  %i35 = load ptr, ptr %poly_type_offset14, align 8
  %i36 = load ptr, ptr %mypoly141, align 8
  %type15 = getelementptr inbounds %struct.Polymer, ptr %i36, i64 0, i32 2
  %i37 = load i32, ptr %type15, align 8
  %idxprom = zext i32 %i37 to i64
  %ptridx16 = getelementptr inbounds i32, ptr %i35, i64 %idxprom
  %i38 = load i32, ptr %ptridx16, align 4
  %idxprom17 = sext i32 %i38 to i64
  %ptridx18 = getelementptr inbounds i32, ptr %i34, i64 %idxprom17
  %i39 = load i32, ptr %ptridx18, align 4
  store i32 %i39, ptr %myN143, align 4
  call void @llvm.lifetime.start.p0(i64 24, ptr %mySets144)
  %i41 = load ptr, ptr %p.addr169, align 8
  %sets = getelementptr inbounds %struct.Phase, ptr %i41, i64 0, i32 52
  %i42 = load ptr, ptr %sets, align 8
  %i43 = load ptr, ptr %mypoly141, align 8
  %type21 = getelementptr inbounds %struct.Polymer, ptr %i43, i64 0, i32 2
  %i44 = load i32, ptr %type21, align 8
  %idxprom22 = zext i32 %i44 to i64
  %ptridx23 = getelementptr inbounds %struct.IndependetSets, ptr %i42, i64 %idxprom22
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 dereferenceable(24) %mySets144, ptr nonnull align 8 dereferenceable(24) %ptridx23, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 4, ptr %n_sets145)
  %n_sets24 = getelementptr inbounds %struct.IndependetSets, ptr %mySets144, i64 0, i32 0
  %i47 = load i32, ptr %n_sets24, align 8
  store i32 %i47, ptr %n_sets145, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr %set_length146)
  %set_length25 = getelementptr inbounds %struct.IndependetSets, ptr %mySets144, i64 0, i32 2
  %i49 = load ptr, ptr %set_length25, align 8
  store ptr %i49, ptr %set_length146, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %sets26147)
  %sets27 = getelementptr inbounds %struct.IndependetSets, ptr %mySets144, i64 0, i32 3
  %i51 = load ptr, ptr %sets27, align 8
  store ptr %i51, ptr %sets26147, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %max_member148)
  %max_member28 = getelementptr inbounds %struct.IndependetSets, ptr %mySets144, i64 0, i32 1
  %i53 = load i32, ptr %max_member28, align 4
  store i32 %i53, ptr %max_member148, align 4
  call void @llvm.lifetime.start.p0(i64 8, ptr %set_states149)
  %i55 = load ptr, ptr %mypoly141, align 8
  %set_states29 = getelementptr inbounds %struct.Polymer, ptr %i55, i64 0, i32 4
  %i56 = load ptr, ptr %set_states29, align 8
  store ptr %i56, ptr %set_states149, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %set_permutation150)
  %i58 = load ptr, ptr %mypoly141, align 8
  %set_permutation30 = getelementptr inbounds %struct.Polymer, ptr %i58, i64 0, i32 5
  %i59 = load ptr, ptr %set_permutation30, align 8
  store ptr %i59, ptr %set_permutation150, align 8
  call void @llvm.lifetime.start.p0(i64 4, ptr %i151)
  br label %for.cond31

for.cond31:                                       ; preds = %for.body34, %omp.inner.for.body
  %storemerge = phi i32 [ 0, %omp.inner.for.body ], [ %inc, %for.body34 ]
  store i32 %storemerge, ptr %i151, align 4
  %i61 = load i32, ptr %n_sets145, align 4
  %cmp32 = icmp ult i32 %storemerge, %i61
  br i1 %cmp32, label %for.body34, label %for.cond.cleanup33

for.cond.cleanup33:                               ; preds = %for.cond31
  store i32 9, ptr %cleanup.dest.slot167, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %i151)
  call void @llvm.lifetime.start.p0(i64 4, ptr %iSet153)
  br label %for.cond42

for.body34:                                       ; preds = %for.cond31
  call void @llvm.lifetime.start.p0(i64 4, ptr %d152)
  %i64 = load ptr, ptr %mypoly141, align 8
  %poly_state = getelementptr inbounds %struct.Polymer, ptr %i64, i64 0, i32 3
  %i65 = load i32, ptr %my_rng_type170, align 4
  %call = call i32 @soma_rng_uint(ptr nonnull %poly_state, i32 %i65)
  %i66 = load i32, ptr %i151, align 4
  %add35 = add i32 %i66, 1
  %rem = urem i32 %call, %add35
  store i32 %rem, ptr %d152, align 4
  %i67 = load ptr, ptr %set_permutation150, align 8
  %idxprom36 = zext i32 %rem to i64
  %ptridx37 = getelementptr inbounds i32, ptr %i67, i64 %idxprom36
  %i68 = load i32, ptr %ptridx37, align 4
  %i69 = load i32, ptr %i151, align 4
  %idxprom38 = zext i32 %i69 to i64
  %ptridx39 = getelementptr inbounds i32, ptr %i67, i64 %idxprom38
  store i32 %i68, ptr %ptridx39, align 4
  %i70 = load i32, ptr %i151, align 4
  %i71 = load ptr, ptr %set_permutation150, align 8
  %i72 = load i32, ptr %d152, align 4
  %idxprom40 = zext i32 %i72 to i64
  %ptridx41 = getelementptr inbounds i32, ptr %i71, i64 %idxprom40
  store i32 %i70, ptr %ptridx41, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %d152)
  %i73 = load i32, ptr %i151, align 4
  %inc = add i32 %i73, 1
  br label %for.cond31

for.cond42:                                       ; preds = %for.cond.cleanup52, %for.cond.cleanup33
  %storemerge182 = phi i32 [ 0, %for.cond.cleanup33 ], [ %inc124, %for.cond.cleanup52 ]
  store i32 %storemerge182, ptr %iSet153, align 4
  %i74 = load i32, ptr %n_sets145, align 4
  %cmp43 = icmp ult i32 %storemerge182, %i74
  br i1 %cmp43, label %for.body45, label %for.cond.cleanup44

for.cond.cleanup44:                               ; preds = %for.cond42
  store i32 12, ptr %cleanup.dest.slot167, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %iSet153)
  %i75 = load i32, ptr %accepted_moves_poly138, align 4
  %i76 = load i32, ptr %n_accepts172, align 4
  %add126 = add i32 %i76, %i75
  store i32 %add126, ptr %n_accepts172, align 4
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %set_permutation150)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %set_states149)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %max_member148)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %sets26147)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %set_length146)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %n_sets145)
  call void @llvm.lifetime.end.p0(i64 24, ptr %mySets144)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %myN143)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %poly_type142)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %mypoly141)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %poly_type_offset140)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %poly_arch139)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %accepted_moves_poly138)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %npoly137)
  %i77 = load volatile i64, ptr %.omp.iv, align 8
  %add127 = add nuw i64 %i77, 1
  store volatile i64 %add127, ptr %.omp.iv, align 8
  br label %omp.inner.for.cond

for.body45:                                       ; preds = %for.cond42
  call void @llvm.lifetime.start.p0(i64 4, ptr %accepted_moves_set154)
  store i32 0, ptr %accepted_moves_set154, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %set_id155)
  %i80 = load ptr, ptr %set_permutation150, align 8
  %i81 = load i32, ptr %iSet153, align 4
  %idxprom46 = zext i32 %i81 to i64
  %ptridx47 = getelementptr inbounds i32, ptr %i80, i64 %idxprom46
  %i82 = load i32, ptr %ptridx47, align 4
  store i32 %i82, ptr %set_id155, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %len156)
  %i84 = load ptr, ptr %set_length146, align 8
  %i85 = load i32, ptr %set_id155, align 4
  %idxprom48 = zext i32 %i85 to i64
  %ptridx49 = getelementptr inbounds i32, ptr %i84, i64 %idxprom48
  %i86 = load i32, ptr %ptridx49, align 4
  store i32 %i86, ptr %len156, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %iP157)
  br label %for.cond50

for.cond50:                                       ; preds = %if.end115, %for.body45
  %storemerge183 = phi i32 [ 0, %for.body45 ], [ %inc120, %if.end115 ]
  store i32 %storemerge183, ptr %iP157, align 4
  %i88 = load i32, ptr %len156, align 4
  %cmp51 = icmp ult i32 %storemerge183, %i88
  br i1 %cmp51, label %for.body53, label %for.cond.cleanup52

for.cond.cleanup52:                               ; preds = %for.cond50
  store i32 15, ptr %cleanup.dest.slot167, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %iP157)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %len156)
  %i89 = load i32, ptr %accepted_moves_set154, align 4
  %i90 = load i32, ptr %accepted_moves_poly138, align 4
  %add122 = add i32 %i90, %i89
  store i32 %add122, ptr %accepted_moves_poly138, align 4
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %set_id155)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %accepted_moves_set154)
  %i91 = load i32, ptr %iSet153, align 4
  %inc124 = add i32 %i91, 1
  br label %for.cond42

for.body53:                                       ; preds = %for.cond50
  call void @llvm.lifetime.start.p0(i64 4, ptr %ibead158)
  %i93 = load ptr, ptr %sets26147, align 8
  %i94 = load i32, ptr %set_id155, align 4
  %i95 = load i32, ptr %max_member148, align 4
  %mul54 = mul i32 %i94, %i95
  %i96 = load i32, ptr %iP157, align 4
  %add55 = add i32 %mul54, %i96
  %idxprom56 = zext i32 %add55 to i64
  %ptridx57 = getelementptr inbounds i32, ptr %i93, i64 %idxprom56
  %i97 = load i32, ptr %ptridx57, align 4
  store i32 %i97, ptr %ibead158, align 4
  call void @llvm.lifetime.start.p0(i64 4, ptr %iwtype159)
  %i99 = load ptr, ptr %poly_arch139, align 8
  %i100 = load ptr, ptr %poly_type_offset140, align 8
  %i101 = load i32, ptr %poly_type142, align 4
  %idxprom58 = zext i32 %i101 to i64
  %ptridx59 = getelementptr inbounds i32, ptr %i100, i64 %idxprom58
  %i102 = load i32, ptr %ptridx59, align 4
  %add60 = add nsw i32 %i102, 1
  %i103 = load i32, ptr %ibead158, align 4
  %add61 = add i32 %add60, %i103
  %idxprom62 = zext i32 %add61 to i64
  %ptridx63 = getelementptr inbounds i32, ptr %i99, i64 %idxprom62
  %i104 = load i32, ptr %ptridx63, align 4
  %call64 = call i32 @get_particle_type(i32 %i104)
  store i32 %call64, ptr %iwtype159, align 4
  call void @llvm.lifetime.start.p0(i64 32, ptr %my_state160)
  %i106 = load ptr, ptr %set_states149, align 8
  %i107 = load i32, ptr %iP157, align 4
  %idxprom65 = zext i32 %i107 to i64
  %ptridx66 = getelementptr inbounds %struct.RNG_STATE, ptr %i106, i64 %idxprom65
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 dereferenceable(32) %my_state160, ptr nonnull align 8 dereferenceable(32) %ptridx66, i64 32, i1 false)
  call void @llvm.lifetime.start.p0(i64 24, ptr %mybead161)
  %i110 = load ptr, ptr %p.addr169, align 8
  %allocator = getelementptr inbounds %struct.Phase, ptr %i110, i64 0, i32 59
  %i111 = load ptr, ptr %allocator, align 8
  %device_buf = getelementptr inbounds %struct.Allocator, ptr %i111, i64 0, i32 0, i32 3
  %i112 = load ptr, ptr %device_buf, align 8
  %i113 = load ptr, ptr %mypoly141, align 8
  %beads = getelementptr inbounds %struct.Polymer, ptr %i113, i64 0, i32 0
  %i114 = load i64, ptr %beads, align 8
  %i115 = load i32, ptr %ibead158, align 4
  %conv = zext i32 %i115 to i64
  %add67 = add i64 %i114, %conv
  %ptridx68 = getelementptr inbounds %struct.Monomer, ptr %i112, i64 %add67
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 dereferenceable(24) %mybead161, ptr nonnull align 8 dereferenceable(24) %ptridx68, i64 24, i1 false)
  call void @llvm.lifetime.start.p0(i64 24, ptr %dx162)
  %z = getelementptr inbounds %struct.Monomer, ptr %dx162, i64 0, i32 2
  store double 0.000000e+00, ptr %z, align 8
  %y = getelementptr inbounds %struct.Monomer, ptr %dx162, i64 0, i32 1
  store double 0.000000e+00, ptr %y, align 8
  %x = getelementptr inbounds %struct.Monomer, ptr %dx162, i64 0, i32 0
  store double 0.000000e+00, ptr %x, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr %smc_deltaE163)
  store double 0.000000e+00, ptr %smc_deltaE163, align 8
  %i119 = load ptr, ptr %p.addr169, align 8
  %move_type_arg = getelementptr inbounds %struct.Phase, ptr %i119, i64 0, i32 46, i32 25
  %i120 = load i32, ptr %move_type_arg, align 8
  switch i32 %i120, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb73
  ]

sw.bb:                                            ; preds = %for.body53
  %i121 = load i32, ptr %ibead158, align 4
  %i122 = load i32, ptr %iwtype159, align 4
  %i123 = load i32, ptr %my_rng_type170, align 4
  call void @trial_move(ptr %i119, i64 undef, i32 %i121, ptr nonnull %x, ptr nonnull %y, ptr nonnull %z, i32 %i122, i32 %i123, ptr %my_state160)
  store double 0.000000e+00, ptr %smc_deltaE163, align 8
  br label %sw.epilog

sw.bb73:                                          ; preds = %for.body53
  %i124 = load i64, ptr %npoly137, align 8
  %i125 = load i32, ptr %ibead158, align 4
  %i126 = load i32, ptr %my_rng_type170, align 4
  %i127 = load i32, ptr %iwtype159, align 4
  call void @trial_move_smc(ptr %i119, i64 %i124, i32 %i125, ptr nonnull %x, ptr nonnull %y, ptr nonnull %z, ptr nonnull %smc_deltaE163, ptr %mybead161, ptr %my_state160, i32 %i126, i32 %i127)
  br label %sw.epilog

sw.default:                                       ; preds = %for.body53
  store double 0.000000e+00, ptr %smc_deltaE163, align 8
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb73, %sw.bb
  call void @llvm.lifetime.start.p0(i64 4, ptr %move_allowed164)
  %i129 = load ptr, ptr %p.addr169, align 8
  %x78 = getelementptr inbounds %struct.Monomer, ptr %mybead161, i64 0, i32 0
  %i130 = load double, ptr %x78, align 8
  %y79 = getelementptr inbounds %struct.Monomer, ptr %mybead161, i64 0, i32 1
  %i131 = load double, ptr %y79, align 8
  %z80 = getelementptr inbounds %struct.Monomer, ptr %mybead161, i64 0, i32 2
  %i132 = load double, ptr %z80, align 8
  %i133 = load double, ptr %x, align 8
  %i134 = load double, ptr %y, align 8
  %i135 = load double, ptr %z, align 8
  %i136 = load i32, ptr %nonexact_area51171, align 4
  %call84 = call i32 @possible_move_area51(ptr %i129, double %i130, double %i131, double %i132, double %i133, double %i134, double %i135, i32 %i136)
  store i32 %call84, ptr %move_allowed164, align 4
  %tobool = icmp eq i32 %call84, 0
  br i1 %tobool, label %if.end115, label %if.then

if.then:                                          ; preds = %sw.epilog
  call void @llvm.lifetime.start.p0(i64 8, ptr %delta_energy165)
  %i138 = load ptr, ptr %p.addr169, align 8
  %i139 = load i64, ptr %npoly137, align 8
  %i140 = load i32, ptr %ibead158, align 4
  %i141 = load double, ptr %x, align 8
  %i142 = load double, ptr %y, align 8
  %i143 = load double, ptr %z, align 8
  %i144 = load i32, ptr %iwtype159, align 4
  %call88 = call fast double @calc_delta_energy(ptr %i138, i64 %i139, ptr %mybead161, i32 %i140, double %i141, double %i142, double %i143, i32 %i144)
  %i145 = load double, ptr %smc_deltaE163, align 8
  %add89 = fadd fast double %call88, %i145
  store double %add89, ptr %delta_energy165, align 8
  %i146 = load i32, ptr %my_rng_type170, align 4
  %call90 = call zeroext i1 @som_accept(ptr %my_state160, i32 %i146, double %add89)
  br i1 %call90, label %if.then94, label %if.end

if.then94:                                        ; preds = %if.then
  call void @llvm.lifetime.start.p0(i64 24, ptr %newx166)
  %i148 = load double, ptr %x78, align 8
  %i149 = load double, ptr %x, align 8
  %add97 = fadd fast double %i148, %i149
  %x98 = getelementptr inbounds %struct.Monomer, ptr %newx166, i64 0, i32 0
  store double %add97, ptr %x98, align 8
  %i150 = load double, ptr %y79, align 8
  %i152 = load double, ptr %y, align 8
  %add101 = fadd fast double %i150, %i152
  %y102 = getelementptr inbounds %struct.Monomer, ptr %newx166, i64 0, i32 1
  store double %add101, ptr %y102, align 8
  %i153 = load double, ptr %z80, align 8
  %i154 = load double, ptr %z, align 8
  %add105 = fadd fast double %i153, %i154
  %z106 = getelementptr inbounds %struct.Monomer, ptr %newx166, i64 0, i32 2
  store double %add105, ptr %z106, align 8
  %i155 = load ptr, ptr %p.addr169, align 8
  %allocator107 = getelementptr inbounds %struct.Phase, ptr %i155, i64 0, i32 59
  %i156 = load ptr, ptr %allocator107, align 8
  %device_buf109 = getelementptr inbounds %struct.Allocator, ptr %i156, i64 0, i32 0, i32 3
  %i157 = load ptr, ptr %device_buf109, align 8
  %i158 = load ptr, ptr %mypoly141, align 8
  %beads110 = getelementptr inbounds %struct.Polymer, ptr %i158, i64 0, i32 0
  %i159 = load i64, ptr %beads110, align 8
  %i160 = load i32, ptr %ibead158, align 4
  %conv111 = zext i32 %i160 to i64
  %add112 = add i64 %i159, %conv111
  %ptridx113 = getelementptr inbounds %struct.Monomer, ptr %i157, i64 %add112
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 dereferenceable(24) %ptridx113, ptr nonnull align 8 dereferenceable(24) %newx166, i64 24, i1 false)
  %i162 = load i32, ptr %accepted_moves_set154, align 4
  %add114 = add i32 %i162, 1
  store i32 %add114, ptr %accepted_moves_set154, align 4
  call void @llvm.lifetime.end.p0(i64 24, ptr %newx166)
  br label %if.end

if.end:                                           ; preds = %if.then94, %if.then
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %delta_energy165)
  br label %if.end115

if.end115:                                        ; preds = %if.end, %sw.epilog
  %i163 = load ptr, ptr %mypoly141, align 8
  %set_states116 = getelementptr inbounds %struct.Polymer, ptr %i163, i64 0, i32 4
  %i164 = load ptr, ptr %set_states116, align 8
  %i165 = load i32, ptr %iP157, align 4
  %idxprom117 = zext i32 %i165 to i64
  %ptridx118 = getelementptr inbounds %struct.RNG_STATE, ptr %i164, i64 %idxprom117
  call void @llvm.memcpy.p0.p0.i64(ptr nonnull align 8 dereferenceable(32) %ptridx118, ptr nonnull align 8 dereferenceable(32) %my_state160, i64 32, i1 false)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %move_allowed164)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %smc_deltaE163)
  call void @llvm.lifetime.end.p0(i64 24, ptr %dx162)
  call void @llvm.lifetime.end.p0(i64 24, ptr %mybead161)
  call void @llvm.lifetime.end.p0(i64 32, ptr %my_state160)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %iwtype159)
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %ibead158)
  %i167 = load i32, ptr %iP157, align 4
  %inc120 = add i32 %i167, 1
  br label %for.cond50

DIR.OMP.END.PARALLEL.LOOP.4:                      ; preds = %omp.inner.for.cond, %DIR.OMP.PARALLEL.LOOP.3181
  call void @llvm.directive.region.exit(token %i12) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.PARALLEL.LOOP.4, %for.body
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %.omp.ub)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %.omp.lb)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %.omp.iv)
  %i171 = load ptr, ptr %p.addr, align 8
  %time = getelementptr inbounds %struct.Phase, ptr %i171, i64 0, i32 24
  %i172 = load i32, ptr %time, align 8
  %add128 = add i32 %i172, 1
  store i32 %add128, ptr %time, align 8
  %num_all_beads_local = getelementptr inbounds %struct.Phase, ptr %i171, i64 0, i32 26
  %i173 = load i64, ptr %num_all_beads_local, align 8
  %n_moves = getelementptr inbounds %struct.Phase, ptr %i171, i64 0, i32 37
  %i174 = load i64, ptr %n_moves, align 8
  %add129 = add i64 %i174, %i173
  store i64 %add129, ptr %n_moves, align 8
  %i175 = load i32, ptr %n_accepts, align 4
  %conv130 = zext i32 %i175 to i64
  %i176 = load ptr, ptr %p.addr, align 8
  %n_accepts131 = getelementptr inbounds %struct.Phase, ptr %i176, i64 0, i32 38
  %i177 = load i64, ptr %n_accepts131, align 8
  %add132 = add i64 %i177, %conv130
  store i64 %add132, ptr %n_accepts131, align 8
  call void @llvm.lifetime.end.p0(i64 4, ptr nonnull %n_accepts)
  %inc134 = add i32 %step.0, 1
  br label %for.cond
}

define void @trial_move_smc(ptr %p, i64 %ipoly, i32 %ibead, ptr %dx, ptr %dy, ptr %dz, ptr %smc_deltaE, ptr %mybead, ptr %myrngstate, i32 %rng_type, i32 %iwtype) local_unnamed_addr {
entry:
  %fx = alloca double, align 8
  %fy = alloca double, align 8
  %fz = alloca double, align 8
  %rx = alloca double, align 8
  %ry = alloca double, align 8
  %rz = alloca double, align 8
  %nfx = alloca double, align 8
  %nfy = alloca double, align 8
  %nfz = alloca double, align 8
  %x1 = getelementptr inbounds %struct.Monomer, ptr %mybead, i64 0, i32 0
  %i = load double, ptr %x1, align 8
  %y2 = getelementptr inbounds %struct.Monomer, ptr %mybead, i64 0, i32 1
  %i1 = load double, ptr %y2, align 8
  %z3 = getelementptr inbounds %struct.Monomer, ptr %mybead, i64 0, i32 2
  %i2 = load double, ptr %z3, align 8
  %A4 = getelementptr inbounds %struct.Phase, ptr %p, i64 0, i32 18
  %i3 = load ptr, ptr %A4, align 8
  %idxprom = zext i32 %iwtype to i64
  %ptridx = getelementptr inbounds double, ptr %i3, i64 %idxprom
  %i4 = load double, ptr %ptridx, align 8
  %R5 = getelementptr inbounds %struct.Phase, ptr %p, i64 0, i32 19
  %i5 = load ptr, ptr %R5, align 8
  %ptridx7 = getelementptr inbounds double, ptr %i5, i64 %idxprom
  %i6 = load double, ptr %ptridx7, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %fx)
  store double 0.000000e+00, ptr %fx, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %fy)
  store double 0.000000e+00, ptr %fy, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %fz)
  store double 0.000000e+00, ptr %fz, align 8
  call void @add_bond_forces(ptr %p, i64 %ipoly, i32 %ibead, double %i, double %i1, double %i2, ptr nonnull %fx, ptr nonnull %fy, ptr nonnull %fz)
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %rx)
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %ry)
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %rz)
  call void @soma_normal_vector(ptr %myrngstate, i32 %rng_type, ptr nonnull %rx, ptr nonnull %ry, ptr nonnull %rz)
  %i13 = load double, ptr %fx, align 8
  %mul = fmul fast double %i4, %i13
  %i14 = load double, ptr %rx, align 8
  %mul8 = fmul fast double %i14, %i6
  %add = fadd fast double %mul, %mul8
  store double %add, ptr %dx, align 8
  %i15 = load double, ptr %fy, align 8
  %mul9 = fmul fast double %i4, %i15
  %i16 = load double, ptr %ry, align 8
  %mul10 = fmul fast double %i16, %i6
  %add11 = fadd fast double %mul9, %mul10
  store double %add11, ptr %dy, align 8
  %i17 = load double, ptr %fz, align 8
  %mul12 = fmul fast double %i4, %i17
  %i18 = load double, ptr %rz, align 8
  %mul13 = fmul fast double %i18, %i6
  %add14 = fadd fast double %mul12, %mul13
  store double %add14, ptr %dz, align 8
  %i19 = load double, ptr %dx, align 8
  %add15 = fadd fast double %i, %i19
  %i20 = load double, ptr %dy, align 8
  %add16 = fadd fast double %i1, %i20
  %add17 = fadd fast double %i2, %add14
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %nfx)
  store double 0.000000e+00, ptr %nfx, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %nfy)
  store double 0.000000e+00, ptr %nfy, align 8
  call void @llvm.lifetime.start.p0(i64 8, ptr nonnull %nfz)
  store double 0.000000e+00, ptr %nfz, align 8
  call void @add_bond_forces(ptr %p, i64 %ipoly, i32 %ibead, double %add15, double %add16, double %add17, ptr nonnull %nfx, ptr nonnull %nfy, ptr nonnull %nfz)
  store double 0.000000e+00, ptr %smc_deltaE, align 8
  %i24 = load double, ptr %nfx, align 8
  %i25 = load double, ptr %fx, align 8
  %add18 = fadd fast double %i24, %i25
  %i26 = load double, ptr %dx, align 8
  %mul19 = fmul fast double %add18, %i26
  %i27 = load double, ptr %nfy, align 8
  %i28 = load double, ptr %fy, align 8
  %add20 = fadd fast double %i27, %i28
  %i29 = load double, ptr %dy, align 8
  %mul21 = fmul fast double %add20, %i29
  %add22 = fadd fast double %mul19, %mul21
  %i30 = load double, ptr %nfz, align 8
  %i31 = load double, ptr %fz, align 8
  %add23 = fadd fast double %i30, %i31
  %i32 = load double, ptr %dz, align 8
  %mul24 = fmul fast double %add23, %i32
  %add25 = fadd fast double %add22, %mul24
  %mul26 = fmul fast double %add25, 5.000000e-01
  store double %mul26, ptr %smc_deltaE, align 8
  %mul28 = fmul fast double %i4, 2.500000e-01
  %i33 = load double, ptr %nfx, align 8
  %mul29 = fmul fast double %i33, %i33
  %i34 = load double, ptr %nfy, align 8
  %mul30 = fmul fast double %i34, %i34
  %add31 = fadd fast double %mul29, %mul30
  %i35 = load double, ptr %nfz, align 8
  %mul32 = fmul fast double %i35, %i35
  %add33 = fadd fast double %add31, %mul32
  %i36 = load double, ptr %fx, align 8
  %mul34 = fmul fast double %i36, %i36
  %i37 = load double, ptr %fy, align 8
  %mul35 = fmul fast double %i37, %i37
  %i38 = fadd fast double %mul34, %mul35
  %i39 = load double, ptr %fz, align 8
  %mul37 = fmul fast double %i39, %i39
  %i40 = fadd fast double %i38, %mul37
  %sub38 = fsub fast double %add33, %i40
  %mul39 = fmul fast double %mul28, %sub38
  %add40 = fadd fast double %mul26, %mul39
  store double %add40, ptr %smc_deltaE, align 8
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %nfz)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %nfy)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %nfx)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %rz)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %ry)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %rx)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %fz)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %fy)
  call void @llvm.lifetime.end.p0(i64 8, ptr nonnull %fx)
  ret void
}

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #2

; Function Attrs: argmemonly nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

attributes #0 = { nounwind }
attributes #1 = { "may-have-openmp-directive"="true" }
attributes #2 = { argmemonly nocallback nofree nosync nounwind willreturn }
attributes #3 = { argmemonly nofree nounwind willreturn }
; end INTEL_FEATURE_SW_ADVANCED
