kernel void test_write(write_only pipe int p, int data) {
  write_pipe(p, &data);
}

kernel void test_read(read_only pipe int p, int data) { read_pipe(p, &data); }
