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
#include "ppdr-tc-utils.h"
#include "ns3/log.h"
#include "ns3/lte-module.h"
#include "ns3/mobility-model.h"

#include <algorithm>
#include <string>

NS_LOG_COMPONENT_DEFINE ("PPDRTCUtils");

namespace ppdrtc {
using namespace ns3;

std::vector<std::string>
&Split (const std::string &s, char delim, std::vector<std::string> &elems)
{
  std::stringstream ss (s);
  std::string item;

  while (std::getline (ss, item, delim))
    {
      if (!item.empty ())
        {
          elems.push_back (item);
        }
    }

  return elems;
}

Section::XYZ
Section::ToXYZ (const std::string &from)
{
  static const char chars[] = "[];";
  std::string s = from;

  for (unsigned int i = 0; i < strlen (chars); ++i)
    {
      s.erase (std::remove (s.begin (), s.end (), chars[i]), s.end ());
    }

  std::vector<std::string> values;

  Split (s, ',', values);

  if (values.size () != 3)
    {
      NS_FATAL_ERROR ("BAD position: " << s);
    }

  XYZ v;

  v.x = atof (values.at (0).c_str ());
  v.y = atof (values.at (1).c_str ());
  v.z = atof (values.at (2).c_str ());

  return v;
}

void
Section::PrintExample ()
{
  std::cout << "[" << GetName () << "]" << std::endl;
  for(KeyMap::iterator it = m_keys.begin (); it != m_keys.end (); ++it)
    {
      std::string name =  it->first;
      KeyDescription *desc = &(it->second);

      if (desc->m_type == "bool")
        {
          bool *v = static_cast<bool*> (desc->m_obj);
          if (!desc->m_desc.empty())
            {
              std::cout << "#" << desc->m_desc << std::endl;
            }
          if (*v)
            {
              std::cout << name << " = true" << " ;type=" << desc->m_type << std::endl;
            }
          else
            {
              std::cout << name << " = false" << " ;type=" << desc->m_type << std::endl;
            }
        }
      else if (desc->m_type == "double")
        {
          double *v = static_cast<double*> (desc->m_obj);
          if (!desc->m_desc.empty())
            {
              std::cout << "#" << desc->m_desc << std::endl;
            }
          std::cout << name << " = "<< (*v) << " ;type=" << desc->m_type << std::endl;
        }
      else if (desc->m_type == "string")
        {
          std::string *v = static_cast<std::string*> (desc->m_obj);
          if (!desc->m_desc.empty())
            {
              std::cout << "#" << desc->m_desc << std::endl;
            }
          std::cout << name << " = "<< (*v) << " ;type=" << desc->m_type << std::endl;
        }
      else if (desc->m_type == "uint32")
        {
          uint32_t *v = static_cast<uint32_t*> (desc->m_obj);
          if (!desc->m_desc.empty())
            {
              std::cout << "#" << desc->m_desc << std::endl;
            }
          std::cout << name << " = "<< (*v) << " ;type=" << desc->m_type << std::endl;
        }
    }

  std::cout << std::endl;
}

void
Section::ConfigureFromFile (const std::string &filename)
{
  if (!filename.empty ())
    {
      if (filename == "none")
        {
          return;
        }
      NS_ASSERT (FileExists (filename));
      Config::SetDefault ("ns3::ConfigStore::Filename",   StringValue (filename));
      Config::SetDefault ("ns3::ConfigStore::Mode",       StringValue ("Load"));
      Config::SetDefault ("ns3::ConfigStore::FileFormat", StringValue ("RawText"));
      ConfigStore inputConfig;
      inputConfig.ConfigureDefaults ();
    }
}

void
Section::Fill (INIReader& ini)
{
  for(KeyMap::iterator it = m_keys.begin (); it != m_keys.end (); ++it)
    {
      std::string name =  it->first;
      KeyDescription *desc = &(it->second);

      std::string value = ini.Get (GetName (), name, "");

      if (value == "")
        {
          NS_LOG_UNCOND ("Warning: key=" << name << " in section [" << GetName () <<
                         "] not present in the config file. Using default value");
          continue;
        }

      if (desc->m_type == "bool")
        {
          bool *v = static_cast<bool*> (desc->m_obj);
          INI_READ_BOOL (ini, GetName (), name, *v);
        }
      else if (desc->m_type == "double")
        {
          double *v = static_cast<double*> (desc->m_obj);
          INI_READ_REAL (ini, GetName (), name, *v);
        }
      else if (desc->m_type == "string")
        {
          std::string *v = static_cast<std::string*> (desc->m_obj);
          INI_READ_STRING (ini, GetName (), name, *v);
        }
      else if (desc->m_type == "uint32")
        {
          uint32_t *v = static_cast<uint32_t*> (desc->m_obj);
          INI_READ_UINT32 (ini, GetName (), name, *v);
        }
    }
}

GeneralSection::GeneralSection ()
{
  DECLARE_KEY ("bool", "EnableC2ML",            "", &EnableC2ML, false);
  DECLARE_KEY ("bool", "Verbose",               "",&Verbose, true);
  DECLARE_KEY ("double", "StopTime", "", &StopTime, 60.0);
  DECLARE_KEY ("uint32", "RemoteN", "", &RemoteN, 1);
  DECLARE_KEY ("string", "Prefix", "", &Prefix, "ppdr-tc-simulation");
}

void
Section::Print ()
{
  NS_LOG_UNCOND (GetName () << " section:");

  for(KeyMap::iterator it = m_keys.begin (); it != m_keys.end (); ++it)
    {
      std::string name =  it->first;
      KeyDescription *desc = &(it->second);

      if (desc->m_type == "bool")
        {
          bool *v = static_cast<bool*> (desc->m_obj);

          NS_LOG_UNCOND ("\t" << name << ": " << *(v));
        }
      else if (desc->m_type == "double")
        {
          double *v = static_cast<double*> (desc->m_obj);

          NS_LOG_UNCOND ("\t" << name << ": " << *(v));
        }
      else if (desc->m_type == "string")
        {
          std::string *v = static_cast<std::string*> (desc->m_obj);

          NS_LOG_UNCOND ("\t" << name << ": " << *(v));
        }
      else if (desc->m_type == "uint32")
        {
          uint32_t *v = static_cast<uint32_t*> (desc->m_obj);

          NS_LOG_UNCOND ("\t" << name << ": " << *(v));
        }
    }
}

LanSection::LanSection ()
{
  DECLARE_KEY ("string", "Type",       "", &Type,       "lte");
  DECLARE_KEY ("string", "Attributes", "", &Attributes, "none");
  DECLARE_KEY ("uint32", "ClientN",    "", &ClientN,    1);
  DECLARE_KEY ("uint32", "GatewayN",   "", &GatewayN,   1);
  DECLARE_KEY ("string", "NPerG",      "", &NPerG,      "[gateway0:client0]");
  DECLARE_KEY ("bool", "EnablePcapUserNetwork", "", &EnablePcapUserNetwork, false);
}

void
LanSection::Fill (INIReader &ini)
{
  Section::Fill (ini);

  if (Type.compare("p2p") != 0 &&
      Type.compare("lte") != 0)
    {
      NS_FATAL_ERROR ("Type " << Type << " for LAN is not supported.");
    }

  if (ClientN == 0)
    {
      NS_FATAL_ERROR ("No clients added in " << GetName () << "->ClientN");
    }

  if (GatewayN == 0)
    {
      NS_FATAL_ERROR ("No gateway added in " << GetName () << "->GatewayN");
    }

  ConfigureFromFile (Attributes);
}

BackhaulSection::BackhaulSection () : Section ()
{
  DECLARE_KEY ("string", "Type",         "", &Type,         "p2p");
  DECLARE_KEY ("string", "P2PDelay",     "", &P2PDelay,     "10ms");
  DECLARE_KEY ("string", "P2PDataRate",  "", &P2PDataRate,  "1Gb/s");
  DECLARE_KEY ("string", "P2PQueueType", "", &P2PQueueType, "ns3::DropTailQueue");
  DECLARE_KEY ("bool", "EnablePcap",    "",&EnablePcap, false);
}

NodeSection::NodeSection (const std::string &name)
{
  m_name = name;

  DECLARE_KEY ("string", "MobilityModel", "", &MobilityModel,
               "ns3::ConstantPositionMobilityModel");
  DECLARE_KEY ("string", "Position", "", &Position,
               "0.0,0.0,1.5");
  DECLARE_KEY ("double", "TxPower",    "", &TxPower,    23.0);
}

AppSection::AppSection (const std::string &name)
{
  m_name = name;

  DECLARE_KEY ("double", "StartTime",    "", &StartTime,    0.0);
  DECLARE_KEY ("double", "StopTime",     "", &StopTime,     60.0);
  DECLARE_KEY ("string", "ConnectedTo", "", &ConnectedTo, "remote0");
  DECLARE_KEY ("string", "InstalledOn", "", &InstalledOn, "remote0");
  DECLARE_KEY ("string", "AppType", "", &AppType, "PacketSink");
  DECLARE_KEY ("uint32", "Port", "", &Port, 5000);
  DECLARE_KEY ("string", "Protocol", "", &Protocol, "TCP");
  DECLARE_KEY ("uint32", "InitialCwnd", "", &InitialCwnd, 10);
  DECLARE_KEY ("uint32", "InitialSSTh", "", &InitialSSTh, 4000000);
  DECLARE_KEY ("uint32", "DelAckCount", "", &DelAckCount, 1);
  DECLARE_KEY ("double", "TxTime", "", &TxTime, 0.125);
  DECLARE_KEY ("double", "BNoordwijk", "", &BNoordwijk, 0.300);
  DECLARE_KEY ("string", "SocketType",   "", &SocketType,   "ns3::TcpCubic");
}


BulkSendSection::BulkSendSection (const std::string &name) : AppSection (name)
{
  DECLARE_KEY ("uint32", "MaxBytes", "", &MaxBytes, 0);
  DECLARE_KEY ("uint32", "SendSize", "", &SendSize, 500);

  AppType = "BulkSend";
}

OnOffSection::OnOffSection (const std::string &name) : BulkSendSection (name)
{
  DECLARE_KEY ("string", "DataRate", "", &DataRate, "500kb/s");
  DECLARE_KEY ("string", "OnTime", "", &OnTime, "ns3::ConstantRandomVariable[Constant=1.0]");
  DECLARE_KEY ("string", "OffTime", "", &OffTime, "ns3::ConstantRandomVariable[Constant=1.0]");

  AppType = "OnOff";
}

ClientSection::ClientSection (const std::string &name) : NodeSection (name)
{
  DECLARE_KEY ("string", "LanDelay",     "", &LanDelay,     "10ms");
  DECLARE_KEY ("string", "LanDataRate",  "", &LanDataRate,  "100Mb/s");
  DECLARE_KEY ("string", "QueueType",    "", &QueueType,    "ns3::DropTailQueue");
  DECLARE_KEY ("string", "CsiVariation", "", &CsiVariation, "none");
}

RemoteSection::RemoteSection (const std::string &name) : NodeSection (name)
{
}

GatewaySection::GatewaySection (const std::string &name) : NodeSection (name)
{
  DECLARE_KEY ("uint32", "EnbSrsPeriodicity", "", &EnbSrsPeriodicity, 80);
  DECLARE_KEY ("string", "P2PDelay", "", &P2PDelay, "none");
  DECLARE_KEY ("string", "P2PDataRate", "", &P2PDataRate, "none");
  DECLARE_KEY ("string", "P2PQueueType", "", &P2PQueueType, "none");
}

Attributes::Attributes (const std::string &name)
{
  this->name = name;

  DECLARE_KEY ("string", "Attribute", "", &Attribute, "none");
}

void
Attributes::Fill (INIReader &ini)
{
  Section::Fill (ini);

  ConfigureFromFile (Attribute);
}

LteSection::LteSection () : Attributes ("lte")
{
  DECLARE_KEY ("bool", "EnableREM", "", &EnableREM, false);
  DECLARE_KEY ("double", "XMinREM", "", &XMinREM, -100);
  DECLARE_KEY ("double", "XMaxREM", "", &XMaxREM, 100);
  DECLARE_KEY ("double", "YMinREM", "", &YMinREM, -100);
  DECLARE_KEY ("double", "YMaxREM", "", &YMaxREM, 100);
  DECLARE_KEY ("double", "HeightREM", "", &HeightREM, 1.5);

  DECLARE_KEY ("string", "EnbAntennaType", "", &EnbAntennaType,
               "ns3::CosineAntennaModel");
  DECLARE_KEY ("double", "EnbAntennaMaxGain", "", &EnbAntennaMaxGain, 18.0);
  DECLARE_KEY ("string", "UeAntennaModel", "", &UeAntennaModel,
               "ns3::IsotropicAntennaModel");
  DECLARE_KEY ("uint32", "UlBandwidth", "", &UlBandwidth, 100);
  DECLARE_KEY ("uint32", "DlBandwidth", "", &DlBandwidth, 100);
  DECLARE_KEY ("uint32", "DlEarfcn", "", &DlEarfcn, 1575);
  DECLARE_KEY ("uint32", "UlEarfcn", "", &UlEarfcn, 19575);
  DECLARE_KEY ("string", "PathlossType", "", &PathlossType,
               "ns3::OkumuraHataPropagationLossModel");
  DECLARE_KEY ("string", "EnvironmentValue", "", &EnvironmentValue,
               "UrbanEnvironment");
  DECLARE_KEY ("string", "CitySizeValue", "", &CitySizeValue,
               "SmallCity");
  DECLARE_KEY ("string", "SchedulerType", "", &SchedulerType,
               "ns3::PfFfMacScheduler");
  DECLARE_KEY ("bool", "EnablePcapX2Link", "", &EnablePcapX2Link, false);
  DECLARE_KEY ("bool", "EnablePcapS1uLink", "", &EnablePcapS1uLink, false);
}

CitySize
LteSection::GetCitySize()
{
  if (CitySizeValue == "SmallCity")
    {
      return SmallCity;
    }
  else if (CitySizeValue == "MediumCity")
    {
      return MediumCity;
    }
  else if (CitySizeValue == "LargeCity")
    {
      return LargeCity;
    }
  else
    {
      return SmallCity;
    }
}

EnvironmentType
LteSection::GetEnvironment()
{
  if (EnvironmentValue == "UrbanEnvironment")
    {
      return UrbanEnvironment;
    }
  else if (EnvironmentValue == "SubUrbanEnvironment")
    {
      return SubUrbanEnvironment;
    }
  else if (EnvironmentValue == "OpenAreasEnvironment")
    {
      return OpenAreasEnvironment;
    }
  else
    {
      return UrbanEnvironment;
    }
}

StatisticsSection::StatisticsSection ()
{
  DECLARE_KEY ("bool", "EnableFlowmon", "", &EnableFlowmon, true);
  DECLARE_KEY ("bool", "EnableQueue", "", &EnableQueue, true);
  DECLARE_KEY ("bool", "EnableDelay", "", &EnableDelay, true);
  DECLARE_KEY ("bool", "EnableJitter", "", &EnableJitter, true);
  DECLARE_KEY ("bool", "EnableThroughput", "", &EnableThroughput, true);
  DECLARE_KEY ("bool", "EnableNetAnim", "", &EnableNetAnim, false);
  DECLARE_KEY ("double", "ThroughputSamplingTime", "", &ThroughputSamplingTime, 1.0);
}



/**
 * \brief Decodify the string NPerG into a map
 *
 \verbatim
 INPUT:
                      block 0                  block 1
           |------------------------| |------------------------|
   Syntax: [gateway0:client0,client1];[gateway1:client2,client3]
            |         |
           gwName     |
                   list of nodes
  \endverbatim
 *
 * Semantic: Gateway 0 has clients 0,1,2,3 and Gateway 1 has clients 4,5,6
 * \param NPerG String
 * \param nodesIdToGwId Input map
 */
void
DecodifyLanNPerG (const std::string &NPerG,
                  std::map <std::string, std::list<std::string> > &nodesIdToGwId)
{
  std::vector<std::string> blocks;
  Split (NPerG, ';', blocks);
  NS_LOG_INFO ("Decodify entire string=" << NPerG);

  for (uint32_t i=0; i<blocks.size (); ++i)
    {
      std::string block = blocks.at (i);

      static const char chars[] = "[];";

      for (unsigned int i = 0; i < strlen (chars); ++i)
        {
          block.erase (std::remove (block.begin (), block.end (), chars[i]), block.end ());
        }

      NS_LOG_INFO ("Decodify block=" << block);

      std::vector<std::string> v;
      Split (block, ':', v);

      if (v.size() != 2)
        {
          NS_FATAL_ERROR ("Bad NPerG string " << NPerG <<
                          " example: [gateway0:client0];[gateway1:client1]");
        }

      std::string gwName = v.at (0);
      NS_LOG_INFO ("gwName=" << gwName);

      std::list<std::string> listOfNodes;
      std::vector<std::string> nodes;

      Split (v.at (1), ',', nodes);

      for (uint32_t i=0; i<nodes.size (); ++i)
        {
          NS_LOG_INFO ("Node: " << nodes.at (i));
          listOfNodes.insert (listOfNodes.end (), nodes.at (i));
        }

      nodesIdToGwId.insert (nodesIdToGwId.end (),
                            std::pair<std::string, std::list<std::string> > (gwName, listOfNodes));
    }
}

/**
 * \brief Get Ue list in a Gnuplot-compatible format
 *
 * Use it to plot REM
 *
 * \param filename output Filename
 */
void
PrintGnuplottableUeListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteUeNetDevice> uedev = node->GetDevice (j)->GetObject <LteUeNetDevice> ();
          if (uedev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" << uedev->GetImsi ()
                      << "\" at "<< pos.x << "," << pos.y << " left font \"Helvetica,4\" textcolor rgb \"grey\" front point pt 1 ps 0.3 lc rgb \"grey\" offset 0,0"
                      << std::endl;
            }
        }
    }
}

