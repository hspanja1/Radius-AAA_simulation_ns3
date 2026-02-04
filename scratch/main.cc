/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * AAA (RADIUS-like) Simulation
 * 
 * This simulation demonstrates proper AAA authentication flow:
 * 1. Client sends Access-Request
 * 2. Server processes and responds with Access-Accept/Reject
 * 3. Client handles response and proceeds to next session
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/error-model.h"
#include "ns3/sptm-client-helper.h"
#include "ns3/sptm-sink-helper.h"
#include "ns3/config.h"
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
//include <set>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("RadiusAAA");

// Global metrics
uint32_t m_bytes_sent = 0;
uint32_t m_bytes_received = 0;
uint32_t m_packets_sent = 0;
uint32_t m_packets_received_client= 0;
uint32_t m_packets_received_server = 0;
uint32_t m_packets_rejected = 0;
uint32_t m_packets_accepted = 0;
uint32_t m_packets_request = 0;
std::map<uint32_t, double> m_delayTable;
std::vector<double> m_rttValues;
double m_totalLatency = 0.0;
uint32_t m_latencyCount = 0;
std::map<uint32_t, uint32_t> m_retransmitAttempts_client,m_retransmitAttempts_server;  // mapa koja prati broj pokušaja za svaki UID paketa
uint32_t m_totalRetransmissions = 0;  // broj svih retransmisija
uint32_t m_totalPackets = 0;  // broj svih paketa koji su prošli kroz proces
//std::set<uint32_t, uint32_t> m_packetSeqNumbers;  // Set za čuvanje svih sequence brojeva

void SentPacket (std::string context, Ptr<const Packet> p)
{
  m_bytes_sent += p->GetSize ();
  m_packets_sent++;
  m_delayTable[p->GetUid ()] = Simulator::Now ().GetSeconds ();
  
  std::cout << "[" << Simulator::Now ().GetSeconds () << "s] "
            << "TX: Packet UID=" << p->GetUid () 
            << " (" << p->GetSize () << " bytes)" << std::endl;
}

