/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Natale Patriciello, UNIMORE, <natale.patriciello@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "c2ml-gateway.h"
#include "fc2ap.h"
#include "unweightedfairbudget.h"
#include "dybra.h"

#include "ns3/log.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/c2ml-struct.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4-l3-protocol.h"
#include "ns3/pointer.h"
#include "ns3/names.h"
#include "ns3/c2ml-rx-queue.h"
#include "ns3/c2ml-tx-queue.h"

#include <list>
#include <map>

NS_LOG_COMPONENT_DEFINE ("C2MLGateway");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (C2MLGateway);

typedef std::list<Ptr<Socket> >::iterator SocketIterator;

TypeId
C2MLGateway::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::C2MLGateway")
    .SetParent<Application> ()
    .AddConstructor<C2MLGateway> ()
    .AddAttribute ("Local", "The Address on which to Bind the rx socket.",
                   AddressValue (),
                   MakeAddressAccessor (&C2MLGateway::m_local),
                   MakeAddressChecker ())
    .AddAttribute ("Protocol", "The type id of the protocol to use for the rx socket.",
                   TypeIdValue (TcpSocketFactory::GetTypeId ()),
                   MakeTypeIdAccessor (&C2MLGateway::m_tid),
                   MakeTypeIdChecker ())
    .AddAttribute ("Bandwidth", "Slow link bandwidth",
                   UintegerValue (15000),
                   MakeUintegerAccessor (&C2MLGateway::m_totalBandwidth),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Mode", "Bandwidth allocation protocol used by the gateway.",
                   UintegerValue (UNWEIGHTED),
                   MakeUintegerAccessor (&C2MLGateway::m_mode),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("AQM", "Enable or disable C2MLQueue",
                   BooleanValue (false),
                   MakeBooleanAccessor (&C2MLGateway::m_aqmEnabled),
                   MakeBooleanChecker ())
    .AddTraceSource ("Rx", "A packet has been received",
                     MakeTraceSourceAccessor (&C2MLGateway::m_rxTrace))
  ;
  return tid;
}

C2MLGateway::C2MLGateway ()
{
  m_socket = 0;
  m_totalRx = 0;
  m_clientNumber = 0;
  m_timerStarted = false;
  m_timestamp = 0;
}

C2MLGateway::~C2MLGateway ()
{
  NS_LOG_FUNCTION (this);
}

Ptr<Socket>
C2MLGateway::GetListeningSocket (void) const
{
  return m_socket;
}

std::list<Ptr<Socket> >
C2MLGateway::GetAcceptedSockets (void) const
{
  return m_socketList;
}

void C2MLGateway::DoDispose (void)
{
  m_socket = 0;
  m_socketList.clear ();

  // chain up
  Application::DoDispose ();
}

// Application Methods
void C2MLGateway::StartApplication ()    // Called at time specified by Start
{
  // Create the socket if not already
  if (!m_socket)
    {
      m_socket = Socket::CreateSocket (GetNode (), m_tid);
      m_socket->Bind (m_local);
      int res = m_socket->Listen ();
      if (res != 0)
        {
          NS_FATAL_ERROR (m_socket->GetErrno ());
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&C2MLGateway::HandleRead, this));
  m_socket->SetAcceptCallback (
    MakeNullCallback <bool, Ptr<Socket>, const ns3::Address&> (),
    MakeCallback (&C2MLGateway::HandleAccept, this));
  m_socket->SetCloseCallbacks (
    MakeCallback (&C2MLGateway::HandlePeerClose, this),
    MakeCallback (&C2MLGateway::HandlePeerError, this));

  if(m_mode == FAIR)
    {
      NS_LOG_DEBUG (this << "Gateway working in mode: FC2AP");
      m_bwAllocator = CreateObject<FC2AP> (m_totalBandwidth);
    }
  else if(m_mode == UNWEIGHTED)
    {
      NS_LOG_DEBUG (this << "Gateway working in mode: Unweighted Fair Budget");
      m_bwAllocator = CreateObject<UnweightedFairBudget> (m_totalBandwidth);
    }
  else if(m_mode == DYBRA)
    {
      NS_LOG_DEBUG (this << "Gateway working in mode: DyBRA");
      m_bwAllocator = CreateObject<DyBRA> (m_totalBandwidth);
    }
  else
    {
      NS_FATAL_ERROR ("Couldn't find an allocation protocol... Go coding!");
    }

  if (m_aqmEnabled)
    {
      Ptr<NetDevice> sat0 = Names::Find<NetDevice> (Names::FindName (m_node) +"/sat0");
      Ptr<Ipv4L3Protocol> ipv4 = m_node->GetObject<Ipv4L3Protocol> ();

      Ptr<C2MLRxQueue> rxQueue = DynamicCast<C2MLRxQueue> (ipv4->GetInputQueue(sat0));
      NS_ASSERT (rxQueue != 0);

      Ptr<C2MLTxQueue> txQueue = DynamicCast<C2MLTxQueue> (ipv4->GetOutputQueue(sat0));
      NS_ASSERT (txQueue != 0);

      m_queue = txQueue;

      Address addr;
      m_socket->GetSockName (addr);
      InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (addr);
      rxQueue->SetGwIp(iaddr.GetIpv4());
    }

  NS_LOG_DEBUG (this << m_clientNumber);

}

