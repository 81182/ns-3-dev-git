/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 University of Modena and Reggio Emilia (UNIMORE)
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
 * Author: Martin Klapez <martin.klapez@unimore.it>
 */

/*
 *      Transfer from UE to remoteHost              :)
 *      Transfer from remoteHost to UE              :)
 *      Transfer from UEx to UEy                    :(
 *      Transfer from UE to remoteHost via MPTCP    :(
 *      Transfer from remoteHost to UE via MPTCP    :(
 *      Transfer from UE to remoteHost with LTE+SAT :(
 *      Transfer from remoteHost to UE with LTE+SAT :(
 *
 *
 *
 *      Current network topology
 *                                                        po2po
 *      UE ---------- eNB ---------- SGW/PGW ---------- (Internet) ---------- remotehost
 *
 *
 *
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/tcp-cubic.h"
//#include "ns3/gtk-config-store.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("En4PPDRLan");

enum {
    TCP_NEWRENO = 0,
    TCP_CUBIC = 1,
    TCP_HIGHSPEED = 2,
    TCP_NOORDWIJK = 3
};


int main (int argc, char *argv[])
{	
  CommandLine cmd;
  std::string dr = "5Mbps";
  std::string delay = "300ms";
  std::string prefix = "en4ppdr";
  uint32_t segmentSize = 1000;
  uint32_t delAckCount = 2;
  uint32_t ssthres = 20000;
  uint32_t initialCwnd = 4;
  uint32_t lteNodes = 2;
  uint32_t lteVoiceNodes = 16;
  double stopTime = 63.0;

  bool enableVoice = true;

  int TcpType = TCP_NOORDWIJK;

  cmd.AddValue ("DataRate", "Data rate da usare sul p2p", dr);
  cmd.AddValue ("TCPType", "Type of the underlying TCP (0=NewReno, 1=Cubic, 2=HighSpeed, 3=Noordwijk", TcpType);
  cmd.AddValue ("SegmentSize", "Segment size della rete", segmentSize);
  cmd.AddValue ("Delay", "One-way sat delay", delay);
  cmd.AddValue ("Voice", "Enable or disable voice background", enableVoice);
  cmd.AddValue ("LTENodes", "LTE Nodes which does TCP", lteNodes);
  cmd.AddValue ("Prefix", "Prefix for output file", prefix);
  cmd.Parse(argc, argv);


  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (segmentSize));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (50000000));

  Config::SetDefault ("ns3::BulkSendApplication::SendSize", UintegerValue (segmentSize));
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue (50000000));

  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (delAckCount));
  Config::SetDefault ("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue (ssthres));
  Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (initialCwnd));

  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue(25));
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue(25));

  TypeId tid;
  switch (TcpType)
    {
      case TCP_CUBIC:
        tid = TypeId::LookupByName ("ns3::TcpCubic");
        NS_LOG_UNCOND ("Using TcpCubic");
        break;
      case TCP_NOORDWIJK:
        tid = TypeId::LookupByName ("ns3::TcpNoordwijk");
        Config::SetDefault ("ns3::TcpNoordwijk::TxTime", TimeValue (MilliSeconds(500)));
        Config::SetDefault ("ns3::TcpSocket::InitialCwnd", UintegerValue (4));
        Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (1));

        Config::SetDefault ("ns3::TcpNoordwijk::B", TimeValue (MilliSeconds (300)));
        NS_LOG_UNCOND ("Using TcpNoordwijk");
        break;
      case TCP_HIGHSPEED:
        tid = TypeId::LookupByName ("ns3::TcpHighSpeed");
        NS_LOG_UNCOND ("Using TcpHighSpeed");
        break;
      case TCP_NEWRENO:
        tid = TypeId::LookupByName ("ns3::TcpNewReno");
        NS_LOG_UNCOND ("Using TcpNewReno");
        break;
      default:
        NS_FATAL_ERROR ("NessunTCP Not Working today");
    }

  Config::SetDefault ("ns3::TcpL4Protocol::SocketType", TypeIdValue (tid));

  /* Creazione Helper LTE Model ed EPC Model */
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper> epcHelper = CreateObject<PointToPointEpcHelper> ();

  /* In questo modo l'helper LTE farà partire le appropriate configurazioni EPC quando certe cose come i nodi o i radio bearer vengono creati */
  lteHelper->SetEpcHelper (epcHelper);

  /* Il nodo pgw è già stato creato da EpcHelper, qui vado a prendermi un riferimento */
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  /* Crea due container, uno per gli eNB e uno per gli UE */
  NodeContainer enbNodes;
  NodeContainer ueNodes;
  NodeContainer voiceNodesContainer;
  /* Per ora mi bastano due nodi, un eNB ed un UE */
  enbNodes.Create (1);
  ueNodes.Create (lteNodes);
  if (enableVoice)
    {
      voiceNodesContainer.Create (lteVoiceNodes);
    }

  /* Uso l'Helper di Mobility per piazzare facilmente i due nodi in certe posizioni fisse */
  MobilityHelper mobility;
  //Vector3D uePosition (0.0,0.0,0.0);
  //Vector3D enbPosition (10.0,10.0,0.0);

  //Config::Set ("/NodeList/*/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Bounds",
  //             RectangleValue (Rectangle(0.0,1000.0,0.0,1000.0)));

  /*Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x->SetAttribute ("Min", DoubleValue (1.0));
  x->SetAttribute ("Max", DoubleValue (4.0));*/
  //Config::Set ("/NodeList/*/$ns3::MobilityModel/$ns3::RandomWalk2dMobilityModel/Speed",
  //             PointerValue (x));

  /* Decido il Mobility Model e lo installo sui nodi */
  //mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel");
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  //mobility.SetPositionAllocator("ns3::ConstantPositionMobilityModel", "Position", enbPosition);
  mobility.Install (enbNodes);
  //BuildingsHelper::Install (enbNodes);
  //mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);
  if (enableVoice)
    {
      mobility.Install (voiceNodesContainer);
    }
  //BuildingsHelper::Install (ueNodes);

  /* Una volta installato sui nodi il Mobility Model a posizione costante vado a prendermi il riferimento del modello di ciascun nodo */
  //Ptr<MobilityModel> enbMobModel = enbNodes.Get(0)->GetObject<MobilityModel> ();
  //Ptr<MobilityModel> ueMobModel = ueNodes.Get(0)->GetObject<MobilityModel> ();

  /* Setto la posizione di ciascun nodo attraverso il metodo del Mobility Model */
  //enbMobModel->SetPosition(enbPosition);
  //ueMobModel->SetPosition(uePosition);

  /* Mi stampo le posizioni dei nodi (fare primahost da console export 'NS_LOG=En4PPDRLan=level_debug|prefix_func' altrimenti non stampa) */
  //NS_LOG_DEBUG("Posizione ue -> " << ueMobModel->GetPosition());
  //NS_LOG_DEBUG("Posizione enb -> " << enbMobModel->GetPosition() << "\n");

  /* Creo i NetDevice in modo standard (scheduler PF) */
  NetDeviceContainer enbDevices;
  NetDeviceContainer ueDevices;
  NetDeviceContainer voiceDevices;
  // Default scheduler is PF, uncomment to use RR
  //lteHelper->SetSchedulerType ("ns3::RrFfMacScheduler");

  /* Li installo sui nodi */
  enbDevices = lteHelper->InstallEnbDevice (enbNodes);
  ueDevices = lteHelper->InstallUeDevice (ueNodes);
  if (enableVoice)
    {
      voiceDevices = lteHelper->InstallUeDevice (voiceNodesContainer);
    }

  //lteHelper->EnableTraces ();

  /* Creo l'host remoto, lo referenzio e ci installo l'internet stack */
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);

  InternetStackHelper internet;
  internet.Install (remoteHostContainer); // Invece che il container potevo anche dargli il nodo diretto

  /* Creo una semplice rete point to point tra pgw e remotehost */

  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", StringValue (dr));
  p2ph.SetChannelAttribute ("Delay", StringValue (delay));

  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHostContainer.Get(0));

  p2ph.EnablePcapAll (prefix, true);

  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");

  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device

  /* Installo lo stack ipv4 sugli ue */
  internet.Install (ueNodes);

  if (enableVoice)
    {
      internet.Install (voiceNodesContainer);
    }

  Ipv4InterfaceContainer ueIpIface, voiceIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (ueDevices);

  if (enableVoice)
    {
      voiceIpIface = epcHelper->AssignUeIpv4Address (voiceDevices);
    }

  /* Collego l'UE all'eNB, attiva di default uno standard radio bearer eps (vedi sotto) */
  for (uint32_t i=0; i<lteNodes; ++i)
    {
       lteHelper->Attach (ueDevices.Get(i), enbDevices.Get (0));
    }

  if (enableVoice)
    {
      for (uint32_t i=0; i<lteVoiceNodes; ++i)
        {
          lteHelper->Attach (voiceDevices.Get(i), enbDevices.Get (0));
        }
    }

  /* Attivo un radio bearer (canale offerto dal livello 2 ai livelli superiori) per il trasferimento di user data */
  //enum EpsBearer::Qci q = EpsBearer::NGBR_VIDEO_TCP_DEFAULT;
  //EpsBearer bearer (q);
  //lteHelper->ActivateEpsBearer (ueDevices, bearer, EpcTft::Default ());
  //lteHelper->EnableTraces ();
  //lteHelper->ActivateEpsBearer (ueDevices, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());

  /* L'helper epc ha già assegnato all'ue un indirizzo ip nella rete 7.0.0.0 */
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHostContainer.Get(0)->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  //Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  for (uint32_t i=0; i<lteNodes; ++i)
    {
      Ptr<Node> ue = ueNodes.Get (i);
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ue->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

      NS_LOG_DEBUG("ue ip address: " << ueIpIface.GetAddress(i));
    }

  /* App di prova, BulkSender sull'ue per sparare pacchetti il più veloce possibile e un Packet Sink sul remoteHost per ciucciarli */
  uint16_t porta = 1234;
  uint32_t maxBytes = 0;// 10485760;
  //BulkSendHelper sorgente ("ns3::TcpSocketFactory", InetSocketAddress (pgw->GetObject<Ipv4> ()->GetAddress(1,0).GetLocal(), porta));
  BulkSendHelper sorgente ("ns3::TcpSocketFactory", InetSocketAddress (remoteHostContainer.Get(0)->GetObject<Ipv4> ()->GetAddress(1,0).GetLocal(), porta));
  sorgente.SetAttribute ("MaxBytes", UintegerValue (maxBytes));

  for (uint32_t i=0; i<lteNodes; ++i)
    {
      ApplicationContainer sourceApp = sorgente.Install (ueNodes.Get (i));
      sourceApp.Start (Seconds (0.0));
      sourceApp.Stop (Seconds (stopTime));
    }

  PacketSinkHelper succhiotto ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), porta));
  ApplicationContainer sinkApp = succhiotto.Install (remoteHostContainer.Get(0));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (stopTime));

  if (enableVoice)
    {
      uint32_t echoPort = 9;
      for (uint32_t i=0; i<lteVoiceNodes/2; ++i)
        {
          OnOffHelper voice ("ns3::UdpSocketFactory",
                             Address (InetSocketAddress (voiceIpIface.GetAddress (i+(lteVoiceNodes/2)), echoPort)));

          //voice.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (5.0)));
          //voice.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (5.0)));

          voice.SetAttribute ("DataRate", DataRateValue (DataRate("13kbps")));
          voice.SetAttribute ("PacketSize", UintegerValue (100));

          ApplicationContainer send = voice.Install (voiceNodesContainer.Get (i));
          send.Start (Seconds (0.0));
          send.Stop (Seconds (stopTime));
        }

      for (uint32_t i=lteVoiceNodes/2; i<lteVoiceNodes; ++i)
        {
          PacketSinkHelper succhiotto ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny(), echoPort));
          ApplicationContainer sinkApp = succhiotto.Install (voiceNodesContainer.Get (i));

          sinkApp.Start (Seconds (0.0));
          sinkApp.Stop (Seconds (stopTime));
        }
    }

  Config::Set ("/NodeList/*/$ns3::TcpL4Protocol/SocketType", TypeIdValue (tid));

  Simulator::Stop (Seconds (stopTime));
  std::cout << "Inizio simulazione comunicazione" << std::endl;
  Simulator::Run ();

  // GtkConfigStore config;
  // config.ConfigureAttributes ();

  Simulator::Destroy ();
  std::cout << "Fine simulazione comunicazione" << std::endl << "\n";

  Ptr<PacketSink> cisti = DynamicCast<PacketSink> (sinkApp.Get (0));
  std::cout << "Byte Totali Ricevuti da remotehost: " << cisti->GetTotalRx () << std::endl;

  return 0;
}
