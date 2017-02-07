//
// TL;DR:
// ------
//
// Pipe built-ins are lock-free and thread-safe. Pipe object is a
// circular buffer, which is allocated with additional space trailing the
// header structure `__pipe_t`. Initialization is done by __pipe_init().
//
// Pipe __reserve* built-ins could fail only when the pipe is either
// empty, or full. If reserve was successful, read/write with reserve_id
// would never fail.
//
//
// Description of the algorithm:
// --------------------------------------
//
// Pipe struct has 2 internal buffers: circular buffer for packets and
// hazard flags buffer. Both buffers has a size of `max_packets * packet size`.
//
// Circular buffer:
//
//  hazard_read_end         hazard_write_begin
//     ~~~~~~~~~~v           v~~~~~~~~~
// |---+---+---+---+---+---+---+---+---+---|
// |   | r | r | x | x | x | w | w |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//       ^~~~ begin           end ~~~^
//
// Where:
//   r - packets reserved for read
//   x - already written packets, available for read
//   w - packets reserved for write
//
// Circular is protected by 4 indexes:
//
//   begin, end - buffer borders, all elements after end and before begin
//   has uninitialized values
//
//   hazard_read_end, hazard_write_begin - borders for _hazardous_ zone,
//   where _reserved_ values are stored (for future read or write
//   operations).
//
//
// Hazard flags:
//
// |---+---+---+---+---+---+---+---+---+---|
// |   | h | h |   |   |   | h | h |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//       ^~~~^~~~~~~~~~~~~~~~^~~~^
//      these elements are hazardous
//
// These flags are set int a _reserve_ operation and removed after a
// successful _read/write_ operation. These flags indicate which items
// are in a hazardous state, when their contents been written/read,
// because write and read are not atomic. See __read_pipe_4 example for a
// use-case.
//
//
// Full/Empty condition:
//
// There is no good way to differentiate between full and empty conditions,
// because `begin` should be equal to `end` in both cases. We change that by
// allowing only (max_packets - 1) writes to the buffer, which makes
// (begin == end) an *empty* and (begin == ++end) a *full* condition. See
// get_write_capacity() for details.
//
//
// When __reserve_read_pipe(pipe, num_packets) is called ...
//
//  hazard_read_end
//       v~~~~~~~
// |---+---+---+---+---+---+---+---+---+---|
// |   | x | x | x | x | x | w | w |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//       ^~~~ begin
//
// ... we should reserve `num_packets` items from the read end of the
// buffer. We first set the hazard flag for the first element: everyone
// else would see that flag and will be forced to retry. Then we
// atomically move the `hazard_read_end` forward by `num_packets` (let's
// assume 2):
//
//  hazard_read_end
//       ~~~~~~~~v
// |---+---+---+---+---+---+---+---+---+---|
// |   | r | r | x | x | x | w | w |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//       ^~~~ begin
//
// `begin` protects our reserved packets from being overwritten by
// overflow, and since `hazard_read_end` is moved, subsequent call to
// `__reserve_read_pipe` also would not affect them.
//
// We also mark the other reserved items as hazardous:
// |---+---+---+---+---+---+---+---+---+---|
// |   | h | h |   |   |   |   |   |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//
//
// When __read_pipe_4(pipe, reserve_id, index, dst_ptr) is called, we
// must read the reserved item at index (let's say 1) into the user
// allocated buffer _dst_ptr_.
//
//  hazard_read_end         hazard_write_begin
//       ~~~~~~~~+           +~~~~~~~~~
// item to read  |           |
//    ~~~~~~~v   v           v
// |---+---+---+---+---+---+---+---+---+---|
// |   | r | r | x | x | x | w | w |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//       ^~~~ begin           end ~~~^
//
// First, me memcpy() the item into dst. Then remove our hazard flag to
// indicate that write is completed. Then we check whether the `begin`
// points to our item. It it is not, so we just exit successfully and
// leave the task to move `begin` to the leftmost hazardous item (which
// `begin` points on).
//
// item to read
//    ~~~~~~~v
// |---+---+---+---+---+---+---+---+---+---|
// |   | h |   |   |   |   |   |   |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//       ^~~~ begin
//
// Next __read_pipe_4 with index = 0 (packet wise) will read the item at
// index 1 (buffer wise), unset its hazard flag, and then we will find
// out that `begin` points to our item and we must move it.
//
// So we find the next hazardous item or the `hazard_read_end` and move
// `begin` there. We also double check that next hazardous item is still
// hazardous, in case there was a race.
//
//  hazard_read_end
//       ~~~~~~~~v
// |---+---+---+---+---+---+---+---+---+---|
// |   |   |   | x | x | x | w | w |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//     begin ~~~~^
// |---+---+---+---+---+---+---+---+---+---|
// |   |   |   |   |   |   |   |   |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//
// Now all hazardous area is clear and we have no reserved items left.
// That is all for read operations.
//
//
// Write operations follows the similar logic:
//
// When __reserve_write_pipe(pipe, num_packets) is called ...
//
//                          hazard_write_begin
//                           v~~~~~~~~~
// |---+---+---+---+---+---+---+---+---+---|
// |   | r | r | x | x | x |   |   |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//                    end ~~~^
//
// ... we fist set the hazard flag of the `end` element, so
// `hazard_write_begin` cannot be moved our reserved. If we fail to do
// so, then someone have already reserved this item and we must start
// again.
//                          hazard_write_begin
//                           v~~~~~~~~~
// |---+---+---+---+---+---+---+---+---+---|
// |   |   |   |   |   |   | h |   |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//                    end ~~~^
//
// Once we did it, other writers cannot reserve anything: `end` item is
// hazardous. We must move the `end` by `num_packets`: this will be our
// reserved area.
//                          hazard_write_begin
//                           v~~~~~~~~~
// |---+---+---+---+---+---+---+---+---+---|
// |   | r | r | x | x | x | w | w |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//                            end ~~~^
//
// We should also mark the rest of the items hazardous:
//
// |---+---+---+---+---+---+---+---+---+---|
// |   |   |   |   |   |   | h | h |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//
// When __write_pipe(.., index=1, ..) is called, we memcpy() the item at
// index 7 (buffer wize), then remove it's hazard flag.
// `hazard_write_begin` does not point to our item, so we leave it
// unchanged.
//
// |---+---+---+---+---+---+---+---+---+---|
// |   |   |   |   |   |   | h |   |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//
// When __write_pipe(.., index=0, ..) is called, we, again, memcpy() the
// item at index 6 and remove our hazard flag. But in this case, we must
// also move the `hazard_write_begin`, because it points to our item.
//
// We find the first hazardous item ahead, and move `hazard_write_begin`
// there. If it was not found, then move it to the `end`.
//
//                          hazard_write_begin
//                           ~~~~~~~~v
// |---+---+---+---+---+---+---+---+---+---|
// |   | r | r | x | x | x | x | x |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//                            end ~~~^



