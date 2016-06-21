/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include <map>
#include <set>
#include <sys/types.h>
#include <sys/socket.h>
#include<pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "webrtc/common_types.h"
#include "webrtc/modules/rtp_rtcp/include/rtp_header_parser.h"
#include "webrtc/modules/rtp_rtcp/include/rtp_rtcp_defines.h"
#include "webrtc/modules/rtp_rtcp/source/rtcp_packet.h"
#include "webrtc/modules/rtp_rtcp/source/rtcp_packet/nack.h"
#include "webrtc/modules/rtp_rtcp/source/rtp_rtcp_impl.h"
#include "webrtc/test/rtcp_packet_parser.h"

#include "webrtc/system_wrappers/include/logging.h"



namespace webrtc {

#define ASSERT_FALSE(cond) \
    do { \
        if ((cond)) { \
            printf("abort at %s:%d \n", __FUNCTION__, __LINE__); \
            abort(); \
        } \
    }while(0)

#define ASSERT_TRUE(cond) ASSERT_FALSE(!(cond))
#define ASSERT_LT(x, y) ASSERT_TRUE((x) < (y))
#define ASSERT_EQ(x, y) ASSERT_TRUE((x) == (y))
#define ASSERT_NE(x, y) ASSERT_TRUE((x) != (y))
#define ASSERT_GT(x, y) ASSERT_TRUE((x) > (y))

#define EXPECT_FALSE(cond) \
    do { \
        if ((cond)) { \
            printf("return at %s:%d \n", __FUNCTION__, __LINE__); \
            exit(-1); \
        } \
    }while(0)

#define EXPECT_TRUE(cond) EXPECT_FALSE(!(cond))
#define EXPECT_LT(x, y) EXPECT_TRUE((x) < (y))
#define EXPECT_EQ(x, y) EXPECT_TRUE((x) == (y))
#define EXPECT_NE(x, y) EXPECT_TRUE((x) != (y))
#define EXPECT_GT(x, y) EXPECT_TRUE((x) > (y))

namespace {
const uint32_t kSenderSsrc = 0x12345;
const uint32_t kReceiverSsrc = 0x23456;
const int64_t kOneWayNetworkDelayMs = 100;
const uint8_t kBaseLayerTid = 0;
const uint8_t kHigherLayerTid = 1;
const uint16_t kSequenceNumber = 100;
const uint32_t KreadUnitLen = 1024;


const int32_t kH264StartCode = 0x1000000;

typedef void (*frame_proc_cb_)(char* data, int len);

void frame_proc_cb(char *data, int len) {
    printf("len %d\n", len);
    FILE* fp = fopen("re.h264", "a+");
    fwrite(data, 1, len, fp);
    fclose(fp);
}

int h264_scanner(const char *filename, frame_proc_cb_ callback)
{
    FILE *fp = fopen(filename, "r");
    int readBytes;
    char UnitBuf[KreadUnitLen+3] = {'b','g','n',};
    char *readBuf = UnitBuf+3;
    if(fp == NULL) {
        printf("open file failure!\n");
        return -1;
    }
    char frameBuf[102400] = {0};
    int byteIdx = 0;
    int frame_end_pos = -1, frame_start_pos= -1;
    int32_t ruler;
    int loop =KreadUnitLen;
    int loop_cnt = 0;
    int i;
    bool got_frame = false;
    do {
        readBytes = fread(readBuf, 1, KreadUnitLen, fp);
        if (readBytes < KreadUnitLen) {
            loop = readBytes;
        } else {
            KreadUnitLen - 4 +1;
        }

        for(i = 0; i < loop; i++) {
            ruler = *(int32_t*)(UnitBuf+i);
            if (ruler == kH264StartCode) {
                frame_end_pos = loop_cnt*KreadUnitLen+i;
                frameBuf[byteIdx] = UnitBuf[i];
                if (frame_start_pos != -1) {
                    if ((UnitBuf[i+4] != 0x41)) {//0x67 ) || (UnitBuf[i+4] == 0x68 )) {
                        byteIdx++;
                        continue;
                    }
                    if (callback != NULL) {
                        callback(frameBuf , byteIdx);
                    }


                    frame_start_pos = frame_end_pos;
                    got_frame = 1;
                } else {
                    frame_start_pos = frame_end_pos;
                }
            }

            if (got_frame) {
                byteIdx = 0;
                got_frame = false;
            }

            if (frame_start_pos != -1) {
                frameBuf[byteIdx++] = UnitBuf[i];
            }
        }
        UnitBuf[0] = readBuf[readBytes-3];
        UnitBuf[1] = readBuf[readBytes-2];
        UnitBuf[2] = readBuf[readBytes-1];
        ++loop_cnt;
    } while (readBytes >= KreadUnitLen);
    fclose(fp);
    fp = NULL;
    return 0;
}
class H264FileScanner{

};
class RtcpRttStatsTestImpl : public RtcpRttStats {
 public:
  RtcpRttStatsTestImpl() : rtt_ms_(0) {}
  virtual ~RtcpRttStatsTestImpl() {}

