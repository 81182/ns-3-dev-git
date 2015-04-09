#ifndef PPDRTCSCENARIOS_H
#define PPDRTCSCENARIOS_H

#include "ppdr-tc-utils.h"

namespace ppdrtc {

class Scenario
{
public:
  Scenario(const std::string &tcp,
           uint32_t clientN, uint32_t remoteUsers, uint32_t remoteN,
           double ftpProb)
  {
    NS_ASSERT (0<=ftpProb && ftpProb<=1);

    m_clientN = clientN;
    m_remoteN = remoteN;
    m_remoteUsers = remoteUsers;
    m_tcp = tcp;
    m_ftpProb = ftpProb;

    if (m_ftpProb == 0.1)
      {
        m_backgroundT = "10";
      }
    else if (m_ftpProb == 0.2)
      {
        m_backgroundT = "20";
      }
    else if (m_ftpProb == 0.3)
      {
        m_backgroundT = "30";
      }
    else if (m_ftpProb == 0.4)
      {
        m_backgroundT = "40";
      }
    else if (m_ftpProb == 0.5)
      {
        m_backgroundT = "50";
      }
    else
      {
        m_backgroundT = "0";
      }

    if (m_tcp.compare("ns3::TcpCubic") == 0)
      {
        m_tcpT = "Cubic";
      }
    else if (m_tcp.compare("ns3::TcpHybla") == 0)
      {
        m_tcpT = "Hybla";
      }
    else if (m_tcp.compare("ns3::TcpHighSpeed") == 0)
      {
        m_tcpT = "Highspeed";
      }
    else if (m_tcp.compare("ns3::TcpTahoe") == 0)
      {
        m_tcpT = "Tahoe";
      }
    else if (m_tcp.compare("ns3::TcpNewReno") == 0)
      {
        m_tcpT = "NewReno";
      }
    else if (m_tcp.compare("ns3::TcpBic") == 0)
      {
        m_tcpT = "Bic";
      }
    else
      {
        NS_FATAL_ERROR ("TCP " << m_tcp << " not recognized");
      }
  }

  virtual ~Scenario()
  {

  }

  void PrintExample()
  {
    PrintGeneral ();
    PrintLan ();
    PrintBackhaul ();
    PrintLte ();
    PrintStats ();
    PrintGw ();
    PrintClients ();
    PrintRemotes ();
    PrintApps ();
  }

  static void PrintSink(const std::string &prot, const std::string &installed,
                        uint32_t port, uint32_t appN)
  {
    AppSection sink(appN);
    sink.Protocol = prot;
    sink.Port = port;
    sink.InstalledOn = installed;
    sink.ConnectedTo = "any";
    sink.PrintExample();
  }

  static void PrintAudioSink (const std::string &installed, uint32_t appN)
  {
    PrintSink ("UDP", installed, 443, appN);
  }

  static void PrintVideoSink (const std::string &installed, uint32_t appN)
  {
    PrintSink ("UDP", installed, 444, appN);
  }

  static void PrintWebSink (const std::string &installed, uint32_t appN)
  {
    PrintSink ("TCP", installed, 80, appN);
  }

  static void PrintFTPSink (const std::string &installed, uint32_t appN)
  {
    PrintSink ("TCP", installed, 21, appN);
  }

  static void PrintAudioOnOff (const std::string &installed,
                               const std::string &connected, uint32_t appN)
  {
    OnOffSection onOff(appN);
    onOff.Port = 443;
    onOff.Protocol = "UDP";
    onOff.DataRate = "20kB/s";
    onOff.OnTime = "ns3::UniformRandomVariable[Min=0.0,Max=1.0]";
    onOff.OffTime = "ns3::UniformRandomVariable[Min=0.0,Max=1.0]";

    onOff.InstalledOn = installed;
    onOff.ConnectedTo = connected;
    onOff.PrintExample ();
  }

  static void PrintVideoOnOff (const std::string &installed,
                               const std::string &connected, uint32_t appN)
  {
    OnOffSection onOff(appN);
    onOff.Port = 444;
    onOff.Protocol = "UDP";
    onOff.DataRate = "40kB/s";
    onOff.OnTime = "ns3::ConstantRandomVariable[Constant=1.0]";
    onOff.OffTime = "ns3::ConstantRandomVariable[Constant=0.0]";

    onOff.InstalledOn = installed;
    onOff.ConnectedTo = connected;
    onOff.PrintExample ();
  }

