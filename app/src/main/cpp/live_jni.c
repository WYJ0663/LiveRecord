#include <jni.h>
#include <malloc.h>

#include "live_x264.h"
#include "log.h"
#include "live_rtmp.h"
#include "live_fdk_aac.h"

LiveX264 *liveX264;
LiveRtmp *liveRtmp;
LiveAAC *liveAAC;

void addQueue(jint length, jbyte *bytes);

//初始化
JNIEXPORT void JNICALL
Java_com_example_liverecord_RmtpManager__1init(JNIEnv *env, jobject instance, jstring url_) {
    const char *url = (*env)->GetStringUTFChars(env, url_, 0);
    LOGE("rtmp init %s", url);
    liveRtmp = malloc(sizeof(LiveRtmp));
    rtmp_init(liveRtmp, url);

    (*env)->ReleaseStringUTFChars(env, url_, url);
}

JNIEXPORT void JNICALL
Java_com_example_liverecord_CameraManager_videoInit(JNIEnv *env, jobject instance, jint width, jint height,
                                                    jint bitrate, jint orientation) {
    liveX264 = malloc(sizeof(LiveX264));
    liveX264->encoder = 0;
    liveX264->pic_in = 0;
    liveX264->pic_out = 0;
    liveX264->pts = 0;
    x264_init(liveX264, width, height, bitrate, orientation);
    liveX264->liveRtmp = liveRtmp;
}

JNIEXPORT jint JNICALL
Java_com_example_liverecord_AudioManager_init(JNIEnv *env, jobject instance, jint channels, jint sampleRate,
                                              jint bitRate) {
    liveAAC = malloc(sizeof(LiveAAC));
    liveAAC->liveRtmp = liveRtmp;
    return aac_init(liveAAC, channels, sampleRate, bitRate);
}


void addDataToQueue(Queue *queue, int type, jbyte *bytes, jint length) {
    LivePackage *package = malloc(sizeof(LivePackage));

    package->type = type;
    package->data = (char *) (malloc(length * sizeof(char)));
    memcpy(package->data, bytes, length);
    package->length = length;
    putQueue(queue, package);
}

void JNICALL
Java_com_example_liverecord_CameraManager_pushNV21(JNIEnv *env, jobject instance, jbyteArray bytes_, jint length) {
    jbyte *bytes = (*env)->GetByteArrayElements(env, bytes_, NULL);

    addDataToQueue(liveX264->queue, TYPE_NV21, bytes, length);

    (*env)->ReleaseByteArrayElements(env, bytes_, bytes, 0);
}

//void encode_PCM(char *data, int length) {
//
//    LOGE("length = %d", length);
//    int16_t *convert_buf = (int16_t *) malloc(length);
//    for (int i = 0; i < length / 2; i++) {
//        const uint8_t *in = &data[2 * i];
//        convert_buf[i] = in[0] | (in[1] << 8);
//    }
//
//    int outLength = length;
//    uint8_t *out = malloc(outLength);
//    LOGE("outLength1 %d", outLength);
//
//    int len = 0;
//    len = aac_encode_audio(liveRtmp->liveAAC, convert_buf, length, out, outLength);
//
//    LOGE("len %d", len);
//
//    pushAudioData(liveRtmp, out, len);
//
//    free(out);
//}

JNIEXPORT void JNICALL
Java_com_example_liverecord_AudioManager_pushPCM(JNIEnv *env, jobject instance, jbyteArray bytes_, jint length) {
    jbyte *bytes = (*env)->GetByteArrayElements(env, bytes_, NULL);

    addDataToQueue(liveAAC->queue, TYPE_PCM, bytes, length);
//    encode_PCM(bytes, length);

    (*env)->ReleaseByteArrayElements(env, bytes_, bytes, 0);
}


//销毁
JNIEXPORT void JNICALL
Java_com_example_liverecord_RmtpManager__1release(JNIEnv *env, jobject instance) {
    rtmp_release(liveRtmp);
    free(liveRtmp);
}

JNIEXPORT void JNICALL
Java_com_example_liverecord_CameraManager_videoRelease(JNIEnv *env, jobject instance) {
    x264_release(liveX264);
    free(liveX264);
}

JNIEXPORT void JNICALL
Java_com_example_liverecord_AudioManager__1release(JNIEnv *env, jobject instance) {
    aac_release(liveAAC);
    free(liveAAC);
}

//////////////////////////

Queue *queue = 0;

void *start_play(void *arg) {

    queue = createQueue();
    for (int i = 0; i < 10; ++i) {
        int *c = malloc(1024 * 1024);
        c[0] = i;
        putQueue(queue, c);
    }

    while (1) {
        int *i = (int *) getQueue(queue);
        LOGE("queue %d", i[0]);
        free(i);
    }

}

JNIEXPORT void JNICALL
Java_com_example_liverecord_RmtpManager_queuetest(JNIEnv *env, jclass type) {
    if (queue == 0) {
        pthread_t t = 0;
        pthread_create(&t, NULL, start_play, NULL);
    }
//    pthread_join(t,0);

}

JNIEXPORT void JNICALL
Java_com_example_liverecord_RmtpManager_queueadd(JNIEnv *env, jclass type) {

    if (queue != 0) {
        int *c = malloc(1024 * 1024 * 100);
        c[0] = 12;
        putQueue(queue, c);
    }

}

JNIEXPORT void JNICALL
Java_com_example_liverecord_RmtpManager_queueend(JNIEnv *env, jclass type) {

    if (queue != 0) {
        cleanQueue(queue);
        free(queue);
        queue = 0;
    }

}