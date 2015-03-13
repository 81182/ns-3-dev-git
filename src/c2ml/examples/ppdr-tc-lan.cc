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

#include <iostream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/lte-module.h"
#include "ns3/c2ml-module.h"
#include "ns3/log.h"
#include "ns3/config-store-module.h"
#include "ns3/stats-module.h"
#include "ns3/mobility-module.h"
#include "ns3/propagation-module.h"
#include "ns3/netanim-module.h"

#include "INIReader.h"
#include "ppdr-tc-utils.h"
#include "ppdr-tc-statistics.h"

using namespace ns3;
using namespace ppdrtc;

NS_LOG_COMPONENT_DEFINE ("PPDRTCLan");

#define TCP_PORT 140
#define UDP_PORT 50

typedef std::map<FlowId, FlowMonitor::FlowStats>::const_iterator FlowMap;

/**
 * \brief Create and read configuration for gateways
 *
 * \param gateways Container for gateways (will be filled)
 * \param gatewayMap Map for gateway configurations
 * \param cfg IniReader to read from
 * \param verbose Self-explanatory
 * \param gatewayN Number of gateway to add/configure
 * \return Number of gateways configured/added
 */
uint32_t
GatewayMaker (NodeContainer &gateways, NodeToConfigurationMap &gatewayMap,
              INIReader &cfg, bool verbose, uint32_t gatewayN)
{
  uint32_t i;
  NS_LOG_INFO ("Creating " << gatewayN << " gateways");

  for (i=0; i<gatewayN; ++i)
    {
      Ptr<Node> gateway = CreateObject<Node> ();
      gateways.Add (gateway);

      std::stringstream ss;
      ss << "gateway" << i;
      Names::Add (ss.str (), gateway);

      GatewaySection *c = new GatewaySection (ss.str ());

      c->Fill (cfg);

      if (verbose)
        {
          c->Print ();
        }

      gatewayMap.insert (NodeConfigurationPair (gateway, c));
    }

  return i;
}

uint32_t
RemoteMaker (NodeContainer &remotes, NodeToConfigurationMap &remoteMap,
             INIReader &cfg, bool verbose, uint32_t remotesN)
{
  uint32_t i;
  InternetStackHelper internetHelper;
  NS_LOG_INFO ("Creating " << remotesN << " remotes");

  for (i=0; i<remotesN; ++i)
    {
      Ptr<Node> remote = CreateObject<Node> ();
      internetHelper.Install(remote);

      remotes.Add (remote);

      std::stringstream ss;
      ss << "remote" << i;
      Names::Add (ss.str (), remote);

      RemoteSection *c = new RemoteSection (ss.str ());

      c->Fill (cfg);

      if (verbose)
        {
          c->Print ();
        }

      remoteMap.insert (NodeConfigurationPair (remote, c));
    }

  return i;
}

/**
 * \brief Create and read configuration for clients
 * \param lanClients Container for lan clients
 * \param clientMap Map for client configuration
 * \param cfg INIReader to read from
 * \param verbose self-explanatory
 * \param enableC2ML self-explanatory
 * \param clientN Number of client to configure/add
 * \return number of clients configured/added
 */
uint32_t
ClientMaker (NodeContainer &lanClients, NodeToConfigurationMap &clientMap,
             INIReader &cfg, bool verbose, bool enableC2ML, uint32_t clientN)
{
  InternetStackHelper internet;
  uint32_t i;

  for (i=0; i<clientN; ++i)
    {
      Ptr<Node> client = CreateObject<Node> ();
      lanClients.Add (client);

      std::stringstream ss;
      ss << "client" << i;
      Names::Add (ss.str (), client);

      ClientSection *c = new ClientSection (ss.str ());

      c->Fill (cfg);

      if (verbose)
        {
          c->Print ();
        }

      clientMap.insert (NodeConfigurationPair (client, c));

      if (enableC2ML)
        {
          //NS_LOG_INFO ("For client" << i << " id=" << client->GetId() <<
          //             " and set CCL45Protocol");
          internet.SetTcp ("ns3::CCL45Protocol");
          NS_FATAL_ERROR ("Test C2ML First");
        }
      else
        {
          //NS_LOG_INFO ("For client" << i << " id=" << client->GetId() <<
          //             " and set TcpL4Protocol");
          internet.SetTcp ("ns3::TcpL4Protocol");
        }

      internet.Install (client);
    }

  return i;
}

/**
 * \brief Install the mobility on a node
 * \param v Position of the node
 * \param mobilityModel Type of mobility model
 * \param node Ptr to a node
 * \param mobility MobilityHelper to use
 */
void
InstallMobilityOnNode (Section::XYZ &v, const std::string &mobilityModel,
                       Ptr<Node> node, MobilityHelper &mobility)
{
  NS_LOG_INFO ("Node " << node->GetId () << " Model=" << mobilityModel <<
               " position" << v.ToString ());
  Ptr<ListPositionAllocator> posList = CreateObject<ListPositionAllocator> ();
  posList->Add (Vector (v.x, v.y, v.z));

  mobility.SetMobilityModel (mobilityModel);
  mobility.SetPositionAllocator (posList);
  mobility.Install (node);
}

/**
 * \brief Install the mobility on a container
 * \param container Container of nodes
 * \param nodeMap Configuration for nodes
 * \param verbose self-explanatory
 */
void
InstallMobilityOnContainer (NodeContainer &container,
                            NodeToConfigurationMap &nodeMap, bool verbose)
{
  MobilityHelper mobility;

  FOR_EACH_NODE (it, container)
  {
    Ptr<Node> node = (*it);
    NodeSection *conf = nodeMap.at (node);

    NS_ASSERT (conf != 0);

    Section::XYZ v = Section::ToXYZ (conf->Position);
    InstallMobilityOnNode (v, conf->MobilityModel, node, mobility);

    if (verbose)
      {
        NS_LOG_UNCOND ("Installed Mobility on node " << Names::FindName(node) <<
                       " type " << conf->MobilityModel << " position " <<
                       conf->Position);
      }
  }
}

