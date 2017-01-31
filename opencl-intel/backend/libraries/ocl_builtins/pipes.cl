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
// First, me memcpy() the item into dst. Then we check if we can move the
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

static int get_write_capacity(int begin, int end, int max_packets) {
  return end > begin ? max_packets - end + begin
                     : begin - end;
}

static int get_read_capacity(int hazard_read_end, int hazard_write_begin,
                             int max_packets) {
  return hazard_write_begin > hazard_read_end
    ? hazard_write_begin - hazard_read_end
    : max_packets - hazard_read_end + hazard_write_begin;
}

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

  while (true) {
    int begin = atomic_load(&p->begin);

    int avail = get_write_capacity(begin, end, p->max_packets);

    if (avail < (int) num_packets) {
      printf("__reserve_write_pipe: overflow: avail = %d, req = %d\n",
             avail, num_packets);

      return CLK_NULL_RESERVE_ID;
    }

    int new_end = (end + num_packets) % p->max_packets;
    if (atomic_compare_exchange_strong(&p->end, &end, new_end)) {
      break;
    }
  }

  int reserved = end;

  for (int i = reserved; i < reserved + num_packets; ++i) {
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

  int write_index = (reserved + index) % p->max_packets;
  printf("__write_pipe_4: writing at index %d\n", write_index);
  __builtin_memcpy(get_packet_ptr(p, write_index), src, p->packet_size);

  int hazard_write_begin = atomic_load(&p->hazard_write_begin);
  printf("__write_pipe_4: hazard_write_begin = %d\n", hazard_write_begin);
  while (true) {
    bool clear = true;
    for (int i = hazard_write_begin; i != write_index;
         i = (i + 1) % p->max_packets) {
      if (get_hazard_flag(p, i)) {
        clear = false;
        break;
      }
    }

    if (clear) {
      // everything from hazard_write_begin to this packet is clear:
      // move hazard begin accordingly
      int new = ++write_index % p->max_packets;
      if (atomic_compare_exchange_weak(&p->hazard_write_begin, &hazard_write_begin,
                                       new)) {
        printf("__write_pipe_4: updated hazard_write_begin to %d\n",
               new);

        break;
      };
    }
  }

  // doing this _after_ hazard_write_begin update to ensure that we don't move it
  // backwards, because nobody can move it beyond our write_index while we keep it
  // hazardous
  set_hazard_flag(p, write_index, false);

  return 0;
}


reserve_id_t __reserve_read_pipe(__global struct __pipe_t* p, uint num_packets) {
  int hazard_read_end = atomic_load(&p->hazard_read_end);

  while (true) {
    int hazard_write_begin = atomic_load(&p->hazard_write_begin);

    int avail = get_read_capacity(hazard_read_end, hazard_write_begin,
                                  p->max_packets);

    if (avail < (int) num_packets) {
      printf("__reserve_read_pipe: overflow: avail = %d, req = %d\n",
             avail, num_packets);

      return CLK_NULL_RESERVE_ID;
    }

    int new = (hazard_read_end + num_packets) % p->max_packets;
    if (atomic_compare_exchange_strong(&p->hazard_read_end, &hazard_read_end,
                                       new)) {
      break;
    }
  }

  int reserved = hazard_read_end;

  for (int i = reserved; i < reserved + num_packets; ++i) {
    set_hazard_flag(p, i, true);
  }

  printf("__reserve_read_pipe: reserved %d elements starting at index %d\n",
         num_packets, reserved);

  return create_reserve_id(reserved);
}

int __read_pipe_4(__global struct __pipe_t* p, reserve_id_t reserve_id,
                  uint index, void* dst) {
  int reserved = extract_reserve_id(reserve_id);

  int read_index = (reserved + index) % p->max_packets;
  __builtin_memcpy(dst, get_packet_ptr(p, read_index), p->packet_size);

  int begin = atomic_load(&p->begin);
  while (true) {
    bool clear = true;
    for (int i = begin == -1 ? 0 : begin; i != read_index;
         i = (i + 1) % p->max_packets) {
      if (get_hazard_flag(p, i)) {
        clear = false;
        break;
      }
    }

    if (clear) {
      // everything from begin to this packet is clear:
      // move begin accordingly
      int new = ++read_index % p->max_packets;
      if (atomic_compare_exchange_weak(&p->begin, &begin, new)) {
        printf("__read_pipe_4: updated begin to %d\n", new);
        break;
      };
    }
  }

  // doing this _after_ hazard_write_begin update to ensure that we don't move it
  // backwards, because nobody can move it beyond our write_index while we keep it
  // hazardous
  set_hazard_flag(p, read_index, false);

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
