/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 */

#ifndef SPTM_CLIENT_H
#define SPTM_CLIENT_H
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "sptm-header.h"

namespace ns3 {

enum AaaClientState {
  AAA_IDLE =0,
  AAA_AUTHENTICATING =1,
  AAA_AUTHENTICATED =2,
  AAA_AUTHORIZING =3,
  AAA_AUTHORIZED =4
};

class Socket;
class Packet;
class SptmHeader; //!< Forward declaration za header koji se koristi u Send()

/**
 * \ingroup SptmClientserver
 *
 * \brief A UDP client for AAA. Sends UDP packets carrying sequence number, time stamp,
 *        and supports retransmissions at the application level.
 */
class SptmClient : public Application
{
public:
  static TypeId GetTypeId (void);

  SptmClient ();
  virtual ~SptmClient ();

  void SetRemote (Address ip, uint16_t port);
  void SetRemote (Address addr);

  // Getter methods for metrics
  double GetASR () const;
  uint32_t GetTotalRetransmissions () const;

protected:
  virtual void DoDispose (void);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void StartAaaSession();
  void SendAccessRequest();
  void HandleRead (Ptr<Socket> socket);
  void HandleAccessAccept (SptmHeader header);
  void HandleAccessReject (SptmHeader header);
  void HandleTimeout ();

  // Basic flow parameters
  uint32_t m_flow;
  uint32_t m_count; //!< Maximum number of packets the application will send
  Time m_interval;  //!< Packet inter-send time
  uint32_t m_size;  //!< Size of the sent packet (including header)

  // Socket info
  Ptr<Socket> m_socket; //!< Socket
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort;   //!< Remote peer port
  EventId m_sendEvent;   //!< Event to send next packet

  // Metrics & tracing
  uint32_t m_sent;
  uint32_t m_retries;
  uint32_t m_authRequests;
  uint32_t m_authSuccess;
  uint32_t m_authFailures;
  uint32_t m_totalRetransmissions;
  Time m_authStartTime;

  // Timeout handling
  EventId m_timeoutEvent;
  Time m_timeout;
  uint32_t m_maxRetries;
  
  AaaClientState m_state;
  // Trace
  TracedCallback<Ptr<const Packet>> m_txTrace;
};

} // namespace ns3

#endif /* SPTM_CLIENT_H */
