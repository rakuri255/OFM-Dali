#pragma once
#include "IDaliOutput.h"
#include "Dali/Master.h"
#include "Dali/Commands.h"

/**
 * Standard DALI output driver for a single short address or group address.
 * Forwards all commands directly to the DALI bus.
 */
class StandardDaliOutput : public IDaliOutput
{
public:
    StandardDaliOutput(Dali::Master& master, uint8_t address, bool isGroup)
        : _master(master), _address(address), _isGroup(isGroup) {}

    void sendArc(uint8_t arc) override
    {
        _master.sendArc(_address, arc, _isGroup);
    }

    void sendOff() override
    {
        _master.sendArc(_address, 0, _isGroup);
    }

    void setTemperature(uint16_t kelvin) override
    {
        const uint16_t mirek = static_cast<uint16_t>(1000000.0f / static_cast<float>(kelvin));
        _master.sendSpecialCommand(Dali::SpecialCommand::SET_DTR,  mirek & 0xFF);
        _master.sendSpecialCommand(Dali::SpecialCommand::SET_DTR1, (mirek >> 8) & 0xFF);
        _master.sendExtendedCommand(_address, 0x08,
            Dali::ExtendedCommandDT8::SET_TEMP_COLOUR_TEMPERATURE, _isGroup);
        _master.sendExtendedCommand(_address, 0x08,
            Dali::ExtendedCommandDT8::ACTIVATE, _isGroup);
    }

    void sendDimCommand(bool up) override
    {
        _master.sendCommand(_address,
            up ? Dali::Command::UP : Dali::Command::DOWN, _isGroup);
    }

    void onBrightnessDimStep(uint8_t /*arc*/) override
    {
        // DALI hardware tracks level; nothing to do.
    }

    void recallMin(uint8_t /*minArc*/) override
    {
        _master.sendCommand(_address, Dali::Command::RECALL_MIN, _isGroup, true);
    }

    void sendDimOffCommand() override
    {
        _master.sendCommand(_address, Dali::Command::OFF, _isGroup);
    }

    bool isTunableWhiteMode() const override { return false; }
    bool supportsQueryLevel()  const override { return true;  }

    void sendScene(uint8_t sceneNumber) override
    {
        _master.sendCommand(_address, Dali::Command::GO_TO_SCENE | sceneNumber, _isGroup);
    }

private:
    Dali::Master& _master;
    uint8_t       _address;
    bool          _isGroup;
};
