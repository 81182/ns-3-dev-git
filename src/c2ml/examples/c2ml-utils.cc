#include "c2ml-utils.h"

using namespace ns3;

static std::vector<std::string>
&Split (const std::string &s, char delim, std::vector<std::string> &elems)
{
  std::stringstream ss(s);
  std::string item;

  while (std::getline(ss, item, delim))
    {
      if (! item.empty())
        {
          elems.push_back(item);
        }
    }

  return elems;
}

static inline void
closeAndDeleteFileMap (StringToFileMap &map)
{
  for (StringToFileMap::iterator it = map.begin (); it != map.end (); ++it)
    {
      std::ofstream *file = (*it).second;
      file->close ();

      delete file;
    }
}

SimulationConfiguration::SimulationConfiguration ()
{
  // Default value for simulation parameters
  //              ;;;;;
  //            ..;;;;;..
  //             ':::::'
  //               ':`
  //
  //
  // general
  enablePcap = false;
  enableC2ML = false;
  enableQueueStatistics = false;
  enableCwndStatistics = false;
  enableSSThStatistics = false;
  enableDataRxStatistics = false;
  enableRTTStatistics = false;
  verbose = false;

  // simulation
  stopTime = 900.0;
  filePrefix = "marnatarlo";

  // satellite
  satStringBw = "256KBps";
  satByteBw = 256000;
  satDelay = "200ms";

  // network
  bytesToTransmit = 10000000;
  segmentSize = 1000;
  queueSize = 500;
  nodes = 0;

  // gateway
  allocationProtocol = 0;
  inputQueueTid = "ns3::DropTailQueue";
  outputQueueTid = "ns3::DropTailQueue";
  queueAttributes = "";

  // socket default
  delAckCount = 1;
  ssthres = 0xffff;
  initialCwnd = 4;
  noord_tx = 50;
  noord_b  = 200;
}

SimulationStatistics::SimulationStatistics (SimulationConfiguration &conf)
{
  m_conf = conf;

  m_queueSize = 0;
  m_queueByte = 0;
  m_queueMax  = 0;

  std::string fn;

  if (conf.enableQueueStatistics)
    {
      fn = conf.filePrefix + "-gwqueue.data";

      if (FileExists (fn))
        {
          NS_FATAL_ERROR ("File " << fn << " exists");
        }

      m_queueStatisticsOutput.open (fn.c_str ());
      m_queueStatisticsOutput << "# Queue size in the gateway\n";
      m_queueStatisticsOutput << "# Sec\tBytes\tPkts\n";

      fn = conf.filePrefix + "-gwqueue-drop.data";
      if (FileExists (fn))
        {
          NS_FATAL_ERROR ("File " << fn << " exists");
        }
      m_dropPacketOutput.open(fn.c_str());
      m_dropPacketOutput << "# Drop in TxQueue in the gateway\n";
      m_dropPacketOutput << "# Time src dest seq ack\n";
    }
}

SimulationStatistics::~SimulationStatistics ()
{
  if (m_conf.enableQueueStatistics)
    {
      std::ofstream queueMaxOutput;
      std::string fn = m_conf.filePrefix + "-gwqueue-max.data";

      if (FileExists (fn))
        {
          NS_FATAL_ERROR ("File " << fn << " exists");
        }

      queueMaxOutput.open (fn.c_str ());
      queueMaxOutput << "# Max Queue size in the gateway\n";
      queueMaxOutput << "# Nodes\tBytes\tPkts\tTimeTransFinish\n";

      queueMaxOutput << m_conf.nodes << "\t" << m_queueMax*m_conf.segmentSize
                     << "\t"<< m_queueMax << "\t" << m_endTransfer.GetSeconds ()
                     << "\n";
      queueMaxOutput.close ();

      m_queueStatisticsOutput.close ();
      m_dropPacketOutput.close ();
    }

  if (m_conf.enableCwndStatistics)
    {
      closeAndDeleteFileMap (m_cWndFileOutput);
    }

  if (m_conf.enableSSThStatistics)
    {
      closeAndDeleteFileMap (m_ssThFileOutput);
    }

  if (m_conf.enableRTTStatistics)
    {
      closeAndDeleteFileMap (m_rttFileOutput);
    }

  if (m_conf.enableDataRxStatistics)
    {
      closeAndDeleteFileMap (m_rxDataFileOutput);
      m_receivedDataOutput.close ();
    }

}

void
SimulationStatistics::DequeueCallback (Ptr<const Packet> p)
{
  (void) p;

  --m_queueSize;
  m_queueByte -= p->GetSize ();

  m_queueStatisticsOutput << Simulator::Now().GetSeconds() << "\t" <<
                             m_queueByte << "\t" << m_queueSize << "\n";
}

