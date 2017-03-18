/*
 *  Copyright (c) 2014 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */
#include "webrtc/base/checks.h"
#include "webrtc/common.h"
#include "webrtc/config.h"
#include "webrtc/test/encoder_settings.h"
#include "webrtc/test/testsupport/fileutils.h"
#include "webrtc/voice_engine/include/voe_base.h"
#include "webrtc/voice_engine/include/voe_codec.h"
#include "webrtc/voice_engine/include/voe_network.h"
#include "webrtc/test/video_capturer.h"
#include "webrtc/test/mycall.h"

using namespace webrtc;
using namespace test;

namespace {
const int kVideoRotationRtpExtensionId = 4;
}

MyCALL::MyCALL() :
		clock_(Clock::GetRealTimeClock()), video_send_config_(nullptr), video_send_stream_(
				nullptr), audio_send_config_(nullptr), audio_send_stream_(
				nullptr), video_encoder_(nullptr), num_video_streams_(1), num_audio_streams_(
				0), fake_send_audio_device_(nullptr), fake_recv_audio_device_(
				nullptr) {
	video_encoder_ = VideoEncoder::Create(VideoEncoder::EncoderType::kH264);
	printf("MyCALL::video_encoder_  %p\n", video_encoder_);
}

MyCALL::~MyCALL() {
}

void MyCALL::RunBaseTest(BaseTest* test) {
	num_video_streams_ = 1;
	num_audio_streams_ = 0;
	RTC_DCHECK(num_video_streams_ > 0 || num_audio_streams_ > 0);
	Call::Config send_config(test->GetSenderCallConfig());
	if (num_audio_streams_ > 0) {
		CreateVoiceEngines();
		AudioState::Config audio_state_config;
		audio_state_config.voice_engine = voe_send_.voice_engine;
		send_config.audio_state = AudioState::Create(audio_state_config);
	}
	CreateSenderCall(send_config);
	if (test->ShouldCreateReceivers()) {
		Call::Config recv_config(test->GetReceiverCallConfig());
		if (num_audio_streams_ > 0) {
			AudioState::Config audio_state_config;
			audio_state_config.voice_engine = voe_recv_.voice_engine;
			recv_config.audio_state = AudioState::Create(audio_state_config);
		}
		CreateReceiverCall(recv_config);
	}
	test->OnCallsCreated(sender_call_.get(), receiver_call_.get());
	udp_transport_.reset(test->CreateUdpSendTransport(sender_call_.get()));
	receive_transport_.reset(test->CreateReceiveTransport());

	if (test->ShouldCreateReceivers()) {
		udp_transport_->StartReceiving(1);
		receive_transport_->SetReceiver(sender_call_->Receiver());
	} else {
		// Sender-only call delivers to itself.
		udp_transport_->StartReceiving(1);
		receive_transport_->SetReceiver(nullptr);
	}

	CreateSendConfig(num_video_streams_, num_audio_streams_,
			udp_transport_.get());
	if (test->ShouldCreateReceivers()) {
		CreateMatchingReceiveConfigs(receive_transport_.get());
	}
	if (num_audio_streams_ > 0)
		SetupVoiceEngineTransports(udp_transport_.get(),
				receive_transport_.get());

	if (num_video_streams_ > 0) {
		test->ModifyVideoConfigs(&video_send_config_, &video_receive_configs_,
				&video_encoder_config_);
	}
	if (num_audio_streams_ > 0)
		test->ModifyAudioConfigs(&audio_send_config_, &audio_receive_configs_);

	if (num_video_streams_ > 0) {
		CreateVideoStreams();
		test->OnVideoStreamsCreated(video_send_stream_, video_receive_streams_);
	}
	if (num_audio_streams_ > 0) {
		CreateAudioStreams();
		test->OnAudioStreamsCreated(audio_send_stream_, audio_receive_streams_);
	}

	if (num_video_streams_ > 0) {
		CreateFrameGeneratorCapturer();
		test->OnFrameGeneratorCapturerCreated(frame_generator_capturer_.get());
	}

	Start();
	test->PerformTest();
	udp_transport_->StopReceiving();
	receive_transport_->StopSending();
	Stop();

	DestroyStreams();
	DestroyCalls();
	if (num_audio_streams_ > 0)
		DestroyVoiceEngines();
}