void C2MLGateway::StopApplication ()     // Called at time specified by Stop
{
  while (!m_socketList.empty ()) //these are accepted sockets, close them
    {
      Ptr<Socket> acceptedSocket = m_socketList.front ();
      m_socketList.pop_front ();
      acceptedSocket->Close ();
    }
  if (m_socket)
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
C2MLGateway::NotifyClientsForBw ()
{
  m_timerStarted = false;

  NS_ASSERT (m_socketList.size () == m_clientNumber);

  NS_LOG_DEBUG ("Notifying clients for bw changed");

  for(SocketIterator it = m_socketList.begin (); it != m_socketList.end (); ++it)
    {
      Ptr<Socket> socket= *it;

      BwStatePair pl;
      pl.SetBw ( m_bwAllocator->GetBwFromSocket (socket));
      pl.SetNodeState (m_bwAllocator->GetStateFromSocket (socket));

      Ptr<Packet> a = CreatePacket (MWHeader::HEADER_ALLOWED, pl);
      socket->Send (a);
    }
}

void C2MLGateway::StartTimer (Ptr<Socket> socket)
{
  m_timer.SetDelay (MilliSeconds (1));
  m_timer.SetFunction (&C2MLGateway::NotifyClientsForBw, this);

  m_timer.Schedule ();

  m_timerStarted = true;
}

void C2MLGateway::HandleRead (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (InetSocketAddress::IsMatchingType (from))
        {
          MWHeader h;
          packet->RemoveHeader (h);

          switch (h.GetHeaderType ())
            {
            case MWHeader::HEADER_HELLO:
              {
                ++m_clientNumber;
                BwStatePair newNodeBwStatePair = m_bwAllocator->HandleArrive (socket);

                NS_LOG_DEBUG ("Socket Node: " << socket <<
                              " assigned: " <<
                              newNodeBwStatePair.GetBw () << ", " <<
                              newNodeBwStatePair.GetNodeState ().GetState ()<<
                              "(ACK_HELLO)");

                if (m_aqmEnabled)
                  {
                    InetSocketAddress transport = InetSocketAddress::ConvertFrom (from);
                    Ipv4Address ipv4 = transport.GetIpv4 ();
                    m_queue->SetAllowedIp(ipv4);
                    m_queue->SetGoodBw(m_bwAllocator->GetGoodBw());
                  }

                Ptr<Packet> ah = CreatePacket (MWHeader::HEADER_ACK_HELLO, newNodeBwStatePair);
                socket->Send (ah);

                NS_ASSERT (m_clientNumber > 0);
                NS_LOG_DEBUG ("New client, I have now client=" << m_clientNumber);

                if (!m_timerStarted)
                  {
                    StartTimer (socket);
                  }
                break;
              }
            case MWHeader::HEADER_BYE:
              {
                --m_clientNumber;
                NS_ASSERT (m_clientNumber >= 0);
                NS_LOG_DEBUG ("Bye client, I have now client=" << m_clientNumber);

                HandlePeerClose (socket);

                NodeState st = h.GetData ().GetNodeState ();
                m_bwAllocator->HandleLeave (socket, st);

                if (m_aqmEnabled)
                  {
                    InetSocketAddress transport = InetSocketAddress::ConvertFrom (from);
                    Ipv4Address ipv4 = transport.GetIpv4 ();
                    m_queue->SetAllowedIp(ipv4);

                    m_queue->SetGoodBw (m_bwAllocator->GetGoodBw());
                  }

                NotifyClientsForBw ();

                break;
              }
            case MWHeader::HEADER_USED_SIZE:
              {
                NS_LOG_DEBUG ("Socket for node: " << socket <<
                              " asks for: " << h.GetData ().GetBw ());
                NS_ASSERT (h.GetData ().GetBw () > 0);
                NS_ASSERT (m_clientNumber > 0);

                NodeState st = h.GetData ().GetNodeState ();
                uint64_t bw = h.GetData ().GetBw ();

                BwStatePair newNodeBwStatePair = m_bwAllocator->HandleBwChange (socket, bw, st);

                NS_LOG_DEBUG ("Socket Node: " << socket <<
                              " assigned bw=" <<
                              newNodeBwStatePair.GetBw () <<  ", state=" <<
                              newNodeBwStatePair.GetNodeState ().GetState () <<
                              "(ACK_USED)");

                if (m_aqmEnabled)
                  {
                    m_queue->SetGoodBw(m_bwAllocator->GetGoodBw());
                  }

                Ptr<Packet> au = CreatePacket (MWHeader::HEADER_ACK_USED,
                                               newNodeBwStatePair);
                socket->Send (au);

                NotifyClientsForBw ();

                break;
              }
            default:
              NS_FATAL_ERROR ("Header type not recognized");
            }
        }

      m_rxTrace (packet, from);
    }
}


void C2MLGateway::HandlePeerClose (Ptr<Socket> socket)
{
  m_socketList.remove (socket);
}

void C2MLGateway::HandlePeerError (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
}


void C2MLGateway::HandleAccept (Ptr<Socket> s, const Address& from)
{
  s->SetRecvCallback (MakeCallback (&C2MLGateway::HandleRead, this));
  m_socketList.push_back (s);
}

Ptr<Packet>
C2MLGateway::CreatePacket (MWHeader::HeaderType header, BwStatePair payload)
{
  NS_ASSERT (payload.GetBw () > 0);
  NS_ASSERT (payload.GetNodeState ().GetBw () > 0);

  Ptr<Packet> p = Create<Packet> ();

  MWHeader h;
  h.SetHeaderType (header);
  h.SetData (payload);
  h.SetTs(m_timestamp);
  ++m_timestamp;
  p->AddHeader (h);

  return p;
}

} // Namespace ns3
