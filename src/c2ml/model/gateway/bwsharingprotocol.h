#ifndef BWSHARINGPROTOCOL_H
#define BWSHARINGPROTOCOL_H


#include "ns3/c2ml-struct.h"
#include "ns3/socket.h"
#include <list>
#include <map>

namespace ns3 {

class BwSharingProtocol : public Object
{
public:

  /** \brief Handle a client arrive
   *
   * With this method, the protocol will register an arrived client socket
   *
   * \internal
   * This method has to accept an arriving socket and has to return the
   * corresponding BwStatePair which will be used in the ACK_HELLO packet
   *
   * \param socket The arriving socket
   */
  virtual BwStatePair HandleArrive (Ptr<Socket> socket) = 0;

    /** \brief Handle a client leave
     *
     * With this method, the protocol will manage a socket leaving the network
     *
     * \internal
     * This method has to accept the leaving socket and its state in order to
     * redistribute its bandwidth to the other nodes.
     *
     * \param socket The leaving socket
     * \param state The last leaving node's state
     */
  virtual void HandleLeave (Ptr<Socket> socket, NodeState state) = 0;

    /** \brief Handle a node's bandwidth change request
     *
     * With this method, the protocol will manage a socket asking for
     * a different bandwidth from the one previously assigned.
     *
     * \internal
     * This method has to accept the socket, its requested bandwidth
     * and its state in order to fairly redistribute the total bandwidth
     * considering request. It has to return the BwStatePair for the socket
     * which made the request, which will be used in the ACK_USED packet.
     *
     * \param socket The socket asking for a different bandwidth
     * \param bw The new requested bandwidth
     * \param state The last leaving node's state
     */
  virtual BwStatePair HandleBwChange (Ptr<Socket> socket, uint64_t bw,
                                      NodeState state) = 0;

    /** \brief Return the bandwidth for a specific socket
     *
     * This method, given a certain socket, returns its assigned bandwidth
     *
     * \internal
     * This method should be based on the specific allocation protocol and its
     * data structures.
     *
     * \param socket The socket in question
     */
  virtual uint64_t GetBwFromSocket (Ptr<Socket> socket) = 0;

    /** \brief Return the NodeState for a specific socket
     *
     * This method, given a certain socket, returns its NodeState
     *
     * \internal
     * This method should be based on the specific allocation protocol and its
     * data structures.
     *
     * \param socket The socket in question
     */
  virtual NodeState GetStateFromSocket (Ptr<Socket> socket) = 0;

  virtual uint64_t GetGoodBw () const = 0;

protected:

  uint64_t m_totalBandwidth; //!< The total available bandwidth

};

} //namespace ns3

#endif // BWSHARINGPROTOCOL_H
