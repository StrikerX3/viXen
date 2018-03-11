#pragma once

#include <cstdint>

#include "../defs.h"
#include "pci.h"

namespace openxbox {

class PCIBridgeDevice : public PCIDevice {
public:
    // constructor
	PCIBridgeDevice(uint16_t vendorID, uint16_t deviceID, uint8_t revisionID);

    // PCI Device functions
    virtual void Init() override;
    virtual void Reset() override;

    void WriteConfig(uint32_t reg, uint32_t value, uint8_t size) override;
};

}