/**
 * \brief Create the Lte LAN
 *
 * Typical lan lte
 * \verbatim
[u1] ----------
               |
[u2] --------[enb1] ----------
               |              |
[u3] ----------               |
                              |
                            [pgw]-------------[ R E M O T E   N O D E ]
                              |
[u4]----------|               |
[u5]---------[enb2] ----------
[u6]----------|
\endverbatim
 * gwToRemote = [g1, g2]
 *
 * \param gateways Container of gateways (Enb)
 * \param clients Container of clients (Ue)
 * \param configurationMap Configuration map
 * \param verbose self-explanatory
 *
 * \return the Pgw node (to be connected with the remote)
 */
NodeContainer
CreateLteLAN (NodeContainer &gateways, NodeContainer &clients,
              NodeToConfigurationMap &configurationMap, LanSection &lan,
              GeneralSection &general, LteSection &lte)
{
  Config::SetDefault("ns3::MacStatsCalculator::DlOutputFilename",
                     StringValue (general.Prefix+"-lte-dlstats.txt"));
  Config::SetDefault("ns3::MacStatsCalculator::UlOutputFilename",
                     StringValue (general.Prefix+"-lte-ulstats.txt"));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::DlRlcOutputFilename",
                     StringValue (general.Prefix+"-DlRlcStats.txt"));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::UlRlcOutputFilename",
                     StringValue (general.Prefix+"-UlRlcStats.txt"));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::DlPdcpOutputFilename",
                     StringValue (general.Prefix+"-DlPdcpStats.txt"));
  Config::SetDefault("ns3::RadioBearerStatsCalculator::UlPdcpOutputFilename",
                     StringValue (general.Prefix+"-UlPdcpStats.txt"));

  NodeContainer ret;
  static Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  static Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();

  epcHelper->SetAttribute("X2LinkPcapPrefix", StringValue (general.Prefix + "-X2"));
  epcHelper->SetAttribute("S1uLinkPcapPrefix", StringValue (general.Prefix + "-S1u"));

  if (lan.EnablePcapUserNetwork)
    {
      epcHelper->SetAttribute("X2LinkEnablePcap", BooleanValue (lte.EnablePcapX2Link));
      epcHelper->SetAttribute("S1uLinkEnablePcap", BooleanValue (lte.EnablePcapS1uLink));
    }

  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetEnbAntennaModelType(lte.EnbAntennaType);
  lteHelper->SetEnbAntennaModelAttribute("MaxGain", DoubleValue (lte.EnbAntennaMaxGain));
  lteHelper->SetEnbDeviceAttribute("UlBandwidth", UintegerValue(lte.UlBandwidth));
  lteHelper->SetEnbDeviceAttribute("DlBandwidth", UintegerValue(lte.DlBandwidth));
  lteHelper->SetEnbDeviceAttribute("DlEarfcn", UintegerValue(lte.DlEarfcn));
  lteHelper->SetEnbDeviceAttribute("UlEarfcn", UintegerValue(lte.UlEarfcn));
  lteHelper->SetUeAntennaModelType(lte.UeAntennaModel);
  lteHelper->SetUeDeviceAttribute("DlEarfcn", UintegerValue(lte.DlEarfcn));
  lteHelper->SetPathlossModelType(lte.PathlossType);
  lteHelper->SetPathlossModelAttribute("Environment", EnumValue (lte.GetEnvironment()));
  lteHelper->SetPathlossModelAttribute("CitySize", EnumValue (lte.GetCitySize()));
  lteHelper->SetSchedulerType(lte.SchedulerType);
  lteHelper->EnableTraces();

  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  ret.Add (pgw);
  Names::Add("pgw0", pgw);

  NS_LOG_INFO ("Installing Devices on nodes");

  NetDeviceContainer enbDevices = lteHelper->InstallEnbDevice (gateways);
  NetDeviceContainer ueDevices = lteHelper->InstallUeDevice (clients);

  NetDeviceContainer::Iterator it;
  for (it = enbDevices.Begin (); it != enbDevices.End (); ++it)
    {
      Ptr<LteEnbNetDevice> dev = DynamicCast<LteEnbNetDevice> (*it);
      NS_ASSERT (dev != 0);
      Ptr<Node> node = dev->GetNode ();
      Names::Add(Names::FindName(node)+"/dev", dev);
      GatewaySection *conf = reinterpret_cast<GatewaySection*> (configurationMap.at (node));
      NS_ASSERT (conf != 0);

      dev->GetPhy ()->SetTxPower (conf->TxPower);
      dev->GetRrc ()->SetSrsPeriodicity (conf->EnbSrsPeriodicity);

      NS_LOG_INFO ("Enb " << Names::FindName(node) << " TxPower=" << conf->TxPower <<
                   " Srs=" << conf->EnbSrsPeriodicity);
    }


  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (ueDevices);

  for (it = ueDevices.Begin (); it != ueDevices.End (); ++it)
    {
      Ptr<LteUeNetDevice> dev = DynamicCast<LteUeNetDevice> (*it);
      NS_ASSERT (dev != 0);
      Ptr<Node> node = dev->GetNode ();
      Names::Add(Names::FindName(node)+"/dev", dev);
      NodeSection *conf = configurationMap.at (node);
      NS_ASSERT (conf != 0);

      dev->GetPhy ()->SetTxPower (conf->TxPower);

      Ptr<Ipv4> ipv4 = node->GetObject<Ipv4> ();
      configurationMap.at(node)->Address = ipv4->GetAddress(1,0).GetLocal();

      NS_LOG_INFO ("Ue " << Names::FindName(node) << " TxPower=" << conf->TxPower);
    }


  std::map <std::string, std::list<std::string> > nodesIdToGwId;
  NS_LOG_INFO ("Decodifying " << lan.NPerG << "...");
  DecodifyLanNPerG (lan.NPerG, nodesIdToGwId);

  if (nodesIdToGwId.size () != gateways.GetN ())
    {
      NS_FATAL_ERROR ("Less gw specified than the total number, in NPerG");
    }

  FOR_EACH_NODE (it, gateways)
  {
    Ptr<Node> gateway = (*it);

    std::list<std::string> lanNodes = nodesIdToGwId.at (Names::FindName(gateway));
    NS_LOG_INFO ("Doing gateway name=" << Names::FindName(gateway));
    Ptr<NetDevice> enbDev = Names::Find<NetDevice> (Names::FindName(gateway)+"/dev");
    NS_ASSERT (enbDev != 0);

    for (std::list<std::string>::iterator it2 = lanNodes.begin (); it2 != lanNodes.end (); ++it2)
      {
        Ptr<Node> client = Names::Find<Node> (*it2);
        NS_ASSERT_MSG (client != 0, "client " << (*it2) << " do not exists");

        Ptr<NetDevice> ueDev = Names::Find<NetDevice> (Names::FindName(client)+"/dev");
        NS_ASSERT (ueDev != 0);

        NS_LOG_INFO ("Attaching dev of " << Names::FindName(client) <<
                     " to " << Names::FindName(gateway));
        lteHelper->Attach(ueDev, enbDev);
      }
  }
