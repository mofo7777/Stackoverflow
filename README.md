# Stackoverflow
All source codes I've provided on stackoverflow as an answer, usually under tags ms-media-foundation.

## CustomVideoMixer
This program shows the basic essentials for implementing a custom video mixer, to be used by a media session.

You must register the CustomVideoMixer.dll with this command, and with the administrator rights : regsrv32 CustomVideoMixer.dll.

Be careful, the width and height of both videos are hardcoded (320x240). See VIDEO_WIDTH_1 and VIDEO_HEIGHT_1, change it if necessary.

In this example, I use two identical mp4 files, big_buck_bunny_240p_5mb.mp4 and a copy of itself. the two videos use NV12 video format as input. therefore, there is no real alpha mix, but both videos can be displayed next to each other, as the picture below.

![Custom Video Mixer](./Image/CustomVideoMixer.jpg)

## FrameRateConverterDSP

This program shows how to use the Frame Rate Converter DSP (CLSID_CFrameRateConvertDmo), using a Source Reader. You can change the frame rate of a video stream. For example, if the video has 30 fps, you can get 60 fps.

## ScreenCaptureEncode

This program takes screenshots using Directx9, and create a mp4 video file with Mediafoundation API.

In this example, the video duration is 5 seconds, see VIDEO_FRAME_COUNT to change it.

## TranscodeMp4ToMp4
This program transcode a mp4 video file into a new mp4 file. There is no real encoding process, because output file has the same format as input file.
The concept was to show how to setup IMFTopologyNode for both audio and video.