#define __ovld __attribute__((overloadable))

// There are no declarations of OpenCL 2.0 builtins in opencl-c.h for named
// address space but in the library they has to be called directly because the
// library won't be handled by "Generic Address Resolution" passes. So declare
// them here.
void __ovld atomic_init(__global volatile atomic_int *object, int value);
int __ovld atomic_load(__global volatile atomic_int *object);
void __ovld atomic_store(__global volatile atomic_int *object, int desired);
bool __ovld atomic_compare_exchange_weak(__global volatile atomic_int *object,
                                         __private int *expected, int desired);

#if 0
#define PRINTF printf
#else
#define PRINTF (void)
#endif

#define DEBUG_INF_LOOPS 0
#define DEBUG_DUMP_HAZ_FLAGS 1
#define DEBUG_ASSERTS 0

struct __pipe_t {
  int packet_size;
  int max_packets;

  volatile atomic_int begin;
  volatile atomic_int end;
  volatile atomic_int hazard_write_begin;
  volatile atomic_int hazard_read_end;

  char buffer[0];
};

static bool get_hazard_flag(__global const struct __pipe_t* p, int index) {
  __global volatile atomic_int* flags = (__global volatile atomic_flag*) p->buffer;
  return atomic_load(&flags[index]);
}

static void set_hazard_flag(__global struct __pipe_t* p, int index, bool value) {
  __global volatile atomic_int* flags = (__global volatile atomic_flag*) p->buffer;
  return atomic_store(&flags[index], value);
}

static bool cmp_xchg_hazard_flag(__global struct __pipe_t* p,
                                  int index,
                                  bool* expected, bool value) {
  __global volatile atomic_int* flags = (__global volatile atomic_flag*) p->buffer;

  int expected_int = *expected;
  bool result = atomic_compare_exchange_weak(&flags[index],
                                             &expected_int, value);
  *expected = expected_int;
  return result;
}


