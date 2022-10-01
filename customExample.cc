/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-routing-table-entry.h"

// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1
//    point-to-point
//
 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
	CommandLine cmd (__FILE__);
	cmd.Parse (argc, argv);

	Time::SetResolution (Time::NS);//Nanoseconds
	LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_ALL);
	LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_ALL);

	NodeContainer nodes;
	nodes.Create (3);

	CsmaHelper pointToPoint;
	pointToPoint.SetChannelAttribute ("DataRate", DataRateValue (5000000));
	pointToPoint.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

	NodeContainer left, right;
	left.Add(nodes.Get(0)); left.Add(nodes.Get(1));
	right.Add(nodes.Get(1)); right.Add(nodes.Get(2));
	NetDeviceContainer leftDevices, rightDevices;
	leftDevices = pointToPoint.Install (left);
	rightDevices = pointToPoint.Install (right);

	InternetStackHelper stack;
	stack.Install (nodes);

	Ipv4AddressHelper address;
	address.SetBase ("10.1.1.0", "255.255.255.0");

	std::set<Ptr<NetDevice>> southBoundDevices;
	for (size_t deviceIndex = 0; deviceIndex < 2; deviceIndex++) {
		southBoundDevices.insert(leftDevices.Get(deviceIndex));
		southBoundDevices.insert(rightDevices.Get(deviceIndex));
	}
	NetDeviceContainer devices;
	std::set<Ptr<NetDevice>>:: iterator it;
	for( it = southBoundDevices.begin(); it!=southBoundDevices.end(); ++it){
		devices.Add(*it);
	}
	address.Assign (devices);
	Ipv4InterfaceContainer interfaces = address.Assign(devices);

	V4PingHelper ping("10.1.1.2");
	ping.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	ping.SetAttribute ("Size", UintegerValue (1024));
	ping.SetAttribute ("Verbose", BooleanValue (true));

	ApplicationContainer apps = ping.Install (nodes.Get(2));
	apps.Start (Seconds (1.0));
	apps.Stop (Seconds (110.0));

	Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
/*
	UdpEchoServerHelper echoServer (9);

	ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));
	serverApps.Start (Seconds (1.0));
	serverApps.Stop (Seconds (10.0));

	UdpEchoClientHelper echoClient (Ipv4Address("10.1.1.2"), 9);//remote and port
	echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
	echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
	echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

	ApplicationContainer clientApps = echoClient.Install (nodes.Get (2));
	clientApps.Start (Seconds (2.0));
	clientApps.Stop (Seconds (10.0));
*/
	Simulator::Run ();
	Simulator::Destroy ();
	return 0;
}
