#ifndef TCPATTACKER_H
#define TCPATTACKER_H

#include "tcp-cubic-mw.h"
#include "tcp-westwood-mw.h"
#include "tcp-newreno-mw.h"
#include "tcp-hybla-mw.h"
#include "tcp-hstcp-mw.h"
#include "tcp-bic-mw.h"
#include "ns3/log.h"

namespace ns3
{

template<class T>
class TcpAttacker : public T
{
public:
  TcpAttacker() : T () {}

  void SetBandwidth (uint32_t b)
  {
    (void) b;
    T::SetBandwidth (2500000);
  }

protected:
  virtual void NewAck (SequenceNumber32 const& seq)
  {
    T::NewAck (seq);
    uint32_t cWnd = GetCwnd ();
    (void) cWnd;
    Time rtt = GetRtt ();
    uint32_t avail = GetAvail();

    cWnd = rtt.GetSeconds() * avail;
  }

  virtual void Retransmit ()
  {
    TcpSocketBase::Retransmit();
  }

  virtual void DupAck (const TcpHeader& tcpHeader, uint32_t count)
  {
    (void) tcpHeader;
    (void) count;

    uint32_t cWnd = GetCwnd ();
    (void) cWnd;
    Time rtt = GetRtt ();
    uint32_t avail = GetAvail();

    cWnd = rtt.GetSeconds() * avail;
  }

  virtual TracedValue<uint32_t> & GetCwnd () = 0;
  virtual TracedValue<uint32_t> & GetSSTh () = 0;
  virtual uint32_t & GetAvail () = 0;
  virtual Time GetRtt () = 0;
};

class TcpCubicMwAttacker : public TcpAttacker <TcpCubicMw>
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

protected:
  TracedValue<uint32_t> & GetCwnd () { return m_cWnd; }
  TracedValue<uint32_t> & GetSSTh () { return m_ssThresh; }
  Time GetRtt ()        { return m_lastRtt; }
  uint32_t & GetAvail () { return m_availableBw; }
};

class TcpBicMwAttacker : public TcpAttacker <TcpBicMw>
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

protected:
  TracedValue<uint32_t> & GetCwnd () { return m_cWnd; }
  TracedValue<uint32_t> & GetSSTh () { return m_ssThresh; }
  Time GetRtt ()        { return m_lastRtt; }
  uint32_t & GetAvail () { return m_availableBw; }
};

class TcpHighSpeedMwAttacker : public TcpAttacker <TcpHighSpeedMw>
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

protected:
  TracedValue<uint32_t> & GetCwnd () { return m_cWnd; }
  TracedValue<uint32_t> & GetSSTh () { return m_ssThresh; }
  Time GetRtt ()        { return m_lastRtt; }
  uint32_t & GetAvail () { return m_availableBw; }
};

class TcpHyblaMwAttacker : public TcpAttacker <TcpHyblaMw>
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

protected:
  TracedValue<uint32_t> & GetCwnd () { return m_cWnd; }
  TracedValue<uint32_t> & GetSSTh () { return m_ssThresh; }
  Time GetRtt ()        { return m_lastRtt; }
  uint32_t & GetAvail () { return m_availableBw; }
};

class TcpNewRenoMwAttacker : public TcpAttacker <TcpNewRenoMw>
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

protected:
  TracedValue<uint32_t> & GetCwnd () { return m_cWnd; }
  TracedValue<uint32_t> & GetSSTh () { return m_ssThresh; }
  Time GetRtt ()        { return m_lastRtt; }
  uint32_t & GetAvail () { return m_availableBw; }
};

class TcpWestwoodMwAttacker : public TcpAttacker <TcpWestwoodMw>
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

protected:
  TracedValue<uint32_t> & GetCwnd () { return m_cWnd; }
  TracedValue<uint32_t> & GetSSTh () { return m_ssThresh; }
  Time GetRtt ()        { return m_lastRtt; }
  uint32_t & GetAvail () { return m_availableBw; }
};

} // namespace ns3
#endif // TCPATTACKER_H
