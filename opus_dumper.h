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
    void init(const char* fileName);
    void save_opus(const uint8_t* date, const int data_len);
    void write_trailer();
    void set_extradata(const uint8_t* extradata, int extradata_size);
private:
    AVFormatContext *output_format_context = NULL;
    int pts = 0;
    int duration = 20;
};


