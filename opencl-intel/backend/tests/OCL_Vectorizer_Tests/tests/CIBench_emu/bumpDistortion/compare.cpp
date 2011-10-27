#include <libc.h>
#include <stdbool.h>
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




int main(int argc, char **argv)
{
	uint8_t          *input1, *input2;
	size_t           w,h, bpr;
	CGImageRef       img;
	CGColorSpaceRef colorspace;
	CGColorRenderingIntent renderintent;
	CGImageAlphaInfo imagealpha;

	if (argc < 3)
	{
		printf ("Error! missing image file names!\n");
		return EXIT_FAILURE;
	}


	{
		CGAccessSession   *session;
		CFStringRef        path;
		CFDictionaryRef    dict;
		CGImageSourceRef   isrc;
		CGDataProviderRef  prov;
		CFURLRef           url;
		size_t             bpp;
		
		path    = CFStringCreateWithCString(NULL, argv[1], kCFStringEncodingUTF8);
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
		
		input1    = (uint8_t*)malloc(h*bpr);
		
		fprintf(stdout, "Size:%dx%d  bytesPerPixel:%d\n", (int)w,(int)h, (int)bpp);
		fflush(stdout);
		
		CGAccessSessionGetBytes(session, input1, h*bpr);
	}
	
	
	{
		CGAccessSession   *session;
		CFStringRef        path;
		CFDictionaryRef    dict;
		CGImageSourceRef   isrc;
		CGDataProviderRef  prov;
		CFURLRef           url;
		size_t             bpp;
		
		path    = CFStringCreateWithCString(NULL, argv[2], kCFStringEncodingUTF8);
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
		
		input2    = (uint8_t*)malloc(h*bpr);
		
		fprintf(stdout, "Size:%dx%d  bytesPerPixel:%d\n", (int)w,(int)h, (int)bpp);
		fflush(stdout);
		
		CGAccessSessionGetBytes(session, input2, h*bpr);
	}

	int fails = 0;
	for (unsigned x = 0; x<h*bpr; x++)
	{
		if (input1[x] != input2[x])
		{
			fails++;
		}
	}
	if (fails)
	{
		printf("Compare mistmatches:  %d\n", fails);
		return EXIT_FAILURE;
	}
	
	printf("Compare successful!\n");
	return EXIT_SUCCESS;
}
