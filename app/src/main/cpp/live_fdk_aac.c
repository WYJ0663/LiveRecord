//
// Created by yijunwu on 2018/12/7.
//


#include <fdk-aac/aacenc_lib.h>
#include <malloc.h>
#include "log.h"
#include "live_fdk_aac.h"

void encode_PCM(LiveAAC *liveAAC, LivePackage *package) {
    int length = package->length;
    char *data = package->data;

    LOGE("length = %d", length);
    int16_t *convert_buf = (int16_t *) malloc(length);
    for (int i = 0; i < length / 2; i++) {
        const uint8_t *in = &data[2 * i];
        convert_buf[i] = in[0] | (in[1] << 8);
    }

    int outLength = length;
    uint8_t *out = malloc(outLength);
    LOGE("outLength1 %d", outLength);

    int len = 0;
    len = aac_encode_audio(liveAAC, convert_buf, length, out, outLength);

    LOGE("len %d", len);

    pushAudioData(liveAAC->liveRtmp, out, len);

    free(data);
    free(out);
}

void *work_audio(void *arg) {
    LiveAAC *liveAAC = (LiveAAC *) arg;
    liveAAC->queue = createQueue();
    while (liveAAC->liveRtmp->is_start) {
        LivePackage *livePackage = getQueue(liveAAC->queue);
        encode_PCM(liveAAC, livePackage);
        free(livePackage);
    }

    freeQueue(liveAAC->queue);
    return 0;
}


/**
 * 初始化fdk-aac的参数，设置相关接口使得
 * @return
 */
int aac_init(LiveAAC *liveAAC, int channels, int sampleRate, int bitRate) {
    liveAAC->channels = channels;
    liveAAC->sample_rate = sampleRate;
    liveAAC->bitRate = bitRate;

    HANDLE_AACENCODER handle;

    int aot = 2;
    int afterburner = 1;
    int eld_sbr = 0;
    int vbr = 0;
    CHANNEL_MODE mode = MODE_1;
    AACENC_InfoStruct info = {0};
    LOGE("channels sample_rate bitrate %d %d %d", channels, sampleRate, bitRate);
    switch (channels) {
        case 1:
            mode = MODE_1;
            break;
        case 2:
            mode = MODE_2;
            break;
        case 3:
            mode = MODE_1_2;
            break;
        case 4:
            mode = MODE_1_2_1;
            break;
        case 5:
            mode = MODE_1_2_2;
            break;
        case 6:
            mode = MODE_1_2_2_1;
            break;
        default:
            LOGE("Unsupported WAV channels %d\n", channels);
            return 1;
    }
    if (aacEncOpen(&handle, 0, channels) != AACENC_OK) {
        LOGE("Unable to open encoder\n");
        return 1;
    }
    if (aacEncoder_SetParam(handle, AACENC_AOT, aot) != AACENC_OK) {
        LOGE("Unable to set the AOT\n");
        return 1;
    }
    if (aot == 39 && eld_sbr) {
        if (aacEncoder_SetParam(handle, AACENC_SBR_MODE, 1) != AACENC_OK) {
            LOGE("Unable to set SBR mode for ELD\n");
            return 1;
        }
    }
    if (aacEncoder_SetParam(handle, AACENC_SAMPLERATE, sampleRate) != AACENC_OK) {
        LOGE("Unable to set the AOT\n");
        return 1;
    }
    if (aacEncoder_SetParam(handle, AACENC_CHANNELMODE, mode) != AACENC_OK) {
        LOGE("Unable to set the channel mode\n");
        return 1;
    }
    if (aacEncoder_SetParam(handle, AACENC_CHANNELORDER, 1) != AACENC_OK) {
        LOGE("Unable to set the wav channel order\n");
        return 1;
    }
    if (vbr) {
        if (aacEncoder_SetParam(handle, AACENC_BITRATEMODE, vbr) != AACENC_OK) {
            LOGE("Unable to set the VBR bitrate mode\n");
            return 1;
        }
    } else {
        if (aacEncoder_SetParam(handle, AACENC_BITRATE, bitRate) != AACENC_OK) {
            LOGE("Unable to set the bitrate\n");
            return 1;
        }
    }
    if (aacEncoder_SetParam(handle, AACENC_TRANSMUX, TT_MP4_ADTS) != AACENC_OK) {
        LOGE("Unable to set the ADTS transmux\n");
        return 1;
    }
    if (aacEncoder_SetParam(handle, AACENC_AFTERBURNER, afterburner) != AACENC_OK) {
        LOGE("Unable to set the afterburner mode\n");
        return 1;
    }
    if (aacEncEncode(handle, NULL, NULL, NULL, NULL) != AACENC_OK) {
        LOGE("Unable to initialize the encoder\n");
        return 1;
    }
    if (aacEncInfo(handle, &info) != AACENC_OK) {
        LOGE("Unable to get the encoder info\n");
        return 1;
    }

    //返回数据给上层，表示每次传递多少个数据最佳，这样encode效率最高
    int inputSize = channels * 2 * info.frameLength;
    LOGE("inputSize = %d", inputSize);

    liveAAC->handle = handle;

    //启动线程
    pthread_create(&liveAAC->t, NULL, work_audio, liveAAC);

    return inputSize;
}

/**
 * Fdk-AAC库压缩裸音频PCM数据，转化为AAC，这里为什么用fdk-aac，这个库相比普通的aac库，压缩效率更高
 * @param inBytes
 * @param length
 * @param outBytes
 * @param outLength
 * @return
 */
int aac_encode_audio(LiveAAC *liveAAC, unsigned char *convert_buf, int read, unsigned char *outbuf, int outLength) {
    AACENC_BufDesc in_buf = {0}, out_buf = {0};
    AACENC_InArgs in_args = {0};
    AACENC_OutArgs out_args = {0};
    int in_identifier = IN_AUDIO_DATA;
    int in_size, in_elem_size;
    int out_identifier = OUT_BITSTREAM_DATA;
    int out_size, out_elem_size;
    void *in_ptr, *out_ptr;

    in_ptr = convert_buf;
    in_size = read;
    in_elem_size = 2;

    in_args.numInSamples = read <= 0 ? -1 : read / 2;
    in_buf.numBufs = 1;
    in_buf.bufs = &in_ptr;
    in_buf.bufferIdentifiers = &in_identifier;
    in_buf.bufSizes = &in_size;
    in_buf.bufElSizes = &in_elem_size;

    out_ptr = outbuf;
//    out_size = sizeof(outbuf);
    out_size = outLength;
    out_elem_size = 1;
    out_buf.numBufs = 1;
    out_buf.bufs = &out_ptr;
    out_buf.bufferIdentifiers = &out_identifier;
    out_buf.bufSizes = &out_size;
    out_buf.bufElSizes = &out_elem_size;

    AACENC_ERROR err;
    if ((err = aacEncEncode(liveAAC->handle, &in_buf, &out_buf, &in_args, &out_args)) != AACENC_OK) {
        if (err == AACENC_ENCODE_EOF)
            LOGE("Encoding failed\n");
        return 0;
    }
    LOGE("numOutBytes %d", out_args.numOutBytes);

    return out_args.numOutBytes;
}

int aac_release(LiveAAC *liveAAC) {
    if (liveAAC->handle) {
        aacEncClose(&liveAAC->handle);
        liveAAC->handle = NULL;
    }
    return 1;
}

