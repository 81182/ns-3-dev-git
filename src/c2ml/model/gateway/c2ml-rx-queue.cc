#include "ns3/log.h"
#include "c2ml-rx-queue.h"
#include "ns3/ipv4-header.h"
#include "ns3/tcp-header.h"
#include "c2ml-tx-queue.h"
#include "ns3/ppp-header.h"
NS_LOG_COMPONENT_DEFINE ("C2MLRxQueue");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (C2MLRxQueue);

TypeId C2MLRxQueue::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::C2MLRxQueue")
    .SetParent<DropTailQueue> ()
    .AddConstructor<C2MLRxQueue> ()
  ;

  return tid;
}

C2MLRxQueue::C2MLRxQueue() :
  DropTailQueue ()
{
  NS_LOG_FUNCTION (this);
}


bool
C2MLRxQueue::DoEnqueue(Ptr<Packet> p)
{
  Ptr<Packet> pktCopy = p->Copy();

  //NS_LOG_UNCOND ("RX: ");
  //pktCopy->Print(std::cerr);

  Ipv4Header header;
  //PppHeader ppp;
  //pktCopy->RemoveHeader (ppp);
  pktCopy->RemoveHeader(header);
  if (header.GetPayloadSize () < pktCopy->GetSize ())
    {
      pktCopy->RemoveAtEnd (pktCopy->GetSize () - header.GetPayloadSize ());
    }

  TcpHeader tcpHeader;
  pktCopy->RemoveHeader(tcpHeader);

  if (header.GetDestination().IsEqual(m_gwAddr))
    {
      return DropTailQueue::DoEnqueue(p);
    }

  if (tcpHeader.GetFlags() & TcpHeader::ACK)
    {
      NS_LOG_LOGIC ("RX: Allowing src=" << header.GetSource() << " dest=" << header.GetDestination() << " seq= " << tcpHeader.GetSequenceNumber() <<
                      " size=" << pktCopy->GetSize() << " ack=" << tcpHeader.GetAckNumber() << " flags=" << (uint32_t) tcpHeader.GetFlags() <<
                     " at " << Simulator::Now().GetSeconds());
      NS_ASSERT (m_txQueue != 0);
      m_txQueue->TrackRcv(header, tcpHeader);
    }

  return DropTailQueue::DoEnqueue(p);
}

void
C2MLRxQueue::SetQDiscManagementFriend (Ptr<C2MLTxQueue> qDisc)
{
  NS_ASSERT (qDisc != 0);
  m_txQueue = qDisc;
}

} //namespace ns3
