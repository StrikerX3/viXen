// StrikeBox NV2A PTIMER (Time measurement and time-based alarms) engine emulation
// (C) Ivan "StrikerX3" Oliveira
//
// Based on envytools and nouveau:
// https://envytools.readthedocs.io/en/latest/index.html
// https://github.com/torvalds/linux/tree/master/drivers/gpu/drm/nouveau
//
// References to particular items in the documentation are denoted between
// brackets optionally followed by a quote from the documentation.
#include "strikebox/hw/gpu/engines/ptimer.h"
#include "strikebox/hw/gpu/state.h"

#include "strikebox/log.h"

namespace strikebox::nv2a {

void PTIMER::SetEnabled(bool enabled) {
    if (m_enabled != enabled) {
        m_enabled = enabled;
        if (enabled) {
            // TODO: start alarm thread
        }
        else {
            Reset();
        }
    }
}

void PTIMER::Reset() {
    m_enabled = false;
    m_interruptLevels = 0;
    m_enabledInterrupts = 0;
    m_clockMul = 1;
    m_clockDiv = 1;
    m_alarm = 0;
    m_lastTickCountRead = std::chrono::high_resolution_clock::now();
    m_tickCount = 0;
    // TODO: stop alarm thread
}

uint32_t PTIMER::Read(const uint32_t addr) {
    switch (addr) {
    case Reg_PTIMER_INTR: return m_interruptLevels;
    case Reg_PTIMER_INTR_ENABLE: return m_enabledInterrupts;
    case Reg_PTIMER_CLOCK_MUL: return m_clockMul;
    case Reg_PTIMER_CLOCK_DIV: return m_clockDiv;
        // [https://envytools.readthedocs.io/en/latest/hw/bus/ptimer.html#the-time-counter]
        // PTIMER's clock is spread across two 32-bit registers:
        // - TIME_LOW: contains the low 27 bits of the counter in bits 5-31.
        // - TIME_HIGH: contains the high 29 bits of the counter in bits 0-28.
    case Reg_PTIMER_TIME_LOW: return static_cast<uint32_t>(GetTickCount() << 5ull);
    case Reg_PTIMER_TIME_HIGH: return static_cast<uint32_t>(GetTickCount() >> 27ull);
    case Reg_PTIMER_ALARM: return m_alarm;
    default:
        log_spew("[NV2A] PTIMER::Read:   Unimplemented read!   address = 0x%x\n", addr);
        return 0;
    }
}

void PTIMER::Write(const uint32_t addr, const uint32_t value) {
    switch (addr) {
    case Reg_PTIMER_INTR:
        // Clear specified interrupts
        m_interruptLevels &= ~value;
        m_nv2a.UpdateIRQ();
        break;
    case Reg_PTIMER_INTR_ENABLE:
        m_enabledInterrupts = value;
        m_nv2a.UpdateIRQ();
        break;
    case Reg_PTIMER_CLOCK_MUL:
        GetTickCount(); // ensure the tick count rate is updated immediately
        m_clockMul = value;
        break;
    case Reg_PTIMER_CLOCK_DIV:
        GetTickCount(); // ensure the tick count rate is updated immediately
        m_clockDiv = value;
        break;
    case Reg_PTIMER_TIME_LOW:
        m_tickCount = (m_tickCount & 0xFFFFFFF8000000ull) | (value & 0x7FFFFFFull);
        break;
    case Reg_PTIMER_TIME_HIGH:
        m_tickCount = (m_tickCount & 0x7FFFFFFull) | ((value & 0x1FFFFFFFull) << 27ull);
        break;
    case Reg_PTIMER_ALARM:
        m_alarm = value;
        break;
    default:
        log_spew("[NV2A] PTIMER::Write:  Unimplemented write!   address = 0x%x,  value = 0x%x\n", addr, value);
        break;
    }
}

uint64_t PTIMER::GetTickCount() {
    // Compute time delta since last read
    auto now = std::chrono::high_resolution_clock::now();
    auto delta = now - m_lastTickCountRead;
    double deltaSeconds = delta.count() / 1000000000.0;
    m_lastTickCountRead = now;

    // [https://envytools.readthedocs.io/en/latest/hw/bus/ptimer.html#the-clock-source]
    // "The clock that PTIMER counts is generated by applying a selectable ratio to a clock source. The clock source depends on the card:
    //  [...]
    //  NV4:NV40: the clock source is NVCLK, the core clock"
    uint64_t coreClock = static_cast<uint64_t>(m_nv2a.pramdac.GetCoreClockCoefficients().CalcClock());

    // [https://envytools.readthedocs.io/en/latest/hw/bus/ptimer.html#the-clock-ratio]
    // "The clock used for the counter is clock_source * CLOCK_MUL / CLOCK_DIV."
    uint64_t timerClock = coreClock * m_clockMul / m_clockDiv;

    // Increment tick count
    m_tickCount += timerClock * deltaSeconds;

    // [https://envytools.readthedocs.io/en/latest/hw/bus/ptimer.html#the-time-counter]
    // "PTIMER's clock is a 56-bit value"
    return m_tickCount & 0xFFFFFFFFFFFFFFull;
}

}
