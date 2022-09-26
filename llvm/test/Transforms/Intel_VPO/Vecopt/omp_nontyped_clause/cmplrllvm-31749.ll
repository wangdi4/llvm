; INTEL_FEATURE_SW_ADVANCED
; REQUIRES: intel_feature_sw_advanced

; RUN: opt < %s -S -vplan-vec -enable-intel-advanced-opts | FileCheck %s
;
; The run line below used to crash w/o the fix to enable DA recalculation in clones
; RUN: opt < %s -S -vplan-vec -enable-intel-advanced-opts -vplan-enable-non-masked-vectorized-remainder | FileCheck %s

; The test is generated from LAMMPS source code and corresponds to one of this
; application hot loops.
; We want this loop to be vectorized with VF=16. The test contains OMP loop
; and enforces prefered vector register width = 512 through function attributes.
;
; In order to ensure that the loop is vectorized with VF=16 we require '16 x'
; type to present in output multiple times.

; CHECK-LABEL: define
; CHECK-COUNT-50: <16 x

target triple = "x86_64-unknown-linux-gnu"

%struct.ident_t = type { i32, i32, i32, i32, i8* }
%"class.LAMMPS_NS::IntelBuffers.0" = type { i32, i32, %"class.LAMMPS_NS::LAMMPS"*, %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"*, float*, %"struct.LAMMPS_NS::IntelBuffers<float, double>::quat_t"*, %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"*, i32, i32, i32, i32*, i32*, i32*, i32*, %"struct.LAMMPS_NS::IntelNeighListPtrs"*, i32, i32, float**, float**, i32, i32, float*, float*, float*, float*, i32*, i32*, i32, i32, float*, float*, float*, i32*, i32*, i32*, i32, i32, i32, double*, i32, i32, [48 x i8], [8 x double], [8 x double] }
%"class.LAMMPS_NS::LAMMPS" = type { %"class.LAMMPS_NS::Memory"*, %"class.LAMMPS_NS::Error"*, %"class.LAMMPS_NS::Universe"*, %"class.LAMMPS_NS::Input"*, %"class.LAMMPS_NS::Atom"*, %"class.LAMMPS_NS::Update"*, %"class.LAMMPS_NS::Neighbor"*, %"class.LAMMPS_NS::Comm"*, %"class.LAMMPS_NS::Domain"*, %"class.LAMMPS_NS::Force"*, %"class.LAMMPS_NS::Modify"*, %"class.LAMMPS_NS::Group"*, %"class.LAMMPS_NS::Output"*, %"class.LAMMPS_NS::Timer"*, i8*, i32, i32, %struct._IO_FILE*, %struct._IO_FILE*, %struct._IO_FILE*, double, i8*, i8*, i8*, i32, i8*, i8***, i32, i32, i8*, i32, %"class.LAMMPS_NS::KokkosLMP"*, %"class.LAMMPS_NS::AtomKokkos"*, %"class.LAMMPS_NS::MemoryKokkos"*, %"class.LAMMPS_NS::Python"*, %"class.LAMMPS_NS::CiteMe"*, %"struct.LAMMPS_NS::package_styles_lists"* }
%"class.LAMMPS_NS::Memory" = type { %"class.LAMMPS_NS::Pointers" }
%"class.LAMMPS_NS::Pointers" = type { i32 (...)**, %"class.LAMMPS_NS::LAMMPS"*, %"class.LAMMPS_NS::Memory"**, %"class.LAMMPS_NS::Error"**, %"class.LAMMPS_NS::Universe"**, %"class.LAMMPS_NS::Input"**, %"class.LAMMPS_NS::Atom"**, %"class.LAMMPS_NS::Update"**, %"class.LAMMPS_NS::Neighbor"**, %"class.LAMMPS_NS::Comm"**, %"class.LAMMPS_NS::Domain"**, %"class.LAMMPS_NS::Force"**, %"class.LAMMPS_NS::Modify"**, %"class.LAMMPS_NS::Group"**, %"class.LAMMPS_NS::Output"**, %"class.LAMMPS_NS::Timer"**, i32*, %struct._IO_FILE**, %struct._IO_FILE**, %struct._IO_FILE**, %"class.LAMMPS_NS::AtomKokkos"**, %"class.LAMMPS_NS::MemoryKokkos"**, %"class.LAMMPS_NS::Python"** }
%"class.LAMMPS_NS::Error" = type <{ %"class.LAMMPS_NS::Pointers", i32, i32, i32, [4 x i8] }>
%"class.LAMMPS_NS::Universe" = type opaque
%"class.LAMMPS_NS::Input" = type opaque
%"class.LAMMPS_NS::Atom" = type { %"class.LAMMPS_NS::Pointers", i8*, %"class.LAMMPS_NS::AtomVec"*, i64, i32, i32, i32, i32, i32, i64, i64, i64, i64, i64, i64, i64, i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i8*, i32*, i32*, i32*, i32*, double**, double**, double**, double*, double*, double**, double*, double**, double**, double**, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32**, i32**, i32, i32*, i32**, i32**, i32*, i32**, i32**, i32**, i32**, i32*, i32**, i32**, i32**, i32**, i32**, i32*, i32**, i32**, i32**, i32**, i32**, double*, double*, double**, double**, double**, double**, i32*, double*, double*, double*, double*, double**, double**, double**, i32*, i32*, double*, double*, double*, double*, double*, double*, double*, i32, double**, double**, double*, double*, double*, double*, i32, double*, i32*, i32**, double*, double**, double**, double*, double*, double*, double*, double*, double*, double*, double*, double**, double*, double*, double*, double*, double*, double*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, double, %"struct.LAMMPS_NS::Atom::PerAtom"*, i32, i32, i32**, double**, i8**, i8**, i32, i32, i32, %"class.LAMMPS_NS::Molecule"**, double**, double*, i32*, i32, i32, i32, i32*, i32*, i32*, i32, i32, i32, i32, i32, i32, i32, %"class.std::set"*, i32, i64, double, i32*, %"class.std::map.12"*, i32*, i32, i32, i32, i32, i32, i32*, %"struct.LAMMPS_NS::Atom::HashElem"*, i32, i32, i32, i32, i32, i32, i32, i32*, i32*, i32*, double, double, double, [3 x double], [3 x double] }
%"class.LAMMPS_NS::AtomVec" = type opaque
%"struct.LAMMPS_NS::Atom::PerAtom" = type { i8*, i8*, i8*, i32*, i32, i32, i32, i32 }
%"class.LAMMPS_NS::Molecule" = type opaque
%"class.std::set" = type opaque
%"class.std::map.12" = type opaque
%"struct.LAMMPS_NS::Atom::HashElem" = type { i32, i32, i32 }
%"class.LAMMPS_NS::Update" = type { %"class.LAMMPS_NS::Pointers", double, double, double, i64, i32, i32, double, i64, i64, i64, i64, i64, i32, i32, i32, i32, i32, i32, i32, i64, i64, i64, i64, i8*, %"class.LAMMPS_NS::Integrate"*, i8*, %"class.LAMMPS_NS::Min"*, i8*, %"class.std::map.10"*, %"class.std::map.11"* }
%"class.LAMMPS_NS::Integrate" = type opaque
%"class.LAMMPS_NS::Min" = type opaque
%"class.std::map.10" = type opaque
%"class.std::map.11" = type opaque
%"class.LAMMPS_NS::Neighbor" = type <{ %"class.LAMMPS_NS::Pointers", i32, i32, i32, i32, i32, i32, i32, i32, i32, [4 x i8], double, double, double, double, double**, double**, double*, double*, double, double, double, i32, [4 x i8], double, i64, i64, i64, double*, double*, i32, i32, i32*, i32*, i32**, i32, [4 x i8], i32*, i32*, i32*, i32*, i32, [4 x i8], i32*, i32*, i32*, [4 x i32], i32, i32, i32, i32, %"class.LAMMPS_NS::NeighList"**, %"class.LAMMPS_NS::NeighRequest"**, %"class.LAMMPS_NS::NeighRequest"**, i32, [4 x i8], i32**, i32, [4 x i8], i32**, i32, [4 x i8], i32**, i32, [4 x i8], i32**, i32, i32, i32, i32, i32, [4 x i8], i32*, double*, double**, i32*, i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, [4 x i8], i32*, double, double**, i32, i32, [3 x double], [3 x double], [8 x [3 x double]], [3 x double]*, [2 x double], [2 x double], i32, i32, i32, i32, i32, i32, i32*, i32*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %"class.LAMMPS_NS::NBin"* (%"class.LAMMPS_NS::LAMMPS"*)**, i8**, i32*, %"class.LAMMPS_NS::NBin"**, %"class.LAMMPS_NS::NStencil"* (%"class.LAMMPS_NS::LAMMPS"*)**, i8**, i32*, %"class.LAMMPS_NS::NStencil"**, %"class.LAMMPS_NS::NPair"* (%"class.LAMMPS_NS::LAMMPS"*)**, i8**, i32*, %"class.LAMMPS_NS::NPair"**, %"class.LAMMPS_NS::NTopo"*, %"class.LAMMPS_NS::NTopo"*, %"class.LAMMPS_NS::NTopo"*, %"class.LAMMPS_NS::NTopo"*, i32, [4 x i8] }>
%"class.LAMMPS_NS::NeighList" = type { %"class.LAMMPS_NS::Pointers", i8*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32*, i32*, i32**, i32, i32, i32, %"class.LAMMPS_NS::MyPage"*, i32, i32, i32*, i32*, i32**, i32, i32, i32*, i32*, i32**, %"class.LAMMPS_NS::MyPage"*, %"class.LAMMPS_NS::MyPage"*, i32*, i32**, %"class.LAMMPS_NS::NeighList"*, %"class.LAMMPS_NS::NeighList"*, %"class.LAMMPS_NS::NeighList"*, %"class.LAMMPS_NS::Fix"*, i32, i32, %"class.LAMMPS_NS::NPair"* }
%"class.LAMMPS_NS::MyPage" = type opaque
%"class.LAMMPS_NS::Fix" = type <{ %"class.LAMMPS_NS::Pointers", i8*, i8*, i32, i32, i32, i32, i32, i32, i32, [4 x i8], i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32*, i32, [4 x i8], double*, double**, double*, double**, i32, i32, i32, [4 x i8], [6 x double], double*, double**, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [4 x i8] }>
%"class.LAMMPS_NS::NPair" = type opaque
%"class.LAMMPS_NS::NeighRequest" = type { %"class.LAMMPS_NS::Pointers", i32, i8*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, double, i32, i32*, i32**, i8*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }
%"class.LAMMPS_NS::NBin" = type opaque
%"class.LAMMPS_NS::NStencil" = type opaque
%"class.LAMMPS_NS::NTopo" = type opaque
%"class.LAMMPS_NS::Comm" = type { %"class.LAMMPS_NS::Pointers", i32, i32, i32, i32, i32, i32, [3 x double], double, double*, double*, i32, i32, i32, i32, i32, i32, [3 x i32], [3 x i32], [3 x i32], [3 x [2 x i32]], double*, double*, double*, i32***, i32, [3 x [2 x double]], double, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [4 x i8], i8*, i8*, i32, i32, [3 x i32], [3 x i32], i32, [3 x i32], [3 x i32], i32 }
%"class.LAMMPS_NS::Domain" = type opaque
%"class.LAMMPS_NS::Force" = type <{ %"class.LAMMPS_NS::Pointers", double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, double, i32, i32, i32, [4 x i8], %"class.LAMMPS_NS::Pair"*, i8*, i8*, %"class.LAMMPS_NS::Bond"*, i8*, %"class.LAMMPS_NS::Angle"*, i8*, %"class.LAMMPS_NS::Dihedral"*, i8*, %"class.LAMMPS_NS::Improper"*, i8*, %"class.LAMMPS_NS::KSpace"*, i8*, %"class.std::map.4"*, %"class.std::map.5"*, %"class.std::map.6"*, %"class.std::map.7"*, %"class.std::map.8"*, %"class.std::map.9"*, [4 x double], [4 x double], i32, i32, i32, [4 x i8] }>
%"class.LAMMPS_NS::Pair" = type <{ %"class.LAMMPS_NS::Pointers", double, double, [6 x double], double*, double**, double**, double, double**, i32**, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, double**, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, double, double, double, double, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, double, double, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, i32, i32, i32, i32, i32, [4 x i8], double*, i32, [4 x i8], double*, %"class.LAMMPS_NS::NeighList"*, %"class.LAMMPS_NS::NeighList"*, %"class.LAMMPS_NS::NeighList"*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %"class.LAMMPS_NS::Compute"**, i32, [4 x i32], i32, i32, i32, double, double, i32, [4 x i8], i8**, i32*, i32**, i32***, i32*, i32, i32, i32, i32, i32, i32, i32, [4 x i8] }>
%"class.LAMMPS_NS::Compute" = type opaque
%"class.LAMMPS_NS::Bond" = type opaque
%"class.LAMMPS_NS::Angle" = type opaque
%"class.LAMMPS_NS::Dihedral" = type opaque
%"class.LAMMPS_NS::Improper" = type opaque
%"class.LAMMPS_NS::KSpace" = type opaque
%"class.std::map.4" = type opaque
%"class.std::map.5" = type opaque
%"class.std::map.6" = type opaque
%"class.std::map.7" = type opaque
%"class.std::map.8" = type opaque
%"class.std::map.9" = type opaque
%"class.LAMMPS_NS::Modify" = type { %"class.LAMMPS_NS::Pointers", i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %"class.LAMMPS_NS::Fix"**, i32*, i32, i32, %"class.LAMMPS_NS::Compute"**, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32*, i32, i32*, i8**, i8**, i8**, i32*, i8**, i8**, i32*, i32*, i32, %"class.std::map"*, %"class.std::map.3"* }
%"class.std::map" = type opaque
%"class.std::map.3" = type opaque
%"class.LAMMPS_NS::Group" = type opaque
%"class.LAMMPS_NS::Output" = type opaque
%"class.LAMMPS_NS::Timer" = type opaque
%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }
%"class.LAMMPS_NS::KokkosLMP" = type opaque
%"class.LAMMPS_NS::AtomKokkos" = type opaque
%"class.LAMMPS_NS::MemoryKokkos" = type opaque
%"class.LAMMPS_NS::Python" = type opaque
%"class.LAMMPS_NS::CiteMe" = type opaque
%"struct.LAMMPS_NS::package_styles_lists" = type opaque
%"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t" = type { float, float, float, i32 }
%"struct.LAMMPS_NS::IntelBuffers<float, double>::quat_t" = type opaque
%"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t" = type { double, double, double, double }
%"struct.LAMMPS_NS::IntelNeighListPtrs" = type { i8*, i32*, i32*, i32 }
%"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1" = type { float, float, float, float }
%"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed2" = type { float, float }
%"class.LAMMPS_NS::PairLJCutIntel" = type { %"class.LAMMPS_NS::PairLJCut", %"class.LAMMPS_NS::FixIntel"*, i32, i32, [48 x i8], %"class.LAMMPS_NS::PairLJCutIntel::ForceConst", %"class.LAMMPS_NS::PairLJCutIntel::ForceConst.2" }
%"class.LAMMPS_NS::PairLJCut" = type { %"class.LAMMPS_NS::Pair.base", double, double**, double**, double**, double**, double**, double**, double**, double**, double* }
%"class.LAMMPS_NS::Pair.base" = type <{ %"class.LAMMPS_NS::Pointers", double, double, [6 x double], double*, double**, double**, double, double**, i32**, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, double**, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, double, double, double, double, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, double, double, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, double*, i32, i32, i32, i32, i32, [4 x i8], double*, i32, [4 x i8], double*, %"class.LAMMPS_NS::NeighList"*, %"class.LAMMPS_NS::NeighList"*, %"class.LAMMPS_NS::NeighList"*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, %"class.LAMMPS_NS::Compute"**, i32, [4 x i32], i32, i32, i32, double, double, i32, [4 x i8], i8**, i32*, i32**, i32***, i32*, i32, i32, i32, i32, i32, i32, i32 }>
%"class.LAMMPS_NS::FixIntel" = type <{ %"class.LAMMPS_NS::Fix.base", [4 x i8], %"class.LAMMPS_NS::IntelBuffers"*, %"class.LAMMPS_NS::IntelBuffers.0"*, %"class.LAMMPS_NS::IntelBuffers.1"*, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, [5 x i32], [60 x i8], [5 x i32], i32, i32, i32, %"struct.LAMMPS_NS::IntelBuffers<float, float>::vec3_acc_t"*, %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"*, %"struct.LAMMPS_NS::IntelBuffers<double, double>::vec3_acc_t"*, float*, double*, i32, i32, i32, [4 x i8], double, double, double, double, [8 x double], [8 x double], [8 x i8], [1 x double], [56 x i8], [1 x double], i32, i32, i32, [44 x i8] }>
%"class.LAMMPS_NS::Fix.base" = type <{ %"class.LAMMPS_NS::Pointers", i8*, i8*, i32, i32, i32, i32, i32, i32, i32, [4 x i8], i64, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32*, i32, [4 x i8], double*, double**, double*, double**, i32, i32, i32, [4 x i8], [6 x double], double*, double**, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32, i32 }>
%"class.LAMMPS_NS::IntelBuffers" = type { i32, i32, %"class.LAMMPS_NS::LAMMPS"*, %"struct.LAMMPS_NS::IntelBuffers<float, float>::atom_t"*, float*, %"struct.LAMMPS_NS::IntelBuffers<float, float>::quat_t"*, %"struct.LAMMPS_NS::IntelBuffers<float, float>::vec3_acc_t"*, i32, i32, i32, i32*, i32*, i32*, i32*, %"struct.LAMMPS_NS::IntelNeighListPtrs"*, i32, i32, float**, float**, i32, i32, float*, float*, float*, float*, i32*, i32*, i32, i32, float*, float*, float*, i32*, i32*, i32*, i32, i32, i32, float*, i32, i32, [48 x i8], [8 x float], [32 x i8], [8 x float], [32 x i8] }
%"struct.LAMMPS_NS::IntelBuffers<float, float>::atom_t" = type opaque
%"struct.LAMMPS_NS::IntelBuffers<float, float>::quat_t" = type opaque
%"class.LAMMPS_NS::IntelBuffers.1" = type { i32, i32, %"class.LAMMPS_NS::LAMMPS"*, %"struct.LAMMPS_NS::IntelBuffers<double, double>::atom_t"*, double*, %"struct.LAMMPS_NS::IntelBuffers<double, double>::quat_t"*, %"struct.LAMMPS_NS::IntelBuffers<double, double>::vec3_acc_t"*, i32, i32, i32, i32*, i32*, i32*, i32*, %"struct.LAMMPS_NS::IntelNeighListPtrs"*, i32, i32, double**, double**, i32, i32, double*, double*, double*, double*, i32*, i32*, i32, i32, double*, double*, double*, i32*, i32*, i32*, i32, i32, i32, double*, i32, i32, [48 x i8], [8 x double], [8 x double] }
%"struct.LAMMPS_NS::IntelBuffers<double, double>::atom_t" = type opaque
%"struct.LAMMPS_NS::IntelBuffers<double, double>::quat_t" = type opaque
%"struct.LAMMPS_NS::IntelBuffers<float, float>::vec3_acc_t" = type opaque
%"struct.LAMMPS_NS::IntelBuffers<double, double>::vec3_acc_t" = type opaque
%"class.LAMMPS_NS::PairLJCutIntel::ForceConst" = type { [4 x float], %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1"**, %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed2"**, i32, i32, %"class.LAMMPS_NS::Memory"*, [16 x i8] }
%"class.LAMMPS_NS::PairLJCutIntel::ForceConst.2" = type { [4 x double], %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<double>::fc_packed1"**, %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<double>::fc_packed2"**, i32, i32, %"class.LAMMPS_NS::Memory"* }
%"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<double>::fc_packed1" = type { double, double, double, double }
%"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<double>::fc_packed2" = type { double, double }
%struct.fast_red_t.14 = type <{ double, double, double, double, double, double, double }>

@.kmpc_loc.0.0 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.12 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.14 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.16 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.18 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.20 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.22 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.24 = external hidden unnamed_addr global %struct.ident_t
@.gomp_critical_user_.fast_reduction.AS0.var = external global [8 x i32]
@.kmpc_loc.0.0.26 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.28 = external hidden unnamed_addr global %struct.ident_t
@.kmpc_loc.0.0.30 = external hidden unnamed_addr global %struct.ident_t

declare dso_local i32 @__gxx_personality_v0(...)

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: argmemonly nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nofree nounwind readonly
declare dso_local i32 @omp_get_thread_num() local_unnamed_addr #2

; Function Attrs: argmemonly nofree nounwind willreturn writeonly
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1 immarg) #3

