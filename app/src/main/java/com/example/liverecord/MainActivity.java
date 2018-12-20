package com.example.liverecord;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;

public class MainActivity extends AppCompatActivity implements SurfaceHolder.Callback {


    private SurfaceView mSurfaceView;
    private Button mRecordVie;

    private RmtpManager mRmtpManager;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mSurfaceView = (SurfaceView) findViewById(R.id.surface_view);
        mSurfaceView.getHolder().addCallback(this);
        mRecordVie = (Button) findViewById(R.id.record);

        mRmtpManager = new RmtpManager();

        mRecordVie.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mRmtpManager.isRecord()) {
                    mRmtpManager.stop();
                    mRecordVie.setText("start");
                } else {
                    mRmtpManager.start();
                    mRecordVie.setText("stop");
                }
            }
        });
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        mRmtpManager.mCameraManager.surfaceCreated(holder);
    }

    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {

    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        mRmtpManager.release();
    }
}