void MyCALL::Start() {
	if (video_send_stream_) {
		video_send_stream_->Start();
	}
	for (VideoReceiveStream* video_recv_stream : video_receive_streams_)
		video_recv_stream->Start();
	if (audio_send_stream_) {
		fake_send_audio_device_->Start();
		audio_send_stream_->Start();
		EXPECT_EQ(0, voe_send_.base->StartSend(voe_send_.channel_id));
	}
	for (AudioReceiveStream* audio_recv_stream : audio_receive_streams_)
		audio_recv_stream->Start();
	if (!audio_receive_streams_.empty()) {
		fake_recv_audio_device_->Start();
		EXPECT_EQ(0, voe_recv_.base->StartPlayout(voe_recv_.channel_id));
		EXPECT_EQ(0, voe_recv_.base->StartReceive(voe_recv_.channel_id));
	}
	printf(" call capture %p\n", frame_generator_capturer_.get());

	if (frame_generator_capturer_.get() != NULL)
		frame_generator_capturer_->Start();
}

void MyCALL::Stop() {
	if (frame_generator_capturer_.get() != NULL)
		frame_generator_capturer_->Stop();
	if (!audio_receive_streams_.empty()) {
		fake_recv_audio_device_->Stop();
		EXPECT_EQ(0, voe_recv_.base->StopReceive(voe_recv_.channel_id));
		EXPECT_EQ(0, voe_recv_.base->StopPlayout(voe_recv_.channel_id));
	}
	for (AudioReceiveStream* audio_recv_stream : audio_receive_streams_)
		audio_recv_stream->Stop();
	if (audio_send_stream_) {
		fake_send_audio_device_->Stop();
		EXPECT_EQ(0, voe_send_.base->StopSend(voe_send_.channel_id));
		audio_send_stream_->Stop();
	}
	for (VideoReceiveStream* video_recv_stream : video_receive_streams_)
		video_recv_stream->Stop();
	if (video_send_stream_)
		video_send_stream_->Stop();
}

void MyCALL::CreateCalls(const Call::Config& sender_config,
		const Call::Config& receiver_config) {
	CreateSenderCall(sender_config);
	CreateReceiverCall(receiver_config);
}

void MyCALL::CreateSenderCall(const Call::Config& config) {
	sender_call_.reset(Call::Create(config));
}

void MyCALL::CreateReceiverCall(const Call::Config& config) {
	receiver_call_.reset(Call::Create(config));
}

void MyCALL::DestroyCalls() {
	sender_call_.reset();
	receiver_call_.reset();
}

void MyCALL::CreateSendConfig(size_t num_video_streams,
		size_t num_audio_streams, Transport* send_transport) {
	RTC_DCHECK(num_video_streams <= kNumSsrcs);
	RTC_DCHECK_LE(num_audio_streams, 1u);
	RTC_DCHECK(num_audio_streams == 0 || voe_send_.channel_id >= 0);
	if (num_video_streams > 0) {
		video_send_config_ = VideoSendStream::Config(send_transport);
		video_send_config_.encoder_settings.encoder = video_encoder_;
		video_send_config_.encoder_settings.payload_name = "H264";
		video_send_config_.encoder_settings.payload_type =
				kH264VideoSendPayloadType;
		video_send_config_.rtp.extensions.push_back(
				RtpExtension(RtpExtension::kAbsSendTime,
						kAbsSendTimeExtensionId));
		video_encoder_config_.streams = test::CreateVideoStreams(
				num_video_streams);
		for (size_t i = 0; i < num_video_streams; ++i)
			video_send_config_.rtp.ssrcs.push_back(kVideoSendSsrcs[i]);
		video_send_config_.rtp.extensions.push_back(
				RtpExtension(RtpExtension::kVideoRotation,
						kVideoRotationRtpExtensionId));

		video_send_config_.rtp.rtx.payload_type = kSendRtxPayloadType;
		for (size_t i = 0; i < num_video_streams; ++i)
			video_send_config_.rtp.rtx.ssrcs.push_back(kSendRtxSsrcs[i]);

		video_send_config_.rtp.nack.rtp_history_ms = 0;
		video_send_config_.rtp.fec.ulpfec_payload_type = kUlpfecPayloadType;
		video_send_config_.rtp.fec.red_payload_type = kRedPayloadType;
		video_send_config_.rtp.fec.red_rtx_payload_type = kRtxRedPayloadType;

	}

	if (num_audio_streams > 0) {
		audio_send_config_ = AudioSendStream::Config(send_transport);
		audio_send_config_.voe_channel_id = voe_send_.channel_id;
		audio_send_config_.rtp.ssrc = kAudioSendSsrc;
	}
}

