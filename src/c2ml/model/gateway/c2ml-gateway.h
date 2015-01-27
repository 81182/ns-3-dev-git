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
#ifndef C2MLGATEWAY_H
#define C2MLGATEWAY_H

#include "ns3/application.h"
#include "ns3/socket.h"
#include "ns3/traced-callback.h"
#include "ns3/timer.h"

#include "bwsharingprotocol.h"

#include <list>

namespace ns3 {

class C2MLTxQueue;

class C2MLGateway : public Application
{
public:
  enum Mode
  {
    UNWEIGHTED = 0,
    FAIR       = 1,
    DYBRA      = 2
  };

  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  C2MLGateway ();

  virtual ~C2MLGateway ();

  /**
   * \return pointer to listening socket
   */
  Ptr<Socket> GetListeningSocket (void) const;

  /**
   * \return list of pointers to accepted sockets
   */
  std::list<Ptr<Socket> > GetAcceptedSockets (void) const;

protected:
  virtual void DoDispose (void);
  virtual Ptr<Packet> CreatePacket (MWHeader::HeaderType header, BwStatePair payload);
  virtual void NotifyClientsForBw ();
  void StartTimer (Ptr<Socket> socket);
  Mode m_mode;

private:
  uint64_t m_clientNumber;
  uint64_t m_totalBandwidth;

  bool m_timerStarted;
  Timer m_timer;

  uint32_t m_timestamp;

  bool m_aqmEnabled;
  Ptr<C2MLTxQueue> m_queue;

  // inherited from Application base class.
  virtual void StartApplication (void);    // Called at time specified by Start
  virtual void StopApplication (void);     // Called at time specified by Stop

  /**
   * \brief Handle a packet received by the application
   * \param socket the receiving socket
   */
  void HandleRead (Ptr<Socket> socket);
  /**
   * \brief Handle an incoming connection
   * \param socket the incoming connection socket
   * \param from the address the connection is from
   */
  void HandleAccept (Ptr<Socket> socket, const Address& from);
  /**
   * \brief Handle an connection close
   * \param socket the connected socket
   */
  void HandlePeerClose (Ptr<Socket> socket);
  /**
   * \brief Handle an connection error
   * \param socket the connected socket
   */
  void HandlePeerError (Ptr<Socket> socket);

  // In the case of TCP, each socket accept returns a new socket, so the
  // listening socket is stored separately from the accepted sockets
  Ptr<Socket>     m_socket;       //!< Listening socket
  std::list<Ptr<Socket> > m_socketList; //!< the accepted sockets

  Address         m_local;        //!< Local address to bind to
  uint64_t        m_totalRx;      //!< Total bytes received
  TypeId          m_tid;          //!< Protocol TypeId

  /// Traced Callback: received packets, source address.
  TracedCallback<Ptr<const Packet>, const Address &> m_rxTrace;

  // Bandwidth allocator object
  Ptr<BwSharingProtocol> m_bwAllocator; //!< The bandwidth allocation protocol object
};

} // namespace ns3

#endif // C2MLGATEWAY_H
