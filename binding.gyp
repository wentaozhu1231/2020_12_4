{
    "targets": [{
        "target_name": "testaddon",
        "cflags!": ["-fno-exceptions"],
        "cflags_cc!": ["-fno-exceptions"],
        "sources": [
            "cppsrc/main.cpp",
            "cppsrc/Samples/functionexample.cpp",
            "cppsrc/Samples/actualclass.cpp",
            "cppsrc/Samples/classexample.cpp"
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")",
            "E:/Program Files/DCMTKlib/include",
            "F:/opencv-4.5.0_build/install/include/opencv2",
            "F:/opencv-4.5.0_build/install/include"
        ],
        'library_dirs':[
            "E:/Program Files/DCMTKlib/lib",
            "F:/opencv-4.5.0_build/install/x64/vc16/staticlib "
        ],



        'libraries': [

            'dcmdata.lib',
            'dcmimgle.lib',
            'dcmimage.lib',
            'ofstd.lib',
            'oflog.lib',
            'ade.lib',
            'IlmImfd.lib',
            'ittnotifyd.lib',
            'libjpeg-turbod.lib',
            'libopenjp2d.lib',
            'libpngd.lib',
            'libprotobufd.lib',
            'libtiffd.lib',
            'libwebpd.lib',
            'opencv_calib3d450d.lib',
            'opencv_core450d.lib',
            'opencv_dnn450d.lib',
            'opencv_features2d450d.lib',
            'opencv_flann450d.lib',
            'opencv_gapi450d.lib',
            'opencv_highgui450d.lib',
            'opencv_imgcodecs450d.lib',
            'opencv_imgproc450d.lib',
            'opencv_ml450d.lib',
            'opencv_objdetect450d.lib',
            'opencv_photo450d.lib',
            'opencv_stitching450d.lib',
            'opencv_video450d.lib',
            'opencv_videoio450d.lib',
            'quircd.lib',
            'zlibd.lib',
            'netapi32.lib',
            'iphlpapi.lib',
            'ws2_32.lib',
            'wsock32.lib',
            'kernel32.lib',
            'user32.lib',
            'gdi32.lib',
            'winspool.lib',
            'comdlg32.lib',
            'advapi32.lib',
            'shell32.lib',
            'ole32.lib',
            'oleaut32.lib',
            'uuid.lib',
            'odbc32.lib',
            'odbccp32.lib'
        ],

        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': ['NAPI_DISABLE_CPP_EXCEPTIONS']
    }]
}
