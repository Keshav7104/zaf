#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/gnuplot.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/wifi-module.h"

#include <iostream>
#include <map>
#include <unordered_set>

uint16_t TOTAL_DRONES = 2;

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ZAF");

Vector destPos;
Ipv4Address destAddr;
uint16_t srcNode;
uint32_t packetIdCounter;

enum Zone
{
    // Zone corresponding to Quadrant 1
    Z_I,
    // Zone corresponding to Quadrant 2
    Z_II,
    // Zone corresponding to Quadrant 3
    Z_III,
    // Zone corresponding to Quadrant 4
    Z_IV
};

// helloHeader Class
class HelloHeader : public Header
{
  public:
    HelloHeader()
    {
    }

    virtual ~HelloHeader()
    {
    }

    // Setters
    void SetSrc(Ipv4Address srcAddr)
    {
        m_src = srcAddr;
    }

    void SetDest(Ipv4Address destAddr)
    {
        m_dest = destAddr;
    }

    void SetSrcPos(Vector srcPosition)
    {
        m_srcPos = srcPosition;
    }

    void SetNeighbourCount(uint16_t count)
    {
        m_neighbourCount = count;
    }

    void SetTimeStamp(Time creation)
    {
        m_timeStamp = creation;
    }

    // Getters
    Ipv4Address GetSrc() const
    {
        return m_src;
    }

    Ipv4Address GetDest() const
    {
        return m_dest;
    }

    Vector GetSrcPos() const
    {
        return m_srcPos;
    }

    uint16_t GetNeighbourCount() const
    {
        return m_neighbourCount;
    }

    Time GetTimeStamp() const
    {
        return m_timeStamp;
    }

    uint8_t GetPacketType() const
    {
        return m_packetType;
    }

    // Required Header methods
    static TypeId GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::HelloHeader")
                                .SetParent<Header>()
                                .SetGroupName("ZAF")
                                .AddConstructor<HelloHeader>();
        return tid;
    }

    virtual TypeId GetInstanceTypeId(void) const
    {
        return GetTypeId();
    }

    virtual void Serialize(Buffer::Iterator i) const
    {
        WriteTo(i, m_src);
        WriteTo(i, m_dest);
        i.WriteU16(m_neighbourCount);
        i.WriteHtonU64(m_srcPos.x);
        i.WriteHtonU64(m_srcPos.y);
        i.WriteHtonU64(m_srcPos.z);
        uint64_t ns = static_cast<uint64_t>(m_timeStamp.GetDouble() * 1e9);
        i.WriteHtonU64(ns);
        i.WriteU8(m_packetType);
    }

    virtual uint32_t Deserialize(Buffer::Iterator start)
    {
        Buffer::Iterator i = start;
        ReadFrom(i, m_src);
        ReadFrom(i, m_dest);
        m_neighbourCount = i.ReadU16();
        m_srcPos.x = i.ReadNtohU64();
        m_srcPos.y = i.ReadNtohU64();
        m_srcPos.z = i.ReadNtohU64();
        uint64_t ns = i.ReadNtohU64();
        m_timeStamp = NanoSeconds(ns);
        m_packetType = i.ReadU8();
        return i.GetDistanceFrom(start);
    }

    virtual uint32_t GetSerializedSize(void) const
    {
        return 43;
    }

    virtual void Print(std::ostream& os) const
    {
        os << "HelloHeader Info";
    }

  private:
    Ipv4Address m_src;
    Ipv4Address m_dest;
    Vector m_srcPos;
    uint16_t m_neighbourCount;
    Time m_timeStamp;
    uint8_t m_packetType = 0;
};

// zafHeader Class
class ZafHeader : public Header
{
  public:
    ZafHeader()
    {
    }

    virtual ~ZafHeader()
    {
    }

    ZafHeader Copy()
    {
        return (*this);
    }

    // Getters
    Vector GetDestPos() const
    {
        return m_destPos;
    }

    uint16_t GetDestNumber() const
    {
        return m_destNumber;
    }

    Vector GetLastNodePos() const
    {
        return m_lastNodePos;
    }

    uint32_t GetPacketId() const
    {
        return m_packetId;
    }

    Ipv4Address GetDestAddr() const
    {
        return m_destAddr;
    }