  void OnRttUpdate(int64_t rtt_ms) override { rtt_ms_ = rtt_ms; }
  int64_t LastProcessedRtt() const override { return rtt_ms_; }
  int64_t rtt_ms_;
};

class SendTransport : public Transport,
                      public NullRtpData {
 public:
  SendTransport()
      : receiver_(NULL),
        clock_(NULL),
        delay_ms_(0),
        rtp_packets_sent_(0),
        send_count_(0) {

      initSocket();
  }

  void SetRtpRtcpModule(ModuleRtpRtcpImpl* receiver) {
    receiver_ = receiver;
  }
  void SimulateNetworkDelay(int64_t delay_ms, SimulatedClock* clock) {
    clock_ = clock;
    delay_ms_ = delay_ms;
  }
  bool SendRtp(const uint8_t* data,
               size_t len,
               const PacketOptions& options) override {


      sendto(send_socket_, data, len, 0, (struct sockaddr *)&send_addr_, sizeof(send_addr_));
    return true;
  }
  bool SendRtcp(const uint8_t* data, size_t len) override {
      //printf("%s %s %d\n", __FILE__,__FUNCTION__, __LINE__);
    EXPECT_EQ(0, receiver_->IncomingRtcpPacket(
        static_cast<const uint8_t*>(data), len));
    return true;
  }
  int initSocket() {
      send_socket_ = socket(AF_INET, SOCK_DGRAM, 0);
      if(send_socket_ < 0) {
          printf("init socket error!\n");
      }
      send_addr_.sin_family = AF_INET;
      send_addr_.sin_port = htons(8999);
      send_addr_.sin_addr.s_addr = inet_addr("192.168.1.1");
      return 0;
  }
  ModuleRtpRtcpImpl* receiver_;
  SimulatedClock* clock_;
  int64_t delay_ms_;
  int rtp_packets_sent_;
  RTPHeader last_rtp_header_;
  int send_count_;
  int send_socket_;
  struct sockaddr_in send_addr_;

  std::vector<uint16_t> last_nack_list_;
};

class RtpRtcpModule : public RtcpPacketTypeCounterObserver {
 public:
  explicit RtpRtcpModule(SimulatedClock* clock)
      : receive_statistics_(ReceiveStatistics::Create(clock)) {
    RtpRtcp::Configuration config;
    config.audio = false;
    config.clock = clock;
    config.outgoing_transport = &transport_;
    config.receive_statistics = receive_statistics_.get();
    config.rtcp_packet_type_counter_observer = this;
    config.rtt_stats = &rtt_stats_;

    impl_.reset(new ModuleRtpRtcpImpl(config));
    impl_->SetRTCPStatus(RtcpMode::kCompound);

    transport_.SimulateNetworkDelay(kOneWayNetworkDelayMs, clock);
  }

  RtcpPacketTypeCounter packets_sent_;
  RtcpPacketTypeCounter packets_received_;
  rtc::scoped_ptr<ReceiveStatistics> receive_statistics_;
  SendTransport transport_;
  RtcpRttStatsTestImpl rtt_stats_;
  rtc::scoped_ptr<ModuleRtpRtcpImpl> impl_;
  uint32_t remote_ssrc_;

  void SetRemoteSsrc(uint32_t ssrc) {
    remote_ssrc_ = ssrc;
    impl_->SetRemoteSSRC(ssrc);
  }

  void RtcpPacketTypesCounterUpdated(
      uint32_t ssrc,
      const RtcpPacketTypeCounter& packet_counter) override {
    counter_map_[ssrc] = packet_counter;
  }

  RtcpPacketTypeCounter RtcpSent() {
    // RTCP counters for remote SSRC.
    return counter_map_[remote_ssrc_];
  }

