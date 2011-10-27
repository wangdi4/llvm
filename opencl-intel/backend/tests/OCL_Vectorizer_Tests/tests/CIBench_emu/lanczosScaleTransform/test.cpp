#include <libc.h>
#include <stdbool.h>
#include <OpenCL/opencl.h>
#include <ApplicationServices/ApplicationServices.h>
#include <mach/mach_time.h>

#include <iostream>
#include <fstream>
#include <vector>
//#include <assert.h>
#include <sys/stat.h>
//#include <time.h>

using namespace std;


  typedef struct CGDataProviderAccessSession CGAccessSession;
  CG_EXTERN const CFStringRef kCGImageSourceShouldPreferRGB32;
  CG_EXTERN CGAccessSession *CGAccessSessionCreate(CGDataProviderRef provider);
  CG_EXTERN size_t CGAccessSessionGetBytes(CGAccessSession *session,
    void *buffer, size_t count);



#define WIDTH       (1024)
#define HEIGHT      (1024)


cl_program createProgramFromFile(const char* fileName, cl_context context)
{
	// Load OpenCL source file
	struct stat stbuf;
	if(stat(fileName, &stbuf) == -1) {
		printf("Could not stat file %s\n", fileName);
		return NULL;
	}
	int clCodeStrSize = stbuf.st_size;
	char *clCodeStr = (char*)malloc(sizeof(char)*(clCodeStrSize + 1));
	if (!clCodeStr)
	{
		printf("Malloc failed!\n");
		return NULL;
	}
	
	ifstream inFile(fileName);
	if (!inFile) {
		printf("Failed to open file %s!\n", fileName);
		return NULL;
	}
	inFile.read(clCodeStr, clCodeStrSize);
	clCodeStr[clCodeStrSize] = '\0';
	inFile.close();
	
	// Load the source into OpenCL driver
	cl_program prog = clCreateProgramWithSource(context, 1, (const char**)&clCodeStr, NULL, NULL);
	if (!prog)
	{
		printf("clCreateProgramWithSource() failed.\n");
		return NULL;
	}
	return prog;
}




void providerFree(void *info, const void *data, size_t size)
{
    free((void *)data);
}

