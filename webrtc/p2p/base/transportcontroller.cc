/*
 *  Copyright 2015 The WebRTC Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/p2p/base/transportcontroller.h"

#include "webrtc/base/bind.h"
#include "webrtc/base/checks.h"
#include "webrtc/base/thread.h"
#include "webrtc/p2p/base/dtlstransport.h"
#include "webrtc/p2p/base/p2ptransport.h"

namespace cricket {

enum {
  MSG_ICECONNECTIONSTATE,
  MSG_RECEIVING,
  MSG_ICEGATHERINGSTATE,
  MSG_CANDIDATESGATHERED,
};

struct CandidatesData : public rtc::MessageData {
  CandidatesData(const std::string& transport_name,
                 const Candidates& candidates)
      : transport_name(transport_name), candidates(candidates) {}

  std::string transport_name;
  Candidates candidates;
};

TransportController::TransportController(rtc::Thread* signaling_thread,
                                         rtc::Thread* worker_thread,
                                         PortAllocator* port_allocator)
    : signaling_thread_(signaling_thread),
      worker_thread_(worker_thread),
      port_allocator_(port_allocator) {}

TransportController::~TransportController() {
  worker_thread_->Invoke<void>(
      rtc::Bind(&TransportController::DestroyAllTransports_w, this));
  signaling_thread_->Clear(this);
}

bool TransportController::SetSslMaxProtocolVersion(
    rtc::SSLProtocolVersion version) {
  return worker_thread_->Invoke<bool>(rtc::Bind(
      &TransportController::SetSslMaxProtocolVersion_w, this, version));
}

void TransportController::SetIceConnectionReceivingTimeout(int timeout_ms) {
  worker_thread_->Invoke<void>(
      rtc::Bind(&TransportController::SetIceConnectionReceivingTimeout_w, this,
                timeout_ms));
}

void TransportController::SetIceRole(IceRole ice_role) {
  worker_thread_->Invoke<void>(
      rtc::Bind(&TransportController::SetIceRole_w, this, ice_role));
}

bool TransportController::GetSslRole(rtc::SSLRole* role) {
  return worker_thread_->Invoke<bool>(
      rtc::Bind(&TransportController::GetSslRole_w, this, role));
}

bool TransportController::SetLocalCertificate(
    const rtc::scoped_refptr<rtc::RTCCertificate>& certificate) {
  return worker_thread_->Invoke<bool>(rtc::Bind(
      &TransportController::SetLocalCertificate_w, this, certificate));
}

bool TransportController::GetLocalCertificate(
    const std::string& transport_name,
    rtc::scoped_refptr<rtc::RTCCertificate>* certificate) {
  return worker_thread_->Invoke<bool>(
      rtc::Bind(&TransportController::GetLocalCertificate_w, this,
                transport_name, certificate));
}

bool TransportController::GetRemoteSSLCertificate(
    const std::string& transport_name,
    rtc::SSLCertificate** cert) {
  return worker_thread_->Invoke<bool>(
      rtc::Bind(&TransportController::GetRemoteSSLCertificate_w, this,
                transport_name, cert));
}

bool TransportController::SetLocalTransportDescription(
    const std::string& transport_name,
    const TransportDescription& tdesc,
    ContentAction action,
    std::string* err) {
  return worker_thread_->Invoke<bool>(
      rtc::Bind(&TransportController::SetLocalTransportDescription_w, this,
                transport_name, tdesc, action, err));
}

bool TransportController::SetRemoteTransportDescription(
    const std::string& transport_name,
    const TransportDescription& tdesc,
    ContentAction action,
    std::string* err) {
  return worker_thread_->Invoke<bool>(
      rtc::Bind(&TransportController::SetRemoteTransportDescription_w, this,
                transport_name, tdesc, action, err));
}

bool TransportController::AddRemoteCandidates(const std::string& transport_name,
                                              const Candidates& candidates,
                                              std::string* err) {
  return worker_thread_->Invoke<bool>(
      rtc::Bind(&TransportController::AddRemoteCandidates_w, this,
                transport_name, candidates, err));
}

bool TransportController::ReadyForRemoteCandidates(
    const std::string& transport_name) {
  return worker_thread_->Invoke<bool>(rtc::Bind(
      &TransportController::ReadyForRemoteCandidates_w, this, transport_name));
}

bool TransportController::GetStats(const std::string& transport_name,
                                   TransportStats* stats) {
  return worker_thread_->Invoke<bool>(
      rtc::Bind(&TransportController::GetStats_w, this, transport_name, stats));
}

TransportChannel* TransportController::CreateTransportChannel_w(
    const std::string& transport_name,
    int component) {
  RTC_DCHECK(worker_thread_->IsCurrent());

  Transport* transport = GetOrCreateTransport_w(transport_name);
  return transport->CreateChannel(component);
}

void TransportController::DestroyTransportChannel_w(
    const std::string& transport_name,
    int component) {
  RTC_DCHECK(worker_thread_->IsCurrent());

  Transport* transport = GetTransport_w(transport_name);
  if (!transport) {
    ASSERT(false);
    return;
  }
  transport->DestroyChannel(component);

  // Just as we create a Transport when its first channel is created,
  // we delete it when its last channel is deleted.
  if (!transport->HasChannels()) {
    DestroyTransport_w(transport_name);
  }
}

const rtc::scoped_refptr<rtc::RTCCertificate>&
TransportController::certificate_for_testing() {
  return certificate_;
}

Transport* TransportController::CreateTransport_w(
    const std::string& transport_name) {
  RTC_DCHECK(worker_thread_->IsCurrent());

  Transport* transport = new DtlsTransport<P2PTransport>(
      transport_name, port_allocator(), certificate_);
  return transport;
}

Transport* TransportController::GetTransport_w(
    const std::string& transport_name) {
  RTC_DCHECK(worker_thread_->IsCurrent());

  auto iter = transports_.find(transport_name);
  return (iter != transports_.end()) ? iter->second : nullptr;
}

void TransportController::OnMessage(rtc::Message* pmsg) {
  RTC_DCHECK(signaling_thread_->IsCurrent());

  switch (pmsg->message_id) {
    case MSG_ICECONNECTIONSTATE: {
      rtc::TypedMessageData<IceConnectionState>* data =
          static_cast<rtc::TypedMessageData<IceConnectionState>*>(pmsg->pdata);
      SignalConnectionState(data->data());
      delete data;
      break;
    }
    case MSG_RECEIVING: {
      rtc::TypedMessageData<bool>* data =
          static_cast<rtc::TypedMessageData<bool>*>(pmsg->pdata);
      SignalReceiving(data->data());
      delete data;
      break;
    }
    case MSG_ICEGATHERINGSTATE: {
      rtc::TypedMessageData<IceGatheringState>* data =
          static_cast<rtc::TypedMessageData<IceGatheringState>*>(pmsg->pdata);
      SignalGatheringState(data->data());
      delete data;
      break;
    }
    case MSG_CANDIDATESGATHERED: {
      CandidatesData* data = static_cast<CandidatesData*>(pmsg->pdata);
      SignalCandidatesGathered(data->transport_name, data->candidates);
      delete data;
      break;
    }
    default:
      ASSERT(false);
  }
}

Transport* TransportController::GetOrCreateTransport_w(
    const std::string& transport_name) {
  RTC_DCHECK(worker_thread_->IsCurrent());

  Transport* transport = GetTransport_w(transport_name);
  if (transport) {
    return transport;
  }

  transport = CreateTransport_w(transport_name);
  // The stuff below happens outside of CreateTransport_w so that unit tests
  // can override CreateTransport_w to return a different type of transport.
  transport->SetSslMaxProtocolVersion(ssl_max_version_);
  transport->SetChannelReceivingTimeout(ice_receiving_timeout_ms_);
  transport->SetIceRole(ice_role_);
  transport->SetIceTiebreaker(ice_tiebreaker_);
  if (certificate_) {
    transport->SetLocalCertificate(certificate_);
  }
  transport->SignalConnecting.connect(
      this, &TransportController::OnTransportConnecting_w);
  transport->SignalWritableState.connect(
      this, &TransportController::OnTransportWritableState_w);
  transport->SignalReceivingState.connect(
      this, &TransportController::OnTransportReceivingState_w);
  transport->SignalCompleted.connect(
      this, &TransportController::OnTransportCompleted_w);
  transport->SignalFailed.connect(this,
                                  &TransportController::OnTransportFailed_w);
  transport->SignalGatheringState.connect(
      this, &TransportController::OnTransportGatheringState_w);
  transport->SignalCandidatesGathered.connect(
      this, &TransportController::OnTransportCandidatesGathered_w);
  transport->SignalRoleConflict.connect(
      this, &TransportController::OnTransportRoleConflict_w);
  transports_[transport_name] = transport;

  return transport;
}

void TransportController::DestroyTransport_w(
    const std::string& transport_name) {
  RTC_DCHECK(worker_thread_->IsCurrent());

  auto iter = transports_.find(transport_name);
  if (iter != transports_.end()) {
    delete iter->second;
    transports_.erase(transport_name);
  }
  // Destroying a transport may cause aggregate state to change.
  UpdateAggregateStates_w();
}

void TransportController::DestroyAllTransports_w() {
  RTC_DCHECK(worker_thread_->IsCurrent());

  for (const auto& kv : transports_) {
    delete kv.second;
  }
  transports_.clear();
}

bool TransportController::SetSslMaxProtocolVersion_w(
    rtc::SSLProtocolVersion version) {
  RTC_DCHECK(worker_thread_->IsCurrent());

  // Max SSL version can only be set before transports are created.
  if (!transports_.empty()) {
    return false;
  }

  ssl_max_version_ = version;
  return true;
}

void TransportController::SetIceConnectionReceivingTimeout_w(int timeout_ms) {
  RTC_DCHECK(worker_thread_->IsCurrent());
  ice_receiving_timeout_ms_ = timeout_ms;
  for (const auto& kv : transports_) {
    kv.second->SetChannelReceivingTimeout(timeout_ms);
  }
}

void TransportController::SetIceRole_w(IceRole ice_role) {
  RTC_DCHECK(worker_thread_->IsCurrent());
  ice_role_ = ice_role;
  for (const auto& kv : transports_) {
    kv.second->SetIceRole(ice_role_);
  }
}

bool TransportController::GetSslRole_w(rtc::SSLRole* role) {
  RTC_DCHECK(worker_thread()->IsCurrent());

  if (transports_.empty()) {
    return false;
  }
  return transports_.begin()->second->GetSslRole(role);
}

bool TransportController::SetLocalCertificate_w(
    const rtc::scoped_refptr<rtc::RTCCertificate>& certificate) {
  RTC_DCHECK(worker_thread_->IsCurrent());

  if (certificate_) {
    return false;
  }
  if (!certificate) {
    return false;
  }
  certificate_ = certificate;

  for (const auto& kv : transports_) {
    kv.second->SetLocalCertificate(certificate_);
  }
  return true;
}

bool TransportController::GetLocalCertificate_w(
    const std::string& transport_name,
    rtc::scoped_refptr<rtc::RTCCertificate>* certificate) {
  RTC_DCHECK(worker_thread_->IsCurrent());

  Transport* t = GetTransport_w(transport_name);
  if (!t) {
    return false;
  }

  return t->GetLocalCertificate(certificate);
}

bool TransportController::GetRemoteSSLCertificate_w(
    const std::string& transport_name,
    rtc::SSLCertificate** cert) {
  RTC_DCHECK(worker_thread_->IsCurrent());

  Transport* t = GetTransport_w(transport_name);
  if (!t) {
    return false;
  }

  return t->GetRemoteSSLCertificate(cert);
}

bool TransportController::SetLocalTransportDescription_w(
    const std::string& transport_name,
    const TransportDescription& tdesc,
    ContentAction action,
    std::string* err) {
  RTC_DCHECK(worker_thread()->IsCurrent());

  Transport* transport = GetTransport_w(transport_name);
  if (!transport) {
    // If we didn't find a transport, that's not an error;
    // it could have been deleted as a result of bundling.
    // TODO(deadbeef): Make callers smarter so they won't attempt to set a
    // description on a deleted transport.
    return true;
  }

  return transport->SetLocalTransportDescription(tdesc, action, err);
}

bool TransportController::SetRemoteTransportDescription_w(
    const std::string& transport_name,
    const TransportDescription& tdesc,
    ContentAction action,
    std::string* err) {
  RTC_DCHECK(worker_thread()->IsCurrent());

  Transport* transport = GetTransport_w(transport_name);
  if (!transport) {
    // If we didn't find a transport, that's not an error;
    // it could have been deleted as a result of bundling.
    // TODO(deadbeef): Make callers smarter so they won't attempt to set a
    // description on a deleted transport.
    return true;
  }

  return transport->SetRemoteTransportDescription(tdesc, action, err);
}

bool TransportController::AddRemoteCandidates_w(
    const std::string& transport_name,
    const Candidates& candidates,
    std::string* err) {
  RTC_DCHECK(worker_thread()->IsCurrent());

  Transport* transport = GetTransport_w(transport_name);
  if (!transport) {
    // If we didn't find a transport, that's not an error;
    // it could have been deleted as a result of bundling.
    return true;
  }

  return transport->AddRemoteCandidates(candidates, err);
}

bool TransportController::ReadyForRemoteCandidates_w(
    const std::string& transport_name) {
  RTC_DCHECK(worker_thread()->IsCurrent());

  Transport* transport = GetTransport_w(transport_name);
  if (!transport) {
    return false;
  }
  return transport->ready_for_remote_candidates();
}

bool TransportController::GetStats_w(const std::string& transport_name,
                                     TransportStats* stats) {
  RTC_DCHECK(worker_thread()->IsCurrent());

  Transport* transport = GetTransport_w(transport_name);
  if (!transport) {
    return false;
  }
  return transport->GetStats(stats);
}

void TransportController::OnTransportConnecting_w(Transport* transport) {
  RTC_DCHECK(worker_thread_->IsCurrent());
  UpdateAggregateStates_w();
}

void TransportController::OnTransportWritableState_w(Transport* transport) {
  RTC_DCHECK(worker_thread_->IsCurrent());
  UpdateAggregateStates_w();
}

void TransportController::OnTransportReceivingState_w(Transport* transport) {
  RTC_DCHECK(worker_thread_->IsCurrent());
  UpdateAggregateStates_w();
}

void TransportController::OnTransportCompleted_w(Transport* transport) {
  RTC_DCHECK(worker_thread_->IsCurrent());
  UpdateAggregateStates_w();
}

void TransportController::OnTransportFailed_w(Transport* transport) {
  RTC_DCHECK(worker_thread_->IsCurrent());
  UpdateAggregateStates_w();
}

void TransportController::OnTransportGatheringState_w(Transport* transport) {
  RTC_DCHECK(worker_thread_->IsCurrent());
  UpdateAggregateStates_w();
}

void TransportController::OnTransportCandidatesGathered_w(
    Transport* transport,
    const std::vector<Candidate>& candidates) {
  RTC_DCHECK(worker_thread_->IsCurrent());
  CandidatesData* data = new CandidatesData(transport->name(), candidates);
  signaling_thread_->Post(this, MSG_CANDIDATESGATHERED, data);
}

void TransportController::OnTransportRoleConflict_w() {
  RTC_DCHECK(worker_thread_->IsCurrent());

  if (ice_role_switch_) {
    LOG(LS_WARNING) << "Repeat of role conflict signal from Transport.";
    return;
  }

  ice_role_switch_ = true;
  IceRole reversed_role = (ice_role_ == ICEROLE_CONTROLLING)
                              ? ICEROLE_CONTROLLED
                              : ICEROLE_CONTROLLING;
  for (const auto& kv : transports_) {
    kv.second->SetIceRole(reversed_role);
  }
}

void TransportController::UpdateAggregateStates_w() {
  RTC_DCHECK(worker_thread_->IsCurrent());

  IceConnectionState new_connection_state = kIceConnectionConnecting;
  IceGatheringState new_gathering_state = kIceGatheringNew;
  bool any_receiving = false;
  bool any_failed = false;
  bool all_connected = HasChannels_w();
  bool all_completed = HasChannels_w();
  bool any_gathering = false;
  bool all_done_gathering = HasChannels_w();
  for (const auto& kv : transports_) {
    // Ignore transports without channels since they're about to be deleted,
    // and their state is meaningless.
    if (!kv.second->HasChannels()) {
      continue;
    }
    any_receiving = any_receiving || kv.second->any_channel_receiving();
    any_failed = any_failed || kv.second->AnyChannelFailed();
    all_connected = all_connected && kv.second->all_channels_writable();
    all_completed = all_completed && kv.second->AllChannelsCompleted();
    any_gathering =
        any_gathering || kv.second->gathering_state() != kIceGatheringNew;
    all_done_gathering = all_done_gathering &&
                         kv.second->gathering_state() == kIceGatheringComplete;
  }

  if (any_failed) {
    new_connection_state = kIceConnectionFailed;
  } else if (all_completed) {
    new_connection_state = kIceConnectionCompleted;
  } else if (all_connected) {
    new_connection_state = kIceConnectionConnected;
  }
  if (connection_state_ != new_connection_state) {
    connection_state_ = new_connection_state;
    signaling_thread_->Post(
        this, MSG_ICECONNECTIONSTATE,
        new rtc::TypedMessageData<IceConnectionState>(new_connection_state));
  }

  if (receiving_ != any_receiving) {
    receiving_ = any_receiving;
    signaling_thread_->Post(this, MSG_RECEIVING,
                            new rtc::TypedMessageData<bool>(any_receiving));
  }

  if (all_done_gathering) {
    new_gathering_state = kIceGatheringComplete;
  } else if (any_gathering) {
    new_gathering_state = kIceGatheringGathering;
  }
  if (gathering_state_ != new_gathering_state) {
    gathering_state_ = new_gathering_state;
    signaling_thread_->Post(
        this, MSG_ICEGATHERINGSTATE,
        new rtc::TypedMessageData<IceGatheringState>(new_gathering_state));
  }
}

bool TransportController::HasChannels_w() {
  for (const auto& kv : transports_) {
    if (kv.second->HasChannels()) {
      return true;
    }
  }
  return false;
}

}  // namespace cricket