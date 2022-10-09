/*
 * Deliverable 2: Uniquely coded using the OfSwitch13 module
 */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/ofswitch13-module.h>
#include <ns3/internet-apps-module.h>
#include "ns3/netanim-module.h"
#include "ns3/mobility-module.h"
#include "ns3/packet.h"
#include "ns3/address.h"
#include "ns3/address-utils.h"
#include "ns3/log.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/node.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/udp-socket-factory.h"
#include "ns3/boolean.h"
#include "ns3/seq-ts-size-header.h"
#include "ns3/applications-module.h"

using namespace ns3;

Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper> ();
NodeContainer switches;
InternetStackHelper internet; // Install the TCP/IP stack into hosts nodes
CsmaHelper csmaHelper;
NetDeviceContainer hostDevices;
NetDeviceContainer switchPorts;

void SetAllNodesXY(NodeContainer nodes, double x, double y, double deltaX);
void SetNodeXY(Ptr<Node> node, double x, double y);
void SetupSwitch(NodeContainer hosts, uint16_t switchID, uint16_t xCoord);
void SetupIpv4Addresses();
void InstallPing(Ptr<Node> src, Ptr<Node> dest);
void MacTxTrace(std::string context, Ptr<const Packet> pkt);
void turnPingDevicesOn();
void toggleSwitchRouting(uint16_t switchID, bool isOn);

int
main (int argc, char *argv[])
{
	uint16_t simTime = 38;
	bool verbose = false;
	bool trace = false;

	// Configure command line parameters
	CommandLine cmd;
	cmd.AddValue ("simTime", "Simulation time (seconds)", simTime);
	cmd.AddValue ("verbose", "Enable verbose output", verbose);
	cmd.AddValue ("trace", "Enable datapath stats and pcap traces", trace);
	cmd.Parse (argc, argv);

	if (verbose)
	{
	  OFSwitch13Helper::EnableDatapathLogs ();
	  LogComponentEnable ("OFSwitch13Interface", LOG_LEVEL_ALL);
	  LogComponentEnable ("OFSwitch13Device", LOG_LEVEL_ALL);
	  LogComponentEnable ("OFSwitch13Port", LOG_LEVEL_ALL);
	  LogComponentEnable ("OFSwitch13Queue", LOG_LEVEL_ALL);
	  LogComponentEnable ("OFSwitch13SocketHandler", LOG_LEVEL_ALL);
	  LogComponentEnable ("OFSwitch13Controller", LOG_LEVEL_ALL);
	  LogComponentEnable ("OFSwitch13LearningController", LOG_LEVEL_ALL);
	  LogComponentEnable ("OFSwitch13Helper", LOG_LEVEL_ALL);
	  LogComponentEnable ("OFSwitch13InternalHelper", LOG_LEVEL_ALL);
	}

	// Enable checksum computations (required by OFSwitch13 module)
	GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));


	switches.Create(3);

	// Use the CsmaHelper to connect host nodes to the switch node
	csmaHelper.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
	csmaHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (500)));

	// Create the controller node
	Ptr<Node> controllerNode = CreateObject<Node> ();
	Names::Add("Controller", controllerNode);

	// Configure the OpenFlow network domain
	of13Helper->InstallController (controllerNode);


	NodeContainer hosts1, hosts2, hosts3;
	hosts1.Create (3); hosts2.Create(3); hosts3.Create(3);
	// Create two host nodes
	//switch 1
	SetupSwitch(hosts1, 0, 3);
	SetupSwitch(hosts2, 1, 10);
	SetupSwitch(hosts3, 2, 17);
	toggleSwitchRouting(0, false);
	toggleSwitchRouting(1, false);
	toggleSwitchRouting(2, false);
	turnPingDevicesOn();

	//OfSwitch Config
	of13Helper->SetChannelType(OFSwitch13Helper::ChannelType::DEDICATEDP2P);
	of13Helper->CreateOpenFlowChannels ();

	SetupIpv4Addresses();
	V4PingHelper pingHelper = V4PingHelper ("10.1.1.4");
	pingHelper.SetAttribute ("Interval", TimeValue (Seconds (10)));
	pingHelper.SetAttribute ("Verbose", BooleanValue (true));

	ApplicationContainer pingApps = pingHelper.Install(hosts1.Get(0));
	pingApps.Start (Seconds (1));
	SetNodeXY(controllerNode, 15, 5);
	SetAllNodesXY(switches, 5, 12.5, 7.5);

	std::vector<NodeContainer> allHosts;
	allHosts.push_back(hosts1);
	allHosts.push_back(hosts2);
	allHosts.push_back(hosts3);

	Ptr<Node> apiNode = CreateObject<Node>();
	SetNodeXY(apiNode, 8, 3);
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
	NetDeviceContainer webPairNetDev;
	webPairNetDev = pointToPoint.Install (apiNode, controllerNode);
	internet.Install(apiNode);

	Ipv4AddressHelper address;
	address.SetBase ("10.1.5.0", "255.255.255.0");
	Ipv4InterfaceContainer webPairInterface;
	webPairInterface = address.Assign (webPairNetDev);

	UdpEchoServerHelper echoServer (80);
	ApplicationContainer serverApps = echoServer.Install (controllerNode);
	serverApps.Start (Seconds (8));
	serverApps.Stop (Seconds (10));

	UdpEchoClientHelper echoClient(webPairInterface.GetAddress(1), 9);
	echoClient.SetAttribute ("MaxPackets", UintegerValue (100));
	echoClient.SetAttribute ("Interval", TimeValue (MilliSeconds (500)));
	echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
	ApplicationContainer clientApps = echoClient.Install(apiNode);
	clientApps.Start (Seconds (7));
	clientApps.Stop (Seconds (8));


	AnimationInterface anim("D2.xml");
	uint32_t switchImageID = anim.AddResource("/home/brian-jesse/Downloads/bake/source/ns-3.32/scratch/Switch.png");
	uint32_t workstationImageID = anim.AddResource("/home/brian-jesse/Downloads/bake/source/ns-3.32/scratch/Workstation.png");
	uint32_t SDNImageID = anim.AddResource("/home/brian-jesse/Downloads/bake/source/ns-3.32/scratch/SDN.png");
	anim.UpdateNodeColor(controllerNode, 0, 0, 0);
	anim.SetBackgroundImage("/home/brian-jesse/Downloads/bake/source/ns-3.32/scratch/background.png", -4, -5, 0.025, 0.0325, 1);
	for(uint16_t i = 0; i < allHosts.size(); i++) {
		NodeContainer hosts = allHosts.at(i);
		for(uint16_t j = 0; j < hosts.GetN(); j++) {
			anim.UpdateNodeColor(hosts.Get(j), 0, 255, 255);
		}
	}
	//Controller
	anim.UpdateNodeImage(3, SDNImageID);
	anim.UpdateNodeSize(3, 3, 3);
	anim.UpdateNodeDescription(3, "SDN Controller");
	anim.UpdateNodeDescription(2, "Switches");
	anim.UpdateNodeDescription(12, "Workstations");
	//Switch
	for(uint16_t i = 0; i <= 2; i++) {
		anim.UpdateNodeImage(i, switchImageID);
		anim.UpdateNodeSize(i, 3, 3);
	}
	//Hosts
	for(uint16_t i = 4; i <= 12; i++) {
		anim.UpdateNodeSize(i, 2, 2);
		anim.UpdateNodeImage(i, workstationImageID);
	}

	// Enable datapath stats and pcap traces at hosts, switch(es), and controller(s)
	if (trace)
	{
	  of13Helper->EnableOpenFlowPcap ("openflow");
	  of13Helper->EnableDatapathStats ("switch-stats");
	  csmaHelper.EnablePcap ("switch", switchPorts, true);
	  csmaHelper.EnablePcap ("host", hostDevices);
	}

	// Run the simulation
	Simulator::Schedule(Seconds(6), &toggleSwitchRouting, 0, true);
	Simulator::Schedule(Seconds(6), &toggleSwitchRouting, 1, true);
	Simulator::Schedule(Seconds(6), &toggleSwitchRouting, 2, true);
	Simulator::Stop (Seconds (simTime));
	Simulator::Run ();
	Simulator::Destroy ();
}

