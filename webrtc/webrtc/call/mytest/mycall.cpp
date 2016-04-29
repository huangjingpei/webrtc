#include <vector>

#include "webrtc/call.h"
#include "webrtc/call/transport_adapter.h"
#include "webrtc/system_wrappers/include/scoped_vector.h"

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



const int kVideoRotationRtpExtensionId = 4;

class MyCaller {
 public:
    MyCaller();
  virtual ~MyCaller();

  static const size_t kNumSsrcs = 3;

  static const int kDefaultTimeoutMs;
  static const int kLongTimeoutMs;
  static const uint8_t kVideoSendPayloadType;
  static const uint8_t kSendRtxPayloadType;
  static const uint8_t kFakeVideoSendPayloadType;
  static const uint8_t kRedPayloadType;
  static const uint8_t kRtxRedPayloadType;
  static const uint8_t kUlpfecPayloadType;
  static const uint8_t kAudioSendPayloadType;
  static const uint32_t kSendRtxSsrcs[kNumSsrcs];
  static const uint32_t kVideoSendSsrcs[kNumSsrcs];
  static const uint32_t kAudioSendSsrc;
  static const uint32_t kReceiverLocalVideoSsrc;
  static const uint32_t kReceiverLocalAudioSsrc;
  static const int kNackRtpHistoryMs;

 protected:


  void CreateCalls(const Call::Config& sender_config,
                   const Call::Config& receiver_config);
  void CreateSenderCall(const Call::Config& config);
  void CreateReceiverCall(const Call::Config& config);
  void DestroyCalls();

  void CreateSendConfig(size_t num_video_streams,
                        size_t num_audio_streams,
                        Transport* send_transport);
  void CreateMatchingReceiveConfigs(Transport* rtcp_send_transport);

  void CreateFrameGeneratorCapturerWithDrift(Clock* drift_clock, float speed);
  void CreateFrameGeneratorCapturer();
  void CreateFakeAudioDevices();

  void CreateVideoStreams();
  void CreateAudioStreams();
  void Start();
  void Stop();
  void DestroyStreams();

  Clock* const clock_;

  rtc::scoped_ptr<Call> sender_call_;
  rtc::scoped_ptr<PacketTransport> send_transport_;
  VideoSendStream::Config video_send_config_;
  VideoEncoderConfig video_encoder_config_;
  VideoSendStream* video_send_stream_;
  AudioSendStream::Config audio_send_config_;
  AudioSendStream* audio_send_stream_;

  rtc::scoped_ptr<Call> receiver_call_;
  rtc::scoped_ptr<PacketTransport> receive_transport_;
  std::vector<VideoReceiveStream::Config> video_receive_configs_;
  std::vector<VideoReceiveStream*> video_receive_streams_;
  std::vector<AudioReceiveStream::Config> audio_receive_configs_;
  std::vector<AudioReceiveStream*> audio_receive_streams_;

  rtc::scoped_ptr<test::FrameGeneratorCapturer> frame_generator_capturer_;
  test::FakeEncoder fake_encoder_;
  ScopedVector<VideoDecoder> allocated_decoders_;
  size_t num_video_streams_;
  size_t num_audio_streams_;

 private:
  // TODO(holmer): Remove once VoiceEngine is fully refactored to the new API.
  // These methods are used to set up legacy voice engines and channels which is
  // necessary while voice engine is being refactored to the new stream API.
  struct VoiceEngineState {
    VoiceEngineState()
        : voice_engine(nullptr),
          base(nullptr),
          network(nullptr),
          codec(nullptr),
          channel_id(-1),
          transport_adapter(nullptr) {}

    VoiceEngine* voice_engine;
    VoEBase* base;
    VoENetwork* network;
    VoECodec* codec;
    int channel_id;
    rtc::scoped_ptr<internal::TransportAdapter> transport_adapter;
  };