/// Return next nth item from circular buffer
static int advance(__global const struct __pipe_t* p, int index, int offset) {
  return (index + offset) % p->max_packets;
}

/// For given \p index_from and \p index_to compute the number of elements
/// between them. The function behaves exactly as std::distance.
static int dist(__global const struct __pipe_t* p,
                    int index_from, int index_to) {
  return index_from <= index_to ? index_to - index_from
    : p->max_packets - index_from + index_to;
}

/// Return the number of elements that can be written into the \p pipe.
/// Note that this capacity may be already outdated, since begin/end could
/// change at any time.
static int get_write_capacity(__global const struct __pipe_t* p,
                              int begin, int end) {
  int avail = dist(p, end, begin);

  // handle full/empty condition
  if (avail > 0) {
    avail = avail - 1;
  } else {
    // begin == end, so the buffer is empty
    avail = p->max_packets - 1;
  }

  PRINTF("get_write_capacity: b = %d, e = %d, avail: %d\n", begin, end, avail);
  return avail;
}

/// Return the number of elements that can be read into the \p pipe.
/// Note that this capacity may be already outdated, since
/// hazard_read_end/hazard_write_begin could change at any time.
static int get_read_capacity(__global const struct __pipe_t* p,
                             int hazard_read_end, int hazard_write_begin) {
  int avail = dist(p, hazard_read_end, hazard_write_begin);
  PRINTF("get_read_capacity: hre = %d, hwb = %d, avail: %d\n",
         hazard_read_end, hazard_write_begin, avail);
  return avail;
}

