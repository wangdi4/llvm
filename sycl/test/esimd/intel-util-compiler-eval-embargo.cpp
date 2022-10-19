// INTEL_FEATURE_ESIMD_EMBARGO
// REQUIRES: intel_feature_esimd_embargo
// RUN: %clangxx -fsycl -fsycl-device-only -c %s
// UNSUPPORTED: linux

#include <CL/sycl.hpp>
#include <CL/sycl/INTEL/esimd.hpp>

using namespace sycl::ext::intel::experimental::esimd;

enum class TheEnum {
  FirstMember,
  SecondMember,
  ThirdMember
};

static_assert(detail::is_one_of_v<float, float>, "");
static_assert(!detail::is_one_of_v<float, double>, "");
static_assert(detail::is_one_of_enum_v<TheEnum, TheEnum::FirstMember,
              TheEnum::SecondMember, TheEnum::FirstMember>, "");
static_assert(!detail::is_one_of_enum_v<TheEnum, TheEnum::FirstMember,
              TheEnum::SecondMember, TheEnum::ThirdMember>, "");
static_assert(detail::is_hf_type<detail::half>::value, "");
static_assert(!detail::is_hf_type<float>::value, "");

// end INTEL_FEATURE_ESIMD_EMBARGO
