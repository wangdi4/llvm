; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced
; RUN: opt < %s -S -passes='cgscc(inline)' -pre-lto-inline-cost -inline-report=0xe807 2>&1 | FileCheck %s
; RUN: opt -passes='inlinereportsetup' -inline-report=0xe886 < %s -S | opt -passes='cgscc(inline)' -pre-lto-inline-cost -inline-report=0xe886 -S | opt -passes='inlinereportemitter' -inline-report=0xe886 -S 2>&1 | FileCheck %s --check-prefixes=CHECK-MD

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

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%struct.Phase = type { i32, double, double, i32, double*, i64, i64, i64, %struct.Polymer*, i16*, i16*, i32*, i8*, double*, double*, double*, i64*, i64*, double*, double*, double*, i32, i32, double*, i32, i64, i64, i32, i32, i32, i64, double, double, double, double, double, double, i64, i64, double, i32*, %struct.Info_MPI, i32, i32*, i32, i32*, %struct.som_args, i64, i32, i8, double, i32, %struct.IndependetSets*, i32, i32, %struct.Autotuner, %struct.Autotuner, double, i32, %struct.Allocator* }
%struct.Polymer = type { i64, i64, i32, %struct.RNG_STATE, %struct.RNG_STATE*, i32* }
%struct.RNG_STATE = type { %struct.PCG_STATE, %struct.MERSENNE_TWISTER_STATE*, %struct.MTTSTATE* }
%struct.Info_MPI = type { i32, i32, i32, i32, %struct.MPI_Status, double, i32 }
%struct.MPI_Status = type { i32, i32, i32, i32, i32 }
%struct.som_args = type { i8*, i8*, i32, i8*, i8*, i32, i8*, i8*, i32, i8*, i8*, double, i8*, i8*, i32, i8*, i8*, i32, i8*, i8*, i32, i8*, i8*, i32, i8*, i32, i8*, i8*, i32, i8*, i8*, i32, i8*, i32, i8*, i8*, i8*, i8*, i8*, i32, i8*, i8*, i32, i8*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%struct.IndependetSets = type { i32, i32, i32*, i32* }
%struct.Autotuner = type { [12 x i32], [12 x double], double, i8, i32, i32, i8, i32 }
%struct.PCG_STATE = type { i64, i64 }
%struct.MERSENNE_TWISTER_STATE = type { [2 x i32], i32, [624 x i32] }
%struct.MTTSTATE = type { [2 x i32], i32, [27 x i32] }
%struct.Monomer = type { double, double, double }
%struct.Allocator = type { %struct.anon, %struct.anon.0, %struct.anon.1, %struct.anon.2, %struct.anon.3, %struct.anon.4 }
%struct.anon = type { i64, i64, %struct.Monomer*, %struct.Monomer* }
%struct.anon.0 = type { i64, i64, %struct.Monomer*, %struct.Monomer* }
%struct.anon.1 = type { i64, i64, %struct.RNG_STATE*, %struct.RNG_STATE* }
%struct.anon.2 = type { i64, i64, %struct.MERSENNE_TWISTER_STATE*, %struct.MERSENNE_TWISTER_STATE* }
%struct.anon.3 = type { i64, i64, %struct.MTTSTATE*, %struct.MTTSTATE* }
%struct.anon.4 = type { i64, i64, i32*, i32* }

declare dso_local i32 @get_bondlist_offset(i32)
declare dso_local i32 @get_end(i32)
declare dso_local i32 @get_bond_type(i32)
declare dso_local i32 @get_offset(i32)
declare dso_local i32 @fprintf(%struct._IO_FILE*, i8*, ...)
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture)
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture)
declare token @llvm.directive.region.entry()
declare dso_local i32 @soma_rng_uint(%struct.RNG_STATE*, i32)
declare dso_local i32 @get_particle_type(i32)
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg)
declare void @trial_move(%struct.Phase* %p, i64 %ipoly, i32 %ibead, double* %dx, double* %dy, double* %dz, i32 %iwtype, i32 %arg_rng_type, %struct.RNG_STATE* %rng_state)
declare i32 @possible_move_area51(%struct.Phase* %p, double %oldx, double %oldy, double %oldz, double %dx, double %dy, double %dz, i32 %nonexact)
declare double @calc_delta_energy(%struct.Phase* %p, i64 %ipoly, %struct.Monomer* %monomer, i32 %ibead, double %dx, double %dy, double %dz, i32 %iwtype)
declare zeroext i1 @som_accept(%struct.RNG_STATE* %rng, i32 %rng_type, double %delta_energy)
declare void @llvm.directive.region.exit(token)
declare dso_local void @soma_normal_vector(%struct.RNG_STATE*, i32, double*, double*, double*)

@stderr = external dso_local global %struct._IO_FILE*, align 8
@.str = private unnamed_addr constant [46 x i8] c"ERROR: %s:%d stiff bond not yet implemented.\0A\00", align 1
@.str.1 = private unnamed_addr constant [5 x i8] c"mc.c\00", align 1
@.str.2 = private unnamed_addr constant [43 x i8] c"ERROR: %s:%d unknow bond type appeared %d\0A\00", align 1

define void @add_bond_forces(%struct.Phase* %p, i64 %ipoly, i32 %ibead, double %x, double %y, double %z, double* %fx, double* %fy, double* %fz) local_unnamed_addr {
entry:
  %poly_arch = getelementptr inbounds %struct.Phase, %struct.Phase* %p, i64 0, i32 45
  %0 = load i32*, i32** %poly_arch, align 8
  %poly_type_offset = getelementptr inbounds %struct.Phase, %struct.Phase* %p, i64 0, i32 43
  %1 = load i32*, i32** %poly_type_offset, align 8
  %polymers = getelementptr inbounds %struct.Phase, %struct.Phase* %p, i64 0, i32 8
  %2 = load %struct.Polymer*, %struct.Polymer** %polymers, align 8
  %type = getelementptr inbounds %struct.Polymer, %struct.Polymer* %2, i64 %ipoly, i32 2
  %3 = load i32, i32* %type, align 8
  %idxprom = zext i32 %3 to i64
  %ptridx1 = getelementptr inbounds i32, i32* %1, i64 %idxprom
  %4 = load i32, i32* %ptridx1, align 4
  %add = add i32 %4, %ibead
  %add2 = add i32 %add, 1
  %idxprom3 = zext i32 %add2 to i64
  %ptridx4 = getelementptr inbounds i32, i32* %0, i64 %idxprom3
  %5 = load i32, i32* %ptridx4, align 4
  %call = call i32 @get_bondlist_offset(i32 %5) #5
  %cmp = icmp sgt i32 %call, 0
  br i1 %cmp, label %do.body, label %if.end

do.body:                                          ; preds = %entry, %sw.epilog
  %i.0 = phi i32 [ %inc, %sw.epilog ], [ %call, %entry ]
  %v1z.0 = phi double [ %v1z.1, %sw.epilog ], [ 0.000000e+00, %entry ]
  %v1y.0 = phi double [ %v1y.1, %sw.epilog ], [ 0.000000e+00, %entry ]
  %v1x.0 = phi double [ %v1x.1, %sw.epilog ], [ 0.000000e+00, %entry ]
  %6 = load i32*, i32** %poly_arch, align 8
  %inc = add nsw i32 %i.0, 1
  %idxprom6 = sext i32 %i.0 to i64
  %ptridx7 = getelementptr inbounds i32, i32* %6, i64 %idxprom6
  %7 = load i32, i32* %ptridx7, align 4
  %call8 = call i32 @get_end(i32 %7) #5
  %call9 = call i32 @get_bond_type(i32 %7) #5
  %call10 = call i32 @get_offset(i32 %7) #5
  %add11 = add i32 %call10, %ibead
  switch i32 %call9, label %sw.default [
    i32 2, label %sw.bb
    i32 0, label %sw.bb12
    i32 1, label %sw.bb53
  ]

sw.bb:                                            ; preds = %do.body
  %harmonic_normb_variable_scale = getelementptr inbounds %struct.Phase, %struct.Phase* %p, i64 0, i32 2
  %8 = load double, double* %harmonic_normb_variable_scale, align 8
  br label %sw.bb12

sw.bb12:                                          ; preds = %do.body, %sw.bb
  %scale.0 = phi double [ 1.000000e+00, %do.body ], [ %8, %sw.bb ]
  %allocator = getelementptr inbounds %struct.Phase, %struct.Phase* %p, i64 0, i32 59
  %9 = load %struct.Allocator*, %struct.Allocator** %allocator, align 8
  %device_buf = getelementptr inbounds %struct.Allocator, %struct.Allocator* %9, i64 0, i32 0, i32 3
  %10 = load %struct.Monomer*, %struct.Monomer** %device_buf, align 8
  %11 = load %struct.Polymer*, %struct.Polymer** %polymers, align 8
  %beads = getelementptr inbounds %struct.Polymer, %struct.Polymer* %11, i64 %ipoly, i32 0
  %12 = load i64, i64* %beads, align 8
  %conv = zext i32 %add11 to i64
  %add15 = add i64 %12, %conv
  %x17 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %10, i64 %add15, i32 0
  %13 = load double, double* %x17, align 8
  %sub = fsub fast double %13, %x
  %mul = fmul fast double %sub, 2.000000e+00
  %harmonic_normb = getelementptr inbounds %struct.Phase, %struct.Phase* %p, i64 0, i32 1
  %14 = load double, double* %harmonic_normb, align 8
  %mul18 = fmul fast double %mul, %14
  %mul19 = fmul fast double %mul18, %scale.0
  %add20 = fadd fast double %v1x.0, %mul19
  %y30 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %10, i64 %add15, i32 1
  %15 = load double, double* %y30, align 8
  %sub31 = fsub fast double %15, %y
  %mul32 = fmul fast double %sub31, 2.000000e+00
  %mul34 = fmul fast double %mul32, %14
  %mul35 = fmul fast double %mul34, %scale.0
  %add36 = fadd fast double %v1y.0, %mul35
  %z46 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %10, i64 %add15, i32 2
  %16 = load double, double* %z46, align 8
  %sub47 = fsub fast double %16, %z
  %mul48 = fmul fast double %sub47, 2.000000e+00
  %mul50 = fmul fast double %mul48, %14
  %mul51 = fmul fast double %mul50, %scale.0
  %add52 = fadd fast double %v1z.0, %mul51
  br label %sw.epilog

sw.bb53:                                          ; preds = %do.body
  %17 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8
  %call54 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %17, i8* getelementptr inbounds ([46 x i8], [46 x i8]* @.str, i64 0, i64 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.1, i64 0, i64 0), i32 748) #9
  br label %sw.epilog

sw.default:                                       ; preds = %do.body
  %18 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8
  %call55 = call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %18, i8* getelementptr inbounds ([43 x i8], [43 x i8]* @.str.2, i64 0, i64 0), i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.str.1, i64 0, i64 0), i32 758, i32 %call9) #9
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
  %19 = load double, double* %fx, align 8
  %add58 = fadd fast double %19, %v1x.2
  store double %add58, double* %fx, align 8
  %20 = load double, double* %fy, align 8
  %add59 = fadd fast double %20, %v1y.2
  store double %add59, double* %fy, align 8
  %21 = load double, double* %fz, align 8
  %add60 = fadd fast double %21, %v1z.2
  store double %add60, double* %fz, align 8
  ret void
}

