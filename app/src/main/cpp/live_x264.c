//
// Created by yijunwu on 2018/12/18.
//

#include <malloc.h>
#include <memory.h>
#include <libx264/x264.h>
#include "live_x264.h"
#include "log.h"
#include "live_rtmp.h"
#include "yuv_util.h"

void video_push(LiveX264 *liveX264, x264_nal_t *nals, int num_nals) {
    LOGE("video_call")
    LiveRtmp *liveRtmp = liveX264->liveRtmp;

    for (int i = 0; i < num_nals; i++) {
        switch (nals[i].i_type) {
            case NAL_SLICE:
                LOGE("push P data")
                pushVideoData(liveRtmp, nals[i].p_payload, nals[i].i_payload, 0);
                break;
            case NAL_SLICE_IDR:
                LOGE("push I data")
                pushSPSPPS(liveRtmp, liveX264->sps, liveX264->sps_len,
                           liveX264->pps, liveX264->pps_len);
                pushVideoData(liveRtmp, nals[i].p_payload, nals[i].i_payload, 1);
                break;
            case NAL_SPS:
                LOGE("push SPS data")
                liveX264->sps = nals[i].p_payload;
                liveX264->sps_len = nals[i].i_payload;
                break;
            case NAL_PPS:
                LOGE("push PPS data")
                liveX264->pps = nals[i].p_payload;
                liveX264->pps_len = nals[i].i_payload;
                break;
        }
    }
}

void encode_NV21(LiveX264 *liveX264, LivePackage *package) {
    int length = package->length;
    char *data = package->data;

    char *I420 = (char *) (malloc(length * sizeof(char)));
    char *I420_90 = (char *) (malloc(length * sizeof(char)));

    NV21ToI420(I420, data, length);
    YUV420Rotate90(I420_90, I420, liveX264->width, liveX264->height);

    LOGE("NV21ToI420 length %d ", length)
    char *out = (char *) malloc(length * sizeof(char));
    int *len = malloc(10 * sizeof(int));

    int num_nals = x264_encode(liveX264, I420_90, liveX264->pts, out, len);

    liveX264->pts++;

    free(data);
    free(out);
    free(len);
    free(I420);
    free(I420_90);
}

void *work_video(void *arg) {
    LiveX264 *liveX264 = (LiveX264 *) arg;
    liveX264->queue = createQueue();

    while (liveX264->liveRtmp->is_start) {
        LivePackage *livePackage = getQueue(liveX264->queue);
        encode_NV21(liveX264, livePackage);
        free(livePackage);
    }

    freeQueue(liveX264->queue);

    return 0;
}

x264_param_t setParams(LiveX264 *liveX264) {
    x264_param_t params;

    LOGE("setParams width %d height %d bitrate %d orientation %d",
         liveX264->width, liveX264->height, liveX264->bitrate, liveX264->orientation);
    LOGE("encoder1 %d %d ", liveX264->encoder, &params);
    //preset
    //默认：medium
    //一些在压缩效率和运算时间中平衡的预设值。如果指定了一个预设值，它会在其它选项生效前生效。
    //可选：ultrafast, superfast, veryfast, faster, fast, medium, slow, slower, veryslow and placebo.
    //建议：可接受的最慢的值
    //tune
    //默认：无
    //说明：在上一个选项基础上进一步优化输入。如果定义了一个tune值，它将在preset之后，其它选项之前生效。
    //可选：film, animation, grain, stillimage, psnr, ssim, fastdecode, zerolatency and touhou.
    //建议：根据输入选择。如果没有合适的就不要指定。
    //后来发现设置x264_param_default_preset(&param, "fast" , "zerolatency" );后就能即时编码了
    x264_param_default_preset(&params, "veryfast", "zerolatency");

    //I帧间隔
    params.i_csp = X264_CSP_I420;
    params.i_csp = X264_CSP_I420;
    if (liveX264->orientation == 90 || liveX264->orientation == 270) {
        params.i_width = liveX264->height;
        params.i_height = liveX264->width;
    } else {
        params.i_width = liveX264->width;
        params.i_height = liveX264->height;
    }

    //并行编码多帧
    params.i_threads = X264_SYNC_LOOKAHEAD_AUTO;
    params.i_fps_num = 25;//getFps();
    params.i_fps_den = 1;

    // B frames 两个相关图像间B帧的数目 */
    params.i_bframe = 5;//getBFrameFrq();
    params.b_sliced_threads = 1;
    params.b_vfr_input = 0;
    params.i_timebase_num = params.i_fps_den;
    params.i_timebase_den = params.i_fps_num;

    // Intra refres:
    params.i_keyint_max = 25;
    params.i_keyint_min = 1;
    params.b_intra_refresh = 1;

    //参数i_rc_method表示码率控制，CQP(恒定质量)，CRF(恒定码率)，ABR(平均码率)
    //恒定码率，会尽量控制在固定码率
    params.rc.i_rc_method = X264_RC_CRF;
    //图像质量控制,rc.f_rf_constant是实际质量，越大图像越花，越小越清晰
    //param.rc.f_rf_constant_max ，图像质量的最大值
    params.rc.f_rf_constant = 25;
    params.rc.f_rf_constant_max = 35;

    // For streaming:
    //* 码率(比特率,单位Kbps)x264使用的bitrate需要/1000
    params.rc.i_bitrate = liveX264->bitrate / 1000;
    //瞬时最大码率,平均码率模式下，最大瞬时码率，默认0(与-B设置相同)
    params.rc.i_vbv_max_bitrate = liveX264->bitrate / 1000 * 1.2;
    params.b_repeat_headers = 1;
    params.b_annexb = 1;

    //是否把SPS和PPS放入每一个关键帧
    //SPS Sequence Parameter Set 序列参数集，PPS Picture Parameter Set 图像参数集
    //为了提高图像的纠错能力,该参数设置是让每个I帧都附带sps/pps。
    //param.b_repeat_headers = 1;
    //设置Level级别,编码复杂度
    params.i_level_idc = 51;

    //profile
    //默认：无
    //说明：限制输出文件的profile。这个参数将覆盖其它所有值，此选项能保证输出profile兼容的视频流。如果使用了这个选项，将不能进行无损压缩（qp 0 or crf 0）。
    //可选：baseline，main，high
    //建议：不设置。除非解码环境只支持main或者baseline profile的解码。
    x264_param_apply_profile(&params, "baseline");

    return params;
}


