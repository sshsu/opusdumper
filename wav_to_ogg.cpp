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


    for(int i = 0; i < pInputFormatCtx->nb_streams; i++){
        AVStream *pStream = pInputFormatCtx->streams[i];
        printf("stream idx %d, duration %d, nb_frames %d\n", 
                i, pStream->duration, pStream->nb_frames);
    }

    av_dump_format(pInputFormatCtx, 0, "pure.wav", 0);
    AVPacket *pPacket = av_packet_alloc();
    

    //OpusDumper opus_dumper;
    //opus_dumper.init("output.ogg");

    int i =1;

    AVCodecContext *av_codec_ctx = avcodec_alloc_context3(NULL);
    const AVCodec *av_codec = avcodec_find_encoder(AV_CODEC_ID_OPUS);
    if(av_codec == NULL){
        printf("could not find opus encoder\n");
        return -1;
    }

    av_codec_ctx->bit_rate = 64000;
    av_codec_ctx->sample_rate = 48000;
    av_codec_ctx->channels = 2;
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

    printf("xxxxxx\n");

    OpusDumper opus_dumper;
    opus_dumper.init("output.mkv");
    opus_dumper.set_extradata(av_codec_ctx->extradata, av_codec_ctx->extradata_size);

    while(!av_read_frame(pInputFormatCtx, pPacket)){

        AVFrame* pFrame = av_frame_alloc();
        pFrame->format = pInputFormatCtx->streams[0]->codecpar->format;
        pFrame->channels = pInputFormatCtx->streams[0]->codecpar->channels;
        pFrame->nb_samples = 960; //pPacket->size / av_get_bytes_per_sample((enum AVSampleFormat)(pFrame->format)) / pFrame->channels;
        pFrame->channel_layout = pInputFormatCtx->streams[0]->codecpar->channel_layout;
        pFrame->sample_rate = pInputFormatCtx->streams[0]->codecpar->sample_rate;
        pFrame->pts = pPacket->pts;
        pFrame->data[0] = pPacket->data;
        pFrame->linesize[0] = pPacket->size;

        
        int ret = avcodec_send_frame(av_codec_ctx, pFrame);
        if(ret < 0){
            //char errbuf[1024];
            //av_strerror(ret, errbuf, sizeof(errbuf));
            printf("could not send frame to opus encoder samples %d, format %d  lineSize %d, errno %d\n",
                  pFrame->nb_samples, pFrame->format, pFrame->linesize[0], ret);
        }


        AVPacket *encodedPacket = av_packet_alloc();
        while(avcodec_receive_packet(av_codec_ctx, encodedPacket) == 0){
            printf("decoded frame %d, packetData Size %d\n", i, encodedPacket->size);
            opus_dumper.save_opus(encodedPacket->data, encodedPacket->size);
        }
        av_packet_unref(encodedPacket);
        av_frame_free(&pFrame);

        printf("read packet pts %d, packet size %d\n", pPacket->pts, pPacket->size);
        av_packet_unref(pPacket);

    }
    opus_dumper.write_trailer();
    av_packet_free(&pPacket);
    avformat_close_input(&pInputFormatCtx);
    return 0;
}