  RtcpPacketTypeCounter RtcpReceived() {
    // Received RTCP stats for (own) local SSRC.
    return counter_map_[impl_->SSRC()];
  }
  int RtpSent() {
    return transport_.rtp_packets_sent_;
  }
  uint16_t LastRtpSequenceNumber() {
    return transport_.last_rtp_header_.sequenceNumber;
  }
  std::vector<uint16_t> LastNackListSent() {
    return transport_.last_nack_list_;
  }

 private:
  std::map<uint32_t, RtcpPacketTypeCounter> counter_map_;
};
}  // namespace

class RtpRtcpImplTest {
 public:
  RtpRtcpImplTest()
      : clock_(133590000000000),
        sender_(&clock_),
        receiver_(&clock_) {
    // Send module.
    EXPECT_EQ(0, sender_.impl_->SetSendingStatus(true));
    sender_.impl_->SetSendingMediaStatus(true);
    sender_.impl_->SetSSRC(kSenderSsrc);
    sender_.SetRemoteSsrc(kReceiverSsrc);
    sender_.impl_->SetSequenceNumber(kSequenceNumber);
    sender_.impl_->SetStorePacketsStatus(true, 100);

    FecProtectionParams delta_fec_params;
    FecProtectionParams key_fec_params;
    key_fec_params.fec_rate = 255;
    delta_fec_params.fec_rate = 255;


    // The RTP module currently requires the same |max_fec_frames| for both
    // key and delta frames.
    delta_fec_params.max_fec_frames = 1;
    key_fec_params.max_fec_frames = 1;

    // Set the FEC packet mask type. |kFecMaskBursty| is more effective for
    // consecutive losses and little/no packet re-ordering. As we currently
    // do not have feedback data on the degree of correlated losses and packet
    // re-ordering, we keep default setting to |kFecMaskRandom| for now.
    delta_fec_params.fec_mask_type = kFecMaskRandom;
    key_fec_params.fec_mask_type = kFecMaskRandom;

    sender_.impl_->SetGenericFECStatus(true, 127, 126);
    sender_.impl_->SetFecParameters(&key_fec_params, &delta_fec_params);

#if 1
    memset(&codec_, 0, sizeof(VideoCodec));
    codec_.plType = 99;
    strncpy(codec_.plName, "H264", 4);
    codec_.width = 640;
    codec_.height = 480;
#else
    memset(&codec_, 0, sizeof(VideoCodec));
    codec_.plType = 100;
    strncpy(codec_.plName, "vp8", 3);
    codec_.width = 640;
    codec_.height = 480;
#endif
    EXPECT_EQ(0, sender_.impl_->RegisterSendPayload(codec_));

    // Receive module.
    EXPECT_EQ(0, receiver_.impl_->SetSendingStatus(false));
    receiver_.impl_->SetSendingMediaStatus(false);
    receiver_.impl_->SetSSRC(kReceiverSsrc);
    receiver_.SetRemoteSsrc(kSenderSsrc);
    // Transport settings.
    sender_.transport_.SetRtpRtcpModule(receiver_.impl_.get());
    receiver_.transport_.SetRtpRtcpModule(sender_.impl_.get());
  }
  SimulatedClock clock_;
  RtpRtcpModule sender_;
  RtpRtcpModule receiver_;
  VideoCodec codec_;

  void SendFrame(const RtpRtcpModule* module, RTPFragmentationHeader &frag, uint8_t *buffer, int32_t len, int32_t ts) {
    RTPVideoHeaderH264 h264_header;
    h264_header.nalu_type = 0x5;
    h264_header.packetization_type = kH264SingleNalu;


    RTPVideoHeader rtp_video_header;

    rtp_video_header.width =codec_.width;
    rtp_video_header.height =codec_.height;
    rtp_video_header.rotation =kVideoRotation_0;
    rtp_video_header.isFirstPacket =true;
    rtp_video_header.simulcastIdx =0;
    rtp_video_header.codec =kRtpVideoH264;
    rtp_video_header.codecHeader.H264.nalu_type =buffer[4]& 0x1f;
    rtp_video_header.codecHeader.H264.packetization_type =kH264FuA;

//    RTPFragmentationHeader frag;
//    frag.VerifyAndAllocateFragmentationHeader(1);
//    frag.fragmentationLength[0] = len;
//    frag.fragmentationOffset[0] = 0;
//    frag.fragmentationVectorSize = 1;
    EXPECT_EQ(0, module->impl_->SendOutgoingData(kVideoFrameKey,
                                                 codec_.plType,
                                                 ts,
                                                 0,
                                                 buffer,
                                                 len,
                                                 &frag,
                                                 &rtp_video_header));

  }


