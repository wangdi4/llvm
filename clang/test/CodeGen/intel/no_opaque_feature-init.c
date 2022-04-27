// Check than intel_features_init_cc calling convention is applied to relevant feature init functions.
// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -disable-llvm-passes -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s

int __libirc_isa_info_initialized = 0;
int __intel_cpu_feature_indicator[2];
int __intel_cpu_feature_indicator_x[2];

typedef struct {
    const char* libirc_name;
    int bitpos;
    const char* isa_name;
} FeatureDefType;

FeatureDefType *proc_info_features;

// CHECK: define{{.*}} intel_features_init_cc i32 @__libirc_isa_init_once()
int __libirc_isa_init_once(void) {
  return 1;
}

// CHECK: define{{.*}} intel_features_init_cc i32 @__intel_set_cpu_indicator(i8* noundef %{{.+}}, i8* noundef %{{.+}})
int __intel_set_cpu_indicator(void *indicator_ptr, void *value_ptr) {
    return 0;
}

// CHECK: define{{.*}} intel_features_init_cc i32 @__libirc_get_feature_bitpos(i32 noundef %{{.+}})
// CHECK: call intel_features_init_cc i32 @__libirc_isa_init_once()
int __libirc_get_feature_bitpos(int feature_id) {
  int bitpos;
  if (!__libirc_isa_info_initialized && !__libirc_isa_init_once())
      return -2;

  bitpos = proc_info_features[feature_id].bitpos;
  return bitpos;
}

// CHECK-NOT: define{{.*}} intel_features_init_cc {{.*}}i32 @__libirc_get_cpu_feature(
// CHECK: call intel_features_init_cc i32 @__libirc_get_feature_bitpos(i32 noundef %{{.+}})
int __libirc_get_cpu_feature(void *features_ptr, int feature) {
  __libirc_get_feature_bitpos(feature);
  return 0;
}


// CHECK: define{{.*}} intel_features_init_cc i32 @__libirc_set_cpu_feature(i8* noundef %{{.+}}, i32 noundef %{{.+}})
// CHECK: call intel_features_init_cc i32 @__libirc_get_feature_bitpos(i32 noundef %{{.+}})
int __libirc_set_cpu_feature(void *features_ptr, int feature) {
  __libirc_get_feature_bitpos(feature);
  return 0;
}

// CHECK: define{{.*}} intel_features_init_cc i32 @__intel_cpu_features_init_body(i32 noundef %{{.+}})
// CHECK: call intel_features_init_cc i32 @__libirc_set_cpu_feature(i8* noundef %{{.+}}, i32 noundef 0)
// CHECK: call intel_features_init_cc i32 @__intel_set_cpu_indicator(i8* noundef bitcast ([2 x i32]* @{{.+}} to i8*), i8* noundef %{{.+}})
// CHECK: call intel_features_init_cc i32 @__intel_set_cpu_indicator(i8* noundef bitcast ([2 x i32]* @{{.+}} to i8*), i8* noundef %{{.+}})
int __intel_cpu_features_init_body(int vendor_check) {
  int cpu_feature_indicator[2] = {0};
  int rcode = __libirc_set_cpu_feature(cpu_feature_indicator, 0);
  if (rcode != 0)
    return rcode;
  __intel_set_cpu_indicator(__intel_cpu_feature_indicator,
                            cpu_feature_indicator);
  __intel_set_cpu_indicator(__intel_cpu_feature_indicator_x,
                            cpu_feature_indicator);
  return 0;
}

// CHECK-NOT: define{{.*}} intel_features_init_cc {{.*}}i8* @__libirc_get_feature_name(
// CHECK: call intel_features_init_cc i32 @__libirc_isa_init_once()
const char* __libirc_get_feature_name(int feature_id) {
    if (!__libirc_isa_info_initialized && !__libirc_isa_init_once())
        return 0;
    return proc_info_features[feature_id].libirc_name;
}

// CHECK: define{{.*}} intel_features_init_cc i32 @__intel_cpu_features_init()
// CHECK: call intel_features_init_cc i32 @__intel_cpu_features_init_body(i32 noundef 1)
int __intel_cpu_features_init(void) {
    return __intel_cpu_features_init_body(1);
}

// CHECK: define{{.*}} intel_features_init_cc i32 @__intel_cpu_features_init_x()
// CHECK: call intel_features_init_cc i32 @__intel_cpu_features_init_body(i32 noundef 0)
int __intel_cpu_features_init_x(void) {
    return __intel_cpu_features_init_body(0);
}

void __intel_proc_init_ftzdazule(int known_to_support_flags, int flags);

// CHECK-NOT: define{{.*}} intel_features_init_cc {{.*}}void @__intel_new_feature_proc_init_n(
void __intel_new_feature_proc_init_n(int idx, int required_features) {
  if ((__intel_cpu_feature_indicator[idx] & required_features) ==
      required_features)
    return;
}

// CHECK-NOT: define{{.*}} intel_features_init_cc {{.*}}void @__intel_new_feature_proc_init(
// CHECK: call void @__intel_new_feature_proc_init_n(i32 noundef 0, i32 noundef %{{.+}})
// CHECK: call i32 @__libirc_get_cpu_feature(i8* noundef bitcast ([2 x i32]* @{{.+}} to i8*), i32 noundef 5)
// CHECK: call void @__intel_proc_init_ftzdazule(i32 noundef 0, i32 noundef %{{.+}})
void __intel_new_feature_proc_init(int fp_flags, int required_features) {
  int is_intel_sse;
  __intel_new_feature_proc_init_n(0, required_features);
  is_intel_sse = __libirc_get_cpu_feature(__intel_cpu_feature_indicator, 5);
  if (is_intel_sse == 1)
    __intel_proc_init_ftzdazule(0, fp_flags);
}
