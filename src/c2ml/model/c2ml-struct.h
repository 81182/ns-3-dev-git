#ifndef C2MLSTRUCT_H
#define C2MLSTRUCT_H

#include "ns3/header.h"
#include "ns3/data-rate.h"

namespace ns3 {

// NodeState Definition -------------------------------------------------------

/** \brief NodeState class
 *
 * The NodeState class models the state object used by the allocation protcols.
 * It holds the bandwidth assigned to the node and the node's state (good/bad).
 * All this data is exclusively managed by the gateway .
 */
class NodeState
{
public:
  enum State
    {
      NODE_BAD,
      NODE_GOOD
    };

  NodeState(State state, uint64_t bw)
  {
    m_state = state;
    m_bw = bw;
  }

  NodeState(){}

  ~NodeState(){}

  State GetState () { return m_state; }
  uint64_t GetBw () { return m_bw; }
  void SetState (State state) { m_state = state; }
  void SetBw (uint64_t bw) { m_bw = bw; }

private:
  State       m_state; //!< The state assigned to the node
  uint64_t    m_bw; //!< The bandwidth assigned to the node
};

// BwStatePair Definition -----------------------------------------------------

/** \brief BwStatePair class
 *
 * This class is used to represent the payload of a C2ML packet.
 * It holds a NodeState and a bandwidth.
 */
class BwStatePair{
public:
  BwStatePair(uint64_t bw, NodeState state)
    {
      m_bw = bw;
      m_nodeState = state;
    }

  BwStatePair(){}

  ~BwStatePair(){}

  uint64_t GetBw () const { return m_bw; }
  NodeState GetNodeState () const { return m_nodeState; }

  void SetBw (uint64_t bw) { m_bw = bw; }
  void SetNodeState (NodeState state) { m_nodeState = state; }

private:
  uint64_t m_bw; //!< The bandwidth
  NodeState m_nodeState; //!< The node's state
};

/**
 * \brief Congestion Control Layer Protocol base messages
 *
 * All message are represented in this class. This is for brevity; a beautiful
 * way to do this is to have an header for each type of message.
 *
 * Waiting NAT to fix this, I pray you: DO NOT READ THE STATE when you
 * aren't supposed to. In particular:
 *
 * - ACK_HELLO do not have BW parameter (only blob), so do not read into StatePair
 * - AVAIL_BW has BW parameter, so you can read into StatePair
 */
class MWHeader : public Header
{
public:
  // must be implemented to become a valid new header.
  static TypeId GetTypeId (void);
  virtual TypeId GetInstanceTypeId (void) const;

  enum HeaderType
    {
      HEADER_HELLO        = 0,
      HEADER_BYE          = 2,
      HEADER_ACK_HELLO    = 4,
      HEADER_ACK_USED     = 8,
      HEADER_USED_SIZE    = 16,
      HEADER_ALLOWED      = 32
    };

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  virtual void Print (std::ostream &os) const;

  // allow protocol-specific access to the header data.
  void SetData (BwStatePair data);
  BwStatePair GetData (void) const;

  void SetTs (uint32_t ts);
  uint32_t GetTs () const;

  void SetHeaderType (HeaderType type);
  HeaderType GetHeaderType (void) const;

private:
  HeaderType m_type;
  BwStatePair m_data;
  uint32_t m_ts;
};

} // namespace ns3

#endif // C2MLSTRUCT_H
