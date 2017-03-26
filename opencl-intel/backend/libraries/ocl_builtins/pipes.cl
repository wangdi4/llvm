


#define __ovld __attribute__((overloadable))

// There are no declarations of OpenCL 2.0 builtins in opencl-c.h for named
// address space but in the library they has to be called directly because the
// library won't be handled by "Generic Address Resolution" passes. So declare
// them here.
void __ovld atomic_init(__global volatile atomic_int *object, int value);
int __ovld atomic_load(__global volatile atomic_int *object);
void __ovld atomic_store(__global volatile atomic_int *object, int desired);
int __ovld atomic_fetch_add(__global volatile atomic_int *object, int desired);
int __ovld atomic_exchange(__global volatile atomic_int *object, int desired);
void __ovld atomic_flag_clear(__global volatile atomic_flag *object);
bool __ovld atomic_flag_test_and_set(__global volatile atomic_flag *object);

bool __ovld atomic_compare_exchange_weak(__global volatile atomic_int *object,
                                         __private int *expected, int desired);

// Debug switches
#define DEBUG_INF_LOOPS 0
#define DEBUG_PRINTF_LEVEL 0
#define DEBUG_DUMP_HAZ_FLAGS 1
#define DEBUG_ASSERTS 0

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#if DEBUG_ASSERTS
#define ASSERT(cond) { if (!(cond)) {printf(">> ASSERT at "              \
                                          STRINGIFY(__FILE__) ":"        \
                                          STRINGIFY(__LINE__) ": " #cond \
                                          "\n");                         \
                                   *((volatile int*)NULL) = 0xbeef;}}

#define ASSERTP(cond, pipe) { if (!(cond)) {printf(">> ASSERT at "         \
                                            STRINGIFY(__FILE__) ":"        \
                                            STRINGIFY(__LINE__) ": " #cond \
                                            "\n");                         \
                                   __pipe_dump(pipe);                      \
                                   *((volatile int*)NULL) = 0xbeef;}}

#else
#define ASSERT(cond) do {} while(0);
#define ASSERTP(cond,pipe) do {} while(0);
#endif

#if DEBUG_PRINTF_LEVEL >= 1
#define PRINTF1 printf
#else
#define PRINTF1 do {} while(0);
#endif

#if DEBUG_PRINTF_LEVEL >= 2
#define PRINTF2 printf
#else
#define PRINTF2 do {} while(0);
#endif

#if DEBUG_PRINTF_LEVEL >= 3
#define PRINTF3 printf
#else
#define PRINTF3 do {} while(0);
#endif

#if DEBUG_PRINTF_LEVEL >= 4
#define PRINTF4 printf
#else
#define PRINTF4 do {} while(0);
#endif


struct __pipe_t {
  int packet_size;
  int max_packets;

  volatile atomic_flag read_lock;
  volatile atomic_flag write_lock;

  volatile atomic_int read_begin;
  volatile atomic_int read_end;

  volatile atomic_int write_begin;
  volatile atomic_int write_end;

  // char buffer[0];
};

static void __pipe_dump(__global const struct __pipe_t* p) {
  printf(">>> pipe dump:\n"
         "> packet_size = %d\n"
         "> max_packets = %d\n"
         "> read_begin = %d\n"
         "> read_end = %d\n"
         "> write_begin = %d\n"
         "> write_end = %d\n",
         p,
         p->packet_size, p->max_packets,
         p->read_begin, p->read_end,
         p->write_begin,p->write_end);
}

static bool __pipe_index_in_range(__global const struct __pipe_t* p,
                                  int index) {
  return index >= 0 && index < p->max_packets;
}

/// Return next nth item from circular buffer
static int advance(__global const struct __pipe_t* p, int index, int offset) {
  ASSERTP(__pipe_index_in_range(p, index), p);
  ASSERTP(offset < p->max_packets, p);
  ASSERT(offset >= 0);
  int new =  (index + offset) % p->max_packets;
  ASSERTP(__pipe_index_in_range(p, new), p);
  return new;
}

/// For given \p index_from and \p index_to compute the number of elements
/// between them. The function behaves exactly as std::distance.
static int dist(__global const struct __pipe_t* p,
                    int index_from, int index_to) {
  int res = index_from <= index_to ? index_to - index_from
    : p->max_packets - index_from + index_to;
  ASSERT(res >= 0);
  return res;
}

static int find_reserve_write(__global struct __pipe_t* p, uint num_packets) {
  int read_begin = atomic_load(&p->read_begin);
  int write_end = atomic_load(&p->write_end);
  ASSERTP(__pipe_index_in_range(p, read_begin), p);
  ASSERTP(__pipe_index_in_range(p, write_end), p);

  // assume 'pipe is full' condition when only one element is b/w read_begin and
  // write_end

  int avail = (write_end == read_begin)
    ? p->max_packets - 1 // empty buffer
    : dist(p, write_end, read_begin) - 1;

  if (num_packets > (uint)avail) {
    return -1;
  }

  return write_end;
}