/*
  Ptr<EpcTft> tftTCP = Create<EpcTft> ();
  Ptr<EpcTft> tftUDP = Create<EpcTft> ();

  EpcTft::PacketFilter PacketFilterB1;
  PacketFilterB1.localPortStart = TCP_PORT-10;
  PacketFilterB1.localPortEnd = TCP_PORT+10;
  tftTCP->Add(PacketFilterB1);

  EpcTft::PacketFilter PacketFilterB2;
  PacketFilterB2.remotePortStart = UDP_PORT-10;
  PacketFilterB2.remotePortEnd = UDP_PORT+10;
  tftUDP->Add(PacketFilterB2);

  enum EpsBearer::Qci qVID = EpsBearer::GBR_CONV_VIDEO;
  GbrQosInformation VIDqos;
  VIDqos.gbrDl = 64; // Downlink GBR
  VIDqos.gbrUl = 0; // Uplink GBR
  VIDqos.mbrDl = 512; // Downlink MBR
  VIDqos.mbrUl = 0; // Uplink MBR
  EpsBearer VIDbearer (qVID, VIDqos);
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      lteHelper->ActivateDedicatedEpsBearer(ueLteDevs.Get(i), VIDbearer, tftTCP);
    }

  enum EpsBearer::Qci qVoIP = EpsBearer::GBR_CONV_VOICE;
  GbrQosInformation VoIPqos;
  VoIPqos.gbrDl = 12.2; // Downlink GBR
  VoIPqos.gbrUl = 0; // Uplink GBR
  VoIPqos.mbrDl = 64; // Downlink MBR
  VoIPqos.mbrUl = 0; // Uplink MBR
  EpsBearer VoIPbearer (qVoIP, VoIPqos);
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      lteHelper->ActivateDedicatedEpsBearer(ueLteDevs.Get(i), VoIPbearer, tftUDP);
    }
*/

  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  for (uint32_t i=0; i<clients.GetN(); ++i)
    {
      Ptr<Node> ue = clients.Get (i);
      Ptr<Ipv4StaticRouting> ueStaticRouting;
      ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());

      // 0 is Loopback, 1 is LteUeNetDevice
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

    }


  return ret;
}

/**
 * \brief Create P2P Lan
 *
 * Typical lan p2p
 * \verbatim
 [c1] ----------
                |
 [c2] --------[g1] --------------------------------------
                |                                       |
 [c3] ----------                                        |
                                                        |
                                              [ R E M O T E   N O D E ]
                                                        |
 [c4]----------|                                        |
 [c5]---------[g2] --------------------------------------
 [c6]----------|
 \endverbatim
 *
 * gwToRemote = [g1, g2]
 *
 * \param clients
 * \param gateways
 * \param lan
 * \param configurationMap
 */