define i32 @mc_polymer_iteration(%struct.Phase* %p, i32 %nsteps, i32 %tuning_parameter) local_unnamed_addr #0 {
entry:
  %p.addr = alloca %struct.Phase*, align 8
  %n_accepts = alloca i32, align 4
  %.omp.iv = alloca i64, align 8
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %npoly = alloca i64, align 8
  %accepted_moves_loc = alloca i32, align 4
  %mypoly = alloca %struct.Polymer*, align 8
  %myN = alloca i32, align 4
  %myrngstate = alloca %struct.RNG_STATE*, align 8
  %arg_rng_type = alloca i32, align 4
  %nmc = alloca i32, align 4
  %dx = alloca double, align 8
  %dy = alloca double, align 8
  %dz = alloca double, align 8
  %delta_energy = alloca double, align 8
  %ibead = alloca i32, align 4
  %iwtype = alloca i32, align 4
  %mybead = alloca %struct.Monomer, align 8
  %mybead_ptr = alloca %struct.Monomer*, align 8
  %smc_deltaE = alloca double, align 8
  %newx = alloca double, align 8
  %newy = alloca double, align 8
  %newz = alloca double, align 8
  %move_allowed = alloca i32, align 4
  store %struct.Phase* %p, %struct.Phase** %p.addr, align 8
  %npoly.addr = alloca i64*, align 8
  %accepted_moves_loc.addr = alloca i32*, align 8
  %mypoly.addr = alloca %struct.Polymer**, align 8
  %myN.addr = alloca i32*, align 8
  %myrngstate.addr = alloca %struct.RNG_STATE**, align 8
  %arg_rng_type.addr = alloca i32*, align 8
  %nmc.addr = alloca i32*, align 8
  %delta_energy.addr = alloca double*, align 8
  %dx.addr = alloca double*, align 8
  %dy.addr = alloca double*, align 8
  %dz.addr = alloca double*, align 8
  %ibead.addr = alloca i32*, align 8
  %iwtype.addr = alloca i32*, align 8
  %mybead.addr = alloca %struct.Monomer*, align 8
  %mybead_ptr.addr = alloca %struct.Monomer**, align 8
  %smc_deltaE.addr = alloca double*, align 8
  %newx.addr = alloca double*, align 8
  %newy.addr = alloca double*, align 8
  %newz.addr = alloca double*, align 8
  %move_allowed.addr = alloca i32*, align 8
  %.omp.lb.addr = alloca i64*, align 8
  %p.addr.addr = alloca %struct.Phase**, align 8
  %n_accepts.addr = alloca i32*, align 8
  br label %for.cond

for.cond:                                         ; preds = %omp.precond.end, %entry
  %step.0 = phi i32 [ 0, %entry ], [ %inc64, %omp.precond.end ]
  %cmp = icmp ult i32 %step.0, %nsteps
  br i1 %cmp, label %for.body, label %for.end65

for.body:                                         ; preds = %for.cond
  %0 = load %struct.Phase*, %struct.Phase** %p.addr, align 8
  %n_polymers1 = getelementptr inbounds %struct.Phase, %struct.Phase* %0, i64 0, i32 5
  %1 = load i64, i64* %n_polymers1, align 8
  %2 = bitcast i32* %n_accepts to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #5
  store i32 0, i32* %n_accepts, align 4
  %cmp4 = icmp eq i64 %1, 0
  br i1 %cmp4, label %omp.precond.end, label %DIR.OMP.PARALLEL.LOOP.397

DIR.OMP.PARALLEL.LOOP.397:                        ; preds = %for.body
  %sub2 = add i64 %1, -1
  %3 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %3) #5
  %4 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %4) #5
  store i64 0, i64* %.omp.lb, align 8
  %5 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %5) #5
  store volatile i64 %sub2, i64* %.omp.ub, align 8
  store i64* %npoly, i64** %npoly.addr, align 8
  store i32* %accepted_moves_loc, i32** %accepted_moves_loc.addr, align 8
  store %struct.Polymer** %mypoly, %struct.Polymer*** %mypoly.addr, align 8
  store i32* %myN, i32** %myN.addr, align 8
  store %struct.RNG_STATE** %myrngstate, %struct.RNG_STATE*** %myrngstate.addr, align 8
  store i32* %arg_rng_type, i32** %arg_rng_type.addr, align 8
  store i32* %nmc, i32** %nmc.addr, align 8
  store double* %delta_energy, double** %delta_energy.addr, align 8
  store double* %dx, double** %dx.addr, align 8
  store double* %dy, double** %dy.addr, align 8
  store double* %dz, double** %dz.addr, align 8
  store i32* %ibead, i32** %ibead.addr, align 8
  store i32* %iwtype, i32** %iwtype.addr, align 8
  store %struct.Monomer* %mybead, %struct.Monomer** %mybead.addr, align 8
  store %struct.Monomer** %mybead_ptr, %struct.Monomer*** %mybead_ptr.addr, align 8
  store double* %smc_deltaE, double** %smc_deltaE.addr, align 8
  store double* %newx, double** %newx.addr, align 8
  store double* %newy, double** %newy.addr, align 8
  store double* %newz, double** %newz.addr, align 8
  store i32* %move_allowed, i32** %move_allowed.addr, align 8
  store i64* %.omp.lb, i64** %.omp.lb.addr, align 8
  store %struct.Phase** %p.addr, %struct.Phase*** %p.addr.addr, align 8
  store i32* %n_accepts, i32** %n_accepts.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32* %n_accepts), "QUAL.OMP.SHARED"(%struct.Phase** %p.addr), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i64* %npoly), "QUAL.OMP.PRIVATE"(i32* %accepted_moves_loc), "QUAL.OMP.PRIVATE"(%struct.Polymer** %mypoly), "QUAL.OMP.PRIVATE"(i32* %myN), "QUAL.OMP.PRIVATE"(%struct.RNG_STATE** %myrngstate), "QUAL.OMP.PRIVATE"(i32* %arg_rng_type), "QUAL.OMP.PRIVATE"(i32* %nmc), "QUAL.OMP.PRIVATE"(double* %delta_energy), "QUAL.OMP.PRIVATE"(double* %dx), "QUAL.OMP.PRIVATE"(double* %dy), "QUAL.OMP.PRIVATE"(double* %dz), "QUAL.OMP.PRIVATE"(i32* %ibead), "QUAL.OMP.PRIVATE"(i32* %iwtype), "QUAL.OMP.PRIVATE"(%struct.Monomer* %mybead), "QUAL.OMP.PRIVATE"(%struct.Monomer** %mybead_ptr), "QUAL.OMP.PRIVATE"(double* %smc_deltaE), "QUAL.OMP.PRIVATE"(double* %newx), "QUAL.OMP.PRIVATE"(double* %newy), "QUAL.OMP.PRIVATE"(double* %newz), "QUAL.OMP.PRIVATE"(i32* %move_allowed), "QUAL.OMP.OPERAND.ADDR"(i64* %npoly, i64** %npoly.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %accepted_moves_loc, i32** %accepted_moves_loc.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.Polymer** %mypoly, %struct.Polymer*** %mypoly.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %myN, i32** %myN.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.RNG_STATE** %myrngstate, %struct.RNG_STATE*** %myrngstate.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %arg_rng_type, i32** %arg_rng_type.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %nmc, i32** %nmc.addr), "QUAL.OMP.OPERAND.ADDR"(double* %delta_energy, double** %delta_energy.addr), "QUAL.OMP.OPERAND.ADDR"(double* %dx, double** %dx.addr), "QUAL.OMP.OPERAND.ADDR"(double* %dy, double** %dy.addr), "QUAL.OMP.OPERAND.ADDR"(double* %dz, double** %dz.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %ibead, i32** %ibead.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %iwtype, i32** %iwtype.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.Monomer* %mybead, %struct.Monomer** %mybead.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.Monomer** %mybead_ptr, %struct.Monomer*** %mybead_ptr.addr), "QUAL.OMP.OPERAND.ADDR"(double* %smc_deltaE, double** %smc_deltaE.addr), "QUAL.OMP.OPERAND.ADDR"(double* %newx, double** %newx.addr), "QUAL.OMP.OPERAND.ADDR"(double* %newy, double** %newy.addr), "QUAL.OMP.OPERAND.ADDR"(double* %newz, double** %newz.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %move_allowed, i32** %move_allowed.addr), "QUAL.OMP.OPERAND.ADDR"(i64* %.omp.lb, i64** %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.Phase** %p.addr, %struct.Phase*** %p.addr.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %n_accepts, i32** %n_accepts.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.PARALLEL.LOOP.4, label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.397
  %npoly67 = load volatile i64*, i64** %npoly.addr, align 8
  %accepted_moves_loc68 = load volatile i32*, i32** %accepted_moves_loc.addr, align 8
  %mypoly69 = load volatile %struct.Polymer**, %struct.Polymer*** %mypoly.addr, align 8
  %myN70 = load volatile i32*, i32** %myN.addr, align 8
  %myrngstate71 = load volatile %struct.RNG_STATE**, %struct.RNG_STATE*** %myrngstate.addr, align 8
  %arg_rng_type72 = load volatile i32*, i32** %arg_rng_type.addr, align 8
  %nmc73 = load volatile i32*, i32** %nmc.addr, align 8
  %delta_energy74 = load volatile double*, double** %delta_energy.addr, align 8
  %dx75 = load volatile double*, double** %dx.addr, align 8
  %dy76 = load volatile double*, double** %dy.addr, align 8
  %dz77 = load volatile double*, double** %dz.addr, align 8
  %ibead78 = load volatile i32*, i32** %ibead.addr, align 8
  %iwtype79 = load volatile i32*, i32** %iwtype.addr, align 8
  %mybead80 = load volatile %struct.Monomer*, %struct.Monomer** %mybead.addr, align 8
  %mybead_ptr81 = load volatile %struct.Monomer**, %struct.Monomer*** %mybead_ptr.addr, align 8
  %smc_deltaE82 = load volatile double*, double** %smc_deltaE.addr, align 8
  %newx83 = load volatile double*, double** %newx.addr, align 8
  %newy84 = load volatile double*, double** %newy.addr, align 8
  %newz85 = load volatile double*, double** %newz.addr, align 8
  %move_allowed86 = load volatile i32*, i32** %move_allowed.addr, align 8
  %.omp.lb87 = load volatile i64*, i64** %.omp.lb.addr, align 8
  %p.addr88 = load volatile %struct.Phase**, %struct.Phase*** %p.addr.addr, align 8
  %n_accepts89 = load volatile i32*, i32** %n_accepts.addr, align 8
  %7 = load i64, i64* %.omp.lb87, align 8
  store volatile i64 %7, i64* %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %for.cond.cleanup, %DIR.OMP.PARALLEL.LOOP.3
  %8 = load volatile i64, i64* %.omp.iv, align 8
  %9 = load volatile i64, i64* %.omp.ub, align 8
  %add5 = add i64 %9, 1
  %cmp6 = icmp ult i64 %8, %add5
  br i1 %cmp6, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.4

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %10 = bitcast i64* %npoly67 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %10) #5
  %11 = load volatile i64, i64* %.omp.iv, align 8
  store i64 %11, i64* %npoly67, align 8
  %12 = bitcast i32* %accepted_moves_loc68 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %12) #5
  store i32 0, i32* %accepted_moves_loc68, align 4
  %13 = bitcast %struct.Polymer** %mypoly69 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %13) #5
  %14 = load %struct.Phase*, %struct.Phase** %p.addr88, align 8
  %polymers = getelementptr inbounds %struct.Phase, %struct.Phase* %14, i64 0, i32 8
  %15 = load %struct.Polymer*, %struct.Polymer** %polymers, align 8
  %16 = load i64, i64* %npoly67, align 8
  %ptridx = getelementptr inbounds %struct.Polymer, %struct.Polymer* %15, i64 %16
  store %struct.Polymer* %ptridx, %struct.Polymer** %mypoly69, align 8
  %17 = bitcast i32* %myN70 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %17) #5
  %18 = load %struct.Phase*, %struct.Phase** %p.addr88, align 8
  %poly_arch = getelementptr inbounds %struct.Phase, %struct.Phase* %18, i64 0, i32 45
  %19 = load i32*, i32** %poly_arch, align 8
  %poly_type_offset = getelementptr inbounds %struct.Phase, %struct.Phase* %18, i64 0, i32 43
  %20 = load i32*, i32** %poly_type_offset, align 8
  %21 = load %struct.Polymer*, %struct.Polymer** %mypoly69, align 8
  %type = getelementptr inbounds %struct.Polymer, %struct.Polymer* %21, i64 0, i32 2
  %22 = load i32, i32* %type, align 8
  %idxprom = zext i32 %22 to i64
  %ptridx8 = getelementptr inbounds i32, i32* %20, i64 %idxprom
  %23 = load i32, i32* %ptridx8, align 4
  %idxprom9 = sext i32 %23 to i64
  %ptridx10 = getelementptr inbounds i32, i32* %19, i64 %idxprom9
  %24 = load i32, i32* %ptridx10, align 4
  store i32 %24, i32* %myN70, align 4
  %25 = bitcast %struct.RNG_STATE** %myrngstate71 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %25) #5
  %26 = load %struct.Polymer*, %struct.Polymer** %mypoly69, align 8
  %poly_state = getelementptr inbounds %struct.Polymer, %struct.Polymer* %26, i64 0, i32 3
  store %struct.RNG_STATE* %poly_state, %struct.RNG_STATE** %myrngstate71, align 8
  %27 = bitcast i32* %arg_rng_type72 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %27) #5
  %28 = load %struct.Phase*, %struct.Phase** %p.addr88, align 8
  %pseudo_random_number_generator_arg = getelementptr inbounds %struct.Phase, %struct.Phase* %28, i64 0, i32 46, i32 17
  %29 = load i32, i32* %pseudo_random_number_generator_arg, align 8
  store i32 %29, i32* %arg_rng_type72, align 4
  %30 = bitcast i32* %nmc73 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %30) #5
  br label %for.cond11

