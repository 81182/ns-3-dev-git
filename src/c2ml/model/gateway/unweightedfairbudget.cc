#include "unweightedfairbudget.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include <stdio.h>
#include <memory>


NS_LOG_COMPONENT_DEFINE ("UnweightedFairBudget");

namespace ns3 {

UnweightedFairBudget::UnweightedFairBudget(uint64_t totalBandwidth)
{
  NS_LOG_FUNCTION (this << totalBandwidth);
  m_totalBandwidth = totalBandwidth;
  m_nodes = 0;
}

UnweightedFairBudget::~UnweightedFairBudget ()
{
  NS_LOG_FUNCTION (this);
}

BwStatePair
UnweightedFairBudget::HandleArrive (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  m_nodes++;

  BwStatePair ret (m_totalBandwidth/m_nodes, NodeState (NodeState::NODE_GOOD, m_totalBandwidth/m_nodes));
  return ret;
}

void
UnweightedFairBudget::HandleLeave (Ptr<Socket> socket,NodeState state)
{
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT (m_nodes > 0);

  m_nodes--;
}

BwStatePair
UnweightedFairBudget::HandleBwChange (Ptr<Socket> socket, uint64_t bw, NodeState state)
{
  NS_LOG_FUNCTION (this << socket << "Using bandwidth:" << bw << " with state: " << state.GetState ());
  NS_ASSERT (m_nodes > 0);

  (void) state;

  if(bw >= m_totalBandwidth/m_nodes){
    BwStatePair ret (m_totalBandwidth/m_nodes, NodeState (NodeState::NODE_GOOD, m_totalBandwidth/m_nodes));
    return ret;
  }
  else {
    BwStatePair ret (m_totalBandwidth/m_nodes, NodeState (NodeState::NODE_BAD, m_totalBandwidth/m_nodes));
    return ret;
  }

}

uint64_t
UnweightedFairBudget::GetBwFromSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  (void) socket;
  if(m_nodes == 0)
    return m_totalBandwidth;
  else
    return m_totalBandwidth/m_nodes;
}

NodeState
UnweightedFairBudget::GetStateFromSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  (void) socket;
  if(m_nodes == 0)
    return NodeState (NodeState::NODE_GOOD, m_totalBandwidth);
  else
    return NodeState (NodeState::NODE_GOOD, m_totalBandwidth/m_nodes);
}


} //namespace ns3
