#include "ns3/applications-module.h"
#include "ns3/cone-antenna.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/measured-2d-antenna.h"
#include "ns3/mobility-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"

/*
 * This examples uses Native ns3 Internet Stack + ns3 Native Application.
 *
 * We use an OnOffApplication to send Traffic using ns3 TCP over P2P link.
 * The traffic goes through a simple network composed of Client PC + Access Router + End Server with Sink Application.
 *
 */

NS_LOG_COMPONENT_DEFINE ("test-tcp-p2p");

using namespace ns3;
using namespace std;

Ptr<PacketSink> sink;                           /* TCP Receiver (Generic Traffic Receiver Application) */
NodeContainer nodes;

/*** Simulation Attributes ***/
uint32_t payloadSize = 1400;                    /* Transport Layer Payload size in bytes. */
bool pcap_tracing = false;                      /* PCAP Tracing is enabled or not. */
double simulationTime = 10.1;                   /* Simulation time in seconds. */
uint64_t lastTotalRx = 0;

void
Progress()
{
  Time now = Simulator::Now () - Seconds (1);                           /* Return the simulator's virtual time. */
  double cur = (sink->GetTotalRx () - lastTotalRx) * (double) 8/1e6;    /* Convert Application RX Packets to MBits. */
  cout << now.GetSeconds () << '\t' << cur << endl;
  lastTotalRx = sink->GetTotalRx ();
  Simulator::Schedule (Seconds(1), Progress);
}

/**
 * Log packet drops.
 */
static void
MacTxpDrop(Ptr<const Packet> p)
{
  NS_LOG_UNCOND("Mac Drop at " << Simulator::Now ().GetSeconds ());
}

static void
PhyTxpDrop (Ptr<const Packet> p)
{
  NS_LOG_UNCOND("Phy Drop at " << Simulator::Now ().GetSeconds ());
}

int
main(int argc, char *argv[])
{
  string applicationType = "bulk";              /* Type of the Tx application */
  uint32_t maxPackets = 0;                      /* Maximum Number of Packets */
  string dataRate = "3Gbps";                    /* Application Layer Data Rate. */
  string tcpVariant = "ns3::TcpNewReno";        /* TCP Variant Type. */
  uint32_t bufferSize = 131072;                 /* TCP Send/Receive Buffer Size. */

  /* Command line argument parser setup. */
  CommandLine cmd;

  cmd.AddValue ("applicationType", "Type of the Tx Application: onoff, bulk", applicationType);
  cmd.AddValue ("maxPackets", "Maximum number of packets to send", maxPackets);
  cmd.AddValue ("payloadSize", "Payload size in bytes", payloadSize);
  cmd.AddValue ("dataRate", "Payload size in bytes", dataRate);
  cmd.AddValue ("tcpVariant", "Transport protocol to use: TcpTahoe, TcpReno, TcpNewReno, TcpWestwood, TcpWestwoodPlus ", tcpVariant);
  cmd.AddValue ("bufferSize", "TCP Send/Receive Buffer Size", bufferSize);
  cmd.AddValue ("simulationTime", "Simulation time in Seconds", simulationTime);
  cmd.AddValue ("pcap", "Enable PCAP Tracing", pcap_tracing);
  cmd.Parse (argc, argv);

  /* Instantiate the Output. */
  cout << "Simulator Time" << '\t' << "Application Goodput (Mbps)" << endl;

  /*** Configure TCP Options ***/

  /* Select TCP variant */
  TypeId tid = TypeId::LookupByName (tcpVariant);
  Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (tid));
  /* Configure TCP Segment Size */
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
  Config::SetDefault ("ns3::TcpSocket::SndBufSize", UintegerValue (bufferSize));
  Config::SetDefault ("ns3::TcpSocket::RcvBufSize", UintegerValue (bufferSize));

  /*** Create Network Topology ***/
  PointToPointHelper p2pHelper;
  p2pHelper.SetDeviceAttribute ("DataRate", StringValue ("5Gbps"));
  p2pHelper.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (3)));
  p2pHelper.SetQueue ("ns3::DropTailQueue", "MaxPackets", UintegerValue (10000));

  nodes.Create (2);

  /* Give all nodes a Linux Stack */
  InternetStackHelper stack;
  stack.Install (nodes);

  /* Assign IP Addresses */
  NetDeviceContainer devices;
  Ipv4AddressHelper ipv4;
  Ipv4InterfaceContainer interfaces;
  ipv4.SetBase ("10.0.0.0", "255.255.0.0");

  /* Configure links between the Edge Router and the End User */
  devices = p2pHelper.Install (nodes.Get (0), nodes.Get (1));
  interfaces = ipv4.Assign (devices);
  ipv4.NewNetwork ();

  /* Setup IP Routes */
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  /* Receiver (Installed on the Wireless Station) */
  PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny (), 9999));
  ApplicationContainer sinkApp = sinkHelper.Install (nodes.Get (0));
  sink = StaticCast<PacketSink> (sinkApp.Get(0));
  sinkApp.Start (Seconds (0.0));
  sinkApp.Stop (Seconds (simulationTime));

  /* Sender (Install on the End Server)*/
  Address dest (InetSocketAddress (interfaces.GetAddress (0), 9999));
  ApplicationContainer srcApp;
  if (applicationType == "onoff")
    {
      OnOffHelper src ("ns3::TcpSocketFactory", dest);
      src.SetAttribute ("MaxBytes", UintegerValue (0));
      src.SetAttribute ("PacketSize", UintegerValue (payloadSize));
      src.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1e6]"));
      src.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      src.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate)));
      srcApp = src.Install (nodes.Get (1));
    }
  else if (applicationType == "bulk")
    {
      BulkSendHelper src ("ns3::TcpSocketFactory", dest);
      src.SetAttribute ("SendSize", UintegerValue (payloadSize));
      src.SetAttribute ("MaxBytes", UintegerValue (maxPackets));
      srcApp = src.Install (nodes.Get (1));
    }
  srcApp.Start (Seconds (1.0));

  Config::ConnectWithoutContext ("/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/MacTxDrop", MakeCallback (&MacTxpDrop));
  Config::ConnectWithoutContext ("/NodeList/2/DeviceList/*/$ns3::PointToPointNetDevice/PhyTxDrop", MakeCallback (&PhyTxpDrop));

  if (pcap_tracing)
    {
      p2pHelper.EnablePcap ("EndServer", devices.Get (0), false);
      p2pHelper.EnablePcap ("User", devices.Get (1), false);
    }

  /* Run Simulation */
  Simulator::Schedule (Seconds (2.0), Progress);
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();

  /* Print Results */
  Time now = Simulator::Now() - Seconds (1);
  cout << "Statistics at the Receiver:"  << endl;
  cout << "Average goodput seen by the application = " << (sink->GetTotalRx() * (double) 8/ 1e6)/now.GetSeconds () << endl;
  cout << "End Simulation at " << now.GetSeconds () << endl;

  Simulator::Destroy ();

  return 0;
}
