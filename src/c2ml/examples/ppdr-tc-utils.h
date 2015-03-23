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
  * \brief Declare a Key inside a Section class
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

/**
 * \brief Base class to represent a key=value in the configuration file
 *
 * In INI files, each parameter is given in a keyed from (key=value). Since
 * this syntax could represent virtually anything, a simple way to map it to
 * a c++-friendly class is a description in term of type, description, and a
 * pointer to the object handling the value. The class aim is exactly this.
 *
 */
class KeyDescription
{
public:
  /**
   * \brief Empty constructor (used by some std functions).
   *
   * Please remember to manually fill the attributes.
   */
  KeyDescription () { }

  /**
   * \brief Default constructor
   *
   * \param type (c++) type of the object
   * \param desc description of the meanings of the value
   * \param obj pointer to the object (check == 0 before use)
   */
  KeyDescription (const std::string &type, const std::string &desc, void *obj)
  {
    m_type=type;
    m_desc=desc;
    m_obj=obj;
  }

  std::string m_type; //!< Type of the object. Typical values: bool, string,
                      //   double, uint32.
  std::string m_desc; //!< Description of the object. Please, be meaningful to users!
  void       *m_obj;  //!< Pointer to the object
};

/**
 * \brief Basic c++ representation of a section in an ini configuration file
 *
 * The class duty is to map a section in an ini file in a c++ representation
 * of its keyes and values. A section is defined, in an ini file, by this form:
 *\verbatim
 [section_name]
 key1=value1
 key2=value2
 ...
 \endverbatim
 *
 * First of all, you need to derive it and create your own section, which
 * reimplement (at least) the Section::GetName() member function,
 * to return "section_name".
 *
 * Then, in such subclass, you should declare the key of the section, with
 * their type, using the macro DECLARE_KEY in the constructor. Please
 * note that accepted types are: bool, string, double, uint32.
 *
 * After that, the member function Section::Fill(INIReader&) can be
 * called, to initialize such values directly from the ini file. You can
 * see various examples of this workflow, e.g. GeneralSection, or
 * start by this ini example, which loads a string value from the key MyProperty:
 *\verbatim
 [example]
 MyProperty=a string value
 \endverbatim
 * Which can be loaded with the following:
 *\code{.cpp}
 class ExampleSection : public Section
 {
 public:
   ExampleSection () : Section ()
   {
     DECLARE_KEY ("string", "MyProperty", "short description of the meanings",
                  &m_myProperty, "Hello World!");
   }

   std::string GetName () { return "example"; }
   std::string GetMyProperty () { return m_myProperty; }

 protected:
   std::string m_myProperty;
 };

 int main ()
 {
   (...)
   ExampleSection section;
   INIReader cfg;
   (...)
   section.Fill (cfg);

   std::cout << "Property is " << section.GetMyProperty () << std::endl;
 }
 \endcode
 *
 * Which gives, as output,
 *\verbatim
 Property is a string value
 \endverbatim
 *
 * For general-purpose section, see Attributes.
 *
 * \see Section::XYZ
 * \see Section::PrintExample
 */
class Section
{
public:
  /** \brief Empty constructor
   */
  Section () { }

  /**
   * \brief Virtual deconstructor
   */
  virtual ~Section () { }

  /**
   * \brief Print an example of the section, taking as value the default
   * ones provided with the DECLARE_KEY macro. With respect to the Example
   * Section defined above, you will have on the standard output the following,
   * after calling this member function:
   *\verbatim
 [example]
 MyProperty=Hello World!
 \endverbatim
   * Code:
   * \code{.cpp}
   (...)
   ExampleSection section;
   section.PrintExample ();
   (...)
   \endcode
   */
  void PrintExample (void);

  /**
   * \brief Print the values read from the ini configuration file.
   *
   * The output on the standard output will be styled as the original ini
   * file. With respect to the example section defined above, the result would
   * be:
   * \code{.cpp}
   (...)
   ExampleSection section;
   INIReader cfg;
   (...)
   section.Fill (cfg);
   section.Print ();
   (...)
   \endcode
   *
   *\verbatim
 [example]
 MyProperty=a string value
 \endverbatim
   */
  virtual void  Print (void);

  /**
   * \brief Get the name of the section
   *
   * \return the name of the section (without "[" and "]")
   */
  virtual std::string GetName  (void) = 0;

  /**
   * \brief Fill the class with values taken from the ini configuration file
   *
   * \param ini configuration file
   */
  virtual void Fill  (INIReader& ini);

  /**
   * \brief Load a configuration (in an ns-3 compatible format) directly from
   * file
   *
   * \param filename file to load
   */
  void ConfigureFromFile (const std::string& filename);

  /**
   * \brief Helper class to represent a vector
   *
   * In the ini file, the vector will be specified as:
   * \verbatim
   [example]
   Vector = [45.0, 10.0, 15.0]
   \endverbatim
   *
   * while in the code it will be referred as an object of type Section::XYZ.
   * You can easily access the x, y, and z values from the object through its
   * member.
   */
  class XYZ {
public:
    double x; //!< X value
    double y; //!< Y value
    double z; //!< Z value

