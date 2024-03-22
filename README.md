## 介绍
一个将opus保存成mkv文件小工具


## Require
ffmpeg相关类库 > 4.3


## example
opus_dumper.h 是工具类
wav_to_ogg.cpp 中示范了如何使用该工具类，可以忽略读wav编码成opus的过程
编译之前要把CMAKELISTS.txt中ffmpeg的相关路径更改成自己的ffmpeg库路径

```
cd build && cmake .. && make
./wav_to_ogg
```
