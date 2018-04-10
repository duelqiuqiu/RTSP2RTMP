# RTSP2RTMP

一个使用FFMPEG API实现的从RTSP取流并且推送到RTMP的简单例子。实际上也可以用于不同媒体封装格式之间的转化（例如avi->flv）。有任何问题欢迎在Issues里提问。

A simple example shows how to use ffmpeg api pull RTSP stream and pubish RTMP stream. Actually, it can be used for convert format without coder (just like ` -vcodec copy -acodec copy` command). If you have any questions please open new issues.


# How to build
`mkdir build && cd build && cmake .. && make`

# Reference
[最简单的基于FFmpeg的推流器（以推送RTMP为例）][1]

[FFMPEG Examples][2]

[1]: https://blog.csdn.net/leixiaohua1020/article/details/39803457 
[2]: https://www.ffmpeg.org/doxygen/3.4/examples.html
