#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
using namespace std;


#include "opus_dumper.h"



extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
}


int main(){

    AVFormatContext *pInputFormatCtx = NULL;
    //AVFormatContext *pOutputFormatCtx = NULL;
    
    pInputFormatCtx = avformat_alloc_context();
    //pOutputFormatCtx = avformat_alloc_context();


    if(avformat_open_input(&pInputFormatCtx, "../pure.wav", NULL, NULL)){
        printf("Could not open input file\n");
        return -1;
    }

    if(avformat_find_stream_info(pInputFormatCtx, NULL)<0){
        printf("Could not find stream information\n");
        return -1;
    }


    AVStream* audioStream = NULL;
    for(int i = 0; i < pInputFormatCtx->nb_streams; i++){
        AVStream *pStream = pInputFormatCtx->streams[i];
        printf("stream idx %d, duration %d, nb_frames %d\n", 
                i, pStream->duration, pStream->nb_frames);
        if(pStream->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audioStream = pStream;
        }
    }


    av_dump_format(pInputFormatCtx, 0, "pure.wav", 0);
    AVPacket *pPacket = av_packet_alloc();
    

    AVCodecContext *av_codec_ctx = avcodec_alloc_context3(NULL);
    const AVCodec *av_codec = avcodec_find_encoder(AV_CODEC_ID_OPUS);
    if(av_codec == NULL){
        printf("could not find opus encoder\n");
        return -1;
    }

    const int bitrate = 128000;
    const int sampleRate = 48000;
    const int channelNum = 2;
    const int foramt = AV_SAMPLE_FMT_S16;
    const int frameMs = 20;
    const int frameNbSamples = audioStream->codecpar->sample_rate * frameMs / 1000;
    const int frame_sample_size = frameNbSamples * audioStream->codecpar->ch_layout.nb_channels *
                                    av_get_bytes_per_sample((AVSampleFormat)(audioStream->codecpar->format));


    av_codec_ctx->bit_rate = bitrate;
    av_codec_ctx->sample_rate = sampleRate;
    av_codec_ctx->channels = channelNum;
    av_codec_ctx->frame_size = pInputFormatCtx->streams[0]->codecpar->frame_size;
    av_codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
    av_codec_ctx->codec_type = AVMEDIA_TYPE_AUDIO;
    av_codec_ctx->codec_id = AV_CODEC_ID_OPUS;
    av_codec_ctx->time_base.num = 1;
    av_codec_ctx->time_base.den = 48000;
    av_codec_ctx->flags = AV_CODEC_FLAG_GLOBAL_HEADER;
    av_codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16;

    if(avcodec_open2(av_codec_ctx, av_codec, NULL)<0){
        printf("could not open opus encoder\n");
        return -1;
    }


    OpusDumper opus_dumper;
    opus_dumper.init("output.mkv", channelNum, sampleRate, bitrate, foramt, frameMs);


    uint8_t buffer[10240];
    int buffer_offset = 0;
    while(!av_read_frame(pInputFormatCtx, pPacket)){
        memcpy(buffer + buffer_offset, pPacket->data, pPacket->size);
        buffer_offset += pPacket->size;
        
        while(buffer_offset >= frame_sample_size){
            static int pts = 0, duration = 20;
            AVFrame* pFrame = av_frame_alloc();
            pFrame->format = audioStream->codecpar->format;
            pFrame->channels = audioStream->codecpar->channels;
            pFrame->nb_samples = frameNbSamples; 
            pFrame->channel_layout = audioStream->codecpar->channel_layout;
            pFrame->sample_rate = audioStream->codecpar->sample_rate;
            pFrame->pts = pPacket->pts;
            pFrame->time_base = audioStream->time_base;
            pts += frameNbSamples;
            pFrame->pts = pts;
            pFrame->data[0] = &buffer[0];
            pFrame->linesize[0] = frame_sample_size;
            int ret = avcodec_send_frame(av_codec_ctx, pFrame);
            if(ret < 0){
                printf("could not send frame to opus encoder samples %d, format %d  lineSize %d, errno %d\n",
                    pFrame->nb_samples, pFrame->format, pFrame->linesize[0], ret);
            }

            memmove(buffer, buffer + frame_sample_size, buffer_offset - frame_sample_size);
            buffer_offset -= frame_sample_size;
            printf("buffer_offset %d frame_sample_size %d frame_sample_size %d\n", 
                    buffer_offset, frame_sample_size, frame_sample_size);


            AVPacket *encodedPacket = av_packet_alloc();
            while(avcodec_receive_packet(av_codec_ctx, encodedPacket) == 0){
                printf("decoded frame %d, packetData Size %d\n", encodedPacket->pts, encodedPacket->size);
                opus_dumper.save_opus(encodedPacket->data, encodedPacket->size);
                av_packet_unref(encodedPacket);
            }
            av_frame_free(&pFrame);
            av_packet_free(&encodedPacket);
        }

        printf("read packet pts %d, packet size %d\n", pPacket->pts, pPacket->size);
        av_packet_unref(pPacket);
    }
    opus_dumper.write_trailer();
    av_packet_free(&pPacket);
    avformat_close_input(&pInputFormatCtx);
    return 0;
}