#ifndef PPDRTCSCENARIOS_H
#define PPDRTCSCENARIOS_H

#include "ppdr-tc-utils.h"

namespace ppdrtc {

class Scenario
{
public:
  Scenario(uint32_t clientN, uint32_t remoteN)
  {
    m_clientN = clientN;
    m_remoteN = remoteN;
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
                               const std::string &connected, uint32_t appN)
  {
    OnOffSection onOff(appN);
    onOff.Port = 80;
    onOff.Protocol = "TCP";
    onOff.DataRate = "20kB/s";
    onOff.OnTime = "ns3::UniformRandomVariable[Min=0.0,Max=1.0]";
    onOff.OffTime = "ns3::UniformRandomVariable[Min=0.0,Max=1.0]";

    onOff.InstalledOn = installed;
    onOff.ConnectedTo = connected;
    onOff.PrintExample ();
  }

  static void PrintFtp (const std::string &installed,
                        const std::string &connected, uint32_t appN)
  {
    BulkSendSection bulk (appN);

    bulk.Port = 21;
    bulk.Protocol = "TCP";

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
};

class DayToDay : public Scenario
{
public:
  DayToDay(uint32_t clientN, uint32_t remoteN) : Scenario (clientN, remoteN)
  {

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
  Planned(uint32_t clientN, uint32_t remoteN) : Scenario (clientN, remoteN)
  {

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
  PlannedInfrastructureless (uint32_t clientN, uint32_t remoteN) :
    Planned (clientN, remoteN)
  {

  }

protected:
  virtual void PrintBackhaul () const;
};

} // namespace ppdrtc
#endif // PPDRTCSCENARIOS_H

