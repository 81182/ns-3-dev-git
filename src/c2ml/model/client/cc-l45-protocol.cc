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

#define NS_LOG_APPEND_CONTEXT \
  if (m_node) \
    { std::clog << Simulator::Now ().GetSeconds () << \
        " [node " << m_node->GetId () << "] "; }

#include "cc-l45-protocol.h"
#include "tcp-noordwijk-mw.h"
#include "tcp-mw.h"
#include "ns3/node.h"
#include "ns3/tcp-newreno.h"

#include "ns3/log.h"

#include <climits>

NS_LOG_COMPONENT_DEFINE ("CCL45Protocol");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (CCL45Protocol);

typedef std::vector<TcpMw * >::iterator SocketIterator;

TypeId
CCL45Protocol::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::CCL45Protocol")
    .SetParent<TcpL4Protocol> ()
    .AddConstructor<CCL45Protocol> ()
    .AddAttribute ("Remote",
                   "Address of Middleware gateway.",
                   AddressValue (),
                   MakeAddressAccessor (&CCL45Protocol::m_gateway),
                   MakeAddressChecker ())
    .AddAttribute ("Local",
                   "Listen address of the local middleware part.",
                   AddressValue (),
                   MakeAddressAccessor (&CCL45Protocol::m_local),
                   MakeAddressChecker ())
  ;
  return tid;
}

CCL45Protocol::CCL45Protocol ()
  : TcpL4Protocol ()
{
  m_connected = false;
  m_connectionsOpened = 0;
  m_availableBandwidth = 0;
  m_gatewaySocket = 0;
  m_lastCsi = ULONG_MAX;
  m_state = NORMAL;
  m_lastTs = 0;
}

Ptr<Socket>
CCL45Protocol::CreateSocket (void)
{
  Ptr<Socket> socket = TcpL4Protocol::CreateSocket (m_socketTypeId);

  TcpMw *p = dynamic_cast<TcpMw*> (GetPointer (socket));

  if (p == 0)
    {
      std::string typeId = m_socketTypeId.GetName() + "Mw";
      return CreateSocket (TypeId::LookupByName(typeId));
    }

  m_sockets.push_back (p);

  return socket;
}

Ptr<Socket>
CCL45Protocol::CreateSocket (TypeId socketTypeId)
{
  Ptr<Socket> socket = TcpL4Protocol::CreateSocket (socketTypeId);

  TcpMw *p = dynamic_cast<TcpMw*> (GetPointer (socket));

  if (p == 0)
    {
      NS_FATAL_ERROR (m_socketTypeId.GetName () << " not compatible with c2ml, " <<
                      "try to use " << m_socketTypeId.GetName () <<
                      "Mw if you coded it");
    }

  m_sockets.push_back (p);

  return socket;
}

void
CCL45Protocol::HandleRead (Ptr<Socket> socket)
{
  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom (from)) || ! m_packetsReceived.empty())
    {
      if (packet == 0)
        {
          packet = m_packetsReceived.front ();
          m_packetsReceived.pop_front ();
        }

      if (packet->GetSize () == 0)
        { //EOF
          break;
        }

      if (InetSocketAddress::IsMatchingType (from))
        {
          MWHeader h;
          packet->RemoveHeader (h);

          if (h.GetHeaderType () == MWHeader::HEADER_ACK_HELLO)
            {
              NS_ASSERT (m_state == WAITING_ACK);
              AckHelloReceived (h);

              m_state = NORMAL;
            }
          else if(h.GetHeaderType () == MWHeader::HEADER_ACK_USED)
            {
              NS_ASSERT (m_state == WAITING_ACK);
              AckUsedReceived (h);

              m_state = NORMAL;
            }
          else if(h.GetHeaderType () == MWHeader::HEADER_ALLOWED)
            {
              if (m_state == WAITING_ACK)
                {
                  NS_LOG_DEBUG ("Reordering packet as we're waiting for an ACK");
                  m_packetsReceived.push_back (packet);
                }
              else
                {
                  AllowedReceived (h);
                }
            }
          else
            {
              NS_FATAL_ERROR ("Header type received mismatch");
            }
        }
      else if (Inet6SocketAddress::IsMatchingType (from))
        {
          NS_FATAL_ERROR ("C2ML and Ipv6 ?!? You're a pioneer");
        }
    }
}

