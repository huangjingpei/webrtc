/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/modules/audio_coding/neteq/decoder_database.h"

#include <assert.h>
#include <utility>  // pair

#include "webrtc/base/checks.h"
#include "webrtc/base/logging.h"
#include "webrtc/modules/audio_coding/codecs/audio_decoder.h"

namespace webrtc {

DecoderDatabase::DecoderDatabase()
    : active_decoder_type_(-1), active_cng_decoder_type_(-1) {
}

DecoderDatabase::~DecoderDatabase() = default;

DecoderDatabase::DecoderInfo::DecoderInfo(NetEqDecoder ct,
                                          const std::string& nm,
                                          int fs,
                                          AudioDecoder* ext_dec)
    : codec_type(ct),
      name(nm),
      fs_hz(fs),
      external_decoder(ext_dec) {}

DecoderDatabase::DecoderInfo::DecoderInfo(DecoderInfo&&) = default;
DecoderDatabase::DecoderInfo::~DecoderInfo() = default;

AudioDecoder* DecoderDatabase::DecoderInfo::GetDecoder() {
  if (external_decoder) {
    RTC_DCHECK(!decoder_);
    return external_decoder;
  }
  if (!decoder_) {
    decoder_.reset(CreateAudioDecoder(codec_type));
  }
  RTC_DCHECK(decoder_);
  return decoder_.get();
}

bool DecoderDatabase::Empty() const { return decoders_.empty(); }

int DecoderDatabase::Size() const { return static_cast<int>(decoders_.size()); }

void DecoderDatabase::Reset() {
  decoders_.clear();
  active_decoder_type_ = -1;
  active_cng_decoder_type_ = -1;
}

int DecoderDatabase::RegisterPayload(uint8_t rtp_payload_type,
                                     NetEqDecoder codec_type,
                                     const std::string& name) {
  if (rtp_payload_type > 0x7F) {
    return kInvalidRtpPayloadType;
  }
  if (!CodecSupported(codec_type)) {
    return kCodecNotSupported;
  }
  const int fs_hz = CodecSampleRateHz(codec_type);
  DecoderInfo info(codec_type, name, fs_hz, nullptr);
  auto ret =
      decoders_.insert(std::make_pair(rtp_payload_type, std::move(info)));
  if (ret.second == false) {
    // Database already contains a decoder with type |rtp_payload_type|.
    return kDecoderExists;
  }
  return kOK;
}

int DecoderDatabase::InsertExternal(uint8_t rtp_payload_type,
                                    NetEqDecoder codec_type,
                                    const std::string& codec_name,
                                    int fs_hz,
                                    AudioDecoder* decoder) {
  if (rtp_payload_type > 0x7F) {
    return kInvalidRtpPayloadType;
  }
  if (!CodecSupported(codec_type)) {
    return kCodecNotSupported;
  }
  if (fs_hz != 8000 && fs_hz != 16000 && fs_hz != 32000 && fs_hz != 48000) {
    return kInvalidSampleRate;
  }
  if (!decoder) {
    return kInvalidPointer;
  }
  std::pair<DecoderMap::iterator, bool> ret;
  DecoderInfo info(codec_type, codec_name, fs_hz, decoder);
  ret = decoders_.insert(std::make_pair(rtp_payload_type, std::move(info)));
  if (ret.second == false) {
    // Database already contains a decoder with type |rtp_payload_type|.
    return kDecoderExists;
  }
  return kOK;
}

int DecoderDatabase::Remove(uint8_t rtp_payload_type) {
  if (decoders_.erase(rtp_payload_type) == 0) {
    // No decoder with that |rtp_payload_type|.
    return kDecoderNotFound;
  }
  if (active_decoder_type_ == rtp_payload_type) {
    active_decoder_type_ = -1;  // No active decoder.
  }
  if (active_cng_decoder_type_ == rtp_payload_type) {
    active_cng_decoder_type_ = -1;  // No active CNG decoder.
  }
  return kOK;
}

const DecoderDatabase::DecoderInfo* DecoderDatabase::GetDecoderInfo(
    uint8_t rtp_payload_type) const {
  DecoderMap::const_iterator it = decoders_.find(rtp_payload_type);
  if (it == decoders_.end()) {
    // Decoder not found.
    return NULL;
  }
  return &(*it).second;
}

uint8_t DecoderDatabase::GetRtpPayloadType(
    NetEqDecoder codec_type) const {
  DecoderMap::const_iterator it;
  for (it = decoders_.begin(); it != decoders_.end(); ++it) {
    if ((*it).second.codec_type == codec_type) {
      // Match found.
      return (*it).first;
    }
  }
  // No match.
  return kRtpPayloadTypeError;
}

AudioDecoder* DecoderDatabase::GetDecoder(uint8_t rtp_payload_type) {
  if (IsDtmf(rtp_payload_type) || IsRed(rtp_payload_type) ||
      IsComfortNoise(rtp_payload_type)) {
    // These are not real decoders.
    return NULL;
  }
  DecoderMap::iterator it = decoders_.find(rtp_payload_type);
  if (it == decoders_.end()) {
    // Decoder not found.
    return NULL;
  }
  DecoderInfo* info = &(*it).second;
  return info->GetDecoder();
}

bool DecoderDatabase::IsType(uint8_t rtp_payload_type,
                             NetEqDecoder codec_type) const {
  DecoderMap::const_iterator it = decoders_.find(rtp_payload_type);
  if (it == decoders_.end()) {
    // Decoder not found.
    return false;
  }
  return ((*it).second.codec_type == codec_type);
}

bool DecoderDatabase::IsComfortNoise(uint8_t rtp_payload_type) const {
  DecoderMap::const_iterator it = decoders_.find(rtp_payload_type);
  if (it == decoders_.end()) {
    // Decoder not found.
    return false;
  }
  const auto& type = it->second.codec_type;
  return type == NetEqDecoder::kDecoderCNGnb
      || type == NetEqDecoder::kDecoderCNGwb
      || type == NetEqDecoder::kDecoderCNGswb32kHz
      || type == NetEqDecoder::kDecoderCNGswb48kHz;
}

bool DecoderDatabase::IsDtmf(uint8_t rtp_payload_type) const {
  return IsType(rtp_payload_type, NetEqDecoder::kDecoderAVT);
}

bool DecoderDatabase::IsRed(uint8_t rtp_payload_type) const {
  return IsType(rtp_payload_type, NetEqDecoder::kDecoderRED);
}

int DecoderDatabase::SetActiveDecoder(uint8_t rtp_payload_type,
                                      bool* new_decoder) {
  // Check that |rtp_payload_type| exists in the database.
  DecoderMap::const_iterator it = decoders_.find(rtp_payload_type);
  if (it == decoders_.end()) {
    // Decoder not found.
    return kDecoderNotFound;
  }
  RTC_CHECK(!IsComfortNoise(rtp_payload_type));
  assert(new_decoder);
  *new_decoder = false;
  if (active_decoder_type_ < 0) {
    // This is the first active decoder.
    *new_decoder = true;
  } else if (active_decoder_type_ != rtp_payload_type) {
    // Moving from one active decoder to another. Delete the first one.
    DecoderMap::iterator it = decoders_.find(active_decoder_type_);
    if (it == decoders_.end()) {
      // Decoder not found. This should not be possible.
      assert(false);
      return kDecoderNotFound;
    }
    it->second.DropDecoder();
    *new_decoder = true;
  }
  active_decoder_type_ = rtp_payload_type;
  return kOK;
}

AudioDecoder* DecoderDatabase::GetActiveDecoder() {
  if (active_decoder_type_ < 0) {
    // No active decoder.
    return NULL;
  }
  return GetDecoder(active_decoder_type_);
}

int DecoderDatabase::SetActiveCngDecoder(uint8_t rtp_payload_type) {
  // Check that |rtp_payload_type| exists in the database.
  DecoderMap::const_iterator it = decoders_.find(rtp_payload_type);
  if (it == decoders_.end()) {
    // Decoder not found.
    return kDecoderNotFound;
  }
  if (active_cng_decoder_type_ >= 0 &&
      active_cng_decoder_type_ != rtp_payload_type) {
    // Moving from one active CNG decoder to another. Delete the first one.
    DecoderMap::iterator it = decoders_.find(active_cng_decoder_type_);
    if (it == decoders_.end()) {
      // Decoder not found. This should not be possible.
      assert(false);
      return kDecoderNotFound;
    }
    // The CNG decoder should never be provided externally.
    RTC_CHECK(!it->second.external_decoder);
    active_cng_decoder_.reset();
  }
  active_cng_decoder_type_ = rtp_payload_type;
  return kOK;
}

ComfortNoiseDecoder* DecoderDatabase::GetActiveCngDecoder() {
  if (active_cng_decoder_type_ < 0) {
    // No active CNG decoder.
    return NULL;
  }
  if (!active_cng_decoder_) {
    active_cng_decoder_.reset(new ComfortNoiseDecoder);
  }
  return active_cng_decoder_.get();
}

int DecoderDatabase::CheckPayloadTypes(const PacketList& packet_list) const {
  PacketList::const_iterator it;
  for (it = packet_list.begin(); it != packet_list.end(); ++it) {
    if (decoders_.find((*it)->header.payloadType) == decoders_.end()) {
      // Payload type is not found.
      LOG(LS_WARNING) << "CheckPayloadTypes: unknown RTP payload type "
                      << static_cast<int>((*it)->header.payloadType);
      return kDecoderNotFound;
    }
  }
  return kOK;
}


}  // namespace webrtc