void
CreateP2PLan (NodeContainer &gateways, LanSection &lan,
              NodeToConfigurationMap &configurationMap, Ipv4AddressHelper &ipv4h,
              GeneralSection &general)
{
  PointToPointHelper p2p;
  std::map <std::string, std::list<std::string> > nodesIdToGwId;
  NS_LOG_INFO ("Decodifying " << lan.NPerG << "...");
  DecodifyLanNPerG (lan.NPerG, nodesIdToGwId);

  if (nodesIdToGwId.size () != gateways.GetN ())
    {
      NS_FATAL_ERROR ("Less gw specified than the total number, in NPerG");
    }

  InternetStackHelper internetHelper;

  FOR_EACH_NODE (it, gateways)
  {
    Ptr<Node> gateway = (*it);
    internetHelper.Install (gateway);

    std::list<std::string> lanNodes = nodesIdToGwId.at (Names::FindName(gateway));
    NS_LOG_INFO ("Doing gateway name=" << Names::FindName(gateway));

    GatewaySection *g_conf = reinterpret_cast<GatewaySection*> (configurationMap.at (gateway));
    NS_ASSERT (g_conf != 0);

    for (std::list<std::string>::iterator it2 = lanNodes.begin (); it2 != lanNodes.end (); ++it2)
      {
        Ptr<Node> client = Names::Find<Node> (*it2);
        NS_ASSERT (client != 0);

        ClientSection *c_conf = reinterpret_cast<ClientSection*> (configurationMap.at (client));
        NS_ASSERT (c_conf != 0);

        if (c_conf->LanDataRate.compare("none") == 0 || c_conf->LanDelay.compare("none") == 0 ||
            c_conf->QueueType.compare("none") == 0)
          {
            NS_FATAL_ERROR ("Parameters missing for p2p LAN");
          }

        p2p.SetChannelAttribute ("Delay", StringValue (c_conf->LanDelay));
        p2p.SetDeviceAttribute  ("DataRate", StringValue (c_conf->LanDataRate));
        p2p.SetQueue (c_conf->QueueType);

        NetDeviceContainer devs = p2p.Install (client, gateway);

        Ipv4InterfaceContainer ifaces = ipv4h.Assign (devs);

        NS_LOG_INFO ("\nSettings from " << Names::FindName(client) << " to " <<
                     Names::FindName(gateway)<< " p2p:\n\tDelay: " <<
                     c_conf->LanDelay << "\n\tDataRate: " << c_conf->LanDataRate <<
                     "\n\tQueue: " << c_conf->QueueType << "\n\tClient addr: " <<
                     devs.Get(0)->GetAddress() << " ip: " << ifaces.GetAddress(0)
                     << "\n\tgw addr: " << devs.Get(1)->GetAddress() <<
                     " ip: " << ifaces.GetAddress(1));

        c_conf->Address = ifaces.GetAddress (0);

        if (c_conf->CsiVariation.compare("none") != 0)
          {
            NS_FATAL_ERROR ("CSI Variation NOT implemented yet");
          }

        if (lan.EnablePcapUserNetwork)
          {
            p2p.EnablePcap(general.Prefix+"-lan", devs);
          }

        Ipv4StaticRoutingHelper ipv4RoutingHelper;
        Ptr<Ipv4StaticRouting> staticRouting;
        staticRouting = ipv4RoutingHelper.GetStaticRouting (client->GetObject<Ipv4> ());
        //staticRouting->AddNetworkRouteTo(Ipv4Address::GetZero(),
        //                                 Ipv4Mask::GetZero(), ifaces.GetAddress(1), 1);
        staticRouting->SetDefaultRoute(ifaces.GetAddress(1), 1);

        ipv4h.NewNetwork ();
      }
  }
}

void
CreateP2PRemote (BackhaulSection &backhaul, NodeContainer &gwToRemote,
                 NodeContainer &remotes, Ipv4AddressHelper &ipv4h,
                 GeneralSection &general, NodeToConfigurationMap &configurationMap)
{
  PointToPointHelper p2p;

  p2p.SetChannelAttribute ("Delay", StringValue (backhaul.P2PDelay));
  p2p.SetDeviceAttribute  ("DataRate", StringValue (backhaul.P2PDataRate));
  p2p.SetQueue (backhaul.P2PQueueType);

  FOR_EACH_NODE (gwIt, gwToRemote)
  {
    Ptr<Node> gw = (*gwIt);
    NS_ASSERT (gw != 0);

    FOR_EACH_NODE (remoteIt, remotes)
    {

      Ptr<Node> remote = (*remoteIt);
      NS_ASSERT (remote != 0);

      NetDeviceContainer devs = p2p.Install(gw, remote);

      Ipv4InterfaceContainer ifaces = ipv4h.Assign (devs);

      configurationMap.at(remote)->Address = ifaces.GetAddress (1);

      NS_LOG_INFO ("\nSettings from " << Names::FindName(gw) << " to " <<
                   Names::FindName(remote)<< " p2p:\n\tDelay: " <<
                   backhaul.P2PDelay << "\n\tDataRate: " << backhaul.P2PDataRate <<
                   "\n\tQueue: " << backhaul.P2PQueueType << "\n\tgw addr: " <<
                   devs.Get(0)->GetAddress() << " ip: " << ifaces.GetAddress(0)
                   << "\n\tremote addr: " << devs.Get(1)->GetAddress() <<
                   " ip: " << ifaces.GetAddress(1));

      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      Ptr<Ipv4StaticRouting> remoteHostStaticRouting;
      remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remote->GetObject<Ipv4> ());
      //remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address::GetZero(),
      //                                           Ipv4Mask::GetZero(), ifaces.GetAddress(0), 1);
      remoteHostStaticRouting->SetDefaultRoute(ifaces.GetAddress(0), 1);

      ipv4h.NewNetwork();
    }
  }

  if (backhaul.EnablePcap)
    {
      p2p.EnablePcap(general.Prefix+"-backhaul", remotes);
    }
}

void
CreateREM (LteSection &lte, GeneralSection &general)
{
  static Ptr<RadioEnvironmentMapHelper> remHelper = CreateObject<RadioEnvironmentMapHelper> ();
  PrintGnuplottableEnbListToFile (general.Prefix + "-enbs.txt");
  PrintGnuplottableUeListToFile (general.Prefix + "-ues.txt");
  remHelper->SetAttribute ("ChannelPath", StringValue ("/ChannelList/0"));
  remHelper->SetAttribute ("OutputFile", StringValue (general.Prefix + "-rem.txt"));
  remHelper->SetAttribute ("XMin", DoubleValue (lte.XMinREM));
  remHelper->SetAttribute ("XMax", DoubleValue (lte.XMaxREM));
  remHelper->SetAttribute ("YMin", DoubleValue (lte.YMinREM));
  remHelper->SetAttribute ("YMax", DoubleValue (lte.YMaxREM));
  remHelper->SetAttribute ("Earfcn", UintegerValue (1575));
  remHelper->SetAttribute ("Z", DoubleValue (lte.HeightREM));
  remHelper->Install ();
}

void
DebugNames (NodeContainer &nodes)
{
  FOR_EACH_NODE (it, nodes)
  {
    Ptr<Node> node = *it;
    NS_LOG_INFO ("Id: " << node->GetId() << " has Name=" << Names::FindName(node));
  }
}