int main(int argc, char **argv)
{
	unsigned         k;
	int				 arg=1;
	uint64_t         t0,t1,t2;
	int              err;
	cl_device_id	 device_id;
	cl_context       context;
	cl_command_queue queue;
	cl_kernel        kernel;
	cl_program       program;
	cl_mem           dst, src;
	cl_image_format  format;
	cl_sampler       sampler;
	uint8_t          *mem, *data;
	size_t           global[2], local[2];
	size_t           groupsize;
	size_t           w,h, bpr;
	unsigned         iterations = 10;
	bool             gpu = false;
	bool             check = false;
	bool			 trans = false;
	CGImageRef       img;
	CGColorSpaceRef colorspace;
	CGColorRenderingIntent renderintent;
	CGImageAlphaInfo imagealpha;
	
	char filterFileName[50] = "kernel.cl";
	char kernelName[40] = ".";
	char imageFile[50];
	bool isUsingImage = false;
	bool output = false;
	char outputFile[50];
	int subdiv = 1;
	
	
	printf("Existing arguments: -gpu   -trans   -iter N   -subdiv N   -filterFile NNN   -kernelName NNN   -imageFile NNN   -output NNN\n");
	// Iterate over all input parameters
	while (arg < argc)
	{
		if (strcmp(argv[arg], "-gpu") == 0)
		{
			if (trans)
			{
				printf("Error! Cannot run transposed on GPU!\n");
				return EXIT_FAILURE;
			}			
			gpu = true;
			arg++;
			continue;
		}
		if (strcmp(argv[arg], "-trans") == 0)
		{
			if (gpu || subdiv>1)
			{
				printf("Error! Cannot run transposed on GPU or with a subdivision!\n");
				return EXIT_FAILURE;
			}			
			trans = true;
			arg++;
			continue;
		}
		if (strcmp(argv[arg], "-iter") == 0)
		{
			iterations=atoi(argv[arg+1]);
			arg+=2;
			continue;
		}
		if (strcmp(argv[arg], "-subdiv") == 0)
		{
			if (trans)
			{
				printf("Error! Cannot subdivide transposed version!\n");
				return EXIT_FAILURE;
			}			
			subdiv=atoi(argv[arg+1]);
			arg+=2;
			continue;
		}
		if (strcmp(argv[arg], "-filterFile") == 0)
		{
			strcpy(filterFileName, argv[arg+1]);
			arg+=2;
			continue;
		}
		if (strcmp(argv[arg], "-kernelName") == 0)
		{
			strcpy(kernelName, argv[arg+1]);
			arg+=2;
			continue;
		}
		if (strcmp(argv[arg], "-imageFile") == 0)
		{
			strcpy(imageFile, argv[arg+1]);
			isUsingImage = true;
			arg+=2;
			continue;
		}
		if (strcmp(argv[arg], "-output") == 0)
		{
			strcpy(outputFile, argv[arg+1]);
			output=true;
			arg+=2;
			continue;
		}
		
		printf("Error! Unknown input argument (%s) !\n", argv[arg]);
		return EXIT_FAILURE;
	}
	
	if (kernelName[0] == '.')
	{
		if (!trans)
			strcpy(kernelName, "program");
		else
			strcpy(kernelName, "program_trans");
	}
		
	printf("Execution parameters:\n");
	printf("Executing on device: %s\n", gpu? "GPU":"CPU");
	printf("Number of iterations: %d\n", iterations);
	printf("Image file name: %s\n", isUsingImage ? imageFile : "not defined - using random");
	printf("Filter file: %s    Kernel name: %s \n", filterFileName, kernelName);
	if (output)
		printf("Output file name: %s\n", outputFile);
	
	if (isUsingImage)
	{
		CGAccessSession   *session;
		CFStringRef        path;
		CFDictionaryRef    dict;
		CGImageSourceRef   isrc;
		CGDataProviderRef  prov;
		CFURLRef           url;
		size_t             bpp;
		
		path    = CFStringCreateWithCString(NULL, imageFile, kCFStringEncodingUTF8);
		url     = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, false);
		
		dict    = CFDictionaryCreate(kCFAllocatorDefault,
									 (const void **)&kCGImageSourceShouldPreferRGB32,(const void **)&kCFBooleanTrue, 1,
									 &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		
		isrc    = CGImageSourceCreateWithURL(url, NULL);
		img     = CGImageSourceCreateImageAtIndex(isrc, 0, dict);
		
		prov    = CGImageGetDataProvider(img);
		session = CGAccessSessionCreate(prov);
		
		w       = CGImageGetWidth(img);
		h       = CGImageGetHeight(img);
		bpr     = CGImageGetBytesPerRow(img);
		bpp     = CGImageGetBitsPerPixel(img)/8;
		
		colorspace = CGImageGetColorSpace (img);
		renderintent = CGImageGetRenderingIntent (img);
		imagealpha = CGImageGetAlphaInfo (img);
		
		data    = (uint8_t*)malloc(h*bpr);
		
		fprintf(stdout, "%s  Size:%dx%d  bytesPerPixel:%d\n", imageFile, (int)w,(int)h, (int)bpp);
		fflush(stdout);
		
		CGAccessSessionGetBytes(session, data, h*bpr);
	}
	else 
	{
		
#define PIXELSHIGH 1024
#define PIXELSWIDE 1024
		
		data = (unsigned char *)calloc(PIXELSWIDE*PIXELSHIGH*4, 1);
		{	
			float xfraction, yfraction, red, green, blue, alpha;
			int row, col;
			
			unsigned char *p, *rp;//, *imageBody = (unsigned char *)calloc(PIXELSWIDE*PIXELSHIGH*4, 1);
			for (row = 0, rp = data; row < PIXELSHIGH; row++, rp += PIXELSWIDE * 4)
			{
				yfraction = (float)row / (float)(PIXELSHIGH - 1);
				for (col = 0, p = rp; col < PIXELSWIDE; col++, p += 4)
				{
					xfraction = (float)col / (float)(PIXELSWIDE - 1);
					red = xfraction;
					green = yfraction;
					blue = (1.0 - xfraction) * (1.0 - yfraction);
					// make alpha a circle
					alpha = (PIXELSHIGH/2-10) - (float)PIXELSWIDE * sqrt((xfraction - 0.5) * (xfraction - 0.5) + (yfraction - 0.5) * (yfraction - 0.5));
					if (alpha > 1.0)
						alpha = 1.0;
					else if (alpha < 0.0)
						alpha = 0.0;
					red *= alpha;
					if (red < 0.0)
						red = 0.0;
					else if (red > 1.0)
						red = 1.0;
					green *= alpha;
					if (green < 0.0)
						green = 0.0;
					else if (green > 1.0)
						green = 1.0;
					blue *= alpha;
					if (blue < 0.0)
						blue = 0.0;
					else if (blue > 1.0)
						blue = 1.0;
					p[0] = (int)round(alpha * 255.0);
					p[1] = (int)round(red * 255.0);
					p[2] = (int)round(green * 255.0);
					p[3] = (int)round(blue * 255.0);
				}
			}
			
			
		}
		
		
		
		
		
		
		w       = PIXELSWIDE;
		h       = PIXELSHIGH;
		bpr     = 4*w;
		
		check   = true;
		colorspace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
		renderintent = kCGRenderingIntentDefault;
		imagealpha = kCGImageAlphaPremultipliedFirst;
	}


	err = clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_CPU, 1, &device_id, NULL);
	if(err)
	{
		printf("clGetComputeDevices() failed.\n");
		return EXIT_FAILURE;
	}
	
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
	if(!context)
	{
		printf("clCreateContext() failed.\n");
		return EXIT_FAILURE;
	}
	
	queue = clCreateCommandQueue(context, device_id, 0, &err);
	if (!queue)
	{
		printf("clCreateCommandQueue() failed: %d\n", err);
	}
	
	
	program = createProgramFromFile(filterFileName, context);	
	if(!program)
	{
		return EXIT_FAILURE;
	}
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if(err != CL_SUCCESS)
	{
		char buildLog[ 1024 * 16 ];
		buildLog[0] = 0;
		
		clGetProgramBuildInfo( program, device_id, CL_PROGRAM_BUILD_LOG, sizeof( buildLog ), buildLog, NULL );
		
		printf("%s\n", buildLog);
		
		return EXIT_FAILURE;
	}
	
	kernel = clCreateKernel(program, kernelName, &err);
	if(!kernel)
	{
		printf("clCreateKernel() failed creating a kernel. %d\n", err);
		return EXIT_FAILURE;
	}
	
	format.image_channel_order = CL_ARGB;
	format.image_channel_data_type  = CL_UNORM_INT8;
	src = clCreateImage2D(context, CL_MEM_READ_ONLY, &format, w, h, 0, NULL, &err);
	if(!src)
	{
		printf("src:clCreateImage2D() failed.\n");
		return EXIT_FAILURE;
	}
	
	size_t origin[3] = {0,0,0};
	size_t region[3] = {w,h,1};
	err = clEnqueueWriteImage(queue, src, CL_TRUE, origin, region, bpr, 0, data, 0, NULL, NULL);
	if(err != CL_SUCCESS)
	{
		printf("clWriteImage() failed (%d).\n", err);
		return EXIT_FAILURE;
	}
	
	format.image_channel_order = CL_ARGB;
	format.image_channel_data_type  = CL_UNORM_INT8;
	dst = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &format, w, h, 0, NULL, &err);
	if(!dst)
	{
		printf("dst:clCreateImage2D() failed.\n");
		return EXIT_FAILURE;
	}
	
	sampler = clCreateSampler(context, CL_FALSE, CL_ADDRESS_CLAMP, CL_FILTER_NEAREST, NULL);
	if (!sampler) 
	{
		printf("clCreateSampler() failed.\n");
		return EXIT_FAILURE;
	}  
	
	
	
	
	
	
	
	
	
	
	
	
	cl_int is_flipped = 1;
	__cl_int4 dim = { w, h, 0, 0 };
	__cl_float2 st_origin = { 0, 0 };
	__cl_float4 st_delta = { 1, 0, 0, 1 };
	__cl_float4 locals[5] = {
		{ 0.645161, 0.0, 0.0, 0.0 },
		{ 1.0, -0.0, 0.0, 0.0 },
		{ 0.0, -1.0, 0.0, 1.0 },
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0, -1.0, 0.0, h }
	};
	
	
	
	
	
	
	err =  clSetKernelArg(kernel, 0, sizeof(cl_mem), &dst);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_int), &is_flipped);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_int4), &dim);
	err |= clSetKernelArg(kernel, 3, sizeof(cl_float2), &st_origin);
	err |= clSetKernelArg(kernel, 4, sizeof(cl_float4), &st_delta);
	
	err |= clSetKernelArg(kernel, 5, sizeof(cl_float4), &(locals[0]));
	err |= clSetKernelArg(kernel, 6, sizeof(cl_float4), &(locals[1]));
	err |= clSetKernelArg(kernel, 7, sizeof(cl_float4), &(locals[2]));
	err |= clSetKernelArg(kernel, 8, sizeof(cl_float4), &(locals[3]));
	err |= clSetKernelArg(kernel, 9, sizeof(cl_float4), &(locals[4]));
	
	err |= clSetKernelArg(kernel, 10, sizeof(cl_mem), &src);
	err |= clSetKernelArg(kernel, 11, sizeof(cl_sampler), &sampler);
	if(err) {
		printf("setkernelargs failed\n");
		return EXIT_FAILURE;
	}
	
	
	
	
	
	err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(groupsize), &groupsize, NULL);
	if(err != CL_SUCCESS)
	{
		printf("clGetKernelWorkGroupInfo() failed (%d).\n", err);
		return EXIT_FAILURE;
	}
	printf("Kernel WorkGroup size is: %d\n", (int)groupsize);
 	printf("-----------------\n\n");	
	
	
	
 
	local[0] = groupsize;
	local[1] = 1;
	int dimentions;
	if (!trans)
	{
		global[0] = w / subdiv;
		global[1] = h;
		dimentions = 2;
	}
	else
	{
		global[0] = h;
		dimentions = 1;
	}
	
	t0 = t1 = mach_absolute_time();
	for(k=0 ; k<iterations ; k++)
	{
		if(k==1)      t1 = mach_absolute_time();
		
		err = clEnqueueNDRangeKernel(queue, kernel, dimentions, NULL, global, NULL, 0, NULL, NULL);
		if(err != CL_SUCCESS)
		{
			printf("clEnqueueNDRangeKernel() failed (%d).\n", err);
			return EXIT_FAILURE;
		}
	}

	clFinish(queue);
	t2 = mach_absolute_time();
	
	t0 = t1 = mach_absolute_time();
	for(k=0 ; k<iterations ; k++)
	{
		if(k==1)      t1 = mach_absolute_time();
		
		err = clEnqueueNDRangeKernel(queue, kernel, dimentions, NULL, global, NULL, 0, NULL, NULL);
		if(err != CL_SUCCESS)
		{
			printf("clEnqueueNDRangeKernel() failed (%d).\n", err);
			return EXIT_FAILURE;
		}
	}
	
	clFinish(queue);
	t2 = mach_absolute_time();
	
	mem = (uint8_t*)malloc(h*bpr);
	err = clEnqueueReadImage(queue, dst, CL_TRUE, origin, region, bpr, 0, mem, 0, NULL, NULL);
	if(err != CL_SUCCESS)
	{
		printf("clReadImage() failed.\n");
		return EXIT_FAILURE;
	}
	
  
 
	/* Calculate execution time */
	struct mach_timebase_info info;
	double                    t;
		
	mach_timebase_info(&info);
	
	t = 1e-9*(t2-t1)*info.numer / (info.denom * (iterations-1));
	printf("%.1fms per pass: %.1fM pixels/s\n\n", 1000.0*t, 1e-6*w*h / t);

	

	
	

	/* dump output image */
	if(output)
	{
		CGImageDestinationRef idst;
		CGDataProviderRef     prov;
		CFStringRef           path;
		CFURLRef              url;
		
		path    = CFStringCreateWithCString(NULL, outputFile, kCFStringEncodingUTF8);
		url     = CFURLCreateWithFileSystemPath(NULL, path, kCFURLPOSIXPathStyle, false);
		prov    = CGDataProviderCreateWithData(NULL, mem, h*bpr, providerFree);
		
		img     = CGImageCreate(w,h, 8, 32, bpr, CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB),
								kCGImageAlphaPremultipliedFirst, prov, NULL, true, kCGRenderingIntentDefault);
		
		
		
		idst = CGImageDestinationCreateWithURL(url, CFSTR("public.jpeg"), 1, NULL);
		CGImageDestinationAddImage(idst, img, NULL);
		CGImageDestinationFinalize(idst);
	}
	
	
	
	return EXIT_SUCCESS;
}