declare dso_local void @_ZN9LAMMPS_NS12IntelBuffersIfdE12fdotr_reduceEiiiRdS2_S2_S2_S2_S2_(%"class.LAMMPS_NS::IntelBuffers.0"* nonnull align 64 dereferenceable(448), i32, i32, i32, double* nonnull align 8 dereferenceable(8), double* nonnull align 8 dereferenceable(8), double* nonnull align 8 dereferenceable(8), double* nonnull align 8 dereferenceable(8), double* nonnull align 8 dereferenceable(8), double* nonnull align 8 dereferenceable(8)) local_unnamed_addr #4

; Function Attrs: convergent nofree nounwind
declare void @__kmpc_barrier(%struct.ident_t*, i32) local_unnamed_addr #5

; Function Attrs: inaccessiblememonly nofree nosync nounwind willreturn
declare void @llvm.assume(i1 noundef) #6

declare void @__kmpc_atomic_float8_add(%struct.ident_t*, i32, double*, double) local_unnamed_addr

; Function Attrs: mustprogress nofree norecurse nosync nounwind willreturn
declare hidden void @_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii_tree_reduce_13(i8* nocapture, i8* nocapture readonly) #7

declare i32 @__kmpc_reduce(%struct.ident_t*, i32, i32, i32, i8*, void (i8*, i8*)*, [8 x i32]*) local_unnamed_addr

; Function Attrs: convergent nounwind
declare void @__kmpc_end_reduce(%struct.ident_t*, i32, [8 x i32]*) local_unnamed_addr #8