  void SendFrame(const RtpRtcpModule* module, uint8_t tid) {
      printf("CCCCCCCCCCCCC\n");
    RTPVideoHeaderVP8 vp8_header = {};
    vp8_header.temporalIdx = tid;
    RTPVideoHeader rtp_video_header = {codec_.width,
                                       codec_.height,
                                       kVideoRotation_0,
                                       true,
                                       0,
                                       kRtpVideoVp8,
                                       {vp8_header}};

    uint8_t payload[500000] = {0};
    for(int i = 0; i <50000; i++ ) {
        for(int j = 0; j < 10; j++) {
            payload[i*10 +j] = j;
        }
    }
    EXPECT_EQ(0, module->impl_->SendOutgoingData(kVideoFrameKey,
                                                 codec_.plType,
                                                 0,
                                                 0,
                                                 payload,
                                                 sizeof(payload),
                                                 NULL,
                                                 &rtp_video_header));
  }



  void IncomingRtcpNack(const RtpRtcpModule* module, uint16_t sequence_number) {
    bool sender = module->impl_->SSRC() == kSenderSsrc;
    rtcp::Nack nack;
    uint16_t list[1];
    list[0] = sequence_number;
    const uint16_t kListLength = sizeof(list) / sizeof(list[0]);
    nack.From(sender ? kReceiverSsrc : kSenderSsrc);
    nack.To(sender ? kSenderSsrc : kReceiverSsrc);
    nack.WithList(list, kListLength);
    rtc::Buffer packet = nack.Build();
    EXPECT_EQ(0, module->impl_->IncomingRtcpPacket(packet.data(),
                                                   packet.size()));
  }

  std::list<std::string> frame_buffer_list;
  int h264_scanner(const char *filename, frame_proc_cb_ callback) {
      FILE *fp = fopen(filename, "r");
      int readBytes;
      char UnitBuf[KreadUnitLen+3] = {'b','g','n',};
      char *readBuf = UnitBuf+3;
      if(fp == NULL) {
          printf("open file failure!\n");
          return -1;
      }
      char frameBuf[102400] = {0};
      int byteIdx = 0;
      int frame_end_pos = -1, frame_start_pos= -1;
      int32_t ruler;
      int loop =KreadUnitLen;
      int loop_cnt = 0;
      int i;
      bool got_frame = false;
      do {
          readBytes = fread(readBuf, 1, KreadUnitLen, fp);
          if (readBytes < KreadUnitLen) {
              loop = readBytes;
          } else {
              KreadUnitLen - 4 +1;
          }

          for(i = 0; i < loop; i++) {
              ruler = *(int32_t*)(UnitBuf+i);
              if (ruler == kH264StartCode) {
                  frame_end_pos = loop_cnt*KreadUnitLen+i;
                  frameBuf[byteIdx] = UnitBuf[i];
                  if (frame_start_pos != -1) {
//                      if ((UnitBuf[i+4] != 0x41)) {// ) || (UnitBuf[i+4] == 0x68 )) {//TODO
//                          byteIdx++;
//                          continue;
//                      }
                      if (callback != NULL) {
                          callback(frameBuf , byteIdx);
                      }
                      printf("byteIdx %d %d %d\n", byteIdx, frame_start_pos, frame_end_pos);
                      std::string frame_string_ = std::string(frameBuf, byteIdx);

                      frame_buffer_list.push_back(frame_string_);
                      frame_start_pos = frame_end_pos;
                      got_frame = 1;
                  } else {
                      frame_start_pos = frame_end_pos;
                  }
              }

              if (got_frame) {
                  byteIdx = 0;
                  got_frame = false;
              }

              if (frame_start_pos != -1) {
                  frameBuf[byteIdx++] = UnitBuf[i];
              }
          }
          UnitBuf[0] = readBuf[readBytes-3];
          UnitBuf[1] = readBuf[readBytes-2];
          UnitBuf[2] = readBuf[readBytes-1];
          ++loop_cnt;
      } while (readBytes >= KreadUnitLen);
      fclose(fp);
      fp = NULL;
      return 0;
  }