void SetAllNodesXY(NodeContainer nodes, double x, double y, double deltaX) {
	MobilityHelper mobileHosts;
	mobileHosts.SetPositionAllocator ("ns3::GridPositionAllocator",
											"MinX", DoubleValue (x),
											"MinY", DoubleValue (y),
											"DeltaX", DoubleValue (deltaX),
										   "DeltaY", DoubleValue (10.0),
										   "GridWidth", UintegerValue (nodes.GetN()),
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

void SetupSwitch(NodeContainer hosts, uint16_t switchID, uint16_t xCoord) {
	for (size_t i = 0; i < hosts.GetN (); i++)
	{
		NodeContainer pair (hosts.Get (i), switches.Get(switchID));
		NetDeviceContainer link = csmaHelper.Install (pair);
		hostDevices.Add (link.Get (0));
		switchPorts.Add (link.Get (1));
		Ptr<Node> host = hosts.Get(i);
	}
	of13Helper->InstallSwitch (switches.Get(switchID), switchPorts);
	internet.Install (hosts);
	SetAllNodesXY(hosts, xCoord, 20, 1);
}

void toggleSwitchRouting(uint16_t switchID, bool isOn) {
	for (size_t i = 0; i < 3; i++)
	{
		Ptr<NetDevice> dev_i = switches.Get(switchID)->GetDevice(i);
		if(dev_i->GetInstanceTypeId() == CsmaNetDevice::GetTypeId()) {
			dev_i->SetAttribute ("SendEnable", BooleanValue (isOn));
		}
	}
}

void turnPingDevicesOn() {
	for(int i = 0; i < 2; i++) {
		Ptr<NetDevice> dev_i = switches.Get(i)->GetDevice(0);
		if(dev_i->GetInstanceTypeId() == CsmaNetDevice::GetTypeId()) {
			dev_i->SetAttribute ("SendEnable", BooleanValue (true));
		}
	}
}

void SetupIpv4Addresses() {
	Ipv4AddressHelper ipv4helpr;
	Ipv4InterfaceContainer hostIpIfaces;
	ipv4helpr.SetBase ("10.1.1.0", "255.255.255.0");
	hostIpIfaces = ipv4helpr.Assign (hostDevices);
}

void InstallPing(Ptr<Node> src, Ptr<Node> dest) {
	// Configure ping application between hosts
	V4PingHelper pingHelper = V4PingHelper ("10.1.1.2");
	pingHelper.SetAttribute ("Interval", TimeValue (Seconds (100)));
	pingHelper.SetAttribute ("Verbose", BooleanValue (true));

	ApplicationContainer pingApps = pingHelper.Install(src);
	pingApps.Start (Seconds (3));
}

void MacTxTrace(std::string context, Ptr<const Packet> pkt) {
	std::cout << context << std::endl;
	std::cout << "\tMaxTX Size=" << pkt->GetSize() << "\t" << Simulator::Now() << std::endl;
}