  static void PrintWebOnOff (const std::string &installed,
                             const std::string &connected,
                             const std::string &tcp,
                             uint32_t appN)
  {
    OnOffSection onOff(appN);
    onOff.Port = 80;
    onOff.Protocol = "TCP";
    onOff.DataRate = "20kB/s";
    onOff.SocketType = tcp;
    onOff.OnTime = "ns3::UniformRandomVariable[Min=0.0,Max=1.0]";
    onOff.OffTime = "ns3::UniformRandomVariable[Min=0.0,Max=1.0]";

    onOff.InstalledOn = installed;
    onOff.ConnectedTo = connected;
    onOff.PrintExample ();
  }

  static void PrintFtp (const std::string &installed,
                        const std::string &connected,
                        const std::string &tcp,
                        uint32_t appN)
  {
    BulkSendSection bulk (appN);

    bulk.Port = 21;
    bulk.Protocol = "TCP";
    bulk.SocketType = tcp;
    bulk.InstalledOn = installed;
    bulk.ConnectedTo = connected;
    bulk.PrintExample();
  }

protected:
  virtual void PrintGeneral () const = 0;
  virtual void PrintLan () const = 0;
  virtual void PrintBackhaul () const = 0;
  virtual void PrintLte () const = 0;
  virtual void PrintStats () const = 0;
  virtual void PrintGw () const = 0;
  virtual void PrintClients () const = 0;
  virtual void PrintRemotes () const = 0;
  virtual void PrintApps () const;

  uint32_t m_clientN;
  uint32_t m_remoteN;
  uint32_t m_remoteUsers;
  double m_ftpProb;

  std::string m_tcp;

  std::string m_tcpT;
  std::string m_backgroundT;
  std::string m_prefix;
};


class DayToDay : public Scenario
{
public:
  DayToDay(const std::string &tcp,
           uint32_t clientN, uint32_t remoteUsers, uint32_t remoteN,
           double ftpProb) :
    Scenario (tcp, clientN, remoteUsers, remoteN, ftpProb)
  {
    std::stringstream ss;
    ss << m_clientN;

    m_prefix = "ppdrtc-d2d-"+ss.str()+"-"+m_tcpT+"-"+m_backgroundT;
  }

protected:
  virtual void PrintGeneral () const;
  virtual void PrintLan () const;
  virtual void PrintBackhaul () const;
  virtual void PrintLte () const;
  virtual void PrintStats () const;
  virtual void PrintGw () const;
  virtual void PrintClients () const;
  virtual void PrintRemotes () const;
};

class Planned : public Scenario
{
public:
  Planned(const std::string &tcp,
          uint32_t clientN, uint32_t remoteUsers, uint32_t remoteN,
          double ftpProb) :
    Scenario (tcp, clientN, remoteUsers, remoteN, ftpProb)
  {
    std::stringstream ss;
    ss << m_clientN;

    m_prefix = "ppdrtc-pla-wired-"+ss.str()+"-"+m_tcpT+"-"+m_backgroundT;
  }

protected:
  virtual void PrintGeneral () const;
  virtual void PrintLan () const;
  virtual void PrintBackhaul () const;
  virtual void PrintLte () const;
  virtual void PrintStats () const;
  virtual void PrintGw () const;
  virtual void PrintClients () const;
  virtual void PrintRemotes () const;
};

class PlannedInfrastructureless : public Planned
{
public:
  PlannedInfrastructureless (const std::string &tcp,
                             uint32_t clientN, uint32_t remoteUsers, uint32_t remoteN,
                             double ftpProb) :
    Planned (tcp, clientN, remoteUsers, remoteN, ftpProb)
  {
    std::stringstream ss;
    ss << m_clientN;

    m_prefix = "ppdrtc-pla-sat-"+ss.str()+"-"+m_tcpT+"-"+m_backgroundT;
  }

protected:
  virtual void PrintBackhaul () const;
};

class Unplanned : public Scenario
{
public:
  Unplanned(const std::string &tcp,
            uint32_t clientN, uint32_t remoteUsers, uint32_t remoteN,
            double ftpProb) :
    Scenario (tcp, clientN, remoteUsers, remoteN, ftpProb)
  {
    std::stringstream ss;
    ss << m_clientN;

    m_prefix = "ppdrtc-unp-"+ss.str()+"-"+m_tcpT+"-"+m_backgroundT;
  }

protected:
  virtual void PrintGeneral () const;
  virtual void PrintLan () const;
  virtual void PrintBackhaul () const;
  virtual void PrintLte () const;
  virtual void PrintStats () const;
  virtual void PrintGw () const;
  virtual void PrintClients () const;
  virtual void PrintRemotes () const;
};

} // namespace ppdrtc
#endif // PPDRTCSCENARIOS_H