    /**
     * \brief "Stringify" the class
     *
     * return the string representation of the class, which is in the form
     * "[x,y,z]".
     *
     * \return string representation of the class
     */
    std::string ToString ()
    {
      std::ostringstream s;
      s << "[" << x << "," << y << "," << z << "]";

      return s.str ();
    }
  };

  /**
   * \brief Convert a vector string ("[x,y,z]") into a Section::XYZ object
   *
   * \param from input string
   * \return an instance of Section::XYZ class
   */
  static Section::XYZ ToXYZ (const std::string &from);

protected:
  /**
   * \brief Map between string (the key name) and its description, represented
   * by a KeyDescription object.
   */
  typedef std::map<std::string, KeyDescription> KeyMap;

  KeyMap m_keys; //!< Map between key names and its (c++-friendly) description
};

/**
 * \brief The General section
 *
 * General parameters for simulation. Please refer to member description for
 * a detailed illustration of the possible keys and their values.
 */
class GeneralSection : public Section
{
public:
  /**
   * \brief Constructor
   */
  GeneralSection ();

  /**
   * \brief Get the name of the general section
   *
   * ... which is "general".
   *
   * \return without much fantasy, it returns "general"
   */
  std::string GetName (void) { return "general"; }

  bool EnableC2ML;    //!< Enable (true) or disable (false) C2ML. Default false
  bool Verbose;       //!< Enable (true) or disable (false) verbose log messages.
                      //   Default true

  double StopTime;    //!< The overall simulation stop time (in s). It has precedence
                      //!< over the single application stop time. Default 60.0 seconds
  uint32_t RemoteN;   //!< The number of remote nodes. Default 1
  std::string Prefix; //!< The prefix for all output files. Default "ppdr-tc-simulation".
                      //!< To see to what files it applies, please look into Statistics
};

/**
 * \brief Helper section class for loading attributes from file
 *
 * It loads the attribute file pointed by the member Attribute into ns-3
 * through the Config system.
 *
 * It is main used as a parent class to avoid the definition of the same
 * member Attribute over all the children.
 */
class Attributes : public Section
{
public:
  /**
   * \brief Constructor
   *
   * \param sectionName the name of the section
   */
  Attributes (const std::string &sectionName);
  /**
    * \brief Get the name of the section
    * \return the name of the section
    */
  std::string GetName (void)            { return m_name; }

  /**
   * \brief Fill the class and do additional checks
   *
   * It checks the file existence, and if it exists it do the load of
   * such file through the ns-3 Config subsystem.
   *
   * \param ini configuration file
   */
  virtual void        Fill    (INIReader& ini);

  std::string Attribute; //!< Attribute file to be loaded. It must exists.
                         //!< Default empty value

protected:
  std::string m_name;    //!< Name of the class
};

/**
 * \brief The Lan section
 *
 * It defines the LAN part of the network, its type, the number of client
 * and gateway. Currently only "p2p" and "lte" are supported for the LAN type.
 *
 * For LTE lan, please fill the attributes through the use of a LteSection. For
 * P2P, it is mandatory to fill the NPerG attribute. Please refer to member
 * description for a detailed illustration of the possible keys and their values.
 */
class LanSection : public Attributes
{
public:
  /**
   * \brief Constructor
   */
  LanSection ();

  /**
   * \brief Conceptually the same member, with some additional checking
   *
   * Checks about the lan type, and the number of clients and remotes.
   * No checks about the correctness of the NPerG string are made or the
   * soundness of the LAN network are made.
   *
   * \param ini configuration file
   */
  void        Fill    (INIReader& ini);

  std::string Type;       //!< Type of the LAN network. It could be "p2p" or "lte".
                          //!< Default to "p2p"
  uint32_t    ClientN;    //!< The number of the client over the entire LAN.
                          //!< Default 3
  uint32_t    GatewayN;   //!< The number of the gateway over the entire LAN.
                          //!< For a lte network, this is the number of enb. Default 1
  std::string NPerG;      //!< String which represents, in case of a "p2p" lan,
                          //!< the connection between gateway and clients. For example,
                          //!< "[gateway0:client1];[gateway1:client2]" means that
                          //!< gateway0 and client1 are connected, and gateway1 and client2
                          //!< are connected too. The rates of such connections
                          //!< are defined in the Client section.
  bool EnablePcapUserNetwork; //!< Enable (true) or disable (false) the Pcap
                              //!< output of the LAN.
};

/**
 * \brief The Backhaul section
 *
 * This section represents the backhaul part of the network. Currently,
 * it is supported only a "p2p" type of backhaul, with characteristics
 * of bandwidth and delay competely programmable.
 *
 * By default, the p2p mode connects every gateway (or gateway which have
 * connectivity to the internet) to each remote node. Aggregation is not
 * supported, so multiple remotes behind router/firewall should be
 * modeled as multiple applications over the same node.
 *
 */
class BackhaulSection : public Section
{
public:
  /**
   * \brief Constructor
   */
  BackhaulSection ();