void
CCL45Protocol::AckHelloReceived (const MWHeader& h)
{
  NS_LOG_DEBUG ("Received a ACK_HELLO -> Bw: " <<
                h.GetData ().GetBw () << " State: " <<
                h.GetData ().GetNodeState ().GetState ());

  NS_ASSERT (h.GetData ().GetBw () > 0);
  NS_ASSERT (h.GetData ().GetNodeState ().GetBw () > 0);

  if (h.GetTs () >= m_lastTs)
    {
      // I am supposed to read blob.
      m_recvState = h.GetData ().GetNodeState ();

      // But not to read inside blob
      //m_availableBandwidth = h.GetData ().GetBw ();
      //HandleBwChanged ();
    }
  else
    {
      NS_LOG_DEBUG ("Retrieved an OLD packet.. discarding");
    }
}

void
CCL45Protocol::AckUsedReceived (const MWHeader& h)
{
  NS_LOG_DEBUG ("Received a ACK_USED -> Bw: " <<
                h.GetData ().GetBw () << " State: " <<
                h.GetData ().GetNodeState ().GetState ());

  NS_ASSERT (h.GetData ().GetBw () > 0);
  NS_ASSERT (h.GetData ().GetNodeState ().GetBw () > 0);

  if (h.GetTs () >= m_lastTs)
    {
      // I am supposed to read blob.
      m_recvState = h.GetData ().GetNodeState ();

      // But not to read inside blob
      //m_availableBandwidth = h.GetData ().GetBw ();
      //HandleBwChanged ();
    }
  else
    {
      NS_LOG_DEBUG ("Retrieved an OLD packet.. discarding");
    }
}

void
CCL45Protocol::AllowedReceived (const MWHeader& h)
{
  NS_LOG_DEBUG ("Received a ALLOWED -> Bw: " <<
                h.GetData ().GetBw () << "  State: " <<
                h.GetData ().GetNodeState ().GetState ());
  NS_ASSERT (h.GetData ().GetBw () > 0);
  NS_ASSERT (h.GetData ().GetNodeState ().GetBw () > 0);

  if (h.GetTs () < m_lastTs)
    {
      NS_LOG_DEBUG ("Retrieved an OLD packet.. discarding");
      return;
    }

  if(m_recvState.GetState () == NodeState::NODE_GOOD)
    { // se sono buono aggiorno lo stato e la banda threshold
      m_availableBandwidth = h.GetData ().GetBw ();
      m_recvState = h.GetData ().GetNodeState ();
      HandleBwChanged ();
      if(h.GetData ().GetBw () > m_lastCsi)
        {
          SendUsedPacket (m_lastCsi);
        }
    }
  else
    {   // se sono cattivo
      if(h.GetData ().GetBw () < m_recvState.GetBw ())
        {
          // se divento buono devo farlo capire al gateway per fargli sapere che la mia banda deve sommarsi a Seff.
          // quindi glielo comunico con un pacchetto used, che farà capire tutto al gateway.
          SendUsedPacket (m_availableBandwidth);
        }
    }
}

void
CCL45Protocol::CsiBwChange (uint64_t bw)
{
  NS_LOG_UNCOND (this << " " << m_node->GetId() << " Channel State Change with new Bw: " << bw);

  if (bw < m_availableBandwidth)
    {
      m_availableBandwidth = bw;
    }

  if(m_gatewaySocket == 0)
    {
      NS_LOG_ERROR (this << "Couldn't find the gateway socket... :(");
    }
  else
    {
      SendUsedPacket (bw);
    }

  HandleBwChanged();

  m_lastCsi = bw;
}