    Ipv4Address GetSrcAddr() const
    {
        return m_srcAddr;
    }

    uint8_t GetPacketType() const
    {
        return m_packetType;
    }

    Ipv4Address GetLastNodeAddr() const
    {
        return m_lastNodeAddr;
    }

    // Setters
    void SetDestPos(Vector pos)
    {
        m_destPos = pos;
    }

    void SetDestNumber(uint16_t number)
    {
        m_destNumber = number;
    }

    void SetLastNodePos(Vector pos)
    {
        m_lastNodePos = pos;
    }

    void SetPacketId(uint32_t id)
    {
        m_packetId = id;
    }

    void SetDestAddr(Ipv4Address addr)
    {
        m_destAddr = addr;
    }

    void SetSrcAddr(Ipv4Address addr)
    {
        m_srcAddr = addr;
    }

    void SetLastNodeAddr(Ipv4Address addr)
    {
        m_lastNodeAddr = addr;
    }

    // Required Header methods
    static TypeId GetTypeId(void)
    {
        static TypeId tid = TypeId("ns3::ZafHeader")
                                .SetParent<Header>()
                                .SetGroupName("ZAF")
                                .AddConstructor<ZafHeader>();
        return tid;
    }

    virtual TypeId GetInstanceTypeId(void) const
    {
        return GetTypeId();
    }

    virtual void Serialize(Buffer::Iterator i) const
    {
        WriteTo(i, m_srcAddr);
        WriteTo(i, m_destAddr);
        WriteTo(i, m_lastNodeAddr);
        i.WriteU32(m_packetId);
        i.WriteU16(m_destNumber);
        i.WriteHtonU64(m_destPos.x);
        i.WriteHtonU64(m_destPos.y);
        i.WriteHtonU64(m_destPos.z);
        i.WriteHtonU64(m_lastNodePos.x);
        i.WriteHtonU64(m_lastNodePos.y);
        i.WriteHtonU64(m_lastNodePos.z);
    }

    virtual uint32_t Deserialize(Buffer::Iterator start)
    {
        Buffer::Iterator i = start;
        ReadFrom(i, m_srcAddr);
        ReadFrom(i, m_destAddr);
        ReadFrom(i, m_lastNodeAddr);
        m_packetId = i.ReadU32();
        m_destNumber = i.ReadU16();
        m_destPos.x = i.ReadNtohU64();
        m_destPos.y = i.ReadNtohU64();
        m_destPos.z = i.ReadNtohU64();
        m_lastNodePos.x = i.ReadNtohU64();
        m_lastNodePos.y = i.ReadNtohU64();
        m_lastNodePos.z = i.ReadNtohU64();
        return i.GetDistanceFrom(start);
    }

    virtual uint32_t GetSerializedSize(void) const
    {
        return 66;
    }

    virtual void Print(std::ostream& os) const
    {
        os << "ZafHeader Info";
    }

  private:
    Vector m_destPos;
    uint16_t m_destNumber;
    Vector m_lastNodePos;
    uint32_t m_packetId;
    Ipv4Address m_destAddr;
    Ipv4Address m_srcAddr;
    Ipv4Address m_lastNodeAddr;
    uint8_t m_packetType = 1;
};

class App : public Application
{
  public:
    App(uint16_t nodeNumber);
    virtual ~App();
    void Setup(Ipv4Address appAddr);
    // Start broadcast from a node
    void SendHello(bool immediate = false);
    void SendHello(double delay);
    // Send Packet to last node of the network
    void SendMessage(Vector destPos, Ipv4Address destAddr);

    static uint32_t totalSent;
    static uint32_t totalReceived;
    static uint32_t noNegibhours;

  private:
    virtual void StartApplication(void);
    virtual void StopApplication(void);

    // Callback for receiving the Directed Hello message
    void HandleHello(Ptr<Socket> socket);
    // Callback for receiving the broadcast discovery on port 5000.
    void HandleBroadcast(Ptr<Socket> socket);
    // Callback for receiving the secret message on port 7000.
    void HandleMessage(Ptr<Socket> socket);
    // Send broadcast discovery packet
    void SendBroadcast();
    // Update Neighbour table
    void UpdateNeighbourTable(Ipv4Address node, Vector nodePos, Time timeStamp);
    // Clear Neighbour table
    void Purge();
    // Select Best Neighbour from the table
    std::vector<Ipv4Address> SelectBestNeighbour(Vector destPos,
                                                 Vector myPosi,
                                                 Ipv4Address destAddr);
    // print Neighbour Table on the console
    void PrintTable();
    // Determine Zone of given drone with respect to current done
    Zone GetZone(Vector destPos);
    // Calculate Distance between 2 Vectors
    double CalculateDistance(Vector a, Vector b);

