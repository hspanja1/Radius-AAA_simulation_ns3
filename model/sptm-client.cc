/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007,2008,2009 INRIA, UDCAST
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Amine Ismail <amine.ismail@sophia.inria.fr>
 *                      <amine.ismail@udcast.com>
 */
#include "ns3/sptm-client.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/attribute.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/socket.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/sptm-header.h"
#include "ns3/log.h"


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SptmClient");
NS_OBJECT_ENSURE_REGISTERED (SptmClient);

TypeId
SptmClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SptmClient")
    .SetParent<Application> ()
    .SetGroupName ("Applications")
    .AddConstructor<SptmClient> ()

    .AddAttribute ("MaxPackets",
                   "Maximum number of AAA requests",
                   UintegerValue (1),
                   MakeUintegerAccessor (&SptmClient::m_count),
                   MakeUintegerChecker<uint32_t> ())

    .AddAttribute ("Interval",
                   "Interval between AAA requests",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&SptmClient::m_interval),
                   MakeTimeChecker ())

    .AddAttribute ("RemoteAddress",
                   "RADIUS server address",
                   AddressValue (),
                   MakeAddressAccessor (&SptmClient::m_peerAddress),
                   MakeAddressChecker ())

    .AddAttribute ("RemotePort",
                   "RADIUS server port",
                   UintegerValue (1812),
                   MakeUintegerAccessor (&SptmClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())

    .AddAttribute ("PacketSize",
                   "AAA packet size",
                   UintegerValue (150),
                   MakeUintegerAccessor (&SptmClient::m_size),
                   MakeUintegerChecker<uint32_t> (16,1500))

    .AddAttribute ("MaxRetries",
                   "Maximum AAA retransmissions",
                   UintegerValue (5),
                   MakeUintegerAccessor (&SptmClient::m_maxRetries),
                   MakeUintegerChecker<uint32_t>())

    .AddAttribute ("Timeout",
                   "AAA timeout",
                   TimeValue (MilliSeconds (500)),
                   MakeTimeAccessor (&SptmClient::m_timeout),
                   MakeTimeChecker ())

    .AddAttribute ("FlowNumber",
                   "The Flow ID of this client",
                   UintegerValue (0),
                   MakeUintegerAccessor (&SptmClient::m_flow),
                   MakeUintegerChecker<uint32_t>())

    .AddTraceSource ("Tx",
                     "Packet transmitted",
                     MakeTraceSourceAccessor (&SptmClient::m_txTrace),
                     "ns3::Packet::AddressTracedCallback");

  return tid;
}

SptmClient::SptmClient ()
  : m_socket (0),
    m_count (1),
    m_size (150),
    m_interval (Seconds (1.0)),
    m_sent (0),
    m_flow (0),
    m_retries (0),
    m_state (AAA_IDLE),
    m_authRequests (0),
    m_authSuccess (0),
    m_authFailures(0),
    m_totalRetransmissions (0),
    m_timeout (MilliSeconds (500)),
    m_maxRetries (5)
{
}

SptmClient::~SptmClient ()
{
}

void
SptmClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void
SptmClient::SetRemote (Address ip, uint16_t port)
{
  m_peerAddress = ip;
  m_peerPort = port;
}

void
SptmClient::SetRemote (Address addr)
{
  m_peerAddress = addr;
}

void
SptmClient::StartApplication ()
{
  NS_LOG_FUNCTION(this);  // Dodano logovanje funkcije

  if (!m_socket)  // Ako socket nije kreiran
  {
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");  // Lookup tipa socketa
    m_socket = Socket::CreateSocket (GetNode (), tid);  // Kreiraj socket

    // Provera tipa adrese
    if (Ipv4Address::IsMatchingType(m_peerAddress)) 
    {
      if (m_socket->Bind () == -1)  // Bindanje socketa
      {
        NS_FATAL_ERROR ("Failed to bind socket");  // Greška ako ne uspe
      }
      m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));  // Povezivanje sa IPv4 adresom
    }
    else if (InetSocketAddress::IsMatchingType(m_peerAddress))
    {
      if (m_socket->Bind () == -1)
      {
        NS_FATAL_ERROR ("Failed to bind socket");
      }
      m_socket->Connect (m_peerAddress);  // Povezivanje sa InetSocketAddress
    }
    else
    {
      NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);  // Ako adresa nije odgovarajuća
    }
  }

  m_socket->SetRecvCallback (MakeCallback (&SptmClient::HandleRead, this));  // Callback za primanje podataka
  Simulator::Schedule (Seconds (0.1), &SptmClient::StartAaaSession, this);  // Zakazivanje slanja paketa
}
/*

Ovaj ne radi jer baca grešku kod InetSocketAddress

void
SptmClient::StartApplication ()
{
  
  if (!m_socket)
  {
    TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
    m_socket = Socket::CreateSocket (GetNode (), tid);
    m_socket->Bind ();
    m_socket->Connect (InetSocketAddress (m_peerAddress, m_peerPort));
  }

  m_socket->SetRecvCallback (MakeCallback (&SptmClient::HandleRead, this));

  Simulator::Schedule (Seconds (0.0), &SptmClient::Send, this);
}*/