void
SimulationStatistics::EnqueueCallback (Ptr<const Packet> p)
{
  (void) p;

  ++m_queueSize;
  m_queueByte += p->GetSize ();

  if (m_queueSize > m_queueMax)
    {
      m_queueMax = m_queueSize;
    }

  m_queueStatisticsOutput << Simulator::Now().GetSeconds () << "\t" <<
                             m_queueByte << "\t" << m_queueSize << "\n";
}

void
SimulationStatistics::DropCallback (Ptr<const Packet> p)
{
  Ptr<Packet> pktCopy = p->Copy();

  Ipv4Header header;

  pktCopy->RemoveHeader(header);
  if (header.GetPayloadSize () < pktCopy->GetSize ())
    {
      pktCopy->RemoveAtEnd (pktCopy->GetSize () - header.GetPayloadSize ());
    }

  TcpHeader tcpHeader;
  pktCopy->RemoveHeader(tcpHeader);

  m_dropPacketOutput << Simulator::Now().GetSeconds() << "\t" << header.GetSource() <<
                        "\t" << header.GetDestination() << "\t" << tcpHeader.GetSequenceNumber() << "\t"
                     << tcpHeader.GetAckNumber() << "\n";
}

void
SimulationStatistics::DataTransferredCallback (Ptr<Node> node)
{
  ClientToDataTransferredMap::iterator it = m_clientToDataTransferredMap.find (node);

  if (m_conf.verbose)
    {
      NS_LOG_UNCOND ("The client " << node->GetId() << " has finished transfer");
    }

  if (it == m_clientToDataTransferredMap.end())
    {
      m_clientToDataTransferredMap.insert(std::pair<Ptr<Node>, bool> (node, true));
    }
  else
    {
      std::pair<Ptr<Node>, bool> pair = (*it);
      pair.second = true;
    }

  if (m_clientToDataTransferredMap.size() == m_conf.nodes)
    {
      Simulator::Cancel (m_queueStatsEvent);
      m_endTransfer = Simulator::Now ();

      if (m_conf.verbose)
        {
          NS_LOG_UNCOND ("At time" << m_endTransfer.GetSeconds() << " s, all nodes concludes");
        }
    }
}

void
SimulationStatistics::SingleDataReceivedCallback (Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t id)
{
  static uint64_t rxData[128];
  StringToFileMap::iterator it;
  std::ofstream *file;
  std::vector<std::string> ip_elem;

  Ipv4Header ipHeader;
  Ipv4Address addr;
  p->PeekHeader (ipHeader);
  addr = ipHeader.GetSource();


  std::stringstream ss;

  addr.Print(ss);

  Split (ss.str(), '.', ip_elem);

  it = m_rxDataFileOutput.find (ip_elem.at(2));

  if (it == m_rxDataFileOutput.end ())
    {
      std::string fn = m_conf.filePrefix + "-remote-rxdata-from-" + ip_elem.at(2) + ".data";

      if (FileExists (fn))
        {
          NS_FATAL_ERROR ("File " << fn << " exists");
        }

      file = new std::ofstream;

      file->open (fn.c_str ());

      (*file) << "# Received byte on the remote host from " + ss.str() + "\n";
      (*file) << "# Time\t rxData (byte)\n";

      m_rxDataFileOutput.insert(std::pair<std::string, std::ofstream*> (ip_elem.at(2), file));
    }
  else
    {
      file = (*it).second;
    }

  rxData[atoi(ip_elem.at(2).c_str())] += p->GetSize();

  (*file) << Simulator::Now ().GetSeconds () << "\t" << rxData[atoi(ip_elem.at(2).c_str())] << "\n";
}

void
SimulationStatistics::CwndChangeCallback (std::string nodeId, uint32_t oldCwnd,
                                          uint32_t newCwnd)
{
  StringToFileMap::iterator it;
  std::ofstream *file;

  it = m_cWndFileOutput.find (nodeId);

  if (it == m_cWndFileOutput.end ())
    {
      std::string fn = m_conf.filePrefix + "-cwnd-" + nodeId + ".data";

      if (FileExists (fn))
        {
          NS_FATAL_ERROR ("File " << fn << " exists");
        }

      file = new std::ofstream;

      file->open (fn.c_str ());

      (*file) << "# cWnd of the client " << nodeId << "\n";
      (*file) << "# Time\tcWnd (byte)\n";

      m_cWndFileOutput.insert (std::pair<std::string, std::ofstream*> (nodeId,file));
    }
  else
    {
      file = (*it).second;
    }

  (*file) << Simulator::Now ().GetSeconds () << "\t" << newCwnd << "\n";
}

