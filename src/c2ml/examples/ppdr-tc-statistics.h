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
#ifndef STATISTICS_H
#define STATISTICS_H

#include "ppdr-tc-utils.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ppdrtc {

using namespace ns3;

class InetPair
{
public:
  InetPair () { }

  InetPair (const std::string &source, const std::string &dest, uint32_t p)
  {
    src = source;
    dst = dest;
    port = p;
  }

  InetPair & operator=(const InetPair &rhs)
  {
    if (this != &rhs)
      {
        src = rhs.src;
        dst = rhs.dst;
        port = rhs.port;
      }

    return *this;
  }

  operator std::string () const
  {
    std::stringstream ss;
    ss << port;
    return src + "-" + dst + "-" + ss.str();
  }

  std::string src;
  std::string dst;
  uint16_t    port;

  Gnuplot2dDataset delayData;
  Gnuplot2dDataset jitterData;
  Gnuplot2dDataset bytesData;
};


class Statistics
{
public:
  Statistics (double thSampling);
  ~Statistics ();

  void Ipv4ClientTxCallback (std::string, Ptr<const Packet>, Ptr<Ipv4>,  uint32_t);
  void Ipv4GatewayTxCallback (std::string, Ptr<const Packet>, Ptr<Ipv4>,  uint32_t);
  void Ipv4RemoteTxCallback (std::string, Ptr<const Packet>, Ptr<Ipv4>,  uint32_t);

  void Ipv4ClientRxCallback (std::string, Ptr<const Packet>, Ptr<Ipv4>, uint32_t);
  void Ipv4GatewayRxCallback (std::string, Ptr<const Packet>, Ptr<Ipv4>, uint32_t);
  void Ipv4RemoteRxCallback (std::string, Ptr<const Packet>, Ptr<Ipv4>, uint32_t);

  void Ipv4ClientDropCallback (std::string, const Ipv4Header &, Ptr<const Packet>,
                          Ipv4L3Protocol::DropReason, Ptr<Ipv4>, uint32_t);
  void Ipv4GatewayDropCallback (std::string, const Ipv4Header &, Ptr<const Packet>,
                           Ipv4L3Protocol::DropReason, Ptr<Ipv4>, uint32_t);
  void Ipv4RemoteDropCallback (std::string, const Ipv4Header &, Ptr<const Packet>,
                          Ipv4L3Protocol::DropReason, Ptr<Ipv4>, uint32_t);

  void OutputStat (StatisticsSection &statistics, const std::string &prefix);

protected:
  typedef std::map<std::string, DelayJitterEstimation*> DelayEstMap;
  typedef std::pair<std::string, DelayJitterEstimation*> DelayEstPair;

  typedef std::vector<InetPair*> Connections;

  DelayEstMap m_delayEst;

  Connections m_connections;

  double m_thSampling;

private:
  static uint16_t GetPortFromPkt (Ptr<const Packet> pkt);
  InetPair* GetInetPair (const std::string &src, const std::string &dest,
                         uint16_t port);
  void PreparePktForTracking (const std::string &from, Ptr<const Packet> pkt);
  void ProcessPacketRcvd (const std::string &src, const std::string &dest,
                                 Ptr<const Packet> pkt);
  void OutputGnuplot (InetPair *conn, const Gnuplot2dDataset &dataSet,
                             const std::string &prefix,
                             const std::string &xLegend, const std::string &yLegend,
                             const std::string &title);
};

} // namespace ns3
#endif // STATISTICS_H
