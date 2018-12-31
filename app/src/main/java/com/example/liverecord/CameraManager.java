package com.example.liverecord;

import android.app.Activity;
import android.graphics.ImageFormat;
import android.hardware.Camera;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
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
    private static final int width = 640;
    private static final int height = 480;

    private static final int framerate = 30;
    private static final int biterate = 1024;

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

            Camera.Size size = getOptimalSize(parameters.getSupportedPreviewSizes(), width, height);


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
                        pushNV21(bytes, bytes.length);
                    }
                }
            });
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

//             size.width =1920  size.height=1080
//             size.width =1440  size.height=1080
//             size.width =1280  size.height=960
//             size.width =1280  size.height=720
//             size.width =960  size.height=720
//             size.width =720  size.height=720
//             size.width =640  size.height=480
//             size.width =352  size.height=288
//             size.width =320  size.height=240
//             size.width =208  size.height=144
//             size.width =176  size.height=144

    private static Camera.Size getOptimalSize(List<Camera.Size> sizes, int w, int h) {
        final double ASPECT_TOLERANCE = 0.1;
        double targetRatio = (double) h / w;
        Camera.Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;

        for (Camera.Size size : sizes) {
            Log.e("yijun", "size.width=" + size.width + "  size.height=" + size.height);
            double ratio = (double) size.width / size.height;
            double de = Math.abs(ratio - targetRatio);
            if (de < minDiff) {
                optimalSize = size;
                minDiff = de;
            }
        }

        return optimalSize;
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

    public native void pushNV21(byte[] bytes, int length);

    public native void videoRelease();
}
