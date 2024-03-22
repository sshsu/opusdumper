#include "opus_dumper.h"


void OpusDumper::init(const char *fileName,
                      const int channelNum,
                      const int sampleRate,
                      const int bitrate,
                      const int format,
                      const int frameMs){

    avformat_alloc_output_context2(&output_format_context, NULL, NULL, fileName);
    avformat_new_stream(output_format_context, NULL);

    AVStream* output_stream = output_format_context->streams[0];
    output_stream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    output_stream->codecpar->codec_id = AV_CODEC_ID_OPUS;
    output_stream->codecpar->channels = channelNum;
    output_stream->codecpar->sample_rate = sampleRate;
    output_stream->codecpar->bit_rate = bitrate;
    output_stream->codecpar->format = format;
    output_stream->time_base = (AVRational){1, 1000};
    if(!(output_format_context->flags & AVFMT_NOFILE)){
        int ret = avio_open(&output_format_context->pb, fileName, AVIO_FLAG_WRITE);
        if(ret < 0){
            printf("Could not open output file %s\n", fileName);
            exit(1);
        }
    }
    avformat_write_header(output_format_context, NULL);
}


void OpusDumper::save_opus(const uint8_t* data, const int data_len){

    AVPacket *packet = av_packet_alloc();
    packet->data = (uint8_t*)data;
    packet->size = data_len;
    packet->stream_index = 0;
    AVRational src_rational; 
    src_rational = (AVRational){1, 1000};
    packet->time_base = (AVRational){1, 1000};
    packet->pts = av_rescale_q(pts, src_rational, packet->time_base);
    packet->dts = av_rescale_q(pts, src_rational, packet->time_base);
    packet->duration = av_rescale_q(duration, src_rational, packet->time_base);
    pts += duration;
    packet->flags = 0;

printf("save opus frame success, pts %d packet->pts %d duration %d\n", 
        pts, packet->pts, packet->duration);


    int ret = av_interleaved_write_frame(output_format_context, packet);
    if(ret < 0){
        printf("Error writing frame\n");
    } else {
        //printf("save opus frame success, pts %d duration %d\n", packet->pts, packet->duration);
    }
    av_packet_free(&packet);
}


void OpusDumper::write_trailer(){
    av_write_trailer(output_format_context);
    avformat_free_context(output_format_context);
}