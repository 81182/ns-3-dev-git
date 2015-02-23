/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Natale Patriciello <natale.patriciello@gmail.com>
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
 */

#ifndef C2MLTXQUEUE_H
#define C2MLTXQUEUE_H

#include <queue>
#include <list>
#include <map>
#include "ns3/drop-tail-queue.h"
#include "ns3/rtt-estimator.h"
#include "ns3/simulator.h"

namespace ns3 {

class Ipv4Header;
class TcpHeader;

/**
 * \brief A FIFO packet queue that drops tail-end packets on overflow
 */
class C2MLTxQueue : public DropTailQueue {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  /**
   * \brief C2MLQueue Constructor
   *
   * Creates a droptail queue with a maximum size of 100 packets by default
   */
  C2MLTxQueue ();

  virtual ~C2MLTxQueue();

  void SetGoodBw (uint64_t goodBw);
  void SetAllowedIp (const Ipv4Address &addr);

  void TrackRcv (const Ipv4Header &ipHeader, const TcpHeader &tcpHeader);

  /**
   * \brief Enumeration of the modes supported in the class.
   *
   */
  enum DropMode
  {
    DROP_HEAD,
    DROP_TAIL
  };

  void SetMode (C2MLTxQueue::DropMode mode);
  C2MLTxQueue::DropMode GetMode ();

protected:
  // From AbstractQueue
  virtual bool DoEnqueue (Ptr<Packet> pContainer);

  // Mine
  typedef std::pair<uint32_t, Time> TimePair;
  typedef std::pair<uint32_t, Ptr<RttMeanDeviation> > RttPair;
  typedef std::pair<uint32_t, uint32_t> BytePair;

  typedef std::map<uint32_t, Ptr<RttMeanDeviation> > RttMap;
  typedef std::map<uint32_t, Time> TimeMap;
  typedef std::map<uint32_t, uint32_t> ByteMap;

  typedef TimeMap::iterator TimeMapIterator;
  typedef RttMap::iterator RttMapIterator;
  typedef ByteMap::iterator ByteMapIterator;

  bool ShouldEnqueueOut (const Ipv4Header &ipHeader, const TcpHeader &tcpHeader,
                         uint32_t size);
  bool IsIpAllowed (const Ipv4Address &ip);
  void TrackSent (const Ipv4Header &ipHeader, const TcpHeader &tcpHeader, uint32_t size);

  void ResetData (const Ipv4Address &addr);

  uint64_t m_goodBw;

  std::list<Ipv4Address> m_allowedIp;
  RttMap m_rtt;
  TimeMap m_time;
  ByteMap m_byte;
  TimeMap m_startTime;
  TimeMap m_startRtt;

  DropMode m_mode;
};

} // namespace ns3

#endif // C2MLQUEUE_H
