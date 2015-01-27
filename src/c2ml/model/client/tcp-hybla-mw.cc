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
#include "tcp-hybla-mw.h"
#include "ns3/log.h"
#include "ns3/cc-l45-protocol.h"
#include "ns3/node.h"

NS_LOG_COMPONENT_DEFINE ("TcpHyblaMw");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpHyblaMw);

TypeId
TcpHyblaMw::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpHyblaMw")
    .SetParent<TcpHybla> ()
    .AddConstructor<TcpHyblaMw> ()
  ;

  return tid;
}

TcpHyblaMw::TcpHyblaMw () : TcpHybla ()
{
  m_availableBw = 0;
}

void
TcpHyblaMw::CloseAndNotify ()
{
  NS_LOG_FUNCTION_NOARGS ();

  TcpHybla::CloseAndNotify ();

  Ptr<CCL45Protocol> tcp = GetMwProtocol ();
  tcp->NotifyConnectionClosed (this);
}

Ptr<CCL45Protocol>
TcpHyblaMw::GetMwProtocol ()
{
  Ptr<CCL45Protocol> tcp = DynamicCast<CCL45Protocol> (m_tcp);

  if (tcp == 0)
    {
      NS_FATAL_ERROR ("No middleware protocol");
    }

  return tcp;
}

void
TcpHyblaMw::NewAck (SequenceNumber32 const& seq)
{
  Time rtt = m_rtt->GetEstimate();

  if (!rtt.IsZero () && m_minRtt > rtt)
    m_minRtt = rtt;

  if (m_minRtt.IsZero ())
    {
      m_minRtt = Seconds (1.0);
    }

  m_ssThresh = m_availableBw * m_minRtt.GetSeconds ();

  NS_LOG_DEBUG ("New ssTh: " << m_ssThresh);

  TcpHybla::NewAck (seq);

  if (m_cWnd > m_ssThresh)
    {
      m_cWnd = m_ssThresh;
    }

  SendPendingData (m_connected);
}

int
TcpHyblaMw::Connect (const Address &address)
{
  int res = TcpHybla::Connect (address);
  Ptr<CCL45Protocol> tcp = GetMwProtocol ();
  tcp->NotifyConnectionOpened (this);

  return res;
}

void TcpHyblaMw::SetBandwidth (uint32_t b)
{
  NS_LOG_FUNCTION (this << b);

  Time rtt = m_rtt->GetEstimate();

  uint32_t oldBw = m_availableBw;
  m_availableBw = b;

  m_ssThresh = m_availableBw * rtt.GetSeconds ();

  if (oldBw > b)
    {
      m_cWnd = m_ssThresh / 2;
    }
}

bool
TcpHyblaMw::SendPendingData (bool withAck)
{
  bool res = TcpHybla::SendPendingData (withAck);

  if (m_state > 4)
    {
      Ptr<CCL45Protocol> tcp = GetMwProtocol ();
      tcp->NotifyConnectionClosed (this);
    }

  return res;
}

} // namespace ns3
