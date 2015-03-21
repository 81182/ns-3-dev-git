/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Natale Patriciello <natale.patriciello@gmail.com>
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
 */
#include "ppdr-tc-statistics.h"
#include <stdexcept>      // std::out_of_range
#include <arpa/inet.h>    // inet fn

namespace ppdrtc {
using namespace ns3;

/**
 * Tag to perform Delay and Jitter estimations
 *
 * The tag holds the packet's creation timestamp
 */
class SourceTag : public Tag
{
public:
  SourceTag ();

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (TagBuffer i) const;
  virtual void Deserialize (TagBuffer i);
  virtual void Print (std::ostream &os) const;

  /**
   * \brief Get the Transmission time stored in the tag
   * \return the transmission time
   */
  std::string GetSource (void) const;

  void SetSource (std::string src);
private:
  std::string m_source; //!< The time stored in the tag
};

SourceTag::SourceTag ()
  : m_source ("")
{
}

TypeId
SourceTag::GetTypeId (void)
{
  static TypeId tid = TypeId ("anon::SourceTag")
    .SetParent<Tag> ()
    .AddConstructor<SourceTag> ()
    .AddAttribute ("Source",
                   "Who created this packet",
                   StringValue (""),
                   MakeStringAccessor (&SourceTag::GetSource),
                   MakeStringChecker ())
  ;
  return tid;
}
TypeId
SourceTag::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
SourceTag::GetSerializedSize (void) const
{
  return sizeof (uint32_t) + m_source.size() + sizeof(',');
}
void
SourceTag::Serialize (TagBuffer i) const
{
  i.WriteU32(m_source.size());
  i.WriteU8(',');
  i.Write((uint8_t*) m_source.c_str(), m_source.size());
}
void
SourceTag::Deserialize (TagBuffer i)
{
  uint32_t size = i.ReadU32();
  i.ReadU8();
  uint8_t *buffer = new uint8_t[size+1];
  i.Read(buffer, size);

  m_source = std::string (buffer, buffer+size);
}
void
SourceTag::Print (std::ostream &os) const
{
  os << "Source=" << m_source;
}
std::string
SourceTag::GetSource (void) const
{
  return m_source;
}

void
SourceTag::SetSource(std::string source)
{
  m_source = source;
}

Statistics::Statistics (double thSampling)
{
  m_thSampling = thSampling;
}

Statistics::~Statistics ()
{
  for (Connections::iterator it = m_connections.begin(); it != m_connections.end(); ++it)
    {
      InetPair *p = (*it);

      delete p;
    }
}

void
Statistics::PreparePktForTracking (const std::string &from,
                                   Ptr<const Packet> pkt)
{
  DelayJitterEstimation *delayEst;
  SourceTag sourceTag;
  sourceTag.SetSource(from);

  try
    {
      delayEst = m_delayEst.at (from);
    }
  catch (const std::out_of_range& oor)
    {
      delayEst = new DelayJitterEstimation ();
      m_delayEst.insert(m_delayEst.begin (), DelayEstPair (from, delayEst));
    }

  delayEst->PrepareTx(pkt);

  pkt->AddByteTag(sourceTag);
}

void
Statistics::Ipv4ClientTxCallback (std::string context, Ptr<const Packet> pkt,
                                  Ptr<Ipv4> ipv4, uint32_t ifIndex)
{
  (void) ipv4;
  (void) ifIndex;

  PreparePktForTracking (context, pkt);
}

void
Statistics::Ipv4GatewayTxCallback (std::string, Ptr<const Packet> pkt, Ptr<Ipv4> ipv4,
                                   uint32_t ifIndex)
{

}

void
Statistics::Ipv4RemoteTxCallback (std::string context, Ptr<const Packet> pkt,
                                  Ptr<Ipv4> ipv4, uint32_t ifIndex)
{
  (void) ipv4;
  (void) ifIndex;

  PreparePktForTracking (context, pkt);
}

void
Statistics::Ipv4ClientRxCallback (std::string context, Ptr<const Packet> pkt,
                                  Ptr<Ipv4> ipv4, uint32_t ifIndex)
{
  (void) ipv4;
  (void) ifIndex;

  SourceTag tag;

  pkt->FindFirstMatchingByteTag(tag);

  NS_ASSERT (!tag.GetSource().empty());

  std::string source = tag.GetSource();

  ProcessPacketRcvd(source, context, pkt);
}

void
Statistics::Ipv4GatewayRxCallback (std::string, Ptr<const Packet> pkt, Ptr<Ipv4> ipv4,
                                   uint32_t ifIndex)
{

}

uint16_t
Statistics::GetPortFromPkt (Ptr<const Packet> pkt)
{
  uint16_t port = 0;
  Ptr<Packet> packet = pkt->Copy();

  Ipv4Header ipv4Header;

  packet->RemoveHeader (ipv4Header);
  uint8_t protocol = ipv4Header.GetProtocol();

  if (protocol == 6)
    {
      TcpHeader tcpHeader;
      packet->RemoveHeader(tcpHeader);
      port = tcpHeader.GetDestinationPort();
    }
  else if (protocol == 17)
    {
      UdpHeader udpHeader;
      packet->RemoveHeader(udpHeader);
      port = udpHeader.GetDestinationPort();
    }
  else
    {
      NS_FATAL_ERROR ("Protocol " << protocol << " not supported");
    }

  return port;
}

InetPair*
Statistics::GetInetPair (const std::string &src, const std::string &dest,
                         uint16_t port)
{
  for (Connections::iterator it = m_connections.begin(); it != m_connections.end(); ++it)
    {
      InetPair *p = (*it);

      if (p->src.compare(src) == 0 &&
          p->dst.compare(dest) == 0 &&
          p->port == port)
        {
          return p;
        }
    }

  InetPair *ret = new InetPair (src, dest, port);
  m_connections.insert(m_connections.begin(), ret);
  return ret;
}

void
Statistics::ProcessPacketRcvd (const std::string &src, const std::string &dest,
                               Ptr<const Packet> pkt)
{
  DelayJitterEstimation *delayEst;

  try
    {
      delayEst = m_delayEst.at (src);
    }
  catch (const std::out_of_range& oor)
    {
      (void) oor;
      NS_FATAL_ERROR ("No TX callback registered");
    }

  delayEst->RecordRx(pkt);

  uint16_t port = GetPortFromPkt(pkt);

  InetPair *p = GetInetPair (src, dest, port);

  p->delayData.Add(Simulator::Now().GetSeconds(),  delayEst->GetLastDelay().GetSeconds());
  p->jitterData.Add(Simulator::Now().GetSeconds(), delayEst->GetLastJitter());
  p->bytesData.Add(Simulator::Now().GetSeconds(), pkt->GetSize());
}

void
Statistics::Ipv4RemoteRxCallback (std::string context, Ptr<const Packet> pkt,
                                  Ptr<Ipv4> ipv4, uint32_t ifIndex)
{
  (void) ipv4;
  (void) ifIndex;

  SourceTag tag;

  pkt->FindFirstMatchingByteTag(tag);

  NS_ASSERT (!tag.GetSource().empty());

  std::string source = tag.GetSource();

  ProcessPacketRcvd(source, context, pkt);
}

void
Statistics::Ipv4ClientDropCallback (std::string context, const Ipv4Header &header, Ptr<const Packet> pkt,
                                    Ipv4L3Protocol::DropReason dropReason,
                                    Ptr<Ipv4> ipv4, uint32_t ifIndex)
{

}

void
Statistics::Ipv4GatewayDropCallback (std::string, const Ipv4Header &header, Ptr<const Packet> pkt,
                                     Ipv4L3Protocol::DropReason dropReason,
                                     Ptr<Ipv4> ipv4, uint32_t ifIndex)
{

}
void
Statistics::Ipv4RemoteDropCallback (std::string, const Ipv4Header &header, Ptr<const Packet> pkt,
                                    Ipv4L3Protocol::DropReason dropReason,
                                    Ptr<Ipv4> ipv4, uint32_t ifIndex)
{

}

void
Statistics::OutputStat(StatisticsSection &statistics, const std::string &prefix)
{
  for (Connections::iterator it = m_connections.begin(); it != m_connections.end(); ++it)
    {
      InetPair *p = (*it);
      if (statistics.EnableDelay)
        {
          OutputGnuplot (p, p->delayData, prefix+"-e2e-delay", "Time (s)", "E2E Delay (s)",
                         "E2E Delay over Time");
        }

      if (statistics.EnableJitter)
        {
          OutputGnuplot (p, p->jitterData, prefix+"-e2e-jitter", "Time (s)", "E2E Jitter (s)",
                         "E2E Jitter over Time");
        }

      if (statistics.EnableThroughput)
        {
          OutputGnuplot (p, p->bytesData, prefix+"-e2e-l3bytes",
                         "Time (s)", "E2E IP Rx Bytes (B)", "E2E Rx Bytes");
        }
    }
}


void
Statistics::OutputGnuplot (InetPair *conn, const Gnuplot2dDataset &dataSet,
                           const std::string &prefix,
                           const std::string &xLegend, const std::string &yLegend,
                           const std::string &title)
{
  Gnuplot plot;

  std::ofstream gnuplotFile, dataFile;

  std::string fileName = prefix+"-"+ (std::string) (*conn);

  std::string dataFileName = fileName+".data";

  plot.AddDataset(dataSet);
  plot.SetLegend(xLegend, yLegend);
  plot.SetTitle(title);
  plot.SetOutputFilename(fileName+".png");

  gnuplotFile.open ((fileName+".gnuplot").c_str());
  dataFile.open ((dataFileName).c_str());
  plot.GenerateOutput(gnuplotFile, dataFile, dataFileName);

  gnuplotFile.close();
  dataFile.close();
}

} //namespace ns3