  bool  getFrame(RTPFragmentationHeader &frag, std::string &frame)
  {
      bool gotted=true;
      std::string nal_buffer;
      while (frame_buffer_list.empty() == false) {
          nal_buffer = *frame_buffer_list.begin();
          frame_buffer_list.erase(frame_buffer_list.begin());
          int nalType = nal_buffer.c_str()[4]&0x1f;
          nal_buffer = nal_buffer.substr(4, nal_buffer.length());
          switch (nalType) {
          case 0x7:
                frag.VerifyAndAllocateFragmentationHeader(3);
                frag.fragmentationLength[0] = nal_buffer.length();
                frag.fragmentationOffset[0] = 0;
                frag.fragmentationVectorSize = 3;
                frame = nal_buffer;
                continue;
          case 0x8:
                frag.fragmentationLength[1] = nal_buffer.length();
                frag.fragmentationOffset[1] = frag.fragmentationLength[0];
                frag.fragmentationVectorSize = 3;
                frame+=nal_buffer;
                continue;
          case 0x5:
                frag.fragmentationLength[2] = nal_buffer.length();
                frag.fragmentationOffset[2] = frag.fragmentationLength[0]+frag.fragmentationLength[1];
                frag.fragmentationVectorSize = 3;
                frame+=nal_buffer;
                gotted = true;
                break;
          case 0x1:
              frag.VerifyAndAllocateFragmentationHeader(1);
              frag.fragmentationLength[0] = nal_buffer.length();
              frag.fragmentationOffset[0] = 0;
              frag.fragmentationVectorSize = 1;
              frame = nal_buffer;
              gotted = true;
               break;
          default:
              printf("unknown naltype\n");
          }
          if (gotted ==true) {
              break;
          }
      }


      return gotted;
  }
  int getTotalFrame(){

      return frame_buffer_list.size();
  }

};


}  // namespace webrtc
using namespace webrtc;
#include <errno.h>
#include <sys/timerfd.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>        /* Definition of uint64_t */
#include <sys/fcntl.h>
#include <iostream>

#include "webrtc/system_wrappers/include/logging.h"


#include "webrtc/base/arraysize.h"
#include "webrtc/base/event.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/system_wrappers/include/trace.h"


namespace webrtc {

const char kTestLogString[] = "Incredibly important test message!(?)";
const int kTestLevel = kTraceWarning;

class LoggingTestCallback : public TraceCallback {
 public:
  LoggingTestCallback(rtc::Event* event) : event_(event) {}

 private:
  void Print(TraceLevel level, const char* msg, int length) override {
      std::cout<<msg<<std::endl;
  }

  rtc::Event* const event_;
};



}  // namespace webrtc

int main()
{
    Trace::CreateTrace();

    rtc::Event event(false, false);
    LoggingTestCallback callback(&event);
    Trace::SetTraceCallback(&callback);
    LOG(LS_WARNING) << kTestLogString;


    RtpRtcpImplTest *instance = new RtpRtcpImplTest();



#if 0
    instance->SendFrame(&instance->sender_, 0);
    return 0;
#endif
#if 1

    instance->h264_scanner("gift.h264", NULL);
    int timerFd = timerfd_create(CLOCK_MONOTONIC, O_NONBLOCK);

    struct itimerspec its;
    its.it_interval.tv_sec = 0;


    its.it_interval.tv_nsec = 66666000;//video mix framerate 15fps
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 10000000;
    if(-1 ==timerfd_settime(timerFd, 0, &its, NULL))
    {
        printf("timerfd_settime error: %d %s\n", errno , strerror(errno));
    }
    int ts = 0;
    RTPFragmentationHeader frag;
    while (1) {

        int fd_num;
        fd_set rs;
        struct timeval waitTime;
        int max_fd;

        waitTime.tv_sec = 1;
        waitTime.tv_usec = 0; //_usec;

        FD_ZERO(&rs);
        FD_SET(timerFd, &rs);

        max_fd = timerFd + 1;

        fd_num = select(max_fd, &rs, NULL, NULL, &waitTime);

        if (FD_ISSET(timerFd, &rs)) {
            if (instance->getTotalFrame() <=0) {
                break;
            }
            std::string frame_buffer_;
            bool gotted = instance->getFrame(frag,frame_buffer_);
            //printf("offset %d %d %d\n", frag.fragmentationOffset[0], frag.fragmentationOffset[1], frag.fragmentationOffset[2]);

            instance->SendFrame(&instance->sender_, frag, (uint8_t *)frame_buffer_.c_str(), frame_buffer_.length(), ts);
            ts += 3600;

        }


    }
#else

    h264_scanner("gift.h264", frame_proc_cb);
#endif
    return 0;
}