/**
 * \brief Get Enb list in a Gnuplot-compatible format
 *
 * Use it to plot REM
 *
 * \param filename output Filename
 */
void
PrintGnuplottableEnbListToFile (std::string filename)
{
  std::ofstream outFile;
  outFile.open (filename.c_str (), std::ios_base::out | std::ios_base::trunc);
  if (!outFile.is_open ())
    {
      return;
    }
  for (NodeList::Iterator it = NodeList::Begin (); it != NodeList::End (); ++it)
    {
      Ptr<Node> node = *it;
      int nDevs = node->GetNDevices ();
      for (int j = 0; j < nDevs; j++)
        {
          Ptr<LteEnbNetDevice> enbdev = node->GetDevice (j)->GetObject <LteEnbNetDevice> ();
          if (enbdev)
            {
              Vector pos = node->GetObject<MobilityModel> ()->GetPosition ();
              outFile << "set label \"" << enbdev->GetCellId ()
                      << "\" at "<< pos.x << "," << pos.y
                      << " left font \"Helvetica,4\" textcolor rgb \"white\" front  point pt 2 ps 0.3 lc rgb \"white\" offset 0,0"
                      << std::endl;
            }
        }
    }
}

void
PrintDayToDay ()
{
  GeneralSection general;

  general.Prefix = "ppdr-tc-d2d-a-0";
  general.RemoteN = 9;
  general.StopTime = 60;
  general.PrintExample();

  LanSection lan;

  lan.ClientN = 26;
  lan.EnablePcapUserNetwork = false;
  lan.GatewayN = 1;
  std::string NPerG;
  {
    std::stringstream names;
    names << "[gateway0:";
    for (uint32_t i = 0; i<24; ++i)
      {
        names << "client" << i <<",";
      }
    names << "client25]";
    NPerG = names.str();
  }
  lan.NPerG = NPerG;
  lan.Type = "lte";
  lan.PrintExample();

  BackhaulSection backhaul;

  backhaul.PrintExample();

  LteSection lte;

  lte.PrintExample();

  StatisticsSection statistics;

  statistics.PrintExample();

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  x->SetAttribute("Min", DoubleValue (95.0));
  x->SetAttribute("Max", DoubleValue (105.0));

  for (uint32_t i=0; i<26; ++i)
    {
      std::stringstream nodename, position;
      nodename << "client" << i;

      ClientSection client (nodename.str());

      position << x->GetValue() << ",0,1.5";
      client.Position = position.str();

      client.PrintExample();
    }

  GatewaySection gw ("gateway0");
  gw.TxPower = 80;
  gw.Position = "[0.0,0.0,35.0]";
  gw.PrintExample();

  for (uint32_t i=0; i<9; ++i)
    {
      std::stringstream nodename;
      nodename << "remote" << i;
      RemoteSection remote (nodename.str());

      remote.Position = "[1000.0,1000.0,25.0]";
      remote.PrintExample();
    }

  // Fire services: 5 udp sink, 5 udp onoff, 1 tcp sink, 1 tcp bulk
  {
    AppSection udp1("app0"), udp2("app1"), udp3("app2"), udp4("app3"), udp5("app4");
    AppSection tcp1("app5");

    OnOffSection onoff1("app6"), onoff2("app7"), onoff3("app8"), onoff4("app9"), onoff5("app10");
    BulkSendSection bulk1("app11");

    // Port 443 for udp onoff
    udp1.Port = udp2.Port = udp3.Port = udp4.Port = udp5.Port =
        onoff1.Port = onoff2.Port = onoff3.Port = onoff4.Port = onoff5.Port = 443;

    // protocol udp for udp onoff (really?)
    udp1.Protocol = udp2.Protocol = udp3.Protocol = udp4.Protocol = udp5.Protocol =
        onoff1.Protocol = onoff2.Protocol = onoff3.Protocol = onoff4.Protocol =
        onoff5.Protocol = "UDP";

    // port and protocol for TCP..
    tcp1.Port = bulk1.Port = 21;
    tcp1.Protocol = bulk1.Protocol = "TCP";

    // data rate for udp onoff
    onoff1.DataRate = onoff2.DataRate = onoff3.DataRate = onoff4.DataRate =
        onoff5.DataRate = "20kb/s";

    // CONNECT!

    // sink before all
    udp1.ConnectedTo = "any";
    udp1.InstalledOn = "remote1";
    udp1.PrintExample();

    udp2.ConnectedTo = "any";
    udp2.InstalledOn = "client1";
    udp2.PrintExample();

    udp3.ConnectedTo = "any";
    udp3.InstalledOn = "client2";
    udp3.PrintExample();

    udp4.ConnectedTo = "any";
    udp4.InstalledOn = "client3";
    udp4.PrintExample();

    udp5.ConnectedTo = "any";
    udp5.InstalledOn = "remote2";
    udp5.PrintExample();

    // onoff
    onoff1.ConnectedTo = "client1";
    onoff1.InstalledOn = "remote0";
    onoff1.PrintExample();

    onoff2.ConnectedTo = "remote1";
    onoff2.InstalledOn = "client4";
    onoff2.PrintExample();

    onoff3.ConnectedTo = "remote2";
    onoff3.InstalledOn = "client5";
    onoff3.PrintExample();

    onoff4.ConnectedTo = "client2";
    onoff4.InstalledOn = "remote1";
    onoff4.PrintExample();

    onoff5.ConnectedTo = "client3";
    onoff5.InstalledOn = "remote2";
    onoff5.PrintExample();

    // tcp
    tcp1.InstalledOn = "remote0";
    tcp1.ConnectedTo = "any";
    tcp1.PrintExample();

    bulk1.InstalledOn = "client0";
    bulk1.ConnectedTo = "remote0";
    bulk1.PrintExample();
  }

  // EMS: 4 udp sink, 3 udp onoff, 2 tcp sink, 4 tcp bulk
  {
    AppSection udp1("app12"), udp2("app13"), udp3("app14"), udp4("app15");
    AppSection tcp1("app16"), tcp2("app17");

    OnOffSection onoff1("app18"), onoff2("app19"), onoff3("app20"), onoff4("app25"), onoff5("app26");
    BulkSendSection bulk1("app21"), bulk2("app22"), bulk3("app23"), bulk4("app24");

    // Port for udp
    udp1.Port = udp2.Port = udp3.Port = udp4.Port = onoff1.Port = onoff2.Port =
        onoff3.Port = 443;
    tcp1.Port = tcp2.Port = bulk1.Port = bulk2.Port = bulk3.Port = bulk4.Port = 21;

    // protocol
    udp1.Protocol = udp2.Protocol = udp3.Protocol = udp4.Protocol =
        onoff1.Protocol = onoff2.Protocol = onoff3.Protocol = "UDP";
    tcp1.Protocol = tcp2.Protocol = bulk1.Protocol = bulk2.Protocol =
        bulk3.Protocol = bulk4.Protocol = "TCP";

    // data rate for udp onoff
    onoff1.DataRate = onoff2.DataRate = onoff3.DataRate = onoff4.DataRate =
        onoff5.DataRate = "20kb/s";

    // CONNECT!

    udp1.ConnectedTo = udp2.ConnectedTo = udp3.ConnectedTo = udp4.ConnectedTo = "any";

    udp1.InstalledOn = "client10";
    udp1.PrintExample();
    udp2.InstalledOn = "client11";
    udp2.PrintExample();
    udp3.InstalledOn = "client12";
    udp3.PrintExample();
    udp4.InstalledOn = "remote3";
    udp4.PrintExample();

    onoff1.InstalledOn = "client6";
    onoff1.ConnectedTo = "remote3";
    onoff1.PrintExample();

    onoff2.InstalledOn = "client13";
    onoff2.ConnectedTo = "remote3";
    onoff2.PrintExample();

    onoff3.InstalledOn = "remote4";
    onoff3.ConnectedTo = "client12";
    onoff3.PrintExample();

    onoff4.InstalledOn = "remote4";
    onoff4.ConnectedTo = "client11";
    onoff4.PrintExample();

    onoff5.InstalledOn = "remote4";
    onoff5.ConnectedTo = "client10";
    onoff5.PrintExample();

    // tcp

    tcp1.ConnectedTo = tcp2.ConnectedTo = "any";
    tcp1.InstalledOn = "remote5";
    tcp1.PrintExample();

    tcp2.InstalledOn = "client9";
    tcp2.PrintExample();

    bulk1.InstalledOn = "client7";
    bulk1.ConnectedTo = "remote5";
    bulk1.PrintExample();

    bulk2.InstalledOn = "client8";
    bulk2.ConnectedTo = "remote5";
    bulk2.PrintExample();

    bulk3.InstalledOn = "remote3";
    bulk3.ConnectedTo = "client9";
    bulk3.PrintExample();

    bulk4.InstalledOn = "remote4";
    bulk4.ConnectedTo = "client9";
    bulk4.PrintExample();
  }

}

} // namespace ppdrtc
