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

GeneralSection::GeneralSection ()
{
  DECLARE_KEY ("bool", "EnableC2ML",            "", &EnableC2ML, false);
  DECLARE_KEY ("bool", "Verbose",               "",&Verbose, true);
  DECLARE_KEY ("double", "StopTime", "", &StopTime, 60.0);
  DECLARE_KEY ("uint32", "RemoteN", "", &RemoteN, 1);
  DECLARE_KEY ("string", "Prefix", "", &Prefix, "ppdr-tc-simulation");
}

LanSection::LanSection () : Attributes ("lan")
{
  DECLARE_KEY ("string", "Type",       "", &Type,       "lte");
  DECLARE_KEY ("uint32", "ClientN",    "", &ClientN,    1);
  DECLARE_KEY ("uint32", "GatewayN",   "", &GatewayN,   1);
  DECLARE_KEY ("string", "NPerG",      "", &NPerG,      "[gateway0:client0]");
  DECLARE_KEY ("bool", "EnablePcapUserNetwork", "", &EnablePcapUserNetwork, false);
}

void
LanSection::Fill (INIReader &ini)
{
  Attributes::Fill (ini);

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

  Init ();
}

AppSection::AppSection (uint32_t n)
{
  std::stringstream name;
  name << "app" << n;
  m_name = name.str ();

  Init ();
}

void
AppSection::Init ()
{
  DECLARE_KEY ("double", "StartTime",    "", &StartTime,    0.0);
  DECLARE_KEY ("double", "StopTime",     "", &StopTime,     60.0);
  DECLARE_KEY ("string", "ConnectedTo", "", &ConnectedTo, "remote0");
  DECLARE_KEY ("string", "InstalledOn", "", &InstalledOn, "remote0");
  DECLARE_KEY ("string", "AppType", "", &AppType, "PacketSink");
  DECLARE_KEY ("uint32", "Port", "", &Port, 5000);
  DECLARE_KEY ("string", "Protocol", "", &Protocol, "TCP");
}

SendAppSection::SendAppSection (const std::string &name) : AppSection (name)
{
  Init ();
}

SendAppSection::SendAppSection (uint32_t n) : AppSection (n)
{
  Init ();
}

void
SendAppSection::Init()
{
  DECLARE_KEY ("uint32", "InitialCwnd", "", &InitialCwnd, 10);
  DECLARE_KEY ("uint32", "InitialSSTh", "", &InitialSSTh, 4000000);
  DECLARE_KEY ("uint32", "DelAckCount", "", &DelAckCount, 1);
  //DECLARE_KEY ("double", "TxTime", "", &TxTime, 0.125);
  //DECLARE_KEY ("double", "BNoordwijk", "", &BNoordwijk, 0.300);
  DECLARE_KEY ("string", "SocketType",   "", &SocketType,   "ns3::TcpCubic");
}

BulkSendSection::BulkSendSection (const std::string &name) : SendAppSection (name)
{
  Init ();
}

BulkSendSection::BulkSendSection (uint32_t n) : SendAppSection (n)
{
  Init ();
}

void
BulkSendSection::Init ()
{
  DECLARE_KEY ("uint32", "MaxBytes", "", &MaxBytes, 0);
  DECLARE_KEY ("uint32", "SendSize", "", &SendSize, 500);

  AppType = "BulkSend";
}

OnOffSection::OnOffSection (const std::string &name) : BulkSendSection (name)
{
  Init ();
}

OnOffSection::OnOffSection (uint32_t n) : BulkSendSection (n)
{
  Init ();
}

void
OnOffSection::Init ()
{
  DECLARE_KEY ("string", "DataRate", "", &DataRate, "500kb/s");
  DECLARE_KEY ("string", "OnTime", "", &OnTime, "ns3::UniformRandomVariable[Min=0.0,Max=1.0]");
  DECLARE_KEY ("string", "OffTime", "", &OffTime, "ns3::UniformRandomVariable[Min=0.0,Max=1.0]");

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
  DECLARE_KEY ("double", "EnbAntennaOrientation", "", &EnbAntennaOrientation,
               0.0);
  DECLARE_KEY ("string", "P2PDelay", "", &P2PDelay, "none");
  DECLARE_KEY ("string", "P2PDataRate", "", &P2PDataRate, "none");
  DECLARE_KEY ("string", "P2PQueueType", "", &P2PQueueType, "none");
}

Attributes::Attributes (const std::string &name)
{
  m_name = name;

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

} // namespace ppdrtc
