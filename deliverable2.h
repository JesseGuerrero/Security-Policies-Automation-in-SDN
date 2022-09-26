#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/ofswitch13-module.h>
#include <ns3/internet-apps-module.h>
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
#include <string>
#include <list>

using namespace ns3;

class Deliverable2 {
private:
	Ptr<Node> controllerNode;
	NodeContainer switches;
	std::vector<NodeContainer> allHosts;
	Ptr<OFSwitch13InternalHelper> of13Helper;

public:
	void InstallNetwork();
	void InstallNodes();//TODO
	void SetupAppearence();
	void SetAllNodesXY(NodeContainer nodes, double x, double y, double deltaX);
	void SetNodeXY(Ptr<Node> node, double x, double y);
};

void Deliverable2::InstallNetwork() {
	controllerNode = CreateObject<Node>();
	of13Helper = CreateObject<OFSwitch13InternalHelper>();
	InstallNodes();

	// Enable datapath stats and pcap traces at hosts, switch(es), and controller(s)
	of13Helper->EnableOpenFlowPcap ("openflow");
	of13Helper->EnableDatapathStats ("switch-stats");
	SetupAppearence();
}

void Deliverable2::InstallNodes() {
	switches.Create(4);

	// Use the CsmaHelper to connect host nodes to the switch node
	CsmaHelper csmaHelper;
	csmaHelper.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
	csmaHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));



	of13Helper->InstallController (controllerNode);//Automatically connects based on of13 magic
	for (size_t switchIndex = 0; switchIndex < switches.GetN (); switchIndex++) {
		NodeContainer hosts;
		hosts.Create (3);
		NetDeviceContainer hostDevices;
		NetDeviceContainer switchPorts;
		for (size_t hostIndex = 0; hostIndex < hosts.GetN (); hostIndex++)
		{
		  NodeContainer pair (hosts.Get (hostIndex), switches.Get(switchIndex));
		  NetDeviceContainer link = csmaHelper.Install (pair);
		  hostDevices.Add (link.Get (0));
		  switchPorts.Add (link.Get (1));
		}
		of13Helper->InstallSwitch(switches.Get(switchIndex), switchPorts);
		Names::Add("Switch " + std::to_string(switchIndex+1), switches.Get(switchIndex));
		SetAllNodesXY(hosts, (switchIndex*7.5) + 3, 30, 1);
		allHosts.push_back(hosts);
	}


	//This defines the open-flow channel type
	of13Helper->SetChannelType(OFSwitch13Helper::ChannelType::DEDICATEDP2P);
	of13Helper->CreateOpenFlowChannels ();//This finalizes the of13Helper
}

void Deliverable2::SetAllNodesXY(NodeContainer nodes, double x, double y, double deltaX) {
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

void Deliverable2::SetNodeXY(Ptr<Node> node, double x, double y) {
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
	//anim.SetConstantPosition(node, x, y, 0);
}

void Deliverable2::SetupAppearence() {
	Names::Add("SDN Controller", controllerNode);
	SetNodeXY(controllerNode, 17.5, 10);
	SetAllNodesXY(switches, 5, 20, 7.5);

	AnimationInterface anim("D2anim.xml");
	anim.UpdateNodeColor(controllerNode, 0, 0, 0);
	anim.SetBackgroundImage("/home/brian-jesse/Downloads/bake/source/ns-3.32/scratch/background.png", -4, -5, 0.03, 0.045, 1);
	for(uint16_t i = 0; i < allHosts.size(); i++) {
		NodeContainer hosts = allHosts.at(i);
		for(uint16_t j = 0; j < hosts.GetN(); j++) {
			anim.UpdateNodeColor(hosts.Get(j), 0, 255, 255);
		}
	}
	for(uint16_t i = 5; i <= 16; i++) {
		anim.UpdateNodeSize(i, 0.5, 0.5);
	}
}