; Function Attrs: uwtable
define hidden void @_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii.DIR.OMP.PARALLEL.2(i32* nocapture readonly %tid, i32* nocapture readnone %bid, i32* nocapture readonly %vflag.addr, %"class.LAMMPS_NS::IntelBuffers.0"** nocapture readonly %buffers.addr, i32* nocapture readonly %astart.addr, i32* nocapture readonly %inum, i32* nocapture readonly %minlocal, i32* nocapture readonly %nall, %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"** nocapture readonly %x, i32** nocapture readonly %ilist, i32** nocapture readonly %numneigh, i32*** nocapture readonly %firstneigh, float** nocapture readnone %special_lj, %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1"** nocapture readonly %ljc12o, %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed2"** nocapture readonly %lj34, i32* nocapture readonly %eatom, i32* nocapture readonly %f_stride, %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"** nocapture readonly %f_start, i32* nocapture readonly %nthreads17, %"class.LAMMPS_NS::PairLJCutIntel"* nocapture readonly %this, double* %oevdwl, double* %ov0, double* %ov1, double* %ov2, double* %ov3, double* %ov4, double* %ov5) #9 personality i32 (...)* @__gxx_personality_v0 {
newFuncRoot:
  %fast_red_struct459 = alloca %struct.fast_red_t.14, align 8
  %ov0.red = alloca double, align 8
  %ov1.red = alloca double, align 8
  %ov2.red = alloca double, align 8
  %ov3.red = alloca double, align 8
  %ov4.red = alloca double, align 8
  %ov5.red = alloca double, align 8
  %sv5.red = alloca double, align 8
  %sv4.red = alloca double, align 8
  %sv3.red = alloca double, align 8
  %sv2.red = alloca double, align 8
  %sv1.red = alloca double, align 8
  %sv0.red = alloca double, align 8
  %sevdwl.red = alloca double, align 8
  %fwtmp.red = alloca double, align 8
  %fztmp.red = alloca double, align 8
  %fytmp.red = alloca double, align 8
  %fxtmp.red = alloca double, align 8
  %jj.linear.iv = alloca i32, align 4
  %ov5.fast_red = getelementptr inbounds %struct.fast_red_t.14, %struct.fast_red_t.14* %fast_red_struct459, i64 0, i32 6
  store double 0.000000e+00, double* %ov5.red, align 8
  %ov4.fast_red = getelementptr inbounds %struct.fast_red_t.14, %struct.fast_red_t.14* %fast_red_struct459, i64 0, i32 5
  store double 0.000000e+00, double* %ov4.red, align 8
  %ov3.fast_red = getelementptr inbounds %struct.fast_red_t.14, %struct.fast_red_t.14* %fast_red_struct459, i64 0, i32 4
  store double 0.000000e+00, double* %ov3.red, align 8
  %ov2.fast_red = getelementptr inbounds %struct.fast_red_t.14, %struct.fast_red_t.14* %fast_red_struct459, i64 0, i32 3
  store double 0.000000e+00, double* %ov2.red, align 8
  %ov1.fast_red = getelementptr inbounds %struct.fast_red_t.14, %struct.fast_red_t.14* %fast_red_struct459, i64 0, i32 2
  store double 0.000000e+00, double* %ov1.red, align 8
  %ov0.fast_red = getelementptr inbounds %struct.fast_red_t.14, %struct.fast_red_t.14* %fast_red_struct459, i64 0, i32 1
  store double 0.000000e+00, double* %ov0.red, align 8
  %oevdwl.fast_red = getelementptr inbounds %struct.fast_red_t.14, %struct.fast_red_t.14* %fast_red_struct459, i64 0, i32 0
  %call20 = tail call i32 @omp_get_thread_num() #1, !range !4
  %0 = load i32, i32* %nthreads17, align 4, !tbaa !5, !alias.scope !9, !noalias !12
  %cmp21 = icmp slt i32 %0, 3
  br i1 %cmp21, label %if.then22, label %if.else

if.then22:                                        ; preds = %newFuncRoot
  %1 = load i32, i32* %inum, align 4, !tbaa !5, !alias.scope !168, !noalias !12
  %phi.cast = sext i32 %0 to i64
  br label %if.end57

if.else:                                          ; preds = %newFuncRoot
  %2 = and i32 %0, 1
  %cmp23 = icmp eq i32 %2, 0
  br i1 %cmp23, label %if.then24, label %if.else40

if.then24:                                        ; preds = %if.else
  %div118 = lshr i32 %0, 1
  %div25119 = lshr i32 %call20, 1
  %rem26120 = and i32 %call20, 1
  %3 = load i32, i32* %inum, align 4, !tbaa !5, !alias.scope !170, !noalias !12
  %div27 = sdiv i32 %3, %div118
  %rem28 = srem i32 %3, %div118
  %mul = mul nsw i32 %div27, %div25119
  %add29 = add nsw i32 %mul, %div27
  %cmp30 = icmp slt i32 %div25119, %rem28
  %add32 = add nuw nsw i32 %div25119, 1
  %4 = select i1 %cmp30, i32 %add32, i32 %rem28
  %5 = select i1 %cmp30, i32 %div25119, i32 %rem28
  %iito.priv.0 = add nsw i32 %add29, %4
  %storemerge408 = add i32 %mul, %rem26120
  %add39 = add i32 %storemerge408, %5
  br label %if.end57

if.else40:                                        ; preds = %if.else
  %6 = load i32, i32* %inum, align 4, !tbaa !5, !alias.scope !172, !noalias !12
  %div42 = sdiv i32 %6, %0
  %rem44 = srem i32 %6, %0
  %mul45 = mul nsw i32 %div42, %call20
  %add46 = add nsw i32 %mul45, %div42
  %cmp47 = icmp slt i32 %call20, %rem44
  br i1 %cmp47, label %if.then48, label %if.else52

if.then48:                                        ; preds = %if.else40
  %add49 = add nuw nsw i32 %call20, 1
  %add50 = add nsw i32 %add49, %add46
  %add51 = add nsw i32 %mul45, %call20
  br label %if.end57

if.else52:                                        ; preds = %if.else40
  %add53 = add nsw i32 %add46, %rem44
  %add54 = add nsw i32 %mul45, %rem44
  br label %if.end57

if.end57:                                         ; preds = %if.else52, %if.then48, %if.then24, %if.then22
  %iifrom.priv.0 = phi i32 [ %call20, %if.then22 ], [ %add39, %if.then24 ], [ %add54, %if.else52 ], [ %add51, %if.then48 ]
  %iip.priv.0 = phi i64 [ %phi.cast, %if.then22 ], [ 2, %if.then24 ], [ 1, %if.else52 ], [ 1, %if.then48 ]
  %iito.priv.2 = phi i32 [ %1, %if.then22 ], [ %iito.priv.0, %if.then24 ], [ %add53, %if.else52 ], [ %add50, %if.then48 ]
  %7 = load i32, i32* %astart.addr, align 4, !tbaa !5, !alias.scope !174, !noalias !12
  %8 = load i32, i32* %f_stride, align 4, !tbaa !5, !alias.scope !176, !noalias !12
  %mul60 = mul nsw i32 %8, %call20
  %9 = load i32, i32* %minlocal, align 4, !tbaa !5, !alias.scope !178, !noalias !12
  %sub61 = sub nsw i32 %mul60, %9
  %10 = load %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"*, %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"** %f_start, align 8, !tbaa !180, !alias.scope !182, !noalias !12
  %idx.ext = sext i32 %sub61 to i64
  %add.ptr = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %10, i64 %idx.ext, !intel-tbaa !184
  %idx.ext62 = sext i32 %9 to i64
  %add.ptr63 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 %idx.ext62, !intel-tbaa !184
  %11 = bitcast %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr63 to i8*
  %conv = sext i32 %8 to i64
  %mul64 = shl nsw i64 %conv, 5
  tail call void @llvm.memset.p0i8.i64(i8* align 8 %11, i8 0, i64 %mul64, i1 false)
  %12 = load %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1"*, %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1"** %ljc12o, align 8, !tbaa !187, !alias.scope !189, !noalias !12
  %_onetype = getelementptr inbounds %"class.LAMMPS_NS::PairLJCutIntel", %"class.LAMMPS_NS::PairLJCutIntel"* %this, i64 0, i32 3, !intel-tbaa !191
  %13 = load i32, i32* %_onetype, align 4, !tbaa !191, !alias.scope !204, !noalias !12
  %idxprom = sext i32 %13 to i64
  %cutsq66 = getelementptr inbounds %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1", %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1"* %12, i64 %idxprom, i32 0
  %14 = load float, float* %cutsq66, align 4, !tbaa !206, !alias.scope !208, !noalias !212
  %lj170 = getelementptr inbounds %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1", %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1"* %12, i64 %idxprom, i32 1
  %15 = load float, float* %lj170, align 4, !tbaa !222, !alias.scope !223, !noalias !212
  %lj274 = getelementptr inbounds %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1", %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1"* %12, i64 %idxprom, i32 2
  %16 = load float, float* %lj274, align 4, !tbaa !225, !alias.scope !226, !noalias !212
  %17 = load %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed2"*, %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed2"** %lj34, align 8, !tbaa !228, !alias.scope !230, !noalias !12
  %lj378 = getelementptr inbounds %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed2", %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed2"* %17, i64 %idxprom, i32 0
  %18 = load float, float* %lj378, align 4, !tbaa !232, !alias.scope !234, !noalias !236
  %lj482 = getelementptr inbounds %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed2", %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed2"* %17, i64 %idxprom, i32 1
  %19 = load float, float* %lj482, align 4, !tbaa !237, !alias.scope !238, !noalias !236
  %offset86 = getelementptr inbounds %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1", %"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1"* %12, i64 %idxprom, i32 3
  %20 = load float, float* %offset86, align 4, !tbaa !240, !alias.scope !241, !noalias !212
  %21 = bitcast i32* %jj.linear.iv to i8*
  %cmp87125 = icmp slt i32 %iifrom.priv.0, %iito.priv.2
  br i1 %cmp87125, label %for.body.preheader, label %for.cond.cleanup

for.body.preheader:                               ; preds = %if.end57
  %add59 = add nsw i32 %7, %iito.priv.2
  %22 = add i32 %iifrom.priv.0, %7
  %23 = sext i32 %22 to i64
  %24 = sext i32 %add59 to i64
  br label %for.body

for.cond.cleanup.loopexit:                        ; preds = %omp.precond.end
  %add202.lcssa = phi double [ %add202, %omp.precond.end ]
  br label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond.cleanup.loopexit, %if.end57
  %oevdwl.red.0.lcssa = phi double [ 0.000000e+00, %if.end57 ], [ %add202.lcssa, %for.cond.cleanup.loopexit ]
  %25 = load i32, i32* %vflag.addr, align 4, !tbaa !5, !alias.scope !243, !noalias !12
  %cmp204 = icmp eq i32 %25, 2
  br i1 %cmp204, label %land.lhs.true, label %if.end207

