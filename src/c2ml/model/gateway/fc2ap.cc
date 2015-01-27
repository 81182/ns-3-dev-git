#include "fc2ap.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include <stdio.h>
#include <memory>

NS_LOG_COMPONENT_DEFINE ("FC2AP");

namespace ns3 {

FC2AP::FC2AP(uint64_t totalBandwidth)
{
  NS_LOG_FUNCTION (this << totalBandwidth);
  m_totalBandwidth = totalBandwidth;
  m_freeCount = 0;
  m_limitedCount = 0;
  m_sumLimitedBw = 0;
}

FC2AP::~FC2AP ()
{
  NS_LOG_FUNCTION (this);
}

BwStatePair
FC2AP::HandleArrive (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  FC2APMapIterator mIt = m_allocation.find (socket);

  if(mIt != m_allocation.end ())
    {
      NS_LOG_ERROR ("Something went wrong with inserting the socket " << socket << "in allocation map");
    }

  AllocationData ad;
  ad.bandwidth = 0;
  ad.isLimited = false;
  ad.requestedBandwidth = 0;

  m_allocation.insert (FC2APMapElement (socket, ad));
  m_freeCount++;

  RedistributeBwToFreeNodes ();

  BwStatePair ret (GetBwFromSocket (socket), GetStateFromSocket (socket));
  return ret;
}

void
FC2AP::HandleLeave (Ptr<Socket> socket, NodeState state)
{
  NS_LOG_FUNCTION (this << socket);

  (void) state;

  FC2APMapIterator mIt = m_allocation.find (socket);

  if(mIt->second.isLimited)
    {
      m_limitedCount--;
      m_sumLimitedBw -= mIt->second.bandwidth;
    }
  else
    {
      m_freeCount--;
    }

  if(m_allocation.erase (socket) == 0)
    {
      NS_LOG_ERROR ("Something went wrong trying to erase the socket from the map");
    }

  if(m_freeCount != 0) RedistributeBwToFreeNodes ();
}

BwStatePair
FC2AP::HandleBwChange (Ptr<Socket> socket, uint64_t bw, NodeState state)
{
  NS_LOG_FUNCTION (this << socket << "Using bandwidth:" << bw << " with state: " << state.GetState ());

  (void) state;

  FC2APMapIterator mIt = m_allocation.find (socket);
  mIt->second.requestedBandwidth = bw;

  if(!mIt->second.isLimited && bw >= mIt->second.bandwidth) // nodo free che vuole più banda
    {
      NS_LOG_ERROR ("A free node shouldn't ask for more bandwidth");
    }
  else if(!mIt->second.isLimited && bw < mIt->second.bandwidth) // nodo free che usa meno banda
    {
      FreeAskingForLessBw (mIt, bw);
    }
  else if(mIt->second.isLimited && bw < mIt->second.bandwidth) // nodo limited che chiede ancora meno banda
    {
      LimitedAskingForLessBw (mIt, bw);
    }
  else if(mIt->second.isLimited && bw >= mIt->second.bandwidth) // nodo limited che vuole più bw
    {
      LimitedAskingForMoreBw (mIt, bw);
    }

  RedistributeBwToFreeNodes ();

  BwStatePair ret (GetBwFromSocket (socket), GetStateFromSocket (socket));
  return ret;
}

void
FC2AP::FreeAskingForLessBw (FC2APMapIterator& mIt, uint64_t bw)
{
  NS_LOG_FUNCTION (this << bw);

  mIt->second.isLimited = true;
  mIt->second.bandwidth = bw;

  NS_ASSERT(m_freeCount > 0);

  m_freeCount--;
  m_limitedCount++;
  m_sumLimitedBw += bw;
}

void
FC2AP::LimitedAskingForLessBw (FC2APMapIterator& mIt, uint64_t bw)
{
  NS_LOG_FUNCTION (this << bw);

  NS_ASSERT(m_sumLimitedBw - (mIt->second.bandwidth - bw) > 0);

  m_sumLimitedBw -= mIt->second.bandwidth - bw;
  mIt->second.bandwidth = bw;
}

void
FC2AP::LimitedAskingForMoreBw (FC2APMapIterator& mIt, uint64_t bw)
{
  NS_LOG_FUNCTION (this << bw);

  NS_ASSERT(m_limitedCount > 0);
  NS_ASSERT( (m_sumLimitedBw - mIt->second.bandwidth > 0));

  if(bw >= (m_totalBandwidth - m_sumLimitedBw + mIt->second.bandwidth ) / (m_freeCount+1))
    {
      mIt->second.isLimited = false;
      m_freeCount++;
      m_limitedCount--;
      m_sumLimitedBw -= mIt->second.bandwidth;
    }
  else
    {
      m_sumLimitedBw += (bw - mIt->second.bandwidth);
      mIt->second.bandwidth = bw;
    }
}

void
FC2AP::RedistributeBwToFreeNodes ()
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT(m_freeCount > 0);

  bool restart = false;

      for (FC2APMapIterator mIt = m_allocation.begin (); mIt != m_allocation.end (); mIt++)
        {
          if(restart){
              mIt--;
              restart = false;
          }

          AllocationData& ad = mIt->second;
          uint64_t toAssign = (m_totalBandwidth - m_sumLimitedBw)/m_freeCount;

          if (!ad.isLimited)
            {
              if (ad.requestedBandwidth !=0 && ad.requestedBandwidth < toAssign)
                {
                  NS_ASSERT(m_freeCount > 0);
                  ad.isLimited = true;
                  ad.bandwidth = ad.requestedBandwidth;
                  m_freeCount--;
                  m_limitedCount++;
                  m_sumLimitedBw += ad.requestedBandwidth;
                  mIt = m_allocation.begin ();  // se sono arrivato qui il for deve ricominciare
                  restart = true;
                }
              else
                {
                  ad.bandwidth = toAssign;
                }
            }
          else
            {
              if(toAssign < ad.bandwidth)
                {
                  NS_ASSERT(m_limitedCount > 0);
                  ad.isLimited = false;
                  m_freeCount++;
                  m_limitedCount--;
                  m_sumLimitedBw -= ad.bandwidth;
                  mIt = m_allocation.begin ();  // se sono arrivato qui il for deve ricomiciare
                  restart = true;
                }
            }
        }
}

uint64_t
FC2AP::GetBwFromSocket (Ptr<Socket> socket){
  NS_LOG_FUNCTION (this << socket);

  FC2APMapIterator mIt = m_allocation.find (socket);
  if(mIt == m_allocation.end ())
    {
      NS_LOG_ERROR ("Something went wrong looking for the socket " << socket << "in allocation map");
    }

  return mIt->second.bandwidth;
}

NodeState
FC2AP::GetStateFromSocket (Ptr<Socket> socket){
  NS_LOG_FUNCTION (this << socket);

  FC2APMapIterator mIt = m_allocation.find (socket);
  if(mIt == m_allocation.end ())
    {
      NS_LOG_ERROR ("Something went wrong looking for the socket " << socket << "in allocation map");
    }

  if(mIt->second.isLimited)
    return NodeState ( NodeState::NODE_BAD, mIt->second.bandwidth);
  else
    return NodeState ( NodeState::NODE_GOOD, mIt->second.bandwidth);
}

} //namespace ns3
