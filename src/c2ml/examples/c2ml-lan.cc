/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include <iostream>
#include <string>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/c2ml-module.h"
#include "ns3/log.h"
#include "ns3/config-store-module.h"
#include "ns3/stats-module.h"

#include "INIReader.h"
#include "c2ml-utils.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("C2MLLan");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  NodeContainer clients;
  std::string configFilePath;

  cmd.AddValue ("ConfigurationFile", "Configuration file path", configFilePath);
  cmd.Parse (argc, argv);

  if (configFilePath.empty ())
    {
      std::string msg ="No configuration file. Running dummy simulation.. Done.";
      NS_FATAL_ERROR (msg);
    }

  INIReader cfg (configFilePath);

  if (cfg.ParseError () != 0)
    {
      std::string msg="File doesn't exist, or parsing errors. Please check";
      NS_FATAL_ERROR (msg);
    }

  SimulationConfiguration simulationConf;

  INI_READ_BOOL (cfg, "general", "EnableC2ML",            simulationConf.enableC2ML);
  INI_READ_BOOL (cfg, "general", "EnablePCAP",            simulationConf.enablePcap);
  INI_READ_BOOL (cfg, "general", "EnableQueueStatistics", simulationConf.enableQueueStatistics);
  INI_READ_BOOL (cfg, "general", "EnableCwndStatistics",  simulationConf.enableCwndStatistics);
  INI_READ_BOOL (cfg, "general", "EnableSSThStatistics",  simulationConf.enableSSThStatistics);
  INI_READ_BOOL (cfg, "general", "EnableRTTStatistics",   simulationConf.enableRTTStatistics);
  INI_READ_BOOL (cfg, "general", "EnableDataRxStatistics",simulationConf.enableDataRxStatistics);
  INI_READ_BOOL (cfg, "general", "Verbose",               simulationConf.verbose);

  INI_READ_REAL (cfg,   "simulation", "StopTime",           simulationConf.stopTime);
  INI_READ_STRING (cfg, "simulation", "FilePrefix",         simulationConf.filePrefix);

  INI_READ_UINT32 (cfg, "network", "SegmentSize",     simulationConf.segmentSize);
  INI_READ_UINT32 (cfg, "network", "BytesToTransmit", simulationConf.bytesToTransmit);
  INI_READ_UINT32 (cfg, "network", "Nodes",           simulationConf.nodes);
  INI_READ_UINT32 (cfg, "network", "L2QueueSize",     simulationConf.queueSize);

  INI_READ_UINT32 (cfg, "satellite", "ByteBandwidth",   simulationConf.satByteBw);
  INI_READ_STRING (cfg, "satellite", "StringBandwidth", simulationConf.satStringBw);
  INI_READ_STRING (cfg, "satellite", "Delay",           simulationConf.satDelay);

  INI_READ_UINT32 (cfg, "socket", "DelAckCount", simulationConf.delAckCount);
  INI_READ_UINT32 (cfg, "socket", "SlowStartTh", simulationConf.ssthres);
  INI_READ_UINT32 (cfg, "socket", "InitialCwnd", simulationConf.initialCwnd);
  INI_READ_UINT32 (cfg, "socket", "NoordB",      simulationConf.noord_b);
  INI_READ_UINT32 (cfg, "socket", "NoordTx",     simulationConf.noord_tx);

  INI_READ_UINT32 (cfg, "gateway", "AllocationProtocol", simulationConf.allocationProtocol);
  INI_READ_STRING (cfg, "gateway", "InputQueueTid",      simulationConf.inputQueueTid);
  INI_READ_STRING (cfg, "gateway", "OutputQueueTid",     simulationConf.outputQueueTid);
  INI_READ_STRING (cfg, "gateway", "QueueAttributes",    simulationConf.queueAttributes);

  if (! simulationConf.queueAttributes.empty())
    {
      Config::SetDefault ("ns3::ConfigStore::Filename", StringValue (simulationConf.queueAttributes));
      Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Load"));
      Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
      ConfigStore inputConfig;
      inputConfig.ConfigureDefaults ();
    }

  for (uint32_t i=0; i<simulationConf.nodes; ++i)
    {
      Ptr<Node> client = CreateObject<Node> ();
      clients.Add (client);

      std::stringstream ss;
      ss << i;
      Names::Add (ss.str (), client);

      ClientConfiguration *c = new ClientConfiguration;
      c->name         = ss.str ();
      c->socketType   = cfg.Get     (ss.str (), "SocketType",    "TcpNewReno");
      c->startTime    = cfg.GetReal (ss.str (), "StartTime",     0.0);
      c->stopTime     = cfg.GetReal (ss.str (), "StopTime",      simulationConf.stopTime);
      c->lanDelay     = cfg.Get     (ss.str (), "OverheadDelay", "0ms");
      c->lanDataRate  = cfg.Get     (ss.str (), "DataRate",      "2Mbps");
      c->csiVariation = cfg.Get     (ss.str (), "CSIVariation",  "");

      simulationConf.clientConfig.insert (ClientConfigurationPair (client, c));

      InternetStackHelper internet;

      if (simulationConf.enableC2ML && c->socketType.find ("Mw") != std::string::npos)
        {
          internet.SetTcp ("ns3::CCL45Protocol");
        }

      internet.Install (client);
    }

  if (simulationConf.verbose)
    {
      NS_LOG_UNCOND ("General section");
      NS_LOG_UNCOND ("\tEnableC2ML: "               << simulationConf.enableC2ML <<
                     "\n\tEnablePCAP: "             << simulationConf.enablePcap <<
                     "\n\tEnableQueueStatistics: "  << simulationConf.enableQueueStatistics <<
                     "\n\tEnableCwndStatistics: "   << simulationConf.enableCwndStatistics <<
                     "\n\tEnableDataRxStatistics: " << simulationConf.enableDataRxStatistics <<
                     "\n\tEnableRTTStatistics: "    << simulationConf.enableRTTStatistics);

      NS_LOG_UNCOND ("Simulation section");
      NS_LOG_UNCOND ("\n\tStopTime: "         << simulationConf.stopTime << " s" <<
                     "\n\tFilePrefix: "       << simulationConf.filePrefix);

      NS_LOG_UNCOND ("Network section");
      NS_LOG_UNCOND ("\tSegmentSize: "       << simulationConf.segmentSize << " B" <<
                     "\n\tBytesToTransmit: " << simulationConf.bytesToTransmit << " B" <<
                     "\n\tQueueSize: "       << simulationConf.queueSize << " packets" <<
                     "\n\tNodes: "           << simulationConf.nodes << " + gateway and remote");

      NS_LOG_UNCOND ("Satellite section");
      NS_LOG_UNCOND ("\tStringBandwidth: " << simulationConf.satStringBw <<
                     "\n\tByteBandwidth: " << simulationConf.satByteBw << " B" <<
                     "\n\tDelay: "         << simulationConf.satDelay);

      NS_LOG_UNCOND ("Gateway section");
      NS_LOG_UNCOND ("\tAllocation Protocol:" << simulationConf.allocationProtocol);
      NS_LOG_UNCOND ("\tInput Queue:"         << simulationConf.inputQueueTid);
      NS_LOG_UNCOND ("\tOutput Queue:"        << simulationConf.outputQueueTid);
    }

  SET_DEFAULT_UINTEGER ("ns3::TcpSocket::SegmentSize",        simulationConf.segmentSize);
  SET_DEFAULT_UINTEGER ("ns3::DropTailQueue::MaxPackets",     simulationConf.queueSize);
  SET_DEFAULT_UINTEGER ("ns3::DropTailQueue::MaxBytes",       simulationConf.queueSize*(simulationConf.segmentSize+60));
  SET_DEFAULT_UINTEGER ("ns3::RedQueue::QueueLimit",          simulationConf.queueSize);
  SET_DEFAULT_UINTEGER ("ns3::CoDelQueue::MaxPackets",        simulationConf.queueSize);
  SET_DEFAULT_UINTEGER ("ns3::CoDelQueue::MaxBytes",          simulationConf.queueSize*(simulationConf.segmentSize+60));
  SET_DEFAULT_DOUBLE   ("ns3::RedQueue::MinTh",               100.0);
  SET_DEFAULT_DOUBLE   ("ns3::RedQueue::MaxTh",               abs(simulationConf.queueSize-100));
  SET_DEFAULT_UINTEGER ("ns3::C2MLGateway::Bandwidth",        simulationConf.satByteBw);
  SET_DEFAULT_UINTEGER ("ns3::TcpSocket::DelAckCount",        simulationConf.delAckCount);
  SET_DEFAULT_UINTEGER ("ns3::TcpSocket::InitialSlowStartThreshold", simulationConf.ssthres);
  SET_DEFAULT_UINTEGER ("ns3::TcpSocket::InitialCwnd",        simulationConf.initialCwnd);
  SET_DEFAULT_UINTEGER ("ns3::TcpSocket::SndBufSize", UintegerValue (2621440)); // *Bytes*
  SET_DEFAULT_UINTEGER ("ns3::TcpSocket::RcvBufSize", UintegerValue (2621440)); // *Bytes*

  SET_DEFAULT_TIME ("ns3::TcpNoordwijk::TxTime", MilliSeconds (simulationConf.noord_tx));
  SET_DEFAULT_TIME ("ns3::TcpNoordwijk::B",      MilliSeconds (simulationConf.noord_b));

  SimulationStatistics simulationStat (simulationConf);
  simulationStat.rttFn = new EventId[simulationConf.nodes];

  NS_LOG_INFO ("Create Gateway and remote nodes.");

  Ptr<Node> remoteHost = CreateNode ();
  Ptr<Node> gatewayHost = CreateNode ();

  Names::Add ("remote", remoteHost);
  Names::Add ("gateway", gatewayHost);

  // Don't pollute main with c e d names.
  {
    ClientConfiguration *c = new ClientConfiguration;
    ClientConfiguration *d = new ClientConfiguration;

    c->name = "gateway";
    c->startTime = 0.0;
    c->stopTime = simulationConf.stopTime;
    c->csiVariation = "";

    d->name = "remote";
    d->startTime = 0.0;
    d->stopTime = simulationConf.stopTime;
    d->csiVariation = "";

    simulationConf.clientConfig.insert (ClientConfigurationPair (gatewayHost, c));
    simulationConf.clientConfig.insert (ClientConfigurationPair (remoteHost,  d));
  }

  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2pLan;
  PointToPointHelper p2pSat;

  p2pSat.SetDeviceAttribute  ("DataRate", StringValue (simulationConf.satStringBw));
  p2pSat.SetChannelAttribute ("Delay",    StringValue (simulationConf.satDelay));

  InstallLANLink (p2pLan, clients, simulationConf, simulationStat);
  InstallSatelliteLink (p2pSat, simulationConf, simulationStat);

  NS_LOG_INFO ("Writing routing table");

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  if (simulationConf.enableC2ML)
    {
      NS_LOG_INFO ("Enabling C2ML");

      Ptr<C2MLGateway> bmw = CreateObject<C2MLGateway> ();
      AddressValue gatewayAddress (InetSocketAddress (simulationConf.clientConfig.at (gatewayHost)->address, 25522));

      bmw->SetAttribute ("Local", gatewayAddress);
      bmw->SetAttribute ("Mode", UintegerValue (simulationConf.allocationProtocol));
      if (simulationConf.inputQueueTid == "ns3::C2MLRxQueue")
        {
          bmw->SetAttribute("AQM", BooleanValue(true));

          Ptr<C2MLRxQueue> rxQueue = CreateObject<C2MLRxQueue> ();
          Ptr<C2MLTxQueue> txQueue = CreateObject<C2MLTxQueue> ();

          rxQueue->SetQDiscManagementFriend(txQueue);

          Ptr<NetDevice> sat0 = Names::Find<NetDevice> (Names::FindName (gatewayHost) +"/sat0");
          Ptr<Ipv4L3Protocol> ipv4 = gatewayHost->GetObject<Ipv4L3Protocol> ();

          ipv4->SetInputQueue(sat0, rxQueue);
          ipv4->SetOutputQueue(sat0, txQueue);
        }
      else
        {
          ObjectFactory f;
          f.SetTypeId(simulationConf.inputQueueTid);

          Ptr<Queue> rx = DynamicCast<Queue> (f.Create());

          f.SetTypeId(simulationConf.outputQueueTid);
          Ptr<Queue> tx = DynamicCast<Queue> (f.Create());

          Ptr<NetDevice> sat0 = Names::Find<NetDevice> (Names::FindName (gatewayHost) +"/sat0");
          Ptr<Ipv4L3Protocol> ipv4 = gatewayHost->GetObject<Ipv4L3Protocol> ();

          ipv4->SetInputQueue(sat0, rx);
          ipv4->SetOutputQueue(sat0, tx);
        }

      gatewayHost->AddApplication (bmw);

      ApplicationContainer bmwContainer;
      bmwContainer.Add (bmw);
      bmwContainer.Start (Seconds (0.0));
      bmwContainer.Stop  (Seconds (simulationConf.stopTime));

      FOR_EACH_CLIENT (it, clients)
        {
          Ptr<CCL45Protocol> mw;
          Ptr<Node> client = (*it);
          ClientConfiguration* c = simulationConf.clientConfig.at (client);

          if (c->socketType.find ("Mw") == std::string::npos)
            {
              continue;
            }

          mw = client->GetObject<CCL45Protocol> ();
          NS_ASSERT_MSG (mw != 0, "Are you sure you enabled c2ml before"\
                         " node configuration in the cmd line?");

          mw->SetAttribute ("Local", AddressValue (InetSocketAddress (c->address, 25523)));
          mw->SetAttribute ("Remote", gatewayAddress);
        }

      SET_DEFAULT_BOOL ("ns3::TcpSocketBase::SendInAck", false);
    }

  if (simulationConf.enableQueueStatistics)
    {
      Ptr<NetDevice> gatewayDev = Names::Find<NetDevice> (Names::FindName (gatewayHost) +"/sat0");
      Ptr<Ipv4L3Protocol> ipv4L3Protocol = gatewayHost->GetObject<Ipv4L3Protocol> ();
      Ptr<Queue> inputQ = ipv4L3Protocol->GetInputQueue(gatewayDev);
      Ptr<Queue> outputQ = ipv4L3Protocol->GetOutputQueue(gatewayDev);

      NS_ASSERT_MSG (inputQ != 0, "No input Queue on gateway");
      NS_ASSERT_MSG (outputQ != 0, "No output Queue on gateway");

      outputQ->TraceConnectWithoutContext ("Enqueue",
                                           MakeCallback (&SimulationStatistics::EnqueueCallback, &simulationStat));
      outputQ->TraceConnectWithoutContext ("Dequeue",
                                           MakeCallback (&SimulationStatistics::DequeueCallback, &simulationStat));
      outputQ->TraceConnectWithoutContext ("Drop",
                                           MakeCallback (&SimulationStatistics::DropCallback, &simulationStat));
    }

  NS_LOG_INFO ("Creating server application");
  //
  // Create a packet sink on remote to receive packets.
  //
  uint16_t sinkPort = 50000;
  PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory",
                                     InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer hubApp = packetSinkHelper.Install (remoteHost);
  hubApp.Start (Seconds (0.0));
  hubApp.Stop (Seconds (simulationConf.stopTime));

  NS_LOG_INFO ("Creating clients application and sockets");

  if (simulationConf.verbose)
    NS_LOG_UNCOND ("Client sections:");

  FOR_EACH_CLIENT (it, clients)
    {
      Ipv4Address remoteAddress = simulationConf.clientConfig.at (remoteHost)->address;
      AddressValue remoteAddressValue = AddressValue (InetSocketAddress (remoteAddress, sinkPort));

      Ptr<Node> client = (*it);
      ClientConfiguration* c = simulationConf.clientConfig.at (client);

      //
      // Create the application which sends packet on the client nodes
      //
      Ptr<BulkSendApplication> app = CreateObject<BulkSendApplication> ();
      app->SetAttribute ("Remote", remoteAddressValue);
      app->SetAttribute ("MaxBytes", UintegerValue (simulationConf.bytesToTransmit));
      app->SetAttribute ("SendSize", UintegerValue (simulationConf.segmentSize));
      app->SetDataTransferredCallback (MakeCallback (&SimulationStatistics::DataTransferredCallback,
                                                     &simulationStat));

      app->SetStartTime (Seconds (c->startTime));
      app->SetStopTime (Seconds (c->stopTime));

      Ptr<TcpL4Protocol> proto = client->GetObject<TcpL4Protocol> ();
      proto->SetAttribute("SocketType", TypeIdValue(TypeId::LookupByName("ns3::" + c->socketType)));

      Ptr<Socket> socket = proto->CreateSocket ();

      NS_ASSERT (socket != 0);

      if (simulationConf.verbose)
        NS_LOG_UNCOND ("\tClient " << c->name);
      /* Attributes for TcpSocket */
      socket->SetAttribute ("InitialCwnd", UintegerValue (cfg.GetInteger (c->name, "InitialCwnd", simulationConf.initialCwnd)));
      socket->SetAttribute ("InitialSlowStartThreshold", UintegerValue (cfg.GetInteger (c->name, "SlowStartThreshold", simulationConf.ssthres)));
      socket->SetAttribute ("DelAckCount", UintegerValue (cfg.GetInteger (c->name, "DelAckCount", simulationConf.delAckCount)));

      if (simulationConf.verbose)
        NS_LOG_UNCOND ("\t\tSocket: " << c->socketType <<
                       "\n\t\tInitialCwnd: " << cfg.GetInteger (c->name, "InitialCwnd", simulationConf.initialCwnd) <<
                       "\n\t\tDelAckCount: " << cfg.GetInteger (c->name, "DelAckCount", simulationConf.delAckCount) <<
                       "\n\t\tSlowStartThres: " << cfg.GetInteger (c->name, "SlowStartThreshold", simulationConf.ssthres) <<
                       "\n\t\tLANBandwidth: " << cfg.Get (c->name, "DataRate", c->lanDataRate) <<
                       "\n\t\tLANDelay: " << cfg.Get (c->name, "OverheadDelay", c->lanDelay));

      /* Attributes for each type of socket */
      if (c->socketType == "TcpNoordwijk" || c->socketType == "TcpNoordwijkMw")
        {
          socket->SetAttribute ("TxTime", TimeValue (MilliSeconds (cfg.GetInteger (c->name, "NoordTx", simulationConf.noord_tx))));
          socket->SetAttribute ("B", TimeValue (MilliSeconds (cfg.GetInteger (c->name, "NoordB", simulationConf.noord_tx))));
          if (simulationConf.verbose)
            NS_LOG_UNCOND ("\t\tTxTime: " << cfg.GetInteger (c->name, "NoordTx", simulationConf.noord_tx) << "ms" <<
                           "\n\t\tB: " << cfg.GetInteger (c->name, "NoordB", simulationConf.noord_tx) << " ms");
        }

      app->SetSocket (socket);
      client->AddApplication (app);

      if (simulationConf.enableCwndStatistics)
        {
          Ptr<Socket> socket = app->GetSocket ();
          socket->TraceConnect ("CongestionWindow", c->name,
                                MakeCallback (&SimulationStatistics::CwndChangeCallback, &simulationStat));
        }

      if (simulationConf.enableSSThStatistics)
        {
          Ptr<Socket> socket = app->GetSocket ();
          socket->TraceConnect ("SlowStartThreshold", c->name,
                                MakeCallback (&SimulationStatistics::SSThChangeCallback, &simulationStat));
        }

      if (simulationConf.enableRTTStatistics)
        {
          Ptr<Socket> socket = app->GetSocket ();
          socket->TraceConnect ("RTT", c->name,
                                MakeCallback (&SimulationStatistics::RttChangeCallback, &simulationStat));
        }
    }

  /*Config::SetDefault ("ns3::ConfigStore::Filename", StringValue ("output-attributes.txt"));
  Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
  Config::SetDefault ("ns3::ConfigStore::Mode", StringValue ("Save"));
  ConfigStore outputConfig2;
  outputConfig2.ConfigureDefaults ();
  outputConfig2.ConfigureAttributes ();*/

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds (simulationConf.stopTime));
  Simulator::Run ();
  NS_LOG_INFO ("Done.");

  if (simulationConf.enableDataRxStatistics)
    {
      std::ofstream file;
      std::string fn = simulationConf.filePrefix + "-remote-rxdata-max.data";
      Ptr<PacketSink> a = DynamicCast<PacketSink> (remoteHost->GetApplication (0));

      if (FileExists (fn))
        {
          NS_FATAL_ERROR ("File " << fn << " exists");
        }


      file.open (fn.c_str ());
      file << "# Total received byte on the remote host\n";
      file << "# Total Recv Data (bytes)\n";

      file << a->GetTotalRx();

      file.close();
    }

  Simulator::Destroy ();

  return 0;
}