    Ptr<Socket>
        m_helloBroadcastSocket; // Socket to receive broadcast discovery (port m_helloEndPort)
    Ptr<Socket> m_singleHelloRecieveSocket; // Socket to receive Data messages (port m_confEndPort)
    Ptr<Socket> m_helloSendSocket;          // Socket to send configuration message to Node1
    Ptr<Socket> m_dataSendSocket;           // Socket to send data messages
    Ptr<Socket> m_dataReciveSocket;         // Socket to recieve data messages
    Ipv4Address m_ipv4;                     // This node's IP address
    // Ipv4Address m_node2Addr;                // Will be determined when config arrives
    uint16_t m_helloEndPort = 5000; // For hello/broadcast messages
    uint16_t m_confEndPort = 6000;  // For configuration  messages
    uint16_t m_dataEndPort = 8080;  // For data messages
    std::map<Ipv4Address, std::pair<Vector, Time>> m_neighbourTable;
    uint16_t m_nodeNumber;
    std::unordered_set<uint32_t> packetCache;
};

App::App(uint16_t nodeNumber)
    : m_helloBroadcastSocket(0),
      m_singleHelloRecieveSocket(0),
      m_helloSendSocket(0),
      m_dataSendSocket(0),
      m_dataReciveSocket(0),
      m_nodeNumber(nodeNumber)
{
}

App::~App()
{
    m_helloBroadcastSocket = 0;
    m_singleHelloRecieveSocket = 0;
    m_helloSendSocket = 0;
    m_dataSendSocket = 0;
    m_dataReciveSocket = 0;
}

void
App::Setup(Ipv4Address appAddress)
{
    m_ipv4 = appAddress;
}

void
App::StartApplication(void)
{
    // Create a UDP socket to receive configuration messages on m_confEndPort.
    if (!m_singleHelloRecieveSocket)
    {
        m_singleHelloRecieveSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_confEndPort);
        if (m_singleHelloRecieveSocket->Bind(local) == -1)
        {
            NS_LOG_ERROR("Failed to bind dataReceiveSocket on port " << m_confEndPort);
            return;
        }
        m_singleHelloRecieveSocket->SetRecvCallback(MakeCallback(&App::HandleHello, this));
    }

    // Create a UDP socket for sending packets.
    if (!m_dataSendSocket)
    {
        m_dataSendSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
    }

    // Socket to receive broadcast discovery on m_helloEndPort.
    if (!m_helloBroadcastSocket)
    {
        m_helloBroadcastSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_helloEndPort);
        if (m_helloBroadcastSocket->Bind(local) == -1)
        {
            NS_LOG_ERROR("Failed to bind helloBroadcastSocket on port " << m_helloEndPort);
            return;
        }
        m_helloBroadcastSocket->SetRecvCallback(MakeCallback(&App::HandleBroadcast, this));
        m_helloBroadcastSocket->SetAllowBroadcast(true);
    }

    // Socket to send configuration messages.
    if (!m_helloSendSocket)
    {
        m_helloSendSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        m_helloSendSocket->SetAllowBroadcast(true);
    }

    // Socket to recieve Data messages.
    if (!m_dataReciveSocket)
    {
        m_dataReciveSocket = Socket::CreateSocket(GetNode(), UdpSocketFactory::GetTypeId());
        InetSocketAddress local = InetSocketAddress(m_ipv4, m_dataEndPort);
        if (m_dataReciveSocket->Bind(local) == -1)
        {
            NS_LOG_ERROR("Failed to bind dataReceiveSocket on port " << m_dataEndPort);
            return;
        }
        m_dataReciveSocket->SetRecvCallback(MakeCallback(&App::HandleMessage, this));
    }
    // if (m_nodeNumber == 1)
    //     Simulator::Schedule(Seconds(5.0), &App::SendMessage, this, destPos, destAddr);
}