void
SimulationStatistics::SSThChangeCallback(std::string nodeId, uint32_t oldSSTh,
                                         uint32_t newSSTh)
{
  StringToFileMap::iterator it;
  std::ofstream *file;

  it = m_ssThFileOutput.find (nodeId);

  if (it == m_ssThFileOutput.end ())
    {
      std::string fn = m_conf.filePrefix + "-ssth-" + nodeId + ".data";

      if (FileExists (fn))
        {
          NS_FATAL_ERROR ("File " << fn << " exists");
        }

      file = new std::ofstream;

      file->open (fn.c_str ());

      (*file) << "# SSTh of the client " << nodeId << "\n";
      (*file) << "# Time\tSSTh (byte)\n";

      m_ssThFileOutput.insert (std::pair<std::string, std::ofstream*> (nodeId,file));
    }
  else
    {
      file = (*it).second;
    }

  (*file) << Simulator::Now ().GetSeconds () << "\t" << newSSTh << "\n";
}


void
SimulationStatistics::RttChangeCallback (std::string nodeId, Time oldRtt, Time newRtt)
{

  StringToFileMap::iterator it;
  std::ofstream* file;

  it = m_rttFileOutput.find (nodeId);

  if (it == m_rttFileOutput.end ())
    {
      std::string fn = m_conf.filePrefix + "-rtt-" + nodeId + ".data";

      if (FileExists (fn))
        {
          NS_FATAL_ERROR ("File " << fn << " exists");
        }

      file = new std::ofstream;
      file->open (fn.c_str ());

      (*file) << "# Rtt of the client " << nodeId << "\n";
      (*file) << "# Time\tRtt (msec)\n";

      m_rttFileOutput.insert (std::pair<std::string, std::ofstream*> (nodeId,file));
    }
  else
    {
      file = (*it).second;
    }

  (*file) << Simulator::Now ().GetSeconds () << "\t" << newRtt.GetMilliSeconds () << "\n";
}

/**
 * \brief Create a node installing the classic InternetStack
 */
Ptr<Node>
ns3::CreateNode (const std::string &inputQueueTid, const std::string &outputQueueTid)
{
  Ptr<Node> node = CreateObject<Node> ();

  InternetStackHelper internet;
  internet.SetInputQueueTid (inputQueueTid);
  internet.SetOutputQueueTid (outputQueueTid);
  internet.Install (node);

  return node;
}

static void
RateChangeCallback (Ptr<NetDevice> clientDev, Ptr<NetDevice> gatewayDev,
                    std::string dataRate, bool withC2ML)
{
  Ptr<PointToPointNetDevice> p2pHost, p2pGateway;

  p2pHost    = DynamicCast<PointToPointNetDevice>(clientDev);
  p2pGateway = DynamicCast<PointToPointNetDevice>(gatewayDev);

  p2pHost->SetDataRate (DataRate (dataRate));
  p2pGateway->SetDataRate (DataRate (dataRate));

  if (withC2ML)
    {
      Ptr<Node> client = p2pHost->GetNode();
      Ptr<TcpL4Protocol> protoHost = client->GetObject<TcpL4Protocol> ();
      Ptr<CCL45Protocol> Mw = DynamicCast<CCL45Protocol> (protoHost);

      if (Mw != 0)
        {
          Mw->CsiBwChange(DataRate(dataRate).GetBitRate()/8);
        }
      else
        {
          std::cerr << "Event datarate=" << dataRate << " for node " <<
                       clientDev->GetNode()->GetId() << " not applied";
        }
    }
}

void
SimulationStatistics::DataReceivedCallback (Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t id)
{
  static uint64_t totalRx = 0;
  if (! m_receivedDataOutput.is_open ())
    {
      std::string fn = m_conf.filePrefix + "-remote-rxdata.data";

      if (FileExists (fn))
        {
          NS_FATAL_ERROR ("File " << fn << " exists");
        }

      m_receivedDataOutput.open (fn.c_str ());
      m_receivedDataOutput << "# Received byte on the remote host\n";
      m_receivedDataOutput << "# Time\tSum of Recv Data (bytes)\n";
    }

  totalRx += p->GetSize();

  m_receivedDataOutput << Simulator::Now ().GetSeconds () << "\t" << totalRx << "\n";

  SingleDataReceivedCallback (p, ipv4, id);
}