void MyCALL::CreateMatchingReceiveConfigs(Transport* rtcp_send_transport) {
	RTC_DCHECK(video_receive_configs_.empty());
	RTC_DCHECK(allocated_decoders_.empty());
	if (num_video_streams_ > 0) {
		RTC_DCHECK(!video_send_config_.rtp.ssrcs.empty());
		VideoReceiveStream::Config video_config(rtcp_send_transport);
		video_config.rtp.remb = true;
		video_config.rtp.local_ssrc = kReceiverLocalVideoSsrc;
		for (const RtpExtension& extension : video_send_config_.rtp.extensions)
			video_config.rtp.extensions.push_back(extension);
		for (size_t i = 0; i < video_send_config_.rtp.ssrcs.size(); ++i) {
			VideoReceiveStream::Decoder decoder = test::CreateMatchingDecoder(
					video_send_config_.encoder_settings);
			allocated_decoders_.push_back(
					std::unique_ptr<VideoDecoder>(decoder.decoder));
			video_config.decoders.clear();
			video_config.decoders.push_back(decoder);
			video_config.rtp.remote_ssrc = video_send_config_.rtp.ssrcs[i];
			video_receive_configs_.push_back(video_config);
		}
	}
	if (num_audio_streams_ == 1) {
		RTC_DCHECK(voe_send_.channel_id >= 0);
		AudioReceiveStream::Config audio_config;
		audio_config.rtp.local_ssrc = kReceiverLocalAudioSsrc;
		audio_config.rtcp_send_transport = rtcp_send_transport;
		audio_config.voe_channel_id = voe_recv_.channel_id;
		audio_config.rtp.remote_ssrc = audio_send_config_.rtp.ssrc;
		audio_receive_configs_.push_back(audio_config);
	}
}

void MyCALL::CreateFrameGeneratorCapturerWithDrift(Clock* clock, float speed) {
	VideoStream stream = video_encoder_config_.streams.back();
	frame_generator_capturer_.reset(
			test::VideoCapturer::Create(video_send_stream_->Input(),
					stream.width, stream.height, stream.max_framerate * speed,
					clock));
}

void MyCALL::CreateFrameGeneratorCapturer() {
	VideoStream stream = video_encoder_config_.streams.back();
	frame_generator_capturer_.reset(
			test::VideoCapturer::Create(video_send_stream_->Input(),
					stream.width, stream.height, stream.max_framerate, clock_));
}

void MyCALL::CreateFakeAudioDevices() {
	fake_send_audio_device_.reset(
			new FakeAudioDevice(clock_,
					test::ResourcePath("voice_engine/audio_long16", "pcm"),
					DriftingClock::kNoDrift));
	fake_recv_audio_device_.reset(
			new FakeAudioDevice(clock_,
					test::ResourcePath("voice_engine/audio_long16", "pcm"),
					DriftingClock::kNoDrift));
}

void MyCALL::CreateVideoStreams() {
	RTC_DCHECK(video_send_stream_ == nullptr);
	RTC_DCHECK(video_receive_streams_.empty());
	RTC_DCHECK(audio_send_stream_ == nullptr);
	RTC_DCHECK(audio_receive_streams_.empty());

	video_send_stream_ = sender_call_->CreateVideoSendStream(video_send_config_,
			video_encoder_config_);
	for (size_t i = 0; i < video_receive_configs_.size(); ++i) {
		video_receive_streams_.push_back(
				receiver_call_->CreateVideoReceiveStream(
						video_receive_configs_[i]));
	}
}

void MyCALL::SetFakeVideoCaptureRotation(VideoRotation rotation) {

}

void MyCALL::CreateAudioStreams() {
	audio_send_stream_ = sender_call_->CreateAudioSendStream(
			audio_send_config_);
	for (size_t i = 0; i < audio_receive_configs_.size(); ++i) {
		audio_receive_streams_.push_back(
				receiver_call_->CreateAudioReceiveStream(
						audio_receive_configs_[i]));
	}
	CodecInst isac = { kAudioSendPayloadType, "ISAC", 16000, 480, 1, 32000 };
	EXPECT_EQ(0, voe_send_.codec->SetSendCodec(voe_send_.channel_id, isac));
}