for.cond11:                                       ; preds = %if.end55, %omp.inner.for.body
  %storemerge = phi i32 [ 0, %omp.inner.for.body ], [ %inc, %if.end55 ]
  store i32 %storemerge, i32* %nmc73, align 4
  %31 = load i32, i32* %myN70, align 4
  %cmp12 = icmp ult i32 %storemerge, %31
  br i1 %cmp12, label %for.body13, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond11
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %30) #5
  %32 = load i32, i32* %accepted_moves_loc68, align 4
  %33 = load i32, i32* %n_accepts89, align 4
  %add56 = add i32 %33, %32
  store i32 %add56, i32* %n_accepts89, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %27) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %25) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %17) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %13) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %12) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %10) #5
  %34 = load volatile i64, i64* %.omp.iv, align 8
  %add57 = add nuw i64 %34, 1
  store volatile i64 %add57, i64* %.omp.iv, align 8
  br label %omp.inner.for.cond

for.body13:                                       ; preds = %for.cond11
  %35 = bitcast double* %dx75 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %35) #5
  store double 0.000000e+00, double* %dx75, align 8
  %36 = bitcast double* %dy76 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %36) #5
  store double 0.000000e+00, double* %dy76, align 8
  %37 = bitcast double* %dz77 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %37) #5
  store double 0.000000e+00, double* %dz77, align 8
  %38 = bitcast double* %delta_energy74 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %38) #5
  store double 0.000000e+00, double* %delta_energy74, align 8
  %39 = bitcast i32* %ibead78 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %39) #5
  %40 = load %struct.RNG_STATE*, %struct.RNG_STATE** %myrngstate71, align 8
  %41 = load i32, i32* %arg_rng_type72, align 4
  %call = call i32 @soma_rng_uint(%struct.RNG_STATE* %40, i32 %41) #5
  %42 = load i32, i32* %myN70, align 4
  %rem = urem i32 %call, %42
  store i32 %rem, i32* %ibead78, align 4
  %43 = bitcast i32* %iwtype79 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %43) #5
  %44 = load %struct.Phase*, %struct.Phase** %p.addr88, align 8
  %poly_arch14 = getelementptr inbounds %struct.Phase, %struct.Phase* %44, i64 0, i32 45
  %45 = load i32*, i32** %poly_arch14, align 8
  %poly_type_offset15 = getelementptr inbounds %struct.Phase, %struct.Phase* %44, i64 0, i32 43
  %46 = load i32*, i32** %poly_type_offset15, align 8
  %47 = load %struct.Polymer*, %struct.Polymer** %mypoly69, align 8
  %type16 = getelementptr inbounds %struct.Polymer, %struct.Polymer* %47, i64 0, i32 2
  %48 = load i32, i32* %type16, align 8
  %idxprom17 = zext i32 %48 to i64
  %ptridx18 = getelementptr inbounds i32, i32* %46, i64 %idxprom17
  %49 = load i32, i32* %ptridx18, align 4
  %add19 = add nsw i32 %49, 1
  %50 = load i32, i32* %ibead78, align 4
  %add20 = add i32 %add19, %50
  %idxprom21 = zext i32 %add20 to i64
  %ptridx22 = getelementptr inbounds i32, i32* %45, i64 %idxprom21
  %51 = load i32, i32* %ptridx22, align 4
  %call23 = call i32 @get_particle_type(i32 %51) #5
  store i32 %call23, i32* %iwtype79, align 4
  %52 = bitcast %struct.Monomer* %mybead80 to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* %52) #5
  %53 = load %struct.Phase*, %struct.Phase** %p.addr88, align 8
  %allocator = getelementptr inbounds %struct.Phase, %struct.Phase* %53, i64 0, i32 59
  %54 = load %struct.Allocator*, %struct.Allocator** %allocator, align 8
  %device_buf = getelementptr inbounds %struct.Allocator, %struct.Allocator* %54, i64 0, i32 0, i32 3
  %55 = load %struct.Monomer*, %struct.Monomer** %device_buf, align 8
  %56 = load %struct.Polymer*, %struct.Polymer** %mypoly69, align 8
  %beads = getelementptr inbounds %struct.Polymer, %struct.Polymer* %56, i64 0, i32 0
  %57 = load i64, i64* %beads, align 8
  %58 = load i32, i32* %ibead78, align 4
  %conv = zext i32 %58 to i64
  %add24 = add i64 %57, %conv
  %ptridx25 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %55, i64 %add24
  %59 = bitcast %struct.Monomer* %ptridx25 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 dereferenceable(24) %52, i8* nonnull align 8 dereferenceable(24) %59, i64 24, i1 false)
  %60 = bitcast %struct.Monomer** %mybead_ptr81 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %60) #5
  %61 = load %struct.Phase*, %struct.Phase** %p.addr88, align 8
  %allocator26 = getelementptr inbounds %struct.Phase, %struct.Phase* %61, i64 0, i32 59
  %62 = load %struct.Allocator*, %struct.Allocator** %allocator26, align 8
  %device_buf28 = getelementptr inbounds %struct.Allocator, %struct.Allocator* %62, i64 0, i32 0, i32 3
  %63 = load %struct.Monomer*, %struct.Monomer** %device_buf28, align 8
  %64 = load %struct.Polymer*, %struct.Polymer** %mypoly69, align 8
  %beads29 = getelementptr inbounds %struct.Polymer, %struct.Polymer* %64, i64 0, i32 0
  %65 = load i64, i64* %beads29, align 8
  %66 = load i32, i32* %ibead78, align 4
  %conv30 = zext i32 %66 to i64
  %add31 = add i64 %65, %conv30
  %ptridx32 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %63, i64 %add31
  store %struct.Monomer* %ptridx32, %struct.Monomer** %mybead_ptr81, align 8
  %67 = bitcast double* %smc_deltaE82 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %67) #5
  %68 = load %struct.Phase*, %struct.Phase** %p.addr88, align 8
  %move_type_arg = getelementptr inbounds %struct.Phase, %struct.Phase* %68, i64 0, i32 46, i32 25
  %69 = load i32, i32* %move_type_arg, align 8
  switch i32 %69, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb34
  ]

sw.bb:                                            ; preds = %for.body13
  %70 = load i32, i32* %ibead78, align 4
  %71 = load i32, i32* %iwtype79, align 4
  %72 = load i32, i32* %arg_rng_type72, align 4
  %73 = load %struct.RNG_STATE*, %struct.RNG_STATE** %myrngstate71, align 8
  call void @trial_move(%struct.Phase* %68, i64 undef, i32 %70, double* nonnull %dx75, double* nonnull %dy76, double* nonnull %dz77, i32 %71, i32 %72, %struct.RNG_STATE* %73) #5
  store double 0.000000e+00, double* %smc_deltaE82, align 8
  br label %sw.epilog

sw.bb34:                                          ; preds = %for.body13
  %74 = load i64, i64* %npoly67, align 8
  %75 = load i32, i32* %ibead78, align 4
  %76 = load %struct.RNG_STATE*, %struct.RNG_STATE** %myrngstate71, align 8
  %77 = load i32, i32* %arg_rng_type72, align 4
  %78 = load i32, i32* %iwtype79, align 4
  call void @trial_move_smc(%struct.Phase* %68, i64 %74, i32 %75, double* nonnull %dx75, double* nonnull %dy76, double* nonnull %dz77, double* %smc_deltaE82, %struct.Monomer* %mybead80, %struct.RNG_STATE* %76, i32 %77, i32 %78) #5
  br label %sw.epilog

