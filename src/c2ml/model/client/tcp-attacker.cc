#include "tcp-attacker.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("TcpCubicMwAttacker");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (TcpCubicMwAttacker);
NS_OBJECT_ENSURE_REGISTERED (TcpNewRenoMwAttacker);
NS_OBJECT_ENSURE_REGISTERED (TcpHyblaMwAttacker);
NS_OBJECT_ENSURE_REGISTERED (TcpHighSpeedMwAttacker);
NS_OBJECT_ENSURE_REGISTERED (TcpBicMwAttacker);
NS_OBJECT_ENSURE_REGISTERED (TcpWestwoodMwAttacker);

TypeId
TcpCubicMwAttacker::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpCubicMwAttacker")
    .SetParent<TcpCubicMw> ()
    .AddConstructor<TcpCubicMwAttacker> ()
  ;

  return tid;
}

TypeId
TcpNewRenoMwAttacker::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpNewRenoMwAttacker")
    .SetParent<TcpNewRenoMw> ()
    .AddConstructor<TcpNewRenoMwAttacker> ()
  ;

  return tid;
}

TypeId
TcpHyblaMwAttacker::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpHyblaMwAttacker")
    .SetParent<TcpHyblaMw> ()
    .AddConstructor<TcpHyblaMwAttacker> ()
  ;

  return tid;
}

TypeId
TcpHighSpeedMwAttacker::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpHighSpeedMwAttacker")
    .SetParent<TcpHighSpeedMw> ()
    .AddConstructor<TcpHighSpeedMwAttacker> ()
  ;

  return tid;
}

TypeId
TcpWestwoodMwAttacker::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpWestwoodMwAttacker")
    .SetParent<TcpWestwoodMw> ()
    .AddConstructor<TcpWestwoodMwAttacker> ()
  ;

  return tid;
}

TypeId
TcpBicMwAttacker::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::TcpBicMwAttacker")
    .SetParent<TcpBicMw> ()
    .AddConstructor<TcpBicMwAttacker> ()
  ;

  return tid;
}

} //namespace ns3
