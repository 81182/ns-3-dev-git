#include "c2ml-struct.h"

namespace ns3
{

TypeId
MWHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MWHeader")
    .SetParent<Header> ()
    .AddConstructor<MWHeader> ()
  ;
  return tid;
}

TypeId
MWHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint32_t
MWHeader::GetSerializedSize (void) const
{
  return 27;
}

void
MWHeader::Serialize (Buffer::Iterator start) const
{
  // The 2 byte-constant
  start.WriteU8 (0xde);
  start.WriteU8 (0xad);
  // The header type
  start.WriteU8 (m_type);
  // The data.
  uint64_t bw = m_data.GetBw ();
  start.WriteHtonU64 (bw);
  NodeState tmp=m_data.GetNodeState ();
  uint64_t oldBw =tmp.GetBw ();
  NodeState::State state=tmp.GetState ();
  start.WriteU32 (state);
  start.WriteHtonU64 (oldBw);

  start.WriteU32(m_ts);
}

uint32_t
MWHeader::Deserialize (Buffer::Iterator start)
{
  uint8_t tmp;
  tmp = start.ReadU8 ();
  NS_ASSERT (tmp == 0xde);
  tmp = start.ReadU8 ();
  NS_ASSERT (tmp == 0xad);

  m_type = (HeaderType) start.ReadU8 ();
  m_data.SetBw (start.ReadNtohU64 ());
  NodeState::State state = (NodeState::State) start.ReadU32 ();
  uint64_t bw=start.ReadNtohU64 ();
  m_data.SetNodeState (NodeState (state,bw));

  m_ts = start.ReadU32 ();

  return 27;   // the number of bytes consumed.
}

void
MWHeader::Print (std::ostream &os) const
{
  os << "type=" << m_type << " data=" << m_data.GetBw ();
}

void
MWHeader::SetData (BwStatePair data)
{
  m_data = data;
}

BwStatePair MWHeader::GetData (void) const
{
  return m_data;
}

void
MWHeader::SetTs (uint32_t ts)
{
  m_ts = ts;
}

uint32_t
MWHeader::GetTs () const
{
  return m_ts;
}

void
MWHeader::SetHeaderType (MWHeader::HeaderType type)
{
  m_type = type;
}

MWHeader::HeaderType
MWHeader::GetHeaderType (void) const
{
  return m_type;
}

} // namespace ns3