void
SptmClient::StopApplication ()
{
  Simulator::Cancel (m_sendEvent);
  Simulator::Cancel (m_timeoutEvent);
  NS_LOG_INFO ("=== AAA CLIENT STATISTICS ===");
  NS_LOG_INFO ("Sessions started: "<<m_authRequests);
  NS_LOG_INFO ("Successful authentications: "<<m_authSuccess);
  NS_LOG_INFO ("Failed authentications: "<<m_authFailures);
  NS_LOG_INFO ("Total retransmissions: "<<m_totalRetransmissions);
  NS_LOG_INFO ("ASR: "<<GetASR());
}

void
SptmClient::StartAaaSession(){
  NS_LOG_FUNCTION (this);
  
  if (m_sent >= m_count){
    NS_LOG_INFO ("All AAA sessions completed");
    return;
  }
  
  m_state=AAA_AUTHENTICATING;
  m_retries=0;
  m_authRequests++;
  m_authStartTime = Simulator::Now();
  
  SendAccessRequest();
}

void
SptmClient::SendAccessRequest ()
{
  NS_LOG_FUNCTION (this);
  
  SptmHeader header;
  header.SetSeq (m_sent);
  header.SetFlow (m_flow);
  header.SetMessageType (AAA_ACCESS_REQUEST);
  
  Ptr<Packet> packet = Create<Packet> (m_size - header.GetSerializedSize ());
  packet->AddHeader (header);

  m_socket->Send (packet);
  m_txTrace (packet);

  NS_LOG_INFO ("AAA Request sent (retry=" << m_retries << ")");

  m_timeoutEvent = Simulator::Schedule (m_timeout, &SptmClient::HandleTimeout, this);
}

void
SptmClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this);
  
  Address from;
  Ptr<Packet> packet = socket->RecvFrom (from);
  
  SptmHeader header;
  packet->RemoveHeader (header);
  
  NS_LOG_INFO ("==Received AAA response: "<<header);
  Simulator::Cancel (m_timeoutEvent);
  
  if (header.GetMessageType () == AAA_ACCESS_ACCEPT){
    HandleAccessAccept (header);
  }
  else if (header.GetMessageType() == AAA_ACCESS_REJECT){
    HandleAccessReject (header);
  }
}

void
SptmClient::HandleAccessAccept (SptmHeader header){
  NS_LOG_FUNCTION (this);
  
  Time latency = Simulator::Now () - m_authStartTime;
  
  NS_LOG_INFO ("AUTHENTICATION SUCCESS for session #"<<header.GetSeq()<<" - Latency: "<<latency.GetMilliSeconds() <<" ms");
  
  m_authSuccess++;
  m_state=AAA_AUTHENTICATED;
  
  m_retries=0;
  m_sent++;
  
  if (m_sent < m_count){
    m_sendEvent = Simulator::Schedule (m_interval,&SptmClient::StartAaaSession,this);
  }
}

void
SptmClient::HandleAccessReject (SptmHeader header){
  NS_LOG_FUNCTION (this);
  
  Time latency = Simulator::Now () - m_authStartTime;
  
  NS_LOG_INFO ("AUTHENTICATION Rejected for session #"<<header.GetSeq()<<" - Latency: "<<latency.GetMilliSeconds() <<" ms");
  
  m_authFailures++;
  m_state=AAA_IDLE;
  
  m_retries=0;
  m_sent++;
  
  if (m_sent < m_count){
    m_sendEvent = Simulator::Schedule (m_interval,&SptmClient::StartAaaSession,this);
  }
}

void
SptmClient::HandleTimeout ()
{
  NS_LOG_FUNCTION (this);
  
  if (m_retries < m_maxRetries)
  {
    m_retries++;
    m_totalRetransmissions++;
    NS_LOG_INFO ("AAA TIMEOUT → retransmission " << m_retries);
    SendAccessRequest ();
  }
  else
  {
    NS_LOG_INFO ("AAA SESSION FAILED");
    m_authFailures++;
    m_retries = 0;
    m_sent++;
    m_state = AAA_IDLE;
  }
  if (m_sent < m_count){
    m_sendEvent = Simulator::Schedule (m_interval,&SptmClient::StartAaaSession,this);
  }
}

double
SptmClient::GetASR () const
{
  if (m_authRequests ==0){
    return 0.0;
  }
  return (double)m_authSuccess / (double)m_authRequests;
}

uint32_t
SptmClient::GetTotalRetransmissions () const
{
  return m_totalRetransmissions;
}

} // namespace ns3
