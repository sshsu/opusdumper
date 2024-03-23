#pragma once

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/avutil.h>
}


class OpusDumper{
public:
    void init(const char *file_name,
              const int channel_num,
              const int sample_rate,
              const int bitrate,
              const int format,
              const int frame_ms);
    void save_opus(const uint8_t* date, const int data_len);
    void write_trailer();
private:
    AVFormatContext *output_format_context = NULL;
    int pts = 0;
    int frame_ms = 20;
};