static int find_reserve_read(__global struct __pipe_t* p, uint num_packets) {
  int read_end = atomic_load(&p->read_end);
  int write_begin = atomic_load(&p->write_begin);
  ASSERTP(__pipe_index_in_range(p, read_end), p);
  ASSERTP(__pipe_index_in_range(p, write_begin), p);

  int avail = dist(p, read_end, write_begin);

  if (num_packets > (uint)avail) {
    return -1;
  }

  return read_end;
}

static __global volatile atomic_int*
get_reserve_ptr(__global struct __pipe_t* p, int index) {
  ASSERTP(__pipe_index_in_range(p, index), p);
  return (__global volatile atomic_int*) (p + 1) + index;
}

/// Return the pointer on the beginning of packet with given \p index
static __global void* get_packet_ptr(__global struct __pipe_t* p, int index) {
  ASSERTP(__pipe_index_in_range(p, index), p);
  __global char* packets_begin =
    (__global char*) (get_reserve_ptr(p, p->max_packets - 1) + 1);

  return &packets_begin[p->packet_size * index];
}

static reserve_id_t create_reserve_id(int reserved) {
  return __builtin_astype(
      (__global void*)((intptr_t) reserved | __PIPE_RESERVE_ID_VALID_BIT),
      reserve_id_t);
}

static int extract_reserve_id(reserve_id_t id) {
  return  (int) ((intptr_t)__builtin_astype(id, __global void*)
                 & ~ __PIPE_RESERVE_ID_VALID_BIT);
}

bool __ovld is_valid_reserve_id(reserve_id_t reserve_id) {
  return (intptr_t)__builtin_astype(reserve_id, __global void*)
    & __PIPE_RESERVE_ID_VALID_BIT;
}


void __pipe_init_intel(__global struct __pipe_t* p,
                       int packet_size, int max_packets) {
  p->packet_size = packet_size;
  p->max_packets = max_packets + 1;

  atomic_flag_clear(&p->read_lock);
  atomic_flag_clear(&p->write_lock);

  atomic_init(&p->read_begin, 0);
  atomic_init(&p->read_end, 0);
  atomic_init(&p->write_begin, 0);
  atomic_init(&p->write_end, 0);

  for (int i = 0; i < p->max_packets; ++i) {
    atomic_init(get_reserve_ptr(p, i), 0);
  }
}

void __pipe_init_array_intel(__global struct __pipe_t* __global * p,
                             int array_size, int packet_size, int max_packets) {
  for (int i = 0; i < array_size; ++i) {
    __pipe_init_intel(p[i], packet_size, max_packets);
  }
}

static void __lock_read(__global struct __pipe_t* p) {
#if DEBUG_INF_LOOPS
  volatile int attempts = 1000000;
#endif
  while (atomic_flag_test_and_set(&p->read_lock)) {
#if DEBUG_INF_LOOPS
    ASSERTP(--attempts, p);
#endif
  }
}

static void __lock_write(__global struct __pipe_t* p) {
#if DEBUG_INF_LOOPS
  volatile int attempts = 1000000;
#endif
  while (atomic_flag_test_and_set(&p->write_lock)) {
#if DEBUG_INF_LOOPS
    ASSERTP(--attempts, p);
#endif
  }
}

static void __unlock_read(__global struct __pipe_t* p) {
  atomic_flag_clear(&p->read_lock);
}

static void __unlock_write(__global struct __pipe_t* p) {
  atomic_flag_clear(&p->write_lock);
}

reserve_id_t __reserve_read_pipe_intel(__global struct __pipe_t* p,
                                       uint num_packets) {
  __lock_read(p);

  int reserved = find_reserve_read(p, num_packets);

  if (reserved == -1) {
    __unlock_read(p);
    return CLK_NULL_RESERVE_ID;
  }

  atomic_store(&p->read_end, advance(p, reserved, num_packets));
  atomic_store(get_reserve_ptr(p, reserved), num_packets);

  __unlock_read(p);

  return create_reserve_id(reserved);
}

reserve_id_t __reserve_write_pipe_intel(__global struct __pipe_t* p,
                                        uint num_packets) {
  __lock_write(p);

  int reserved = find_reserve_write(p, num_packets);
  if (reserved == -1) {
    __unlock_write(p);
    return CLK_NULL_RESERVE_ID;
  }

  atomic_store(&p->write_end, advance(p, reserved, num_packets));
  atomic_store(get_reserve_ptr(p, reserved), num_packets);

  __unlock_write(p);

  return create_reserve_id(reserved);
}

