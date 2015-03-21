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

bool
InetPair::operator ==(const InetPair &other) const
{
  struct in_addr lhs_src, rhs_src, lhs_dst, rhs_dst;

  inet_aton (src.c_str(), &lhs_src);
  inet_aton (other.src.c_str(), &rhs_src);
  inet_aton (dst.c_str(), &lhs_dst);
  inet_aton (other.dst.c_str(), &rhs_dst);

  return (lhs_src.s_addr == rhs_src.s_addr &&
          lhs_dst.s_addr == rhs_dst.s_addr &&
          port == other.port);
}

bool InetPair::operator <(const InetPair &other) const
{
  if (src < other.src)
    return true;
  if (dst < other.dst)
    return true;
  if (port < other.port)
    return true;

  return false;
}

bool
InetPair::operator !=(const InetPair &other) const
{
  return !(*this == other);
}

Statistics::Statistics (double thSampling)
{
  m_thSampling = thSampling;
}

Statistics::~Statistics ()
{
  DeleteFromMap (m_delayData);
  DeleteFromMap (m_jitterData);
  DeleteFromMap (m_delayEst);
}

void
Statistics::DeleteFromMap(DelayEstMap &dataMap)
{
  DelayEstMap::iterator it;

  for (it = dataMap.begin(); it != dataMap.end(); ++it)
    {
      delete it->second;
    }
}

void
Statistics::DeleteFromMap(DataMap &dataMap)
{
  DataMap::iterator it;

  for (it = dataMap.begin(); it != dataMap.end(); ++it)
    {
      delete it->second;
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

  InetPair key (src, dest, port);

  NS_LOG_UNCOND ("Add Delay Point for " << (std::string) key);
  AddPoint (m_delayData, key, Simulator::Now().GetSeconds(),
            delayEst->GetLastDelay().GetSeconds());
  NS_LOG_UNCOND ("Add Jitter Point for " << (std::string) key);
  AddPoint (m_jitterData, key, Simulator::Now().GetSeconds(),
            delayEst->GetLastJitter());

  NS_LOG_UNCOND ("Add Bytes Point for " << (std::string) key);
  KeepTrackOfBytes (m_rxBytesFromSources, m_throughputFromSources, key, pkt->GetSize());
  NS_LOG_UNCOND ("Add Total B Point for " << (std::string) key);
  KeepTrackOfBytes (m_rxBytesTotal, m_throughputTotal, key, pkt->GetSize());
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
Statistics::AddPoint(DataMap &dataMap, const InetPair &key, double x, double y)
{
  Gnuplot2dDataset *dataSet;

  try
    {
      dataSet = dataMap.at(key);
      NS_LOG_UNCOND ("preso data per " << (std::string) key);
    }
  catch (const std::out_of_range& oor)
    {
      (void) oor;
      dataSet = new Gnuplot2dDataset ();
      dataMap.insert(dataMap.begin(), DataPair (key, dataSet));
      NS_LOG_UNCOND ("INSERT data for " << (std::string) key);
    }

  dataSet->Add(x, y);
}

void
Statistics::KeepTrackOfBytes(DataMap &bytesDM, DataMap &throughputDM,
                             const InetPair &key, uint32_t size)
{
  typedef std::map<InetPair, uint32_t> BytesMap;

  static BytesMap rxBytes;
  static BytesMap rxBytesStepBefore;
  static Time from = Time (0);
  static Time to = Time(0+m_thSampling);

  BytesMap::iterator itBytes, itBytesStepBefore;

  itBytes = rxBytes.find(key);
  itBytesStepBefore = rxBytesStepBefore.find(key);

  if (itBytes == rxBytes.end())
    {
      std::pair<std::map<InetPair, uint32_t>::iterator, bool> ret;
      ret = rxBytes.insert (std::make_pair (key, 0));
      itBytes = ret.first;
      NS_ASSERT (ret.second);

      ret = rxBytesStepBefore.insert(std::make_pair (key, 0));
      itBytesStepBefore = ret.first;
      NS_ASSERT (ret.second);
    }

  // This lose the last step... TODO
  while (Simulator::Now() > to)
    {
      uint32_t bytesSinceStart = itBytes->second - itBytesStepBefore->second;
      AddPoint (throughputDM, key, from.GetSeconds(), bytesSinceStart);

      itBytesStepBefore->second = itBytes->second;
      from = to;
      to = Time::FromDouble(to.GetSeconds() + m_thSampling, Time::S);
    }

  itBytes->second += size;
  AddPoint (bytesDM, key, Simulator::Now().GetSeconds(), itBytes->second);
}

void
Statistics::Ipv4ClientDropCallback (std::string, const Ipv4Header &header, Ptr<const Packet> pkt,
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
  if (statistics.EnableDelay)
    {
      OutputGnuplot (m_delayData, prefix+"-e2e-delay-from", "Time (s)", "E2E Delay (s)",
                     "E2E Delay over Time");
    }

  if (statistics.EnableJitter)
    {
      OutputGnuplot (m_jitterData, prefix+"-e2e-jitter-from", "Time (s)", "E2E Jitter (s)",
                     "E2E Jitter over Time");
    }

  if (statistics.EnableThroughput)
    {
      OutputGnuplot (m_rxBytesFromSources, prefix+"-e2e-l3rxbytes-from",
                     "Time (s)", "E2E IP Rx Bytes (B)", "E2E Rx Bytes");
      OutputGnuplot (m_rxBytesTotal, prefix+"-e2e-l3rxbytes","Time (s)",
                     "E2E IP Rx Bytes (B)", "E2E Total Rx Bytes");
      OutputGnuplot (m_throughputFromSources, prefix+"-e2e-l3throughput-from",
                     "Time (s)", "E2E IP Throughput (B)", "E2E Throughput");
      OutputGnuplot (m_throughputTotal, prefix+"-e2e-l3throughput", "Time (s)",
                     "E2E IP Throughput (B)", "E2E Total throughput");
    }
}

void
Statistics::OutputGnuplot (DataMap &dataMap, const std::string &prefix,
                           const std::string &xLegend, const std::string &yLegend,
                           const std::string &title)
{
  DataMap::iterator it;
  for (it = dataMap.begin(); it != dataMap.end(); ++it)
    {
      Gnuplot plot;

      std::ofstream gnuplotFile, dataFile;

      InetPair inetPair = it->first;
      std::string fileName = prefix+"-"+ (std::string) inetPair;

      std::string dataFileName = fileName+".data";

      plot.AddDataset(*(it->second));
      plot.SetLegend(xLegend, yLegend);
      plot.SetTitle(title);
      plot.SetOutputFilename(fileName+".png");

      gnuplotFile.open ((fileName+".gnuplot").c_str());
      dataFile.open ((dataFileName).c_str());
      plot.GenerateOutput(gnuplotFile, dataFile, dataFileName);

      gnuplotFile.close();
      dataFile.close();
    }
}
} //namespace ns3