for.body:                                         ; preds = %omp.precond.end, %for.body.preheader
  %indvars.iv138 = phi i64 [ %23, %for.body.preheader ], [ %indvars.iv.next139, %omp.precond.end ]
  %oevdwl.red.0126 = phi double [ 0.000000e+00, %for.body.preheader ], [ %add202, %omp.precond.end ]
  %26 = load i32*, i32** %ilist, align 8, !tbaa !245, !alias.scope !247, !noalias !12
  %arrayidx89 = getelementptr inbounds i32, i32* %26, i64 %indvars.iv138
  %27 = load i32, i32* %arrayidx89, align 4, !tbaa !5, !alias.scope !249, !noalias !250
  %28 = load i32**, i32*** %firstneigh, align 8, !tbaa !254, !alias.scope !256, !noalias !12
  %idxprom90 = sext i32 %27 to i64
  %arrayidx91 = getelementptr inbounds i32*, i32** %28, i64 %idxprom90
  %29 = load i32*, i32** %arrayidx91, align 8, !tbaa !245, !alias.scope !258, !noalias !260
  %30 = load i32*, i32** %numneigh, align 8, !tbaa !245, !alias.scope !261, !noalias !12
  %arrayidx93 = getelementptr inbounds i32, i32* %30, i64 %idxprom90
  %31 = load i32, i32* %arrayidx93, align 4, !tbaa !5, !alias.scope !263, !noalias !264
  %sub96 = add i32 %31, 15
  %and = and i32 %sub96, -16
  %32 = load %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"*, %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"** %x, align 8, !tbaa !265, !alias.scope !267, !noalias !12
  %x100 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"* %32, i64 %idxprom90, i32 0
  %33 = load float, float* %x100, align 4, !tbaa !269, !alias.scope !271, !noalias !273
  %y = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"* %32, i64 %idxprom90, i32 1
  %34 = load float, float* %y, align 4, !tbaa !274, !alias.scope !275, !noalias !273
  %z = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"* %32, i64 %idxprom90, i32 2
  %35 = load float, float* %z, align 4, !tbaa !277, !alias.scope !278, !noalias !273
  %cmp110 = icmp sgt i32 %and, 0
  br i1 %cmp110, label %DIR.OMP.SIMD.1, label %omp.precond.end

DIR.OMP.SIMD.1:                                   ; preds = %for.body
  store double 0.000000e+00, double* %sv5.red, align 8
  store double 0.000000e+00, double* %sv4.red, align 8
  store double 0.000000e+00, double* %sv3.red, align 8
  store double 0.000000e+00, double* %sv2.red, align 8
  store double 0.000000e+00, double* %sv1.red, align 8
  store double 0.000000e+00, double* %sv0.red, align 8
  store double 0.000000e+00, double* %sevdwl.red, align 8
  store double 0.000000e+00, double* %fwtmp.red, align 8
  store double 0.000000e+00, double* %fztmp.red, align 8
  store double 0.000000e+00, double* %fytmp.red, align 8
  store double 0.000000e+00, double* %fxtmp.red, align 8
  br label %DIR.OMP.SIMD.1146

DIR.OMP.SIMD.1146:                                ; preds = %DIR.OMP.SIMD.1
  %36 = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"(), "QUAL.OMP.REDUCTION.ADD"(double* %fxtmp.red), "QUAL.OMP.REDUCTION.ADD"(double* %fytmp.red), "QUAL.OMP.REDUCTION.ADD"(double* %fztmp.red), "QUAL.OMP.REDUCTION.ADD"(double* %fwtmp.red), "QUAL.OMP.REDUCTION.ADD"(double* %sevdwl.red), "QUAL.OMP.REDUCTION.ADD"(double* %sv0.red), "QUAL.OMP.REDUCTION.ADD"(double* %sv1.red), "QUAL.OMP.REDUCTION.ADD"(double* %sv2.red), "QUAL.OMP.REDUCTION.ADD"(double* %sv3.red), "QUAL.OMP.REDUCTION.ADD"(double* %sv4.red), "QUAL.OMP.REDUCTION.ADD"(double* %sv5.red), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(i32** null, i32 64), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(%"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"** null, i32 64), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(%"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed1"** null, i32 64), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(float** null, i32 64), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(%"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"** null, i32 64), "QUAL.OMP.ALIGNED:PTR_TO_PTR"(%"struct.LAMMPS_NS::PairLJCutIntel::ForceConst<float>::fc_packed2"** null, i32 64), "QUAL.OMP.NORMALIZED.IV"(i8* null), "QUAL.OMP.NORMALIZED.UB"(i8* null), "QUAL.OMP.LINEAR:IV"(i32* %jj.linear.iv, i32 1) ]
  br label %DIR.OMP.SIMD.2

DIR.OMP.SIMD.2:                                   ; preds = %DIR.OMP.SIMD.1146
  call void @llvm.assume(i1 true) [ "align"(i32* %29, i64 64) ], !llvm.access.group !280
  %37 = load %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"*, %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"** %x, align 8, !tbaa !265, !alias.scope !281, !noalias !12, !llvm.access.group !280
  call void @llvm.assume(i1 true) [ "align"(%"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"* %37, i64 64) ], !llvm.access.group !280
  call void @llvm.assume(i1 true) [ "align"(%"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"* %37, i64 64) ], !llvm.access.group !280
  call void @llvm.assume(i1 true) [ "align"(%"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"* %37, i64 64) ], !llvm.access.group !280
  %fxtmp.red.promoted = load double, double* %fxtmp.red, align 8, !tbaa !283, !alias.scope !284, !noalias !285
  %fytmp.red.promoted = load double, double* %fytmp.red, align 8, !tbaa !283, !alias.scope !286, !noalias !287
  %fztmp.red.promoted = load double, double* %fztmp.red, align 8, !tbaa !283, !alias.scope !288, !noalias !289
  %sevdwl.red.promoted = load double, double* %sevdwl.red, align 8, !tbaa !283, !alias.scope !290, !noalias !291
  %fwtmp.red.promoted = load double, double* %fwtmp.red, align 8, !tbaa !283, !alias.scope !292, !noalias !293
  %wide.trip.count141 = zext i32 %and to i64
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %if.end184, %DIR.OMP.SIMD.2
  %indvars.iv = phi i64 [ 0, %DIR.OMP.SIMD.2 ], [ %indvars.iv.next, %if.end184 ]
  %38 = phi double [ %fwtmp.red.promoted, %DIR.OMP.SIMD.2 ], [ %52, %if.end184 ]
  %39 = phi double [ %sevdwl.red.promoted, %DIR.OMP.SIMD.2 ], [ %53, %if.end184 ]
  %40 = phi double [ %fztmp.red.promoted, %DIR.OMP.SIMD.2 ], [ %54, %if.end184 ]
  %41 = phi double [ %fytmp.red.promoted, %DIR.OMP.SIMD.2 ], [ %55, %if.end184 ]
  %42 = phi double [ %fxtmp.red.promoted, %DIR.OMP.SIMD.2 ], [ %56, %if.end184 ]
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %21) #1, !llvm.access.group !280
  %arrayidx115 = getelementptr inbounds i32, i32* %29, i64 %indvars.iv
  %43 = load i32, i32* %arrayidx115, align 4, !tbaa !5, !alias.scope !294, !noalias !295, !llvm.access.group !280
  %idxprom116 = sext i32 %43 to i64
  %x118 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"* %37, i64 %idxprom116, i32 0
  %44 = load float, float* %x118, align 16, !tbaa !269, !alias.scope !296, !noalias !273, !llvm.access.group !280
  %sub119 = fsub fast float %33, %44
  %y122 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"* %37, i64 %idxprom116, i32 1
  %45 = load float, float* %y122, align 4, !tbaa !274, !alias.scope !298, !noalias !273, !llvm.access.group !280
  %sub123 = fsub fast float %34, %45
  %z126 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::atom_t"* %37, i64 %idxprom116, i32 2
  %46 = load float, float* %z126, align 8, !tbaa !277, !alias.scope !300, !noalias !273, !llvm.access.group !280
  %sub127 = fsub fast float %35, %46
  %mul128 = fmul fast float %sub119, %sub119
  %mul129 = fmul fast float %sub123, %sub123
  %add130 = fadd fast float %mul129, %mul128
  %mul131 = fmul fast float %sub127, %sub127
  %add132 = fadd fast float %add130, %mul131
  %cmp133 = fcmp fast olt float %add132, %14
  br i1 %cmp133, label %if.then134, label %if.end184

