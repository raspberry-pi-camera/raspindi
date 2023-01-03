/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2020, Raspberry Pi (Trading) Ltd.
 *
 * file_output.cpp - Write output to file.
 */

#include "ndi_output.hpp"

NdiOutput::NdiOutput(VideoOptions const *options, std::string neopixelPath)
	: Output(options)
{
    this->NDI_send_create_desc.p_ndi_name = "Video Feed";
    this->pNDI_send = NDIlib_send_create(&NDI_send_create_desc);
	if (!pNDI_send)
	{
		std::cerr << "Failed to create NDI Send" << std::endl;
        exit(1);
	}
    // std::cout << "Width: " << options->width << " x Height: " << options->height << std::endl;
    this->NDI_video_frame.xres = options->width;
    this->NDI_video_frame.yres = options->height;
    this->NDI_video_frame.FourCC = NDIlib_FourCC_type_I420;
    this->NDI_video_frame.line_stride_in_bytes = options->width;

    this->neopixelpath = neopixelPath;
}

NdiOutput::~NdiOutput()
{
}

void NdiOutput::outputBuffer(void *mem, size_t size, int64_t timestamp_us, uint32_t flags)
{
    this->NDI_video_frame.p_data = (uint8_t*)mem;
    NDIlib_send_send_video_v2(this->pNDI_send, &this->NDI_video_frame);
    NDIlib_tally_t NDI_tally;
    NDIlib_send_get_tally(this->pNDI_send, &NDI_tally, 0);
    this->program = NDI_tally.on_program;
    this->preview = NDI_tally.on_preview;

    char pixelStatus;
    std::ofstream neopixel;

    if(NDI_tally.on_program)
    {
        neopixel.open(neopixelpath);
        neopixel << "L";
        neopixel.close();
    }
    else if (NDI_tally.on_preview)
    {
        neopixel.open(neopixelpath);
        neopixel << "L";
        neopixel.close();
    }
    else
    {
        neopixel.open(neopixelpath);
        neopixel << "L";
        neopixel.close();
    }
}


bool NdiOutput::isProgram()
{
    return this->program;
}
bool NdiOutput::isPreview()
{
    return this->preview;
}