// Pipe struct has 2 internal buffers: circular buffer for packets and
// hazard flags buffer.
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
//   where _reserved_ values are stored.
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
// These flags are set after _reserve_ operation and removed after
// successful _read/write_ operation. They are required to determine
// which elements in hazardous area are not yet commited.
// See __read_pipe_4 example for a use-case.
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
// ... we should reserve _num_packets_ items from the read end of the
// buffer, we atomically move the _hazard_read_end_ forward by
// _num_packets (let's assume 2):
//
//  hazard_read_end
//       ~~~~~~~~v
// |---+---+---+---+---+---+---+---+---+---|
// |   | r | r | x | x | x | w | w |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//       ^~~~ begin
//
// _begin_ protects our reserved packets from being overwritten by
// overflow, and since _hazard_read_end_ is moved, subsequent calls to
// __reserve_read_pipe also would not affect them.
//
// We also mark the reserved items as hazardous:
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
// First, me memcpy() the item into dst. Then remove our hazard flag to indicate
// that write is completed. Then our goal is to move the _hazard_write_begin_
// and make it point to a valid item (hazardous or _end_). So we find the first
// hazardous item in range [_hazard_write_begin, _end_) and move
// _hazard_write_begin_ there.

// TODO: asavonic: ohh, crap. the paragraph above belongs to
// __write_pipe_4. move it there and write a corresponding one here

// we check if we can move the
// _begin_: if all elements to the left are not hazardous, it is safe to
// move _begin_ forward up to the next hazardous element or
// _hazard_read_end_. Since the item at index 1 is still hazardous, it
// is not legal, so we do nothing with _begin_ and remove our hazardous
// flag.
//
//  // TODO: asavonic: fix a buf with begin forwarding: make it move to the
//  leftmost hazard item
//
// |---+---+---+---+---+---+---+---+---+---|
// |   | h |   |   |   |   |   |   |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//
// Next __read_pipe_4 with index = 0 (packet wise) will read the item at
// index 1 (buffer wise) and move begin to the hazard_read_end.
//
//  hazard_read_end
//       ~~~~~~~~v
// |---+---+---+---+---+---+---+---+---+---|
// |   |   |   | x | x | x | w | w |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//     begin ~~~~^
//
// Since we moved _begin_, the area before begin may already be occupied,
// and it is not safe to remove the hazardous flag.
// There is nothing bad in leaving it hazardous, because:
//   - we don't care about its state, because we don't maintain this area
//     (it is before begin)
//   - it will only be affected when __reserve_write takes place, and it
//     will only _set_ the flag
//
// // TODO asavonic: fix this!
//
//
// Since we don't care about this hazard flag, consider it removed:
//
// |---+---+---+---+---+---+---+---+---+---|
// |   |   |   |   |   |   |   |   |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//
//
// Write operations follows the same logic:
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
// We move _end_ by 2 items, and mark them hazardous:
//
//                          hazard_write_begin
//                           v~~~~~~~~~
// |---+---+---+---+---+---+---+---+---+---|
// |   | r | r | x | x | x | w | w |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//                            end ~~~^
//
// |---+---+---+---+---+---+---+---+---+---|
// |   |   |   |   |   |   | h | h |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//
// When __write_pipe(.., index=1, ..) is called, we memcpy() the item at
// index 7 (buffer wize), and remove it's hazard flag, since we cannot
// move _hazard_write_begin_ yet.
//
// |---+---+---+---+---+---+---+---+---+---|
// |   |   |   |   |   |   | h |   |   |   |
// |---+---+---+---+---+---+---+---+---+---|
//
// When __write_pipe(.., index=0, ..) is called, we memcpy() the item at
// index 6, and then check whether we can move _hazard_write_begin_:
// yes, we can (no hazardous items before ours). Move it, and leave our
// hazard flag, since we don't care about it (see __read_pipe_4 example).

#define __ovld __attribute__((overloadable))


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
/// TODO: asavonic: rename into distance() after integraion into BE (when it
/// won't include opencl-c.h), otherwize its name conflicts with: half __ovld
/// __cnfn distance(half4 p0, half4 p1);
static int __distance(__global const struct __pipe_t* p,
                    int index_from, int index_to) {
  return index_from < index_to ? index_to - index_from
    : p->max_packets - index_from + index_to;
}

