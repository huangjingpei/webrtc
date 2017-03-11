/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */


#include "webrtc/base/criticalsection.h"
#include "webrtc/base/platform_thread.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "webrtc/system_wrappers/include/event_wrapper.h"
#include "webrtc/system_wrappers/include/sleep.h"
#include "webrtc/test/frame_generator.h"
#include "webrtc/video_send_stream.h"
#include "webrtc/modules/desktop_capture/desktop_frame.h"
#include "screen_region_capture.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/modules/desktop_capture/desktop_capture_options.h"


namespace webrtc {
namespace test {

ScreenRegionCapture* ScreenRegionCapture::Create(VideoCaptureInput* input,
                                                       size_t width,
                                                       size_t height,
                                                       int target_fps,
                                                       Clock* clock) {
  ScreenRegionCapture* capturer = new ScreenRegionCapture(
      clock, input, ScreenCapturer::Create(DesktopCaptureOptions::CreateDefault()),
      target_fps);
  if (!capturer->Init()) {
    delete capturer;
    return NULL;
  }

  return capturer;
}

ScreenRegionCapture::ScreenRegionCapture(Clock* clock,
                                               VideoCaptureInput* input,
                                               ScreenCapturer* screen_capture,
                                               int target_fps)
    : input_(input),
      clock_(clock),
      sending_(false),
      tick_(EventTimerWrapper::Create()),
      thread_(ScreenRegionCapture::Run, this, "ScreenRegionCapture"),
      capturer_(screen_capture),
      target_fps_(target_fps),
      first_frame_capture_time_(-1) {
  assert(input != NULL);
  assert(capturer_ != NULL);
  assert(target_fps > 0);
}

ScreenRegionCapture::~ScreenRegionCapture() {
  Stop();

  thread_.Stop();
}

void ScreenRegionCapture::SetFakeRotation(VideoRotation rotation) {
  rtc::CritScope cs(&lock_);
  fake_rotation_ = rotation;
}

bool ScreenRegionCapture::Init() {
  // This check is added because frame_generator_ might be file based and should
  // not crash because a file moved.
  if (capturer_.get() == NULL)
    return false;

  if (!tick_->StartTimer(true, 1000 / target_fps_))
    return false;
  thread_.Start();
  thread_.SetPriority(rtc::kHighPriority);

  capturer_->Start(this);
  return true;
}

bool ScreenRegionCapture::Run(void* obj) {
  static_cast<ScreenRegionCapture*>(obj)->InsertFrame();
  return true;
}

void ScreenRegionCapture::InsertFrame() {
  {
    rtc::CritScope cs(&lock_);
	printf("thread ru clock %lld\n", clock_->TimeInMilliseconds());

    if (sending_) {

      //capturer_->Capture(DesktopRegion());

      //input_->IncomingCapturedFrame(*frame);
    }
  }
  tick_->Wait(WEBRTC_EVENT_INFINITE);
}

void ScreenRegionCapture::Start() {
  rtc::CritScope cs(&lock_);
  sending_ = true;
}

void ScreenRegionCapture::Stop() {
  rtc::CritScope cs(&lock_);
  sending_ = false;
}

void ScreenRegionCapture::ForceFrame() {
  tick_->Set();
}

void ScreenRegionCapture::OnCaptureCompleted(DesktopFrame* frame)
{
    frame_.reset(frame);
    int width = frame->size().width();
    int height = frame->size().height();
    printf("w  %d h %d\n", width, height);
    VideoFrame video_frame_;
    char *frame_buffer = (char *)malloc(width*height*3/2);
    video_frame_.CreateFrame((const uint8_t*)frame_buffer, width, height, kVideoRotation_0);
    int ret = ConvertToI420(VideoType::kARGB, frame->data(), 0, 0, frame->size().width(),
            frame->size().height(), 0, kVideoRotation_0, &video_frame_);
    free(frame_buffer);
}

}  // test
}  // webrtc
