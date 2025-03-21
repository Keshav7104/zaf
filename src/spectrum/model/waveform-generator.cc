/*
 * Copyright (c) 2009 CTTC
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Nicola Baldo <nbaldo@cttc.es>
 */

#include "waveform-generator.h"

#include <ns3/antenna-model.h>
#include <ns3/double.h>
#include <ns3/log.h>
#include <ns3/object-factory.h>
#include <ns3/packet-burst.h>
#include <ns3/simulator.h>

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("WaveformGenerator");

NS_OBJECT_ENSURE_REGISTERED(WaveformGenerator);

WaveformGenerator::WaveformGenerator()
    : m_mobility(nullptr),
      m_netDevice(nullptr),
      m_channel(nullptr),
      m_txPowerSpectralDensity(nullptr),
      m_startTime()
{
}

WaveformGenerator::~WaveformGenerator()
{
}

void
WaveformGenerator::DoDispose()
{
    NS_LOG_FUNCTION(this);
    m_channel = nullptr;
    m_netDevice = nullptr;
    m_mobility = nullptr;
    if (m_nextWave.IsPending())
    {
        m_nextWave.Cancel();
    }
}

TypeId
WaveformGenerator::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::WaveformGenerator")
            .SetParent<SpectrumPhy>()
            .SetGroupName("Spectrum")
            .AddConstructor<WaveformGenerator>()
            .AddAttribute(
                "Period",
                "the period (=1/frequency)",
                TimeValue(Seconds(1)),
                MakeTimeAccessor(&WaveformGenerator::SetPeriod, &WaveformGenerator::GetPeriod),
                MakeTimeChecker())
            .AddAttribute("DutyCycle",
                          "the duty cycle of the generator, i.e., the fraction of the period that "
                          "is occupied by a signal",
                          DoubleValue(0.5),
                          MakeDoubleAccessor(&WaveformGenerator::SetDutyCycle,
                                             &WaveformGenerator::GetDutyCycle),
                          MakeDoubleChecker<double>())
            .AddTraceSource("TxStart",
                            "Trace fired when a new transmission is started",
                            MakeTraceSourceAccessor(&WaveformGenerator::m_phyTxStartTrace),
                            "ns3::Packet::TracedCallback")
            .AddTraceSource("TxEnd",
                            "Trace fired when a previously started transmission is finished",
                            MakeTraceSourceAccessor(&WaveformGenerator::m_phyTxEndTrace),
                            "ns3::Packet::TracedCallback");
    return tid;
}

Ptr<NetDevice>
WaveformGenerator::GetDevice() const
{
    return m_netDevice;
}

Ptr<MobilityModel>
WaveformGenerator::GetMobility() const
{
    return m_mobility;
}

Ptr<const SpectrumModel>
WaveformGenerator::GetRxSpectrumModel() const
{
    // this device is not interested in RX
    return nullptr;
}

void
WaveformGenerator::SetDevice(Ptr<NetDevice> d)
{
    m_netDevice = d;
}

void
WaveformGenerator::SetMobility(Ptr<MobilityModel> m)
{
    m_mobility = m;
}

void
WaveformGenerator::SetChannel(Ptr<SpectrumChannel> c)
{
    NS_LOG_FUNCTION_NOARGS();
    m_channel = c;
}

void
WaveformGenerator::StartRx(Ptr<SpectrumSignalParameters> params)
{
    NS_LOG_FUNCTION(this << params);
}

void
WaveformGenerator::SetTxPowerSpectralDensity(Ptr<SpectrumValue> txPsd)
{
    NS_LOG_FUNCTION(this << *txPsd);
    m_txPowerSpectralDensity = txPsd;
}

Ptr<Object>
WaveformGenerator::GetAntenna() const
{
    return m_antenna;
}

void
WaveformGenerator::SetAntenna(Ptr<AntennaModel> a)
{
    NS_LOG_FUNCTION(this << a);
    m_antenna = a;
}

void
WaveformGenerator::SetPeriod(Time period)
{
    m_period = period;
}

Time
WaveformGenerator::GetPeriod() const
{
    return m_period;
}

void
WaveformGenerator::SetDutyCycle(double dutyCycle)
{
    m_dutyCycle = dutyCycle;
}

double
WaveformGenerator::GetDutyCycle() const
{
    return m_dutyCycle;
}

void
WaveformGenerator::GenerateWaveform()
{
    NS_LOG_FUNCTION(this);

    Ptr<SpectrumSignalParameters> txParams = Create<SpectrumSignalParameters>();
    txParams->duration = Time(m_period.GetTimeStep() * m_dutyCycle);
    txParams->psd = m_txPowerSpectralDensity;
    txParams->txPhy = GetObject<SpectrumPhy>();
    txParams->txAntenna = m_antenna;

    NS_LOG_LOGIC("generating waveform : " << *m_txPowerSpectralDensity);
    m_phyTxStartTrace(nullptr);
    m_channel->StartTx(txParams);

    NS_LOG_LOGIC("scheduling next waveform");
    m_nextWave = Simulator::Schedule(m_period, &WaveformGenerator::GenerateWaveform, this);
}

void
WaveformGenerator::Start()
{
    NS_LOG_FUNCTION(this);
    if (!m_nextWave.IsPending())
    {
        NS_LOG_LOGIC("generator was not active, now starting");
        m_startTime = Now();
        m_nextWave = Simulator::ScheduleNow(&WaveformGenerator::GenerateWaveform, this);
    }
}

void
WaveformGenerator::Stop()
{
    NS_LOG_FUNCTION(this);
    if (m_nextWave.IsPending())
    {
        m_nextWave.Cancel();
    }
}
} // namespace ns3
