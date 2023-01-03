/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (C) 2020, Raspberry Pi (Trading) Ltd.
 *
 * file_output.hpp - Write output to file.
 */

#pragma once

#include <output/output.hpp>
#include <Processing.NDI.Embedded.h>

class NdiOutput : public Output
{
public:
	NdiOutput(VideoOptions const *options);
	~NdiOutput();

    bool isProgram();
    bool isPreview();

protected:
	void outputBuffer(void *mem, size_t size, int64_t timestamp_us, uint32_t flags) override;

private:
    NDIlib_tally_t NDI_tally;
    NDIlib_send_create_t NDI_send_create_desc;
    NDIlib_send_instance_t pNDI_send;
    NDIlib_video_frame_v2_t NDI_video_frame;
    bool preview;
    bool program;
};
