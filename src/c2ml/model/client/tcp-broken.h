#ifndef TCPBROKEN_H
#define TCPBROKEN_H

#include "ns3/tcp-socket-base.h"

namespace ns3 {

class TcpBroken : public TcpSocketBase
{
public:
  static TypeId GetTypeId (void);

  TcpBroken ();
  virtual ~TcpBroken ();

  // From TcpSocketBase
  virtual int Connect (const Address &address);
  virtual int Listen (void);

protected:
  // Implementing ns3::TcpSocket -- Attribute get/set
  virtual void     SetSSThresh (uint32_t threshold);
  virtual uint32_t GetSSThresh (void) const;
  virtual void     SetInitialCwnd (uint32_t cwnd);
  virtual uint32_t GetInitialCwnd (void) const;

  virtual void NewAck (SequenceNumber32 const& rAck);   // Update buffers w.r.t. ACK
  virtual Ptr<TcpSocketBase> Fork (void);   // Call CopyObject<TcpReno> to clone me
  virtual void DupAck (const TcpHeader& t, uint32_t count);
  virtual void Retransmit (void);   // Retransmit timeout
  virtual uint32_t Window (void);   // Return the max possible number of unacked bytes

protected:
  Time                   m_txTime;
  uint32_t               m_bWnd;         //!< Congestion window
  uint32_t               m_ssThresh;     //!< Slow Start Threshold
  uint32_t               m_initialCWnd;  //!< Initial cWnd value

private:
  void Initialize ();
};

} // namespace ns3
#endif // TCPBROKEN_H