static void
FlowMonStats (Ptr<FlowMonitor> monitor, FlowMonitorHelper &flowmon,
              NodeContainer &lanClients, NodeContainer &remotes,
              NodeToConfigurationMap &configurationMap, GeneralSection &general,
              StatisticsSection &statistics)
{
  Ptr<Ipv4FlowClassifier> classifier;
  std::map<FlowId, FlowMonitor::FlowStats> stats;
  Ipv4FlowClassifier::FiveTuple t;

  monitor->CheckForLostPackets();
  monitor->SerializeToXmlFile(general.Prefix+"-flowmon.xml", true,
                              true);
  classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  stats = monitor->GetFlowStats ();

  for (FlowMap iter = stats.begin (); iter != stats.end (); ++iter)
    {
      t = classifier->FindFlow (iter->first);

      Ptr<Node> source = 0;
      Ptr<Node> dest = 0;

      FOR_EACH_NODE (it, lanClients)
      {
        Ptr<Node> client = (*it);
        Ipv4Address clientAddr = configurationMap.at(client)->Address;
        if (t.sourceAddress.IsEqual(clientAddr))
          {
            source = client;
          }
        else if (t.destinationAddress.IsEqual (clientAddr))
          {
            dest = client;
          }
      }

      FOR_EACH_NODE (it, remotes)
      {
        Ptr<Node> remote = (*it);
        Ipv4Address remoteAddr = configurationMap.at(remote)->Address;
        if (t.sourceAddress.IsEqual(remoteAddr))
          {
            source = remote;
          }
        else if (t.destinationAddress.IsEqual (remoteAddr))
          {
            dest = remote;
          }
      }

      if (source == 0 || dest == 0)
        {
          NS_FATAL_ERROR ("Can't print stats for flow src=" << t.sourceAddress
                          << " dst= " << t.destinationAddress);
        }

      std::ofstream outFile;
      FlowMonitor::FlowStats stat = iter->second;

      outFile.open((general.Prefix+"-"+Names::FindName(source)+"-"+
                   Names::FindName(dest)+"-l3stats.txt").c_str());

      outFile << "#TX (p)\tRX(p)\tLOST (p)\tAVGDELAY(ns)\tAVGJITTER(ns)" << std::endl;

      outFile << stat.txBytes << "\t" << stat.rxBytes << "\t" <<
             stat.lostPackets << "\t";

      if (stat.rxPackets > 0)
        {
          Time delay = stat.delaySum / stat.rxPackets;
          Time jitter = stat.jitterSum / stat.rxPackets;
          outFile << delay.GetMilliSeconds()
                << "\t" << jitter.GetMilliSeconds() << std::endl;
        }
    }
}

static void
ConnectClientStatistics (NodeContainer &container, Statistics &simStats,
                         StatisticsSection &statistics)
{
  FOR_EACH_NODE (it, container)
  {
    Ptr<Node> node = *it;
    std::string name = Names::FindName(node);

    //NS_LOG_DEBUG ("Connecting statistics Trace for " << name);

    Ptr<Ipv4L3Protocol> ipv4L3Protocol = node->GetObject<Ipv4L3Protocol> ();
    ipv4L3Protocol->TraceConnect("Tx", name,
                                 MakeCallback(&Statistics::Ipv4ClientTxCallback,
                                              &simStats));
    ipv4L3Protocol->TraceConnect("Rx", name,
                                 MakeCallback(&Statistics::Ipv4ClientRxCallback,
                                              &simStats));
    if (statistics.EnableQueue)
      {
        ipv4L3Protocol->TraceConnect("Drop", name,
                                     MakeCallback(&Statistics::Ipv4ClientDropCallback,
                                                  &simStats));
      }
  }
}

static void
ConnectGatewayStatistics (NodeContainer &container, Statistics &simStats,
                          StatisticsSection &statistics)
{
  FOR_EACH_NODE (it, container)
  {
    Ptr<Node> node = *it;
    std::string name = Names::FindName(node);

    //NS_LOG_DEBUG ("Connecting statistics Trace for " << name);

    Ptr<Ipv4L3Protocol> ipv4L3Protocol = node->GetObject<Ipv4L3Protocol> ();
    ipv4L3Protocol->TraceConnect("Tx", name,
                                 MakeCallback(&Statistics::Ipv4GatewayTxCallback,
                                              &simStats));
    ipv4L3Protocol->TraceConnect("Rx", name,
                                 MakeCallback(&Statistics::Ipv4GatewayRxCallback,
                                              &simStats));
    if (statistics.EnableQueue)
      {
        ipv4L3Protocol->TraceConnect("Drop", name,
                                     MakeCallback(&Statistics::Ipv4GatewayDropCallback,
                                                  &simStats));
      }
  }
}

static void
ConnectRemoteStatistics (NodeContainer &container, Statistics &simStats,
                         StatisticsSection &statistics)
{
  FOR_EACH_NODE (it, container)
  {
    Ptr<Node> node = *it;
    std::string name = Names::FindName(node);

    //NS_LOG_DEBUG ("Connecting statistics Trace for " << name);

    Ptr<Ipv4L3Protocol> ipv4L3Protocol = node->GetObject<Ipv4L3Protocol> ();
    ipv4L3Protocol->TraceConnect("Tx", name,
                                 MakeCallback(&Statistics::Ipv4RemoteTxCallback,
                                              &simStats));
    ipv4L3Protocol->TraceConnect("Rx", name,
                                 MakeCallback(&Statistics::Ipv4RemoteRxCallback,
                                              &simStats));
    if (statistics.EnableQueue)
      {
        ipv4L3Protocol->TraceConnect("Drop", name,
                                     MakeCallback(&Statistics::Ipv4RemoteDropCallback,
                                                  &simStats));
      }
  }
}

