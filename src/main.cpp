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

#define VERSION "2.0.1"


#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2
#define VIDEO_FRAME_RATE_DEN 1
#define VIDEO_OUTPUT_BUFFERS_NUM 3

void video_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

static std::atomic<bool> exit_loop(false);

MMAL_ES_FORMAT_T *format=nullptr;
MMAL_POOL_T *video_pool;
MMAL_BUFFER_HEADER_T *buffer;
MMAL_COMPONENT_T *camera = 0;
MMAL_PORT_T  *video_port = NULL;
MMAL_STATUS_T status;

std::ofstream neopixel;

int width = 1920;
int height = 1080;
int framerate = 25;

NDIlib_tally_t NDI_tally;
NDIlib_send_create_t NDI_send_create_desc;
NDIlib_send_instance_t pNDI_send;
NDIlib_video_frame_v2_t NDI_video_frame;

libconfig::Config cfg;


static void sigint_handler(int)
{
	exit_loop = true;
}

int loadConfig()
{
	try
	{
		cfg.readFile("/etc/raspindi.conf");
	}
	catch(const libconfig::FileIOException &fioex)
	{
		std::cerr << "Could not open config file /etc/raspindi.conf"
			<< std::endl;
		return(EXIT_FAILURE);
	}
	catch(const libconfig::ParseException &pex)
	{
		std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
              << " - " << pex.getError() << std::endl;
    	return(EXIT_FAILURE);
	}
	return 0;
}