/*void ReceivedPacket (std::string context, Ptr<const Packet> p, const Address &addr)
{
  m_bytes_received += p->GetSize ();
  m_packets_received++;
  
  auto it = m_delayTable.find (p->GetUid ());
  if (it != m_delayTable.end ())
  {
    double delay = Simulator::Now ().GetSeconds () - it->second;
    double rtt_ms = delay * 1000.0;
    m_rttValues.push_back (rtt_ms);
    m_totalLatency += rtt_ms;
    m_latencyCount++;
    
    std::cout << "[" << Simulator::Now ().GetSeconds () << "s] "
              << "RX: Packet UID=" << p->GetUid () 
              << " - RTT: " << rtt_ms << " ms" << std::endl;
    m_delayTable.erase (it);
  }
}*/
void ReceivedPacket (std::string context, Ptr<const Packet> p, const Address &addr)
{
  m_bytes_received += p->GetSize ();
  //m_packets_received++;
  
  // Prvo ukloni zaglavlje iz paketa da bismo mogli da izvučemo informacije o tipovima paketa
  SptmHeader sptmHead;
  if (p->PeekHeader(sptmHead))  // Ako uspješno uklonimo zaglavlje
  {
    // Dobijanje tipa poruke
    //uint64_t uid = p->GetUid();
    uint8_t messageType = sptmHead.GetMessageType();
    uint32_t seq = sptmHead.GetSeq();
    if (messageType == AAA_ACCESS_REQUEST ) {
        m_packets_received_server++;
        std::cout << "Hali" << std::endl;
        // Broj retransmisija za ovaj UID
        if (m_retransmitAttempts_client.find(seq) == m_retransmitAttempts_client.end()) {
          m_retransmitAttempts_client[seq] = 1;  // Ako još nismo zabilježili pokušaj, zabilježi prvi
          // Prepoznavanje tipa paketa
          m_packets_request++;
        } 
        else {
          m_retransmitAttempts_client[seq]++;  // Ako smo već imali pokušaj, povećaj broj
        }
      }
    else if (messageType == AAA_ACCESS_ACCEPT) {
        
        std::cout << "Halo" << std::endl;
        m_packets_received_client++;
        if (m_retransmitAttempts_server.find(seq) == m_retransmitAttempts_server.end()) {
          m_retransmitAttempts_server[seq] = 1;  // Ako još nismo zabilježili pokušaj, zabilježi prvi
          // Prepoznavanje tipa paketa
          m_packets_accepted++;
        } 
        else {
          m_retransmitAttempts_server[seq]++;  // Ako smo već imali pokušaj, povećaj broj
        }
      }
    else if (messageType == AAA_ACCESS_REJECT) {
        std::cout << "Ola" << std::endl;
        m_packets_received_client++;
        if (m_retransmitAttempts_server.find(seq) == m_retransmitAttempts_server.end()) {
          m_retransmitAttempts_server[seq] = 1;  // Ako još nismo zabilježili pokušaj, zabilježi prvi
          // Prepoznavanje tipa paketa
          m_packets_rejected++;
        } 
        else {
          m_retransmitAttempts_server[seq]++;  // Ako smo već imali pokušaj, povećaj broj
        }
      }
    else if (messageType == AAA_ACCOUNTING_REQUEST) {
        std::cout << "Lala" << std::endl;
        m_packets_received_server++;
        if (m_retransmitAttempts_client.find(seq) == m_retransmitAttempts_client.end()) {
          m_retransmitAttempts_client[seq] = 1;  // Ako još nismo zabilježili pokušaj, zabilježi prvi
          // Prepoznavanje tipa paketa
          m_packets_request++;
        } 
        else {
          m_retransmitAttempts_client[seq]++;  // Ako smo već imali pokušaj, povećaj broj
        }
      }
    else {
        std::cout << "Unknown packet type received." << std::endl;
      }
    
    
    
    
    // Ispis RTT informacija
    auto it = m_delayTable.find (p->GetUid ());
    if (it != m_delayTable.end ()) {
      double delay = Simulator::Now ().GetSeconds () - it->second;
      double rtt_ms = delay * 1000.0;  // RTT u milisekundama
      m_rttValues.push_back (rtt_ms);
      m_totalLatency += rtt_ms;
      m_latencyCount++;
      
      std::cout << "[" << Simulator::Now ().GetSeconds () << "s] "
                << "RX: Packet UID=" << p->GetUid () 
                << " - RTT: " << rtt_ms << " ms" << std::endl;
      m_delayTable.erase (it);
    }
  }
  else {
    std::cout << "Failed to parse packet header." << std::endl;
  }
}


void PacketAccepted (uint32_t count)
{
  m_packets_accepted++;
  std::cout << "[" << Simulator::Now ().GetSeconds () << "s] "
            << "AAA: Access-Accept" << std::endl;
}

void PacketRejected (uint32_t count)
{
  m_packets_rejected++;
  std::cout << "[" << Simulator::Now ().GetSeconds () << "s] "
            << "AAA: Access-Reject" << std::endl;
}

void PrintResults ()
{
  std::cout << "\n";
  std::cout << "╔════════════════════════════════════════╗\n";
  std::cout << "║    AAA SIMULATION FINAL RESULTS       ║\n";
  std::cout << "╠════════════════════════════════════════╣\n";
  std::cout << "║ Total packets sent:     " << m_packets_sent << " ║\n";
  std::cout << "║ Ukupno jedinstvenih paketa poslanih serveru: " << m_retransmitAttempts_server.size()<<" ║\n";
  std::cout << "║ Ukupno jedinstvenih paketa poslanih klijentu: " << m_retransmitAttempts_server.size()<<" ║\n";
  std::cout << "║ Total packets received server: "  << m_packets_received_server << " ║\n";
  std::cout << "║ Total packets received client: "  << m_packets_received_client << " ║\n";
  std::cout << "║ Packets accepted:       " << m_packets_accepted << " ║\n";
  std::cout << "║ Packets rejected:       " << m_packets_rejected << " ║\n";
  std::cout << "║ Total bytes sent:       " << m_bytes_sent << " ║\n";
  std::cout << "║ Total bytes received:   " << m_bytes_received/2 << " ║\n";
  
  if (m_packets_sent > 0)
  {
    double asr = ((double)m_packets_accepted) / ((double)m_packets_accepted+(double)m_packets_rejected) * 100.0;
    std::cout << "║ ASR (Success Rate):     " << asr << " % ║\n";
  }
  
  if (m_latencyCount > 0)
  {
    double avg_latency = m_totalLatency / m_latencyCount;
    std::cout << "║ Average RTT:            " << avg_latency << " ms ║\n";
  }
  
  std::cout << "╚════════════════════════════════════════╝\n";
  std::cout << std::endl;
}

