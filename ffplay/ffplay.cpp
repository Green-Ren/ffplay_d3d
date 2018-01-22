
#include "stdafx.h"
#include "dxva.h"
#include "D3DVidRender.h"
#include "ffplay.h"

AVPixelFormat GetHwFormat(AVCodecContext *s, const AVPixelFormat *pix_fmts)
{
	InputStream* ist = (InputStream*)s->opaque;
	ist->active_hwaccel_id = HWACCEL_DXVA2;
	ist->hwaccel_pix_fmt = AV_PIX_FMT_DXVA2_VLD;
	return ist->hwaccel_pix_fmt;
}

BOOL HWAccelInit(AVCodec *codec, AVCodecContext *ctx, HWND hWnd)
{
	bool bRet = TRUE;
	switch (codec->id)
	{
		case AV_CODEC_ID_MPEG2VIDEO:
		case AV_CODEC_ID_H264:
		case AV_CODEC_ID_VC1:
		case AV_CODEC_ID_WMV3:
		case AV_CODEC_ID_HEVC:
		case AV_CODEC_ID_VP9:
		{
			// multi threading is apparently not compatible 
			// with hardware decoding
			ctx->thread_count = 1;  
			InputStream *ist = new InputStream();
			ist->hwaccel_id = HWACCEL_AUTO;
			ist->active_hwaccel_id = HWACCEL_AUTO;
			ist->hwaccel_device = "dxva2";
			ist->dec = codec;
			ist->dec_ctx = ctx;
			ctx->opaque = ist;

			if (dxva2_init(ctx, hWnd) == 0)
			{
				ctx->get_buffer2 = ist->hwaccel_get_buffer;
				ctx->get_format = GetHwFormat;
				ctx->thread_safe_callbacks = 1;
				bRet = TRUE;
			}
			else
			{
				bRet = FALSE;
			}

			break;
		}
		default:
		{
			bRet = FALSE;
			break;
		}

	}

	return bRet;
}



CD3DVidRender m_D3DVidRender;

BOOL PlayVideoFile(const char *filename, HWND winId)
{
	DWORD start_time = GetTickCount();
	printf("start decoding, time = %d\n", start_time);

	int setup_hwdecode = 0;

	av_register_all();

	AVFormatContext *fc = NULL;
	int res = avformat_open_input(&fc, filename, NULL, NULL);
	if (res < 0) 
	{
		printf("error %x in avformat_open_input\n", res);
		return FALSE;
	}

	res = avformat_find_stream_info(fc, NULL);
	if (res < 0)
	{
		printf("error %x in avformat_find_stream_info\n", res);
		return FALSE;
	}

	av_dump_format(fc, 0, filename, 0);

	int videoindex = -1;

	for (UINT i = 0; i < fc->nb_streams; i++)
	{
		if (AVMEDIA_TYPE_VIDEO == fc->streams[i]->codec->codec_type)
		{
			videoindex = i;
		}
	}

	if (videoindex == -1)
	{
		av_log(NULL, AV_LOG_DEBUG, "can't find video stream\n");
		return FALSE;
	}

	AVCodec *codec = avcodec_find_decoder(fc->streams[videoindex]->codec->codec_id);
	if (!codec)
	{
		printf("decoder not found\n");
		return FALSE;
	}

	AVCodecContext *ctx = fc->streams[videoindex]->codec;

	AVCodecContext ctxTmp = *ctx;
	bool bAccel = false;
	if (bAccel)
	{
		BOOL bRet = HWAccelInit(codec, ctx, winId);
		if (!bRet)
		{
			bAccel = false;
		}
	}

	if (ctx->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		printf("codec type: AVMEDIA_TYPE_VIDEO\n");
	}

	AVFrame	*pFrameBGR = NULL;
	uint8_t	*out_buffer = NULL;
	struct SwsContext *img_convert_ctx = NULL ;
	if (!bAccel)
	{
		avcodec_close(ctx);
		ctx = &ctxTmp;

		m_D3DVidRender.InitD3D_YUV(winId, ctx->width, ctx->height);

		pFrameBGR = av_frame_alloc();
		out_buffer = (uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, 
			ctx->width, ctx->height, 1));
		av_image_fill_arrays(pFrameBGR->data, ((AVPicture *)pFrameBGR)->linesize, 
			out_buffer, AV_PIX_FMT_YUV420P, 
			ctx->width, ctx->height, 1);

		img_convert_ctx = sws_getContext(ctx->width, ctx->height, 
			ctx->pix_fmt, 
			ctx->width, ctx->height, 
			AV_PIX_FMT_YUV420P, 
			SWS_BICUBIC, 
			NULL, NULL, NULL);
	}

	res = avcodec_open2(ctx, codec, NULL);
	if (res < 0) 
	{
		printf("error %x in avcodec_open2\n", res);
		return FALSE;
	}

	AVPacket pkt = { 0 };
	AVFrame *picture = av_frame_alloc();
	DWORD wait_for_keyframe = 60;

	int count = 0;

	while (av_read_frame(fc, &pkt) == 0)
	{
		if (pkt.stream_index == videoindex)
		{
			int got_picture = 0;

			DWORD t_start = GetTickCount();
			int bytes_used = avcodec_decode_video2(ctx, picture, &got_picture, &pkt);
			if (got_picture)
			{
				Sleep(30);
				if (bAccel)
				{
					dxva2_retrieve_data_call(ctx, picture);
					DWORD t_end = GetTickCount();
					printf("dxva2 time using: %lu\n", t_end - t_start);
				}
				else
				{
					if (img_convert_ctx &&pFrameBGR && out_buffer)
					{
						sws_scale(img_convert_ctx, 
							(const uint8_t* const*)picture->data, 
							picture->linesize, 0, 
							ctx->height, 
							pFrameBGR->data, 
							pFrameBGR->linesize);

						m_D3DVidRender.Render_YUV(out_buffer, picture->width, picture->height);

						DWORD t_end = GetTickCount();
						printf("normal time using: %lu\n", t_end - t_start);
					}
				}
				count++;
			}

			av_packet_unref(&pkt);
		}
		else
		{
			av_packet_unref(&pkt);
		}
	}

	av_free(picture);

	avcodec_close(ctx);

	avformat_close_input(&fc);

	return TRUE;
}