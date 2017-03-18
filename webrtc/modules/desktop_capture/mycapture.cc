/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <unistd.h>
#include <memory>



#include "webrtc/video_frame.h"
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"

#include "webrtc/modules/desktop_capture/desktop_capture_options.h"
#include "webrtc/modules/desktop_capture/desktop_frame.h"
#include "webrtc/modules/desktop_capture/desktop_region.h"
#include "webrtc/system_wrappers/include/logging.h"
#include "webrtc/modules/desktop_capture/screen_capturer.h"

namespace webrtc {

class CapturerCallback : public DesktopCapturer::Callback {
 public:

  // DesktopCapturer::Callback interface
  void OnCaptureCompleted(DesktopFrame* frame) override {
      frame_.reset(frame);
      int width = frame->size().width();
      int height = frame->size().height();
      printf ("w %d h %d\n", width, height);
      VideoFrame video_frame_;
      char *frame_buffer = (char *)malloc(width*height*3/2);
      video_frame_.CreateFrame((const uint8_t*)frame_buffer, width, height, kVideoRotation_0);
      int ret = ConvertToI420(VideoType::kARGB, frame->data(), 0, 0, frame->size().width(),
              frame->size().height(), 0, kVideoRotation_0, &video_frame_);

      FILE *fp = fopen("data.yuv", "a+");
      video_frame_.buffer(kYPlane);
      fwrite(video_frame_.buffer(kYPlane), 1, width*height, fp);
      fwrite(video_frame_.buffer(kUPlane), 1, width*height/2, fp);
      fclose(fp);
  }

 protected:
  std::unique_ptr<DesktopFrame> frame_;
};

}

#include "./screen_region_capture.h"
#include "webrtc/system_wrappers/include/clock.h"

using namespace webrtc;
using namespace test;
int main()
{
#if 1
    ScreenRegionCapture* mycapture_ =
    		ScreenRegionCapture::Create(NULL, 1280,800, 100, Clock::GetRealTimeClock());
    mycapture_->Start();
    while(1){
    	sleep(10);
    }


#else
    std::unique_ptr<ScreenCapturer> capturer_;
    std::unique_ptr<CapturerCallback> callback_;
    callback_.reset(new CapturerCallback());
    capturer_.reset(
            ScreenCapturer::Create(DesktopCaptureOptions::CreateDefault()));
    ScreenCapturer::ScreenList screens;
    capturer_->GetScreenList(&screens);

    for (ScreenCapturer::ScreenList::iterator it = screens.begin();
         it != screens.end(); ++it) {
    }

    capturer_->Start(callback_.get());
    capturer_->SelectScreen(screens[0].id);

    while (1) {
        capturer_->Capture(DesktopRegion());
        sleep(1);
    }
#endif
    return 0;


}
