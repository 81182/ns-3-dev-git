#ifndef C2MLRXQUEUE_H
#define C2MLRXQUEUE_H

#include "ns3/drop-tail-queue.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

class C2MLTxQueue;

/**
 * \brief A FIFO packet queue that drops tail-end packets on overflow
 */
class C2MLRxQueue : public DropTailQueue {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  C2MLRxQueue ();

  void SetQDiscManagementFriend (Ptr<C2MLTxQueue> qDisc);
  void SetGwIp (const Ipv4Address &addr)
  {
    m_gwAddr = addr;
  }

protected:
  // From AbstractQueue
  virtual bool DoEnqueue (Ptr<Packet> p);

  Ptr<C2MLTxQueue> m_txQueue;

  Ipv4Address m_gwAddr;
};

} // namespace ns3

#endif // C2MLRXQUEUE_H
