/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 INRIA
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
 * Author: Miralem Mehic <miralem.mehic@ieee.org>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "sptm-header.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SptmHeader");

NS_OBJECT_ENSURE_REGISTERED (SptmHeader);

SptmHeader::SptmHeader ()
  : m_seq (0),
    m_ts (Simulator::Now ().GetTimeStep ()),
    m_messageType (AAA_ACCESS_REQUEST)
{
  NS_LOG_FUNCTION (this);
}

void
SptmHeader::SetSeq (uint32_t seq)
{
  NS_LOG_FUNCTION (this << seq);
  m_seq = seq;
}
uint32_t
SptmHeader::GetSeq (void) const
{
  NS_LOG_FUNCTION (this);
  return m_seq;
}

void
SptmHeader::SetFlow (uint32_t flow)
{
  NS_LOG_FUNCTION (this << flow);
  m_flow = flow;
}
uint32_t
SptmHeader::GetFlow (void) const
{
  NS_LOG_FUNCTION (this);
  return m_flow;
}
 
Time
SptmHeader::GetTs (void) const
{
  NS_LOG_FUNCTION (this);
  return TimeStep (m_ts);
}

void
SptmHeader::SetMessageType (uint8_t type)
{
  NS_LOG_FUNCTION (this<< (uint32_t)type);
  m_messageType = type;
}

uint8_t
SptmHeader::GetMessageType (void) const
{
  NS_LOG_FUNCTION (this);
  return m_messageType;
}

TypeId
SptmHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SptmHeader")
    .SetParent<Header> ()
    .SetGroupName("Applications")
    .AddConstructor<SptmHeader> ()
  ;
  return tid;
}
TypeId
SptmHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}
void
SptmHeader::Print (std::ostream &os) const
{
  NS_LOG_FUNCTION (this << &os);
  std::string msgType;
  switch (m_messageType) {
    case AAA_ACCESS_REQUEST : msgType = "ACCESS-REQUEST"; break;
    case AAA_ACCESS_ACCEPT : msgType = "ACCESS-ACCEPT"; break;
    case AAA_ACCESS_REJECT : msgType = "ACCESS-REJECT"; break;
    case AAA_ACCOUNTING_REQUEST : msgType = "ACCOUNTING-REQUEST"; break;
    case AAA_ACCOUNTING_RESPONSE : msgType = "ACCOUNTING-RESPONSE"; break;
    default: msgType = "UNKNOWN"; break;
  }
  os << "(Type=" << msgType << "flow =" << m_flow << " seq="<<m_seq << " time="<< TimeStep (m_ts).GetSeconds() << ")";
}
uint32_t
SptmHeader::GetSerializedSize (void) const
{
  NS_LOG_FUNCTION (this);
  return 2 * sizeof(uint32_t) + sizeof(uint64_t) + sizeof(uint8_t); //4+4+8+1
}

void
SptmHeader::Serialize (Buffer::Iterator start) const
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_flow);
  i.WriteHtonU32 (m_seq);
  i.WriteHtonU64 (m_ts);
  i.WriteU8 (m_messageType);
}
uint32_t
SptmHeader::Deserialize (Buffer::Iterator start)
{
  NS_LOG_FUNCTION (this << &start);
  Buffer::Iterator i = start;
  m_flow = i.ReadNtohU32 ();
  m_seq = i.ReadNtohU32 ();
  m_ts = i.ReadNtohU64 ();
  m_messageType = i.ReadU8();
  return GetSerializedSize ();
}

} // namespace ns3
