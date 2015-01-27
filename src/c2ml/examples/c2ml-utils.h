#ifndef C2MLUTILS_H
#define C2MLUTILS_H

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/c2ml-module.h"
#include "ns3/log.h"
#include "ns3/config-store-module.h"

#include "INIReader.h"

namespace ns3 {

/**
 * \brief Client configuration parameters
 *
 * See INI file for a field description
 */
class ClientConfiguration
{
public:
  std::string name;        //!< Name of the node
  std::string socketType;  //!< Tcp to employ for the node
  std::string lanDelay;    //!< delay of lan p2p link
  std::string lanDataRate; //!< rate of lan p2p link
  double startTime;        //!< Node start time in simulation
  double stopTime;         //!< Node stop time in simulation
  Ipv4Address address;     //!< Placeholder for node address
  std::string csiVariation;//!< String (to parse) which contains csi variation
};

typedef std::map<Ptr<Node>, ClientConfiguration* > ClientToConfigurationMap;

/**
 * \brief Simulation Configuration parameters
 *
 * See INI file for a field description
 */
class SimulationConfiguration
{
public:

  SimulationConfiguration ();

  // general
  bool enableFlowMonitor;
  bool enablePcap;
  bool enableC2ML;
  bool enableQueueStatistics;
  bool enableCwndStatistics;
  bool enableSSThStatistics;
  bool enableDataRxStatistics;
  bool enableRTTStatistics;
  bool verbose;

  // simulation
  double stopTime;
  std::string filePrefix;

  // satellite
  std::string satStringBw;
  uint32_t satByteBw;
  std::string satDelay;

  // network
  uint32_t bytesToTransmit;
  uint32_t segmentSize;
  uint32_t queueSize;
  uint32_t nodes;

  // gateway
  uint32_t allocationProtocol;
  std::string inputQueueTid;
  std::string outputQueueTid;
  std::string queueAttributes;

  // socket default
  uint32_t delAckCount;
  uint32_t ssthres;
  uint32_t initialCwnd;
  uint32_t noord_tx;
  uint32_t noord_b;

  // Node configuration pointer
  ClientToConfigurationMap clientConfig;
};

typedef std::map<std::string, std::ofstream* > StringToFileMap;
typedef std::pair<Ptr<Node>, ClientConfiguration* > ClientConfigurationPair;

typedef std::map<FlowId, FlowMonitor::FlowStats>::const_iterator FlowConstIterator;
typedef NodeContainer::Iterator ClientIterator;

typedef std::pair<Time, std::string> CsiEvent;
typedef std::list<CsiEvent> CsiEventList;
typedef std::map<Ptr<Node>, CsiEventList> ClientToCsiEventMap;

typedef std::map <Ptr<Node>, bool> ClientToDataTransferredMap;

/**
 * \brief Statistics about the simulation
 *
 * The class is capable to do and to track some simple statistics.
 * Each callback write into an output file (cwnd callback writes into
 * a specified output file for each node) what is happening.
 *
 *
 */
class SimulationStatistics
{
public:

  SimulationStatistics (SimulationConfiguration &conf);
  ~SimulationStatistics ();

  EventId *rttFn;

  void DequeueCallback (Ptr<const Packet> p);
  void EnqueueCallback (Ptr<const Packet> p);
  void DropCallback    (Ptr<const Packet> p);

  void CwndChangeCallback (std::string nodeId, uint32_t oldCwnd,
                           uint32_t newCwnd);
  void SSThChangeCallback (std::string nodeId, uint32_t oldSSTh,
                           uint32_t newSSTh);
  void RttChangeCallback (std::string nodeId, Time oldRtt, Time newRtt);

  // Data over Application Layer
  /**
   * \brief Invoked by a client when it finishes the transmission
   */
  void DataTransferredCallback (Ptr<Node> node);

  /**
   * \brief Callback to trace Rx event of PacketSink
   *
   */
  void DataReceivedCallback (Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t id);

protected:
  void
  SingleDataReceivedCallback (Ptr<const Packet> p, Ptr<Ipv4> ipv4, uint32_t id);

