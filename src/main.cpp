#include <chrono>
#include <poll.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/stat.h>

#include "core/libcamera_encoder.hpp"
#include "ndi_output.hpp"
#include <libconfig.h++>

using namespace std::placeholders;
bool exit_loop = false;
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

int _getValue(std::string parameter, int defaultValue, int min, int max)
{
    try
    {
        int value = cfg.lookup(parameter);
        if (value > max)
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

int _getValue(std::string parameter, int defaultValue)
{
	try
    {
        int value = cfg.lookup(parameter);
        return value;
    } catch (libconfig::SettingNotFoundException)
    {
        return defaultValue;
    }
}

float _getValue(std::string parameter, float defaultValue)
{
	try
    {
        float value = cfg.lookup(parameter);
        return value;
    } catch (libconfig::SettingNotFoundException)
    {
        return defaultValue;
    }
}

std::string _getValue(std::string parameter, std::string defaultValue)
{
	try
    {
        std::string value = cfg.lookup(parameter);
        return value;
    } catch (libconfig::SettingNotFoundException)
    {
        return defaultValue;
    }
}

void mirrored_rotation(VideoOptions *options)
{
	std::string value = _getValue("mirror", "none");
	libcamera::Transform transform = libcamera::Transform::Identity;
	bool hflip = false;
	bool vflip = false;
	if (value == "horizontal")
	{
		hflip = true;
	}
	if (value == "vertical")
	{
		vflip = true;
	}
	if (value == "both")
	{
		hflip = true;
		vflip = true;
	}
	if (hflip)
		transform = libcamera::Transform::HFlip * transform;
	if (vflip)
		transform = libcamera::Transform::VFlip * transform;
	
	bool ok;
	libcamera::Transform rotation = libcamera::transformFromRotation(_getValue("rotation", 0), &ok);
	if (!ok)
		throw std::runtime_error("illegal rotation value");
	transform = rotation * transform;
	options->transform = transform;
}

static void event_loop(LibcameraEncoder &app)
{
	VideoOptions const *options = app.GetOptions();
	std::unique_ptr<Output> output = std::unique_ptr<Output>(new NdiOutput(options, _getValue("neopixel_path", "/tmp/neopixel.state")));
	app.SetEncodeOutputReadyCallback(std::bind(&Output::OutputReady, output.get(), _1, _2, _3, _4));


	app.OpenCamera();
	app.ConfigureVideo(LibcameraEncoder::FLAG_VIDEO_JPEG_COLOURSPACE);
	app.StartEncoder();
	app.StartCamera();
	while (!exit_loop)
	{
		LibcameraEncoder::Msg msg = app.Wait();
		if (msg.type == LibcameraEncoder::MsgType::Quit)
			return;
		else if (msg.type != LibcameraEncoder::MsgType::RequestComplete)
			throw std::runtime_error("unrecognised message!");

		CompletedRequestPtr &completed_request = std::get<CompletedRequestPtr>(msg.payload);
		app.EncodeBuffer(completed_request, app.VideoStream());
	}
}

int main(int argc, char *argv[])
{
	try
	{
		LibcameraEncoder app;
		VideoOptions *options = app.GetOptions();
		loadConfig();
		options->codec = "YUV420";
		options->verbose = false;
		options->nopreview = true;
		options->denoise = "off";
		// Set flip
		options->width = _getValue("width", 1280);
		options->height = _getValue("height", 720);
		options->framerate = _getValue("framerate", 25);
		options->awb = _getValue("awb", "auto");
		options->awb_gain_b = _getValue("b_gain", 0.0f);
		options->awb_gain_r = _getValue("r_gain", 0.0f);
		options->saturation = _getValue("saturation", 1);
		options->sharpness = _getValue("sharpness", 1);
		options->contrast = _getValue("contrast", 1);
		options->brightness = ((_getValue("brightness", 50) / 50) - 1);
		options->exposure = _getValue("exposuremode", "auto");
		options->metering = _getValue("meteringmode", "average");
		mirrored_rotation(options);
		options->Print();
		event_loop(app);
	}
	catch (std::exception const &e)
	{
		std::cerr << "ERROR: *** " << e.what() << " ***" << std::endl;
		return -1;
	}
	return 0;
}