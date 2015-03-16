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
#ifndef PPDRTCLANUTILS_H
#define PPDRTCLANUTILS_H

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
#include "ns3/propagation-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/c2ml-module.h"
#include "ns3/log.h"
#include "ns3/config-store-module.h"
#include "ns3/stats-module.h"

#include "INIReader.h"

namespace ppdrtc {
using namespace ns3;

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
  * \brief Set a default Config Value (uinteger)
  *
  * \param name Name of the Value
  * \param attribute Attribute (without UintegerValue())
  */
#define SET_DEFAULT_UINTEGER(name, attribute) \
  Config::SetDefault (name, UintegerValue (attribute))

/**
  * \brief Set a default Config Value (time)
  *
  * \param name Name of the Value
  * \param attribute Attribute (without TimeValue())
  */
#define SET_DEFAULT_TIME(name, attribute) \
  Config::SetDefault (name, TimeValue (attribute))

/**
  * \brief Set a default Config Value (boolean)
  *
  * \param name Name of the Value
  * \param attribute Attribute (without BooleanValue())
  */
#define SET_DEFAULT_BOOL(name, attribute) \
  Config::SetDefault (name, BooleanValue (attribute))

/**
  * \brief Set a default Config Value (Double)
  *
  * \param name Name of the Value
  * \param attribute Attribute (without DoubleValue())
  */
#define SET_DEFAULT_DOUBLE(name, attribute) \
  Config::SetDefault (name, DoubleValue (attribute))

/**
  * \brief Declare a Key inside a *Section class
  *
  * \param type Type of the key (string, bool, double, uint32)
  * \param name Name of the key in the INI
  * \param desc Brief description
  * \param obj Pointer to the obj which will store the value inside the class
  * \param def Default value for the obj
  */
#define DECLARE_KEY(type, name, desc, obj, def)    \
  {                                                \
    m_keys[name] = KeyDescription (type, desc, obj); \
    *(obj) = def;                                    \
  }

/**
 * \brief Helper to loop into the client list
 *
 * Example usage (pseudocode):
 \verbatim
 client = new std::list;
 FOR_EACH_NODE (it, client)
 {
   (*it)->DoSomething ();
 }
 \endverbatim
 *
 * \param it Iterator name
 * \param clientList List of client name to iterate
 */
#define FOR_EACH_NODE(it, nodecontainer) \
  for (NodeContainer::Iterator it = nodecontainer.Begin (); it != nodecontainer.End (); ++it)

class KeyDescription
{
public:
  KeyDescription () { }
  KeyDescription (const std::string &type, const std::string &desc, void *obj)
  {
    m_type=type;
    m_desc=desc;
    m_obj=obj;
  }

  std::string m_type;
  std::string m_desc;
  void       *m_obj;
};

/**
 * \brief The Section class
 */
class Section
{
public:
  void PrintExample (void);
  virtual std::string GetName  (void) = 0;
  virtual void Fill  (INIReader& ini);
  void ConfigureFromFile (const std::string& filename);
  virtual void  Print (void);

  class XYZ {
public:
    double x;
    double y;
    double z;

    std::string ToString ()
    {
      std::ostringstream s;
      s << "[" << x << "," << y << "," << z << "]";

      return s.str ();
    }
  };

  static Section::XYZ ToXYZ (const std::string &from);

protected:
  typedef std::map<std::string, KeyDescription> KeyMap;
  KeyMap m_keys;
};

class GeneralSection : public Section
{
public:
  GeneralSection ();

  std::string GetName (void) { return "general"; }

  bool EnableC2ML;
  bool Verbose;

  double StopTime;
  uint32_t RemoteN;
  std::string Prefix;
};

class LanSection : public Section
{
public:
  LanSection ();
  std::string GetName (void)            { return "lan"; }
  void        Fill    (INIReader& ini);

  std::string Type;
  std::string Attributes;
  uint32_t    ClientN;
  uint32_t    GatewayN;
  std::string NPerG;
  bool EnablePcapUserNetwork;
};

class BackhaulSection : public Section
{
public:
  BackhaulSection ();
  std::string GetName (void) { return "backhaul"; }

  std::string Type;
  std::string P2PDelay;    //!< delay of lan (if applicable)
  std::string P2PDataRate; //!< rate of lan (if applicable)
  std::string P2PQueueType;
  bool EnablePcap;
};

class NodeSection : public Section
{
public:
  NodeSection (const std::string &name);
  virtual ~NodeSection () { }

