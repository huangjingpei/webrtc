/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <stdlib.h>
#include <string.h>

//#include "gflags/gflags.h"
#include "webrtc/modules/video_coding/include/video_coding.h"
#include "webrtc/modules/video_coding/test/receiver_tests.h"
#include "webrtc/test/testsupport/fileutils.h"

//DEFINE_string(codec, "VP8", "Codec to use (VP8 or I420).");
//DEFINE_int32(width, 352, "Width in pixels of the frames in the input file.");
//DEFINE_int32(height, 288, "Height in pixels of the frames in the input file.");
//DEFINE_int32(rtt, 0, "RTT (round-trip time), in milliseconds.");
//DEFINE_string(input_filename,
//              webrtc::test::ResourcePath("foreman_cif", "yuv"),
//              "Input file.");
//DEFINE_string(output_filename,
//              webrtc::test::OutputPath() +
//                  "video_coding_test_output_352x288.yuv",
//              "Output file.");

namespace webrtc {

/*
 * Build with EVENT_DEBUG defined
 * to build the tests with simulated events.
 */

int vcmMacrosTests = 0;
int vcmMacrosErrors = 0;

int ParseArguments(CmdArgs* args) {
  args->width = 1920;
  args->height = 1080;
  if (args->width < 1 || args->height < 1) {
    return -1;
  }
  args->codecName = "H264";
  if (args->codecName == "VP8") {
    args->codecType = kVideoCodecVP8;
  } else if (args->codecName == "VP9") {
    args->codecType = kVideoCodecVP9;
  } else if (args->codecName == "I420") {
    args->codecType = kVideoCodecI420;
  } else if ((args->codecName == "H264")) {
      args->codecType = kVideoCodecH264;
  } else {
    printf("Invalid codec: %s\n", args->codecName.c_str());
    return -1;
  }
  printf("args->codecType %d\n",args->codecType);
  args->inputFile = "chrome-vp8-1.pcap";
  args->outputFile = "abc.yuv";
  args->rtt = 2000;
  return 0;
}
}  // namespace webrtc

int main(int argc, char** argv) {
  // Initialize WebRTC fileutils.h so paths to resources can be resolved.

  CmdArgs args;
  if (webrtc::ParseArguments(&args) != 0) {
    printf("Unable to parse input arguments\n");
    return -1;
  }

  printf("Running video coding tests...\n");
  return RtpPlay(args);
}
