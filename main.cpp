#include <iostream>
#include <assert.h>

extern "C"{
#include <libavutil/time.h>
#include <libavformat/avformat.h>
}

int main(int argc,const char** argv){
	if(argc < 3){
		std::cout << "Usage ./rtsp2rtmp [rtsp:url] [rtmp:url]" << std::endl;
		return -1;
	}

	std::string rtsp_url(argv[1]);
	std::string rtmp_url(argv[2]);

	std::cout << "rtsp url " << rtsp_url << std::endl;
	std::cout << "rtmp url " << rtmp_url << std::endl;

	std::cout << "init" << std::endl;
	av_register_all();
	avformat_network_init();
	av_log_set_level(AV_LOG_DEBUG);

	std::cout << "create AVFormatContext" << std::endl;
	AVFormatContext* format_ctx = avformat_alloc_context();

	std::cout << "open rtsp" << std::endl;
	int ret = avformat_open_input(&format_ctx, rtsp_url.c_str(), NULL, NULL);
	assert(ret == 0);

	ret = avformat_find_stream_info(format_ctx,NULL);
	std::cout << "wait..." << std::endl;
	assert(ret == 0);

	av_dump_format(format_ctx, 0, rtsp_url.c_str(), 0);
	std::cout << "prepare output context" << std::endl;

	AVFormatContext* output_format = NULL;
	avformat_alloc_output_context2(&output_format, NULL, "flv", rtmp_url.c_str());
	//avformat_alloc_output_context2(&output_format, NULL, "h264", rtmp_url.c_str());
	assert(output_format != NULL);

	for (int i = 0; i < format_ctx->nb_streams; i ++) {
		AVStream* out_stream = avformat_new_stream(output_format, NULL);
		assert(out_stream != NULL);

		AVStream* in_stream = format_ctx->streams[i];

		ret = avcodec_parameters_copy(out_stream->codecpar, format_ctx->streams[i]->codecpar);
		assert(ret >= 0);

		out_stream->codecpar->codec_tag = 0;
	}

	av_dump_format(output_format, 0, rtmp_url.c_str(), 1);

	if(!(output_format->oformat->flags & AVFMT_NOFILE)){
		ret = avio_open(&output_format->pb, rtmp_url.c_str(), AVIO_FLAG_WRITE);
		assert(ret >= 0);
	}

	ret = avformat_write_header(output_format, NULL);
	assert(ret >= 0);

	int64_t start_time = av_gettime();
	AVPacket packet;
	while (true) {
		AVStream *in_stream,*out_stream;
		ret = av_read_frame(format_ctx, &packet);
		if(ret < 0)
			break;

		in_stream = format_ctx->streams[packet.stream_index];
		out_stream = output_format->streams[packet.stream_index];

		if(in_stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
			AVRational time_base = in_stream->time_base;
			AVRational time_base_q = AV_TIME_BASE_Q;
			int64_t pts_time = av_rescale_q(packet.dts, time_base, time_base_q);
			int64_t now_time = av_gettime() - start_time;
			if(pts_time > now_time)
				av_usleep(pts_time - now_time);
		}

		//Convert PTS/DTS
		//ac_rescale_q(a,b,c) = a * b / c
		packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);

		av_write_frame(output_format, &packet);

		av_packet_unref(&packet);
	}

	av_write_trailer(output_format);

	if(!(output_format->oformat->flags & AVFMT_NOFILE)){
		ret = avio_close(output_format->pb);
		assert(ret >= 0);
	}

	avformat_close_input(&format_ctx);

	avformat_free_context(output_format);
	avformat_free_context(format_ctx);

	avformat_network_deinit();

	return 0;
}