/*void ExportToCSV (std::string filename, double rejectProb, uint32_t nClients, 
                  uint32_t maxPackets, double interval, double errorRate)
{
  std::ofstream csvFile;
  bool fileExists = false;
  
  // Check if file exists
  std::ifstream checkFile (filename.c_str ());
  if (checkFile.good ())
  {
    fileExists = true;
  }
  checkFile.close ();
  
  // Open file in append mode
  csvFile.open (filename.c_str (), std::ios::app);
  
  if (!csvFile.is_open ())
  {
    std::cerr << "ERROR: Cannot open CSV file: " << filename << std::endl;
    return;
  }
  
  // Write header if file is new
  if (!fileExists)
  {
    csvFile << "RejectProbability,NumClients,SessionsPerClient,SessionInterval,"
            << "ErrorRate,PacketsSent,PacketsReceived,PacketsAccepted,"
            << "PacketsRejected,BytesSent,BytesReceived,SuccessRate,"
            << "AverageRTT,MinRTT,MaxRTT,MedianRTT" << std::endl;
  }
  
  // Calculate statistics
  double successRate = 0.0;
  if (m_packets_sent > 0)
  {
    successRate = ((double)m_packets_accepted+(double)m_packets_rejected) / (double)m_packets_sent * 100.0;
  }
  
  double avgRTT = 0.0;
  double minRTT = 0.0;
  double maxRTT = 0.0;
  double medianRTT = 0.0;
  
  if (!m_rttValues.empty ())
  {
    avgRTT = m_totalLatency / m_latencyCount;
    
    std::sort (m_rttValues.begin (), m_rttValues.end ());
    minRTT = m_rttValues.front ();
    maxRTT = m_rttValues.back ();
    
    size_t size = m_rttValues.size ();
    if (size % 2 == 0)
    {
      medianRTT = (m_rttValues[size / 2 - 1] + m_rttValues[size / 2]) / 2.0;
    }
    else
    {
      medianRTT = m_rttValues[size / 2];
    }
  }
  
  // Write data
  csvFile << rejectProb << ","
          << nClients << ","
          << maxPackets << ","
          << interval << ","
          << errorRate << ","
          << m_packets_sent << ","
          << m_packets_accepted+m_packets_rejected << ","
          << m_packets_accepted << ","
          << m_packets_rejected << ","
          << successRate << ","
          << avgRTT << ","
          << minRTT << ","
          << maxRTT << ","
          << medianRTT << std::endl;
  
  csvFile.close ();
  
  std::cout << "Results exported to: " << filename << std::endl;
}*/