/// Return the pointer on the beginning of packet with given \p index
static void* get_packet_ptr(__global struct __pipe_t* p, int index) {
  // skip hazard flags
  char* packets_buffer = p->buffer + sizeof(atomic_int) * (p->max_packets);
  return packets_buffer + (p->packet_size * index);
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

/// Find index of the first hazardous item between \p index_from and \p index_to.
/// Returns \p index_to if no hazardous item found in range.
static int find_hazard_index(__global struct __pipe_t* p,
                             int index_from, int index_to) {
  for (int i = index_from; i != index_to; i = advance(p, i, 1)) {
    if (get_hazard_flag(p, i)) {
      return i;
    }
  }

  return index_to;
}

static int __pipe_dump(__global struct __pipe_t* p) {
  printf(">>>> pipe dump:\n"
         ">> packet_size = %d, max_packets = %d\n"
         ">> begin = %d, end = %d\n"
         ">> hazard_read_end = %d, hazard_write_begin = %d\n"
         ">> write_capacity = %d\n"
         ">> read_capacity = %d\n",
         p->packet_size, p->max_packets,
         atomic_load(&p->begin), atomic_load(&p->end),
         atomic_load(&p->hazard_read_end),
         atomic_load(&p->hazard_write_begin),
         get_write_capacity(p, atomic_load(&p->begin),
                            atomic_load(&p->end)),
         get_read_capacity(p, atomic_load(&p->hazard_read_end),
                           atomic_load(&p->hazard_write_begin)));
#if DEBUG_DUMP_HAZ_FLAGS
  printf(">> hazard flags = ");
  for (int i = 0; i < p->max_packets; ++i) {
    printf("%d", (int)get_hazard_flag(p, i));
  }
  printf("\n");
#endif

}

bool __ovld is_valid_reserve_id(reserve_id_t reserve_id) {
  return (intptr_t)__builtin_astype(reserve_id, __global void*)
    & __PIPE_RESERVE_ID_VALID_BIT;
}


void __pipe_init_intel(__global struct __pipe_t* p,
                       int packet_size, int max_packets) {
  p->packet_size = packet_size;
  p->max_packets = max_packets;

  atomic_init(&p->begin, 0);
  atomic_init(&p->end, 0);
  atomic_init(&p->hazard_write_begin, 0);
  atomic_init(&p->hazard_read_end, 0);

  __global volatile atomic_int* hazard_flags =
    (__global volatile atomic_flag*) p->buffer;

  for (int i = 0; i < max_packets; ++i) {
    atomic_init(hazard_flags + i, 0);
  }
}

reserve_id_t __reserve_write_pipe_intel(__global struct __pipe_t* p,
                                        uint num_packets) {
  int end = atomic_load(&p->end);
  int reserved = end;

  while (true) {
    int begin = atomic_load(&p->begin);

    int avail = get_write_capacity(p, begin, end);

    if (avail < (int) num_packets) {
      PRINTF("__reserve_write_pipe: overflow: avail = %d, req = %d, "
             "begin = %d, end = %d\n",
             avail, num_packets, begin, end);

      return CLK_NULL_RESERVE_ID;
    }

    reserved = end;

    // first, set the hazard flag to make sure that `hazard_write_begin` will
    // not be moved ahead of our first packet when we move the `end`.
    bool hazard_expected = false;
    if (!cmp_xchg_hazard_flag(p, reserved, &hazard_expected, true)) {
      // somebody set the hazard flag before us
      // they propbably moved the end, so we must load it and start again

      end = atomic_load(&p->end);

      // TODO: asavonic: performance: maybe skip the load here?  if the `end` is
      // not yet moved, we get one extra load and still must make another
      // iteration.

      continue;
    }

    int new_end = advance(p, end, num_packets);
    if (atomic_compare_exchange_weak(&p->end, &end, new_end)) {
      break;
    }
  }

  for (int i = advance(p, reserved, 1);
       i != advance(p, reserved, num_packets); i = advance(p, i, 1)) {
    set_hazard_flag(p, i, true);
  }

  PRINTF("__reserve_write_pipe: reserved %d elements starting at index %d\n",
         num_packets, reserved);

  return create_reserve_id(reserved);
}

int __write_pipe_4_intel(__global struct __pipe_t* p, reserve_id_t reserve_id,
                         uint index, const void* src) {
  PRINTF("__write_pipe_4: enter\n", index);
  int reserved = extract_reserve_id(reserve_id);

  int write_index = advance(p, reserved, index);
  PRINTF("__write_pipe_4: writing at index %d\n", write_index);
  __builtin_memcpy(get_packet_ptr(p, write_index), src, p->packet_size);

  set_hazard_flag(p, write_index, false);

  // OK, write is done and our hazard flag is not set. Now we need to atomically
  // move `hazard_write_begin` forward to the first hazardous item or to the
  // `end`.
  int hazard_write_begin = atomic_load(&p->hazard_write_begin);

  if (hazard_write_begin != write_index) {
    // only the first hazardous item should move the `hazard_write_begin`
    PRINTF("__write_pipe_4: ok at index %d\n", write_index);
    return 0;
  }

  PRINTF("__write_pipe_4: start update hazard_write_begin from %d\n",
         hazard_write_begin);

  while (true) {
    int end = atomic_load(&p->end);
    int haz_index = find_hazard_index(p, advance(p, write_index, 1), end);

    atomic_store(&p->hazard_write_begin, haz_index);

    if (haz_index == end) {
      PRINTF("__write_pipe_4: hazardous area is clear, end = %d\n", end);
      break;
    }

    // make sure `haz_index` item didn't unset hazard flag between our
    // find_hazard_index() and `hazard_write_begin` move
    if (get_hazard_flag(p, haz_index) == true) {
      // it's still hazardous, we can rely on him in moving `hazard_write_begin`
      PRINTF("__write_pipe_4: updated hazard_write_begin to %d\n", haz_index);
      break;
    }
  }

  return 0;
}


reserve_id_t __reserve_read_pipe_intel(__global struct __pipe_t* p,
                                       uint num_packets) {
  int hazard_read_end = atomic_load(&p->hazard_read_end);
  int reserved = hazard_read_end;

  int attempts = 1000000;
  while (true) {
#if DEBUG_INF_LOOPS
    if (!--attempts) {
      attempts = 1000000;
      printf("__reserve_read_pipe: deadlock detected!");
      __pipe_dump(p);
      *((volatile int*)NULL) = 0xdead;
    }
#endif
    int hazard_write_begin = atomic_load(&p->hazard_write_begin);

    int avail = get_read_capacity(p, hazard_read_end, hazard_write_begin);

    if (avail < (int) num_packets) {
      PRINTF("__reserve_read_pipe: overflow: avail = %d, req = %d, "
             "hre = %d, hwb = %d\n",
             avail, num_packets, hazard_read_end, hazard_write_begin);

      return CLK_NULL_RESERVE_ID;
    }

    reserved = hazard_read_end;

    // first, set the hazard flag to make sure that `hazard_read_end` will not be
    // moved to the item ahead of our first packet after we move the `end`.
    bool hazard_expected = false;
    if (!cmp_xchg_hazard_flag(p, reserved, &hazard_expected, true)) {
      // somebody set the hazard flag before us
      // they probably moved the end, so we must load it and start again

      hazard_read_end = atomic_load(&p->hazard_read_end);

      // TODO: asavonic: performance: maybe skip the load here?  if the `end` is
      // not yet moved, we get one extra load and still must make another
      // iteration.

      continue;
    }


    int new = advance(p, hazard_read_end, num_packets);
    if (atomic_compare_exchange_weak(&p->hazard_read_end, &hazard_read_end,
                                       new)) {
      break;
    }
  }

  for (int i = advance(p, reserved, 1);
       i != advance(p, reserved, num_packets); advance(p, i, 1)) {
    set_hazard_flag(p, i, true);
  }

  PRINTF("__reserve_read_pipe: reserved %d elements starting at index %d\n",
         num_packets, reserved);

  return create_reserve_id(reserved);
}

int __read_pipe_4_intel(__global struct __pipe_t* p, reserve_id_t reserve_id,
                        uint index, void* dst) {
  int reserved = extract_reserve_id(reserve_id);

  int read_index = advance(p, reserved, index);
  __builtin_memcpy(dst, get_packet_ptr(p, read_index), p->packet_size);

  set_hazard_flag(p, read_index, false);

  // OK, read is done and our hazard flag is not set. Now we need to atomically
  // move `begin` forward to the first hazardous item or to the
  // `hazard_read_end`.
  int begin = atomic_load(&p->begin);

  if (begin != read_index) {
    // only the first hazardous item should move the `begin`
    PRINTF("__read_pipe_4: ok at index %d\n", read_index);
    return 0;
  }

  PRINTF("__read_pipe_4: start update begin from %d\n", begin);

  long attempts = 100000;
  while (true) {
#if DEBUG_INF_LOOPS
    if (!--attempts) {
      attempts = 1000000;
      printf("__read_pipe: deadlock detected!");
      __pipe_dump(p);
      *((volatile int*)NULL) = 0xdead;
    }
#endif

    int hazard_read_end = atomic_load(&p->hazard_read_end);
    int haz_index = find_hazard_index(p, advance(p, read_index, 1),
                                      hazard_read_end);

    atomic_store(&p->begin, haz_index);

    if (haz_index == hazard_read_end) {
      PRINTF("__read_pipe_4: hazardous area is clear, begin = %d\n",
             haz_index);
      break;
    }

    // make sure `haz_index` item didn't unset hazard flag between our
    // find_hazard_index() and `begin` move
    if (get_hazard_flag(p, haz_index) == true) {
      // it's still hazardous, we can rely on him in moving `hazard_write_begin`
      PRINTF("__read_pipe_4: updated begin to %d\n", haz_index);
      break;
    }
  }

  return 0;
}

int __read_pipe_2_intel(__global struct __pipe_t* p, void* dst) {
  reserve_id_t id = __reserve_read_pipe_intel(p, 1);
  if (!is_valid_reserve_id(id)) {
    return 1;
  }
  return __read_pipe_4_intel(p, id, 0, dst);
}

int __write_pipe_2_intel(__global struct __pipe_t* p, void* src) {
  reserve_id_t id = __reserve_write_pipe_intel(p, 1);
  if (!is_valid_reserve_id(id)) {
    PRINTF("__write_pipe_2: reserve id was invalid\n");
    return 1;
  }
  PRINTF("__write_pipe_2: reserve id - ok\n");
  return __write_pipe_4_intel(p, id, 0, src);
}

int __read_pipe_2_bl_intel(__global struct __pipe_t* p, void* dst) {
  long attempts = 100000;
  while(__read_pipe_2_intel(p, dst)) {
#if DEBUG_INF_LOOPS
    if (!--attempts) {
      printf("read_pipe deadlock detected:\n");
      __pipe_dump(p);
      *((volatile int*)NULL) = 0xdead;
      attempts = 100000;
    }
#endif
  }
  return 0;
}

int __write_pipe_2_bl_intel(__global struct __pipe_t* p, void* src) {
  long attempts = 10000000;
  while(__write_pipe_2_intel(p, src)) {
#if DEBUG_INF_LOOPS
    if (!--attempts) {
      printf("write_pipe deadlock detected:\n");
      __pipe_dump(p);
      *((volatile int*)NULL) = 0xdead;
      attempts = 1000000000;
    }
#endif
  }
  return 0;
}
