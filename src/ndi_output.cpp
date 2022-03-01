/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2020, Raspberry Pi (Trading) Ltd.
 *
 * file_output.cpp - Write output to file.
 */

#include "ndi_output.hpp"

NdiOutput::NdiOutput(VideoOptions const *options)
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
    this->NDI_video_frame.FourCC = NDIlib_FourCC_type_UYVY;
    this->NDI_video_frame.line_stride_in_bytes = options->width * 2;
}

NdiOutput::~NdiOutput()
{
}

void NdiOutput::outputBuffer(void *mem, size_t size, int64_t timestamp_us, uint32_t flags)
{
    // std::cout << "Getting frame" << std::endl;
    this->NDI_video_frame.p_data = (uint8_t*)mem;
    NDIlib_send_send_video_v2(this->pNDI_send, &this->NDI_video_frame);
    // std::cout << "Sent frame" << std::endl;
}