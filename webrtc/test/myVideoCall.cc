#include "webrtc/common.h"
#include "webrtc/config.h"
#include "webrtc/video_receive_stream.h"
#include "webrtc/video_send_stream.h"
using namespace webrtc;
class VideoCall {
	struct VideoConfig {
		VideoSendStream::Config 	send_config_;
		VideoEncoderConfig 			encoder_config_;
		VideoReceiveStream::Config 	recv_config;
	}video_config_;

	int32_t Init();
	int32_t Destory();
	int32_t CreateVideoCallConfig();
	int32_t UpdateVideoCallConfig();

	int32_t CreateVideoCall();
	int32_t DestoryVideoCall();
};