  void CreateVoiceEngines();
  void SetupVoiceEngineTransports(PacketTransport* send_transport,
                                  PacketTransport* recv_transport);
  void DestroyVoiceEngines();

  VoiceEngineState voe_send_;
  VoiceEngineState voe_recv_;

  // The audio devices must outlive the voice engines.
  rtc::scoped_ptr<test::FakeAudioDevice> fake_send_audio_device_;
  rtc::scoped_ptr<test::FakeAudioDevice> fake_recv_audio_device_;
};

MyCaller::MyCaller()
    : clock_(Clock::GetRealTimeClock()),
      video_send_config_(nullptr),
      video_send_stream_(nullptr),
      audio_send_config_(nullptr),
      audio_send_stream_(nullptr),
      fake_encoder_(clock_),
      num_video_streams_(1),
      num_audio_streams_(0),
      fake_send_audio_device_(nullptr),
      fake_recv_audio_device_(nullptr) {}

MyCaller::~MyCaller() {
}

void MyCaller::CreateCalls(const Call::Config& sender_config,
                           const Call::Config& receiver_config) {
  CreateSenderCall(sender_config);
  CreateReceiverCall(receiver_config);
}

void MyCaller::CreateSenderCall(const Call::Config& config) {
  sender_call_.reset(Call::Create(config));
}

void MyCaller::CreateReceiverCall(const Call::Config& config) {
  receiver_call_.reset(Call::Create(config));
}

void MyCaller::DestroyCalls() {
  sender_call_.reset();
  receiver_call_.reset();
}

void MyCaller::CreateSendConfig(size_t num_video_streams,
                                size_t num_audio_streams,
                                Transport* send_transport) {
  RTC_DCHECK(num_video_streams <= kNumSsrcs);
  RTC_DCHECK_LE(num_audio_streams, 1u);
  RTC_DCHECK(num_audio_streams == 0 || voe_send_.channel_id >= 0);
  if (num_video_streams > 0) {
    video_send_config_ = VideoSendStream::Config(send_transport);
    video_send_config_.encoder_settings.encoder = &fake_encoder_;
    video_send_config_.encoder_settings.payload_name = "FAKE";
    video_send_config_.encoder_settings.payload_type =
        kFakeVideoSendPayloadType;
    video_send_config_.rtp.extensions.push_back(
        RtpExtension(RtpExtension::kAbsSendTime, kAbsSendTimeExtensionId));
    video_encoder_config_.streams = test::CreateVideoStreams(num_video_streams);
    for (size_t i = 0; i < num_video_streams; ++i)
      video_send_config_.rtp.ssrcs.push_back(kVideoSendSsrcs[i]);
    video_send_config_.rtp.extensions.push_back(RtpExtension(
        RtpExtension::kVideoRotation, kVideoRotationRtpExtensionId));
  }

  if (num_audio_streams > 0) {
    audio_send_config_ = AudioSendStream::Config(send_transport);
    audio_send_config_.voe_channel_id = voe_send_.channel_id;
    audio_send_config_.rtp.ssrc = kAudioSendSsrc;
  }
}

void MyCaller::CreateMatchingReceiveConfigs(Transport* rtcp_send_transport) {
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
      VideoReceiveStream::Decoder decoder =
          test::CreateMatchingDecoder(video_send_config_.encoder_settings);
      allocated_decoders_.push_back(decoder.decoder);
      video_config.decoders.clear();
      video_config.decoders.push_back(decoder);
      video_config.rtp.remote_ssrc = video_send_config_.rtp.ssrcs[i];
      video_receive_configs_.push_back(video_config);
    }
  }

  RTC_DCHECK(num_audio_streams_ <= 1);
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

void MyCaller::CreateFrameGeneratorCapturerWithDrift(Clock* clock,
                                                     float speed) {
  VideoStream stream = video_encoder_config_.streams.back();
  frame_generator_capturer_.reset(test::FrameGeneratorCapturer::Create(
      video_send_stream_->Input(), stream.width, stream.height,
      stream.max_framerate * speed, clock));
}

