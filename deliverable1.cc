/*
 * Computer Networks Semester Project Deliverable 1
 * Adapted from OFSwitch13-first.cc
 */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/ofswitch13-module.h>
#include <ns3/internet-apps-module.h>
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

static void SetAllNodesXY(NodeContainer nodes, double x, double y);
static void SetNodeXY(Ptr<Node> node, double x, double y);

int main (int argc, char *argv[])
{
  uint16_t simTime = 10;
  bool verbose = true;
  bool trace = true;

  // Configure command line parameters
  CommandLine cmd;
  cmd.Parse (argc, argv);

  OFSwitch13Helper::EnableDatapathLogs ();

  // Enable checksum computations (required by OFSwitch13 module)
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  // Create two host nodes
  NodeContainer hosts;
  hosts.Create (3);

  // Create the switch node
  Ptr<Node> switchNode = CreateObject<Node> ();

  // Use the CsmaHelper to connect host nodes to the switch node
  CsmaHelper csmaHelper;
  csmaHelper.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
  csmaHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  NetDeviceContainer hostDevices;
  NetDeviceContainer switchPorts;
  for (size_t i = 0; i < hosts.GetN (); i++)
    {

      NodeContainer pair (hosts.Get (i), switchNode);
      NetDeviceContainer link = csmaHelper.Install (pair);
      hostDevices.Add (link.Get (0));
      switchPorts.Add (link.Get (1));
    }



  // Create the controller node
  Ptr<Node> controllerNode = CreateObject<Node> ();

  // Configure the OpenFlow network domain
  Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper> ();
  of13Helper->InstallController (controllerNode);
  of13Helper->InstallSwitch (switchNode, switchPorts);
  of13Helper->CreateOpenFlowChannels ();

  // Install the TCP/IP stack into hosts nodes
  InternetStackHelper internet;
  internet.Install (hosts);

  // Set IPv4 host addresses
  Ipv4AddressHelper ipv4helpr;
  Ipv4InterfaceContainer hostIpIfaces;
  ipv4helpr.SetBase ("10.1.1.0", "255.255.255.0");
  hostIpIfaces = ipv4helpr.Assign (hostDevices);

  // Configure ping application between hosts
  V4PingHelper pingHelper = V4PingHelper (hostIpIfaces.GetAddress (1));
  pingHelper.SetAttribute ("Verbose", BooleanValue (true));
  ApplicationContainer pingApps = pingHelper.Install (hosts.Get (0));
  pingApps.Start (Seconds (1));

  // Enable datapath stats and pcap traces at hosts, switch(es), and controller(s)
  if (trace)
    {
      of13Helper->EnableOpenFlowPcap ("openflow");
      of13Helper->EnableDatapathStats ("switch-stats");
      csmaHelper.EnablePcap ("switch", switchPorts, true);
      csmaHelper.EnablePcap ("host", hostDevices);
    }

  //Visual configurations
  AnimationInterface anim ("anim.xml");
  Names::Add("SDN Controller", controllerNode);
  Names::Add("Switch", switchNode);
  anim.UpdateNodeColor(controllerNode, 0, 0, 0);
  SetNodeXY(controllerNode, 10, -20);
  SetNodeXY(switchNode, 10, -10);
  SetAllNodesXY(hosts, 0, 0);


  // Run the simulation
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();
}

void SetAllNodesXY(NodeContainer nodes, double x, double y) {
	MobilityHelper mobileHosts;
	    mobileHosts.SetPositionAllocator ("ns3::GridPositionAllocator",
	                                        "MinX", DoubleValue (x),
	                                        "MinY", DoubleValue (y),
	                                        "DeltaX", DoubleValue (10.0),
	                                       "DeltaY", DoubleValue (10.0),
	                                       "GridWidth", UintegerValue (3),
	                                         "LayoutType", StringValue ("RowFirst"));
	    mobileHosts.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	    mobileHosts.Install (nodes);
}

void SetNodeXY(Ptr<Node> node, double x, double y) {
	MobilityHelper mobileHosts;
	    mobileHosts.SetPositionAllocator ("ns3::GridPositionAllocator",
	                                        "MinX", DoubleValue (x),
	                                        "MinY", DoubleValue (y),
	                                        "DeltaX", DoubleValue (10.0),
	                                       "DeltaY", DoubleValue (10.0),
	                                       "GridWidth", UintegerValue (3),
	                                         "LayoutType", StringValue ("RowFirst"));
	    mobileHosts.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	    mobileHosts.Install (node);
}