void
CCL45Protocol::NotifyConnectionClosed (TcpMw * socket)
{
  (void) socket;

  /* Here, our TcpMw could call us multiple time. Please, code a way
 * to ignore only multiple call from the same socket, because in this
 * way one badly coded socket could lead to get m_connectionsOpened = 0
 */
  if (m_connectionsOpened == 0)
    {
      return;
    }

  NS_ASSERT (m_connectionsOpened >= 1);
  --m_connectionsOpened;

  if (m_connectionsOpened == 0)
    {
      SendByePacket ();

      m_gatewaySocket->Close ();
      m_gatewaySocket = 0;
    }
  else
    {
      HandleBwChanged ();
    }
}

void
CCL45Protocol::NotifyConnectionOpened (TcpMw * socket)
{
  (void) socket;

  if (m_gatewaySocket == 0)
    {
      m_gatewaySocket = TcpL4Protocol::CreateSocket (TcpNewReno::GetTypeId ());

      int res = m_gatewaySocket->Bind (m_local);
      res |= m_gatewaySocket->Connect (m_gateway);

      if (res != 0)
        {
          NS_FATAL_ERROR ("Can't connect to the gateway " << m_gateway << " :-/ error=" << res);
        }

      m_gatewaySocket->SetRecvCallback (MakeCallback (&CCL45Protocol::HandleRead, this));
      m_gatewaySocket->SetConnectCallback (
        MakeCallback (&CCL45Protocol::ConnectionSucceeded, this),
        MakeCallback (&CCL45Protocol::ConnectionFailed, this));
    }

  ++m_connectionsOpened;
  NS_ASSERT (m_connectionsOpened > 0);

  HandleBwChanged ();
}

void
CCL45Protocol::HandleBwChanged ()
{
  uint64_t streamBandwidth = CalcStreamBandwidth ();
  if (streamBandwidth == 0)
    {
      return;
    }

  TcpMw *s;

  for (SocketIterator it = m_sockets.begin (); it != m_sockets.end (); ++it)
    {
      s = dynamic_cast<TcpMw*> (*it);

      if (s == 0)
        {
          NS_LOG_WARN ("Socket isn't of TcpMw type");
          continue;
        }

      //NS_LOG_UNCOND (m_node->GetId() << " Set Bw " << Simulator::Now().GetSeconds() << " s " << streamBandwidth);
      s->SetBandwidth (streamBandwidth);
    }
}

uint64_t CCL45Protocol::CalcStreamBandwidth ()
{
  if (m_availableBandwidth > 0 && m_connectionsOpened > 0)
    {
      uint64_t streamBandwidth = m_availableBandwidth / m_connectionsOpened;
      return streamBandwidth;
    }

  return 0;
}

void
CCL45Protocol::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_LOGIC ("CCL45Protocol Connection succeeded");
  m_connected = true;

  SendHelloPacket ();
}

void
CCL45Protocol::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_WARN ("CCL45Protocol Connection Failed");
  m_connected = false;
}

void
CCL45Protocol::SendHelloPacket ()
{
  Ptr<Packet> p = Create<Packet> ();
  MWHeader h;
  BwStatePair payload;

  h.SetHeaderType (MWHeader::HEADER_HELLO);
  h.SetData (payload);

  p->AddHeader (h);

  m_gatewaySocket->Send (p);

  m_state = WAITING_ACK;
}

void
CCL45Protocol::SendByePacket ()
{
  Ptr<Packet> p = Create<Packet> ();

  MWHeader h;
  h.SetHeaderType (MWHeader::HEADER_BYE);

  BwStatePair payload;
  payload.SetBw (0);
  payload.SetNodeState (m_recvState);

  h.SetData (payload);

  p->AddHeader (h);

  NS_ASSERT (m_gatewaySocket != 0);
  m_gatewaySocket->Send (p, 0);
}

void
CCL45Protocol::SendUsedPacket (uint64_t used)
{
  NS_ASSERT (used > 0);

  Ptr<Packet> p = Create<Packet> ();
  MWHeader h;

  h.SetHeaderType (MWHeader::HEADER_USED_SIZE);
  BwStatePair payload;
  payload.SetBw (used);
  payload.SetNodeState (m_recvState);
  h.SetData (payload);
  p->AddHeader (h);

  m_gatewaySocket->Send (p);
  m_state = WAITING_ACK;
}

} //namespace ns3