static AppSection* GetAppSectionConfFor (const std::string &type,
                                         const std::string &name, INIReader &cfg)
{
  AppSection *appConf;

  if (type.compare("none") == 0)
    {
      return 0;
    }
  else if (type.compare("BulkSend") == 0)
    {
      BulkSendSection *aConf = new BulkSendSection (name);
      aConf->Fill(cfg);
      appConf = aConf;
    }
  else if (type.compare("PacketSink") == 0)
    {
      appConf = new AppSection (name);
      appConf->Fill(cfg);
    }
  else if (type.compare("OnOff") == 0)
    {
      OnOffSection *aConf = new OnOffSection (name);
      aConf->Fill(cfg);
      appConf = aConf;
    }
  else
    {
      NS_FATAL_ERROR ("App type " << type << " not recognized. Please use" <<
                      " BulkSend, OnOff, or PacketSink");
    }

  return appConf;
}

static Ptr<Socket>
CreateSocket (AppSection *appConf, Ptr<Node> node)
{
  Ptr<Socket> socket;
  if (appConf->Protocol.compare("TCP") == 0)
    {
      Ptr<TcpL4Protocol> proto = node->GetObject<TcpL4Protocol> ();
      NS_ASSERT (proto != 0);
      proto->SetAttribute("SocketType",
                          TypeIdValue(TypeId::LookupByName(appConf->SocketType)));

      socket = proto->CreateSocket ();

      if (socket == 0)
        {
          NS_FATAL_ERROR ("SocketType " << appConf->SocketType <<
                          " is not valid. Try ns3::TcpCubic");
        }

      socket->SetAttribute ("InitialCwnd", UintegerValue (appConf->InitialCwnd));
      socket->SetAttribute ("InitialSlowStartThreshold", UintegerValue (appConf->InitialSSTh));
      socket->SetAttribute ("DelAckCount", UintegerValue (appConf->DelAckCount));

      /* Special attributes for special socket */

      if (appConf->SocketType.compare("TcpNoordwijk") == 0 ||
          appConf->SocketType.compare("TcpNoordwijkMw") == 0)
        {
          socket->SetAttribute ("TxTime", TimeValue (Seconds (appConf->TxTime)));
          socket->SetAttribute ("B", TimeValue (Seconds (appConf->TxTime)));
        }
    }
  else if (appConf->Protocol.compare("UDP") == 0)
    {
      Ptr<UdpL4Protocol> proto = node->GetObject<UdpL4Protocol> ();
      NS_ASSERT (proto != 0);

      socket = proto->CreateSocket();
    }
  else
    {
      NS_FATAL_ERROR ("What kind of protocol is " << appConf->Protocol <<
                      " ?!?! Accepted values are TCP or UDP");
    }

  return socket;
}

static Ptr<BulkSendApplication>
CreateBulkSend (AppSection *appConf, Ptr<Socket> socket, AddressValue &addrTo)
{
  Ptr<BulkSendApplication> app = CreateObject <BulkSendApplication> ();
  BulkSendSection *bulkConf = dynamic_cast<BulkSendSection*> (appConf);
  NS_ASSERT (bulkConf != 0);

  app->SetAttribute ("MaxBytes", UintegerValue (bulkConf->MaxBytes));
  app->SetAttribute ("SendSize", UintegerValue (bulkConf->SendSize));
  app->SetAttribute ("Remote", AddressValue (addrTo));
  app->SetSocket (socket);

  return app;
}

static Ptr<PacketSink>
CreatePacketSink (AppSection *appConf, Ptr<Socket> socket, AddressValue &addrTo)
{
  Ptr<PacketSink> app = CreateObject <PacketSink> ();
  app->SetAttribute("Local", AddressValue (addrTo));
  app->SetSocket (socket);

  if (appConf->Protocol.compare("TCP") == 0)
    {
      app->SetAttribute("Protocol", TypeIdValue (TypeId::LookupByName ("ns3::TcpSocketFactory")));
    }
  else if (appConf->Protocol.compare("UDP") == 0)
    {
      app->SetAttribute("Protocol", TypeIdValue (TypeId::LookupByName ("ns3::UdpSocketFactory")));
    }

  return app;
}

static Ptr<OnOffApplication>
CreateOnOff (AppSection *appConf, Ptr<Socket> socket, AddressValue &addrTo)
{
  Ptr<OnOffApplication> app = CreateObject <OnOffApplication> ();
  OnOffSection *onOffConf = dynamic_cast<OnOffSection*> (appConf);
  NS_ASSERT (onOffConf != 0);

  if (appConf->Protocol.compare("TCP") == 0)
    {
      app->SetAttribute("Protocol", TypeIdValue (TypeId::LookupByName ("ns3::TcpSocketFactory")));
    }
  else if (appConf->Protocol.compare("UDP") == 0)
    {
      app->SetAttribute("Protocol", TypeIdValue (TypeId::LookupByName ("ns3::UdpSocketFactory")));
    }

  app->SetAttribute ("PacketSize", UintegerValue (onOffConf->SendSize));
  app->SetAttribute("MaxBytes", UintegerValue (onOffConf->MaxBytes));
  app->SetAttribute ("DataRate", DataRateValue (DataRate (onOffConf->DataRate)));
  app->SetAttribute("OnTime", StringValue (onOffConf->OnTime));
  app->SetAttribute("OffTime", StringValue (onOffConf->OffTime));
  app->SetAttribute("Remote", AddressValue (addrTo));
  app->SetSocket (socket);

  return app;
}

