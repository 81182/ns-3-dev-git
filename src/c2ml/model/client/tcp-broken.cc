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

#define NS_LOG_APPEND_CONTEXT \
  if (m_node) \
    { std::clog << Simulator::Now ().GetSeconds () << \
        " [node " << m_node->GetId () << "] "; }

#include "tcp-broken.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/abort.h"

NS_LOG_COMPONENT_DEFINE ("TcpBroken");

using namespace ns3;

NS_OBJECT_ENSURE_REGISTERED (TcpBroken);

TypeId
TcpBroken::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpBroken")
    .SetParent<TcpSocketBase> ()
    .AddConstructor<TcpBroken> ()
  ;
  return tid;
}

TcpBroken::TcpBroken()
{
}

void
TcpBroken::SetSSThresh (uint32_t threshold)
{
  (void) threshold;
  NS_LOG_WARN ("TcpBroken does not perform slow start");
}

uint32_t
TcpBroken::GetSSThresh (void) const
{
  NS_LOG_WARN ("TcpBroken does not perform slow start");
  return 0;
}

void
TcpBroken::SetInitialCwnd (uint32_t cwnd)
{
  NS_ABORT_MSG_UNLESS (m_state == CLOSED,
                       "TcpBroken::SetInitialCwnd() cannot change initial "
                       "cwnd after connection started.");
  m_initialBWnd = cwnd;
}

uint32_t
TcpBroken::GetInitialCwnd (void) const
{
  return m_initialBWnd;
}

/* We initialize m_cWnd from this function, after attributes initialized */
int
TcpBroken::Listen (void)
{
  NS_LOG_FUNCTION (this);
  Initialize ();
  return TcpSocketBase::Listen ();
}

/* We initialize m_cWnd from this function, after attributes initialized */
int
TcpBroken::Connect (const Address & address)
{
  NS_LOG_FUNCTION (this << address);
  Initialize ();
  return TcpSocketBase::Connect (address);
}

Ptr<TcpSocketBase>
TcpBroken::Fork (void)
{
  return CopyObject<TcpBroken> (this);
}
