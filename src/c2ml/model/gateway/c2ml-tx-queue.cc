/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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

#include "ns3/log.h"
#include "ns3/enum.h"
#include "ns3/tcp-header.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-header.h"
#include "../src/internet/model/tcp-option-ts.h"

#include "c2ml-tx-queue.h"

NS_LOG_COMPONENT_DEFINE ("C2MLTxQueue");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (C2MLTxQueue);

#define RTT_OF(src) m_startRtt.find(src.Get())->second
#define BYTE_OF(src) m_byte.find(src.Get())->second
#define START_OF(src) m_startTime.find(src.Get())->second

#define NOW Simulator::Now()

TypeId C2MLTxQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::C2MLTxQueue")
    .SetParent<DropTailQueue> ()
    .AddConstructor<C2MLTxQueue> ()
    .AddAttribute ("DropMode",
                   "Whether to use Head or Tail drop",
                   EnumValue (DROP_HEAD),
                   MakeEnumAccessor (&C2MLTxQueue::SetMode),
                   MakeEnumChecker (DROP_HEAD, "DROP_HEAD",
                                    DROP_TAIL, "DROP_TAIL"))
  ;

  return tid;
}

C2MLTxQueue::C2MLTxQueue () :
  DropTailQueue (),
  m_goodBw (0)
{
  NS_LOG_FUNCTION (this);
}

C2MLTxQueue::~C2MLTxQueue()
{

}

void
C2MLTxQueue::SetMode(C2MLTxQueue::DropMode mode)
{
  m_mode = mode;
}

C2MLTxQueue::DropMode
C2MLTxQueue::GetMode()
{
  return m_mode;
}

void
C2MLTxQueue::SetGoodBw (uint64_t goodBw)
{
  NS_LOG_DEBUG("Set bwgood= " << goodBw);
  m_goodBw = goodBw;
}

void
C2MLTxQueue::SetAllowedIp (const Ipv4Address &in)
{
  Ipv4Address addr = in;
  NS_LOG_DEBUG ("Set ALLOWED: " << addr);
  m_allowedIp.push_front(addr);

  m_byte.insert(BytePair (addr.Get(), 0));
  m_startTime.insert(TimePair (addr.Get(), Time::Min()));
}

void
C2MLTxQueue::TrackSent (const Ipv4Header &ipHeader, const TcpHeader &tcpHeader,
                        uint32_t size)
{

}

void
C2MLTxQueue::TrackRcv (const Ipv4Header &ipHeader, const TcpHeader &tcpHeader)
{
  RttMapIterator it = m_rtt.find(ipHeader.GetDestination().Get());
  if (it == m_rtt.end())
    {
      Ptr<RttMeanDeviation> rtt = CreateObject<RttMeanDeviation> ();
      m_rtt.insert(RttPair (ipHeader.GetSource().Get(), rtt));
    }

  it = m_rtt.find(ipHeader.GetSource().Get());

  NS_ASSERT (tcpHeader.HasOption(TcpOption::TS));
  Ptr<TcpOptionTS> tp = DynamicCast<TcpOptionTS> (tcpHeader.GetOption(TcpOption::TS));

  Time t = TcpOptionTS::ElapsedTimeFromTsValue(tp->GetEcho());
  NS_ASSERT (!t.IsZero());

  it->second->Measurement (t);
  Time rtt = it->second->GetEstimate ();

  NS_ASSERT (!rtt.IsZero());

  Time oldRtt = Time::FromInteger(0, Time::S);

  TimeMapIterator i = m_time.find(ipHeader.GetDestination().Get());

  if (i == m_time.end())
    {
      m_time.insert(TimePair (ipHeader.GetDestination().Get(), rtt));
    }
  else
    {
      oldRtt = i->second;
      i->second = rtt;
    }

  if (oldRtt.IsZero())
    {
      m_startRtt.insert(TimePair(ipHeader.GetDestination().Get(), rtt));
      Simulator::Schedule(rtt, &C2MLTxQueue::ResetData, this, ipHeader.GetDestination());

      NS_LOG_DEBUG (ipHeader.GetDestination() << " ACK; rtt= " << rtt.GetMilliSeconds());
    }
}

bool
C2MLTxQueue::IsIpAllowed (const Ipv4Address &ip)
{
  for (std::list<Ipv4Address>::iterator it = m_allowedIp.begin(); it != m_allowedIp.end(); ++it)
    {
      if (ip.IsEqual(*it))
        {
          return true;
        }
    }

  return false;
}