void MyCALL::DestroyStreams() {
	if (video_send_stream_)
		sender_call_->DestroyVideoSendStream(video_send_stream_);
	video_send_stream_ = nullptr;
	for (VideoReceiveStream* video_recv_stream : video_receive_streams_)
		receiver_call_->DestroyVideoReceiveStream(video_recv_stream);

	if (audio_send_stream_)
		sender_call_->DestroyAudioSendStream(audio_send_stream_);
	audio_send_stream_ = nullptr;
	for (AudioReceiveStream* audio_recv_stream : audio_receive_streams_)
		receiver_call_->DestroyAudioReceiveStream(audio_recv_stream);
	video_receive_streams_.clear();

	allocated_decoders_.clear();
}

void MyCALL::CreateVoiceEngines() {
	CreateFakeAudioDevices();
	voe_send_.voice_engine = VoiceEngine::Create();
	voe_send_.base = VoEBase::GetInterface(voe_send_.voice_engine);
	voe_send_.network = VoENetwork::GetInterface(voe_send_.voice_engine);
	voe_send_.codec = VoECodec::GetInterface(voe_send_.voice_engine);
	EXPECT_EQ(0, voe_send_.base->Init(fake_send_audio_device_.get(), nullptr));
	Config voe_config;
	voe_config.Set<VoicePacing>(new VoicePacing(true));
	voe_send_.channel_id = voe_send_.base->CreateChannel(voe_config);
	EXPECT_GE(voe_send_.channel_id, 0);

	voe_recv_.voice_engine = VoiceEngine::Create();
	voe_recv_.base = VoEBase::GetInterface(voe_recv_.voice_engine);
	voe_recv_.network = VoENetwork::GetInterface(voe_recv_.voice_engine);
	voe_recv_.codec = VoECodec::GetInterface(voe_recv_.voice_engine);
	EXPECT_EQ(0, voe_recv_.base->Init(fake_recv_audio_device_.get(), nullptr));
	voe_recv_.channel_id = voe_recv_.base->CreateChannel();
	EXPECT_GE(voe_recv_.channel_id, 0);
}

void MyCALL::SetupVoiceEngineTransports(UdpTransport* send_transport,
		PacketTransport* recv_transport) {
	voe_send_.transport_adapter.reset(
			new internal::TransportAdapter(send_transport));
	voe_send_.transport_adapter->Enable();
	EXPECT_EQ(0,
			voe_send_.network->RegisterExternalTransport(voe_send_.channel_id,
					*voe_send_.transport_adapter.get()));

	voe_recv_.transport_adapter.reset(
			new internal::TransportAdapter(recv_transport));
	voe_recv_.transport_adapter->Enable();
	EXPECT_EQ(0,
			voe_recv_.network->RegisterExternalTransport(voe_recv_.channel_id,
					*voe_recv_.transport_adapter.get()));
}

void MyCALL::DestroyVoiceEngines() {
	voe_recv_.base->DeleteChannel(voe_recv_.channel_id);
	voe_recv_.channel_id = -1;
	voe_recv_.base->Release();
	voe_recv_.base = nullptr;
	voe_recv_.network->Release();
	voe_recv_.network = nullptr;
	voe_recv_.codec->Release();
	voe_recv_.codec = nullptr;

	voe_send_.base->DeleteChannel(voe_send_.channel_id);
	voe_send_.channel_id = -1;
	voe_send_.base->Release();
	voe_send_.base = nullptr;
	voe_send_.network->Release();
	voe_send_.network = nullptr;
	voe_send_.codec->Release();
	voe_send_.codec = nullptr;

	VoiceEngine::Delete(voe_send_.voice_engine);
	voe_send_.voice_engine = nullptr;
	VoiceEngine::Delete(voe_recv_.voice_engine);
	voe_recv_.voice_engine = nullptr;
}