int __read_pipe_4_intel(__global struct __pipe_t* p, reserve_id_t reserve_id,
                        uint index, void* dst) {
  int reserved = extract_reserve_id(reserve_id);
  ASSERTP(__pipe_index_in_range(p, reserved), p);

  int read_index = advance(p, reserved, index);
  __builtin_memcpy(dst, get_packet_ptr(p, read_index), p->packet_size);

  return 0;
}

int __write_pipe_4_intel(__global struct __pipe_t* p, reserve_id_t reserve_id,
                         uint index, const void* src) {
  int reserved = extract_reserve_id(reserve_id);
  ASSERTP(__pipe_index_in_range(p, reserved), p);

  int write_index = advance(p, reserved, index);
  __builtin_memcpy(get_packet_ptr(p, write_index), src, p->packet_size);

  return 0;
}

static int find_first_reserved(__global struct __pipe_t* p,
                              int begin, int end) {
  for (int i = begin; i != end; i = advance(p, i, 1)) {
    if (atomic_load(get_reserve_ptr(p, i))) {
      return i;
    }
  }

  return end;
}

int __commit_read_pipe_intel(__global struct __pipe_t* p,
                             reserve_id_t reserve_id) {
  int reserved = extract_reserve_id(reserve_id);
  ASSERTP(__pipe_index_in_range(p, reserved), p);
  ASSERTP(0 < atomic_load(get_reserve_ptr(p, reserved)), p);

  __lock_read(p);

  int read_begin = atomic_load(&p->read_begin);
  int read_end = atomic_load(&p->read_end);
  ASSERTP(__pipe_index_in_range(p, read_begin), p);
  ASSERTP(__pipe_index_in_range(p, read_end), p);

  if (read_begin != reserved) {
    atomic_store(get_reserve_ptr(p, reserved), 0);
    __unlock_read(p);
    return 0;
  }

  int num_reserved = atomic_load(get_reserve_ptr(p, reserved));
  atomic_store(get_reserve_ptr(p, reserved), 0);

  int new_read_begin =
    find_first_reserved(p, advance(p, reserved, num_reserved), read_end);

  atomic_store(&p->read_begin, new_read_begin);

  __unlock_read(p);

  return 0;
}

int __commit_write_pipe_intel(__global struct __pipe_t* p,
                              reserve_id_t reserve_id) {
  int reserved = extract_reserve_id(reserve_id);
  ASSERTP(__pipe_index_in_range(p, reserved), p);
  ASSERTP(0 < atomic_load(get_reserve_ptr(p, reserved)), p);

  __lock_write(p);

  int write_begin = atomic_load(&p->write_begin);
  int write_end = atomic_load(&p->write_end);
  ASSERTP(__pipe_index_in_range(p, write_begin), p);
  ASSERTP(__pipe_index_in_range(p, write_end), p);

  if (write_begin != reserved) {
    atomic_store(get_reserve_ptr(p, reserved), 0);
    __unlock_write(p);
    return 0;
  }

  int num_reserved = atomic_load(get_reserve_ptr(p, reserved));
  atomic_store(get_reserve_ptr(p, reserved), 0);

  int new_write_begin =
    find_first_reserved(p, advance(p, reserved, num_reserved), write_end);

  atomic_store(&p->write_begin, new_write_begin);

  __unlock_write(p);

  return 0;
}

int __read_pipe_2_intel(__global struct __pipe_t* p, void* dst) {
  reserve_id_t id = __reserve_read_pipe_intel(p, 1);
  if (!is_valid_reserve_id(id)) {
    return 1;
  }

  __read_pipe_4_intel(p, id, 0, dst);
  __commit_read_pipe_intel(p, id);

  return 0;
}

int __write_pipe_2_intel(__global struct __pipe_t* p, void* src) {
  reserve_id_t id = __reserve_write_pipe_intel(p, 1);
  if (!is_valid_reserve_id(id)) {
    return 1;
  }

  __write_pipe_4_intel(p, id, 0, src);
  __commit_write_pipe_intel(p, id);

  return 0;
}

int __read_pipe_2_bl_intel(__global struct __pipe_t* p, void* dst) {
  PRINTF1("__read_pipe_2_bl: start for %p\n", dst);
#if DEBUG_INF_LOOPS
  volatile int attempts = 1000000;
#endif
  while(__read_pipe_2_intel(p, dst)) {
#if DEBUG_INF_LOOPS
    ASSERTP(--attempts, p);
#endif
  }
  PRINTF1("__read_pipe_2_bl: ok for %p\n", dst);
  return 0;
}

int __write_pipe_2_bl_intel(__global struct __pipe_t* p, void* src) {
  PRINTF1("__write_pipe_2_bl: start for %p\n", src);
#if DEBUG_INF_LOOPS
  volatile int attempts = 1000000;
#endif
  while(__write_pipe_2_intel(p, src)) {
#if DEBUG_INF_LOOPS
    ASSERTP(--attempts, p);
#endif
  }
  PRINTF1("__write_pipe_2_bl: ok for %p\n", src);
  return 0;
}