sw.default:                                       ; preds = %for.body13
  store double 0.000000e+00, double* %smc_deltaE82, align 8
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb34, %sw.bb
  %79 = bitcast double* %newx83 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %79) #5
  %x = getelementptr inbounds %struct.Monomer, %struct.Monomer* %mybead80, i64 0, i32 0
  %80 = load double, double* %x, align 8
  %81 = load double, double* %dx75, align 8
  %add36 = fadd fast double %80, %81
  store double %add36, double* %newx83, align 8
  %82 = bitcast double* %newy84 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %82) #5
  %y = getelementptr inbounds %struct.Monomer, %struct.Monomer* %mybead80, i64 0, i32 1
  %83 = load double, double* %y, align 8
  %84 = load double, double* %dy76, align 8
  %add37 = fadd fast double %83, %84
  store double %add37, double* %newy84, align 8
  %85 = bitcast double* %newz85 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %85) #5
  %z = getelementptr inbounds %struct.Monomer, %struct.Monomer* %mybead80, i64 0, i32 2
  %86 = load double, double* %z, align 8
  %87 = load double, double* %dz77, align 8
  %add38 = fadd fast double %86, %87
  store double %add38, double* %newz85, align 8
  %88 = bitcast i32* %move_allowed86 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %88) #5
  %89 = load %struct.Phase*, %struct.Phase** %p.addr88, align 8
  %90 = load double, double* %x, align 8
  %91 = load double, double* %y, align 8
  %92 = load double, double* %z, align 8
  %93 = load double, double* %dx75, align 8
  %94 = load double, double* %dy76, align 8
  %95 = load double, double* %dz77, align 8
  %nonexact_area51_flag = getelementptr inbounds %struct.Phase, %struct.Phase* %89, i64 0, i32 46, i32 23
  %96 = load i32, i32* %nonexact_area51_flag, align 8
  %call43 = call i32 @possible_move_area51(%struct.Phase* %89, double %90, double %91, double %92, double %93, double %94, double %95, i32 %96) #5
  store i32 %call43, i32* %move_allowed86, align 4
  %tobool = icmp eq i32 %call43, 0
  br i1 %tobool, label %if.end55, label %if.then

if.then:                                          ; preds = %sw.epilog
  %97 = load %struct.Phase*, %struct.Phase** %p.addr88, align 8
  %98 = load i64, i64* %npoly67, align 8
  %99 = load i32, i32* %ibead78, align 4
  %100 = load double, double* %dx75, align 8
  %101 = load double, double* %dy76, align 8
  %102 = load double, double* %dz77, align 8
  %103 = load i32, i32* %iwtype79, align 4
  %call44 = call fast double @calc_delta_energy(%struct.Phase* %97, i64 %98, %struct.Monomer* %mybead80, i32 %99, double %100, double %101, double %102, i32 %103) #5
  store double %call44, double* %delta_energy74, align 8
  %104 = load double, double* %smc_deltaE82, align 8
  %add45 = fadd fast double %call44, %104
  store double %add45, double* %delta_energy74, align 8
  %105 = load %struct.RNG_STATE*, %struct.RNG_STATE** %myrngstate71, align 8
  %106 = load i32, i32* %arg_rng_type72, align 4
  %call46 = call zeroext i1 @som_accept(%struct.RNG_STATE* %105, i32 %106, double %add45) #5
  br i1 %call46, label %if.then50, label %if.end55

if.then50:                                        ; preds = %if.then
  %107 = load double, double* %newx83, align 8
  %108 = load %struct.Monomer*, %struct.Monomer** %mybead_ptr81, align 8
  %x51 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %108, i64 0, i32 0
  store double %107, double* %x51, align 8
  %109 = load double, double* %newy84, align 8
  %110 = load %struct.Monomer*, %struct.Monomer** %mybead_ptr81, align 8
  %y52 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %110, i64 0, i32 1
  store double %109, double* %y52, align 8
  %111 = load double, double* %newz85, align 8
  %112 = load %struct.Monomer*, %struct.Monomer** %mybead_ptr81, align 8
  %z53 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %112, i64 0, i32 2
  store double %111, double* %z53, align 8
  %113 = load i32, i32* %accepted_moves_loc68, align 4
  %add54 = add i32 %113, 1
  store i32 %add54, i32* %accepted_moves_loc68, align 4
  br label %if.end55

if.end55:                                         ; preds = %if.then, %if.then50, %sw.epilog
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %88) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %85) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %82) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %79) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* %67) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %60) #5
  call void @llvm.lifetime.end.p0i8(i64 24, i8* %52) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %43) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %39) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %38) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %37) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %36) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %35) #5
  %114 = load i32, i32* %nmc73, align 4
  %inc = add i32 %114, 1
  br label %for.cond11

DIR.OMP.END.PARALLEL.LOOP.4:                      ; preds = %DIR.OMP.PARALLEL.LOOP.397, %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.PARALLEL.LOOP.4, %for.body
  %115 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %115) #5
  %116 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %116) #5
  %117 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %117) #5
  %118 = load %struct.Phase*, %struct.Phase** %p.addr, align 8
  %time = getelementptr inbounds %struct.Phase, %struct.Phase* %118, i64 0, i32 24
  %119 = load i32, i32* %time, align 8
  %add58 = add i32 %119, 1
  store i32 %add58, i32* %time, align 8
  %num_all_beads_local = getelementptr inbounds %struct.Phase, %struct.Phase* %118, i64 0, i32 26
  %120 = load i64, i64* %num_all_beads_local, align 8
  %n_moves = getelementptr inbounds %struct.Phase, %struct.Phase* %118, i64 0, i32 37
  %121 = load i64, i64* %n_moves, align 8
  %add59 = add i64 %121, %120
  store i64 %add59, i64* %n_moves, align 8
  %122 = load i32, i32* %n_accepts, align 4
  %conv60 = zext i32 %122 to i64
  %123 = load %struct.Phase*, %struct.Phase** %p.addr, align 8
  %n_accepts61 = getelementptr inbounds %struct.Phase, %struct.Phase* %123, i64 0, i32 38
  %124 = load i64, i64* %n_accepts61, align 8
  %add62 = add i64 %124, %conv60
  store i64 %add62, i64* %n_accepts61, align 8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #5
  %inc64 = add i32 %step.0, 1
  br label %for.cond

for.end65:                                        ; preds = %for.cond
  ret i32 0
}

define i32 @mc_set_iteration(%struct.Phase* %p, i32 %nsteps, i32 %tuning_parameter) local_unnamed_addr #0 {
entry:
  %p.addr = alloca %struct.Phase*, align 8
  %my_rng_type = alloca i32, align 4
  %nonexact_area51 = alloca i32, align 4
  %cleanup.dest.slot = alloca i32, align 4
  %n_accepts = alloca i32, align 4
  %.omp.iv = alloca i64, align 8
  %.omp.lb = alloca i64, align 8
  %.omp.ub = alloca i64, align 8
  %npoly = alloca i64, align 8
  %accepted_moves_poly = alloca i32, align 4
  %poly_arch = alloca i32*, align 8
  %poly_type_offset = alloca i32*, align 8
  %mypoly = alloca %struct.Polymer*, align 8
  %poly_type = alloca i32, align 4
  %myN = alloca i32, align 4
  %mySets = alloca %struct.IndependetSets, align 8
  %n_sets = alloca i32, align 4
  %set_length = alloca i32*, align 8
  %sets26 = alloca i32*, align 8
  %max_member = alloca i32, align 4
  %set_states = alloca %struct.RNG_STATE*, align 8
  %set_permutation = alloca i32*, align 8
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
  store %struct.Phase* %p, %struct.Phase** %p.addr, align 8
  %0 = bitcast i32* %my_rng_type to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %0) #5
  %pseudo_random_number_generator_arg = getelementptr inbounds %struct.Phase, %struct.Phase* %p, i64 0, i32 46, i32 17
  %1 = load i32, i32* %pseudo_random_number_generator_arg, align 8
  store i32 %1, i32* %my_rng_type, align 4
  %2 = bitcast i32* %nonexact_area51 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %2) #5
  %3 = load %struct.Phase*, %struct.Phase** %p.addr, align 8
  %nonexact_area51_flag = getelementptr inbounds %struct.Phase, %struct.Phase* %3, i64 0, i32 46, i32 23
  %4 = load i32, i32* %nonexact_area51_flag, align 8
  store i32 %4, i32* %nonexact_area51, align 4
  %npoly.addr = alloca i64*, align 8
  %accepted_moves_poly.addr = alloca i32*, align 8
  %poly_arch.addr = alloca i32**, align 8
  %poly_type_offset.addr = alloca i32**, align 8
  %mypoly.addr = alloca %struct.Polymer**, align 8
  %poly_type.addr = alloca i32*, align 8
  %myN.addr = alloca i32*, align 8
  %mySets.addr = alloca %struct.IndependetSets*, align 8
  %n_sets.addr = alloca i32*, align 8
  %set_length.addr = alloca i32**, align 8
  %sets26.addr = alloca i32**, align 8
  %max_member.addr = alloca i32*, align 8
  %set_states.addr = alloca %struct.RNG_STATE**, align 8
  %set_permutation.addr = alloca i32**, align 8
  %i.addr = alloca i32*, align 8
  %d.addr = alloca i32*, align 8
  %iSet.addr = alloca i32*, align 8
  %accepted_moves_set.addr = alloca i32*, align 8
  %set_id.addr = alloca i32*, align 8
  %len.addr = alloca i32*, align 8
  %iP.addr = alloca i32*, align 8
  %ibead.addr = alloca i32*, align 8
  %iwtype.addr = alloca i32*, align 8
  %my_state.addr = alloca %struct.RNG_STATE*, align 8
  %mybead.addr = alloca %struct.Monomer*, align 8
  %dx.addr = alloca %struct.Monomer*, align 8
  %smc_deltaE.addr = alloca double*, align 8
  %move_allowed.addr = alloca i32*, align 8
  %delta_energy.addr = alloca double*, align 8
  %newx.addr = alloca %struct.Monomer*, align 8
  %cleanup.dest.slot.addr = alloca i32*, align 8
  %.omp.lb.addr = alloca i64*, align 8
  %p.addr.addr = alloca %struct.Phase**, align 8
  %my_rng_type.addr = alloca i32*, align 8
  %nonexact_area51.addr = alloca i32*, align 8
  %n_accepts.addr = alloca i32*, align 8
  br label %for.cond

for.cond:                                         ; preds = %omp.precond.end, %entry
  %step.0 = phi i32 [ 0, %entry ], [ %inc134, %omp.precond.end ]
  %cmp = icmp ult i32 %step.0, %nsteps
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  store i32 1, i32* %cleanup.dest.slot, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %2) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %0) #5
  ret i32 0

for.body:                                         ; preds = %for.cond
  %5 = load %struct.Phase*, %struct.Phase** %p.addr, align 8
  %n_polymers2 = getelementptr inbounds %struct.Phase, %struct.Phase* %5, i64 0, i32 5
  %6 = load i64, i64* %n_polymers2, align 8
  %7 = bitcast i32* %n_accepts to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %7) #5
  store i32 0, i32* %n_accepts, align 4
  %cmp6 = icmp eq i64 %6, 0
  br i1 %cmp6, label %omp.precond.end, label %DIR.OMP.PARALLEL.LOOP.3181

