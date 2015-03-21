#include "ppdr-tc-scenarios.h"
#include "ppdr-tc-utils.h"
#include "ppdr-tc-statistics.h"

namespace ppdrtc {

void
DayToDay::PrintGeneral () const
{
  GeneralSection general;

  general.Prefix = "ppdr-tc-d2d-a-0";
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

  backhaul.P2PDataRate = "1Gb/s";
  backhaul.P2PDelay = "10ms";
  backhaul.P2PQueueType = "ns3::CoDelQueue";
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

  for (uint32_t i=0; i<m_remoteN; ++i)
    {
      std::stringstream installedOn, peerName1;
      installedOn << "remote" << i;
      peerName1 << "client" << (i+1)%m_clientN;

      Scenario::PrintAudioSink (installedOn.str(), appN++);
      Scenario::PrintVideoSink (installedOn.str(), appN++);
      Scenario::PrintWebSink (installedOn.str(), appN++);
      Scenario::PrintFTPSink (installedOn.str(), appN++);

      if (i < m_remoteN*50/100)
        {
          Scenario::PrintAudioOnOff(installedOn.str(), peerName1.str(), appN++);
        }
    }

  Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();

  x->SetAttribute("Min", DoubleValue (0.0));
  x->SetAttribute("Max", DoubleValue (1.0));

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
          Scenario::PrintWebOnOff(installedOn.str(), peerName3.str(), appN++);
        }

      prob = x->GetValue();

      if (prob <= 0.7)
        {
          Scenario::PrintAudioOnOff(installedOn.str(), peerName2.str(), appN++);

        }

      prob = x->GetValue();

      if (prob <= 0.1)
        {
          Scenario::PrintFtp(installedOn.str(), peerName4.str(), appN++);
        }
    }
}

void
Planned::PrintGeneral () const
{
  GeneralSection general;
  general.Prefix = "ppdr-tc-pla-a-0";
  general.RemoteN = 103;
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
  backhaul.P2PDataRate = "1Gb/s";
  backhaul.P2PDelay = "10ms";
  backhaul.P2PQueueType = "ns3::CoDelQueue";
  backhaul.PrintExample();
}

void
PlannedInfrastructureless::PrintBackhaul () const
{
  BackhaulSection backhaul;
  backhaul.P2PDataRate = "500Mb/s";
  backhaul.P2PDelay = "350ms";
  backhaul.P2PQueueType = "ns3::DropTailQueue";
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
      gw.EnbSrsPeriodicity = 80;
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
PrintUnplannedNoBackhaul ()
{
  GeneralSection general;

  general.Prefix = "ppdr-tc-unp-a-0";

  general.RemoteN = 0;
  general.StopTime = 60;
  general.PrintExample();

  LanSection lan;

  lan.ClientN = 610;
  lan.EnablePcapUserNetwork = false;
  lan.GatewayN = 200;
  lan.NPerG = "";
  lan.Type = "lte";
  lan.PrintExample();

  BackhaulSection backhaul;
  backhaul.P2PDataRate = "1Gb/s";
  backhaul.P2PDelay = "10ms";
  backhaul.P2PQueueType = "ns3::CoDelQueue";
  backhaul.PrintExample();

  LteSection lte;

  lte.PrintExample();

  StatisticsSection statistics;
  statistics.PrintExample();

  // Gateways
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

    for (uint32_t i=0; i<lan.GatewayN; ++i)
      {
        std::stringstream name, position;

        position << x->GetValue() << "," <<  y->GetValue() << ",15";

        name << "gateway" << i;
        GatewaySection *gw = new GatewaySection (name.str());

        gw->TxPower = 40;
        gw->EnbSrsPeriodicity = 80;
        gw->Position = position.str();
        gw->EnbAntennaOrientation = g->GetValue();
        gw->PrintExample();

        delete gw;
      }
  }

  // clients
  {
    Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
    Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable> ();

    x->SetAttribute("Min", DoubleValue (0.0));
    x->SetAttribute("Max", DoubleValue (10000.0));

    y->SetAttribute("Min", DoubleValue (0.0));
    y->SetAttribute("Max", DoubleValue (10000.0));

    for (uint32_t i=0; i<lan.ClientN; ++i)
      {
        std::stringstream nodename, position;
        nodename << "client" << i;

        ClientSection client (nodename.str());

        position << x->GetValue() << "," << y->GetValue() <<",1.5";
        client.Position = position.str();
        client.PrintExample();
      }
  }

  // remotes
  {
    for (uint32_t i=0; i<general.RemoteN; ++i)
      {
        std::stringstream nodename;
        nodename << "remote" << i;

        RemoteSection remote (nodename.str());
        remote.Position = "[10000.0,10000.0,25.0]";
        remote.PrintExample();
      }
  }

  // applications
  {
  }
}

} // namespace ppdr-tc