void ExportToCSV (std::string filename, double rejectProb, uint32_t nClients, 
                  uint32_t maxPackets, double interval, double errorRate)
{
  std::ofstream csvFile;
  bool fileExists = false;
  
  // Check if file exists
  std::ifstream checkFile (filename.c_str ());
  if (checkFile.good ())
  {
    fileExists = true;
  }
  checkFile.close ();
  
  // Open file in append mode
  csvFile.open (filename.c_str (), std::ios::app);
  
  if (!csvFile.is_open ())
  {
    std::cerr << "ERROR: Cannot open CSV file: " << filename << std::endl;
    return;
  }
  
  // Write header if file is new
  if (!fileExists)
  {
    csvFile << "RejectProbability,NumClients,SessionsPerClient,SessionInterval,"
            << "ErrorRate,PacketsSentTotal,UniquePacketsToServer,UniquePacketsToClient,"
            << "PacketsReceivedServer,PacketsReceivedClient,PacketsAccepted,PacketsRejected,"
            << "BytesSent,BytesReceived,SuccessRate,AverageRTT,MinRTT,MaxRTT,MedianRTT,"
            << "TotalRetransmissionsServer,TotalRetransmissionsClient" << std::endl;
  }
  
  // Calculate statistics
  double successRate = 0.0;
  if (m_packets_sent > 0)
  {
    successRate = ((double)m_packets_accepted + (double)m_packets_rejected) / (double)m_packets_sent * 100.0;
  }
  
  double avgRTT = 0.0;
  double minRTT = 0.0;
  double maxRTT = 0.0;
  double medianRTT = 0.0;
  
  if (!m_rttValues.empty ())
  {
    avgRTT = m_totalLatency / m_latencyCount;
    
    std::sort (m_rttValues.begin (), m_rttValues.end ());
    minRTT = m_rttValues.front ();
    maxRTT = m_rttValues.back ();
    
    size_t size = m_rttValues.size ();
    if (size % 2 == 0)
    {
      medianRTT = (m_rttValues[size / 2 - 1] + m_rttValues[size / 2]) / 2.0;
    }
    else
    {
      medianRTT = m_rttValues[size / 2];
    }
  }
  
  // Calculate retransmissions
  uint32_t totalRetransServer = 0;
  for (auto& kv : m_retransmitAttempts_server)
  {
    if (kv.second > 1)  // Ako je bilo više od 1 pokušaja
    {
      totalRetransServer += (kv.second - 1);  // Broj retransmisija = ukupni pokušaji - 1
    }
  }
  
  uint32_t totalRetransClient = 0;
  for (auto& kv : m_retransmitAttempts_client)
  {
    if (kv.second > 1)
    {
      totalRetransClient += (kv.second - 1);
    }
  }
  
  // Write data
  csvFile << rejectProb << ","
          << nClients << ","
          << maxPackets << ","
          << interval << ","
          << errorRate << ","
          << m_packets_sent << ","
          << m_retransmitAttempts_client.size() << ","  // Jedinstveni paketi serveru
          << m_retransmitAttempts_server.size() << ","  // Jedinstveni paketi klijentu
          << m_packets_received_server << ","
          << m_packets_received_client << ","
          << m_packets_accepted << ","
          << m_packets_rejected << ","
          << m_bytes_sent << ","
          << m_bytes_received / 2 << ","  // Podijeljeno sa 2 kao u PrintResults
          << successRate << ","
          << avgRTT << ","
          << minRTT << ","
          << maxRTT << ","
          << medianRTT << ","
          << totalRetransServer << ","
          << totalRetransClient << std::endl;
  
  csvFile.close ();
  
  std::cout << "Results exported to: " << filename << std::endl;
}