DIR.OMP.PARALLEL.LOOP.3181:                       ; preds = %for.body
  %sub3 = add i64 %6, -1
  %8 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %8) #5
  %9 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %9) #5
  store i64 0, i64* %.omp.lb, align 8
  %10 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %10) #5
  store volatile i64 %sub3, i64* %.omp.ub, align 8
  store i64* %npoly, i64** %npoly.addr, align 8
  store i32* %accepted_moves_poly, i32** %accepted_moves_poly.addr, align 8
  store i32** %poly_arch, i32*** %poly_arch.addr, align 8
  store i32** %poly_type_offset, i32*** %poly_type_offset.addr, align 8
  store %struct.Polymer** %mypoly, %struct.Polymer*** %mypoly.addr, align 8
  store i32* %poly_type, i32** %poly_type.addr, align 8
  store i32* %myN, i32** %myN.addr, align 8
  store %struct.IndependetSets* %mySets, %struct.IndependetSets** %mySets.addr, align 8
  store i32* %n_sets, i32** %n_sets.addr, align 8
  store i32** %set_length, i32*** %set_length.addr, align 8
  store i32** %sets26, i32*** %sets26.addr, align 8
  store i32* %max_member, i32** %max_member.addr, align 8
  store %struct.RNG_STATE** %set_states, %struct.RNG_STATE*** %set_states.addr, align 8
  store i32** %set_permutation, i32*** %set_permutation.addr, align 8
  store i32* %i, i32** %i.addr, align 8
  store i32* %d, i32** %d.addr, align 8
  store i32* %iSet, i32** %iSet.addr, align 8
  store i32* %accepted_moves_set, i32** %accepted_moves_set.addr, align 8
  store i32* %set_id, i32** %set_id.addr, align 8
  store i32* %len, i32** %len.addr, align 8
  store i32* %iP, i32** %iP.addr, align 8
  store i32* %ibead, i32** %ibead.addr, align 8
  store i32* %iwtype, i32** %iwtype.addr, align 8
  store %struct.RNG_STATE* %my_state, %struct.RNG_STATE** %my_state.addr, align 8
  store %struct.Monomer* %mybead, %struct.Monomer** %mybead.addr, align 8
  store %struct.Monomer* %dx, %struct.Monomer** %dx.addr, align 8
  store double* %smc_deltaE, double** %smc_deltaE.addr, align 8
  store i32* %move_allowed, i32** %move_allowed.addr, align 8
  store double* %delta_energy, double** %delta_energy.addr, align 8
  store %struct.Monomer* %newx, %struct.Monomer** %newx.addr, align 8
  store i32* %cleanup.dest.slot, i32** %cleanup.dest.slot.addr, align 8
  store i64* %.omp.lb, i64** %.omp.lb.addr, align 8
  store %struct.Phase** %p.addr, %struct.Phase*** %p.addr.addr, align 8
  store i32* %my_rng_type, i32** %my_rng_type.addr, align 8
  store i32* %nonexact_area51, i32** %nonexact_area51.addr, align 8
  store i32* %n_accepts, i32** %n_accepts.addr, align 8
  %end.dir.temp = alloca i1, align 1
  %11 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL.LOOP"(), "QUAL.OMP.REDUCTION.ADD"(i32* %n_accepts), "QUAL.OMP.SHARED"(%struct.Phase** %p.addr), "QUAL.OMP.SHARED"(i32* %my_rng_type), "QUAL.OMP.SHARED"(i32* %nonexact_area51), "QUAL.OMP.NORMALIZED.IV"(i64* %.omp.iv), "QUAL.OMP.FIRSTPRIVATE"(i64* %.omp.lb), "QUAL.OMP.NORMALIZED.UB"(i64* %.omp.ub), "QUAL.OMP.PRIVATE"(i64* %npoly), "QUAL.OMP.PRIVATE"(i32* %accepted_moves_poly), "QUAL.OMP.PRIVATE"(i32** %poly_arch), "QUAL.OMP.PRIVATE"(i32** %poly_type_offset), "QUAL.OMP.PRIVATE"(%struct.Polymer** %mypoly), "QUAL.OMP.PRIVATE"(i32* %poly_type), "QUAL.OMP.PRIVATE"(i32* %myN), "QUAL.OMP.PRIVATE"(%struct.IndependetSets* %mySets), "QUAL.OMP.PRIVATE"(i32* %n_sets), "QUAL.OMP.PRIVATE"(i32** %set_length), "QUAL.OMP.PRIVATE"(i32** %sets26), "QUAL.OMP.PRIVATE"(i32* %max_member), "QUAL.OMP.PRIVATE"(%struct.RNG_STATE** %set_states), "QUAL.OMP.PRIVATE"(i32** %set_permutation), "QUAL.OMP.PRIVATE"(i32* %i), "QUAL.OMP.PRIVATE"(i32* %d), "QUAL.OMP.PRIVATE"(i32* %iSet), "QUAL.OMP.PRIVATE"(i32* %accepted_moves_set), "QUAL.OMP.PRIVATE"(i32* %set_id), "QUAL.OMP.PRIVATE"(i32* %len), "QUAL.OMP.PRIVATE"(i32* %iP), "QUAL.OMP.PRIVATE"(i32* %ibead), "QUAL.OMP.PRIVATE"(i32* %iwtype), "QUAL.OMP.PRIVATE"(%struct.RNG_STATE* %my_state), "QUAL.OMP.PRIVATE"(%struct.Monomer* %mybead), "QUAL.OMP.PRIVATE"(%struct.Monomer* %dx), "QUAL.OMP.PRIVATE"(double* %smc_deltaE), "QUAL.OMP.PRIVATE"(i32* %move_allowed), "QUAL.OMP.PRIVATE"(double* %delta_energy), "QUAL.OMP.PRIVATE"(%struct.Monomer* %newx), "QUAL.OMP.PRIVATE"(i32* %cleanup.dest.slot), "QUAL.OMP.OPERAND.ADDR"(i64* %npoly, i64** %npoly.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %accepted_moves_poly, i32** %accepted_moves_poly.addr), "QUAL.OMP.OPERAND.ADDR"(i32** %poly_arch, i32*** %poly_arch.addr), "QUAL.OMP.OPERAND.ADDR"(i32** %poly_type_offset, i32*** %poly_type_offset.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.Polymer** %mypoly, %struct.Polymer*** %mypoly.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %poly_type, i32** %poly_type.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %myN, i32** %myN.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.IndependetSets* %mySets, %struct.IndependetSets** %mySets.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %n_sets, i32** %n_sets.addr), "QUAL.OMP.OPERAND.ADDR"(i32** %set_length, i32*** %set_length.addr), "QUAL.OMP.OPERAND.ADDR"(i32** %sets26, i32*** %sets26.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %max_member, i32** %max_member.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.RNG_STATE** %set_states, %struct.RNG_STATE*** %set_states.addr), "QUAL.OMP.OPERAND.ADDR"(i32** %set_permutation, i32*** %set_permutation.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %i, i32** %i.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %d, i32** %d.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %iSet, i32** %iSet.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %accepted_moves_set, i32** %accepted_moves_set.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %set_id, i32** %set_id.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %len, i32** %len.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %iP, i32** %iP.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %ibead, i32** %ibead.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %iwtype, i32** %iwtype.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.RNG_STATE* %my_state, %struct.RNG_STATE** %my_state.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.Monomer* %mybead, %struct.Monomer** %mybead.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.Monomer* %dx, %struct.Monomer** %dx.addr), "QUAL.OMP.OPERAND.ADDR"(double* %smc_deltaE, double** %smc_deltaE.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %move_allowed, i32** %move_allowed.addr), "QUAL.OMP.OPERAND.ADDR"(double* %delta_energy, double** %delta_energy.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.Monomer* %newx, %struct.Monomer** %newx.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %cleanup.dest.slot, i32** %cleanup.dest.slot.addr), "QUAL.OMP.OPERAND.ADDR"(i64* %.omp.lb, i64** %.omp.lb.addr), "QUAL.OMP.OPERAND.ADDR"(%struct.Phase** %p.addr, %struct.Phase*** %p.addr.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %my_rng_type, i32** %my_rng_type.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %nonexact_area51, i32** %nonexact_area51.addr), "QUAL.OMP.OPERAND.ADDR"(i32* %n_accepts, i32** %n_accepts.addr), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ]
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1
  br i1 %temp.load, label %DIR.OMP.END.PARALLEL.LOOP.4, label %DIR.OMP.PARALLEL.LOOP.3

DIR.OMP.PARALLEL.LOOP.3:                          ; preds = %DIR.OMP.PARALLEL.LOOP.3181
  %npoly137 = load volatile i64*, i64** %npoly.addr, align 8
  %accepted_moves_poly138 = load volatile i32*, i32** %accepted_moves_poly.addr, align 8
  %poly_arch139 = load volatile i32**, i32*** %poly_arch.addr, align 8
  %poly_type_offset140 = load volatile i32**, i32*** %poly_type_offset.addr, align 8
  %mypoly141 = load volatile %struct.Polymer**, %struct.Polymer*** %mypoly.addr, align 8
  %poly_type142 = load volatile i32*, i32** %poly_type.addr, align 8
  %myN143 = load volatile i32*, i32** %myN.addr, align 8
  %mySets144 = load volatile %struct.IndependetSets*, %struct.IndependetSets** %mySets.addr, align 8
  %n_sets145 = load volatile i32*, i32** %n_sets.addr, align 8
  %set_length146 = load volatile i32**, i32*** %set_length.addr, align 8
  %sets26147 = load volatile i32**, i32*** %sets26.addr, align 8
  %max_member148 = load volatile i32*, i32** %max_member.addr, align 8
  %set_states149 = load volatile %struct.RNG_STATE**, %struct.RNG_STATE*** %set_states.addr, align 8
  %set_permutation150 = load volatile i32**, i32*** %set_permutation.addr, align 8
  %i151 = load volatile i32*, i32** %i.addr, align 8
  %d152 = load volatile i32*, i32** %d.addr, align 8
  %iSet153 = load volatile i32*, i32** %iSet.addr, align 8
  %accepted_moves_set154 = load volatile i32*, i32** %accepted_moves_set.addr, align 8
  %set_id155 = load volatile i32*, i32** %set_id.addr, align 8
  %len156 = load volatile i32*, i32** %len.addr, align 8
  %iP157 = load volatile i32*, i32** %iP.addr, align 8
  %ibead158 = load volatile i32*, i32** %ibead.addr, align 8
  %iwtype159 = load volatile i32*, i32** %iwtype.addr, align 8
  %my_state160 = load volatile %struct.RNG_STATE*, %struct.RNG_STATE** %my_state.addr, align 8
  %mybead161 = load volatile %struct.Monomer*, %struct.Monomer** %mybead.addr, align 8
  %dx162 = load volatile %struct.Monomer*, %struct.Monomer** %dx.addr, align 8
  %smc_deltaE163 = load volatile double*, double** %smc_deltaE.addr, align 8
  %move_allowed164 = load volatile i32*, i32** %move_allowed.addr, align 8
  %delta_energy165 = load volatile double*, double** %delta_energy.addr, align 8
  %newx166 = load volatile %struct.Monomer*, %struct.Monomer** %newx.addr, align 8
  %cleanup.dest.slot167 = load volatile i32*, i32** %cleanup.dest.slot.addr, align 8
  %.omp.lb168 = load volatile i64*, i64** %.omp.lb.addr, align 8
  %p.addr169 = load volatile %struct.Phase**, %struct.Phase*** %p.addr.addr, align 8
  %my_rng_type170 = load volatile i32*, i32** %my_rng_type.addr, align 8
  %nonexact_area51171 = load volatile i32*, i32** %nonexact_area51.addr, align 8
  %n_accepts172 = load volatile i32*, i32** %n_accepts.addr, align 8
  %12 = load i64, i64* %.omp.lb168, align 8
  store volatile i64 %12, i64* %.omp.iv, align 8
  br label %omp.inner.for.cond

