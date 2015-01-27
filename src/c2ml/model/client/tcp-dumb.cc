#include "tcp-dumb.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("TcpCubicMwDumb");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpCubicMwDumb);
NS_OBJECT_ENSURE_REGISTERED (TcpNewRenoMwDumb);
NS_OBJECT_ENSURE_REGISTERED (TcpHyblaMwDumb);
NS_OBJECT_ENSURE_REGISTERED (TcpHighSpeedMwDumb);
NS_OBJECT_ENSURE_REGISTERED (TcpBicMwDumb);
NS_OBJECT_ENSURE_REGISTERED (TcpWestwoodMwDumb);

TypeId
TcpCubicMwDumb::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpCubicMwDumb")
    .SetParent<TcpCubicMw> ()
    .AddConstructor<TcpCubicMwDumb> ()
  ;

  return tid;
}

TypeId
TcpNewRenoMwDumb::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpNewRenoMwDumb")
    .SetParent<TcpNewRenoMw> ()
    .AddConstructor<TcpNewRenoMwDumb> ()
  ;

  return tid;
}

TypeId
TcpHyblaMwDumb::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpHyblaMwDumb")
    .SetParent<TcpHyblaMw> ()
    .AddConstructor<TcpHyblaMwDumb> ()
  ;

  return tid;
}

TypeId
TcpHighSpeedMwDumb::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpHighSpeedMwDumb")
    .SetParent<TcpHighSpeedMw> ()
    .AddConstructor<TcpHighSpeedMwDumb> ()
  ;

  return tid;
}

TypeId
TcpWestwoodMwDumb::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpWestwoodMwDumb")
    .SetParent<TcpWestwoodMw> ()
    .AddConstructor<TcpWestwoodMwDumb> ()
  ;

  return tid;
}

TypeId
TcpBicMwDumb::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpBicMwDumb")
    .SetParent<TcpBicMw> ()
    .AddConstructor<TcpBicMwDumb> ()
  ;

  return tid;
}

} //namespace ns3

