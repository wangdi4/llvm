#include <CL/cl.h>

bool write_bmp_image(const char * filename,cl_uchar4 * pixels_);
cl_uchar4 * read_bmp_image(const char * filename, cl_uint * width, cl_uint *heigh);