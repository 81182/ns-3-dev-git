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
#include "ppdr-tc-scenarios.h"
#include "ppdr-tc-utils.h"
#include "ppdr-tc-statistics.h"

namespace ppdrtc {

void
DayToDay::PrintGeneral () const
{
  GeneralSection general;

  general.Prefix = m_prefix;
  general.RemoteN = m_remoteN;
  general.StopTime = 60;
  general.PrintExample();
}

void
DayToDay::PrintLan () const
{
  LanSection lan;
  lan.ClientN = m_clientN;
  lan.EnablePcapUserNetwork = false;
  lan.GatewayN = 1;
  lan.NPerG = "";
  lan.Type = "lte";
  lan.PrintExample();
}

void
DayToDay::PrintBackhaul () const
{
  BackhaulSection backhaul;

  backhaul.P2PDataRate = "10Gb/s";
  backhaul.P2PDelay = "10ms";
  backhaul.P2PQueueType = "ns3::CoDelQueue";
  backhaul.MaxBytes = 1500000;
  backhaul.PrintExample();
}
void
DayToDay::PrintLte () const
{
  LteSection lte;
  lte.PrintExample();
}

void
DayToDay::PrintStats () const
{
  StatisticsSection statistics;
  statistics.PrintExample();
}

void
DayToDay::PrintGw () const
{
  GatewaySection gw ("gateway0");
  gw.TxPower = 40;
  gw.Position = "[0.0,0.0,35.0]";
  gw.PrintExample();
}

void
DayToDay::PrintClients () const
{
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable> ();
  x->SetAttribute("Min", DoubleValue (95.0));
  x->SetAttribute("Max", DoubleValue (105.0));

  y->SetAttribute("Min", DoubleValue (-5.0));
  y->SetAttribute("Max", DoubleValue (5.0));

  for (uint32_t i=0; i<m_clientN; ++i)
    {
      std::stringstream nodename, position;
      nodename << "client" << i;

      ClientSection client (nodename.str());

      position << x->GetValue() << "," << y->GetValue() << ",1.5";
      client.Position = position.str();

      client.PrintExample();
    }
}

void
DayToDay::PrintRemotes () const
{
  for (uint32_t i=0; i<m_remoteN; ++i)
    {
      std::stringstream nodename;
      nodename << "remote" << i;
      RemoteSection remote (nodename.str());

      remote.Position = "[1000.0,1000.0,25.0]";
      remote.PrintExample();
    }
}

void
Scenario::PrintApps () const
{
  uint32_t appN = 0;
  uint32_t div = m_remoteUsers / m_remoteN;
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();

  x->SetAttribute("Min", DoubleValue (0.0));
  x->SetAttribute("Max", DoubleValue (1.0));

  for (uint32_t i=0; i<m_remoteN; ++i)
    {
      std::stringstream installedOn;
      installedOn << "remote" << i;

      Scenario::PrintAudioSink (installedOn.str(), appN++);
      Scenario::PrintVideoSink (installedOn.str(), appN++);
      Scenario::PrintWebSink (installedOn.str(), appN++);
      Scenario::PrintFTPSink (installedOn.str(), appN++);

      for (uint32_t j=i*div; j<i*div+div; ++j)
        {
          double prob = x->GetValue();

          if (prob < 0.5)
            {
              std::stringstream peerName1;
              peerName1 << "client" << (j+1)%m_clientN;
              Scenario::PrintAudioOnOff(installedOn.str(), peerName1.str(), appN++);
            }
        }
    }

  for (uint32_t i=0; i<m_clientN; ++i)
    {
      std::stringstream installedOn, peerName1, peerName2, peerName3, peerName4;
      installedOn << "client" << i;
      peerName1 << "remote" << (i)%m_remoteN;
      peerName2 << "remote" << (i+1)%m_remoteN;
      peerName3 << "remote" << (i+2)%m_remoteN;
      peerName4 << "remote" << (i+3)%m_remoteN;

      Scenario::PrintAudioSink (installedOn.str(), appN++);
      Scenario::PrintVideoSink (installedOn.str(), appN++);
      Scenario::PrintWebSink (installedOn.str(), appN++);
      Scenario::PrintFTPSink (installedOn.str(), appN++);

      double prob = x->GetValue();

      if (prob <= 0.1)
        {
         Scenario::PrintVideoOnOff(installedOn.str(), peerName1.str(), appN++);
        }

      prob = x->GetValue();

      if (prob <= 0.4)
        {
          Scenario::PrintWebOnOff(installedOn.str(), peerName3.str(), m_tcp,
                                  appN++);
        }

      prob = x->GetValue();

      if (prob <= 0.7)
        {
          Scenario::PrintAudioOnOff(installedOn.str(), peerName2.str(), appN++);

        }

      prob = x->GetValue();

      if (prob <= m_ftpProb)
        {
          Scenario::PrintFtp(installedOn.str(), peerName4.str(), m_tcp,
                             appN++);
        }
    }
}

void
Planned::PrintGeneral () const
{
  GeneralSection general;
  general.Prefix = m_prefix;
  general.RemoteN = m_remoteN; // from 103 originally
  general.StopTime = 60;
  general.PrintExample();
}

void
Planned::PrintLan () const
{
  LanSection lan;

  lan.ClientN = m_clientN;
  lan.EnablePcapUserNetwork = false;
  lan.GatewayN = 16;
  lan.NPerG = "";
  lan.Type = "lte";
  lan.PrintExample();

}

void
Planned::PrintBackhaul () const
{
  BackhaulSection backhaul;
  backhaul.P2PDataRate = "10Gb/s";
  backhaul.P2PDelay = "10ms";
  backhaul.P2PQueueType = "ns3::CoDelQueue";
  backhaul.MaxBytes = 1500000;
  backhaul.PrintExample();
}

void
PlannedInfrastructureless::PrintBackhaul () const
{
  BackhaulSection backhaul;
  backhaul.P2PDataRate = "500Mb/s";
  backhaul.P2PDelay = "350ms";
  backhaul.P2PQueueType = "ns3::DropTailQueue";
  backhaul.MaxBytes = 43750000;
  backhaul.PrintExample();
}

void
Planned::PrintLte () const
{
  LteSection lte;
  lte.PrintExample();
}
void
Planned::PrintStats () const
{
  StatisticsSection statistics;
  statistics.PrintExample();
}

void
Planned::PrintGw () const
{
  static std::string positions[] = { "[0,0,35.0]", "[0,50,35]", "[0,100,35]",
                                     "[0,150,35.0]", "[0,200,35]", "[50,200,35]",
                                     "[100,200,35]", "[150,200,35]", "[200,200,35]",
                                     "[200,150,35]", "[200,100,35]", "[200,50,35]",
                                     "[200,0,35]", "[150,0,35]", "[100,0,35]",
                                     "[50,0,35]"
                                   };
  static double orientations[]   = { 45, 0, 0, 0,
                                     -45, -90, -90, -90,
                                     -135, 180, 180, 180,
                                     135, 90, 90, 90
                                   };

  for (uint32_t i=0; i<16; ++i)
    {
      std::stringstream name;

      name << "gateway" << i;
      GatewaySection gw(name.str());

      gw.TxPower = 40;
      gw.EnbSrsPeriodicity = (m_clientN / 16) * 2 ;

      while (gw.EnbSrsPeriodicity % 80 != 0)
        {
          ++gw.EnbSrsPeriodicity;
        }

      gw.Position = positions[i];
      gw.EnbAntennaOrientation = orientations[i];
      gw.PrintExample();
    }
}

void
Planned::PrintClients () const
{
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable> ();

  x->SetAttribute("Min", DoubleValue (0.0));
  x->SetAttribute("Max", DoubleValue (200.0));

  y->SetAttribute("Min", DoubleValue (0.0));
  y->SetAttribute("Max", DoubleValue (200.0));

  for (uint32_t i=0; i<m_clientN; ++i)
    {
      std::stringstream nodename, position;
      nodename << "client" << i;

      ClientSection client (nodename.str());

      position << x->GetValue() << "," << y->GetValue() <<",1.5";
      client.Position = position.str();
      client.PrintExample();
    }
}

void
Planned::PrintRemotes () const
{
  for (uint32_t i=0; i<m_remoteN; ++i)
    {
      std::stringstream nodename;
      nodename << "remote" << i;

      RemoteSection remote (nodename.str());
      remote.Position = "[10000.0,10000.0,25.0]";
      remote.PrintExample();
    }
}


void
Unplanned::PrintGeneral () const
{
  GeneralSection general;
  general.Prefix = m_prefix;
  general.RemoteN = m_remoteN;
  general.StopTime = 60;
  general.PrintExample();
}

void
Unplanned::PrintLan () const
{
  LanSection lan;

  lan.ClientN = m_clientN;
  lan.EnablePcapUserNetwork = false;
  lan.GatewayN = 200;
  lan.NPerG = "";
  lan.Type = "lte";
  lan.PrintExample();
}

void
Unplanned::PrintBackhaul () const
{
  BackhaulSection backhaul;
  backhaul.P2PDataRate = "500Mb/s";
  backhaul.P2PDelay = "350ms";
  backhaul.P2PQueueType = "ns3::DropTailQueue";
  backhaul.MaxBytes = 43750000;
  backhaul.PrintExample();
}

void
Unplanned::PrintLte () const
{
  LteSection lte;
  lte.PrintExample();
}
void
Unplanned::PrintStats () const
{
  StatisticsSection statistics;
  statistics.PrintExample();
}

void
Unplanned::PrintGw () const
{
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable> ();
  Ptr<UniformRandomVariable> g = CreateObject<UniformRandomVariable> ();

  x->SetAttribute("Min", DoubleValue (0.0));
  x->SetAttribute("Max", DoubleValue (10000.0));

  y->SetAttribute("Min", DoubleValue (0.0));
  y->SetAttribute("Max", DoubleValue (10000.0));

  g->SetAttribute("Min", DoubleValue (0.0));
  g->SetAttribute("Max", DoubleValue (360.0));

  for (uint32_t i=0; i<200; ++i)
    {
      std::stringstream name, position;

      position << x->GetValue() << "," <<  y->GetValue() << ",15";

      name << "gateway" << i;
      GatewaySection gw (name.str());

      gw.TxPower = 40;
      gw.EnbSrsPeriodicity = 80;
      gw.Position = position.str();
      gw.EnbAntennaOrientation = g->GetValue();
      gw.PrintExample();
    }
}

void
Unplanned::PrintClients () const
{
  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
  Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable> ();

  x->SetAttribute("Min", DoubleValue (0.0));
  x->SetAttribute("Max", DoubleValue (10000.0));

  y->SetAttribute("Min", DoubleValue (0.0));
  y->SetAttribute("Max", DoubleValue (10000.0));

  for (uint32_t i=0; i<m_clientN; ++i)
    {
      std::stringstream nodename, position;
      nodename << "client" << i;

      ClientSection client (nodename.str());

      position << x->GetValue() << "," << y->GetValue() <<",1.5";
      client.Position = position.str();
      client.PrintExample();
    }
}

void
Unplanned::PrintRemotes () const
{
  for (uint32_t i=0; i<m_remoteN; ++i)
    {
      std::stringstream nodename;
      nodename << "remote" << i;

      RemoteSection remote (nodename.str());
      remote.Position = "[10000.0,10000.0,25.0]";
      remote.PrintExample();
    }
}

} // namespace ppdr-tc
