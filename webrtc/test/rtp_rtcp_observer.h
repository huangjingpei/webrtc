/*
 *  Copyright (c) 2013 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#ifndef WEBRTC_TEST_RTP_RTCP_OBSERVER_H_
#define WEBRTC_TEST_RTP_RTCP_OBSERVER_H_

#include <map>
#include <vector>

#include "testing/gtest/include/gtest/gtest.h"

#include "webrtc/base/criticalsection.h"
#include "webrtc/base/event.h"
#include "webrtc/modules/rtp_rtcp/include/rtp_header_parser.h"
#include "webrtc/test/constants.h"
#include "webrtc/test/direct_transport.h"
#include "webrtc/typedefs.h"
#include "webrtc/video_send_stream.h"

namespace webrtc {
namespace test {

class PacketTransport;

class RtpRtcpObserver {
 public:
  enum Action {
    SEND_PACKET,
    DROP_PACKET,
  };

  virtual ~RtpRtcpObserver() {}

  virtual bool Wait() { return observation_complete_.Wait(timeout_ms_); }

  virtual Action OnSendRtp(const uint8_t* packet, size_t length) {
    return SEND_PACKET;
  }

  virtual Action OnSendRtcp(const uint8_t* packet, size_t length) {
    return SEND_PACKET;
  }

  virtual Action OnReceiveRtp(const uint8_t* packet, size_t length) {
    return SEND_PACKET;
  }

  virtual Action OnReceiveRtcp(const uint8_t* packet, size_t length) {
    return SEND_PACKET;
  }

 protected:
  explicit RtpRtcpObserver(int event_timeout_ms)
      : observation_complete_(false, false),
        parser_(RtpHeaderParser::Create()),
        timeout_ms_(event_timeout_ms) {
    parser_->RegisterRtpHeaderExtension(kRtpExtensionTransmissionTimeOffset,
                                        kTOffsetExtensionId);
    parser_->RegisterRtpHeaderExtension(kRtpExtensionAbsoluteSendTime,
                                        kAbsSendTimeExtensionId);
    parser_->RegisterRtpHeaderExtension(kRtpExtensionTransportSequenceNumber,
                                        kTransportSequenceNumberExtensionId);
  }

  rtc::Event observation_complete_;
  const rtc::scoped_ptr<RtpHeaderParser> parser_;

 private:
  const int timeout_ms_;
};

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
//#define WITH_TRANSNET
class PacketTransport : public test::DirectTransport {
 public:
  enum TransportType { kReceiver, kSender };

  PacketTransport(Call* send_call,
                  RtpRtcpObserver* observer,
                  TransportType transport_type,
                  const FakeNetworkPipe::Config& configuration)
      : test::DirectTransport(configuration, send_call),
        observer_(observer),
        transport_type_(transport_type) {

        char buf[BUFSIZ];  //数据传送的缓冲区
        memset(&remote_rtp_addr_,0,sizeof(remote_rtp_addr_)); //数据初始化--清零
        remote_rtp_addr_.sin_family=AF_INET; //设置为IP通信
        remote_rtp_addr_.sin_addr.s_addr=inet_addr("172.172.172.198");//服务器IP地址
        remote_rtp_addr_.sin_port=htons(9696); //服务器端口号
        memset(&remote_rtcp_addr_,0,sizeof(remote_rtcp_addr_)); //数据初始化--清零
        remote_rtcp_addr_.sin_family=AF_INET; //设置为IP通信
        remote_rtcp_addr_.sin_addr.s_addr=inet_addr("172.172.172.198");//服务器IP地址
        remote_rtcp_addr_.sin_port=htons(9697); //服务器端口号

        if((transport_rtp_fd_=socket(AF_INET,SOCK_DGRAM,0))<0)
        {
            printf("transport_fd_ %d\n", transport_rtp_fd_);
        }

        if((transport_rtcp_fd_=socket(AF_INET,SOCK_DGRAM,0))<0)
        {
            printf("transport_fd_ %d\n", transport_rtcp_fd_);
        }

        lost_seq_num_ = 0;


  }

 private:
  bool SendRtp(const uint8_t* packet,
               size_t length,
               const PacketOptions& options) override {
    EXPECT_FALSE(RtpHeaderParser::IsRtcp(packet, length));
    RtpRtcpObserver::Action action;
    {
      if (transport_type_ == kSender) {
        action = observer_->OnSendRtp(packet, length);
      } else {
        action = observer_->OnReceiveRtp(packet, length);
      }
    }
    switch (action) {
      case RtpRtcpObserver::DROP_PACKET:
        // Drop packet silently.
        return true;
      case RtpRtcpObserver::SEND_PACKET:
#ifdef WITH_TRANSNET

          if (++lost_seq_num_ %100 == 0) {
              return true;
          }
          if((length=sendto(transport_rtp_fd_,packet,length,0,(struct sockaddr *)&remote_rtp_addr_,sizeof(struct sockaddr)))<0) {

          }
#else
        return test::DirectTransport::SendRtp(packet, length, options);
#endif
    }
    return true;  // Will never happen, makes compiler happy.
  }

  bool SendRtcp(const uint8_t* packet, size_t length) override {
      if (++lost_seq_num_ %500 == 0) {
          return true;
      }
    EXPECT_TRUE(RtpHeaderParser::IsRtcp(packet, length));
    RtpRtcpObserver::Action action;
    {
      if (transport_type_ == kSender) {
        action = observer_->OnSendRtcp(packet, length);
      } else {
        action = observer_->OnReceiveRtcp(packet, length);
      }
    }
    switch (action) {
      case RtpRtcpObserver::DROP_PACKET:
        // Drop packet silently.
        return true;
      case RtpRtcpObserver::SEND_PACKET:
#ifdef WITH_TRANSNET
          if((length=sendto(transport_rtcp_fd_,packet,length,0,(struct sockaddr *)&remote_rtcp_addr_,sizeof(struct sockaddr)))<0) {

          }
#else
        return test::DirectTransport::SendRtcp(packet, length);
#endif
    }
    return true;  // Will never happen, makes compiler happy.
  }

  RtpRtcpObserver* const observer_;
  TransportType transport_type_;


  struct sockaddr_in remote_rtp_addr_;
  struct sockaddr_in remote_rtcp_addr_;

  int transport_rtp_fd_;
  int transport_rtcp_fd_;

  int lost_seq_num_;

};
}  // namespace test
}  // namespace webrtc

#endif  // WEBRTC_TEST_RTP_RTCP_OBSERVER_H_