  SimulationConfiguration m_conf;
  std::ofstream m_queueStatisticsOutput;
  std::ofstream m_receivedDataOutput;
  std::ofstream m_dropPacketOutput;

  StringToFileMap m_cWndFileOutput;
  StringToFileMap m_ssThFileOutput;
  StringToFileMap m_rttFileOutput;
  StringToFileMap m_rxDataFileOutput;

  EventId m_queueStatsEvent;

  uint32_t m_queueSize;
  uint64_t m_queueByte;
  uint32_t m_queueMax;

  ClientToDataTransferredMap m_clientToDataTransferredMap;

  Time m_endTransfer;

};



/**
  * \brief Helper to read a Boolean from config file
  *
  * \param ini Name of INIReader object
  * \param section Name of section to read from (string)
  * \param name Value to read (string)
  * \param obj Where store value
  */
#define INI_READ_BOOL(ini, section, name, obj) \
  obj = ini.GetBoolean (section, name, obj);

/**
  * \brief Helper to read a Real from config file
  *
  * \param ini Name of INIReader object
  * \param section Name of section to read from (string)
  * \param name Value to read (string)
  * \param obj Where store value
  */
#define INI_READ_REAL(ini, section, name, obj) \
  obj = ini.GetReal (section, name, obj);

/**
  * \brief Helper to read a uint32_t from config file
  *
  * \param ini Name of INIReader object
  * \param section Name of section to read from (string)
  * \param name Value to read (string)
  * \param obj Where store value
  */
#define INI_READ_UINT32(ini, section, name, obj) \
  obj = (uint32_t) ini.GetInteger (section, name, obj);

/**
  * \brief Helper to read a String from config file
  *
  * \param ini Name of INIReader object
  * \param section Name of section to read from (string)
  * \param name Value to read (string)
  * \param obj Where store value
  */
#define INI_READ_STRING(ini, section, name, obj) \
  obj = ini.Get (section, name, obj);

/**
 * \brief Helper to loop into the client list
 *
 * Example usage (pseudocode):
 \verbatim
 client = new std::list;
 FOR_EACH_CLIENT (it, client)
 {
   (*it)->DoSomething ();
 }
 \endverbatim
 *
 * \param it Iterator name
 * \param clientList List of client name to iterate
 */
#define FOR_EACH_CLIENT(it, clientList) \
  for (ClientIterator it = clientList.Begin (); it != clientList.End (); ++it)

/**
  * \brief Set a default Config Value (uinteger)
  *
  * \param name Name of the Value
  * \param attribute Attribute (without UintegerValue())
  */
#define SET_DEFAULT_UINTEGER(name, attribute) \
  Config::SetDefault (name, UintegerValue(attribute))

/**
  * \brief Set a default Config Value (time)
  *
  * \param name Name of the Value
  * \param attribute Attribute (without TimeValue())
  */
#define SET_DEFAULT_TIME(name, attribute) \
  Config::SetDefault (name, TimeValue(attribute))

#define SET_DEFAULT_BOOL(name, attribute) \
  Config::SetDefault (name, BooleanValue(attribute))

#define SET_DEFAULT_DOUBLE(name, attribute) \
  Config::SetDefault (name, DoubleValue(attribute))

/**
 * \brief Check if file exists
 *
 * \param name File name
 */
inline bool
FileExists (const std::string& name)
{
  struct stat buffer;
  return (stat (name.c_str (), &buffer) == 0);
}

//void ThroughputMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> flowMon);
void InstallLANLink (PointToPointHelper &p2p, NodeContainer &clients,
                     SimulationConfiguration &conf, SimulationStatistics &stat);
void
InstallSatelliteLink (PointToPointHelper& p2p, SimulationConfiguration &conf,
                      SimulationStatistics &stat);

Ptr<Node> CreateNode (const std::string &inputQueueTid = "ns3::DropTailQueue",
                      const std::string &outputQueueTid = "ns3::DropTailQueue");

} // namespace ns3
#endif // C2MLUTILS_H
