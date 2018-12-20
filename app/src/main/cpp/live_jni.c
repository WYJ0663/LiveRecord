#include <jni.h>
#include <malloc.h>

#include "live_x264.h"
#include "yuv_util.h"
#include "log.h"
#include "live_rtmp.h"
#include "live_fdk_aac.h"

LiveX264 *liveX264;
LiveRtmp *liveRtmp;
LiveAAC *liveAAC;


void video_call(x264_nal_t *nals, int num_nals) {
    LOGE("video_call")
    for (int i = 0; i < num_nals; i++) {
        switch (nals[i].i_type) {
            case NAL_SLICE:
                LOGE("push P data")
                pushVideoData(liveRtmp, nals[i].p_payload, nals[i].i_payload, 0);
                break;
            case NAL_SLICE_IDR:
                LOGE("push I data")
                pushSPSPPS(liveRtmp, liveX264->sps, liveX264->sps_len, liveX264->pps, liveX264->pps_len);
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

JNIEXPORT void JNICALL
Java_com_example_liverecord_CameraManager_videoInit(JNIEnv *env, jobject instance, jint width, jint height,
                                                    jint bitrate, jint orientation) {
//    const char *outfile1 = "/sdcard/2333.h264";
//    out1 = fopen(outfile1, "wb");

    liveX264 = malloc(sizeof(LiveX264));
    liveX264->encoder = 0;
    liveX264->pic_in = 0;
    liveX264->pic_out = 0;
    liveX264->pts = 0;
    x264_init(liveX264, width, height, bitrate, orientation);

    liveX264->video_call = video_call;
}

JNIEXPORT void JNICALL
Java_com_example_liverecord_CameraManager_pushYUV420(JNIEnv *env, jobject instance, jbyteArray bytes_, jint length) {
    jbyte *bytes = (*env)->GetByteArrayElements(env, bytes_, NULL);

    char *I420 = (char *) (malloc(length * sizeof(char)));
    char *I420_90 = (char *) (malloc(length * sizeof(char)));

    NV21ToI420(I420, bytes, length);
    YUV420Rotate90(I420_90, I420, liveX264->width, liveX264->height);

    LOGE("NV21ToI420 length %d ", length)
    char *out = (char *) malloc(length * sizeof(char));
    int *len = malloc(10 * sizeof(int));
//    int num_nals = x264_encode(liveX264, I420_90, liveX264->pts, out, len);


    free(out);
    free(len);
    free(I420);
    free(I420_90);
    (*env)->ReleaseByteArrayElements(env, bytes_, bytes, 0);
}

JNIEXPORT void JNICALL
Java_com_example_liverecord_CameraManager_videoRelease(JNIEnv *env, jobject instance) {
    x264_release(liveX264);

//    fclose(out1);
}

JNIEXPORT jint JNICALL
Java_com_example_liverecord_AudioManager_init(JNIEnv *env, jobject instance, jint channels, jint sampleRate,
                                              jint bitRate) {
    liveAAC = malloc(sizeof(LiveAAC));
    return aac_init(liveAAC, channels, sampleRate, bitRate);
}

JNIEXPORT void JNICALL
Java_com_example_liverecord_AudioManager_pushPCM(JNIEnv *env, jobject instance, jbyteArray bytes_, jint length) {
    jbyte *bytes = (*env)->GetByteArrayElements(env, bytes_, NULL);

    LOGE("length = %d", length);
    int16_t *convert_buf = (int16_t *) malloc(length);
    for (int i = 0; i < length / 2; i++) {
        const uint8_t *in = &bytes[2 * i];
        convert_buf[i] = in[0] | (in[1] << 8);
    }

    int outLength = length;
    uint8_t *out = malloc(outLength);
    LOGE("outLength1 %d", outLength);

    int len = 0;
    len = aac_encode_audio(liveAAC, convert_buf, length, out, outLength);

    LOGE("len %d", len);
//    fwrite(out, 1, len, out1);

    pushAudioData(liveRtmp, out, len);

    free(out);

    (*env)->ReleaseByteArrayElements(env, bytes_, bytes, 0);
}

JNIEXPORT void JNICALL
Java_com_example_liverecord_AudioManager__1release(JNIEnv *env, jobject instance) {

    aac_release(liveAAC);
}

JNIEXPORT jint JNICALL
Java_com_example_liverecord_RmtpManager__1init(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = (*env)->GetStringUTFChars(env, url_, 0);
    LOGE("rtmp init %s",url);
    liveRtmp = malloc(sizeof(LiveRtmp));
    rtmp_init(liveRtmp, url);

    (*env)->ReleaseStringUTFChars(env, url_, url);
}

JNIEXPORT void JNICALL
Java_com_example_liverecord_RmtpManager__1release(JNIEnv *env, jobject instance) {

    rtmp_release(liveRtmp);
}