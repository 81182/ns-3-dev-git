/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 Natale Patriciello <natale.patriciello@gmail.com>
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
#include "tcp-cubic-mw.h"
#include "ns3/log.h"
#include "ns3/cc-l45-protocol.h"
#include "ns3/node.h"

NS_LOG_COMPONENT_DEFINE ("TcpCubicMw");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpCubicMw);

TypeId
TcpCubicMw::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpCubicMw")
    .SetParent<TcpCubic> ()
    .AddConstructor<TcpCubicMw> ()
  ;

  return tid;
}

TcpCubicMw::TcpCubicMw () : TcpCubic ()
{
  m_availableBw = 0;
  m_minRtt = Time::Max ();
}

void
TcpCubicMw::CloseAndNotify ()
{
  NS_LOG_FUNCTION_NOARGS ();

  TcpCubic::CloseAndNotify ();

  Ptr<CCL45Protocol> tcp = GetMwProtocol ();
  tcp->NotifyConnectionClosed (this);
}

Ptr<CCL45Protocol>
TcpCubicMw::GetMwProtocol ()
{
  Ptr<CCL45Protocol> tcp = DynamicCast<CCL45Protocol> (m_tcp);

  if (tcp == 0)
    {
      NS_FATAL_ERROR ("No middleware protocol");
    }

  return tcp;
}

void
TcpCubicMw::NewAck (SequenceNumber32 const& seq)
{
  Time rtt = m_lastRtt.Get ();
  if (rtt.IsZero ())
    rtt = m_rtt->GetCurrentEstimate ();

  if (m_minRtt > rtt)
    m_minRtt = rtt;

  m_ssThresh = m_availableBw * m_minRtt.GetSeconds ();

  TcpCubic::NewAck (seq);

  if (m_cWnd > m_ssThresh)
    {
      m_cWnd = m_ssThresh;
    }

  SendPendingData (m_connected);
}

int
TcpCubicMw::Connect (const Address &address)
{
  int res = TcpCubic::Connect (address);
  Ptr<CCL45Protocol> tcp = GetMwProtocol ();
  tcp->NotifyConnectionOpened (this);

  return res;
}

void TcpCubicMw::SetBandwidth (uint32_t b)
{
  NS_LOG_FUNCTION (this << b);

  Time rtt = m_lastRtt.Get ();
  if (rtt.IsZero ())
    rtt = m_rtt->GetCurrentEstimate ();

  if (m_minRtt > rtt)
    m_minRtt = rtt;

  uint32_t oldBw = m_availableBw;
  m_availableBw = b;

  m_ssThresh = m_availableBw * m_minRtt.GetSeconds ();

  if (oldBw > b && m_cWnd >= m_ssThresh) {
    m_cWnd = m_ssThresh / 2;
  }
}

bool
TcpCubicMw::SendPendingData (bool withAck)
{
  bool res = TcpCubic::SendPendingData (withAck);

  if (m_state > 4)
    {
      Ptr<CCL45Protocol> tcp = GetMwProtocol ();
      tcp->NotifyConnectionClosed (this);
    }

  return res;
}

} // namespace ns3