static Ptr<Application>
CreateApplication (AppSection *appConf, Ptr<Socket> socket,
                   Ptr<Node> node, AddressValue &addrTo, const std::string &name)
{
  Ptr<Application> app;

  InetSocketAddress inetAddr = InetSocketAddress::ConvertFrom(addrTo.Get());
  Ipv4Address ipv4Addr = inetAddr.GetIpv4();

  if (appConf->AppType.compare("BulkSend") == 0)
    {
      app = CreateBulkSend (appConf, socket, addrTo);
      NS_LOG_DEBUG ("Creating " << name << " installed on " << Names::FindName(node) <<
                    " connected to " << ipv4Addr << " port " << appConf->Port <<
                    " type BulkSendApplication");
    }
  else if (appConf->AppType.compare("PacketSink") == 0)
    {
      app = CreatePacketSink (appConf, socket, addrTo);
      NS_LOG_DEBUG ("Creating " << name << " installed on " << Names::FindName(node) <<
                    " which listen on " << ipv4Addr << " port " << appConf->Port <<
                    " type PacketSink");
    }
  else if (appConf->AppType.compare("OnOff") == 0)
    {

      app = CreateOnOff(appConf, socket, addrTo);
      NS_LOG_DEBUG ("Creating " << name << " installed on " << Names::FindName(node) <<
                    " connected to " << ipv4Addr << " port " << appConf->Port <<
                    " type OnOff");
    }
  else
    {
      NS_FATAL_ERROR ("App type " << appConf->AppType <<
                      " not supported. Accepted values are BulkSend," <<
                      " OnOff, PacketSink");
    }

  app->SetStartTime(Time::FromDouble (appConf->StartTime, Time::S));
  app->SetStopTime(Time::FromDouble (appConf->StopTime, Time::S));

  return app;
}

static void
BuildServices (NodeToConfigurationMap &configurationMap, INIReader &cfg)
{
  ApplicationContainer apps;

  for (uint32_t i=0; i<UINT32_MAX; ++i)
    {
      NS_ASSERT (i<=1000);

      std::stringstream ss;
      ss << "app" << i;
      std::string type = cfg.Get(ss.str(), "AppType", "none");

      AppSection *appConf = GetAppSectionConfFor (type, ss.str(), cfg);

      if (appConf == 0)
        {
          break;
        }

      Ipv4Address ipv4AddrTo;
      AddressValue addrTo;

      if (appConf->ConnectedTo.compare("all") == 0 ||
          appConf->ConnectedTo.compare("any") == 0)
        {
          addrTo = AddressValue (InetSocketAddress (Ipv4Address::GetAny (), appConf->Port));
        }
      else
        {
          Ptr<Node> remote = Names::Find<Node> (appConf->ConnectedTo);
          if (remote == 0)
            {
              NS_FATAL_ERROR ("Cannot connect application " << ss.str () <<
                              " to node " << appConf->ConnectedTo <<
                              " because it does not exist");
            }

          ipv4AddrTo = configurationMap.at(remote)->Address;
          addrTo  = AddressValue (InetSocketAddress (ipv4AddrTo, appConf->Port));
        }

      Ptr<Node> node = Names::Find<Node> (appConf->InstalledOn);

      if (node == 0)
        {
          NS_FATAL_ERROR ("Cannot install application " << ss.str() <<
                          " on node " << appConf->InstalledOn <<
                          " because it does not exist");
        }

      Ptr<Socket> socket = CreateSocket (appConf, node);
      Ptr<Application> app = CreateApplication (appConf, socket, node, addrTo, ss.str());
      apps.Add(app);

      node->AddApplication(app);

      delete appConf;
    }
}

static int
BuildAndPrintExample ()
{
  GeneralSection general;
  LanSection lan;
  BackhaulSection backhaul;
  LteSection lte;
  StatisticsSection statistics;
  ClientSection client0 ("client0");
  ClientSection client1 ("client1");
  ClientSection client2 ("client2");

  GatewaySection gateway0 ("gateway0");
  RemoteSection remote0 ("remote0");
  RemoteSection remote1 ("remote1");

  BulkSendSection bulk0 ("app0");
  BulkSendSection bulk1 ("app1");
  OnOffSection onOff0 ("app2");
  AppSection sink0 ("app3");
  AppSection sink1 ("app4");
  AppSection sink2 ("app5");

  general.RemoteN = 2;
  lan.ClientN = 3;
  lan.NPerG = "[gateway0:client0,client1,client2]";

  client0.Position = "95.0, 0.0, 1.5";
  client1.Position = "90.0, 0.0, 1.5";
  client2.Position = "95.0, 5.0, 1.5";

  bulk0.AppType = "BulkSend";
  bulk0.InstalledOn = "client0";
  bulk0.ConnectedTo = "remote0";
  bulk0.Port = 9;

  bulk1.AppType = "BulkSend";
  bulk1.InstalledOn = "remote1";
  bulk1.ConnectedTo = "client1";
  bulk1.Port = 10;

  onOff0.AppType = "OnOff";
  onOff0.InstalledOn = "client2";
  onOff0.ConnectedTo = "remote0";
  onOff0.Port = 11;

  sink0.AppType = "PacketSink";
  sink0.InstalledOn = "remote0";
  sink0.ConnectedTo = "any";
  sink0.Port = 9;

  sink1.AppType = "PacketSink";
  sink1.InstalledOn = "client1";
  sink1.ConnectedTo = "any";
  sink1.Port = 10;

  sink2.AppType = "PacketSink";
  sink2.InstalledOn = "remote0";
  sink2.ConnectedTo = "any";
  sink2.Port = 11;

  general.PrintExample();
  lan.PrintExample();
  backhaul.PrintExample();
  lte.PrintExample();
  statistics.PrintExample();

  client0.PrintExample();
  client1.PrintExample();
  client2.PrintExample();
  gateway0.PrintExample();
  remote0.PrintExample();
  remote1.PrintExample();

  bulk0.PrintExample();
  bulk1.PrintExample();
  onOff0.PrintExample();
  sink1.PrintExample();
  sink2.PrintExample();
  sink0.PrintExample();

  return 0;
}

