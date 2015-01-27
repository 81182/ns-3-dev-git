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
#ifndef CCL45PROTOCOL_H
#define CCL45PROTOCOL_H

#include "ns3/tcp-l4-protocol.h"
#include "ns3/c2ml-struct.h"

#include "tcp-mw.h"

#include <list>

namespace ns3 {



/**
 * \brief The Congestion Control Layer
 *
 * This layer take care of distribuiting the available bandwidth
 * between all sockets; to do so, it intercept all socket creations,
 * and communicate with the gateway with CCLP, Congestion Control
 * Layer Protocol.
 *
 * It has a strong interaction with the TCP protocol used. For example,
 * the socket creations are intercepted through NotifyConnectionOpened,
 * which TCP should call to signaling that its opening; the same is valid for
 * NotifyConnectionClosed.
 *
 * Communications with gateway are done through a socket, with a standard TCP.
 * A notable callback is HandleRead for incoming messages.
 *
 * Bandwidth sharing is done through CalcStreamBandwidth, which is called by
 * NotifyBwChange. Please note that a general rework is required for supporting
 * different bandwidth for different TCP socket.
 *
 * To use this protocol in conjuction with a InternetStackHelper, do the
 * following:
 *
 * \verbatim
 InternetStackHelper h;
 h.SetTcp("ns3::CCL45Protocol");

 ... (other code which uses h)
 \endverbatim
 *
 *
 */
class CCL45Protocol : public TcpL4Protocol
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  CCL45Protocol ();

  // From TcpL4Protocol
  virtual Ptr<Socket> CreateSocket (void);
  virtual Ptr<Socket> CreateSocket (TypeId socketTypeId);

  /**
   * \brief CSI notifies about a channel bandwidth change
   *
   * \param bw New available bandwidth
   */
  void CsiBwChange (uint64_t bw);

  /**
   * \brief A socket informs it has opened the connection
   *
   * \param socket socket which called us
   */
  void NotifyConnectionOpened (TcpMw * socket);

  /**
   * \brief A socket informs it has closed the connection
   *
   * \param socket socket which called us
   */
  void NotifyConnectionClosed (TcpMw * socket);

protected:

  enum State
  {
    WAITING_ACK = 1,
    NORMAL      = 2
  };

  /**
   * \brief Called when a packet is arriving from the gateway
   *
   * \param socket Socket with the gateway
   */
  void HandleRead (Ptr<Socket> socket);

  /**
   * \brief Some socket doesn't use the full bandwidth.
   *
   * NOT IMPLEMENTED YET
   */
  void HandleBwChanged ();

  /**
   * \brief The connection with the gateway is opened
   *
   * \param socket socket with the gateway
   */
  void ConnectionSucceeded (Ptr<Socket> socket);

  /**
   * \brief The connection with the gateway failed
   *
   * \param socket socket with the gateway
   */
  void ConnectionFailed (Ptr<Socket> socket);

  /**
   * \brief Calculate the bandwidth for each flow
   *
   * Please note that this case is for unweighted fair budget.
   * To implement other, a refactor is needed.
   *
   * \param socket socket with the gateway
   */
  virtual uint64_t CalcStreamBandwidth ();

  /**
   * \brief Create an Hello Packet
   *
   * \return the hello packet
   */
  void SendHelloPacket ();

  /**
   * \brief Create a Bye Packet
   *
   * \return the bye packet
   */
  void SendByePacket ();

  /**
   * \brief Create an Used Packet
   *
   * \return the used packet
   */
  void SendUsedPacket (uint64_t used);

  void AckHelloReceived (const MWHeader& header);
  void AckUsedReceived (const MWHeader& header);
  void AllowedReceived (const MWHeader& header);

  Address m_gateway;
  Address m_local;
  Ptr<Socket> m_gatewaySocket;
  bool m_connected;

  uint16_t m_connectionsOpened;
  uint64_t m_availableBandwidth;

  std::vector<TcpMw * > m_sockets;

  NodeState m_recvState; //!< The last received state from the gateway
  uint64_t m_lastCsi; //!< The last received CSI message

  State m_state;
  std::list<Ptr<Packet> > m_packetsReceived;

  uint32_t m_lastTs;
};

} // namespace ns3

#endif /* CCL45PROTOCOL_H */

