#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/ofswitch13-module.h>
#include <ns3/internet-apps-module.h>
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

class Deliverable2 {
private:
	Ptr<Node> controllerNode;
	NodeContainer switches;
public:
	void InstallNetwork();
	void InstallHosts();//TODO
	void SetupAppearence();//TODO
	void SetAllNodesXY(NodeContainer nodes, double x, double y);
	void SetNodeXY(Ptr<Node> node, double x, double y);
};

void Deliverable2::InstallNetwork() {
		controllerNode = CreateObject<Node> ();
	  switches.Create(4);

	  // Configure the OpenFlow network domain
	  Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper> ();
	  of13Helper->InstallController (controllerNode);//Automatically connects based on of13 magic
	  for (size_t i = 0; i < switches.GetN (); i++) {
		  of13Helper->InstallSwitch(switches.Get(i));
	  }
	  of13Helper->SetChannelType(OFSwitch13Helper::ChannelType::DEDICATEDP2P);
	  of13Helper->CreateOpenFlowChannels ();//This finalizes the of13Helper

	  // Enable datapath stats and pcap traces at hosts, switch(es), and controller(s)
	  of13Helper->EnableOpenFlowPcap ("openflow");
	  of13Helper->EnableDatapathStats ("switch-stats");

	  AnimationInterface anim ("anim.xml");
	  Names::Add("SDN Controller", controllerNode);
	  anim.UpdateNodeColor(controllerNode, 0, 0, 0);
	  SetNodeXY(controllerNode, 15, -20);
	  SetAllNodesXY(switches, 0, 0);
}

void Deliverable2::InstallHosts() {

}

void Deliverable2::SetAllNodesXY(NodeContainer nodes, double x, double y) {
	MobilityHelper mobileHosts;
	mobileHosts.SetPositionAllocator ("ns3::GridPositionAllocator",
											"MinX", DoubleValue (x),
											"MinY", DoubleValue (y),
											"DeltaX", DoubleValue (10.0),
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
}

void Deliverable2::SetupAppearence() {

}
