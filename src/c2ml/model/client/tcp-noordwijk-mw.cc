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


#include "tcp-noordwijk-mw.h"
#include "cc-l45-protocol.h"
#include "ns3/node.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("TcpNoordwijkMw");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpNoordwijkMw);

TypeId
TcpNoordwijkMw::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpNoordwijkMw")
    .SetParent<TcpNoordwijk> ()
    .AddConstructor<TcpNoordwijkMw> ()
  ;
  return tid;
}

TcpNoordwijkMw::TcpNoordwijkMw () : TcpNoordwijk ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_availableBandwidth = 0;
}

int
TcpNoordwijkMw::Connect (const Address &address)
{
  int res = TcpNoordwijk::Connect (address);
  Ptr<CCL45Protocol> tcp = GetMwProtocol ();
  tcp->NotifyConnectionOpened (this);

  m_availableBandwidth = m_initialBWnd / m_initialTxTime.GetSeconds ();

  return res;
}

uint32_t
TcpNoordwijkMw::GetBandwidth () const
{
  uint32_t bw = (uint32_t) m_bWnd / m_txTime.Get ().GetSeconds ();

  NS_LOG_LOGIC ("Calculated BW: " << bw <<
                " with burstw=" << m_bWnd <<
                " and txTime (s)= " << m_txTime.Get ().GetSeconds () <<
                " Available BW: " << m_availableBandwidth);
  return bw;
}

void
TcpNoordwijkMw::SetBandwidth (uint32_t b)
{
  NS_LOG_FUNCTION (b);
  NS_LOG_DEBUG ("Set bw=" << b);

  NS_ASSERT (TcpSocketBase::AvailableWindow () <= m_bWnd);
  uint32_t oldBWnd = m_bWnd;

  m_availableBandwidth = b;

  m_bWnd = (uint32_t) (b * m_initialTxTime.GetSeconds ());

  RoundInf (m_bWnd);

  if (m_bWnd < 3*m_segmentSize)
    {
      m_bWnd = 3*m_segmentSize;
    }

  double tmp = (double) m_bWnd / m_availableBandwidth;
  tmp = (std::floor (tmp * 10000) + 1) / 10000;

  m_initialTxTime = Seconds (tmp);
  m_txTime = m_initialTxTime;

  //RoundTxTime ();

  m_initialBWnd = m_bWnd;

  if (oldBWnd > m_bWnd)
    {
      uint32_t diff = oldBWnd - m_bWnd;

      if (m_cWnd - UnAckDataCount () > diff)
        {
          m_cWnd -= diff;
        }
      else
        {
          m_cWnd = UnAckDataCount ();
        }

      NS_ASSERT (m_bWnd >= m_cWnd - UnAckDataCount ());
    }

  NS_LOG_DEBUG ("Results: initialTxTime: " << m_initialTxTime.GetSeconds () <<
                " bWnd: " << m_bWnd << " initialBWnd: " << m_initialBWnd);
}

void
TcpNoordwijkMw::CloseAndNotify ()
{
  NS_LOG_FUNCTION_NOARGS ();

  TcpNoordwijk::CloseAndNotify ();

  //Ptr<CCL45Protocol> tcp = GetMwProtocol ();
  //tcp->NotifyConnectionClosed (this);
}

Ptr<CCL45Protocol>
TcpNoordwijkMw::GetMwProtocol ()
{
  Ptr<CCL45Protocol> tcp = DynamicCast<CCL45Protocol> (m_tcp);

  if (tcp == 0)
    {
      NS_FATAL_ERROR ("No middleware protocol");
    }

  return tcp;
}

void
TcpNoordwijkMw::RateTracking (uint32_t bWnd, Time ackDispersion)
{
  uint32_t nextBw, actualBw = GetBandwidth ();

  NS_LOG_DEBUG (actualBw);
  NS_ASSERT (actualBw <= m_availableBandwidth);
  NS_ASSERT (TcpSocketBase::AvailableWindow () <= m_bWnd);

  Time txTime = m_txTime;

  TcpNoordwijk::RateTracking (bWnd, ackDispersion);

  if (txTime < m_txTime)
    {
      m_txTime = txTime;
    }

  nextBw = GetBandwidth ();

  if (nextBw > m_availableBandwidth)
    {
      SetBandwidth (m_availableBandwidth);
    }

  NS_ASSERT (TcpSocketBase::AvailableWindow () <= m_bWnd);
  NS_LOG_LOGIC ("After Tracking, I ended with bWnd=" << m_bWnd << " tim: " << m_txTime.Get ().GetSeconds ());

}

void
TcpNoordwijkMw::RateAdjustment (uint32_t bWnd, const Time& delta, const Time& deltaRtt)
{
  uint32_t nextBw;

  TcpNoordwijk::RateAdjustment (bWnd, delta, deltaRtt);

  nextBw = GetBandwidth ();

  if (nextBw > m_availableBandwidth)
    {
      SetBandwidth (m_availableBandwidth);
    }

  NS_LOG_LOGIC ("After Adjustment, I ended with b=" << nextBw << " with avail="
                                                    << m_availableBandwidth << " if necessary, I reverted");
}

bool
TcpNoordwijkMw::SendPendingData (bool withAck)
{
  bool res = TcpNoordwijk::SendPendingData (withAck);

  if (m_state > 4)
    {
      Ptr<CCL45Protocol> tcp = GetMwProtocol ();
      tcp->NotifyConnectionClosed (this);
      //m_txTimer.Cancel ();
    }

  return res;
}

} // namespace ns3