  std::string GetName (void) { return m_name; }
  std::string MobilityModel;
  std::string Position;
  double TxPower;

  Ipv4Address Address;     //!< Placeholder for node address
protected:
  std::string m_name;
};

class AppSection : public Section
{
public:
  AppSection (const std::string &name);

  virtual std::string GetName (void) { return m_name; }
  virtual ~AppSection () { }

  std::string InstalledOn;
  std::string ConnectedTo;
  std::string AppType;
  std::string Protocol;
  uint32_t Port;

  double StartTime;        //!< Node start time in simulation
  double StopTime;         //!< Node stop time in simulation

  uint32_t InitialCwnd;
  uint32_t InitialSSTh;
  uint32_t DelAckCount;

  std::string SocketType;

  double TxTime;
  double BNoordwijk;

protected:
  std::string m_name;
};

class BulkSendSection : public AppSection
{
public:
  BulkSendSection (const std::string &name);
  virtual ~BulkSendSection () { }

  uint32_t MaxBytes;
  uint32_t SendSize;
};

class OnOffSection : public BulkSendSection
{
public:
  OnOffSection (const std::string &name);
  virtual ~OnOffSection () { }

  std::string OnTime;
  std::string OffTime;
  std::string DataRate;
};

class GatewaySection : public NodeSection
{
public:
  GatewaySection (const std::string &name);

  Ipv4Address ExternalAddress;


  uint32_t EnbSrsPeriodicity;
  double EnbAntennaOrientation;
  std::string P2PDelay;    //!< delay of lan (if applicable)
  std::string P2PDataRate; //!< rate of lan (if applicable)
  std::string P2PQueueType;
};

class ClientSection : public NodeSection
{
public:
  ClientSection (const std::string &name);

  std::string LanDelay;    //!< delay of lan (if applicable)
  std::string LanDataRate; //!< rate of lan (if applicable)
  std::string QueueType;
  std::string CsiVariation; //!< String (to parse) which contains csi variation
};

class RemoteSection : public NodeSection
{
public:
  RemoteSection (const std::string &name);
};

class Attributes : public Section
{
public:
  Attributes (const std::string &sectionName);
  std::string GetName (void)            { return name; }
  virtual void        Fill    (INIReader& ini);

  std::string Attribute;
  std::string name;
};

class StatisticsSection : public Section
{
public:
  StatisticsSection ();
  std::string GetName (void) { return "statistics"; }

  bool EnableFlowmon;
  bool EnableQueue;
  bool EnableDelay;
  bool EnableJitter;
  bool EnableThroughput;
  bool EnableNetAnim;

  double ThroughputSamplingTime;
};

class LteSection : public Attributes
{
public:
  LteSection ();

  std::string EnbAntennaType;
  double EnbAntennaMaxGain;
  std::string UeAntennaModel;
  uint32_t UlBandwidth;
  uint32_t DlBandwidth;
  uint32_t DlEarfcn;
  uint32_t UlEarfcn;
  std::string PathlossType;
  std::string EnvironmentValue;
  std::string CitySizeValue;
  std::string SchedulerType;

  bool EnablePcapX2Link;
  bool EnablePcapS1uLink;

  bool EnableREM;
  double XMinREM;
  double XMaxREM;
  double YMinREM;
  double YMaxREM;
  double HeightREM;

  EnvironmentType GetEnvironment ();
  CitySize        GetCitySize ();
};

typedef std::map<Ptr<Node>,  NodeSection* > NodeToConfigurationMap;
typedef std::pair<Ptr<Node>, NodeSection* > NodeConfigurationPair;
typedef std::map<Ptr<Node>,  NodeSection* >::iterator NodeConfigurationIterator;

typedef std::pair<Time, std::string> CsiEvent;
typedef std::list<CsiEvent> CsiEventList;
typedef std::map<Ptr<Node>, CsiEventList> ClientToCsiEventMap;

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

std::vector<std::string>& Split (const std::string &s, char delim,
                                 std::vector<std::string> &elems);
void DecodifyLanNPerG (const std::string &NPerG,
                       std::map <std::string, std::list<std::string> > &nodesIdToGwId);
void PrintGnuplottableEnbListToFile (std::string filename);
void PrintGnuplottableUeListToFile (std::string filename);

} // namespace ns3
#endif // PPDRTCLANUTILS_H