omp.inner.for.cond:                               ; preds = %for.cond.cleanup44, %DIR.OMP.PARALLEL.LOOP.3
  %13 = load volatile i64, i64* %.omp.iv, align 8
  %14 = load volatile i64, i64* %.omp.ub, align 8
  %add7 = add i64 %14, 1
  %cmp8 = icmp ult i64 %13, %add7
  br i1 %cmp8, label %omp.inner.for.body, label %DIR.OMP.END.PARALLEL.LOOP.4

omp.inner.for.body:                               ; preds = %omp.inner.for.cond
  %15 = bitcast i64* %npoly137 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %15) #5
  %16 = load volatile i64, i64* %.omp.iv, align 8
  store i64 %16, i64* %npoly137, align 8
  %17 = bitcast i32* %accepted_moves_poly138 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %17) #5
  store i32 0, i32* %accepted_moves_poly138, align 4
  %18 = bitcast i32** %poly_arch139 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %18) #5
  %19 = load %struct.Phase*, %struct.Phase** %p.addr169, align 8
  %poly_arch11 = getelementptr inbounds %struct.Phase, %struct.Phase* %19, i64 0, i32 45
  %20 = load i32*, i32** %poly_arch11, align 8
  store i32* %20, i32** %poly_arch139, align 8
  %21 = bitcast i32** %poly_type_offset140 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %21) #5
  %22 = load %struct.Phase*, %struct.Phase** %p.addr169, align 8
  %poly_type_offset12 = getelementptr inbounds %struct.Phase, %struct.Phase* %22, i64 0, i32 43
  %23 = load i32*, i32** %poly_type_offset12, align 8
  store i32* %23, i32** %poly_type_offset140, align 8
  %24 = bitcast %struct.Polymer** %mypoly141 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %24) #5
  %25 = load %struct.Phase*, %struct.Phase** %p.addr169, align 8
  %polymers = getelementptr inbounds %struct.Phase, %struct.Phase* %25, i64 0, i32 8
  %26 = load %struct.Polymer*, %struct.Polymer** %polymers, align 8
  %27 = load i64, i64* %npoly137, align 8
  %ptridx = getelementptr inbounds %struct.Polymer, %struct.Polymer* %26, i64 %27
  store %struct.Polymer* %ptridx, %struct.Polymer** %mypoly141, align 8
  %28 = bitcast i32* %poly_type142 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %28) #5
  %29 = load %struct.Polymer*, %struct.Polymer** %mypoly141, align 8
  %type = getelementptr inbounds %struct.Polymer, %struct.Polymer* %29, i64 0, i32 2
  %30 = load i32, i32* %type, align 8
  store i32 %30, i32* %poly_type142, align 4
  %31 = bitcast i32* %myN143 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %31) #5
  %32 = load %struct.Phase*, %struct.Phase** %p.addr169, align 8
  %poly_arch13 = getelementptr inbounds %struct.Phase, %struct.Phase* %32, i64 0, i32 45
  %33 = load i32*, i32** %poly_arch13, align 8
  %poly_type_offset14 = getelementptr inbounds %struct.Phase, %struct.Phase* %32, i64 0, i32 43
  %34 = load i32*, i32** %poly_type_offset14, align 8
  %35 = load %struct.Polymer*, %struct.Polymer** %mypoly141, align 8
  %type15 = getelementptr inbounds %struct.Polymer, %struct.Polymer* %35, i64 0, i32 2
  %36 = load i32, i32* %type15, align 8
  %idxprom = zext i32 %36 to i64
  %ptridx16 = getelementptr inbounds i32, i32* %34, i64 %idxprom
  %37 = load i32, i32* %ptridx16, align 4
  %idxprom17 = sext i32 %37 to i64
  %ptridx18 = getelementptr inbounds i32, i32* %33, i64 %idxprom17
  %38 = load i32, i32* %ptridx18, align 4
  store i32 %38, i32* %myN143, align 4
  %39 = bitcast %struct.IndependetSets* %mySets144 to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* %39) #5
  %40 = load %struct.Phase*, %struct.Phase** %p.addr169, align 8
  %sets = getelementptr inbounds %struct.Phase, %struct.Phase* %40, i64 0, i32 52
  %41 = load %struct.IndependetSets*, %struct.IndependetSets** %sets, align 8
  %42 = load %struct.Polymer*, %struct.Polymer** %mypoly141, align 8
  %type21 = getelementptr inbounds %struct.Polymer, %struct.Polymer* %42, i64 0, i32 2
  %43 = load i32, i32* %type21, align 8
  %idxprom22 = zext i32 %43 to i64
  %ptridx23 = getelementptr inbounds %struct.IndependetSets, %struct.IndependetSets* %41, i64 %idxprom22
  %44 = bitcast %struct.IndependetSets* %ptridx23 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 dereferenceable(24) %39, i8* nonnull align 8 dereferenceable(24) %44, i64 24, i1 false)
  %45 = bitcast i32* %n_sets145 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %45) #5
  %n_sets24 = getelementptr inbounds %struct.IndependetSets, %struct.IndependetSets* %mySets144, i64 0, i32 0
  %46 = load i32, i32* %n_sets24, align 8
  store i32 %46, i32* %n_sets145, align 4
  %47 = bitcast i32** %set_length146 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %47) #5
  %set_length25 = getelementptr inbounds %struct.IndependetSets, %struct.IndependetSets* %mySets144, i64 0, i32 2
  %48 = load i32*, i32** %set_length25, align 8
  store i32* %48, i32** %set_length146, align 8
  %49 = bitcast i32** %sets26147 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %49) #5
  %sets27 = getelementptr inbounds %struct.IndependetSets, %struct.IndependetSets* %mySets144, i64 0, i32 3
  %50 = load i32*, i32** %sets27, align 8
  store i32* %50, i32** %sets26147, align 8
  %51 = bitcast i32* %max_member148 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %51) #5
  %max_member28 = getelementptr inbounds %struct.IndependetSets, %struct.IndependetSets* %mySets144, i64 0, i32 1
  %52 = load i32, i32* %max_member28, align 4
  store i32 %52, i32* %max_member148, align 4
  %53 = bitcast %struct.RNG_STATE** %set_states149 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %53) #5
  %54 = load %struct.Polymer*, %struct.Polymer** %mypoly141, align 8
  %set_states29 = getelementptr inbounds %struct.Polymer, %struct.Polymer* %54, i64 0, i32 4
  %55 = load %struct.RNG_STATE*, %struct.RNG_STATE** %set_states29, align 8
  store %struct.RNG_STATE* %55, %struct.RNG_STATE** %set_states149, align 8
  %56 = bitcast i32** %set_permutation150 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %56) #5
  %57 = load %struct.Polymer*, %struct.Polymer** %mypoly141, align 8
  %set_permutation30 = getelementptr inbounds %struct.Polymer, %struct.Polymer* %57, i64 0, i32 5
  %58 = load i32*, i32** %set_permutation30, align 8
  store i32* %58, i32** %set_permutation150, align 8
  %59 = bitcast i32* %i151 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %59) #5
  br label %for.cond31

for.cond31:                                       ; preds = %for.body34, %omp.inner.for.body
  %storemerge = phi i32 [ 0, %omp.inner.for.body ], [ %inc, %for.body34 ]
  store i32 %storemerge, i32* %i151, align 4
  %60 = load i32, i32* %n_sets145, align 4
  %cmp32 = icmp ult i32 %storemerge, %60
  br i1 %cmp32, label %for.body34, label %for.cond.cleanup33

for.cond.cleanup33:                               ; preds = %for.cond31
  store i32 9, i32* %cleanup.dest.slot167, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %59) #5
  %61 = bitcast i32* %iSet153 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %61) #5
  br label %for.cond42

for.body34:                                       ; preds = %for.cond31
  %62 = bitcast i32* %d152 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %62) #5
  %63 = load %struct.Polymer*, %struct.Polymer** %mypoly141, align 8
  %poly_state = getelementptr inbounds %struct.Polymer, %struct.Polymer* %63, i64 0, i32 3
  %64 = load i32, i32* %my_rng_type170, align 4
  %call = call i32 @soma_rng_uint(%struct.RNG_STATE* nonnull %poly_state, i32 %64) #5
  %65 = load i32, i32* %i151, align 4
  %add35 = add i32 %65, 1
  %rem = urem i32 %call, %add35
  store i32 %rem, i32* %d152, align 4
  %66 = load i32*, i32** %set_permutation150, align 8
  %idxprom36 = zext i32 %rem to i64
  %ptridx37 = getelementptr inbounds i32, i32* %66, i64 %idxprom36
  %67 = load i32, i32* %ptridx37, align 4
  %68 = load i32, i32* %i151, align 4
  %idxprom38 = zext i32 %68 to i64
  %ptridx39 = getelementptr inbounds i32, i32* %66, i64 %idxprom38
  store i32 %67, i32* %ptridx39, align 4
  %69 = load i32, i32* %i151, align 4
  %70 = load i32*, i32** %set_permutation150, align 8
  %71 = load i32, i32* %d152, align 4
  %idxprom40 = zext i32 %71 to i64
  %ptridx41 = getelementptr inbounds i32, i32* %70, i64 %idxprom40
  store i32 %69, i32* %ptridx41, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %62) #5
  %72 = load i32, i32* %i151, align 4
  %inc = add i32 %72, 1
  br label %for.cond31

for.cond42:                                       ; preds = %for.cond.cleanup52, %for.cond.cleanup33
  %storemerge182 = phi i32 [ 0, %for.cond.cleanup33 ], [ %inc124, %for.cond.cleanup52 ]
  store i32 %storemerge182, i32* %iSet153, align 4
  %73 = load i32, i32* %n_sets145, align 4
  %cmp43 = icmp ult i32 %storemerge182, %73
  br i1 %cmp43, label %for.body45, label %for.cond.cleanup44

for.cond.cleanup44:                               ; preds = %for.cond42
  store i32 12, i32* %cleanup.dest.slot167, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %61) #5
  %74 = load i32, i32* %accepted_moves_poly138, align 4
  %75 = load i32, i32* %n_accepts172, align 4
  %add126 = add i32 %75, %74
  store i32 %add126, i32* %n_accepts172, align 4
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %56) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %53) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %51) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %49) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %47) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %45) #5
  call void @llvm.lifetime.end.p0i8(i64 24, i8* %39) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %31) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %28) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %24) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %21) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %18) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %17) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %15) #5
  %76 = load volatile i64, i64* %.omp.iv, align 8
  %add127 = add nuw i64 %76, 1
  store volatile i64 %add127, i64* %.omp.iv, align 8
  br label %omp.inner.for.cond

