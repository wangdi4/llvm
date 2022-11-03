__kernel void set_false(__global bool *data) {
  for (int i = 0; i < 4; ++i)
    if (data[i])
      data[i] = false;
}