int
main (int argc, char *argv[])
{
  CommandLine   cmd;
  std::string   configFilePath;
  bool          printExample = false;
  bool          printDayToDay = false;

  cmd.AddValue ("ConfigurationFile", "Configuration file path", configFilePath);
  cmd.AddValue ("PrintExample", "Print an example configuration file and exit",
                printExample);
  cmd.AddValue ("PrintDayToDay", "Print an example configuration for d2d and exit",
                printDayToDay);

  cmd.Parse    (argc, argv);

  /*
  Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.txt"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig;
  outputConfig.ConfigureDefaults ();
  outputConfig.ConfigureAttributes ();
  */

  if (!printExample && !printDayToDay && configFilePath.empty ())
    {
      NS_FATAL_ERROR ("No configuration file. Running dummy simulation.. Done.");
    }

  INIReader cfg (configFilePath);
  GeneralSection general;
  LanSection lan;
  BackhaulSection backhaul;
  LteSection lte;
  StatisticsSection statistics;

  if (printExample)
    {
      return BuildAndPrintExample();
    }
  else if (printDayToDay)
    {
      PrintDayToDay();
      return 0;
    }

  general.Fill (cfg);
  lan.Fill (cfg);
  backhaul.Fill (cfg);
  statistics.Fill (cfg);

  if (general.Verbose)
    {
      NS_LOG_UNCOND ("#### Configuration recap:");
      general.Print ();
      lan.Print ();
      backhaul.Print ();
      NS_LOG_UNCOND ("#### End.");
    }

  NodeContainer lanClients; // lan clients
  NodeContainer gateways;   // gateway
  NodeContainer remotes;    // remote node (1, for today)

  NodeContainer gwToRemote; // will contain the nodes which should connect to remote

  NodeToConfigurationMap configurationMap;

  NS_LOG_INFO ("Loaded configuration. Now, building the network");

  ClientMaker (lanClients, configurationMap, cfg, general.Verbose, general.EnableC2ML,
               lan.ClientN);
  GatewayMaker (gateways, configurationMap, cfg, general.Verbose, lan.GatewayN);
  RemoteMaker (remotes, configurationMap, cfg, general.Verbose, general.RemoteN);

  NodeContainer all (lanClients, gateways, remotes);

  NS_ASSERT (configurationMap.size() == (lanClients.GetN() + gateways.GetN() +
                                         remotes.GetN()));

  DebugNames (lanClients);
  DebugNames (gateways);
  DebugNames (remotes);

  InstallMobilityOnContainer (all, configurationMap, general.Verbose);

  // LAN Creation
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("7.0.0.0", "255.255.255.0");

  if (lan.Type.compare("p2p") == 0)
    {
      gwToRemote = gateways;
      CreateP2PLan (gateways, lan, configurationMap, ipv4h, general);
    }
  else if (lan.Type.compare("lte") == 0)
    {
      lte.Fill (cfg);

      if (general.Verbose)
        {
          lte.Print ();
        }

      gwToRemote = CreateLteLAN (gateways, lanClients, configurationMap,
                                 lan, general, lte);

      if (lte.EnableREM)
        {
          if (general.Verbose)
            {
              NS_LOG_UNCOND ("Generating REM file and exiting");
            }

          CreateREM (lte, general);
        }
    }
  else
    {
      NS_FATAL_ERROR ("Lan " << lan.Type << " not supported");
    }

  NS_ASSERT (gwToRemote.GetN () > 0);

  // Build now the Remote Network.
  ipv4h.SetBase("192.168.0.0", "255.255.255.0");

  if (backhaul.Type.compare ("p2p") == 0)
    {
      CreateP2PRemote(backhaul, gwToRemote, remotes, ipv4h, general,
                      configurationMap);
    }
  else
    {
      NS_FATAL_ERROR ("Backhaul type " << backhaul.Type << " not supported");
    }

  if (lan.Type.compare("p2p") == 0)
    {
      //Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
    }
  else if (lan.Type.compare("lte") == 0)
    {
      // Static routing already did.
    }

  // Build the services
  BuildServices (configurationMap, cfg);

  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor;
  NodeContainer flow_nodes;

  if (statistics.EnableFlowmon)
    {
      flow_nodes.Add(lanClients);
      flow_nodes.Add(remotes);
      monitor = flowmon.Install(flow_nodes);
      monitor->SetAttribute ("DelayBinWidth", DoubleValue(0.001));
      monitor->SetAttribute ("JitterBinWidth",DoubleValue (0.001));
    }

  // Stats connecting.. to/from tracesources
  Statistics simStats (statistics.ThroughputSamplingTime);
  ConnectClientStatistics(lanClients, simStats, statistics);
  ConnectGatewayStatistics(gateways, simStats, statistics);
  ConnectRemoteStatistics(remotes, simStats, statistics);

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (general.StopTime));

  if (statistics.EnableNetAnim)
    {
      AnimationInterface anim ("animation.xml");

      anim.EnableIpv4RouteTracking("animation-routing.xml", Time(0), Seconds (general.StopTime),
                               NodeContainer(lanClients, gateways, remotes, gwToRemote));
    }

  Simulator::Run ();
  Simulator::Destroy();
  NS_LOG_INFO ("Done.");

  // Output the things
  if (statistics.EnableFlowmon)
    {
      FlowMonStats (monitor, flowmon, lanClients, remotes, configurationMap,
                    general, statistics);
    }

  simStats.OutputStat(statistics, general.Prefix);

  // DELETE part
  for (NodeConfigurationIterator it = configurationMap.begin (); it != configurationMap.end (); ++it)
    {
      delete it->second;
    }

  return 0;
}