for.body45:                                       ; preds = %for.cond42
  %77 = bitcast i32* %accepted_moves_set154 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %77) #5
  store i32 0, i32* %accepted_moves_set154, align 4
  %78 = bitcast i32* %set_id155 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %78) #5
  %79 = load i32*, i32** %set_permutation150, align 8
  %80 = load i32, i32* %iSet153, align 4
  %idxprom46 = zext i32 %80 to i64
  %ptridx47 = getelementptr inbounds i32, i32* %79, i64 %idxprom46
  %81 = load i32, i32* %ptridx47, align 4
  store i32 %81, i32* %set_id155, align 4
  %82 = bitcast i32* %len156 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %82) #5
  %83 = load i32*, i32** %set_length146, align 8
  %84 = load i32, i32* %set_id155, align 4
  %idxprom48 = zext i32 %84 to i64
  %ptridx49 = getelementptr inbounds i32, i32* %83, i64 %idxprom48
  %85 = load i32, i32* %ptridx49, align 4
  store i32 %85, i32* %len156, align 4
  %86 = bitcast i32* %iP157 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %86) #5
  br label %for.cond50

for.cond50:                                       ; preds = %if.end115, %for.body45
  %storemerge183 = phi i32 [ 0, %for.body45 ], [ %inc120, %if.end115 ]
  store i32 %storemerge183, i32* %iP157, align 4
  %87 = load i32, i32* %len156, align 4
  %cmp51 = icmp ult i32 %storemerge183, %87
  br i1 %cmp51, label %for.body53, label %for.cond.cleanup52

for.cond.cleanup52:                               ; preds = %for.cond50
  store i32 15, i32* %cleanup.dest.slot167, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %86) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %82) #5
  %88 = load i32, i32* %accepted_moves_set154, align 4
  %89 = load i32, i32* %accepted_moves_poly138, align 4
  %add122 = add i32 %89, %88
  store i32 %add122, i32* %accepted_moves_poly138, align 4
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %78) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %77) #5
  %90 = load i32, i32* %iSet153, align 4
  %inc124 = add i32 %90, 1
  br label %for.cond42

for.body53:                                       ; preds = %for.cond50
  %91 = bitcast i32* %ibead158 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %91) #5
  %92 = load i32*, i32** %sets26147, align 8
  %93 = load i32, i32* %set_id155, align 4
  %94 = load i32, i32* %max_member148, align 4
  %mul54 = mul i32 %93, %94
  %95 = load i32, i32* %iP157, align 4
  %add55 = add i32 %mul54, %95
  %idxprom56 = zext i32 %add55 to i64
  %ptridx57 = getelementptr inbounds i32, i32* %92, i64 %idxprom56
  %96 = load i32, i32* %ptridx57, align 4
  store i32 %96, i32* %ibead158, align 4
  %97 = bitcast i32* %iwtype159 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %97) #5
  %98 = load i32*, i32** %poly_arch139, align 8
  %99 = load i32*, i32** %poly_type_offset140, align 8
  %100 = load i32, i32* %poly_type142, align 4
  %idxprom58 = zext i32 %100 to i64
  %ptridx59 = getelementptr inbounds i32, i32* %99, i64 %idxprom58
  %101 = load i32, i32* %ptridx59, align 4
  %add60 = add nsw i32 %101, 1
  %102 = load i32, i32* %ibead158, align 4
  %add61 = add i32 %add60, %102
  %idxprom62 = zext i32 %add61 to i64
  %ptridx63 = getelementptr inbounds i32, i32* %98, i64 %idxprom62
  %103 = load i32, i32* %ptridx63, align 4
  %call64 = call i32 @get_particle_type(i32 %103) #5
  store i32 %call64, i32* %iwtype159, align 4
  %104 = bitcast %struct.RNG_STATE* %my_state160 to i8*
  call void @llvm.lifetime.start.p0i8(i64 32, i8* %104) #5
  %105 = load %struct.RNG_STATE*, %struct.RNG_STATE** %set_states149, align 8
  %106 = load i32, i32* %iP157, align 4
  %idxprom65 = zext i32 %106 to i64
  %ptridx66 = getelementptr inbounds %struct.RNG_STATE, %struct.RNG_STATE* %105, i64 %idxprom65
  %107 = bitcast %struct.RNG_STATE* %ptridx66 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 dereferenceable(32) %104, i8* nonnull align 8 dereferenceable(32) %107, i64 32, i1 false)
  %108 = bitcast %struct.Monomer* %mybead161 to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* %108) #5
  %109 = load %struct.Phase*, %struct.Phase** %p.addr169, align 8
  %allocator = getelementptr inbounds %struct.Phase, %struct.Phase* %109, i64 0, i32 59
  %110 = load %struct.Allocator*, %struct.Allocator** %allocator, align 8
  %device_buf = getelementptr inbounds %struct.Allocator, %struct.Allocator* %110, i64 0, i32 0, i32 3
  %111 = load %struct.Monomer*, %struct.Monomer** %device_buf, align 8
  %112 = load %struct.Polymer*, %struct.Polymer** %mypoly141, align 8
  %beads = getelementptr inbounds %struct.Polymer, %struct.Polymer* %112, i64 0, i32 0
  %113 = load i64, i64* %beads, align 8
  %114 = load i32, i32* %ibead158, align 4
  %conv = zext i32 %114 to i64
  %add67 = add i64 %113, %conv
  %ptridx68 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %111, i64 %add67
  %115 = bitcast %struct.Monomer* %ptridx68 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 dereferenceable(24) %108, i8* nonnull align 8 dereferenceable(24) %115, i64 24, i1 false)
  %116 = bitcast %struct.Monomer* %dx162 to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* %116) #5
  %z = getelementptr inbounds %struct.Monomer, %struct.Monomer* %dx162, i64 0, i32 2
  store double 0.000000e+00, double* %z, align 8
  %y = getelementptr inbounds %struct.Monomer, %struct.Monomer* %dx162, i64 0, i32 1
  store double 0.000000e+00, double* %y, align 8
  %x = getelementptr inbounds %struct.Monomer, %struct.Monomer* %dx162, i64 0, i32 0
  store double 0.000000e+00, double* %x, align 8
  %117 = bitcast double* %smc_deltaE163 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %117) #5
  store double 0.000000e+00, double* %smc_deltaE163, align 8
  %118 = load %struct.Phase*, %struct.Phase** %p.addr169, align 8
  %move_type_arg = getelementptr inbounds %struct.Phase, %struct.Phase* %118, i64 0, i32 46, i32 25
  %119 = load i32, i32* %move_type_arg, align 8
  switch i32 %119, label %sw.default [
    i32 0, label %sw.bb
    i32 1, label %sw.bb73
  ]

sw.bb:                                            ; preds = %for.body53
  %120 = load i32, i32* %ibead158, align 4
  %121 = load i32, i32* %iwtype159, align 4
  %122 = load i32, i32* %my_rng_type170, align 4
  call void @trial_move(%struct.Phase* %118, i64 undef, i32 %120, double* nonnull %x, double* nonnull %y, double* nonnull %z, i32 %121, i32 %122, %struct.RNG_STATE* %my_state160) #5
  store double 0.000000e+00, double* %smc_deltaE163, align 8
  br label %sw.epilog

sw.bb73:                                          ; preds = %for.body53
  %123 = load i64, i64* %npoly137, align 8
  %124 = load i32, i32* %ibead158, align 4
  %125 = load i32, i32* %my_rng_type170, align 4
  %126 = load i32, i32* %iwtype159, align 4
  call void @trial_move_smc(%struct.Phase* %118, i64 %123, i32 %124, double* nonnull %x, double* nonnull %y, double* nonnull %z, double* nonnull %smc_deltaE163, %struct.Monomer* %mybead161, %struct.RNG_STATE* %my_state160, i32 %125, i32 %126) #5
  br label %sw.epilog

sw.default:                                       ; preds = %for.body53
  store double 0.000000e+00, double* %smc_deltaE163, align 8
  br label %sw.epilog

sw.epilog:                                        ; preds = %sw.default, %sw.bb73, %sw.bb
  %127 = bitcast i32* %move_allowed164 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %127) #5
  %128 = load %struct.Phase*, %struct.Phase** %p.addr169, align 8
  %x78 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %mybead161, i64 0, i32 0
  %129 = load double, double* %x78, align 8
  %y79 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %mybead161, i64 0, i32 1
  %130 = load double, double* %y79, align 8
  %z80 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %mybead161, i64 0, i32 2
  %131 = load double, double* %z80, align 8
  %132 = load double, double* %x, align 8
  %133 = load double, double* %y, align 8
  %134 = load double, double* %z, align 8
  %135 = load i32, i32* %nonexact_area51171, align 4
  %call84 = call i32 @possible_move_area51(%struct.Phase* %128, double %129, double %130, double %131, double %132, double %133, double %134, i32 %135) #5
  store i32 %call84, i32* %move_allowed164, align 4
  %tobool = icmp eq i32 %call84, 0
  br i1 %tobool, label %if.end115, label %if.then

if.then:                                          ; preds = %sw.epilog
  %136 = bitcast double* %delta_energy165 to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* %136) #5
  %137 = load %struct.Phase*, %struct.Phase** %p.addr169, align 8
  %138 = load i64, i64* %npoly137, align 8
  %139 = load i32, i32* %ibead158, align 4
  %140 = load double, double* %x, align 8
  %141 = load double, double* %y, align 8
  %142 = load double, double* %z, align 8
  %143 = load i32, i32* %iwtype159, align 4
  %call88 = call fast double @calc_delta_energy(%struct.Phase* %137, i64 %138, %struct.Monomer* %mybead161, i32 %139, double %140, double %141, double %142, i32 %143) #5
  %144 = load double, double* %smc_deltaE163, align 8
  %add89 = fadd fast double %call88, %144
  store double %add89, double* %delta_energy165, align 8
  %145 = load i32, i32* %my_rng_type170, align 4
  %call90 = call zeroext i1 @som_accept(%struct.RNG_STATE* %my_state160, i32 %145, double %add89) #5
  br i1 %call90, label %if.then94, label %if.end

