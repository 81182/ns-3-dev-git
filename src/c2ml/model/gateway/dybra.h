#ifndef DYBRA_H
#define DYBRA_H

#include "ns3/socket.h"
#include "ns3/system-mutex.h"
#include "bwsharingprotocol.h"

#include <list>
#include <map>

namespace ns3 {

/** \brief The DyBRA Allocation protocol
 *
 * DyBRA (Dynamic Bandwidth Redistribution Algorithm) is a distributed allocation
 * protocol. It is a stateless protocol, the gateway doesn't have to maintain any
 * information about the nodes and their state.
 * The gateway calculates the new allocation values using only parameters sent by
 * the nodes.
 * This algorithm distinguishes between two classes of nodes, the good ones and
 * the bad ones. Those nodes that are using the bandwidth assigned by the gateway
 * are called "good nodes", those that are asking for less bandwidth and therefore
 * can't use all the assigned bandwidth are called "bad nodes".
 */
class DyBRA : public BwSharingProtocol
{
public:
  DyBRA(uint64_t totalBandwidth);
  ~DyBRA();

  BwStatePair HandleArrive (Ptr<Socket> socket);
  void HandleLeave (Ptr<Socket> socket, NodeState state);
  BwStatePair HandleBwChange (Ptr<Socket> socket,uint64_t bw, NodeState state);

  uint64_t GetBwFromSocket (Ptr<Socket> socket);
  NodeState GetStateFromSocket (Ptr<Socket> socket);

  uint64_t GetGoodBw() const
  {
    if(m_nbad != m_nodes)
      {
        return m_Seff/(m_nodes-m_nbad);
      }
    else
      {
        return m_Seff;
      }
  }

private:
  /**
   * \brief Handle a node going from good state to bad state
   * \param bw The new bandwidth requested by the node
   */
  BwStatePair FromGoodToBad (uint64_t bw);

  /**
   * \brief Handle a node going from bad state to good state
   * \param state The current state of the node
   */
  BwStatePair FromBadToGood (NodeState state);

  /**
   * \brief Handle a node keeping its bad state but asking for less bandwidth
   * \param bw The new bandwidth requested by the node
   * \param state The current state of the node
   */
  BwStatePair FromBadToWorse (uint64_t bw, NodeState state);

  /**
   * \brief Handle a node improving from bad state but not enough to be good again
   * \param bw The new bandwidth requested by the node
   * \param state The current state of the node
   */
  BwStatePair ImproveFromBad (uint64_t bw, NodeState state);

  uint32_t m_nodes; //!< The number of nodes managed by the gateway
  uint32_t m_nbad; //!< The number of bad nodes
  uint64_t m_Seff; //!< The total bandwidth used by the good nodes

  SystemMutex m_mutex;

};

} //namespace ns3

#endif // DYBRA_H
