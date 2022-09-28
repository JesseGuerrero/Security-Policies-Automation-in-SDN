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
	void InstallNodes();
	void InstallSouthBound();
	void SetupAppearencePyVis();
	void SetupAppearenceNetAnim();
	void SetAllNodesXY(NodeContainer nodes, double x, double y, double deltaX);
	void SetNodeXY(Ptr<Node> node, double x, double y);
};

void Deliverable2::InstallNetwork() {
	controllerNode = CreateObject<Node>();
	of13Helper = CreateObject<OFSwitch13InternalHelper>();
	InstallNodes();
	SetupAppearencePyVis();
	SetupAppearenceNetAnim();
}

void Deliverable2::InstallNodes() {
	switches.Create(4);
	of13Helper->InstallController (controllerNode);//Automatically connects based on of13 magic
	InstallSouthBound();

	//This defines the open-flow channel type
	of13Helper->SetChannelType(OFSwitch13Helper::ChannelType::DEDICATEDP2P);
	of13Helper->CreateOpenFlowChannels ();//This finalizes the of13Helper
}

void Deliverable2::InstallSouthBound() {
	for (size_t switchIndex = 0; switchIndex < switches.GetN (); switchIndex++) {
			NodeContainer hosts;
			hosts.Create (3);

			//Wire the switch to the hosts
			PointToPointHelper pointToPoint;
			pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
			pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
			for (size_t hostIndex = 0; hostIndex < hosts.GetN (); hostIndex++)
			{
				NodeContainer nodes;
				nodes.Add(switches.Get(switchIndex));
				nodes.Add(hosts.Get(hostIndex));
				NetDeviceContainer devices;
				devices = pointToPoint.Install (nodes);
				if(switchIndex == 0) {
					//InternetStackHelper stack;
					//stack.Install (nodes);
					Ipv4AddressHelper address;
					const char *str = ("10.1." + std::to_string(switchIndex+1) + ".0").c_str();
					address.SetBase(str, "255.255.255.0");
					Ipv4InterfaceContainer interfaces = address.Assign (devices);
				}
			}
			if(switchIndex+1 < switches.GetN ()) {
				NodeContainer nodes;
				nodes.Add(switches.Get(switchIndex));
				nodes.Add(switches.Get(switchIndex+1));
				NetDeviceContainer devices;
				devices = pointToPoint.Install (nodes);
			}

			of13Helper->InstallSwitch(switches.Get(switchIndex));
			Names::Add("Switch " + std::to_string(switchIndex+1), switches.Get(switchIndex));
			SetAllNodesXY(hosts, (switchIndex*7.5) + 3, 20, 1);
			allHosts.push_back(hosts);
		}
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

void Deliverable2::SetupAppearencePyVis() {
	Names::Add("SDN Controller", controllerNode);
	SetNodeXY(controllerNode, 15, 5);
	SetAllNodesXY(switches, 5, 12.5, 7.5);
}

void Deliverable2::SetupAppearenceNetAnim() {
	AnimationInterface anim("D2anim.xml");
	uint32_t switchImageID = anim.AddResource("/home/brian-jesse/Downloads/bake/source/ns-3.32/scratch/Switch.png");
	uint32_t workstationImageID = anim.AddResource("/home/brian-jesse/Downloads/bake/source/ns-3.32/scratch/Workstation.png");
	uint32_t SDNImageID = anim.AddResource("/home/brian-jesse/Downloads/bake/source/ns-3.32/scratch/SDN.png");
	anim.UpdateNodeColor(controllerNode, 0, 0, 0);
	anim.SetBackgroundImage("/home/brian-jesse/Downloads/bake/source/ns-3.32/scratch/background.png", -4, -5, 0.03, 0.0375, 1);
	for(uint16_t i = 0; i < allHosts.size(); i++) {
		NodeContainer hosts = allHosts.at(i);
		for(uint16_t j = 0; j < hosts.GetN(); j++) {
			anim.UpdateNodeColor(hosts.Get(j), 0, 255, 255);

		}
	}
	//Controller
	anim.UpdateNodeImage(0, SDNImageID);
	anim.UpdateNodeSize(0, 3, 3);

	//Switch
	for(uint16_t i = 1; i <= 4; i++) {
		anim.UpdateNodeImage(i, switchImageID);
		anim.UpdateNodeSize(i, 3, 3);
	}

	//Hosts
	for(uint16_t i = 5; i <= 16; i++) {
		anim.UpdateNodeSize(i, 2, 2);
		anim.UpdateNodeImage(i, workstationImageID);
	}
}
