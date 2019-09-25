#include <string>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"

using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("TcpBulkSendExample");
int main (int argc, char *argv[])
{

  bool tracing = false;
  uint32_t maxBytes = 0;

//
// Allow the user to override any of the defaults at
// run-time, via command-line arguments
//
  CommandLine cmd;
  cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
  cmd.AddValue ("maxBytes",
                "Total number of bytes for application to send", maxBytes);
  cmd.Parse (argc, argv);

//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");

  NodeContainer nodes;
  nodes.Create (3);


  NodeContainer nodes12 = NodeContainer (nodes.Get (0), nodes.Get (1));
  NodeContainer nodes23 = NodeContainer (nodes.Get (1), nodes.Get (2));

//
// Explicitly create the point-to-point link required by the topology (shown above).
//
  PointToPointHelper pointToPoint12;
  pointToPoint12.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint12.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices12;
  devices12 = pointToPoint12.Install (nodes12);

  PointToPointHelper pointToPoint23;
  pointToPoint23.SetDeviceAttribute ("DataRate", StringValue ("500Kbps"));
  pointToPoint23.SetChannelAttribute ("Delay", StringValue ("5ms"));

  NetDeviceContainer devices23;
  devices23 = pointToPoint23.Install (nodes23);

//
// Install the internet stack on the nodes
//
  InternetStackHelper internet;
  internet.Install (nodes12.Get(0));
  internet.Install (nodes23);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices12);

  Ipv4AddressHelper ipv42;
  ipv42.SetBase ("10.2.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i2 = ipv4.Assign (devices23);

//
// Create a BulkSendApplication and install it on node 0
//
  uint16_t port = 9;  // well-known echo port number


  BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (i.GetAddress (1), port));
  // Set the amount of data to send in bytes.  Zero is unlimited.
  source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
  ApplicationContainer sourceApps = source.Install (nodes12.Get (0));
  sourceApps.Start (Seconds (1.0));
  sourceApps.Stop (Seconds (10.0));

//
// Create a PacketSinkApplication and install it on node 1
//
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (nodes12.Get (1));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (10.0));

  //
  // Create ONOff Application
  //

  OnOffHelper onoff ("ns3::TcpSocketFactory", Address (InetSocketAddress (i2.GetAddress (0), port)));
   onoff.SetConstantRate (DataRate ("448kb/s"));
   ApplicationContainer apps = onoff.Install (nodes23.Get (1));
   apps.Start (Seconds (1.0));
   apps.Stop (Seconds (10.0));

//
// Set up tracing if enabled
//
   /*
  if (tracing)
    {
      AsciiTraceHelper ascii;
      pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("tcp-bulk-send.tr"));
      pointToPoint.EnablePcapAll ("tcp-bulk-send", false);
    }*/
	
//
// Now, do the actual simulation.
//
    
  //NS_LOG_INFO ("Run Simulation.");

  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;
  flowMonitor = flowHelper.InstallAll();


  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  flowMonitor->SerializeToXmlFile("tcp-bulk-send.xml", true, true);
  Simulator::Destroy ();
  //NS_LOG_INFO ("Done.");

  Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));
  std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
}
