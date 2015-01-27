#include "dybra.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include <stdio.h>
#include <memory>

NS_LOG_COMPONENT_DEFINE ("DyBRA");

namespace ns3 {

DyBRA::DyBRA(uint64_t totalBandwidth)
{
  NS_LOG_FUNCTION (this << totalBandwidth);
  m_Seff = totalBandwidth;
  m_nodes = 0;
  m_nbad = 0;
}

DyBRA::~DyBRA ()
{
  NS_LOG_FUNCTION (this);
}

BwStatePair
DyBRA::HandleArrive (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  m_nodes++;

  BwStatePair ret (m_Seff/(m_nodes-m_nbad), NodeState (NodeState::NODE_GOOD, m_Seff/(m_nodes-m_nbad)));
  return ret;
}

void
DyBRA::HandleLeave (Ptr<Socket> socket, NodeState state)
{
  NS_LOG_FUNCTION (this << socket);
  NS_ASSERT (m_nodes > 0);

  m_nodes--;

  if(state.GetState () == NodeState::NODE_BAD) {
      NS_ASSERT (m_nbad > 0);
      m_nbad--;
      m_Seff += state.GetBw ();
    }
}

BwStatePair
DyBRA::HandleBwChange (Ptr<Socket> socket, uint64_t bw, NodeState state)
{
  NS_LOG_FUNCTION (this << socket << "Using bandwidth:" << bw << " with state: " << state.GetState ());

  if( bw < state.GetBw () && state.GetState () == NodeState::NODE_GOOD) {
      // se un nodo buono chiede meno banda
      return FromGoodToBad (bw);
    }
  else if( bw >= (state.GetBw ()+m_Seff)/(m_nodes-m_nbad+1) && state.GetState () == NodeState::NODE_BAD) {
      // nodo cattivo che chiede più banda, abbastanza da tornare buono
      return FromBadToGood (state);
    }
  else if( bw < state.GetBw () && state.GetState () == NodeState::NODE_BAD) {
      // nodo cattivo che chiede meno banda
      return FromBadToWorse (bw, state);
    }
  else if( bw < (state.GetBw ()+m_Seff)/(m_nodes-m_nbad+1) && state.GetState () == NodeState::NODE_BAD) {
      // nodo cattivo che chiede più banda, ma non abbastanza
      return ImproveFromBad (bw, state);
    }
  else{
      // errore
      NS_LOG_ERROR ("Something weird just happened... Please check code.");
      BwStatePair ret;
      return ret;
    }
}

BwStatePair
DyBRA::FromGoodToBad (uint64_t bw)
{
  NS_LOG_FUNCTION (this << bw);
  NS_ASSERT ((m_Seff - bw) > 0);

  m_mutex.Lock ();

  m_nbad++;
  m_Seff -= bw;

  BwStatePair ret (bw, NodeState (NodeState::NODE_BAD, bw));

  m_mutex.Unlock ();

  NS_ASSERT (m_nbad <= m_nodes);
  return ret;
}

BwStatePair
DyBRA::FromBadToGood (NodeState state)
{
  NS_LOG_FUNCTION (this << state.GetBw () << state.GetState ());
  NS_ASSERT (m_nbad > 0);
  BwStatePair ret;

  m_mutex.Lock ();

  m_nbad--;
  m_Seff += state.GetBw ();

  if(m_nbad == m_nodes)
    {
      ret.SetBw (m_Seff);
      ret.SetNodeState (NodeState (NodeState::NODE_GOOD, m_Seff));
    }
  else
    {
      ret.SetBw (m_Seff/(m_nodes-m_nbad));
      ret.SetNodeState (NodeState (NodeState::NODE_GOOD, m_Seff/(m_nodes-m_nbad)));
    }

  m_mutex.Unlock ();
  NS_ASSERT (m_nbad <= m_nodes);

  return ret;
}

BwStatePair
DyBRA::FromBadToWorse (uint64_t bw, NodeState state)
{
  NS_LOG_FUNCTION (this << bw << state.GetBw () << state.GetState ());

  m_mutex.Lock ();

  m_Seff += (state.GetBw () - bw);

  BwStatePair ret (bw, NodeState (NodeState::NODE_BAD, bw));

  m_mutex.Unlock ();

  NS_ASSERT (m_nbad <= m_nodes);

  return ret;
}

BwStatePair
DyBRA::ImproveFromBad (uint64_t bw, NodeState state)
{
  NS_LOG_FUNCTION (this << bw << state.GetBw () << state.GetState ());
  NS_ASSERT ( (m_Seff -(bw - state.GetBw ())) > 0);

  m_mutex.Lock ();

  m_Seff -= (bw - state.GetBw ());

  BwStatePair ret (bw, NodeState (NodeState::NODE_BAD, bw));

  m_mutex.Unlock ();

  NS_ASSERT (m_nbad <= m_nodes);

  return ret;
}

uint64_t
DyBRA::GetBwFromSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  (void) socket;

  uint64_t ret;

  m_mutex.Lock ();

  if(m_nbad != m_nodes)
    {
      ret = m_Seff/(m_nodes-m_nbad);
    }
  else
    {
      ret = m_Seff;
    }

  m_mutex.Unlock ();

  NS_ASSERT (ret != 0);
  NS_ASSERT (m_nbad <= m_nodes);

  return ret;
}

NodeState
DyBRA::GetStateFromSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  (void) socket;
  NodeState ret;

  m_mutex.Lock ();

  if(m_nbad == m_nodes)
    ret = NodeState (NodeState::NODE_GOOD,m_Seff);
  else
    ret = NodeState (NodeState::NODE_GOOD,m_Seff/(m_nodes-m_nbad));

  m_mutex.Unlock ();

  NS_ASSERT (m_nbad <= m_nodes);

  return ret;
}

} //namespace ns3

