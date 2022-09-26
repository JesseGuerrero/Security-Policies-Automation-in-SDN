/*
 * Computer Networks Semester Project Deliverable 2
 * Adapted from deliverable1.cc
 */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/ofswitch13-module.h>
#include <ns3/internet-apps-module.h>
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include "deliverable2.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  uint16_t simTime = 10;

  // Configure command line parameters
  CommandLine cmd;
  cmd.Parse (argc, argv);

  OFSwitch13Helper::EnableDatapathLogs ();

  // Enable checksum computations (required by OFSwitch13 module)
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  Deliverable2 networkDeliverable;
  networkDeliverable.InstallNetwork();

  // Run the simulation
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();
}