bool
C2MLTxQueue::ShouldEnqueueOut (const Ipv4Header &ipHeader, const TcpHeader &tcpHeader,
                               uint32_t size)
{
  const Ipv4Address src = ipHeader.GetSource();
  const Ipv4Address dest= ipHeader.GetDestination();
  SequenceNumber32 seq  = tcpHeader.GetSequenceNumber();
  SequenceNumber32 ack  = tcpHeader.GetAckNumber();
  uint8_t flags = tcpHeader.GetFlags();

  if (flags != TcpHeader::SYN && ! IsIpAllowed (src))
    {
      NS_LOG_DEBUG (" NOT ALLOWED: " << src);
      return false;
    }
  else if (flags == TcpHeader::SYN)
    {
      NS_LOG_DEBUG ("Track a SYN from " << src);
      //TrackSent (ipHeader, tcpHeader, size);
      return true;
    }

  if (START_OF(src) == Time::Min())
    {
      START_OF(src) = Simulator::Now();
    }

    NS_ASSERT(RTT_OF(src).GetSeconds() != 0.0);
    if (BYTE_OF(src) / RTT_OF(src).GetSeconds() >= m_goodBw)
      {
        NS_LOG_DEBUG ("Dropping src=" << src << " dest=" << dest << " seq= " << seq <<
                       " size=" << size << " ack=" << ack << " flags=" << (uint32_t) flags <<
                       " at " << Simulator::Now().GetSeconds() <<
                       " Droppo byte=" << BYTE_OF(src) << " rtt=" << RTT_OF(src).GetMilliSeconds());

        return false;
      }
    BYTE_OF(src) += size;

    //TrackSent (ipHeader, tcpHeader, size);
    NS_LOG_LOGIC ("TX: Allowing src=" << src << " dest=" << dest << " seq= " << seq <<
                   " size=" << size << " ack=" << ack << " flags=" << (uint32_t) flags <<
                   " at " << Simulator::Now().GetSeconds() <<
                   " Byte OUT from " << src << "=" << BYTE_OF(src) << " rtt: " << RTT_OF(src).GetMilliSeconds());

    return true;
}

void C2MLTxQueue::ResetData(const Ipv4Address &addr)
{
  START_OF(addr) = NOW;
  BYTE_OF(addr) = 0;
  Time rtt = m_time.find(addr.Get())->second;
  if (rtt > m_startRtt.find (addr.Get())->second)
    {
      rtt = m_startRtt.find (addr.Get())->second;
    }
  else
    {
      m_startRtt.find (addr.Get())->second = rtt;
    }

  NS_ASSERT (!rtt.IsZero());

  NS_LOG_DEBUG ("RESET FOR " << addr << " at " << NOW.GetSeconds());
  Simulator::Schedule(rtt, &C2MLTxQueue::ResetData, this, addr);
}

bool
C2MLTxQueue::DoEnqueue (Ptr<Packet> pContainer)
{
  Ptr<Packet> pktCopy = pContainer->Copy();

  //NS_LOG_DEBUG ("TX: ");
  //pktCopy->Print(std::cerr);
  Ipv4Header header;

  pktCopy->RemoveHeader(header);
  if (header.GetPayloadSize () < pktCopy->GetSize ())
    {
      pktCopy->RemoveAtEnd (pktCopy->GetSize () - header.GetPayloadSize ());
    }

  TcpHeader tcpHeader;
  pktCopy->RemoveHeader(tcpHeader);

  if (ShouldEnqueueOut (header, tcpHeader, pktCopy->GetSize ()))
    {
      return DropTailQueue::DoEnqueue(pContainer);
    }
  else
    {
      if (m_mode == DROP_TAIL)
        {
          DropTailQueue::Drop(pContainer);
        }
      else
        {
          NS_LOG_DEBUG ("Drop Head, size=" << m_packets.size () << " packet");
          Ipv4Address src = header.GetSource();
          Ipv4Header itemHeader;

          for (std::list< Ptr<Packet> >::reverse_iterator it = m_packets.rbegin();
               it != m_packets.rend (); ++it)
            {
              Ptr<Packet> item = (*it);
              item->PeekHeader(itemHeader);

              if (src.IsEqual(itemHeader.GetSource()))
                {
                  NS_LOG_DEBUG ("DROPPO p prima" << item->ToString() << " accodo " << pContainer->ToString());
                  m_packets.remove(item);
                  m_traceDequeue (item);
                  DropTailQueue::Drop (item);
                  m_bytesInQueue -= item->GetSize ();
                  m_nBytes -= item->GetSize ();
                  m_nPackets--;
                  NS_ASSERT (m_packets.size() == m_nPackets);
                  return DropTailQueue::DoEnqueue(pContainer);
                }
            }

          DropTailQueue::Drop (pContainer);
        }
      return false;
    }
}

} //namespace ns3