/// Return the number of elements that can be written into the \p pipe.
/// Note that this capacity may be already outdated, since begin/end could
/// change at any time.
static int get_write_capacity(__global const struct __pipe_t* p,
                              int begin, int end) {
  return __distance(p, end, begin);
}

/// Return the number of elements that can be read into the \p pipe.
/// Note that this capacity may be already outdated, since
/// hazard_read_end/hazard_write_begin could change at any time.
static int get_read_capacity(__global const struct __pipe_t* p,
                             int hazard_read_end, int hazard_write_begin) {
  return __distance(p, hazard_read_end, hazard_write_begin);
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

bool __ovld is_valid_reserve_id(reserve_id_t reserve_id) {
  return (intptr_t)__builtin_astype(reserve_id, __global void*)
    & __PIPE_RESERVE_ID_VALID_BIT;
}


void __pipe_init(__global struct __pipe_t* p, int packet_size, int max_packets) {
  p->packet_size = packet_size;
  p->max_packets = max_packets;

  atomic_init(&p->begin, -1);
  atomic_init(&p->end, 0);
  atomic_init(&p->hazard_write_begin, 0);
  atomic_init(&p->hazard_read_end, 0);

  __global volatile atomic_int* hazard_flags =
    (__global volatile atomic_flag*) p->buffer;

  for (int i = 0; i < max_packets; ++i) {
    atomic_init(hazard_flags + i, 0);
  }
}

reserve_id_t __reserve_write_pipe(__global struct __pipe_t* p, uint num_packets) {
  int end = atomic_load(&p->end);
  int reserved = end;

  while (true) {
    int begin = atomic_load(&p->begin);

    int avail = get_write_capacity(p, begin, end);

    if (avail < (int) num_packets) {
      printf("__reserve_write_pipe: overflow: avail = %d, req = %d\n",
             avail, num_packets);

      return CLK_NULL_RESERVE_ID;
    }

    reserved = end;

    // first, set the hazard flag to make sure that hazard_write_begin will not
    // be moved ahead of our first packet when we move the _end_.
    bool hazard_expected = false;
    if (!cmp_xchg_hazard_flag(p, reserved, &hazard_expected, true)) {
      // somebody set the hazard flag before us
      // they probably moved the end, so we must load it and start again

      end = atomic_load(&p->end);

      // TODO: asavonic: performance: maybe skip the load here?  if the _end_ is
      // not yet moved, we get one extra load and still must make another
      // iteration.

      // TODO: asavonic: update the docs

      continue;
    }

    int new_end = advance(p, end, num_packets);
    if (atomic_compare_exchange_strong(&p->end, &end, new_end)) {
      break;
    }
  }

  for (int i = advance(p, reserved, 1);
       i != advance(p, reserved, num_packets); i = advance(p, i, 1)) {
    set_hazard_flag(p, i, true);
  }

  printf("__reserve_write_pipe: reserved %d elements starting at index %d\n",
         num_packets, reserved);

  return create_reserve_id(reserved);
}

int __write_pipe_4(__global struct __pipe_t* p, reserve_id_t reserve_id,
                   uint index, const void* src) {
  printf("__write_pipe_4: enter\n", index);
  int reserved = extract_reserve_id(reserve_id);

  int write_index = advance(p, reserved, index);
  printf("__write_pipe_4: writing at index %d\n", write_index);
  __builtin_memcpy(get_packet_ptr(p, write_index), src, p->packet_size);

  set_hazard_flag(p, write_index, false);

  // OK, write is done and our hazard flag is not set. Now we need to atomically
  // move _hazard_write_begin_ forward to the first hazardous item or to the
  // _end_.
  int hazard_write_begin = atomic_load(&p->hazard_write_begin);

  if (hazard_write_begin != write_index) {
    // only the first hazardous item should move the _hazard_write_begin_
    printf("__write_pipe_4: ok at index %d", write_index);
    return 0;
  }

  printf("__write_pipe_4: start update hazard_write_begin from %d\n",
         hazard_write_begin);

  while (true) {
    int end = atomic_load(&p->end);
    int haz_index = find_hazard_index(p, advance(p, write_index, 1), end);

    atomic_store(&p->hazard_write_begin, haz_index);

    if (haz_index == end) {
      printf("__write_pipe_4: hazardous area is clear, end = %d\n", end);
      break;
    }

    // make sure haz_index item didn't unset hazard flag between our
    // find_hazard_index() and hazard_write_begin move
    if (get_hazard_flag(p, haz_index) == true) {
      // it's still hazardous, we can rely on him in moving hazard_write_begin
      printf("__write_pipe_4: updated hazard_write_begin to %d\n", haz_index);
      break;
    }
  }

  return 0;
}


reserve_id_t __reserve_read_pipe(__global struct __pipe_t* p, uint num_packets) {
  int hazard_read_end = atomic_load(&p->hazard_read_end);
  int reserved = hazard_read_end;

  while (true) {
    int hazard_write_begin = atomic_load(&p->hazard_write_begin);

    int avail = get_read_capacity(p, hazard_read_end, hazard_write_begin);

    if (avail < (int) num_packets) {
      printf("__reserve_read_pipe: overflow: avail = %d, req = %d\n",
             avail, num_packets);

      return CLK_NULL_RESERVE_ID;
    }

    reserved = hazard_read_end;

    // first, set the hazard flag to make sure that hazard_read_end will not
    // be moved to the item after our first packet when we move the _end_.
    bool hazard_expected = false;
    if (!cmp_xchg_hazard_flag(p, reserved, &hazard_expected, true)) {
      // somebody set the hazard flag before us
      // they probably moved the end, so we must load it and start again

      hazard_read_end = atomic_load(&p->hazard_read_end);

      // TODO: asavonic: performance: maybe skip the load here?  if the _end_ is
      // not yet moved, we get one extra load and still must make another
      // iteration.

      // TODO: asavonic: update the docs

      continue;
    }


    int new = advance(p, hazard_read_end, num_packets);
    if (atomic_compare_exchange_strong(&p->hazard_read_end, &hazard_read_end,
                                       new)) {
      break;
    }
  }

  for (int i = advance(p, reserved, 1);
       i != advance(p, reserved, num_packets); advance(p, i, 1)) {
    set_hazard_flag(p, i, true);
  }

  printf("__reserve_read_pipe: reserved %d elements starting at index %d\n",
         num_packets, reserved);

  return create_reserve_id(reserved);
}

int __read_pipe_4(__global struct __pipe_t* p, reserve_id_t reserve_id,
                  uint index, void* dst) {
  int reserved = extract_reserve_id(reserve_id);

  int read_index = advance(p, reserved, index);
  __builtin_memcpy(dst, get_packet_ptr(p, read_index), p->packet_size);

  set_hazard_flag(p, read_index, false);

  // OK, read is done and our hazard flag is not set. Now we need to atomically
  // move _begin_ forward to the first hazardous item or to the
  // _hazard_read_end_.
  int begin = atomic_load(&p->begin);

  if (begin != read_index) {
    // only the first hazardous item should move the _begin_
    printf("__read_pipe_4: ok at index %d", read_index);
    return 0;
  }

  printf("__read_pipe_4: start update begin from %d\n", begin);

  while (true) {
    int hazard_read_end = atomic_load(&p->hazard_read_end);
    int haz_index = find_hazard_index(p, advance(p, read_index, 1),
                                      hazard_read_end);

    atomic_store(&p->begin, haz_index);

    if (haz_index == hazard_read_end) {
      printf("__read_pipe_4: hazardous area is clear, begin = %d\n",
             haz_index);
      break;
    }

    // make sure haz_index item didn't unset hazard flag between our
    // find_hazard_index() and begin move
    if (get_hazard_flag(p, haz_index) == true) {
      // it's still hazardous, we can rely on him in moving hazard_write_begin
      printf("__write_pipe_4: updated begin to %d\n", haz_index);
      break;
    }
  }

  return 0;
}


int __read_pipe_2(__global struct __pipe_t* p, void* dst) {
  reserve_id_t id = __reserve_read_pipe(p, 1);
  if (!is_valid_reserve_id(id)) {
    return 1;
  }
  return __read_pipe_4(p, id, 0, dst);
}

int __write_pipe_2(__global struct __pipe_t* p, void* dst) {
  reserve_id_t id = __reserve_write_pipe(p, 1);
  if (!is_valid_reserve_id(id)) {
    printf("__write_pipe_2: reserve id was invalid\n");
    return 1;
  }
    printf("__write_pipe_2: reserve id - ok\n");
  return __write_pipe_4(p, id, 0, dst);
}