void
App::StopApplication(void)
{
    if (m_singleHelloRecieveSocket)
    {
        m_singleHelloRecieveSocket->Close();
        m_singleHelloRecieveSocket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
    if (m_dataSendSocket)
    {
        m_dataSendSocket->Close();
    }
    if (m_helloBroadcastSocket)
    {
        m_helloBroadcastSocket->Close();
        m_helloBroadcastSocket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
    if (m_helloSendSocket)
    {
        m_helloSendSocket->Close();
    }
    if (m_dataReciveSocket)
    {
        m_dataReciveSocket->Close();
        m_dataReciveSocket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
    }
}

void
App::SendBroadcast()
{
    NS_LOG_DEBUG(std::to_string(m_nodeNumber) + " sending broadcast discovery packet");
    Ptr<Packet> packet = Create<Packet>((uint8_t*)"DISCOVER", 8);

    Ptr<MobilityModel> mobilityModel = GetNode()->GetObject<MobilityModel>();
    HelloHeader helloHdr;
    helloHdr.SetDest(Ipv4Address::GetZero());
    helloHdr.SetSrc(m_ipv4);
    helloHdr.SetNeighbourCount(m_neighbourTable.size());
    helloHdr.SetSrcPos(mobilityModel->GetPosition());
    helloHdr.SetTimeStamp(Simulator::Now());
    packet->AddHeader(helloHdr);

    InetSocketAddress broadcastAddr =
        InetSocketAddress(Ipv4Address("255.255.255.255"), m_helloEndPort);
    m_helloSendSocket->SendTo(packet, 0, broadcastAddr);
}

void
App::HandleHello(Ptr<Socket> socket)
{
    Address from;
    Ptr<Packet> packet = socket->RecvFrom(from);
    if (packet == nullptr)
    {
        NS_LOG_ERROR("Received null packet in HandleRead");
        return;
    }
    uint8_t buffer[100];
    packet->CopyData(buffer, packet->GetSize());
    std::string data(reinterpret_cast<char*>(buffer), packet->GetSize());
    InetSocketAddress addr = InetSocketAddress::ConvertFrom(from);
    NS_LOG_DEBUG(std::to_string(m_nodeNumber) + " received config message: " << data << " from "
                                                                             << addr.GetIpv4());

    HelloHeader posHeader;
    packet->RemoveHeader(posHeader);

    UpdateNeighbourTable(posHeader.GetSrc(), posHeader.GetSrcPos(), posHeader.GetTimeStamp());
}

void
App::SendMessage(Vector destPos, Ipv4Address destAddr)
{
    NS_LOG_INFO(std::to_string(m_nodeNumber) + " sending message to " << destAddr);

    Ptr<MobilityModel> MM = GetNode()->GetObject<MobilityModel>();
    Ptr<Packet> packet = Create<Packet>();
    ZafHeader zafHeader;
    Vector myPosi = MM->GetPosition();
    zafHeader.SetDestNumber(TOTAL_DRONES);
    zafHeader.SetDestAddr(destAddr);
    zafHeader.SetDestPos(destPos);
    zafHeader.SetLastNodePos(myPosi);
    zafHeader.SetPacketId(++packetIdCounter);
    zafHeader.SetSrcAddr(m_ipv4);
    zafHeader.SetLastNodeAddr(m_ipv4);
    packet->AddHeader(zafHeader);
    Purge();
    std::vector<Ipv4Address> bestNeighbours = SelectBestNeighbour(destPos, myPosi, destAddr);
    for (auto bestNeighbour : bestNeighbours)
    {
        InetSocketAddress remote = InetSocketAddress(bestNeighbour, m_dataEndPort);
        m_dataSendSocket->SendTo(packet, 0, remote);
        NS_LOG_INFO(std::to_string(m_nodeNumber) + " message sent from SendMessage to "
                    << bestNeighbour);
    }
    App::totalSent++;
}

void
App::HandleBroadcast(Ptr<Socket> socket)
{
    Address from;
    Ptr<Packet> packet = socket->RecvFrom(from);
    if (packet == nullptr)
    {
        NS_LOG_ERROR("Received null packet in HandleBroadcast");
        return;
    }
    HelloHeader helloHdr;
    packet->RemoveHeader(helloHdr);
    Vector senderPos = helloHdr.GetSrcPos();
    Ipv4Address senderAddress = helloHdr.GetSrc();
    Time timeStamp = helloHdr.GetTimeStamp();
    InetSocketAddress addr = InetSocketAddress::ConvertFrom(from);

    NS_LOG_DEBUG(std::to_string(m_nodeNumber) + " received broadcast from " << addr.GetIpv4());
    UpdateNeighbourTable(senderAddress, senderPos, timeStamp);

    Ptr<Packet> configPacket = Create<Packet>();

    Ptr<MobilityModel> mobilityModel = GetNode()->GetObject<MobilityModel>();
    HelloHeader posHeader;
    posHeader.SetDest(Ipv4Address::GetZero());
    posHeader.SetSrc(m_ipv4);
    posHeader.SetNeighbourCount(m_neighbourTable.size());
    posHeader.SetSrcPos(mobilityModel->GetPosition());
    posHeader.SetTimeStamp(Simulator::Now());
    configPacket->AddHeader(posHeader);
    // Send configuration message to the sender on data port.
    InetSocketAddress remote = InetSocketAddress(addr.GetIpv4(), m_confEndPort);
    NS_LOG_DEBUG(std::to_string(m_nodeNumber) + " sending configuration message to Node1 at "
                 << addr.GetIpv4());
    m_helloSendSocket->SendTo(configPacket, 0, remote);
}

void
App::HandleMessage(Ptr<Socket> socket)
{
    Address from;
    Ptr<Packet> packet = socket->RecvFrom(from);
    InetSocketAddress inetFrom = InetSocketAddress::ConvertFrom(from);
    Ipv4Address senderAddr = inetFrom.GetIpv4();

    NS_LOG_INFO(std::to_string(m_nodeNumber) + " Recived packet from " << senderAddr);
    if (packet == nullptr)
    {
        NS_LOG_INFO("Received null packet in HandleMessage");
        return;
    }

    ZafHeader zafHeader;
    packet->RemoveHeader(zafHeader);
    UpdateNeighbourTable(zafHeader.GetLastNodeAddr(), zafHeader.GetLastNodePos(), Simulator::Now());
    if (packetCache.find(zafHeader.GetPacketId()) != packetCache.end())
    {
        NS_LOG_INFO("Duplicate...Dropping....");
        return;
    }
    if (zafHeader.GetDestNumber() == m_nodeNumber)
    {
        NS_LOG_INFO("Wallah!!! Packet Recieved");
        App::totalReceived++;
        return;
    }
    else
    {
        packetCache.emplace(zafHeader.GetPacketId());
        ZafHeader sendHeader = zafHeader.Copy();
        Ptr<MobilityModel> MM = GetNode()->GetObject<MobilityModel>();
        Vector myPosi = MM->GetPosition();
        sendHeader.SetLastNodePos(myPosi);
        Ptr<Packet> sendPacket = Create<Packet>();
        sendHeader.SetLastNodeAddr(m_ipv4);
        sendPacket->AddHeader(sendHeader);
        Purge();
        std::vector<Ipv4Address> bestNeighbours =
            SelectBestNeighbour(zafHeader.GetDestPos(), myPosi, zafHeader.GetDestAddr());
        for (auto bestNeighbour : bestNeighbours)
        {
            InetSocketAddress remote = InetSocketAddress(bestNeighbour, m_dataEndPort);
            m_dataSendSocket->SendTo(sendPacket, 0, remote);
            // NS_LOG_INFO("debug");
            NS_LOG_INFO(std::to_string(m_nodeNumber) + " message sent to via HandleMessage "
                        << bestNeighbour);
        }
    }
}

void
App::SendHello(bool immediate)
{
    // Increase delay range to avoid collisions when many nodes are present.
    double delay = immediate ? 0 : m_nodeNumber;
    NS_LOG_DEBUG(std::to_string(m_nodeNumber) + " starting application; scheduling broadcast in "
                 << delay << " seconds");
    Simulator::Schedule(Seconds(delay), &App::SendBroadcast, this);
}

void
App::SendHello(double delay)
{
    // Increase delay range to avoid collisions when many nodes are present.
    // double delay = immediate ? 0 : m_nodeNumber;
    NS_LOG_DEBUG(std::to_string(m_nodeNumber) + " starting application; scheduling broadcast in "
                 << delay << " seconds");
    Simulator::Schedule(Seconds(delay), &App::SendBroadcast, this);
}

void
App::UpdateNeighbourTable(Ipv4Address node, Vector nodePos, Time timeStamp)
{
    m_neighbourTable[node] = std::make_pair(nodePos, timeStamp);
}

void
App::Purge()
{
    for (auto it = m_neighbourTable.begin(); it != m_neighbourTable.end();)
    {
        if (Simulator::Now() - it->second.second > Seconds(50))
        {
            it = m_neighbourTable.erase(it);
        }
        else
        {
            ++it;
        }
    }
    SendHello(0.1);
}

std::vector<Ipv4Address>
App::SelectBestNeighbour(Vector destPos, Vector myPosi, Ipv4Address destAddr)
{
    std::multimap<double, Ipv4Address> sameZoneNeighbours;
    std::multimap<double, Ipv4Address> adjancentZoneNeighboursl; //! Add Zone info later
    std::multimap<double, Ipv4Address> adjancentZoneNeighboursr; //! Add Zone info later
    std::multimap<double, Ipv4Address> backZoneNeighbours;       //! Add Zone info later
    Zone destZone = GetZone(destPos);

    for (auto entry : m_neighbourTable)
    {
        if (entry.first == destAddr)
        {
            NS_LOG_INFO("Destination Found");
            return {destAddr};
        }
        Zone neighbourZone = GetZone(entry.second.first);
        NS_LOG_INFO("Node " << m_nodeNumber << " have neighbour " << entry.first << " in Zone "
                            << neighbourZone << " and destination is in " << destZone);
        if (destZone == neighbourZone)
        {
            sameZoneNeighbours.emplace(CalculateDistance(destPos, entry.second.first), entry.first);
        }
        if (destZone == Z_I)
        {
            if (neighbourZone == Z_II)
                adjancentZoneNeighboursl.emplace(CalculateDistance(destPos, entry.second.first),
                                                 entry.first);
            if (neighbourZone == Z_IV)
                adjancentZoneNeighboursr.emplace(CalculateDistance(destPos, entry.second.first),
                                                 entry.first);
            if (neighbourZone == Z_III)
                backZoneNeighbours.emplace(CalculateDistance(destPos, entry.second.first),
                                           entry.first);
        }
        if (destZone == Z_II)
        {
            if (neighbourZone == Z_III)
                adjancentZoneNeighboursl.emplace(CalculateDistance(destPos, entry.second.first),
                                                 entry.first);
            if (neighbourZone == Z_I)
                adjancentZoneNeighboursr.emplace(CalculateDistance(destPos, entry.second.first),
                                                 entry.first);
            if (neighbourZone == Z_IV)
                backZoneNeighbours.emplace(CalculateDistance(destPos, entry.second.first),
                                           entry.first);
        }
        if (destZone == Z_III)
        {
            if (neighbourZone == Z_IV)
                adjancentZoneNeighboursl.emplace(CalculateDistance(destPos, entry.second.first),
                                                 entry.first);
            if (neighbourZone == Z_II)
                adjancentZoneNeighboursr.emplace(CalculateDistance(destPos, entry.second.first),
                                                 entry.first);
            if (neighbourZone == Z_I)
                backZoneNeighbours.emplace(CalculateDistance(destPos, entry.second.first),
                                           entry.first);
        }
        if (destZone == Z_IV)
        {
            if (neighbourZone == Z_I)
                adjancentZoneNeighboursl.emplace(CalculateDistance(destPos, entry.second.first),
                                                 entry.first);
            if (neighbourZone == Z_III)
                adjancentZoneNeighboursr.emplace(CalculateDistance(destPos, entry.second.first),
                                                 entry.first);
            if (neighbourZone == Z_II)
                backZoneNeighbours.emplace(CalculateDistance(destPos, entry.second.first),
                                           entry.first);
        }
    }
    if (sameZoneNeighbours.size() > 0)
    {
        SendHello(0.1);
        NS_LOG_INFO("Same Zone Neighbour Selected from " << sameZoneNeighbours.size());
        return {sameZoneNeighbours.begin()->second};
    }
    if (adjancentZoneNeighboursl.size() > 0 || adjancentZoneNeighboursr.size() > 0)
    {
        SendHello(0.1);
        NS_LOG_INFO("Adjancent Zone Neighbour Selected from "
                    << adjancentZoneNeighboursl.size() + adjancentZoneNeighboursr.size());
        std::vector<Ipv4Address> adjancentZoneNeighbours;
        if (adjancentZoneNeighboursl.size() > 0)
            adjancentZoneNeighbours.push_back(adjancentZoneNeighboursl.begin()->second);
        if (adjancentZoneNeighboursr.size() > 0)
            adjancentZoneNeighbours.push_back(adjancentZoneNeighboursr.begin()->second);
        return adjancentZoneNeighbours;
    }
    if (backZoneNeighbours.size() > 0)
    {
        SendHello(0.1);
        NS_LOG_INFO("Back Zone Neighbour Selected from " << backZoneNeighbours.size());
        return {backZoneNeighbours.begin()->second};
    }
    noNegibhours++;
    SendHello(0.1);
    return {};
}

double
App::CalculateDistance(ns3::Vector a, ns3::Vector b)
{
    return std::sqrt(std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2) + std::pow(b.z - a.z, 2));
}

void
App::PrintTable()
{
    std::cout << "\n-----------------------" << m_nodeNumber << "----------------------------\n";
    std::cout << "Neighbour IP\tNeighbour Location\tTime Stamp\n";
    for (auto entry : m_neighbourTable)
    {
        std::cout << entry.first << "\t" << entry.second.first << "\t" << entry.second.second
                  << "\n";
    }
    std::cout << "\n-----------------------" << m_nodeNumber << "----------------------------\n";
}

Zone
App::GetZone(Vector destPos)
{
    Vector myPosi = GetNode()->GetObject<MobilityModel>()->GetPosition();

    if (destPos.x - myPosi.x > 0 && destPos.y - myPosi.y > 0)
        return Z_I;
    if (destPos.x - myPosi.x < 0 && destPos.y - myPosi.y > 0)
        return Z_II;
    if (destPos.x - myPosi.x < 0 && destPos.y - myPosi.y < 0)
        return Z_III;
    else
        return Z_IV;
};

void
handler(int arg0)
{
    std::cout << "The simulation is now at: " << arg0 << " seconds" << std::endl;
}

//
// Main simulation program
//
uint32_t App::totalSent = 0;
uint32_t App::totalReceived = 0;
uint32_t App::noNegibhours = 0;

void
RunSimulation(uint8_t numDrones, double& pdr)
{
    std::cout<<"Simulation Running with " << std::to_string(numDrones) <<" drones.\n";
    App::totalSent = 0;
    App::totalReceived = 0;
    App::noNegibhours = 0;
    double totalTime = 500.0;
    TOTAL_DRONES = numDrones;
    NodeContainer nodes;
    nodes.Create(numDrones);

    // Set up Wi-Fi.
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);

    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper();
    wifiPhy.SetChannel(wifiChannel.Create());

    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");
    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel",
                                   "Frequency",
                                   DoubleValue(1e10), // 5 GHz
                                   "SystemLoss",
                                   DoubleValue(0.1)); // Decrease SystemLoss to increase range

    wifiPhy.Set("TxPowerStart", DoubleValue(100.0)); // Set transmit power to 20 dBm
    wifiPhy.Set("TxPowerEnd", DoubleValue(100.0));   // Set transmit power to 20 dBm

    // Use Adhoc MAC so that nodes can communicate directly.
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install(wifiPhy, wifiMac, nodes);
    // wifiPhy.EnablePcapAll("scratch-simulator");

    // Install Internet stack.
    InternetStackHelper internet;
    internet.Install(nodes);

    // Assign IP addresses.
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

    // Set up mobility.
    MobilityHelper mobility;
    // mobility.SetMobilityModel("ns3::GaussMarkovMobilityModel",
    //                           "Bounds", BoxValue(Box(0, 1000, 0, 1000, 50, 100)),
    //                           "TimeStep", TimeValue(Seconds(0.5)),
    //                           "Alpha", DoubleValue(0.85),
    //                           "MeanVelocity", StringValue("ns3::UniformRandomVariable[Min=8.0|Max=20.0]"),
    //                           "MeanDirection", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=6.283185307]"),
    //                           "MeanPitch", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=0.0]"),
    //                           "NormalVelocity", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=1.0|Bound=2.0]"),
    //                           "NormalDirection", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=1.0|Bound=2.0]"),
    //                           "NormalPitch", StringValue("ns3::NormalRandomVariable[Mean=0.0|Variance=1.0|Bound=2.0]"));

    mobility.SetMobilityModel("ns3::RandomWaypointMobilityModel",
                              "Speed",
                              StringValue("ns3::UniformRandomVariable[Min=8.0|Max=20.0]"),
                              "Pause",
                              StringValue("ns3::UniformRandomVariable[Min=0.0|Max=2.0]"),
                              "PositionAllocator",
                              StringValue("ns3::RandomBoxPositionAllocator"));
    mobility.SetPositionAllocator("ns3::RandomBoxPositionAllocator",
                                  "X", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"),
                                  "Y", StringValue("ns3::UniformRandomVariable[Min=0.0|Max=1000.0]"),
                                  "Z", StringValue("ns3::UniformRandomVariable[Min=50.0|Max=100.0]"));
    mobility.Install(nodes);

    // Create and install the application on each node.
    Ptr<App> appNode[100];
    for (uint16_t i = 0; i < TOTAL_DRONES; i++)
    {
        appNode[i] = CreateObject<App>(i + 1);
        appNode[i]->Setup(interfaces.GetAddress(i));
        nodes.Get(i)->AddApplication(appNode[i]);
        appNode[i]->SetStartTime(Seconds(0.1));
        appNode[i]->SetStopTime(Seconds(totalTime));
        appNode[i]->SendHello();
    }

    // For example, start the hello broadcast on Node 0.

    Ptr<Node> destination = nodes.Get(TOTAL_DRONES - 1);
    destPos = destination->GetObject<MobilityModel>()->GetPosition();
    destAddr = interfaces.GetAddress(TOTAL_DRONES - 1);
    // appNode[0]->SendMessage(destPos,destAddr);

    // for (int i = 1; i <= totalTime; i++)
    // {
    //     if (i % 10 == 0) // at every 10s
    //         Simulator::Schedule(Seconds(i), &handler, i);
    // }

    for (uint32_t i = 0; i < 1000; ++i)
    {
        Simulator::Schedule(Seconds(5.0 + i * 1.0),
                            &App::SendMessage,
                            appNode[0],
                            destPos,
                            destAddr);
    }

    Simulator::Stop(Seconds(totalTime));
    Simulator::Run();

    if (App::totalSent > 0)
    {
        pdr = (static_cast<double>(App::totalReceived) / App::totalSent) * 100.0;
        // std::cout << "Packet Delivery Ratio (PDR): " << pdr << "%" << std::endl;
    }
    else
    {
        // std::cout << "No packets were sent." << std::endl;
        pdr = 0;
    }

    Simulator::Destroy();
}