const int MyCALL::kDefaultTimeoutMs = 30 * 1000;
const int MyCALL::kLongTimeoutMs = 120 * 1000;
const uint8_t MyCALL::kVideoSendPayloadType = 100;
const uint8_t MyCALL::kFakeVideoSendPayloadType = 125;
const uint8_t MyCALL::kH264VideoSendPayloadType = 122;
const uint8_t MyCALL::kSendRtxPayloadType = 98;
const uint8_t MyCALL::kRedPayloadType = 118;
const uint8_t MyCALL::kRtxRedPayloadType = 99;
const uint8_t MyCALL::kUlpfecPayloadType = 119;
const uint8_t MyCALL::kAudioSendPayloadType = 103;
const uint32_t MyCALL::kSendRtxSsrcs[kNumSsrcs] = { 0xBADCAFD, 0xBADCAFE,
		0xBADCAFF };
const uint32_t MyCALL::kVideoSendSsrcs[kNumSsrcs] = { 0xC0FFED, 0xC0FFEE,
		0xC0FFEF };
const uint32_t MyCALL::kAudioSendSsrc = 0xDEADBEEF;
const uint32_t MyCALL::kReceiverLocalVideoSsrc = 0x123456;
const uint32_t MyCALL::kReceiverLocalAudioSsrc = 0x1234567;
const int MyCALL::kNackRtpHistoryMs = 1000;

BaseTest::BaseTest(unsigned int timeout_ms) :
		RtpRtcpObserver(timeout_ms) {
}

BaseTest::~BaseTest() {
}

Call::Config BaseTest::GetSenderCallConfig() {
	return Call::Config();
}

Call::Config BaseTest::GetReceiverCallConfig() {
	return Call::Config();
}

void BaseTest::OnCallsCreated(Call* sender_call, Call* receiver_call) {
}

test::PacketTransport* BaseTest::CreateSendTransport(Call* sender_call) {
	return new PacketTransport(sender_call, this,
			test::PacketTransport::kSender, FakeNetworkPipe::Config());
}

test::PacketTransport* BaseTest::CreateReceiveTransport() {
	return new PacketTransport(nullptr, this, test::PacketTransport::kReceiver,
			FakeNetworkPipe::Config());
}

UdpTransport* BaseTest::CreateUdpSendTransport(Call* sender_call) {
	uint8_t numSocketThreads = 1;
	UdpTransport* send_trans_ = UdpTransport::Create(1, numSocketThreads);
	send_trans_->InitializeSendSockets("127.0.0.1", 5004, 5005);
	return send_trans_;
}

UdpTransport* BaseTest::CreateUdpReceiveTransport() {
	uint8_t numSocketThreads = 1;
	UdpTransport* recv_trans_ = UdpTransport::Create(1, numSocketThreads);
	recv_trans_->InitializeReceiveSockets(new UdpTransportDataImpl(), 5004);
	return recv_trans_;

}


void BaseTest::ModifyVideoConfigs(VideoSendStream::Config* send_config,
		std::vector<VideoReceiveStream::Config>* receive_configs,
		VideoEncoderConfig* encoder_config) {
}

void BaseTest::OnVideoStreamsCreated(VideoSendStream* send_stream,
		const std::vector<VideoReceiveStream*>& receive_streams) {
}

void BaseTest::ModifyAudioConfigs(AudioSendStream::Config* send_config,
		std::vector<AudioReceiveStream::Config>* receive_configs) {
}

void BaseTest::OnAudioStreamsCreated(AudioSendStream* send_stream,
		const std::vector<AudioReceiveStream*>& receive_streams) {
}

void BaseTest::OnFrameGeneratorCapturerCreated(
		VideoCapturer * frame_generator_capturer) {
}

void BaseTest::PerformTest() {
	while (1) {
		sleep(10);
	}
}
;

SendTest::SendTest(unsigned int timeout_ms) :
		BaseTest(timeout_ms) {
}

bool SendTest::ShouldCreateReceivers() const {
	return false;
}

EndToEndTest::EndToEndTest(unsigned int timeout_ms) :
		BaseTest(timeout_ms) {
}

bool EndToEndTest::ShouldCreateReceivers() const {
	return true;
}

#include <memory>

#include "webrtc/test/testsupport/trace_to_stderr.h"
#include "mycall.h"

int main() {
	std::unique_ptr<test::TraceToStderr> trace_to_stderr;

	trace_to_stderr.reset(new test::TraceToStderr);

	std::unique_ptr<BaseTest> testob;
	testob.reset(new EndToEndTest(10));
	std::unique_ptr<MyCALL> callaa;
	callaa.reset(new MyCALL());

	callaa->RunBaseTest(testob.get());
	return 0;
}