static CsiEventList
DecodeCsi (std::string csiVariation)
{
  CsiEventList list;

  std::vector<std::string> events;
  Split (csiVariation, '|', events);

  for (std::vector<std::string>::iterator it = events.begin(); it != events.end(); ++it)
    {
      std::string event = (*it);

      std::vector<std::string> decoded;
      Split (event, ';', decoded);

      std::string time = decoded.at(0);
      std::string datarate = decoded.at(1);
      datarate.erase (datarate.size () - 1);
      time.erase(0,1);

      list.insert(list.end(), CsiEvent (Time (time), datarate) );
    }

  return list;
}


void
ns3::InstallLANLink (PointToPointHelper &p2p, NodeContainer &clients,
                SimulationConfiguration &conf, SimulationStatistics &stat)
{
  Ipv4AddressHelper ipv4;

  Ptr<Node> gatewayHost = Names::Find<Node> ("/Names/gateway");
  std::string gatewayName = Names::FindName (gatewayHost);

  NS_ASSERT (gatewayHost != 0);

  FOR_EACH_CLIENT (it, clients)
    {
      Ptr<Node> client = (*it);
      ClientConfiguration *c = conf.clientConfig.at (client);

      p2p.SetDeviceAttribute  ("DataRate", StringValue (c->lanDataRate));
      p2p.SetChannelAttribute ("Delay",    StringValue (c->lanDelay));

      NetDeviceContainer devs = p2p.Install (client, gatewayHost);

      Names::Add (Names::FindName (client) + "/eth0", devs.Get (0));
      Names::Add (gatewayName + "/eth"+c->name, devs.Get (1));

      std::string baseAddr = "10.1." + c->name + ".0";
      ipv4.SetBase (Ipv4Address (baseAddr.c_str ()), "255.255.255.0");

      Ipv4InterfaceContainer ifaces = ipv4.Assign (devs);
      c->address = ifaces.GetAddress (0);

      if (c->csiVariation != "")
        {
          CsiEventList list = DecodeCsi (c->csiVariation);

          for (CsiEventList::iterator it = list.begin (); it != list.end(); ++it)
            {
              CsiEvent event = (*it);

              Simulator::Schedule (event.first, RateChangeCallback,
                                   devs.Get(0), devs.Get(1), event.second,
                                   conf.enableC2ML);
            }
        }
    }
}

void
ns3::InstallSatelliteLink (PointToPointHelper& p2p, SimulationConfiguration &conf,
                      SimulationStatistics &stat)
{
  Ptr<Node> gatewayHost = Names::Find<Node> ("/Names/gateway");
  Ptr<Node> remoteHost = Names::Find<Node> ("/Names/remote");

  NS_ASSERT (gatewayHost != 0);
  NS_ASSERT (remoteHost != 0);

  NetDeviceContainer devicesGatewayRemote = p2p.Install (gatewayHost, remoteHost);
  Ptr<PointToPointNetDevice> gatewayDev = DynamicCast<PointToPointNetDevice> (devicesGatewayRemote.Get (0));
  Ptr<PointToPointNetDevice> remoteDev = DynamicCast<PointToPointNetDevice> (devicesGatewayRemote.Get (1));

  //Ptr<RateErrorModel> em = CreateObject<RateErrorModel> ();
  //em->SetAttribute ("ErrorRate", DoubleValue (0.0000005));
  //devicesGatewayRemote.Get (1)->SetAttribute ("ReceiveErrorModel", PointerValue (em));

  if (conf.enableDataRxStatistics)
    {
      Ptr<Ipv4L3Protocol> ipv4L3Protocol = remoteHost->GetObject<Ipv4L3Protocol> ();
      ipv4L3Protocol->TraceConnectWithoutContext("Rx",
                              MakeCallback (&SimulationStatistics::DataReceivedCallback, &stat));
    }

  Names::Add (Names::FindName (remoteHost) + "/sat0", devicesGatewayRemote.Get (1));
  Names::Add (Names::FindName (gatewayHost) +"/sat0", devicesGatewayRemote.Get (0));

  // gatewayToRemote.Get (1)->SetAttribute ("DataRate", StringValue ("1280KBps"));

  if (conf.enablePcap)
    {
      //p2pLan.EnablePcap ("bmw-lan", clients);
      //p2pSat.EnablePcap (filePrefix, NodeContainer(gatewayHost));
      p2p.EnablePcap (conf.filePrefix, devicesGatewayRemote.Get (0));
      //p2pSat.EnablePcap ("bmw-sat", NodeContainer(remoteHost));
    }

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.254.0", "255.255.255.0");

  Ipv4InterfaceContainer interfacesGatewayRemote = ipv4.Assign (devicesGatewayRemote);

  conf.clientConfig.at (gatewayHost)->address = interfacesGatewayRemote.GetAddress (0);
  conf.clientConfig.at (remoteHost)->address = interfacesGatewayRemote.GetAddress (1);
}
