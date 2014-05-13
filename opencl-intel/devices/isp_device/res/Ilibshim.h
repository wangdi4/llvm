/*
 * Header file for other applications to link to libshim.
 * Does not contain private members.
 */

#ifndef __ILIBSHIM_H__
#define __ILIBSHIM_H__

namespace Intel { namespace OpenCL { namespace ISPDevice {

    // For improved readability
    typedef void* host_ptr;     //!< Any normal pointer. Only used here to indicate a pointer valid in the host program
    typedef void* isp_ptr;      //!< A mapped host pointer gives a isp pointer, that is valid on ISP. ISP uses different virtual address space

    // camera_id. As per jb/system/core/include/system/camera.h
    enum {
        /** The facing of the camera is opposite to that of the screen. */
        CAMERA_FACING_BACK = 0,
        /** The facing of the camera is the same as that of the screen. */
        CAMERA_FACING_FRONT = 1
    };

    // A preview frame
    struct Frame {
        void* img_data;         //!< Raw image data
        int id;                 //!< Id for debugging data flow path
        int frameCounter;       //!< Frame counter. Reset upon preview_start
        int width;              //!< Pixel width of image
        int height;             //!< Pixel height of image
        int format;             //!< TODO not valid at preset
        int stride;             //!< Stride of the buffer
        int size;               //!< Size of img_data in bytes. For NV12 size==1.5*stride*height
    };
    typedef Frame frame;    // Camhal API legacy

    struct fw_info {
        size_t size;
        host_ptr data;
        unsigned int fw_handle;
    };

    typedef void (*image_callback)(host_ptr p);

    class ITaskDispatcher
    {
        public:
            virtual ~ITaskDispatcher() {};
            virtual void PipelineCallback(Frame* f) = 0;
    };

    typedef int status_t;
    class CameraShim
    {
        public:
            // Camhal API unchanged
            static CameraShim* instance(const char* raw_name, int cameraId);
            status_t preview_start();
            bool preview_available();
            status_t preview_grabbuffer(Frame &f);
            status_t preview_releasebuffer(Frame &f);
            status_t preview_stop();
            status_t acc_read_fw(const char* filename, fw_info &fw);
            status_t acc_upload_fw_standalone(fw_info &fw);
            status_t acc_upload_fw_extension(fw_info &fw);
            status_t acc_start_standalone(const fw_info &fw);
            status_t acc_wait_standalone(const fw_info &fw);
            status_t acc_unload(fw_info &fw);
            host_ptr host_alloc(int size);

            // Camhal API legacy
            status_t acc_map(host_ptr in, size_t bytes, isp_ptr &out);
            status_t acc_sendarg(const fw_info &fw, isp_ptr arg, size_t arg_bytes);
            status_t acc_unmap(isp_ptr p, size_t bytes);

            // New API
            status_t set_preview_size(int preview_width, int preview_height);
            status_t get_preview_size(int* preview_width, int* preview_height);
            status_t set_preview_buffer_size(int size);
            status_t set_picture_size(int picture_width, int picture_height);
            status_t get_picture_size(int* picture_width, int* picture_height);
            status_t focus(bool blocking);
            status_t enable_shutter_sound(bool enable);
            status_t take_picture(bool run_focus = true);
            status_t take_picture(host_ptr ret, size_t size, bool run_focus = true);
            bool is_preview_running();
            status_t acc_start_and_wait_standalone(const fw_info &fw);
            status_t acc_abort_standalone(const fw_info &fw);
            status_t host_free(host_ptr ptr);
            status_t acc_map(host_ptr in, isp_ptr &out);
            status_t acc_sendarg(const fw_info &fw, isp_ptr arg);
            status_t acc_unmap(isp_ptr p);
            status_t register_callback(ITaskDispatcher* cb);
            status_t register_callback_image(image_callback cb);
            static bool dumpImage2File(const void* data, const unsigned int width_padded, unsigned int width,
                                       unsigned int height, const char* name);

            // Debug
            void shimLOGI(const char *string);
            void shimLOGD(const char *string);
            void shimLOGW(const char *string);
            void shimLOGE(const char *string);
    };
    typedef CameraShim camhal;  // Camhal API legacy
}}}

#endif // __ILIBSHIM_H__