int main (int argc, char *argv[])
{
  // ============ Parametri simulacije ============
  uint32_t nClients = 1;
  uint32_t maxPackets = 10;
  double interval = 1.0;
  double errorRate = 0.0;
  double rejectProbability = 0.0;  // 0% default rejection
  uint32_t processingDelay = 10;   // 10ms server processing
  std::string csvFilename = "aaa_simulation_results.csv";
  bool enableCsvExport = true;
  
  CommandLine cmd;
  cmd.AddValue ("clients", "Number of AAA clients", nClients);
  cmd.AddValue ("packets", "Number of AAA sessions per client", maxPackets);
  cmd.AddValue ("interval", "Interval between sessions (seconds)", interval);
  cmd.AddValue ("errorRate", "Network packet loss rate", errorRate);
  cmd.AddValue ("rejectProb", "Server rejection probability", rejectProbability);
  cmd.AddValue ("procDelay", "Server processing delay (ms)", processingDelay);
  cmd.AddValue ("csvFile", "CSV output filename", csvFilename);
  cmd.AddValue ("enableCsv", "Enable CSV export", enableCsvExport);
  cmd.Parse (argc, argv);
  // =======================================================

  // Enable logging
  LogComponentEnable ("RadiusAAA", LOG_LEVEL_INFO);
  LogComponentEnable ("SptmClient", LOG_LEVEL_INFO);
  LogComponentEnable ("SptmSink", LOG_LEVEL_INFO);

  std::cout << "\n";
  std::cout << "╔════════════════════════════════════════╗\n";
  std::cout << "║     AAA PROTOCOL SIMULATION START     ║\n";
  std::cout << "╠════════════════════════════════════════╣\n";
  std::cout << "║ Number of clients:      " << nClients << " ║\n";
  std::cout << "║ Sessions per client:    "  << maxPackets << " ║\n";
  std::cout << "║ Session interval:       "  << interval << " s ║\n";
  std::cout << "║ Network error rate:     "  << errorRate << "   ║\n";
  std::cout << "║ Rejection probability:  "  << rejectProbability << "   ║\n";
  std::cout << "║ Processing delay:       " << processingDelay << " ms║\n";
  std::cout << "╚════════════════════════════════════════╝\n";
  std::cout << std::endl;

  Time::SetResolution (Time::NS);

  NodeContainer nodes;
  nodes.Create (2);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("5ms"));
  
  NetDeviceContainer devices = p2p.Install (nodes);

  // ============ DODATO: Optional error model ============
  if (errorRate > 0.0)
  {
    Ptr<RateErrorModel> errorModel = CreateObject<RateErrorModel> ();
    errorModel->SetAttribute ("ErrorRate", DoubleValue (errorRate));
    devices.Get (1)->SetAttribute ("ReceiveErrorModel",
                                   PointerValue (errorModel));
  }
  // =======================================================

  InternetStackHelper internet;
  internet.Install (nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  uint16_t port = 1812;
  Ipv4Address serverAddr = interfaces.GetAddress (1);
  Ipv4Address clientAddr = interfaces.GetAddress (0);
  std::cout << "AAA Server address: " << serverAddr << ":" << port << "\n\n";

  // ============ PROMIJENJENO: SptmSink sa AAA response capability ============
  SptmSinkHelper aaaServer ("ns3::UdpSocketFactory",
                            InetSocketAddress (Ipv4Address::GetAny (), port));
  aaaServer.SetAttribute ("SendResponse", BooleanValue (true));  // ============
  aaaServer.SetAttribute ("ProcessingDelay", 
                         TimeValue (MilliSeconds (processingDelay)));
  aaaServer.SetAttribute ("RejectProbability", 
                         DoubleValue (rejectProbability));
  
  ApplicationContainer serverApp = aaaServer.Install (nodes.Get (1));
  serverApp.Start (Seconds (0.0));
  serverApp.Stop (Seconds (50.0));
  // ============================================================================

  // ============ PROMIJENJENO: SptmClient sa pravim AAA parametrima ============
  SptmClientHelper aaaClient ("ns3::UdpSocketFactory",
                              InetSocketAddress (serverAddr, port));
  aaaClient.SetAttribute ("PacketSize", UintegerValue (150));
  aaaClient.SetAttribute ("MaxPackets", UintegerValue (maxPackets));
  aaaClient.SetAttribute ("Interval", TimeValue (Seconds (interval)));
  aaaClient.SetAttribute ("FlowNumber", UintegerValue (1));
  aaaClient.SetAttribute ("MaxRetries", UintegerValue (2));
  aaaClient.SetAttribute ("Timeout", TimeValue (MilliSeconds (500)));

  ApplicationContainer clientApp = aaaClient.Install (nodes.Get (0));
  clientApp.Start (Seconds (1.0));
  clientApp.Stop (Seconds (50.0));
  // =============================================================================

    // Tracing
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::SptmClient/Tx",
                   MakeCallback (&SentPacket));
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::SptmSink/Rx",
                   MakeCallback (&ReceivedPacket));
  Config::Connect ("/NodeList/*/ApplicationList/*/$ns3::SptmSink/Tx",
                   MakeCallback (&ReceivedPacket));
  
  p2p.EnablePcapAll ("proba");
  
  
  Simulator::Stop (Seconds (50.0));
  Simulator::Run ();

  PrintResults ();
  
  // Export to CSV if enabled
  if (enableCsvExport)
  {
    ExportToCSV (csvFilename, rejectProbability, nClients, maxPackets, 
                 interval, errorRate);
  }
  
  Simulator::Destroy ();
  return 0;
}
