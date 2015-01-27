#ifndef FC2AP_H
#define FC2AP_H

#include "ns3/socket.h"
#include "bwsharingprotocol.h"

#include <list>
#include <map>

namespace ns3 {

/** \brief The FC2AP Allocation protocol
 *
 * FC2AP (Fair Congestion Control Allocation Protocol) is a centralized allocation
 * protocol. It is a stateful protocol, the gateway maintains all the information
 * about the network and the nodes, it knows the states of the nodes, the requested
 * bandwidth and the assigned bandwidth. The nodes don't have to konw their state
 * because the gateway manage everything.
 * This algorithm distinguishes between two classes of nodes, the free ones and
 * the limited ones. Those nodes that are using the bandwidth assigned by the gateway
 * are called "free nodes", those that are asking for less bandwidth and therefore
 * can't use all the assigned bandwidth are called "limited nodes".
 */
class FC2AP : public BwSharingProtocol
{
public:
  FC2AP(uint64_t totalBandwidth);
  ~FC2AP();

  BwStatePair HandleArrive (Ptr<Socket> socket);
  void HandleLeave (Ptr<Socket> socket, NodeState state);
  BwStatePair HandleBwChange (Ptr<Socket> socket,uint64_t bw, NodeState state);

  uint64_t GetBwFromSocket (Ptr<Socket> socket);
  NodeState GetStateFromSocket (Ptr<Socket> socket);

  uint64_t GetGoodBw() const
  {
    return 0;
  }

private:
  struct AllocationData {
     uint64_t bandwidth;
     uint64_t  requestedBandwidth;
     bool isLimited;
  };

  typedef std::map<Ptr<Socket>, AllocationData> FC2APMap;
  typedef FC2APMap::iterator FC2APMapIterator;
  typedef std::pair<Ptr<Socket>, AllocationData> FC2APMapElement;

  /**
   * \brief Redistribute the available bandwidth to the free nodes
   * It has no parameters since in this protocol the gateway is always aware
   * of the information it needs
   */
  void RedistributeBwToFreeNodes ();

  /**
   * \brief Handle a free node asking for less bandwidth and therefore becoming limited
   * \param mIt A reference to the node's data in the allocation map
   * \param bw The new requested bandwidth
   */
  void FreeAskingForLessBw (FC2APMapIterator &mIt, uint64_t bw);

  /**
   * \brief Handle a limited node asking for less bandwidth
   * \param mIt A reference to the node's data in the allocation map
   * \param bw The new requested bandwidth
   */
  void LimitedAskingForLessBw (FC2APMapIterator &mIt, uint64_t bw);

  /**
   * \brief Handle a limited node asking for more bandwidth
   * \param mIt A reference to the node's data in the allocation map
   * \param bw The new requested bandwidth
   */
  void LimitedAskingForMoreBw (FC2APMapIterator &mIt, uint64_t bw);

  int m_freeCount; //!< The total number of free nodes
  int m_limitedCount; //!< The total number of limited nodes
  uint64_t m_sumLimitedBw; //!< The total amount of bandwidth assigned to limited nodes

  FC2APMap m_allocation; //!< The map that maintains the node's data

};

} //namespace ns3

#endif // FC2AP_H