  /**
   * \brief Get the name of the backhaul section
   *
   * ... which is "backhaul".
   *
   * \return without much fantasy, it returns "backhaul"
   */
  std::string GetName (void) { return "backhaul"; }

  std::string Type;        //!< Type of backhaul. Default "p2p".
  std::string P2PDelay;    //!< Delay of point to point links. Default 10ms.
  std::string P2PDataRate; //!< Data rate of point to point links. Default 1Gb/s.
  std::string P2PQueueType; //!< Queue type installed on each node of the backhaul network.
                            //!< Default ns3::DropTailQueue.
  bool EnablePcap;          //!< Enable Pcap output over the backhaul network.
                            //!< Default false.
};

/**
 * \brief The Node section
 *
 * This class represent the common characteristics of a node. Please,
 * do not use it directly, but use instead one of its subclasses: GatewaySection,
 * ClientSection or RemoteSection.
 *
 */
class NodeSection : public Section
{
public:
  /**
   * \brief Constructor
   * \param name the name of the section
   */
  NodeSection (const std::string &name);

  /**
   * \brief Virtual deconstructor
   */
  virtual ~NodeSection () { }

  std::string GetName (void) { return m_name; }

  std::string MobilityModel; //!< The mobility model of the node.
                             //!< Default ns3::ConstantPositionMobilityModel
  std::string Position;      //!< The initial position of the node. Default [0,0,1.5]
  double TxPower;            //!< The transmission power of the node. It has meanings
                             //!< only in a LTE context, for now.

  Ipv4Address Address;       //!< Placeholder for node LAN address. Please do not use
                             //!< in user-written code.

  /**
   * \brief Real ugly way to force YOU to not use this class directly.
   */
  virtual void MakeMeInstantiable() = 0;

protected:
  std::string m_name; //!< Name of the section
};

/**
 * \brief The gateway representation
 *
 * With this class you can describe some attributes for a gateway in a PPDR
 * network.
 */
class GatewaySection : public NodeSection
{
public:
  /**
   * \brief Constructor
   *
   * \param name Name of the class (e.g. gateway0)
   */
  GatewaySection (const std::string &name);

  Ipv4Address ExternalAddress; //!< Placeholder for gw external address. Please do not use
                               //!< in user-written code.
  uint32_t EnbSrsPeriodicity;  //!< Lte attribute; please refer to the ns-3
                               //!< manual for a description
  double EnbAntennaOrientation; //!< Degrees of orientation of the antenna in
                                //!< an LTE context
  std::string P2PDelay;    //!< Delay of backhaul network (if applicable)
  std::string P2PDataRate; //!< Data rate of backaul network (if applicable)
  std::string P2PQueueType;//!< Queue type over the backhaul interface

  virtual void MakeMeInstantiable() { }
};

/**
 * \brief A PPDR client representation
 */
class ClientSection : public NodeSection
{
public:
  /**
   * \brief Constructor
   *
   * \param name Name of the class (e.g. client0)
   */
  ClientSection (const std::string &name);

  std::string LanDelay;    //!< Delay of LAN network (if applicable)
  std::string LanDataRate; //!< Data rate of LAN network (if applicable)
  std::string QueueType;   //!< Queue type over the LAN interface
  std::string CsiVariation; //!< String (to parse) which contains csi variation \todo

  virtual void MakeMeInstantiable() { }
};

/**
 * \brief A remote node representation
 */
class RemoteSection : public NodeSection
{
public:
  /**
   * \brief Constructor
   *
   * \param name Name of the class (e.g. gateway0)
   */
  RemoteSection (const std::string &name);
};

/**
 * \brief The section to configure an application over a node
 */
class AppSection : public Section
{
public:
  AppSection (const std::string &name);
  AppSection (uint32_t n);

  virtual std::string GetName (void) { return m_name; }
  virtual ~AppSection () { }

  virtual void Init ();

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
  BulkSendSection (uint32_t n);
  virtual ~BulkSendSection () { }

  virtual void Init ();

  uint32_t MaxBytes;
  uint32_t SendSize;
};

class OnOffSection : public BulkSendSection
{
public:
  OnOffSection (const std::string &name);
  OnOffSection (uint32_t n);
  virtual ~OnOffSection () { }

  virtual void Init ();

  std::string OnTime;
  std::string OffTime;
  std::string DataRate;
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

  virtual void MakeMeInstantiable() { }
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

/**
 * \brief Split a string into a vector of string
 *
 * \param s input string
 * \param delim delimiter character
 * \param elems return vector
 *
 * \return a reference of elems (to chain function calls)
 */
std::vector<std::string>& Split (const std::string &s, char delim,
                                 std::vector<std::string> &elems);
void DecodifyLanNPerG (const std::string &NPerG,
                       std::map <std::string, std::list<std::string> > &nodesIdToGwId);
void PrintGnuplottableEnbListToFile (std::string filename);
void PrintGnuplottableUeListToFile (std::string filename);

} // namespace ns3
#endif // PPDRTCLANUTILS_H
