/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef WEBRTC_TEST_FRAME_GENERATOR_CAPTURER_H_
#define WEBRTC_TEST_FRAME_GENERATOR_CAPTURER_H_

#include <string>

#include "webrtc/base/criticalsection.h"
#include "webrtc/base/platform_thread.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/common_video/rotation.h"
#include "webrtc/test/video_capturer.h"
#include "webrtc/typedefs.h"
#include "webrtc/modules/desktop_capture/desktop_frame.h"
#include "webrtc/modules/desktop_capture/screen_capturer.h"


namespace webrtc {

class CriticalSectionWrapper;
class EventTimerWrapper;

namespace test {

class FrameGenerator;

class ScreenRegionCapture :public DesktopCapturer::Callback {
 public:
  static ScreenRegionCapture* Create(VideoCaptureInput* input,
                                        size_t width,
                                        size_t height,
                                        int target_fps,
                                        Clock* clock);


  virtual ~ScreenRegionCapture();

  void Start();
  void Stop();
  void ForceFrame();
  void SetFakeRotation(VideoRotation rotation);

  int64_t first_frame_capture_time() const { return first_frame_capture_time_; }

  ScreenRegionCapture(Clock* clock,
                         VideoCaptureInput* input,
                         ScreenCapturer* screen_capture,
                         int target_fps);
  bool Init();
  ScreenCapturer* GetScreenCapture() {return (capturer_.get());}
 private:
  void InsertFrame();
  static bool Run(void* obj);

  Clock* const clock_;
  bool sending_;

  rtc::scoped_ptr<EventTimerWrapper> tick_;
  rtc::CriticalSection lock_;
  rtc::PlatformThread thread_;
  rtc::scoped_ptr<FrameGenerator> frame_generator_;

  int target_fps_;
  VideoRotation fake_rotation_ = kVideoRotation_0;

  int64_t first_frame_capture_time_;
private:
  void OnCaptureCompleted(DesktopFrame* frame) override;
  std::unique_ptr<DesktopFrame> frame_;
  std::unique_ptr<ScreenCapturer> capturer_;
  VideoCaptureInput* input_;


};
}  // test
}  // webrtc

#endif  // WEBRTC_TEST_FRAME_GENERATOR_CAPTURER_H_
