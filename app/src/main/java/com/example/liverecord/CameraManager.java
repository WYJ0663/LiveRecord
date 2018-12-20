package com.example.liverecord;

import android.app.Activity;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.os.Handler;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

import java.io.IOException;
import java.util.List;

/**
 * 录制视频x264编码
 */
public class CameraManager implements IManager {
    private static final int width = 1280;
    private static final int height = 720;
    private static final int framerate = 30;
    private static final int biterate = 128;

    private Camera mCamera;
    private boolean mIsRecord = false;

    //orientation 0, 90, 180, and 270.
    private int mOrientation = 90;

    private Handler mHandler;

    public CameraManager(Handler handler) {
        mHandler = handler;
        videoInit(width, height, biterate, mOrientation);
    }

    //------ Surface 预览 -------
    public void surfaceCreated(SurfaceHolder surfaceHolder) {
        try {
            mCamera = Camera.open();
            mCamera.setDisplayOrientation(mOrientation);
            mCamera.cancelAutoFocus();
            // setCameraDisplayOrientation(this, 0, camera);
            Camera.Parameters parameters = mCamera.getParameters();
            parameters.setPreviewFormat(ImageFormat.NV21);
            parameters.setPreviewSize(width, height);
            parameters.setPictureSize(width, height);
//            for (String s : parameters.getSupportedFocusModes()) {
//                if (Camera.Parameters.FOCUS_MODE_AUTO.equals(s)) {
//                    parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_AUTO);
//                }
//            }
            parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_PICTURE);
            mCamera.setParameters(parameters);
            mCamera.setPreviewDisplay(surfaceHolder);
            mCamera.startPreview();
            mCamera.setPreviewCallback(new Camera.PreviewCallback() {
                @Override
                public void onPreviewFrame(final byte[] bytes, Camera camera) {
                    // 获取NV21数据
                    if (mIsRecord) {
                        mHandler.post(new Runnable() {
                            @Override
                            public void run() {
//                                pushYUV420(bytes, bytes.length);
                            }
                        });
                        pushYUV420(bytes, bytes.length);
                    }
                }
            });
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    public void start() {
        mIsRecord = true;
    }

    public void stop() {
        mIsRecord = false;
    }

    public boolean isRecord() {
        return mIsRecord;
    }

    public void release() {
        videoRelease();
    }

    static {
        System.loadLibrary("x2641");
        System.loadLibrary("native-lib");
    }

    //orientation 0, 90, 180, and 270.
    public native void videoInit(int width, int height, int bitrate, int orientation);

    public native void pushYUV420(byte[] bytes, int length);

    public native void videoRelease();
}