if.then94:                                        ; preds = %if.then
  %146 = bitcast %struct.Monomer* %newx166 to i8*
  call void @llvm.lifetime.start.p0i8(i64 24, i8* %146) #5
  %147 = load double, double* %x78, align 8
  %148 = load double, double* %x, align 8
  %add97 = fadd fast double %147, %148
  %x98 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %newx166, i64 0, i32 0
  store double %add97, double* %x98, align 8
  %149 = load double, double* %y79, align 8
  %150 = load double, double* %y, align 8
  %add101 = fadd fast double %149, %150
  %y102 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %newx166, i64 0, i32 1
  store double %add101, double* %y102, align 8
  %151 = load double, double* %z80, align 8
  %152 = load double, double* %z, align 8
  %add105 = fadd fast double %151, %152
  %z106 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %newx166, i64 0, i32 2
  store double %add105, double* %z106, align 8
  %153 = load %struct.Phase*, %struct.Phase** %p.addr169, align 8
  %allocator107 = getelementptr inbounds %struct.Phase, %struct.Phase* %153, i64 0, i32 59
  %154 = load %struct.Allocator*, %struct.Allocator** %allocator107, align 8
  %device_buf109 = getelementptr inbounds %struct.Allocator, %struct.Allocator* %154, i64 0, i32 0, i32 3
  %155 = load %struct.Monomer*, %struct.Monomer** %device_buf109, align 8
  %156 = load %struct.Polymer*, %struct.Polymer** %mypoly141, align 8
  %beads110 = getelementptr inbounds %struct.Polymer, %struct.Polymer* %156, i64 0, i32 0
  %157 = load i64, i64* %beads110, align 8
  %158 = load i32, i32* %ibead158, align 4
  %conv111 = zext i32 %158 to i64
  %add112 = add i64 %157, %conv111
  %ptridx113 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %155, i64 %add112
  %159 = bitcast %struct.Monomer* %ptridx113 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 dereferenceable(24) %159, i8* nonnull align 8 dereferenceable(24) %146, i64 24, i1 false)
  %160 = load i32, i32* %accepted_moves_set154, align 4
  %add114 = add i32 %160, 1
  store i32 %add114, i32* %accepted_moves_set154, align 4
  call void @llvm.lifetime.end.p0i8(i64 24, i8* %146) #5
  br label %if.end

if.end:                                           ; preds = %if.then94, %if.then
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %136) #5
  br label %if.end115

if.end115:                                        ; preds = %if.end, %sw.epilog
  %161 = load %struct.Polymer*, %struct.Polymer** %mypoly141, align 8
  %set_states116 = getelementptr inbounds %struct.Polymer, %struct.Polymer* %161, i64 0, i32 4
  %162 = load %struct.RNG_STATE*, %struct.RNG_STATE** %set_states116, align 8
  %163 = load i32, i32* %iP157, align 4
  %idxprom117 = zext i32 %163 to i64
  %ptridx118 = getelementptr inbounds %struct.RNG_STATE, %struct.RNG_STATE* %162, i64 %idxprom117
  %164 = bitcast %struct.RNG_STATE* %ptridx118 to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* nonnull align 8 dereferenceable(32) %164, i8* nonnull align 8 dereferenceable(32) %104, i64 32, i1 false)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %127) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %117) #5
  call void @llvm.lifetime.end.p0i8(i64 24, i8* %116) #5
  call void @llvm.lifetime.end.p0i8(i64 24, i8* %108) #5
  call void @llvm.lifetime.end.p0i8(i64 32, i8* %104) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %97) #5
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %91) #5
  %165 = load i32, i32* %iP157, align 4
  %inc120 = add i32 %165, 1
  br label %for.cond50

DIR.OMP.END.PARALLEL.LOOP.4:                      ; preds = %DIR.OMP.PARALLEL.LOOP.3181, %omp.inner.for.cond
  call void @llvm.directive.region.exit(token %11) [ "DIR.OMP.END.PARALLEL.LOOP"() ]
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.PARALLEL.LOOP.4, %for.body
  %166 = bitcast i64* %.omp.ub to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %166) #5
  %167 = bitcast i64* %.omp.lb to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %167) #5
  %168 = bitcast i64* %.omp.iv to i8*
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %168) #5
  %169 = load %struct.Phase*, %struct.Phase** %p.addr, align 8
  %time = getelementptr inbounds %struct.Phase, %struct.Phase* %169, i64 0, i32 24
  %170 = load i32, i32* %time, align 8
  %add128 = add i32 %170, 1
  store i32 %add128, i32* %time, align 8
  %num_all_beads_local = getelementptr inbounds %struct.Phase, %struct.Phase* %169, i64 0, i32 26
  %171 = load i64, i64* %num_all_beads_local, align 8
  %n_moves = getelementptr inbounds %struct.Phase, %struct.Phase* %169, i64 0, i32 37
  %172 = load i64, i64* %n_moves, align 8
  %add129 = add i64 %172, %171
  store i64 %add129, i64* %n_moves, align 8
  %173 = load i32, i32* %n_accepts, align 4
  %conv130 = zext i32 %173 to i64
  %174 = load %struct.Phase*, %struct.Phase** %p.addr, align 8
  %n_accepts131 = getelementptr inbounds %struct.Phase, %struct.Phase* %174, i64 0, i32 38
  %175 = load i64, i64* %n_accepts131, align 8
  %add132 = add i64 %175, %conv130
  store i64 %add132, i64* %n_accepts131, align 8
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %7) #5
  %inc134 = add i32 %step.0, 1
  br label %for.cond
}

define void @trial_move_smc(%struct.Phase* %p, i64 %ipoly, i32 %ibead, double* %dx, double* %dy, double* %dz, double* %smc_deltaE, %struct.Monomer* %mybead, %struct.RNG_STATE* %myrngstate, i32 %rng_type, i32 %iwtype) local_unnamed_addr {
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
  %x1 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %mybead, i64 0, i32 0
  %0 = load double, double* %x1, align 8
  %y2 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %mybead, i64 0, i32 1
  %1 = load double, double* %y2, align 8
  %z3 = getelementptr inbounds %struct.Monomer, %struct.Monomer* %mybead, i64 0, i32 2
  %2 = load double, double* %z3, align 8
  %A4 = getelementptr inbounds %struct.Phase, %struct.Phase* %p, i64 0, i32 18
  %3 = load double*, double** %A4, align 8
  %idxprom = zext i32 %iwtype to i64
  %ptridx = getelementptr inbounds double, double* %3, i64 %idxprom
  %4 = load double, double* %ptridx, align 8
  %R5 = getelementptr inbounds %struct.Phase, %struct.Phase* %p, i64 0, i32 19
  %5 = load double*, double** %R5, align 8
  %ptridx7 = getelementptr inbounds double, double* %5, i64 %idxprom
  %6 = load double, double* %ptridx7, align 8
  %7 = bitcast double* %fx to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %7) #5
  store double 0.000000e+00, double* %fx, align 8
  %8 = bitcast double* %fy to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %8) #5
  store double 0.000000e+00, double* %fy, align 8
  %9 = bitcast double* %fz to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %9) #5
  store double 0.000000e+00, double* %fz, align 8
  call void @add_bond_forces(%struct.Phase* %p, i64 %ipoly, i32 %ibead, double %0, double %1, double %2, double* nonnull %fx, double* nonnull %fy, double* nonnull %fz)
  %10 = bitcast double* %rx to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %10) #5
  %11 = bitcast double* %ry to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %11) #5
  %12 = bitcast double* %rz to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %12) #5
  call void @soma_normal_vector(%struct.RNG_STATE* %myrngstate, i32 %rng_type, double* nonnull %rx, double* nonnull %ry, double* nonnull %rz) #5
  %13 = load double, double* %fx, align 8
  %mul = fmul fast double %4, %13
  %14 = load double, double* %rx, align 8
  %mul8 = fmul fast double %14, %6
  %add = fadd fast double %mul, %mul8
  store double %add, double* %dx, align 8
  %15 = load double, double* %fy, align 8
  %mul9 = fmul fast double %4, %15
  %16 = load double, double* %ry, align 8
  %mul10 = fmul fast double %16, %6
  %add11 = fadd fast double %mul9, %mul10
  store double %add11, double* %dy, align 8
  %17 = load double, double* %fz, align 8
  %mul12 = fmul fast double %4, %17
  %18 = load double, double* %rz, align 8
  %mul13 = fmul fast double %18, %6
  %add14 = fadd fast double %mul12, %mul13
  store double %add14, double* %dz, align 8
  %19 = load double, double* %dx, align 8
  %add15 = fadd fast double %0, %19
  %20 = load double, double* %dy, align 8
  %add16 = fadd fast double %1, %20
  %add17 = fadd fast double %2, %add14
  %21 = bitcast double* %nfx to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %21) #5
  store double 0.000000e+00, double* %nfx, align 8
  %22 = bitcast double* %nfy to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %22) #5
  store double 0.000000e+00, double* %nfy, align 8
  %23 = bitcast double* %nfz to i8*
  call void @llvm.lifetime.start.p0i8(i64 8, i8* nonnull %23) #5
  store double 0.000000e+00, double* %nfz, align 8
  call void @add_bond_forces(%struct.Phase* %p, i64 %ipoly, i32 %ibead, double %add15, double %add16, double %add17, double* nonnull %nfx, double* nonnull %nfy, double* nonnull %nfz)
  store double 0.000000e+00, double* %smc_deltaE, align 8
  %24 = load double, double* %nfx, align 8
  %25 = load double, double* %fx, align 8
  %add18 = fadd fast double %24, %25
  %26 = load double, double* %dx, align 8
  %mul19 = fmul fast double %add18, %26
  %27 = load double, double* %nfy, align 8
  %28 = load double, double* %fy, align 8
  %add20 = fadd fast double %27, %28
  %29 = load double, double* %dy, align 8
  %mul21 = fmul fast double %add20, %29
  %add22 = fadd fast double %mul19, %mul21
  %30 = load double, double* %nfz, align 8
  %31 = load double, double* %fz, align 8
  %add23 = fadd fast double %30, %31
  %32 = load double, double* %dz, align 8
  %mul24 = fmul fast double %add23, %32
  %add25 = fadd fast double %add22, %mul24
  %mul26 = fmul fast double %add25, 5.000000e-01
  store double %mul26, double* %smc_deltaE, align 8
  %mul28 = fmul fast double %4, 2.500000e-01
  %33 = load double, double* %nfx, align 8
  %mul29 = fmul fast double %33, %33
  %34 = load double, double* %nfy, align 8
  %mul30 = fmul fast double %34, %34
  %add31 = fadd fast double %mul29, %mul30
  %35 = load double, double* %nfz, align 8
  %mul32 = fmul fast double %35, %35
  %add33 = fadd fast double %add31, %mul32
  %36 = load double, double* %fx, align 8
  %mul34 = fmul fast double %36, %36
  %37 = load double, double* %fy, align 8
  %mul35 = fmul fast double %37, %37
  %38 = fadd fast double %mul34, %mul35
  %39 = load double, double* %fz, align 8
  %mul37 = fmul fast double %39, %39
  %40 = fadd fast double %38, %mul37
  %sub38 = fsub fast double %add33, %40
  %mul39 = fmul fast double %mul28, %sub38
  %add40 = fadd fast double %mul26, %mul39
  store double %add40, double* %smc_deltaE, align 8
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %23) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %22) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %21) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %12) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %11) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %10) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %9) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %8) #5
  call void @llvm.lifetime.end.p0i8(i64 8, i8* nonnull %7) #5
  ret void
}

attributes #0 = { "may-have-openmp-directive"="true" }
; end INTEL_FEATURE_SW_ADVANCED
