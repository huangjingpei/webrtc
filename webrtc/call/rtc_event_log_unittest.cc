/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifdef ENABLE_RTC_EVENT_LOG

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "testing/gtest/include/gtest/gtest.h"
#include "webrtc/base/buffer.h"
#include "webrtc/base/checks.h"
#include "webrtc/base/random.h"
#include "webrtc/call.h"
#include "webrtc/call/rtc_event_log.h"
#include "webrtc/modules/rtp_rtcp/source/rtcp_packet.h"
#include "webrtc/modules/rtp_rtcp/source/rtcp_packet/sender_report.h"
#include "webrtc/modules/rtp_rtcp/source/rtp_sender.h"
#include "webrtc/system_wrappers/include/clock.h"
#include "webrtc/test/test_suite.h"
#include "webrtc/test/testsupport/fileutils.h"

// Files generated at build-time by the protobuf compiler.
#ifdef WEBRTC_ANDROID_PLATFORM_BUILD
#include "external/webrtc/webrtc/call/rtc_event_log.pb.h"
#else
#include "webrtc/call/rtc_event_log.pb.h"
#endif

namespace webrtc {

namespace {

const RTPExtensionType kExtensionTypes[] = {
    RTPExtensionType::kRtpExtensionTransmissionTimeOffset,
    RTPExtensionType::kRtpExtensionAudioLevel,
    RTPExtensionType::kRtpExtensionAbsoluteSendTime,
    RTPExtensionType::kRtpExtensionVideoRotation,
    RTPExtensionType::kRtpExtensionTransportSequenceNumber};
const char* kExtensionNames[] = {RtpExtension::kTOffset,
                                 RtpExtension::kAudioLevel,
                                 RtpExtension::kAbsSendTime,
                                 RtpExtension::kVideoRotation,
                                 RtpExtension::kTransportSequenceNumber};
const size_t kNumExtensions = 5;

}  // namespace

// TODO(terelius): Place this definition with other parsing functions?
MediaType GetRuntimeMediaType(rtclog::MediaType media_type) {
  switch (media_type) {
    case rtclog::MediaType::ANY:
      return MediaType::ANY;
    case rtclog::MediaType::AUDIO:
      return MediaType::AUDIO;
    case rtclog::MediaType::VIDEO:
      return MediaType::VIDEO;
    case rtclog::MediaType::DATA:
      return MediaType::DATA;
  }
  RTC_NOTREACHED();
  return MediaType::ANY;
}

// Checks that the event has a timestamp, a type and exactly the data field
// corresponding to the type.
::testing::AssertionResult IsValidBasicEvent(const rtclog::Event& event) {
  if (!event.has_timestamp_us())
    return ::testing::AssertionFailure() << "Event has no timestamp";
  if (!event.has_type())
    return ::testing::AssertionFailure() << "Event has no event type";
  rtclog::Event_EventType type = event.type();
  if ((type == rtclog::Event::RTP_EVENT) != event.has_rtp_packet())
    return ::testing::AssertionFailure()
           << "Event of type " << type << " has "
           << (event.has_rtp_packet() ? "" : "no ") << "RTP packet";
  if ((type == rtclog::Event::RTCP_EVENT) != event.has_rtcp_packet())
    return ::testing::AssertionFailure()
           << "Event of type " << type << " has "
           << (event.has_rtcp_packet() ? "" : "no ") << "RTCP packet";
  if ((type == rtclog::Event::AUDIO_PLAYOUT_EVENT) !=
      event.has_audio_playout_event())
    return ::testing::AssertionFailure()
           << "Event of type " << type << " has "
           << (event.has_audio_playout_event() ? "" : "no ")
           << "audio_playout event";
  if ((type == rtclog::Event::VIDEO_RECEIVER_CONFIG_EVENT) !=
      event.has_video_receiver_config())
    return ::testing::AssertionFailure()
           << "Event of type " << type << " has "
           << (event.has_video_receiver_config() ? "" : "no ")
           << "receiver config";
  if ((type == rtclog::Event::VIDEO_SENDER_CONFIG_EVENT) !=
      event.has_video_sender_config())
    return ::testing::AssertionFailure()
           << "Event of type " << type << " has "
           << (event.has_video_sender_config() ? "" : "no ") << "sender config";
  if ((type == rtclog::Event::AUDIO_RECEIVER_CONFIG_EVENT) !=
      event.has_audio_receiver_config()) {
    return ::testing::AssertionFailure()
           << "Event of type " << type << " has "
           << (event.has_audio_receiver_config() ? "" : "no ")
           << "audio receiver config";
  }
  if ((type == rtclog::Event::AUDIO_SENDER_CONFIG_EVENT) !=
      event.has_audio_sender_config()) {
    return ::testing::AssertionFailure()
           << "Event of type " << type << " has "
           << (event.has_audio_sender_config() ? "" : "no ")
           << "audio sender config";
  }
  return ::testing::AssertionSuccess();
}

void VerifyReceiveStreamConfig(const rtclog::Event& event,
                               const VideoReceiveStream::Config& config) {
  ASSERT_TRUE(IsValidBasicEvent(event));
  ASSERT_EQ(rtclog::Event::VIDEO_RECEIVER_CONFIG_EVENT, event.type());
  const rtclog::VideoReceiveConfig& receiver_config =
      event.video_receiver_config();
  // Check SSRCs.
  ASSERT_TRUE(receiver_config.has_remote_ssrc());
  EXPECT_EQ(config.rtp.remote_ssrc, receiver_config.remote_ssrc());
  ASSERT_TRUE(receiver_config.has_local_ssrc());
  EXPECT_EQ(config.rtp.local_ssrc, receiver_config.local_ssrc());
  // Check RTCP settings.
  ASSERT_TRUE(receiver_config.has_rtcp_mode());
  if (config.rtp.rtcp_mode == RtcpMode::kCompound)
    EXPECT_EQ(rtclog::VideoReceiveConfig::RTCP_COMPOUND,
              receiver_config.rtcp_mode());
  else
    EXPECT_EQ(rtclog::VideoReceiveConfig::RTCP_REDUCEDSIZE,
              receiver_config.rtcp_mode());
  ASSERT_TRUE(receiver_config.has_remb());
  EXPECT_EQ(config.rtp.remb, receiver_config.remb());
  // Check RTX map.
  ASSERT_EQ(static_cast<int>(config.rtp.rtx.size()),
            receiver_config.rtx_map_size());
  for (const rtclog::RtxMap& rtx_map : receiver_config.rtx_map()) {
    ASSERT_TRUE(rtx_map.has_payload_type());
    ASSERT_TRUE(rtx_map.has_config());
    EXPECT_EQ(1u, config.rtp.rtx.count(rtx_map.payload_type()));
    const rtclog::RtxConfig& rtx_config = rtx_map.config();
    const VideoReceiveStream::Config::Rtp::Rtx& rtx =
        config.rtp.rtx.at(rtx_map.payload_type());
    ASSERT_TRUE(rtx_config.has_rtx_ssrc());
    ASSERT_TRUE(rtx_config.has_rtx_payload_type());
    EXPECT_EQ(rtx.ssrc, rtx_config.rtx_ssrc());
    EXPECT_EQ(rtx.payload_type, rtx_config.rtx_payload_type());
  }
  // Check header extensions.
  ASSERT_EQ(static_cast<int>(config.rtp.extensions.size()),
            receiver_config.header_extensions_size());
  for (int i = 0; i < receiver_config.header_extensions_size(); i++) {
    ASSERT_TRUE(receiver_config.header_extensions(i).has_name());
    ASSERT_TRUE(receiver_config.header_extensions(i).has_id());
    const std::string& name = receiver_config.header_extensions(i).name();
    int id = receiver_config.header_extensions(i).id();
    EXPECT_EQ(config.rtp.extensions[i].id, id);
    EXPECT_EQ(config.rtp.extensions[i].name, name);
  }
  // Check decoders.
  ASSERT_EQ(static_cast<int>(config.decoders.size()),
            receiver_config.decoders_size());
  for (int i = 0; i < receiver_config.decoders_size(); i++) {
    ASSERT_TRUE(receiver_config.decoders(i).has_name());
    ASSERT_TRUE(receiver_config.decoders(i).has_payload_type());
    const std::string& decoder_name = receiver_config.decoders(i).name();
    int decoder_type = receiver_config.decoders(i).payload_type();
    EXPECT_EQ(config.decoders[i].payload_name, decoder_name);
    EXPECT_EQ(config.decoders[i].payload_type, decoder_type);
  }
}

void VerifySendStreamConfig(const rtclog::Event& event,
                            const VideoSendStream::Config& config) {
  ASSERT_TRUE(IsValidBasicEvent(event));
  ASSERT_EQ(rtclog::Event::VIDEO_SENDER_CONFIG_EVENT, event.type());
  const rtclog::VideoSendConfig& sender_config = event.video_sender_config();
  // Check SSRCs.
  ASSERT_EQ(static_cast<int>(config.rtp.ssrcs.size()),
            sender_config.ssrcs_size());
  for (int i = 0; i < sender_config.ssrcs_size(); i++) {
    EXPECT_EQ(config.rtp.ssrcs[i], sender_config.ssrcs(i));
  }
  // Check header extensions.
  ASSERT_EQ(static_cast<int>(config.rtp.extensions.size()),
            sender_config.header_extensions_size());
  for (int i = 0; i < sender_config.header_extensions_size(); i++) {
    ASSERT_TRUE(sender_config.header_extensions(i).has_name());
    ASSERT_TRUE(sender_config.header_extensions(i).has_id());
    const std::string& name = sender_config.header_extensions(i).name();
    int id = sender_config.header_extensions(i).id();
    EXPECT_EQ(config.rtp.extensions[i].id, id);
    EXPECT_EQ(config.rtp.extensions[i].name, name);
  }
  // Check RTX settings.
  ASSERT_EQ(static_cast<int>(config.rtp.rtx.ssrcs.size()),
            sender_config.rtx_ssrcs_size());
  for (int i = 0; i < sender_config.rtx_ssrcs_size(); i++) {
    EXPECT_EQ(config.rtp.rtx.ssrcs[i], sender_config.rtx_ssrcs(i));
  }
  if (sender_config.rtx_ssrcs_size() > 0) {
    ASSERT_TRUE(sender_config.has_rtx_payload_type());
    EXPECT_EQ(config.rtp.rtx.payload_type, sender_config.rtx_payload_type());
  }
  // Check encoder.
  ASSERT_TRUE(sender_config.has_encoder());
  ASSERT_TRUE(sender_config.encoder().has_name());
  ASSERT_TRUE(sender_config.encoder().has_payload_type());
  EXPECT_EQ(config.encoder_settings.payload_name,
            sender_config.encoder().name());
  EXPECT_EQ(config.encoder_settings.payload_type,
            sender_config.encoder().payload_type());
}

void VerifyRtpEvent(const rtclog::Event& event,
                    PacketDirection direction,
                    MediaType media_type,
                    const uint8_t* header,
                    size_t header_size,
                    size_t total_size) {
  ASSERT_TRUE(IsValidBasicEvent(event));
  ASSERT_EQ(rtclog::Event::RTP_EVENT, event.type());
  const rtclog::RtpPacket& rtp_packet = event.rtp_packet();
  ASSERT_TRUE(rtp_packet.has_incoming());
  EXPECT_EQ(direction == kIncomingPacket, rtp_packet.incoming());
  ASSERT_TRUE(rtp_packet.has_type());
  EXPECT_EQ(media_type, GetRuntimeMediaType(rtp_packet.type()));
  ASSERT_TRUE(rtp_packet.has_packet_length());
  EXPECT_EQ(total_size, rtp_packet.packet_length());
  ASSERT_TRUE(rtp_packet.has_header());
  ASSERT_EQ(header_size, rtp_packet.header().size());
  for (size_t i = 0; i < header_size; i++) {
    EXPECT_EQ(header[i], static_cast<uint8_t>(rtp_packet.header()[i]));
  }
}

void VerifyRtcpEvent(const rtclog::Event& event,
                     PacketDirection direction,
                     MediaType media_type,
                     const uint8_t* packet,
                     size_t total_size) {
  ASSERT_TRUE(IsValidBasicEvent(event));
  ASSERT_EQ(rtclog::Event::RTCP_EVENT, event.type());
  const rtclog::RtcpPacket& rtcp_packet = event.rtcp_packet();
  ASSERT_TRUE(rtcp_packet.has_incoming());
  EXPECT_EQ(direction == kIncomingPacket, rtcp_packet.incoming());
  ASSERT_TRUE(rtcp_packet.has_type());
  EXPECT_EQ(media_type, GetRuntimeMediaType(rtcp_packet.type()));
  ASSERT_TRUE(rtcp_packet.has_packet_data());
  ASSERT_EQ(total_size, rtcp_packet.packet_data().size());
  for (size_t i = 0; i < total_size; i++) {
    EXPECT_EQ(packet[i], static_cast<uint8_t>(rtcp_packet.packet_data()[i]));
  }
}

void VerifyPlayoutEvent(const rtclog::Event& event, uint32_t ssrc) {
  ASSERT_TRUE(IsValidBasicEvent(event));
  ASSERT_EQ(rtclog::Event::AUDIO_PLAYOUT_EVENT, event.type());
  const rtclog::AudioPlayoutEvent& playout_event = event.audio_playout_event();
  ASSERT_TRUE(playout_event.has_local_ssrc());
  EXPECT_EQ(ssrc, playout_event.local_ssrc());
}

void VerifyBweLossEvent(const rtclog::Event& event,
                        int32_t bitrate,
                        uint8_t fraction_loss,
                        int32_t total_packets) {
  ASSERT_TRUE(IsValidBasicEvent(event));
  ASSERT_EQ(rtclog::Event::BWE_PACKET_LOSS_EVENT, event.type());
  const rtclog::BwePacketLossEvent& bwe_event = event.bwe_packet_loss_event();
  ASSERT_TRUE(bwe_event.has_bitrate());
  EXPECT_EQ(bitrate, bwe_event.bitrate());
  ASSERT_TRUE(bwe_event.has_fraction_loss());
  EXPECT_EQ(fraction_loss, bwe_event.fraction_loss());
  ASSERT_TRUE(bwe_event.has_total_packets());
  EXPECT_EQ(total_packets, bwe_event.total_packets());
}

void VerifyLogStartEvent(const rtclog::Event& event) {
  ASSERT_TRUE(IsValidBasicEvent(event));
  EXPECT_EQ(rtclog::Event::LOG_START, event.type());
}

void VerifyLogEndEvent(const rtclog::Event& event) {
  ASSERT_TRUE(IsValidBasicEvent(event));
  EXPECT_EQ(rtclog::Event::LOG_END, event.type());
}

/*
 * Bit number i of extension_bitvector is set to indicate the
 * presence of extension number i from kExtensionTypes / kExtensionNames.
 * The least significant bit extension_bitvector has number 0.
 */
size_t GenerateRtpPacket(uint32_t extensions_bitvector,
                         uint32_t csrcs_count,
                         uint8_t* packet,
                         size_t packet_size,
                         Random* prng) {
  RTC_CHECK_GE(packet_size, 16 + 4 * csrcs_count + 4 * kNumExtensions);
  Clock* clock = Clock::GetRealTimeClock();

  RTPSender rtp_sender(false,     // bool audio
                       clock,     // Clock* clock
                       nullptr,   // Transport*
                       nullptr,   // PacedSender*
                       nullptr,   // PacketRouter*
                       nullptr,   // SendTimeObserver*
                       nullptr,   // BitrateStatisticsObserver*
                       nullptr,   // FrameCountObserver*
                       nullptr,   // SendSideDelayObserver*
                       nullptr);  // RtcEventLog*

  std::vector<uint32_t> csrcs;
  for (unsigned i = 0; i < csrcs_count; i++) {
    csrcs.push_back(prng->Rand<uint32_t>());
  }
  rtp_sender.SetCsrcs(csrcs);
  rtp_sender.SetSSRC(prng->Rand<uint32_t>());
  rtp_sender.SetStartTimestamp(prng->Rand<uint32_t>(), true);
  rtp_sender.SetSequenceNumber(prng->Rand<uint16_t>());

  for (unsigned i = 0; i < kNumExtensions; i++) {
    if (extensions_bitvector & (1u << i)) {
      rtp_sender.RegisterRtpHeaderExtension(kExtensionTypes[i], i + 1);
    }
  }

  int8_t payload_type = prng->Rand(0, 127);
  bool marker_bit = prng->Rand<bool>();
  uint32_t capture_timestamp = prng->Rand<uint32_t>();
  int64_t capture_time_ms = prng->Rand<uint32_t>();
  bool timestamp_provided = prng->Rand<bool>();
  bool inc_sequence_number = prng->Rand<bool>();

  size_t header_size = rtp_sender.BuildRTPheader(
      packet, payload_type, marker_bit, capture_timestamp, capture_time_ms,
      timestamp_provided, inc_sequence_number);

  for (size_t i = header_size; i < packet_size; i++) {
    packet[i] = prng->Rand<uint8_t>();
  }

  return header_size;
}

rtc::Buffer GenerateRtcpPacket(Random* prng) {
  rtcp::ReportBlock report_block;
  report_block.To(prng->Rand<uint32_t>());  // Remote SSRC.
  report_block.WithFractionLost(prng->Rand(50));

  rtcp::SenderReport sender_report;
  sender_report.From(prng->Rand<uint32_t>());  // Sender SSRC.
  sender_report.WithNtp(
      NtpTime(prng->Rand<uint32_t>(), prng->Rand<uint32_t>()));
  sender_report.WithPacketCount(prng->Rand<uint32_t>());
  sender_report.WithReportBlock(report_block);

  return sender_report.Build();
}

void GenerateVideoReceiveConfig(uint32_t extensions_bitvector,
                                VideoReceiveStream::Config* config,
                                Random* prng) {
  // Create a map from a payload type to an encoder name.
  VideoReceiveStream::Decoder decoder;
  decoder.payload_type = prng->Rand(0, 127);
  decoder.payload_name = (prng->Rand<bool>() ? "VP8" : "H264");
  config->decoders.push_back(decoder);
  // Add SSRCs for the stream.
  config->rtp.remote_ssrc = prng->Rand<uint32_t>();
  config->rtp.local_ssrc = prng->Rand<uint32_t>();
  // Add extensions and settings for RTCP.
  config->rtp.rtcp_mode =
      prng->Rand<bool>() ? RtcpMode::kCompound : RtcpMode::kReducedSize;
  config->rtp.remb = prng->Rand<bool>();
  // Add a map from a payload type to a new ssrc and a new payload type for RTX.
  VideoReceiveStream::Config::Rtp::Rtx rtx_pair;
  rtx_pair.ssrc = prng->Rand<uint32_t>();
  rtx_pair.payload_type = prng->Rand(0, 127);
  config->rtp.rtx.insert(std::make_pair(prng->Rand(0, 127), rtx_pair));
  // Add header extensions.
  for (unsigned i = 0; i < kNumExtensions; i++) {
    if (extensions_bitvector & (1u << i)) {
      config->rtp.extensions.push_back(
          RtpExtension(kExtensionNames[i], prng->Rand<int>()));
    }
  }
}

void GenerateVideoSendConfig(uint32_t extensions_bitvector,
                             VideoSendStream::Config* config,
                             Random* prng) {
  // Create a map from a payload type to an encoder name.
  config->encoder_settings.payload_type = prng->Rand(0, 127);
  config->encoder_settings.payload_name = (prng->Rand<bool>() ? "VP8" : "H264");
  // Add SSRCs for the stream.
  config->rtp.ssrcs.push_back(prng->Rand<uint32_t>());
  // Add a map from a payload type to new ssrcs and a new payload type for RTX.
  config->rtp.rtx.ssrcs.push_back(prng->Rand<uint32_t>());
  config->rtp.rtx.payload_type = prng->Rand(0, 127);
  // Add header extensions.
  for (unsigned i = 0; i < kNumExtensions; i++) {
    if (extensions_bitvector & (1u << i)) {
      config->rtp.extensions.push_back(
          RtpExtension(kExtensionNames[i], prng->Rand<int>()));
    }
  }
}

// Test for the RtcEventLog class. Dumps some RTP packets and other events
// to disk, then reads them back to see if they match.
void LogSessionAndReadBack(size_t rtp_count,
                           size_t rtcp_count,
                           size_t playout_count,
                           size_t bwe_loss_count,
                           uint32_t extensions_bitvector,
                           uint32_t csrcs_count,
                           unsigned int random_seed) {
  ASSERT_LE(rtcp_count, rtp_count);
  ASSERT_LE(playout_count, rtp_count);
  ASSERT_LE(bwe_loss_count, rtp_count);
  std::vector<rtc::Buffer> rtp_packets;
  std::vector<rtc::Buffer> rtcp_packets;
  std::vector<size_t> rtp_header_sizes;
  std::vector<uint32_t> playout_ssrcs;
  std::vector<std::pair<int32_t, uint8_t> > bwe_loss_updates;

  VideoReceiveStream::Config receiver_config(nullptr);
  VideoSendStream::Config sender_config(nullptr);

  Random prng(random_seed);

  // Create rtp_count RTP packets containing random data.
  for (size_t i = 0; i < rtp_count; i++) {
    size_t packet_size = prng.Rand(1000, 1100);
    rtp_packets.push_back(rtc::Buffer(packet_size));
    size_t header_size =
        GenerateRtpPacket(extensions_bitvector, csrcs_count,
                          rtp_packets[i].data(), packet_size, &prng);
    rtp_header_sizes.push_back(header_size);
  }
  // Create rtcp_count RTCP packets containing random data.
  for (size_t i = 0; i < rtcp_count; i++) {
    rtcp_packets.push_back(GenerateRtcpPacket(&prng));
  }
  // Create playout_count random SSRCs to use when logging AudioPlayout events.
  for (size_t i = 0; i < playout_count; i++) {
    playout_ssrcs.push_back(prng.Rand<uint32_t>());
  }
  // Create bwe_loss_count random bitrate updates for BwePacketLoss.
  for (size_t i = 0; i < bwe_loss_count; i++) {
    bwe_loss_updates.push_back(
        std::make_pair(prng.Rand<int32_t>(), prng.Rand<uint8_t>()));
  }
  // Create configurations for the video streams.
  GenerateVideoReceiveConfig(extensions_bitvector, &receiver_config, &prng);
  GenerateVideoSendConfig(extensions_bitvector, &sender_config, &prng);
  const int config_count = 2;

  // Find the name of the current test, in order to use it as a temporary
  // filename.
  auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
  const std::string temp_filename =
      test::OutputPath() + test_info->test_case_name() + test_info->name();

  // When log_dumper goes out of scope, it causes the log file to be flushed
  // to disk.
  {
    SimulatedClock fake_clock(prng.Rand<uint32_t>());
    std::unique_ptr<RtcEventLog> log_dumper(RtcEventLog::Create(&fake_clock));
    log_dumper->LogVideoReceiveStreamConfig(receiver_config);
    fake_clock.AdvanceTimeMicroseconds(prng.Rand(1, 1000));
    log_dumper->LogVideoSendStreamConfig(sender_config);
    fake_clock.AdvanceTimeMicroseconds(prng.Rand(1, 1000));
    size_t rtcp_index = 1;
    size_t playout_index = 1;
    size_t bwe_loss_index = 1;
    for (size_t i = 1; i <= rtp_count; i++) {
      log_dumper->LogRtpHeader(
          (i % 2 == 0) ? kIncomingPacket : kOutgoingPacket,
          (i % 3 == 0) ? MediaType::AUDIO : MediaType::VIDEO,
          rtp_packets[i - 1].data(), rtp_packets[i - 1].size());
      fake_clock.AdvanceTimeMicroseconds(prng.Rand(1, 1000));
      if (i * rtcp_count >= rtcp_index * rtp_count) {
        log_dumper->LogRtcpPacket(
            (rtcp_index % 2 == 0) ? kIncomingPacket : kOutgoingPacket,
            rtcp_index % 3 == 0 ? MediaType::AUDIO : MediaType::VIDEO,
            rtcp_packets[rtcp_index - 1].data(),
            rtcp_packets[rtcp_index - 1].size());
        rtcp_index++;
        fake_clock.AdvanceTimeMicroseconds(prng.Rand(1, 1000));
      }
      if (i * playout_count >= playout_index * rtp_count) {
        log_dumper->LogAudioPlayout(playout_ssrcs[playout_index - 1]);
        playout_index++;
        fake_clock.AdvanceTimeMicroseconds(prng.Rand(1, 1000));
      }
      if (i * bwe_loss_count >= bwe_loss_index * rtp_count) {
        log_dumper->LogBwePacketLossEvent(
            bwe_loss_updates[bwe_loss_index - 1].first,
            bwe_loss_updates[bwe_loss_index - 1].second, i);
        bwe_loss_index++;
        fake_clock.AdvanceTimeMicroseconds(prng.Rand(1, 1000));
      }
      if (i == rtp_count / 2) {
        log_dumper->StartLogging(temp_filename, 10000000);
        fake_clock.AdvanceTimeMicroseconds(prng.Rand(1, 1000));
      }
    }
    log_dumper->StopLogging();
  }

  // Read the generated file from disk.
  rtclog::EventStream parsed_stream;

  ASSERT_TRUE(RtcEventLog::ParseRtcEventLog(temp_filename, &parsed_stream));

  // Verify that what we read back from the event log is the same as
  // what we wrote down. For RTCP we log the full packets, but for
  // RTP we should only log the header.
  const int event_count = config_count + playout_count + bwe_loss_count +
                          rtcp_count + rtp_count + 2;
  EXPECT_GE(1000, event_count);  // The events must fit in the message queue.
  EXPECT_EQ(event_count, parsed_stream.stream_size());
  if (event_count != parsed_stream.stream_size()) {
    // Print the expected and actual event types for easier debugging.
    std::map<int, size_t> actual_event_counts;
    for (size_t i = 0; i < static_cast<size_t>(parsed_stream.stream_size());
         i++) {
      actual_event_counts[parsed_stream.stream(i).type()]++;
    }
    printf("Actual events: ");
    for (auto kv : actual_event_counts) {
      printf("%d_count = %zu, ", kv.first, kv.second);
    }
    printf("\n");
    for (size_t i = 0; i < static_cast<size_t>(parsed_stream.stream_size());
         i++) {
      printf("%4d ", parsed_stream.stream(i).type());
    }
    printf("\n");
    printf(
        "Expected events: rtp_count = %zu, rtcp_count = %zu,"
        "playout_count = %zu, bwe_loss_count = %zu\n",
        rtp_count, rtcp_count, playout_count, bwe_loss_count);
    size_t rtcp_index = 1, playout_index = 1, bwe_loss_index = 1;
    printf("strt  cfg  cfg ");
    for (size_t i = 1; i <= rtp_count; i++) {
      printf(" rtp ");
      if (i * rtcp_count >= rtcp_index * rtp_count) {
        printf("rtcp ");
        rtcp_index++;
      }
      if (i * playout_count >= playout_index * rtp_count) {
        printf("play ");
        playout_index++;
      }
      if (i * bwe_loss_count >= bwe_loss_index * rtp_count) {
        printf("loss ");
        bwe_loss_index++;
      }
    }
    printf("\n");
  }
  VerifyLogStartEvent(parsed_stream.stream(0));
  VerifyReceiveStreamConfig(parsed_stream.stream(1), receiver_config);
  VerifySendStreamConfig(parsed_stream.stream(2), sender_config);
  size_t event_index = config_count + 1;
  size_t rtcp_index = 1;
  size_t playout_index = 1;
  size_t bwe_loss_index = 1;
  for (size_t i = 1; i <= rtp_count; i++) {
    VerifyRtpEvent(parsed_stream.stream(event_index),
                   (i % 2 == 0) ? kIncomingPacket : kOutgoingPacket,
                   (i % 3 == 0) ? MediaType::AUDIO : MediaType::VIDEO,
                   rtp_packets[i - 1].data(), rtp_header_sizes[i - 1],
                   rtp_packets[i - 1].size());
    event_index++;
    if (i * rtcp_count >= rtcp_index * rtp_count) {
      VerifyRtcpEvent(parsed_stream.stream(event_index),
                      (rtcp_index % 2 == 0) ? kIncomingPacket : kOutgoingPacket,
                      rtcp_index % 3 == 0 ? MediaType::AUDIO : MediaType::VIDEO,
                      rtcp_packets[rtcp_index - 1].data(),
                      rtcp_packets[rtcp_index - 1].size());
      event_index++;
      rtcp_index++;
    }
    if (i * playout_count >= playout_index * rtp_count) {
      VerifyPlayoutEvent(parsed_stream.stream(event_index),
                         playout_ssrcs[playout_index - 1]);
      event_index++;
      playout_index++;
    }
    if (i * bwe_loss_count >= bwe_loss_index * rtp_count) {
      VerifyBweLossEvent(parsed_stream.stream(event_index),
                         bwe_loss_updates[bwe_loss_index - 1].first,
                         bwe_loss_updates[bwe_loss_index - 1].second, i);
      event_index++;
      bwe_loss_index++;
    }
  }

  // Clean up temporary file - can be pretty slow.
  remove(temp_filename.c_str());
}

TEST(RtcEventLogTest, LogSessionAndReadBack) {
  // Log 5 RTP, 2 RTCP, 0 playout events and 0 BWE events
  // with no header extensions or CSRCS.
  LogSessionAndReadBack(5, 2, 0, 0, 0, 0, 321);

  // Enable AbsSendTime and TransportSequenceNumbers.
  uint32_t extensions = 0;
  for (uint32_t i = 0; i < kNumExtensions; i++) {
    if (kExtensionTypes[i] == RTPExtensionType::kRtpExtensionAbsoluteSendTime ||
        kExtensionTypes[i] ==
            RTPExtensionType::kRtpExtensionTransportSequenceNumber) {
      extensions |= 1u << i;
    }
  }
  LogSessionAndReadBack(8, 2, 0, 0, extensions, 0, 3141592653u);

  extensions = (1u << kNumExtensions) - 1;  // Enable all header extensions.
  LogSessionAndReadBack(9, 2, 3, 2, extensions, 2, 2718281828u);

  // Try all combinations of header extensions and up to 2 CSRCS.
  for (extensions = 0; extensions < (1u << kNumExtensions); extensions++) {
    for (uint32_t csrcs_count = 0; csrcs_count < 3; csrcs_count++) {
      LogSessionAndReadBack(5 + extensions,   // Number of RTP packets.
                            2 + csrcs_count,  // Number of RTCP packets.
                            3 + csrcs_count,  // Number of playout events.
                            1 + csrcs_count,  // Number of BWE loss events.
                            extensions,       // Bit vector choosing extensions.
                            csrcs_count,      // Number of contributing sources.
                            extensions * 3 + csrcs_count + 1);  // Random seed.
    }
  }
}

TEST(RtcEventLogTest, LogEventAndReadBack) {
  Random prng(987654321);

  // Create one RTP and one RTCP packet containing random data.
  size_t packet_size = prng.Rand(1000, 1100);
  rtc::Buffer rtp_packet(packet_size);
  size_t header_size =
      GenerateRtpPacket(0, 0, rtp_packet.data(), packet_size, &prng);
  rtc::Buffer rtcp_packet = GenerateRtcpPacket(&prng);

  // Find the name of the current test, in order to use it as a temporary
  // filename.
  auto test_info = ::testing::UnitTest::GetInstance()->current_test_info();
  const std::string temp_filename =
      test::OutputPath() + test_info->test_case_name() + test_info->name();

  // Add RTP, start logging, add RTCP and then stop logging
  SimulatedClock fake_clock(prng.Rand<uint32_t>());
  std::unique_ptr<RtcEventLog> log_dumper(RtcEventLog::Create(&fake_clock));

  log_dumper->LogRtpHeader(kIncomingPacket, MediaType::VIDEO, rtp_packet.data(),
                           rtp_packet.size());
  fake_clock.AdvanceTimeMicroseconds(prng.Rand(1, 1000));

  log_dumper->StartLogging(temp_filename, 10000000);
  fake_clock.AdvanceTimeMicroseconds(prng.Rand(1, 1000));

  log_dumper->LogRtcpPacket(kOutgoingPacket, MediaType::VIDEO,
                            rtcp_packet.data(), rtcp_packet.size());
  fake_clock.AdvanceTimeMicroseconds(prng.Rand(1, 1000));

  log_dumper->StopLogging();

  // Read the generated file from disk.
  rtclog::EventStream parsed_stream;
  ASSERT_TRUE(RtcEventLog::ParseRtcEventLog(temp_filename, &parsed_stream));

  // Verify that what we read back from the event log is the same as
  // what we wrote down.
  EXPECT_EQ(4, parsed_stream.stream_size());

  VerifyLogStartEvent(parsed_stream.stream(0));

  VerifyRtpEvent(parsed_stream.stream(1), kIncomingPacket, MediaType::VIDEO,
                 rtp_packet.data(), header_size, rtp_packet.size());

  VerifyRtcpEvent(parsed_stream.stream(2), kOutgoingPacket, MediaType::VIDEO,
                  rtcp_packet.data(), rtcp_packet.size());

  VerifyLogEndEvent(parsed_stream.stream(3));

  // Clean up temporary file - can be pretty slow.
  remove(temp_filename.c_str());
}
}  // namespace webrtc

#endif  // ENABLE_RTC_EVENT_LOG