int x264_init(LiveX264 *liveX264, int mWidth, int mHeight, int bitrate, int orientation) {
    liveX264->width = mWidth;
    liveX264->height = mHeight;
    liveX264->bitrate = bitrate;
    liveX264->orientation = orientation;
    LOGE("video encoder setting width %d height %d bitrate %d orientation %d",
         liveX264->width, liveX264->height, liveX264->bitrate, liveX264->orientation);

    int r = 0;
    int nheader = 0;
    int header_size = 0;

    if (liveX264->encoder) {
        LOGE("Already opened. first call close()");
        return 0;
    }

    // set encoder parameters
    x264_param_t params = setParams(liveX264);
    //按照色度空间分配内存，即为图像结构体x264_picture_t分配内存，并返回内存的首地址作为指针
    //i_csp(图像颜色空间参数，目前只支持I420/YUV420)为X264_CSP_I420
    liveX264->pic_in = (x264_picture_t *) malloc(sizeof(x264_picture_t));
    liveX264->pic_out = (x264_picture_t *) malloc(sizeof(x264_picture_t));

    x264_picture_alloc(liveX264->pic_in, params.i_csp, params.i_width, params.i_height);
    //create the encoder using our params 打开编码器
    liveX264->encoder = x264_encoder_open(&params);
    LOGE("encoder %d %d ", liveX264->encoder, &params);
    if (!liveX264->encoder) {
        LOGE("Cannot open the encoder");
//        x264_release(liveX264);
        return 0;
    }

    x264_nal_t *nals;
    // write headers
    r = x264_encoder_headers(liveX264->encoder, &nals, &nheader);
    if (r < 0) {
        LOGE("x264_encoder_headers() failed");
        return 0;
    }

    //启动线程
    pthread_create(&liveX264->t, NULL, work_video, liveX264);

    return 1;
}

//编码h264帧
int x264_encode(LiveX264 *liveX264, char *inBytes, int pts, char *outBytes, int *outFrameSize) {
    int num_nals = 0;
    x264_nal_t *nals;

    //YUV420P数据转化为h264
    int i420_y_size = liveX264->width * liveX264->height;
    int i420_u_size = (liveX264->width >> 1) * (liveX264->height >> 1);
    int i420_v_size = i420_u_size;

    uint8_t *i420_y_data = (uint8_t *) inBytes;
    uint8_t *i420_u_data = (uint8_t *) inBytes + i420_y_size;
    uint8_t *i420_v_data = (uint8_t *) inBytes + i420_y_size + i420_u_size;
    //将Y,U,V数据保存到pic_in.img的对应的分量中，还有一种方法是用AV_fillPicture和sws_scale来进行变换
    memcpy(liveX264->pic_in->img.plane[0], i420_y_data, i420_y_size);
    memcpy(liveX264->pic_in->img.plane[1], i420_u_data, i420_u_size);
    memcpy(liveX264->pic_in->img.plane[2], i420_v_data, i420_v_size);
    LOGE("encodeFrame data");
    // and encode and store into pic_out
    liveX264->pic_in->i_pts = pts;
    //最主要的函数，x264编码，pic_in为x264输入，pic_out为x264输出
    int frame_size = x264_encoder_encode(liveX264->encoder, &nals, &num_nals, liveX264->pic_in,
                                         liveX264->pic_out);
    LOGE("encodeFrame data %d num_nals %d", frame_size, num_nals);
    if (frame_size) {
        if (num_nals > 0 && nals != 0) {
            video_push(liveX264, nals, num_nals);
        }
        return num_nals;
    }
    return -1;
}


int x264_release(LiveX264 *liveX264) {
    x264_encoder_close(liveX264->encoder);
    x264_picture_clean(liveX264->pic_in);
    x264_picture_clean(liveX264->pic_out);
    free(liveX264->pic_in);
    free(liveX264->pic_out);
}
