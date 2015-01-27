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
#ifndef TCPNOORDWIJKMW_H
#define TCPNOORDWIJKMW_H

#include "ns3/tcp-noordwijk.h"
#include "tcp-mw.h"

namespace ns3 {

class CCL45Protocol;

class TcpNoordwijkMw : public TcpNoordwijk,
                       public TcpMw
{
public:
  static TypeId GetTypeId (void);

  TcpNoordwijkMw ();

  void SetBandwidth (uint32_t b);

protected:
  uint32_t GetBandwidth () const;
  virtual void RateTracking (uint32_t bWnd, Time ackDispersion);
  virtual void RateAdjustment (uint32_t bWnd, const Time& delta, const Time& deltaRtt);
  virtual bool SendPendingData (bool withAck = false);

  virtual int Connect (const Address &address);
  virtual void CloseAndNotify (void);

  Ptr<CCL45Protocol> GetMwProtocol ();

  uint32_t m_availableBandwidth;
};

} // namespace ns3
#endif // TCPNOORDWIJKMW_H
