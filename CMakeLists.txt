cmake_minimum_required(VERSION 3.0)
project(wav_to_ogg)

#指定c++库 版本
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -stdlib=libc++ -framework OpenGL -framework AppKit -framework Security -framework CoreFoundation -framework CoreVideo -framework CoreMedia -framework QuartzCore -framework CoreFoundation -framework VideoDecodeAcceleration -framework Cocoa -framework AudioToolbox -framework VideoToolbox -framework OpenCL ")


SET(FFMPEG_HEADER_PATH /Users/yishen/git2/ffmpeg-6.1.1/libs/include)
SET(FFMPEG_LIB_PATH /Users/yishen/git2/ffmpeg-6.1.1/libs/lib)

# 添加头文件搜索路径
include_directories(
    ${FFMPEG_HEADER_PATH}
)

# 添加库文件搜索路径
link_directories(
    ${FFMPEG_LIB_PATH}
    /usr/local/lib/
)

# 添加源文件
add_executable(wav_to_ogg 
    wav_to_ogg.cpp
    opus_dumper.cpp
)

# 添加链接的库文件
target_link_libraries(wav_to_ogg
    iconv
    avformat
    avdevice
    x264
    opus    
    avcodec
    avutil
    pthread
    swscale
    swresample
    z
    m
    bz2
    lzma
)