MMAL_PARAM_AWBMODE_T getAwbMode()
{
    try
    {
        std::string value = cfg.lookup("awb");
        if (value == "sunlight")
        {
            return MMAL_PARAM_AWBMODE_SUNLIGHT;
        }
        if (value == "cloudy")
        {
            return MMAL_PARAM_AWBMODE_CLOUDY;
        }
        if (value == "shade")
        {
            return MMAL_PARAM_AWBMODE_SHADE;
        }
        if (value == "tungsten")
        {
            return MMAL_PARAM_AWBMODE_TUNGSTEN;
        }
        if (value == "fluorescent")
        {
            return MMAL_PARAM_AWBMODE_FLUORESCENT;
        }
        if (value == "incandescent")
        {
            return MMAL_PARAM_AWBMODE_INCANDESCENT;
        }
        if (value == "flash")
        {
            return MMAL_PARAM_AWBMODE_FLASH;
        }
        if (value == "horizon")
        {
            return MMAL_PARAM_AWBMODE_HORIZON;
        }
        if (value == "max")
        {
            return MMAL_PARAM_AWBMODE_MAX;
        }
        if (value == "off")
        {
            return MMAL_PARAM_AWBMODE_OFF;
        }
        return MMAL_PARAM_AWBMODE_AUTO;
    } catch (libconfig::SettingNotFoundException)
    {
        return MMAL_PARAM_AWBMODE_AUTO;
    }
}
MMAL_PARAM_EXPOSUREMODE_T getExposureMode()
{
    // Options: auto, night, nightpreview, backlight, spotlight, sports, snow, beach, verylong, fixedfps, antishake, fireworks, max, off
    try
    {
        std::string value = cfg.lookup("exposuremode");
        if (value == "night")
        {
            return MMAL_PARAM_EXPOSUREMODE_NIGHT;
        }
        if (value == "nightpreview")
        {
            return MMAL_PARAM_EXPOSUREMODE_NIGHTPREVIEW;
        }
        if (value == "backlight")
        {
            return MMAL_PARAM_EXPOSUREMODE_BACKLIGHT;
        }
        if (value == "spotlight")
        {
            return MMAL_PARAM_EXPOSUREMODE_SPOTLIGHT;
        }
        if (value == "sports")
        {
            return MMAL_PARAM_EXPOSUREMODE_SPORTS;
        }
        if (value == "snow")
        {
            return MMAL_PARAM_EXPOSUREMODE_SNOW;
        }
        if (value == "beach")
        {
            return MMAL_PARAM_EXPOSUREMODE_BEACH;
        }
        if (value == "verylong")
        {
            return MMAL_PARAM_EXPOSUREMODE_VERYLONG;
        }
        if (value == "fixedfps")
        {
            return MMAL_PARAM_EXPOSUREMODE_FIXEDFPS;
        }
        if (value == "antishake")
        {
            return MMAL_PARAM_EXPOSUREMODE_ANTISHAKE;
        }
        if (value == "fireworks")
        {
            return MMAL_PARAM_EXPOSUREMODE_FIREWORKS;
        }
        if (value == "max")
        {
            return MMAL_PARAM_EXPOSUREMODE_MAX;
        }
        if (value == "off")
        {
            return MMAL_PARAM_EXPOSUREMODE_OFF;
        }
        return MMAL_PARAM_EXPOSUREMODE_AUTO;
    } catch (libconfig::SettingNotFoundException)
    {
        return MMAL_PARAM_EXPOSUREMODE_AUTO;
    }
}
MMAL_PARAM_EXPOSUREMETERINGMODE_T getMeteringMode()
{
    // Options: average, spot, backlit, matrix, max
    try
    {
        std::string value = cfg.lookup("meteringmode");
        if (value == "spot")
        {
            return MMAL_PARAM_EXPOSUREMETERINGMODE_SPOT;
        }
        if (value == "backlit")
        {
            return MMAL_PARAM_EXPOSUREMETERINGMODE_BACKLIT;
        }
        if (value == "matrix")
        {
            return MMAL_PARAM_EXPOSUREMETERINGMODE_MATRIX;
        }
        if (value == "max")
        {
            return MMAL_PARAM_EXPOSUREMETERINGMODE_MAX;
        }
        return MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
    } catch (libconfig::SettingNotFoundException)
    {
        return MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
    }
}
MMAL_PARAM_MIRROR_T getMirror()
{
    // Options: none, horizontal, vertical, both
    try
    {
        std::string value = cfg.lookup("mirror");
        if (value == "horizontal")
        {
            return MMAL_PARAM_MIRROR_HORIZONTAL;
        }
        if (value == "vertical")
        {
            return MMAL_PARAM_MIRROR_VERTICAL;
        }
        if (value == "both")
        {
            return MMAL_PARAM_MIRROR_BOTH;
        }
        return MMAL_PARAM_MIRROR_NONE;
    } catch (libconfig::SettingNotFoundException)
    {
        return MMAL_PARAM_MIRROR_NONE;
    }
}
MMAL_PARAM_FLICKERAVOID_T getFlickerAvoidMode()
{
    // Options: off, auto, 50hz, 60hz, max
    try
    {
        std::string value = cfg.lookup("flickeravoid");
        if (value == "auto")
        {
            return MMAL_PARAM_FLICKERAVOID_AUTO;
        }
        if (value == "50hz")
        {
            return MMAL_PARAM_FLICKERAVOID_50HZ;
        }
        if (value == "60hz")
        {
            return MMAL_PARAM_FLICKERAVOID_60HZ;
        }
        if (value == "max")
        {
            return MMAL_PARAM_FLICKERAVOID_MAX;
        }
        if (value == "off")
        {
            return MMAL_PARAM_FLICKERAVOID_OFF;
        }
        return MMAL_PARAM_FLICKERAVOID_OFF;
    } catch (libconfig::SettingNotFoundException)
    {
        return MMAL_PARAM_FLICKERAVOID_OFF;
    }



}
int _getIntVal(std::string parameter, int defaultValue)
{
    try
    {
        int value = cfg.lookup(parameter);
        if (value > 100)
        {
            std::cerr << "Invalid value for " << parameter << ": " << value << std::endl;
            return 100;
        }
        if (value < 0)
        {
            std::cerr << "Invalid value for " << parameter << ": " << value << std::endl;
            return 0;
        }
        return value;
    } catch (libconfig::SettingNotFoundException)
    {
        return defaultValue;
    }
}
int getSaturation()
{
    // Values between 0 - 100; default 0
    return _getIntVal("saturation", 0);
}
int getSharpness()
{
    // Values between 0 - 100; default 0
    return _getIntVal("sharpness", 0);
}
int getContrast()
{
    // Values between 0 - 100; default 0
    return _getIntVal("contrast", 0);
}
int getBrightness()
{
    // Values between 0 - 100; default 0
    return _getIntVal("brightness", 50);
}
int getRotation()
{
    // Options: 0, 90, 180, 270
    try
    {
        int value = cfg.lookup("rotation");
        switch(value)
        {
            case 90:
                return 90;
            case 180:
                return 180;
            case 270:
                return 270;
            case 0:
            default:
                return 0;
        }
    } catch (libconfig::SettingNotFoundException)
    {
        return 0;
    }
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
        exit(1);
    }

    status = mmal_port_enable(video_port, video_buffer_callback);
    if (status)
    {
        std::cerr << "camera video callback2 error" << std::endl;
        mmal_component_destroy(camera);
        exit(1);
    }


    NDI_video_frame.line_stride_in_bytes = mmal_encoding_width_to_stride(MMAL_ENCODING_I420, video_port->format->es->video.width);;

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

    MMAL_PARAMETER_AWBMODE_T awbParam = {{MMAL_PARAMETER_AWB_MODE,sizeof(MMAL_PARAMETER_AWBMODE_T)}, getAwbMode()};
    if(mmal_port_parameter_set(camera->control, &awbParam.hdr) != MMAL_SUCCESS)
    {
        std::cout << "Failed to set awb parameter." << std::endl;
    }
    if(mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_SATURATION, (MMAL_RATIONAL_T){getSaturation(), 100}) != MMAL_SUCCESS)
    {
        std::cout << "Failed to set saturation parameter." << std::endl;
    } 
    if(mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_SHARPNESS, (MMAL_RATIONAL_T) {getSharpness(), 100}) != MMAL_SUCCESS)
    {
        std::cout << "Failed to set sharpness parameter." << std::endl;
    }
    if(mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_CONTRAST, (MMAL_RATIONAL_T) {getContrast(), 100}) != MMAL_SUCCESS)
    {
        std::cout << "Failed to set contrast parameter." << std::endl;
    }
    if(mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_BRIGHTNESS, (MMAL_RATIONAL_T) {getBrightness(), 100}) != MMAL_SUCCESS)
    {
        std::cout << "Failed to set brightness parameter." << std::endl;
    }
    MMAL_PARAMETER_EXPOSUREMODE_T exp_mode = {{MMAL_PARAMETER_EXPOSURE_MODE, sizeof(MMAL_PARAMETER_EXPOSUREMODE_T)}, getExposureMode()};
    if(mmal_port_parameter_set(camera->control, &exp_mode.hdr) != MMAL_SUCCESS)
    {
        std::cout << "Failed to set exposure parameter." << std::endl;
    }

    MMAL_PARAMETER_EXPOSUREMETERINGMODE_T meter_mode = {{MMAL_PARAMETER_EXP_METERING_MODE, sizeof(MMAL_PARAMETER_EXPOSUREMETERINGMODE_T)}, getMeteringMode()};
    if(mmal_port_parameter_set(camera->control, &meter_mode.hdr) != MMAL_SUCCESS)
    {
        std::cout << "Failed to set metering parameter." << std::endl;
    }

    int rotation = getRotation();
    mmal_port_parameter_set_int32(camera->output[0], MMAL_PARAMETER_ROTATION, rotation);
    mmal_port_parameter_set_int32(camera->output[1], MMAL_PARAMETER_ROTATION, rotation);
    mmal_port_parameter_set_int32(camera->output[2], MMAL_PARAMETER_ROTATION, rotation);

    MMAL_PARAMETER_MIRROR_T mirror = {{MMAL_PARAMETER_MIRROR, sizeof(MMAL_PARAMETER_MIRROR_T)}, getMirror()};

    if (    mmal_port_parameter_set(camera->output[0], &mirror.hdr) != MMAL_SUCCESS ||
            mmal_port_parameter_set(camera->output[1], &mirror.hdr) != MMAL_SUCCESS ||
            mmal_port_parameter_set(camera->output[2], &mirror.hdr))
    {
        std::cout << "Failed to set flip parameter." << std::endl;
    }

   MMAL_PARAMETER_FLICKERAVOID_T flickeravoid = {{MMAL_PARAMETER_FLICKER_AVOID, sizeof(MMAL_PARAMETER_FLICKERAVOID_T)}, getFlickerAvoidMode()};
   if (     mmal_port_parameter_set(camera->control, &flickeravoid.hdr) != MMAL_SUCCESS)
    {
        std::cout << "Failed to set flicker avoid parameter." << std::endl;
    }
    

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
    for (int i=0; i < num; i++)
    {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(video_pool->queue);

        if (!buffer)
        {
            std::cerr << "Unable to get a required buffer" << i << " from pool queue" << std::endl;
        }
        if (mmal_port_send_buffer(video_port, buffer) != MMAL_SUCCESS)
        {
            std::cerr<<"Unable to send a buffer to encoder output port "<< i <<std::endl;
        }
    }
    while (!exit_loop)
    {
		// Get Tally information
		NDIlib_send_get_tally(pNDI_send, &NDI_tally, 0);
		if (NDI_tally.on_program)
		{
			neopixel.open(neopixelpath);
			neopixel << "L";
			neopixel.close();
		} else if (NDI_tally.on_preview)
		{
			neopixel.open(neopixelpath);
			neopixel << "P";
			neopixel.close();
		} else
		{
			neopixel.open(neopixelpath);
			neopixel << "N";
			neopixel.close();
		}
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