/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 * Author:  Tom Henderson (tomhend@u.washington.edu)
 */
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "sptm-sink.h"
#include "ns3/sptm-header.h"
#include "ns3/boolean.h"
#include "ns3/double.h"
#include "ns3/config.h"

namespace ns3
{

NS_LOG_COMPONENT_DEFINE ("SptmSink");

NS_OBJECT_ENSURE_REGISTERED (SptmSink);

TypeId
SptmSink::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SptmSink")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<SptmSink> ()
    .AddAttribute ("Local",
                   "The Address on which to Bind the rx socket.",
                   AddressValue (),
                   MakeAddressAccessor (&SptmSink::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol",
                   "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (UdpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&SptmSink::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute  ("SendResponse",
                   "Wheather server should send AAA responses",
                   BooleanValue (true),
                   MakeBooleanAccessor (&SptmSink::m_sendResponse),
                   MakeBooleanChecker())
    .AddAttribute  ("ProcessingDelay",
                   "Server processing delay before sending response",
                   TimeValue (MilliSeconds (10)),
                   MakeTimeAccessor (&SptmSink::m_processingDelay),
                   MakeTimeChecker())
    .AddAttribute  ("RejectProbability",
                   "Probability of rejecting authentication (0.0-1.0)",
                   DoubleValue (0.0),
                   MakeDoubleAccessor (&SptmSink::m_rejectProbability),
                   MakeDoubleChecker <double> (0.0,1.0))
    .AddTraceSource ("Rx",
                     "A packet has been received",
                     MakeTraceSourceAccessor (&SptmSink::m_rxTrace),
                     "ns3::Packet::AddressTracedCallback")
    .AddTraceSource ("Tx",
                     "A packet has been sent",
                     MakeTraceSourceAccessor (&SptmSink::m_txTrace),
                     "ns3::Packet::AddressTracedCallback")
  ;
  return tid;
}

SptmSink::SptmSink ()
 : m_sendResponse (true),
   m_processingDelay (MilliSeconds(10)),
   m_rejectProbability (0.0),
   m_accessRequests (0),
   m_accessAccepts (0),
   m_accessRejects (0)
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_totalRx = 0;
  m_uniformRv = CreateObject <UniformRandomVariable> ();
}

SptmSink::~SptmSink()
{
  NS_LOG_FUNCTION (this);
}

uint64_t SptmSink::GetTotalRx () const
{
  NS_LOG_FUNCTION (this);
  return m_totalRx;
}

Ptr<Socket>
SptmSink::GetListeningSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

std::list<Ptr<Socket> >
SptmSink::GetAcceptedSockets (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socketList;
}

void SptmSink::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_socket = nullptr;
  m_socketList.clear ();

  // chain up
  Application::DoDispose ();
}


// Application Methods
void SptmSink::StartApplication ()    // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);
  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      if (m_socket->Bind (m_local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      m_socket->Listen ();
      //m_socket->ShutdownSend ();
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: joining multicast on a non-UDP socket");
            }
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&SptmSink::HandleRead, this));
  m_socket->SetAcceptCallback (
    MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
    MakeCallback (&SptmSink::HandleAccept, this));
  m_socket->SetCloseCallbacks (
    MakeCallback (&SptmSink::HandlePeerClose, this),
    MakeCallback (&SptmSink::HandlePeerError, this));
  
  // Inicijalizuj trace za Access Accept i Access Reject
  //Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::SptmSink/AccessAccept", 
  //                 MakeCallback (&m_accessAcceptTrace));
  //Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::SptmSink/AccessReject", 
  //                 MakeCallback (&m_accessRejectTrace));
                   
                   
}

void SptmSink::StopApplication ()     // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("=== AAA SERVER STATISTICS ===");
  NS_LOG_INFO ("Access Requests: "<< m_accessRequests);
  NS_LOG_INFO ("Access Accepts: "<< m_accessAccepts);
  NS_LOG_INFO ("Access Rejects: "<< m_accessRejects);
  
  while(!m_socketList.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_socketList.front ();
      m_socketList.pop_front ();
      acceptedSocket->Close ();
    }
  if (m_socket)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void SptmSink::SendAaaResponse (Ptr<Socket> socket, Address to, SptmHeader requestHeader) {
  NS_LOG_FUNCTION (this);
  
  bool accept = (m_uniformRv -> GetValue () > m_rejectProbability);
  
  SptmHeader responseHeader;
  responseHeader.SetSeq (requestHeader.GetSeq());
  responseHeader.SetFlow (requestHeader.GetFlow());
  
  if (accept){
    responseHeader.SetMessageType (AAA_ACCESS_ACCEPT);
    m_accessAccepts++;
    m_accessAcceptCount++;
    m_accessAcceptTrace (m_accessAcceptCount);
    NS_LOG_INFO ("Sending ACCESS-ACCEPT for rquest #" <<requestHeader.GetSeq());
  }
  else{
    responseHeader.SetMessageType (AAA_ACCESS_REJECT);
    m_accessRejects++;
    m_accessRejectCount++;
    m_accessRejectTrace (m_accessRejectCount);
    NS_LOG_INFO ("Sending ACCESS-REJECT for rquest #" <<requestHeader.GetSeq());
  }
  Ptr <Packet> responsePacket = Create<Packet> (100);
  responsePacket->AddHeader(responseHeader);
  
  
  if (InetSocketAddress::IsMatchingType (to)){
    NS_LOG_UNCOND ("Is Matching Type: " << InetSocketAddress::IsMatchingType(to));
    int bytesSent = socket->SendTo (responsePacket,0,to);
    NS_LOG_UNCOND ("Bytes sent: " << bytesSent);
    m_txTrace (responsePacket,to);
  }
}

void SptmSink::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->GetSize () == 0)
        { //EOF
          break;
        }

      m_rxTrace (packet, from);
     
      SptmHeader sptmHead;
      packet->RemoveHeader(sptmHead);
      NS_ASSERT (sptmHead.GetSeq() >= 0 );
 
      m_totalRx += packet->GetSize ();

      NS_LOG_INFO ("Received AAA message: " << sptmHead);
      if (sptmHead.GetMessageType() == AAA_ACCESS_REQUEST){
        m_accessRequests++;
        NS_LOG_INFO ("Processing ACCESS-REQUEST " << sptmHead.GetSeq());
        
        if (m_sendResponse){
          Simulator::Schedule (m_processingDelay,&SptmSink::SendAaaResponse,this,socket,from,sptmHead);
          
        }
      }
      else if (sptmHead.GetMessageType () == AAA_ACCOUNTING_REQUEST){
        NS_LOG_INFO ("Received ACCOUNTING-REQUEST");
      }
      if (InetSocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << InetSocketAddress::ConvertFrom(from).GetIpv4 ()
                       << " port " << InetSocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds ()
                       << "s packet sink received "
                       <<  packet->GetSize () << " bytes from "
                       << Inet6SocketAddress::ConvertFrom(from).GetIpv6 ()
                       << " port " << Inet6SocketAddress::ConvertFrom (from).GetPort ()
                       << " total Rx " << m_totalRx << " bytes");
        }
    }
}


void SptmSink::HandlePeerClose (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 
void SptmSink::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}
 

void SptmSink::HandleAccept (Ptr<Socket> s, const Address& from)
{
  NS_LOG_FUNCTION (this << s << from);
  s->SetRecvCallback (MakeCallback (&SptmSink::HandleRead, this));
  m_socketList.push_back (s);
}

}
