#include <csignal>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <atomic>

#include <iostream>
#include <fstream>

#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <raspicam/raspicam_cv.h>

#include <Processing.NDI.Lib.h>

#include <libconfig.h++>

static std::atomic<bool> exit_loop(false);
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

int main(int argc, char* argv[])
{
	for (int i=0; i < argc; i++) {
		if (!strcmp("-v", argv[i])) {
			std::cout << "1.0";
			return 0;
		}
	}
	int loaded = loadConfig();
	if (loaded > 0) {
		return loaded;
	}

	std::string neopixelpath = cfg.lookup("neopixel_path");

	if (!NDIlib_initialize())
	{
		std::cerr << "Cannot run NDI." << std::endl;
		return 1;
	}

	signal(SIGINT, sigint_handler);
	
	NDIlib_send_create_t NDI_send_create_desc;
	NDI_send_create_desc.p_ndi_name = "Video Feed";

	std::ofstream neopixel;

	raspicam::RaspiCam_Cv Camera;

	int width, height;
	width = 1280;
	height = 720;

	bool idx = false;
	
	Camera.set(cv::CAP_PROP_FRAME_WIDTH, width);
	Camera.set(cv::CAP_PROP_FRAME_HEIGHT, height);
	Camera.set(cv::CAP_PROP_FPS, 30);
	Camera.set(cv::CAP_PROP_FORMAT, CV_8UC4);

	if (!Camera.open())
	{
		printf("Error opening camera.");
		return 2;
	}

	cv::Mat images[2];

	NDIlib_send_instance_t pNDI_send = NDIlib_send_create(&NDI_send_create_desc);
	if (!pNDI_send)
	{
		return 3;
	}

	NDIlib_video_frame_v2_t NDI_video_frame;
	NDI_video_frame.xres = width;
	NDI_video_frame.yres = height;
	NDI_video_frame.FourCC = NDIlib_FourCC_type_BGRX;
	NDI_video_frame.line_stride_in_bytes = width * 4;

	while (!exit_loop)
	{
		Camera.grab();
		Camera.retrieve(images[(int)idx]); //get camera image

		// Convert native colour (BGR) to BGRA
		cv::cvtColor(images[(int)idx], images[(int)idx], cv::COLOR_BGR2BGRA);

		// Get Tally information
		NDIlib_tally_t NDI_tally;
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

		// Send the video frame
		NDI_video_frame.p_data = (uint8_t*)images[(int)idx].data;
		NDIlib_send_send_video_async_v2(pNDI_send, &NDI_video_frame);

		idx = !idx;
	}

	NDIlib_send_send_video_async_v2(pNDI_send, NULL);

	NDIlib_send_destroy(pNDI_send);

	NDIlib_destroy();

	return 0;
}

