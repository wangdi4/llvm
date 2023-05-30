// Test host codegen.
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-late-outline -fopenmp-version=50 %s

typedef enum omp_allocator_handle_t {
  omp_null_allocator = 0,
  omp_default_mem_alloc = 1,
  omp_large_cap_mem_alloc = 2,
  omp_const_mem_alloc = 3,
  omp_high_bw_mem_alloc = 4,
  omp_low_lat_mem_alloc = 5,
  omp_cgroup_mem_alloc = 6,
  omp_pteam_mem_alloc = 7,
  omp_thread_mem_alloc = 8,
  KMP_ALLOCATOR_MAX_HANDLE = __UINTPTR_MAX__
} omp_allocator_handle_t;

typedef enum omp_alloctrait_key_t { omp_atk_sync_hint = 1,
                                    omp_atk_alignment = 2,
                                    omp_atk_access = 3,
                                    omp_atk_pool_size = 4,
                                    omp_atk_fallback = 5,
                                    omp_atk_fb_data = 6,
                                    omp_atk_pinned = 7,
                                    omp_atk_partition = 8
} omp_alloctrait_key_t;

typedef struct omp_alloctrait_t {
  omp_alloctrait_key_t key;
  __UINTPTR_TYPE__ value;
} omp_alloctrait_t;

void fie(void) {
  omp_allocator_handle_t my_allocator;
  omp_alloctrait_t traits[10];
  // expected-error@+1 {{unexpected OpenMP clause 'uses_allocators' in directive '#pragma omp target}}
  #pragma omp target uses_allocators(my_allocator(traits))
  {}
}

