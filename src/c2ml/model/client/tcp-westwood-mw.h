#ifndef TCPWESTWOODMW_H
#define TCPWESTWOODMW_H

#include "ns3/tcp-westwood.h"
#include "ns3/timer.h"

#include "tcp-mw.h"
namespace ns3 {

class CCL45Protocol;

class TcpWestwoodMw : public TcpWestwood,
                      public TcpMw
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  TcpWestwoodMw ();

  void SetBandwidth (uint32_t b);

  // a long long time ago this method was protected ...
  Ptr<CCL45Protocol> GetMwProtocol ();

protected:
  virtual int Connect (const Address &address);
  virtual void CloseAndNotify (void);
  virtual void NewAck (SequenceNumber32 const& seq);
  virtual bool SendPendingData (bool withAck = false);

  uint32_t m_availableBw;
  Time m_minRtt;

};

} // namespace ns3
#endif // TCPWESTWOODMW_H