void MyCaller::CreateFrameGeneratorCapturer() {
  VideoStream stream = video_encoder_config_.streams.back();
  frame_generator_capturer_.reset(test::FrameGeneratorCapturer::Create(
      video_send_stream_->Input(), stream.width, stream.height,
      stream.max_framerate, clock_));
}

void MyCaller::CreateVideoStreams() {
  RTC_DCHECK(video_send_stream_ == nullptr);
  RTC_DCHECK(video_receive_streams_.empty());
  RTC_DCHECK(audio_send_stream_ == nullptr);
  RTC_DCHECK(audio_receive_streams_.empty());

  video_send_stream_ = sender_call_->CreateVideoSendStream(
      video_send_config_, video_encoder_config_);
  for (size_t i = 0; i < video_receive_configs_.size(); ++i) {
    video_receive_streams_.push_back(
        receiver_call_->CreateVideoReceiveStream(video_receive_configs_[i]));
  }
}

void MyCaller::CreateAudioStreams() {
  audio_send_stream_ = sender_call_->CreateAudioSendStream(audio_send_config_);
  for (size_t i = 0; i < audio_receive_configs_.size(); ++i) {
    audio_receive_streams_.push_back(
        receiver_call_->CreateAudioReceiveStream(audio_receive_configs_[i]));
  }
  CodecInst isac = {kAudioSendPayloadType, "ISAC", 16000, 480, 1, 32000};
  EXPECT_EQ(0, voe_send_.codec->SetSendCodec(voe_send_.channel_id, isac));
}

void MyCaller::DestroyStreams() {
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

void MyCaller::CreateVoiceEngines() {
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

void MyCaller::SetupVoiceEngineTransports(PacketTransport* send_transport,
                                          PacketTransport* recv_transport) {
  voe_send_.transport_adapter.reset(
      new internal::TransportAdapter(send_transport));
  voe_send_.transport_adapter->Enable();
  EXPECT_EQ(0, voe_send_.network->RegisterExternalTransport(
                   voe_send_.channel_id, *voe_send_.transport_adapter.get()));

  voe_recv_.transport_adapter.reset(
      new internal::TransportAdapter(recv_transport));
  voe_recv_.transport_adapter->Enable();
  EXPECT_EQ(0, voe_recv_.network->RegisterExternalTransport(
                   voe_recv_.channel_id, *voe_recv_.transport_adapter.get()));
}

void MyCaller::DestroyVoiceEngines() {
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

const int MyCaller::kDefaultTimeoutMs = 30 * 1000;
const int MyCaller::kLongTimeoutMs = 120 * 1000;
const uint8_t MyCaller::kVideoSendPayloadType = 100;
const uint8_t MyCaller::kFakeVideoSendPayloadType = 125;
const uint8_t MyCaller::kSendRtxPayloadType = 98;
const uint8_t MyCaller::kRedPayloadType = 118;
const uint8_t MyCaller::kRtxRedPayloadType = 99;
const uint8_t MyCaller::kUlpfecPayloadType = 119;
const uint8_t MyCaller::kAudioSendPayloadType = 103;
const uint32_t MyCaller::kSendRtxSsrcs[kNumSsrcs] = {0xBADCAFD, 0xBADCAFE,
                                                     0xBADCAFF};
const uint32_t MyCaller::kVideoSendSsrcs[kNumSsrcs] = {0xC0FFED, 0xC0FFEE,
                                                       0xC0FFEF};
const uint32_t MyCaller::kAudioSendSsrc = 0xDEADBEEF;
const uint32_t MyCaller::kReceiverLocalVideoSsrc = 0x123456;
const uint32_t MyCaller::kReceiverLocalAudioSsrc = 0x1234567;
const int MyCaller::kNackRtpHistoryMs = 1000;
}

int main()
{
    MyCaller *caller = new MyCaller();
    return 0;
}
