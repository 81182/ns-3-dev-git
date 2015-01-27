#ifndef TCPDUMB_H
#define TCPDUMB_H

#include "tcp-cubic-mw.h"
#include "tcp-westwood-mw.h"
#include "tcp-newreno-mw.h"
#include "tcp-hybla-mw.h"
#include "tcp-hstcp-mw.h"
#include "tcp-bic-mw.h"

namespace ns3
{

template<class T>
class TcpDumb : public T
{
public:
  TcpDumb() : T () {}

  void SetBandwidth (uint32_t b)
  {
    T::SetBandwidth (b+b/10);
  }

protected:
  virtual TracedValue<uint32_t> & GetCwnd () = 0;
  virtual TracedValue<uint32_t> & GetSSTh () = 0;
  virtual uint32_t & GetAvail () = 0;
  virtual Time GetRtt () = 0;
};

class TcpCubicMwDumb : public TcpDumb <TcpCubicMw>
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

class TcpBicMwDumb : public TcpDumb <TcpBicMw>
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

class TcpHighSpeedMwDumb : public TcpDumb <TcpHighSpeedMw>
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

class TcpHyblaMwDumb : public TcpDumb <TcpHyblaMw>
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

class TcpNewRenoMwDumb : public TcpDumb <TcpNewRenoMw>
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

class TcpWestwoodMwDumb : public TcpDumb <TcpWestwoodMw>
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
#endif // TCPDumb_H
