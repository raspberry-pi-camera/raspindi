#include <csignal>
#include <cstddef>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <atomic>

#include <iostream>
#include <fstream>

#include <unistd.h>
#include <raspicam/raspicam.h>

#include <Processing.NDI.Lib.h>

#include <libconfig.h++>

#define VERSION "1.1.0"

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
			std::cout << VERSION;
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

	raspicam::RaspiCam Camera;

	int width, height, fps;
	width = stoi(cfg.lookup("width");
	height = stoi(cfg.lookup("height");
	fps = stoi(cfg.lookup("fps");
		   
	Camera.setWidth(width);
	Camera.setHeight(height);
	Camera.setFrameRate(fps);
	Camera.setFormat(raspicam::RASPICAM_FORMAT_RGB);

	int rawSize = Camera.getImageBufferSize();
	int dataSize = rawSize * 1.3;
	int pixels = width * height;
	int zero = 0;

	bool idx = false;
	unsigned char *raw[2];
	raw[0] = new unsigned char[rawSize];
	raw[1] = new unsigned char[rawSize];
	unsigned char *data[2];
	data[0] = new unsigned char[dataSize];
	data[1] = new unsigned char[dataSize];

	if (!Camera.open())
	{
		printf("Error opening camera.");
		return 2;
	}

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
		Camera.retrieve(raw[(int)idx]); //get camera image

		// Convert native colour (BGR) to BGRA
		for (int i=0; i<pixels;i++) {
			memcpy(data[(int)idx] + i*4, raw[(int)idx] + i*3, 3);
			data[(int)idx][i*4 + 3] = 0;
		}

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
		NDI_video_frame.p_data = (uint8_t*)data[(int)idx];
		NDIlib_send_send_video_v2(pNDI_send, &NDI_video_frame);

		idx = !idx;
	}

	NDIlib_send_send_video_v2(pNDI_send, NULL);

	NDIlib_send_destroy(pNDI_send);

	NDIlib_destroy();

	return 0;
}