int
main(int argc, char* argv[])
{
    // LogComponentEnable("ZAF", LOG_LEVEL_INFO);
    // LogComponentEnable("UdpSocketImpl", LOG_LEVEL_INFO);
    double pdr;
    std::vector<double> pdrs;
    std::vector<int> droneCounts;
    for (int i = 10; i <= 200; i = i + 10)
    {
        RunSimulation(i, pdr);
        pdrs.push_back(pdr);
        droneCounts.push_back(i);
    }

    // Plot the results using Gnuplot
    Gnuplot plot("pdr-vs-drones.png");
    plot.SetTitle("PDR vs. Number of Drones");
    plot.SetTerminal("png");
    plot.SetLegend("Number of Drones", "PDR (%)");

    Gnuplot2dDataset dataset;
    dataset.SetTitle("PDR");
    dataset.SetStyle(Gnuplot2dDataset::LINES_POINTS);

    for (size_t i = 0; i < droneCounts.size(); ++i)
    {
        dataset.Add(droneCounts[i], pdrs[i]);
    }

    plot.AddDataset(dataset);

    std::ofstream plotFile("pdr-vs-drones-RWP.plt");
    plot.GenerateOutput(plotFile);
    plotFile.close();

    std::cout << "Simulation completed. Plot saved as pdr-vs-drones.png" << std::endl;

    return 0;
}

// Aodv , gpsr, tora, olsr
// PDR, DELAY , THROUGHPUT