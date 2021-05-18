#include <csignal>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <string>

#include <iostream>
#include <fstream>

#include <unistd.h>
#include <interface/mmal/mmal.h>
#include <interface/mmal/util/mmal_util.h>
#include <interface/mmal/util/mmal_util_params.h>
#include <interface/mmal/util/mmal_default_components.h>

#include <Processing.NDI.Embedded.h>

#include <libconfig.h++>

#define VERSION "2.0.0"


#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2
#define VIDEO_FRAME_RATE_DEN 1
#define VIDEO_OUTPUT_BUFFERS_NUM 3

void video_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
MMAL_ES_FORMAT_T *format=nullptr;

MMAL_POOL_T *video_pool;

static std::atomic<bool> exit_loop(false);


NDIlib_send_create_t NDI_send_create_desc;
NDIlib_send_instance_t pNDI_send;
NDIlib_video_frame_v2_t NDI_video_frame;

unsigned char *raw;

int width = 1920;
int height = 1080;
int framerate = 45;

static void sigint_handler(int)
{
	exit_loop = true;
}

int main(int argc, char* argv[])
{
	for (int i=0; i < argc; i++) {
		if (!strcmp("-v", argv[i])) {
			std::cout << VERSION;
			return 0;
		}
	}
	int loaded = loadConfig();
	if (loaded > 0) {
		return loaded;
	}
    signal(SIGINT, sigint_handler);

    std::string neopixelpath = cfg.lookup("neopixel_path");
	width = std::stoi(cfg.lookup("width"));
	height = std::stoi(cfg.lookup("height"));
	framerate = std::stoi(cfg.lookup("fps"));

    // create_camera_component
    MMAL_COMPONENT_T *camera = 0;
    MMAL_PORT_T  *video_port = NULL;

    MMAL_STATUS_T status;
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);

    if (status != MMAL_SUCCESS)
    {
        std::cerr << "Failed to create camera component" << std::endl;
        exit(1);
    }

    if (!camera->output_num)
    {
        std::cerr << "Camera doesn't have output ports" << std::endl;
        mmal_component_destroy(camera);
        exit(1);
    }

    video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];


    //  set up the camera configuration
    MMAL_PARAMETER_CAMERA_CONFIG_T cam_config;
    cam_config.hdr.id = MMAL_PARAMETER_CAMERA_CONFIG;
    cam_config.hdr.size = sizeof(cam_config);
    cam_config.max_stills_w = VCOS_ALIGN_UP(width, 32);
    cam_config.max_stills_h = VCOS_ALIGN_UP(height, 16);
    cam_config.stills_yuv422 = 0;
    cam_config.one_shot_stills = 0;
    cam_config.max_preview_video_w = VCOS_ALIGN_UP(width, 32);
    cam_config.max_preview_video_h = VCOS_ALIGN_UP(height, 16);
    cam_config.num_preview_video_frames = 3;
    cam_config.stills_capture_circular_buffer_height = 0;
    cam_config.fast_preview_resume = 0;
    cam_config.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC;
    mmal_port_parameter_set(camera->control, &cam_config.hdr);


    // Set the encode format on the video port
    format = video_port->format;
    format->encoding_variant =  MMAL_ENCODING_I420;
    format->encoding = MMAL_ENCODING_I420;
    format->es->video.width = VCOS_ALIGN_UP(width, 32);
    format->es->video.height = VCOS_ALIGN_UP(height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = VCOS_ALIGN_UP(width, 32);
    format->es->video.crop.height = VCOS_ALIGN_UP(height, 16);;
    format->es->video.frame_rate.num =  framerate;
    format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;
    format->es->video.color_space = MMAL_COLOR_SPACE_ITUR_BT709;


    NDI_send_create_desc.p_ndi_name = "Video Feed";
    pNDI_send = NDIlib_send_create(&NDI_send_create_desc);
	if (!pNDI_send)
	{
		std::cerr << "Failed to create NDI Send" << std::endl;
        exit(1);
	}
    NDI_video_frame.xres = format->es->video.width;
    NDI_video_frame.yres = format->es->video.height;
    NDI_video_frame.FourCC = NDIlib_FourCC_type_I420;

    if (format->es->video.width != width || format->es->video.height != height)
    {
        std::cout << "Aspect ratio forced to: " << format->es->video.width << " x " << format->es->video.height << std::endl;
    }


    status = mmal_port_format_commit(video_port);
    if (status)
    {
        std::cerr << "camera video format couldn't be set" << std::endl;
        mmal_component_destroy(camera);
        exit(1)
    }

    status = mmal_port_enable(video_port, video_buffer_callback);
    if (status)
    {
        std::cerr << "camera video callback2 error" << std::endl;
        mmal_component_destroy(camera);
        exit(1)
    }


    unsigned int mmal_stride = mmal_encoding_width_to_stride(MMAL_ENCODING_I420, video_port->format->es->video.width);
    NDI_video_frame.line_stride_in_bytes = mmal_stride;

    video_port->buffer_size = video_port->buffer_size_recommended;
    video_port->buffer_num = video_port->buffer_num_recommended;
    video_pool = mmal_port_pool_create(video_port, video_port->buffer_num, video_port->buffer_size);
    if(!video_pool)
    {
        std::cerr << "Failed to create buffer header pool for video output port" << std::endl;
        exit(1);
    }


    /* Enable component */
    status = mmal_component_enable(camera);

    if (status)
    {
        std::cerr << "camera component couldn't be enabled" << std::endl;
        mmal_component_destroy(camera);
        exit(1);
    }

    // if ( mmal_port_parameter_set_rational ( camera->control, MMAL_PARAMETER_SATURATION, ( MMAL_RATIONAL_T ) {
    //     0, 100
    // } ) != MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set saturation parameter.\n";
    // if ( mmal_port_parameter_set_rational ( camera->control, MMAL_PARAMETER_SHARPNESS, ( MMAL_RATIONAL_T ) {
    //     0, 100
    // } ) != MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set sharpness parameter.\n";

    // if ( mmal_port_parameter_set_rational ( camera->control, MMAL_PARAMETER_CONTRAST, ( MMAL_RATIONAL_T ) {
    //     0, 100
    // } ) != MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set contrast parameter.\n";

    // mmal_port_parameter_set_rational ( camera->control, MMAL_PARAMETER_BRIGHTNESS, ( MMAL_RATIONAL_T ) {
    //     50, 100
    // } );

    // if ( mmal_port_parameter_set_uint32 ( camera->control, MMAL_PARAMETER_ISO, 400 ) != MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set ISO parameter.\n";

    // if ( mmal_port_parameter_set_uint32 ( camera->control, MMAL_PARAMETER_SHUTTER_SPEED, 0 ) !=  MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set shutter parameter.\n";
    

    // MMAL_PARAMETER_EXPOSUREMODE_T exp_mode = {{MMAL_PARAMETER_EXPOSURE_MODE,sizeof ( exp_mode ) }, MMAL_PARAM_EXPOSUREMODE_AUTO };
    // if ( mmal_port_parameter_set ( camera->control, &exp_mode.hdr ) != MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set exposure parameter.\n";

    // if ( mmal_port_parameter_set_int32 ( camera->control, MMAL_PARAMETER_EXPOSURE_COMP , 0 ) !=MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set Exposure Compensation parameter.\n";


    // MMAL_PARAMETER_EXPOSUREMETERINGMODE_T meter_mode = {{MMAL_PARAMETER_EXP_METERING_MODE, sizeof ( meter_mode ) }, MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE };
    // if ( mmal_port_parameter_set ( camera->control, &meter_mode.hdr ) != MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set metering parameter.\n";

    // MMAL_PARAMETER_IMAGEFX_T imgFX = {{MMAL_PARAMETER_IMAGE_EFFECT,sizeof ( imgFX ) }, MMAL_PARAM_IMAGEFX_NONE };
    // if ( mmal_port_parameter_set ( camera->control, &imgFX.hdr ) != MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set image effect parameter.\n";
    

    // mmal_port_parameter_set_int32 ( camera->output[0], MMAL_PARAMETER_ROTATION, 0 );
    // mmal_port_parameter_set_int32 ( camera->output[1], MMAL_PARAMETER_ROTATION, 0 );
    // mmal_port_parameter_set_int32 ( camera->output[2], MMAL_PARAMETER_ROTATION, 0 );


    // MMAL_PARAMETER_MIRROR_T mirror = {{MMAL_PARAMETER_MIRROR, sizeof ( MMAL_PARAMETER_MIRROR_T ) }, MMAL_PARAM_MIRROR_NONE};
    // if ( mmal_port_parameter_set ( camera->output[0], &mirror.hdr ) != MMAL_SUCCESS ||
    //         mmal_port_parameter_set ( camera->output[1], &mirror.hdr ) != MMAL_SUCCESS ||
    //         mmal_port_parameter_set ( camera->output[2], &mirror.hdr ) )
    //     std::cout << __func__ << ": Failed to set horizontal/vertical flip parameter.\n";
    

    // if ( mmal_port_parameter_set_boolean ( camera->control, MMAL_PARAMETER_VIDEO_STABILISATION, false ) != MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set video stabilization parameter.\n";


    // MMAL_PARAMETER_AWBMODE_T awbParam = {{MMAL_PARAMETER_AWB_MODE,sizeof ( awbParam ) }, MMAL_PARAM_AWBMODE_AUTO };
    // if ( mmal_port_parameter_set ( camera->control, &awbParam.hdr ) != MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set AWB parameter.\n";

    // MMAL_PARAMETER_AWB_GAINS_T param = {{MMAL_PARAMETER_CUSTOM_AWB_GAINS,sizeof(param)}, {0,0}, {0,0}};
    // param.r_gain.num = (unsigned int)(65536);
    // param.b_gain.num = (unsigned int)(65536);
    // param.r_gain.den = param.b_gain.den = 65536;
    // if ( mmal_port_parameter_set(camera->control, &param.hdr) != MMAL_SUCCESS )
    //     std::cout << __func__ << ": Failed to set AWBG gains parameter.\n";
    

    // Start Capture
    if (mmal_port_parameter_set_boolean(video_port, MMAL_PARAMETER_CAPTURE, 1) != MMAL_SUCCESS)
    {
        mmal_port_disable(video_port);
        mmal_component_disable(camera);
        mmal_port_pool_destroy(camera->output[MMAL_CAMERA_VIDEO_PORT], video_pool);
        mmal_component_destroy(camera);
        std::cerr << "Failed to start capture" << std::endl;
        return false;
    }

    int num = mmal_queue_length(video_pool->queue);
    for (int i=0; i<num; i++)
    {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get ( video_pool->queue);

        if (!buffer)
        {
            std::cerr << "Unable to get a required buffer" << i << " from pool queue" << std::endl;
        }
        if (mmal_port_send_buffer(video_port, buffer) != MMAL_SUCCESS)
        {
            std::cerr<<"Unable to send a buffer to encoder output port "<< q<<std::endl;
        }
    }
    MMAL_BUFFER_HEADER_T *buffer;
    while (!exit_loop)
    {
        // This is the main loop; nothing needs to be done in the main thread
    }
    mmal_port_disable(video_port);
    mmal_component_disable(camera);
    mmal_port_pool_destroy(camera->output[MMAL_CAMERA_VIDEO_PORT], video_pool);
    mmal_component_destroy(camera);

    NDIlib_send_send_video_v2(pNDI_send, NULL);

	NDIlib_send_destroy(pNDI_send);

	NDIlib_destroy();

}


void video_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    MMAL_BUFFER_HEADER_T *new_buffer;
    if (buffer->length > 0)
    {
        mmal_buffer_header_mem_lock(buffer );
        NDI_video_frame.p_data = (uint8_t*)buffer->data;
        NDIlib_send_send_video_v2(pNDI_send, &NDI_video_frame);
    
        mmal_buffer_header_mem_unlock(buffer );

        mmal_buffer_header_release(buffer );
    }

    if (port->is_enabled)
    {
        MMAL_STATUS_T status;

        new_buffer = mmal_queue_get(video_pool->queue);

        if (new_buffer)
        {
            status = mmal_port_send_buffer(port, new_buffer);
        }
        if (!new_buffer || status != MMAL_SUCCESS)
        {
            std::cerr << "Unable to return a buffer to the encoder port" << std::endl;
        }
    }
}