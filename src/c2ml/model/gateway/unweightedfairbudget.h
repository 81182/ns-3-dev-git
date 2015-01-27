#ifndef UNWEIGHTEDFAIRBUDGET_H
#define UNWEIGHTEDFAIRBUDGET_H

#include "ns3/socket.h"
#include "bwsharingprotocol.h"

#include <list>
#include <map>

namespace ns3 {

/** \brief The Unweighted Fair Budget Allocation protocol
 *
 * This allocation protocol simply assigns equal bandwidth the each node
 * in the network, regardless of their requests.
 * It only uses this formula to calculate the bandwidth: BWi = totBW/nNodes.
 * This is very fast and simple but could lead to not optimal throughput.
 */
class UnweightedFairBudget : public BwSharingProtocol
{
public:
  UnweightedFairBudget(uint64_t totalBandwidth);
  ~UnweightedFairBudget();

  BwStatePair HandleArrive (Ptr<Socket> socket);
  void HandleLeave (Ptr<Socket> socket, NodeState state);
  BwStatePair HandleBwChange (Ptr<Socket> socket,uint64_t bw, NodeState state);

  uint64_t GetBwFromSocket (Ptr<Socket> socket);
  NodeState GetStateFromSocket (Ptr<Socket> socket);

  uint64_t GetGoodBw() const
  {
    return m_totalBandwidth/m_nodes;
  }

private:

  uint64_t m_nodes; //!< The total number of nodes in the network

};

} //namespace ns3

#endif // UNWEIGHTEDFAIRBUDGET_H
