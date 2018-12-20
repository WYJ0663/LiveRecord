package com.example.liverecord;

import android.os.Handler;
import android.os.HandlerThread;

import javax.xml.transform.sax.TemplatesHandler;

public class RmtpManager implements IManager {
    private boolean mIsRecord = false;
    private AudioManager mAudioManager;
    public CameraManager mCameraManager;

    HandlerThread mHandlerThread;
    Handler mHandler;

    public RmtpManager() {
        mHandlerThread = new HandlerThread("rtmp");
        mHandlerThread.start();
        mHandler = new Handler(mHandlerThread.getLooper());

        _init("rtmp://172.17.12.113:7777/live/room");

        mAudioManager = new AudioManager(mHandler);
        mCameraManager = new CameraManager(mHandler);
    }

    @Override
    public void start() {
        mIsRecord = true;
        mAudioManager.start();
        mCameraManager.start();
    }

    @Override
    public void stop() {
        mIsRecord = false;
        mAudioManager.stop();
        mCameraManager.stop();
    }

    @Override
    public boolean isRecord() {
        return mIsRecord;
    }

    @Override
    public void release() {
//        mAudioManager.release();
        mCameraManager.release();
        _release();
    }

    static {
        System.loadLibrary("rtmp");
        System.loadLibrary("native-lib");
    }


    public native int _init(String url);

    public native void _release();

}
