// StrikeBox NV2A PSTRAPS (Straps readout) engine emulation
// (C) Ivan "StrikerX3" Oliveira
//
// Based on envytools and nouveau:
// https://envytools.readthedocs.io/en/latest/index.html
// https://github.com/torvalds/linux/tree/master/drivers/gpu/drm/nouveau
//
// References to particular items in the documentation are denoted between
// brackets optionally followed by a quote from the documentation.
#include "strikebox/hw/gpu/engines/pstraps.h"

#include "strikebox/log.h"

namespace strikebox::nv2a {

void PSTRAPS::Reset() {
}

uint32_t PSTRAPS::Read(const uint32_t addr) {
    log_spew("[NV2A] PSTRAPS::Read:   Unimplemented read!   address = 0x%x\n", addr);
    return 0;
}

void PSTRAPS::Write(const uint32_t addr, const uint32_t value) {
    log_spew("[NV2A] PSTRAPS::Write:  Unimplemented write!   address = 0x%x,  value = 0x%x\n", addr, value);
}

}