if.then134:                                       ; preds = %omp.inner.for.body
  %conv137 = fdiv fast float 1.000000e+00, %add132
  %mul138 = fmul fast float %conv137, %conv137
  %mul139 = fmul fast float %mul138, %conv137
  %mul140 = fmul fast float %mul139, %15
  %sub141 = fsub fast float %mul140, %16
  %mul142 = fmul fast float %mul139, %conv137
  %mul143 = fmul fast float %mul142, %sub141
  %mul144 = fmul fast float %mul143, %sub119
  %conv145 = fpext float %mul144 to double
  %add146 = fadd fast double %42, %conv145
  call void @llvm.assume(i1 true) [ "align"(%"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 64) ], !llvm.access.group !280
  %x150 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 %idxprom116, i32 0
  %47 = load double, double* %x150, align 32, !tbaa !302, !alias.scope !303, !noalias !304, !llvm.access.group !280
  %sub151 = fsub fast double %47, %conv145
  store double %sub151, double* %x150, align 32, !tbaa !302, !alias.scope !305, !noalias !306, !llvm.access.group !280
  %mul152 = fmul fast float %mul143, %sub123
  %conv153 = fpext float %mul152 to double
  %add154 = fadd fast double %41, %conv153
  call void @llvm.assume(i1 true) [ "align"(%"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 64) ], !llvm.access.group !280
  %y158 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 %idxprom116, i32 1
  %48 = load double, double* %y158, align 8, !tbaa !330, !alias.scope !331, !noalias !332, !llvm.access.group !280
  %sub159 = fsub fast double %48, %conv153
  store double %sub159, double* %y158, align 8, !tbaa !330, !alias.scope !333, !noalias !334, !llvm.access.group !280
  %mul160 = fmul fast float %mul143, %sub127
  %conv161 = fpext float %mul160 to double
  %add162 = fadd fast double %40, %conv161
  call void @llvm.assume(i1 true) [ "align"(%"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 64) ], !llvm.access.group !280
  %z166 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 %idxprom116, i32 2
  %49 = load double, double* %z166, align 16, !tbaa !335, !alias.scope !336, !noalias !337, !llvm.access.group !280
  %sub167 = fsub fast double %49, %conv161
  store double %sub167, double* %z166, align 16, !tbaa !335, !alias.scope !338, !noalias !339, !llvm.access.group !280
  %mul168 = fmul fast float %mul139, %18
  %sub169 = fsub fast float %mul168, %19
  %mul170 = fmul fast float %sub169, %mul139
  %sub171 = fsub fast float %mul170, %20
  %conv172 = fpext float %sub171 to double
  %add173 = fadd fast double %39, %conv172
  %50 = load i32, i32* %eatom, align 4, !tbaa !5, !alias.scope !340, !noalias !12, !llvm.access.group !280
  %tobool.not = icmp eq i32 %50, 0
  br i1 %tobool.not, label %if.end184, label %if.then174

if.then174:                                       ; preds = %if.then134
  %mul175 = fmul fast float %sub171, 5.000000e-01
  %conv176 = fpext float %mul175 to double
  %add177 = fadd fast double %38, %conv176
  call void @llvm.assume(i1 true) [ "align"(%"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 64) ], !llvm.access.group !280
  %w = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 %idxprom116, i32 3
  %51 = load double, double* %w, align 8, !tbaa !341, !alias.scope !342, !noalias !343, !llvm.access.group !280
  %add182 = fadd fast double %51, %conv176
  store double %add182, double* %w, align 8, !tbaa !341, !alias.scope !344, !noalias !345, !llvm.access.group !280
  br label %if.end184

if.end184:                                        ; preds = %if.then174, %if.then134, %omp.inner.for.body
  %52 = phi double [ %38, %if.then134 ], [ %add177, %if.then174 ], [ %38, %omp.inner.for.body ]
  %53 = phi double [ %add173, %if.then134 ], [ %add173, %if.then174 ], [ %39, %omp.inner.for.body ]
  %54 = phi double [ %add162, %if.then134 ], [ %add162, %if.then174 ], [ %40, %omp.inner.for.body ]
  %55 = phi double [ %add154, %if.then134 ], [ %add154, %if.then174 ], [ %41, %omp.inner.for.body ]
  %56 = phi double [ %add146, %if.then134 ], [ %add146, %if.then174 ], [ %42, %omp.inner.for.body ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %21) #1, !llvm.access.group !280
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond.not = icmp eq i64 %indvars.iv.next, %wide.trip.count141
  br i1 %exitcond.not, label %DIR.OMP.END.SIMD.2140, label %omp.inner.for.body, !llvm.loop !346

DIR.OMP.END.SIMD.2140:                            ; preds = %if.end184
  %.lcssa154 = phi double [ %52, %if.end184 ]
  %.lcssa153 = phi double [ %53, %if.end184 ]
  %.lcssa152 = phi double [ %54, %if.end184 ]
  %.lcssa151 = phi double [ %55, %if.end184 ]
  %.lcssa = phi double [ %56, %if.end184 ]
  store double %.lcssa, double* %fxtmp.red, align 8, !tbaa !283, !alias.scope !284, !noalias !285
  store double %.lcssa151, double* %fytmp.red, align 8, !tbaa !283, !alias.scope !286, !noalias !287
  store double %.lcssa152, double* %fztmp.red, align 8, !tbaa !283, !alias.scope !288, !noalias !289
  store double %.lcssa153, double* %sevdwl.red, align 8, !tbaa !283, !alias.scope !290, !noalias !291
  store double %.lcssa154, double* %fwtmp.red, align 8, !tbaa !283, !alias.scope !292, !noalias !293
  br label %DIR.OMP.END.SIMD.3

DIR.OMP.END.SIMD.3:                               ; preds = %DIR.OMP.END.SIMD.2140
  call void @llvm.directive.region.exit(token %36) [ "DIR.OMP.END.SIMD"() ]
  br label %DIR.OMP.END.SIMD.4

DIR.OMP.END.SIMD.4:                               ; preds = %DIR.OMP.END.SIMD.3
  %57 = fadd double %.lcssa, 0.000000e+00
  %58 = fadd double %.lcssa151, 0.000000e+00
  %59 = fadd double %.lcssa152, 0.000000e+00
  %60 = fadd double %.lcssa154, 0.000000e+00
  %61 = fadd double %.lcssa153, 0.000000e+00
  br label %omp.precond.end

omp.precond.end:                                  ; preds = %DIR.OMP.END.SIMD.4, %for.body
  %fwtmp.priv.1 = phi double [ 0.000000e+00, %for.body ], [ %60, %DIR.OMP.END.SIMD.4 ]
  %fxtmp.priv.1 = phi double [ 0.000000e+00, %for.body ], [ %57, %DIR.OMP.END.SIMD.4 ]
  %fytmp.priv.1 = phi double [ 0.000000e+00, %for.body ], [ %58, %DIR.OMP.END.SIMD.4 ]
  %fztmp.priv.1 = phi double [ 0.000000e+00, %for.body ], [ %59, %DIR.OMP.END.SIMD.4 ]
  %sevdwl.priv.1 = phi double [ 0.000000e+00, %for.body ], [ %61, %DIR.OMP.END.SIMD.4 ]
  %x188 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 %idxprom90, i32 0
  %62 = load double, double* %x188, align 8, !tbaa !302, !alias.scope !349, !noalias !304
  %add189 = fadd fast double %62, %fxtmp.priv.1
  store double %add189, double* %x188, align 8, !tbaa !302, !alias.scope !350, !noalias !306
  %y192 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 %idxprom90, i32 1
  %63 = load double, double* %y192, align 8, !tbaa !330, !alias.scope !351, !noalias !332
  %add193 = fadd fast double %63, %fytmp.priv.1
  store double %add193, double* %y192, align 8, !tbaa !330, !alias.scope !352, !noalias !334
  %z196 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 %idxprom90, i32 2
  %64 = load double, double* %z196, align 8, !tbaa !335, !alias.scope !353, !noalias !337
  %add197 = fadd fast double %64, %fztmp.priv.1
  store double %add197, double* %z196, align 8, !tbaa !335, !alias.scope !354, !noalias !339
  %w200 = getelementptr inbounds %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t", %"struct.LAMMPS_NS::IntelBuffers<float, double>::vec3_acc_t"* %add.ptr, i64 %idxprom90, i32 3
  %65 = load double, double* %w200, align 8, !tbaa !341, !alias.scope !355, !noalias !343
  %add201 = fadd fast double %65, %fwtmp.priv.1
  store double %add201, double* %w200, align 8, !tbaa !341, !alias.scope !356, !noalias !345
  %add202 = fadd fast double %sevdwl.priv.1, %oevdwl.red.0126
  %indvars.iv.next139 = add i64 %indvars.iv138, %iip.priv.0
  %cmp87 = icmp slt i64 %indvars.iv.next139, %24
  br i1 %cmp87, label %for.body, label %for.cond.cleanup.loopexit, !llvm.loop !357

land.lhs.true:                                    ; preds = %for.cond.cleanup
  %66 = load i32, i32* %nthreads17, align 4, !tbaa !5, !alias.scope !359, !noalias !12
  %cmp205 = icmp sgt i32 %66, 2
  br i1 %cmp205, label %DIR.OMP.BARRIER.6, label %if.end207

DIR.OMP.BARRIER.6:                                ; preds = %land.lhs.true
  fence acq_rel
  %my.tid = load i32, i32* %tid, align 4, !alias.scope !360, !noalias !361
  call void @__kmpc_barrier(%struct.ident_t* nonnull @.kmpc_loc.0.0, i32 %my.tid)
  %67 = load %"class.LAMMPS_NS::IntelBuffers.0"*, %"class.LAMMPS_NS::IntelBuffers.0"** %buffers.addr, align 8, !tbaa !362, !alias.scope !364, !noalias !12
  %68 = load i32, i32* %nall, align 4, !tbaa !5, !alias.scope !365, !noalias !12
  %69 = load i32, i32* %nthreads17, align 4, !tbaa !5, !alias.scope !366, !noalias !12
  %70 = load i32, i32* %f_stride, align 4, !tbaa !5, !alias.scope !367, !noalias !12
  call void @_ZN9LAMMPS_NS12IntelBuffersIfdE12fdotr_reduceEiiiRdS2_S2_S2_S2_S2_(%"class.LAMMPS_NS::IntelBuffers.0"* nonnull align 64 dereferenceable(448) %67, i32 %68, i32 %69, i32 %70, double* nonnull align 8 dereferenceable(8) %ov0.red, double* nonnull align 8 dereferenceable(8) %ov1.red, double* nonnull align 8 dereferenceable(8) %ov2.red, double* nonnull align 8 dereferenceable(8) %ov3.red, double* nonnull align 8 dereferenceable(8) %ov4.red, double* nonnull align 8 dereferenceable(8) %ov5.red) #1
  br label %if.end207

if.end207:                                        ; preds = %DIR.OMP.BARRIER.6, %land.lhs.true, %for.cond.cleanup
  store double %oevdwl.red.0.lcssa, double* %oevdwl.fast_red, align 8
  %71 = load double, double* %ov0.red, align 8
  store double %71, double* %ov0.fast_red, align 8
  %72 = load double, double* %ov1.red, align 8
  store double %72, double* %ov1.fast_red, align 8
  %73 = load double, double* %ov2.red, align 8
  store double %73, double* %ov2.fast_red, align 8
  %74 = load double, double* %ov3.red, align 8
  store double %74, double* %ov3.fast_red, align 8
  %75 = load double, double* %ov4.red, align 8
  store double %75, double* %ov4.fast_red, align 8
  %76 = load double, double* %ov5.red, align 8
  store double %76, double* %ov5.fast_red, align 8
  %77 = bitcast %struct.fast_red_t.14* %fast_red_struct459 to i8*
  %my.tid480 = load i32, i32* %tid, align 4
  %78 = call i32 @__kmpc_reduce(%struct.ident_t* nonnull @.kmpc_loc.0.0.26, i32 %my.tid480, i32 7, i32 56, i8* nonnull %77, void (i8*, i8*)* nonnull @_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii_tree_reduce_13, [8 x i32]* nonnull @.gomp_critical_user_.fast_reduction.AS0.var)
  switch i32 %78, label %atomic.reduce.exit [
    i32 1, label %tree.reduce.exit
    i32 2, label %atomic.reduce
  ]

tree.reduce.exit:                                 ; preds = %if.end207
  %79 = load double, double* %oevdwl.fast_red, align 8
  %80 = load double, double* %oevdwl, align 8
  %81 = fadd double %79, %80
  store double %81, double* %oevdwl, align 8
  %82 = load double, double* %ov0.fast_red, align 8
  %83 = load double, double* %ov0, align 8
  %84 = fadd double %82, %83
  store double %84, double* %ov0, align 8
  %85 = load double, double* %ov1.fast_red, align 8
  %86 = load double, double* %ov1, align 8
  %87 = fadd double %85, %86
  store double %87, double* %ov1, align 8
  %88 = load double, double* %ov2.fast_red, align 8
  %89 = load double, double* %ov2, align 8
  %90 = fadd double %88, %89
  store double %90, double* %ov2, align 8
  %91 = load double, double* %ov3.fast_red, align 8
  %92 = load double, double* %ov3, align 8
  %93 = fadd double %91, %92
  store double %93, double* %ov3, align 8
  %94 = load double, double* %ov4.fast_red, align 8
  %95 = load double, double* %ov4, align 8
  %96 = fadd double %94, %95
  store double %96, double* %ov4, align 8
  %97 = load double, double* %ov5.fast_red, align 8
  %98 = load double, double* %ov5, align 8
  %99 = fadd double %97, %98
  store double %99, double* %ov5, align 8
  %my.tid481 = load i32, i32* %tid, align 4
  call void @__kmpc_end_reduce(%struct.ident_t* nonnull @.kmpc_loc.0.0.28, i32 %my.tid481, [8 x i32]* nonnull @.gomp_critical_user_.fast_reduction.AS0.var)
  br label %atomic.reduce.exit

atomic.reduce:                                    ; preds = %if.end207
  %100 = load double, double* %oevdwl.fast_red, align 8
  %my.tid473 = load i32, i32* %tid, align 4
  call void @__kmpc_atomic_float8_add(%struct.ident_t* nonnull @.kmpc_loc.0.0.12, i32 %my.tid473, double* %oevdwl, double %100)
  %101 = load double, double* %ov0.fast_red, align 8
  %my.tid474 = load i32, i32* %tid, align 4
  call void @__kmpc_atomic_float8_add(%struct.ident_t* nonnull @.kmpc_loc.0.0.14, i32 %my.tid474, double* %ov0, double %101)
  %102 = load double, double* %ov1.fast_red, align 8
  %my.tid475 = load i32, i32* %tid, align 4
  call void @__kmpc_atomic_float8_add(%struct.ident_t* nonnull @.kmpc_loc.0.0.16, i32 %my.tid475, double* %ov1, double %102)
  %103 = load double, double* %ov2.fast_red, align 8
  %my.tid476 = load i32, i32* %tid, align 4
  call void @__kmpc_atomic_float8_add(%struct.ident_t* nonnull @.kmpc_loc.0.0.18, i32 %my.tid476, double* %ov2, double %103)
  %104 = load double, double* %ov3.fast_red, align 8
  %my.tid477 = load i32, i32* %tid, align 4
  call void @__kmpc_atomic_float8_add(%struct.ident_t* nonnull @.kmpc_loc.0.0.20, i32 %my.tid477, double* %ov3, double %104)
  %105 = load double, double* %ov4.fast_red, align 8
  %my.tid478 = load i32, i32* %tid, align 4
  call void @__kmpc_atomic_float8_add(%struct.ident_t* nonnull @.kmpc_loc.0.0.22, i32 %my.tid478, double* %ov4, double %105)
  %106 = load double, double* %ov5.fast_red, align 8
  %my.tid479 = load i32, i32* %tid, align 4
  call void @__kmpc_atomic_float8_add(%struct.ident_t* nonnull @.kmpc_loc.0.0.24, i32 %my.tid479, double* %ov5, double %106)
  %my.tid484 = load i32, i32* %tid, align 4
  call void @__kmpc_end_reduce(%struct.ident_t* nonnull @.kmpc_loc.0.0.30, i32 %my.tid484, [8 x i32]* nonnull @.gomp_critical_user_.fast_reduction.AS0.var)
  br label %atomic.reduce.exit

atomic.reduce.exit:                               ; preds = %atomic.reduce, %tree.reduce.exit, %if.end207
  ret void
}

attributes #0 = { argmemonly nofree nosync nounwind willreturn }
attributes #1 = { nounwind }
attributes #2 = { nofree nounwind readonly "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #3 = { argmemonly nofree nounwind willreturn writeonly }
attributes #4 = { "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "prefer-vector-width"="512" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }
attributes #5 = { convergent nofree nounwind }
attributes #6 = { inaccessiblememonly nofree nosync nounwind willreturn }
attributes #7 = { mustprogress nofree norecurse nosync nounwind willreturn }
attributes #8 = { convergent nounwind }
attributes #9 = { uwtable "denormal-fp-math"="preserve-sign,preserve-sign" "denormal-fp-math-f32"="ieee,ieee" "frame-pointer"="none" "loopopt-pipeline"="full" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "mt-func"="true" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "prefer-vector-width"="512" "processed-by-vpo" "stack-protector-buffer-size"="8" "target-cpu"="skylake-avx512" "target-features"="+adx,+aes,+avx,+avx2,+avx512bw,+avx512cd,+avx512dq,+avx512f,+avx512vl,+bmi,+bmi2,+clflushopt,+clwb,+crc32,+cx16,+cx8,+f16c,+fma,+fsgsbase,+fxsr,+invpcid,+lzcnt,+mmx,+movbe,+pclmul,+pku,+popcnt,+prfchw,+rdrnd,+rdseed,+sahf,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsavec,+xsaveopt,+xsaves" "unsafe-fp-math"="true" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}
!nvvm.annotations = !{}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 1}
!3 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2022.1.0 (2022.x.0.YYYYMMDD)"}
!4 = !{i32 0, i32 2147483647}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C++ TBAA"}
!9 = !{!10}
!10 = distinct !{!10, !11, !"OMPAliasScope"}
!11 = distinct !{!11, !"OMPDomain"}
!12 = !{!13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !165, !166, !167}
!13 = distinct !{!13, !11, !"OMPAliasScope"}
!14 = distinct !{!14, !11, !"OMPAliasScope"}
!15 = distinct !{!15, !11, !"OMPAliasScope"}
!16 = distinct !{!16, !11, !"OMPAliasScope"}
!17 = distinct !{!17, !11, !"OMPAliasScope"}
!18 = distinct !{!18, !11, !"OMPAliasScope"}
!19 = distinct !{!19, !11, !"OMPAliasScope"}
!20 = distinct !{!20, !11, !"OMPAliasScope"}
!21 = distinct !{!21, !11, !"OMPAliasScope"}
!22 = distinct !{!22, !11, !"OMPAliasScope"}
!23 = distinct !{!23, !11, !"OMPAliasScope"}
!24 = distinct !{!24, !11, !"OMPAliasScope"}
!25 = distinct !{!25, !11, !"OMPAliasScope"}
!26 = distinct !{!26, !11, !"OMPAliasScope"}
!27 = distinct !{!27, !11, !"OMPAliasScope"}
!28 = distinct !{!28, !11, !"OMPAliasScope"}
!29 = distinct !{!29, !11, !"OMPAliasScope"}
!30 = distinct !{!30, !11, !"OMPAliasScope"}
!31 = distinct !{!31, !11, !"OMPAliasScope"}
!32 = distinct !{!32, !11, !"OMPAliasScope"}
!33 = distinct !{!33, !11, !"OMPAliasScope"}
!34 = distinct !{!34, !11, !"OMPAliasScope"}
!35 = distinct !{!35, !11, !"OMPAliasScope"}
!36 = distinct !{!36, !11, !"OMPAliasScope"}
!37 = distinct !{!37, !11, !"OMPAliasScope"}
!38 = distinct !{!38, !11, !"OMPAliasScope"}
!39 = distinct !{!39, !11, !"OMPAliasScope"}
!40 = distinct !{!40, !11, !"OMPAliasScope"}
!41 = distinct !{!41, !11, !"OMPAliasScope"}
!42 = distinct !{!42, !11, !"OMPAliasScope"}
!43 = distinct !{!43, !11, !"OMPAliasScope"}
!44 = distinct !{!44, !11, !"OMPAliasScope"}
!45 = distinct !{!45, !11, !"OMPAliasScope"}
!46 = distinct !{!46, !11, !"OMPAliasScope"}
!47 = distinct !{!47, !11, !"OMPAliasScope"}
!48 = distinct !{!48, !11, !"OMPAliasScope"}
!49 = distinct !{!49, !11, !"OMPAliasScope"}
!50 = distinct !{!50, !11, !"OMPAliasScope"}
!51 = distinct !{!51, !11, !"OMPAliasScope"}
!52 = distinct !{!52, !11, !"OMPAliasScope"}
!53 = distinct !{!53, !11, !"OMPAliasScope"}
!54 = distinct !{!54, !11, !"OMPAliasScope"}
!55 = distinct !{!55, !11, !"OMPAliasScope"}
!56 = distinct !{!56, !11, !"OMPAliasScope"}
!57 = distinct !{!57, !11, !"OMPAliasScope"}
!58 = distinct !{!58, !11, !"OMPAliasScope"}
!59 = distinct !{!59, !11, !"OMPAliasScope"}
!60 = distinct !{!60, !11, !"OMPAliasScope"}
!61 = distinct !{!61, !11, !"OMPAliasScope"}
!62 = distinct !{!62, !11, !"OMPAliasScope"}
!63 = distinct !{!63, !11, !"OMPAliasScope"}
!64 = distinct !{!64, !11, !"OMPAliasScope"}
!65 = distinct !{!65, !11, !"OMPAliasScope"}
!66 = distinct !{!66, !11, !"OMPAliasScope"}
!67 = distinct !{!67, !11, !"OMPAliasScope"}
!68 = distinct !{!68, !11, !"OMPAliasScope"}
!69 = distinct !{!69, !11, !"OMPAliasScope"}
!70 = distinct !{!70, !11, !"OMPAliasScope"}
!71 = distinct !{!71, !11, !"OMPAliasScope"}
!72 = distinct !{!72, !11, !"OMPAliasScope"}
!73 = distinct !{!73, !11, !"OMPAliasScope"}
!74 = distinct !{!74, !11, !"OMPAliasScope"}
!75 = distinct !{!75, !11, !"OMPAliasScope"}
!76 = distinct !{!76, !11, !"OMPAliasScope"}
!77 = distinct !{!77, !11, !"OMPAliasScope"}
!78 = distinct !{!78, !11, !"OMPAliasScope"}
!79 = distinct !{!79, !11, !"OMPAliasScope"}
!80 = distinct !{!80, !11, !"OMPAliasScope"}
!81 = distinct !{!81, !11, !"OMPAliasScope"}
!82 = distinct !{!82, !11, !"OMPAliasScope"}
!83 = distinct !{!83, !11, !"OMPAliasScope"}
!84 = distinct !{!84, !11, !"OMPAliasScope"}
!85 = distinct !{!85, !11, !"OMPAliasScope"}
!86 = distinct !{!86, !11, !"OMPAliasScope"}
!87 = distinct !{!87, !11, !"OMPAliasScope"}
!88 = distinct !{!88, !11, !"OMPAliasScope"}
!89 = distinct !{!89, !11, !"OMPAliasScope"}
!90 = distinct !{!90, !11, !"OMPAliasScope"}
!91 = distinct !{!91, !11, !"OMPAliasScope"}
!92 = distinct !{!92, !11, !"OMPAliasScope"}
!93 = distinct !{!93, !11, !"OMPAliasScope"}
!94 = distinct !{!94, !11, !"OMPAliasScope"}
!95 = distinct !{!95, !11, !"OMPAliasScope"}
!96 = distinct !{!96, !11, !"OMPAliasScope"}
!97 = distinct !{!97, !11, !"OMPAliasScope"}
!98 = distinct !{!98, !11, !"OMPAliasScope"}
!99 = distinct !{!99, !11, !"OMPAliasScope"}
!100 = distinct !{!100, !11, !"OMPAliasScope"}
!101 = distinct !{!101, !11, !"OMPAliasScope"}
!102 = distinct !{!102, !11, !"OMPAliasScope"}
!103 = distinct !{!103, !11, !"OMPAliasScope"}
!104 = distinct !{!104, !11, !"OMPAliasScope"}
!105 = distinct !{!105, !11, !"OMPAliasScope"}
!106 = distinct !{!106, !11, !"OMPAliasScope"}
!107 = distinct !{!107, !11, !"OMPAliasScope"}
!108 = distinct !{!108, !11, !"OMPAliasScope"}
!109 = distinct !{!109, !11, !"OMPAliasScope"}
!110 = distinct !{!110, !11, !"OMPAliasScope"}
!111 = distinct !{!111, !11, !"OMPAliasScope"}
!112 = distinct !{!112, !11, !"OMPAliasScope"}
!113 = distinct !{!113, !11, !"OMPAliasScope"}
!114 = distinct !{!114, !11, !"OMPAliasScope"}
!115 = distinct !{!115, !11, !"OMPAliasScope"}
!116 = distinct !{!116, !11, !"OMPAliasScope"}
!117 = distinct !{!117, !11, !"OMPAliasScope"}
!118 = distinct !{!118, !11, !"OMPAliasScope"}
!119 = distinct !{!119, !11, !"OMPAliasScope"}
!120 = distinct !{!120, !11, !"OMPAliasScope"}
!121 = distinct !{!121, !11, !"OMPAliasScope"}
!122 = distinct !{!122, !11, !"OMPAliasScope"}
!123 = distinct !{!123, !11, !"OMPAliasScope"}
!124 = distinct !{!124, !11, !"OMPAliasScope"}
!125 = distinct !{!125, !11, !"OMPAliasScope"}
!126 = distinct !{!126, !11, !"OMPAliasScope"}
!127 = distinct !{!127, !11, !"OMPAliasScope"}
!128 = distinct !{!128, !11, !"OMPAliasScope"}
!129 = distinct !{!129, !11, !"OMPAliasScope"}
!130 = distinct !{!130, !11, !"OMPAliasScope"}
!131 = distinct !{!131, !11, !"OMPAliasScope"}
!132 = distinct !{!132, !11, !"OMPAliasScope"}
!133 = distinct !{!133, !11, !"OMPAliasScope"}
!134 = distinct !{!134, !11, !"OMPAliasScope"}
!135 = distinct !{!135, !11, !"OMPAliasScope"}
!136 = distinct !{!136, !11, !"OMPAliasScope"}
!137 = distinct !{!137, !11, !"OMPAliasScope"}
!138 = distinct !{!138, !11, !"OMPAliasScope"}
!139 = distinct !{!139, !11, !"OMPAliasScope"}
!140 = distinct !{!140, !11, !"OMPAliasScope"}
!141 = distinct !{!141, !11, !"OMPAliasScope"}
!142 = distinct !{!142, !11, !"OMPAliasScope"}
!143 = distinct !{!143, !11, !"OMPAliasScope"}
!144 = distinct !{!144, !11, !"OMPAliasScope"}
!145 = distinct !{!145, !11, !"OMPAliasScope"}
!146 = distinct !{!146, !11, !"OMPAliasScope"}
!147 = distinct !{!147, !11, !"OMPAliasScope"}
!148 = distinct !{!148, !11, !"OMPAliasScope"}
!149 = distinct !{!149, !11, !"OMPAliasScope"}
!150 = distinct !{!150, !11, !"OMPAliasScope"}
!151 = distinct !{!151, !11, !"OMPAliasScope"}
!152 = distinct !{!152, !11, !"OMPAliasScope"}
!153 = distinct !{!153, !11, !"OMPAliasScope"}
!154 = distinct !{!154, !11, !"OMPAliasScope"}
!155 = distinct !{!155, !11, !"OMPAliasScope"}
!156 = distinct !{!156, !11, !"OMPAliasScope"}
!157 = distinct !{!157, !11, !"OMPAliasScope"}
!158 = distinct !{!158, !11, !"OMPAliasScope"}
!159 = distinct !{!159, !11, !"OMPAliasScope"}
!160 = distinct !{!160, !11, !"OMPAliasScope"}
!161 = distinct !{!161, !11, !"OMPAliasScope"}
!162 = distinct !{!162, !11, !"OMPAliasScope"}
!163 = distinct !{!163, !11, !"OMPAliasScope"}
!164 = distinct !{!164, !11, !"OMPAliasScope"}
!165 = distinct !{!165, !11, !"OMPAliasScope"}
!166 = distinct !{!166, !11, !"OMPAliasScope"}
!167 = distinct !{!167, !11, !"OMPAliasScope"}
!168 = !{!169}
!169 = distinct !{!169, !11, !"OMPAliasScope"}
!170 = !{!171}
!171 = distinct !{!171, !11, !"OMPAliasScope"}
!172 = !{!173}
!173 = distinct !{!173, !11, !"OMPAliasScope"}
!174 = !{!175}
!175 = distinct !{!175, !11, !"OMPAliasScope"}
!176 = !{!177}
!177 = distinct !{!177, !11, !"OMPAliasScope"}
!178 = !{!179}
!179 = distinct !{!179, !11, !"OMPAliasScope"}
!180 = !{!181, !181, i64 0}
!181 = !{!"pointer@_ZTSPN9LAMMPS_NS12IntelBuffersIfdE10vec3_acc_tE", !7, i64 0}
!182 = !{!183}
!183 = distinct !{!183, !11, !"OMPAliasScope"}
!184 = !{!185, !185, i64 0}
!185 = !{!"struct@_ZTSN9LAMMPS_NS12IntelBuffersIfdE10vec3_acc_tE", !186, i64 0, !186, i64 8, !186, i64 16, !186, i64 24}
!186 = !{!"double", !7, i64 0}
!187 = !{!188, !188, i64 0}
!188 = !{!"pointer@_ZTSPN9LAMMPS_NS14PairLJCutIntel10ForceConstIfE10fc_packed1E", !7, i64 0}
!189 = !{!190}
!190 = distinct !{!190, !11, !"OMPAliasScope"}
!191 = !{!192, !6, i64 972}
!192 = !{!"struct@_ZTSN9LAMMPS_NS14PairLJCutIntelE", !193, i64 960, !6, i64 968, !6, i64 972, !194, i64 1024, !200, i64 1088}
!193 = !{!"pointer@_ZTSPN9LAMMPS_NS8FixIntelE", !7, i64 0}
!194 = !{!"struct@_ZTSN9LAMMPS_NS14PairLJCutIntel10ForceConstIfEE", !195, i64 0, !197, i64 16, !198, i64 24, !6, i64 32, !6, i64 36, !199, i64 40}
!195 = !{!"array@_ZTSA4_f", !196, i64 0}
!196 = !{!"float", !7, i64 0}
!197 = !{!"pointer@_ZTSPPN9LAMMPS_NS14PairLJCutIntel10ForceConstIfE10fc_packed1E", !7, i64 0}
!198 = !{!"pointer@_ZTSPPN9LAMMPS_NS14PairLJCutIntel10ForceConstIfE10fc_packed2E", !7, i64 0}
!199 = !{!"pointer@_ZTSPN9LAMMPS_NS6MemoryE", !7, i64 0}
!200 = !{!"struct@_ZTSN9LAMMPS_NS14PairLJCutIntel10ForceConstIdEE", !201, i64 0, !202, i64 32, !203, i64 40, !6, i64 48, !6, i64 52, !199, i64 56}
!201 = !{!"array@_ZTSA4_d", !186, i64 0}
!202 = !{!"pointer@_ZTSPPN9LAMMPS_NS14PairLJCutIntel10ForceConstIdE10fc_packed1E", !7, i64 0}
!203 = !{!"pointer@_ZTSPPN9LAMMPS_NS14PairLJCutIntel10ForceConstIdE10fc_packed2E", !7, i64 0}
!204 = !{!205}
!205 = distinct !{!205, !11, !"OMPAliasScope"}
!206 = !{!207, !196, i64 0}
!207 = !{!"struct@_ZTSN9LAMMPS_NS14PairLJCutIntel10ForceConstIfE10fc_packed1E", !196, i64 0, !196, i64 4, !196, i64 8, !196, i64 12}
!208 = !{!209, !211}
!209 = distinct !{!209, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: ljc12o"}
!210 = distinct !{!210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii"}
!211 = distinct !{!211, !11, !"OMPAliasScope"}
!212 = !{!213, !214, !215, !216, !217, !218, !219, !220, !221, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !165, !166, !167}
!213 = distinct !{!213, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: x"}
!214 = distinct !{!214, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: ilist"}
!215 = distinct !{!215, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: numneigh"}
!216 = distinct !{!216, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: firstneigh"}
!217 = distinct !{!217, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: special_lj"}
!218 = distinct !{!218, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: lj34"}
!219 = distinct !{!219, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: f_start"}
!220 = distinct !{!220, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: ev_global"}
!221 = distinct !{!221, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: f"}
!222 = !{!207, !196, i64 4}
!223 = !{!209, !224}
!224 = distinct !{!224, !11, !"OMPAliasScope"}
!225 = !{!207, !196, i64 8}
!226 = !{!209, !227}
!227 = distinct !{!227, !11, !"OMPAliasScope"}
!228 = !{!229, !229, i64 0}
!229 = !{!"pointer@_ZTSPN9LAMMPS_NS14PairLJCutIntel10ForceConstIfE10fc_packed2E", !7, i64 0}
!230 = !{!231}
!231 = distinct !{!231, !11, !"OMPAliasScope"}
!232 = !{!233, !196, i64 0}
!233 = !{!"struct@_ZTSN9LAMMPS_NS14PairLJCutIntel10ForceConstIfE10fc_packed2E", !196, i64 0, !196, i64 4}
!234 = !{!218, !235}
!235 = distinct !{!235, !11, !"OMPAliasScope"}
!236 = !{!213, !214, !215, !216, !217, !209, !219, !220, !221, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !165, !166, !167}
!237 = !{!233, !196, i64 4}
!238 = !{!218, !239}
!239 = distinct !{!239, !11, !"OMPAliasScope"}
!240 = !{!207, !196, i64 12}
!241 = !{!209, !242}
!242 = distinct !{!242, !11, !"OMPAliasScope"}
!243 = !{!244}
!244 = distinct !{!244, !11, !"OMPAliasScope"}
!245 = !{!246, !246, i64 0}
!246 = !{!"pointer@_ZTSPi", !7, i64 0}
!247 = !{!248}
!248 = distinct !{!248, !11, !"OMPAliasScope"}
!249 = !{!214, !166, !135, !116}
!250 = !{!213, !215, !216, !217, !209, !218, !219, !220, !221, !251, !252, !253, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !167}
!251 = distinct !{!251, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: ljc12oi"}
!252 = distinct !{!252, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: lj34i"}
!253 = distinct !{!253, !210, !"_ZN9LAMMPS_NS14PairLJCutIntel4evalILi1ELi1ELi1EfdEEviiPNS_12IntelBuffersIT2_T3_EERKNS0_10ForceConstIS3_EEii: jlist"}
!254 = !{!255, !255, i64 0}
!255 = !{!"pointer@_ZTSPPKi", !7, i64 0}
!256 = !{!257}
!257 = distinct !{!257, !11, !"OMPAliasScope"}
!258 = !{!216, !259}
!259 = distinct !{!259, !11, !"OMPAliasScope"}
!260 = !{!213, !214, !215, !217, !209, !218, !219, !220, !221, !251, !252, !253, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !165, !166, !167}
!261 = !{!262}
!262 = distinct !{!262, !11, !"OMPAliasScope"}
!263 = !{!215, !165, !134, !115}
!264 = !{!213, !214, !216, !217, !209, !218, !219, !220, !221, !251, !252, !253, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !167}
!265 = !{!266, !266, i64 0}
!266 = !{!"pointer@_ZTSPN9LAMMPS_NS12IntelBuffersIfdE6atom_tE", !7, i64 0}
!267 = !{!268}
!268 = distinct !{!268, !11, !"OMPAliasScope"}
!269 = !{!270, !196, i64 0}
!270 = !{!"struct@_ZTSN9LAMMPS_NS12IntelBuffersIfdE6atom_tE", !196, i64 0, !196, i64 4, !196, i64 8, !6, i64 12}
!271 = !{!213, !272}
!272 = distinct !{!272, !11, !"OMPAliasScope"}
!273 = !{!214, !215, !216, !217, !209, !218, !219, !220, !221, !251, !252, !253, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !165, !166, !167}
!274 = !{!270, !196, i64 4}
!275 = !{!213, !276}
!276 = distinct !{!276, !11, !"OMPAliasScope"}
!277 = !{!270, !196, i64 8}
!278 = !{!213, !279}
!279 = distinct !{!279, !11, !"OMPAliasScope"}
!280 = distinct !{}
!281 = !{!282}
!282 = distinct !{!282, !11, !"OMPAliasScope"}
!283 = !{!186, !186, i64 0}
!284 = !{!103, !104}
!285 = !{!13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !165, !166, !167}
!286 = !{!101, !102}
!287 = !{!13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !165, !166, !167}
!288 = !{!99, !100}
!289 = !{!13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !165, !166, !167}
!290 = !{!105, !106}
!291 = !{!13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !165, !166, !167}
!292 = !{!107, !108}
!293 = !{!13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !165, !166, !167}
!294 = !{!253, !131, !113}
!295 = !{!213, !214, !215, !216, !217, !209, !218, !219, !220, !221, !251, !252, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !117, !118, !119, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !167}
!296 = !{!213, !297}
!297 = distinct !{!297, !11, !"OMPAliasScope"}
!298 = !{!213, !299}
!299 = distinct !{!299, !11, !"OMPAliasScope"}
!300 = !{!213, !301}
!301 = distinct !{!301, !11, !"OMPAliasScope"}
!302 = !{!185, !186, i64 0}
!303 = !{!221, !117}
!304 = !{!213, !214, !215, !216, !217, !209, !218, !219, !220, !251, !252, !253, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !120, !121, !122, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !161, !162, !163, !164, !165, !166, !167}
!305 = !{!221, !117, !118, !160}
!306 = !{!213, !214, !215, !216, !217, !209, !218, !219, !220, !251, !252, !253, !13, !14, !15, !16, !17, !18, !10, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !169, !43, !44, !45, !46, !47, !48, !49, !50, !307, !51, !175, !308, !177, !179, !52, !183, !53, !54, !55, !56, !57, !58, !59, !60, !309, !310, !190, !205, !211, !61, !311, !224, !62, !312, !227, !63, !231, !235, !64, !313, !239, !65, !314, !242, !66, !315, !67, !68, !69, !70, !71, !72, !171, !73, !74, !316, !75, !76, !173, !317, !77, !78, !318, !319, !79, !80, !81, !82, !248, !166, !135, !116, !83, !84, !85, !86, !87, !88, !89, !90, !91, !257, !259, !92, !262, !165, !134, !115, !93, !94, !95, !268, !272, !96, !320, !276, !97, !321, !279, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !244, !113, !114, !162, !122, !120, !121, !164, !125, !123, !124, !167, !128, !126, !127, !129, !322, !323, !324, !325, !326, !130, !131, !132, !133, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !282, !297, !146, !147, !327, !299, !148, !149, !328, !301, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !161, !163, !329}
!307 = distinct !{!307, !11, !"OMPAliasScope"}
!308 = distinct !{!308, !11, !"OMPAliasScope"}
!309 = distinct !{!309, !11, !"OMPAliasScope"}
!310 = distinct !{!310, !11, !"OMPAliasScope"}
!311 = distinct !{!311, !11, !"OMPAliasScope"}
!312 = distinct !{!312, !11, !"OMPAliasScope"}
!313 = distinct !{!313, !11, !"OMPAliasScope"}
!314 = distinct !{!314, !11, !"OMPAliasScope"}
!315 = distinct !{!315, !11, !"OMPAliasScope"}
!316 = distinct !{!316, !11, !"OMPAliasScope"}
!317 = distinct !{!317, !11, !"OMPAliasScope"}
!318 = distinct !{!318, !11, !"OMPAliasScope"}
!319 = distinct !{!319, !11, !"OMPAliasScope"}
!320 = distinct !{!320, !11, !"OMPAliasScope"}
!321 = distinct !{!321, !11, !"OMPAliasScope"}
!322 = distinct !{!322, !11, !"OMPAliasScope"}
!323 = distinct !{!323, !11, !"OMPAliasScope"}
!324 = distinct !{!324, !11, !"OMPAliasScope"}
!325 = distinct !{!325, !11, !"OMPAliasScope"}
!326 = distinct !{!326, !11, !"OMPAliasScope"}
!327 = distinct !{!327, !11, !"OMPAliasScope"}
!328 = distinct !{!328, !11, !"OMPAliasScope"}
!329 = distinct !{!329, !11, !"OMPAliasScope"}
!330 = !{!185, !186, i64 8}
!331 = !{!221, !120}
!332 = !{!213, !214, !215, !216, !217, !209, !218, !219, !220, !251, !252, !253, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !123, !124, !125, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !163, !164, !165, !166, !167}
!333 = !{!221, !120, !121, !162}
!334 = !{!213, !214, !215, !216, !217, !209, !218, !219, !220, !251, !252, !253, !13, !14, !15, !16, !17, !18, !10, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !169, !43, !44, !45, !46, !47, !48, !49, !50, !307, !51, !175, !308, !177, !179, !52, !183, !53, !54, !55, !56, !57, !58, !59, !60, !309, !310, !190, !205, !211, !61, !311, !224, !62, !312, !227, !63, !231, !235, !64, !313, !239, !65, !314, !242, !66, !315, !67, !68, !69, !70, !71, !72, !171, !73, !74, !316, !75, !76, !173, !317, !77, !78, !318, !319, !79, !80, !81, !82, !248, !166, !135, !116, !83, !84, !85, !86, !87, !88, !89, !90, !91, !257, !259, !92, !262, !165, !134, !115, !93, !94, !95, !268, !272, !96, !320, !276, !97, !321, !279, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !244, !113, !114, !160, !119, !117, !118, !164, !125, !123, !124, !167, !128, !126, !127, !129, !322, !323, !324, !325, !326, !130, !131, !132, !133, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !282, !297, !146, !147, !327, !299, !148, !149, !328, !301, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !161, !163, !329}
!335 = !{!185, !186, i64 16}
!336 = !{!221, !123}
!337 = !{!213, !214, !215, !216, !217, !209, !218, !219, !220, !251, !252, !253, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !126, !127, !128, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !165, !166, !167}
!338 = !{!221, !123, !124, !164}
!339 = !{!213, !214, !215, !216, !217, !209, !218, !219, !220, !251, !252, !253, !13, !14, !15, !16, !17, !18, !10, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !169, !43, !44, !45, !46, !47, !48, !49, !50, !307, !51, !175, !308, !177, !179, !52, !183, !53, !54, !55, !56, !57, !58, !59, !60, !309, !310, !190, !205, !211, !61, !311, !224, !62, !312, !227, !63, !231, !235, !64, !313, !239, !65, !314, !242, !66, !315, !67, !68, !69, !70, !71, !72, !171, !73, !74, !316, !75, !76, !173, !317, !77, !78, !318, !319, !79, !80, !81, !82, !248, !166, !135, !116, !83, !84, !85, !86, !87, !88, !89, !90, !91, !257, !259, !92, !262, !165, !134, !115, !93, !94, !95, !268, !272, !96, !320, !276, !97, !321, !279, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !244, !113, !114, !160, !119, !117, !118, !162, !122, !120, !121, !167, !128, !126, !127, !129, !322, !323, !324, !325, !326, !130, !131, !132, !133, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !282, !297, !146, !147, !327, !299, !148, !149, !328, !301, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !161, !163, !329}
!340 = !{!329}
!341 = !{!185, !186, i64 24}
!342 = !{!221, !126}
!343 = !{!213, !214, !215, !216, !217, !209, !218, !219, !220, !251, !252, !253, !13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !117, !118, !119, !120, !121, !122, !123, !124, !125, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !160, !161, !162, !163, !164, !165, !166}
!344 = !{!221, !126, !127, !167}
!345 = !{!213, !214, !215, !216, !217, !209, !218, !219, !220, !251, !252, !253, !13, !14, !15, !16, !17, !18, !10, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !169, !43, !44, !45, !46, !47, !48, !49, !50, !307, !51, !175, !308, !177, !179, !52, !183, !53, !54, !55, !56, !57, !58, !59, !60, !309, !310, !190, !205, !211, !61, !311, !224, !62, !312, !227, !63, !231, !235, !64, !313, !239, !65, !314, !242, !66, !315, !67, !68, !69, !70, !71, !72, !171, !73, !74, !316, !75, !76, !173, !317, !77, !78, !318, !319, !79, !80, !81, !82, !248, !166, !135, !116, !83, !84, !85, !86, !87, !88, !89, !90, !91, !257, !259, !92, !262, !165, !134, !115, !93, !94, !95, !268, !272, !96, !320, !276, !97, !321, !279, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !244, !113, !114, !160, !119, !117, !118, !162, !122, !120, !121, !164, !125, !123, !124, !129, !322, !323, !324, !325, !326, !130, !131, !132, !133, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !282, !297, !146, !147, !327, !299, !148, !149, !328, !301, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !161, !163, !329}
!346 = distinct !{!346, !347, !348}
!347 = !{!"llvm.loop.vectorize.ivdep_loop", i32 0}
!348 = !{!"llvm.loop.parallel_accesses", !280}
!349 = !{!221, !160, !119}
!350 = !{!221, !117, !118, !119}
!351 = !{!221, !162, !122}
!352 = !{!221, !120, !121, !122}
!353 = !{!221, !164, !125}
!354 = !{!221, !123, !124, !125}
!355 = !{!221, !167, !128}
!356 = !{!221, !126, !127, !128}
!357 = distinct !{!357, !358}
!358 = !{!"llvm.loop.mustprogress"}
!359 = !{!322}
!360 = !{!127, !124, !121, !118}
!361 = !{!13, !14, !15, !16, !17, !18, !19, !20, !21, !22, !23, !24, !25, !26, !27, !28, !29, !30, !31, !32, !33, !34, !35, !36, !37, !38, !39, !40, !41, !42, !43, !44, !45, !46, !47, !48, !49, !50, !51, !52, !53, !54, !55, !56, !57, !58, !59, !60, !61, !62, !63, !64, !65, !66, !67, !68, !69, !70, !71, !72, !73, !74, !75, !76, !77, !78, !79, !80, !81, !82, !83, !84, !85, !86, !87, !88, !89, !90, !91, !92, !93, !94, !95, !96, !97, !98, !99, !100, !101, !102, !103, !104, !105, !106, !107, !108, !109, !110, !111, !112, !113, !114, !115, !116, !129, !130, !131, !132, !133, !134, !135, !136, !137, !138, !139, !140, !141, !142, !143, !144, !145, !146, !147, !148, !149, !150, !151, !152, !153, !154, !155, !156, !157, !158, !159, !161, !163, !165, !166}
!362 = !{!363, !363, i64 0}
!363 = !{!"pointer@_ZTSPN9LAMMPS_NS12IntelBuffersIfdEE", !7, i64 0}
!364 = !{!323}
!365 = !{!324}
!366 = !{!325}
!367 = !{!326}

; end INTEL_FEATURE_SW_ADVANCED
