#include "ppdr-tc-scenarios.h"
#include "ppdr-tc-utils.h"
#include "ppdr-tc-statistics.h"

namespace ppdrtc {

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

  backhaul.P2PDataRate = "1Gb/s";
  backhaul.P2PDelay = "10ms";
  backhaul.P2PQueueType = "ns3::CoDelQueue";
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
  gw.TxPower = 40;
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
        onoff5.DataRate = "20kB/s";

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
        onoff5.DataRate = "20kB/s";

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

  // Police : 3 remote with 3 sink, and 12 clients with onoff
  {
    AppSection udp1("app27"), udp2("app28"), udp3("app29");

    OnOffSection *onOff[12] = { new OnOffSection ("app30"), new OnOffSection ("app31"),
                                new OnOffSection ("app32"), new OnOffSection ("app33"),
                                new OnOffSection ("app34"), new OnOffSection ("app35"),
                                new OnOffSection ("app36"), new OnOffSection ("app37"),
                                new OnOffSection ("app38"), new OnOffSection ("app39"),
                                new OnOffSection ("app40"), new OnOffSection ("app41")
                              };

    udp1.Port = udp2.Port = udp3.Port = 443;
    udp1.Protocol = udp2.Protocol = udp3.Protocol = "UDP";
    udp1.ConnectedTo = udp2.ConnectedTo = udp3.ConnectedTo = "any";

    udp1.InstalledOn = "remote6";
    udp1.PrintExample();
    udp2.InstalledOn = "remote7";
    udp2.PrintExample();
    udp3.InstalledOn = "remote8";
    udp3.PrintExample();

    for (uint32_t i=0; i<11; ++i)
      {
        std::stringstream name;

        onOff[i]->AppType = "OnOff";
        onOff[i]->Port = 443;
        onOff[i]->Protocol = "UDP";

        name << "client" << i+14;
        onOff[i]->InstalledOn = name.str();

        if (i%3 == 0)
          {
            onOff[i]->ConnectedTo = "remote6";
          }
        else if (i%3 == 1)
          {
            onOff[i]->ConnectedTo = "remote7";
          }
        else if (i%3 == 2)
          {
            onOff[i]->ConnectedTo = "remote8";
          }

        onOff[i]->PrintExample();
        delete onOff[i];
      }
  }

}

// Infrastructured
void
PrintPlanned ()
{
  GeneralSection general;

  general.Prefix = "ppdr-tc-pla-a-0";
  general.RemoteN = 1;
  general.StopTime = 60;
  general.PrintExample();

  LanSection lan;

  lan.ClientN = 600;
  lan.EnablePcapUserNetwork = false;
  lan.GatewayN = 16;

  std::string NPerG;
  {
    for (uint32_t gw=0; gw<16; ++gw)
      {
        std::stringstream names;
        uint32_t to, from;

        if (gw == 15)
          {
            from = gw*37;
            to = (gw*37)+36+8;
          }
        else
          {
            from = gw*37;
            to = (gw*37)+36;
          }

        names << "[gateway" << gw << ":";

        for (uint32_t client=from; client<to; ++client)
          {
            names << "client" << client << ",";
          }
        names << "client" << to << "];";

        NPerG += names.str();
      }
  }

  lan.NPerG = NPerG;
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

    for (uint32_t i=0; i<lan.GatewayN; ++i)
      {
        std::stringstream name;

        name << "gateway" << i;
        GatewaySection *gw = new GatewaySection (name.str());

        gw->TxPower = 40;
        gw->EnbSrsPeriodicity = 80;
        gw->Position = positions[i];
        gw->EnbAntennaOrientation = orientations[i];
        gw->PrintExample();

        delete gw;
      }
  }

  // clients
  {
    Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
    Ptr<UniformRandomVariable> y = CreateObject<UniformRandomVariable> ();

    x->SetAttribute("Min", DoubleValue (0.0));
    x->SetAttribute("Max", DoubleValue (200.0));

    y->SetAttribute("Min", DoubleValue (0.0));
    y->SetAttribute("Max", DoubleValue (200.0));

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
    for (uint32_t i=0; i<lan.ClientN; ++i)
      {
        std::stringstream sendName, recvName, sendAppName, recvAppName;
        sendAppName << "app" << i;
        recvAppName << "app" << lan.ClientN+i;

        if (i%2 == 0)
          {
            sendName << "client" << i;
            recvName << "remote" << i;
          }
        else
          {
            sendName << "remote" << i;
            recvName << "client" << i;
          }

        if (i<lan.ClientN*90/100)
          {
            OnOffSection onOff (sendAppName.str());
            AppSection recv (recvAppName.str());

            onOff.Port = recv.Port = 443;
            onOff.Protocol = recv.Protocol = "UDP";
            onOff.DataRate = "20kB/s";

            onOff.InstalledOn = sendName.str();
            onOff.ConnectedTo = recvName.str();

            recv.InstalledOn = recvName.str();
            recv.ConnectedTo = "any";

            onOff.PrintExample();
            recv.PrintExample();
          }
        else if (i<lan.ClientN*95/100)
          {
            OnOffSection onOff (sendAppName.str());
            AppSection recv (recvAppName.str());

            onOff.Port = recv.Port = 22;
            onOff.Protocol = recv.Protocol = "TCP";
            onOff.DataRate = "40kB/s";

            onOff.InstalledOn = sendName.str();
            onOff.ConnectedTo = recvName.str();

            recv.InstalledOn = recvName.str();
            recv.ConnectedTo = "any";

            onOff.PrintExample();
            recv.PrintExample();
          }
        else
          {
            BulkSendSection bulk (sendAppName.str());
            AppSection recv (recvAppName.str());

            bulk.Port = recv.Port = 21;
            bulk.Protocol = recv.Protocol = "TCP";

            bulk.InstalledOn = sendName.str();
            bulk.ConnectedTo = recvName.str();

            recv.InstalledOn = recvName.str();
            recv.ConnectedTo = "any";

            bulk.PrintExample();
            recv.PrintExample();
          }
      }
  }


}

} // namespace ppdr-tc
