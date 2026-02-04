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

#ifndef SPTM_HEADER_H
#define SPTM_HEADER_H

#include "ns3/header.h"
#include "ns3/nstime.h"

namespace ns3 {

enum AaaMessageType {
   AAA_ACCESS_REQUEST = 1,
   AAA_ACCESS_ACCEPT = 2,
   AAA_ACCESS_REJECT = 3,
   AAA_ACCOUNTING_REQUEST = 4,
   AAA_ACCOUNTING_RESPONSE = 5,
};
/**
 * \ingroup udpclientserver
 *
 * \brief Packet header for UDP client/server application.
 *
 * The header is made of a 32bits sequence number followed by
 * a 64bits time stamp.
 */
class SptmHeader : public Header
{
public:
  SptmHeader ();

  /**
   * \param seq the sequence number
   */
  void SetSeq (uint32_t seq);
  /**
   * \return the sequence number
   */
  uint32_t GetSeq (void) const;
  /**
   * \param seq the flow number
   */
  void SetFlow (uint32_t flow);
  /**
   * \return the flow number
   */
  uint32_t GetFlow (void) const;

  /**
   * \return the time stamp
   */
  Time GetTs (void) const;
  
  void SetMessageType (uint8_t type);
  
  uint8_t GetMessageType (void) const;
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint32_t m_flow; //!< Flow number
  uint32_t m_seq; //!< Sequence number
  uint64_t m_ts; //!< Timestamp
  uint8_t  m_messageType;
};

} // namespace ns3

#endif /* SPTM_HEADER_H */
