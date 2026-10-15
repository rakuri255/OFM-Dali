#pragma once
#include "IDaliOutput.h"
#include "Dali/Master.h"
#include "DaliHelper.h"

/**
 * Virtual Tunable-White output driver.
 *
 * Emulates a single DT8 TW channel by mixing two physical DT6 lamps:
 *   - Warm-White (WW) lamp at wwAddress
 *   - Cold-White (CW) lamp at cwAddress
 *
 * TW mixing formula (constant luminous-flux):
 *   ratio = clamp((M_ww - M) / (M_ww - M_cw),  0, 1)   with M = 1e6 / T_kelvin (mirek)
 *   WW_arc = percentToArc( arcToPercent(brightness) * (1 - ratio) )
 *   CW_arc = percentToArc( arcToPercent(brightness) *      ratio  )
 *
 * At any ratio, WW_flux + CW_flux == master brightness flux.
 * The logarithmic arc<->percent conversion ensures perceptually correct
 * levels on each physical channel.
 */
class VirtualTWOutput : public IDaliOutput
{
public:
    VirtualTWOutput(Dali::Master& master,
                    uint8_t wwAddress, uint8_t cwAddress,
                    uint16_t tempMin,  uint16_t tempMax)
        : _master(master),
          _wwAddress(wwAddress), _cwAddress(cwAddress),
          _tempMin(tempMin), _tempMax(tempMax),
          _currentTemp(tempMin), _currentArc(0)
    {}

    void sendArc(uint8_t arc) override
    {
        _currentArc = arc;
        _applyTW();
    }

    void sendOff() override
    {
        _currentArc = 0;
        _master.sendArc(_wwAddress, 0, false);
        _master.sendArc(_cwAddress, 0, false);
    }

    void setTemperature(uint16_t kelvin) override
    {
        _currentTemp = kelvin;
        _applyTW();
    }

    void sendDimCommand(bool /*up*/) override
    {
        // Arc is driven explicitly via onBrightnessDimStep(); no DALI UP/DOWN.
    }

    void onBrightnessDimStep(uint8_t arc) override
    {
        _currentArc = arc;
        _applyTW();
    }

    void recallMin(uint8_t minArc) override
    {
        _currentArc = minArc;
        _applyTW();
    }

    void sendDimOffCommand() override
    {
        sendOff();
    }

    bool isTunableWhiteMode() const override { return true;  }
    bool supportsQueryLevel()  const override { return false; }

    void sendScene(uint8_t sceneNumber) override
    {
        // Send scene to both lamps so both react to the stored scene level.
        _master.sendCommand(_wwAddress, Dali::Command::GO_TO_SCENE | sceneNumber, false);
        _master.sendCommand(_cwAddress, Dali::Command::GO_TO_SCENE | sceneNumber, false);
    }

private:
    void _applyTW()
    {
        float ratio = 0.5f;
        if (_tempMax > _tempMin && _tempMin > 0 && _currentTemp > 0)
        {
            // Interpolate in mirek like a DT8 ballast does, not linear in Kelvin
            const float mWw  = 1000000.0f / _tempMin;
            const float mCw  = 1000000.0f / _tempMax;
            const float mCur = 1000000.0f / _currentTemp;
            ratio = (mWw - mCur) / (mWw - mCw);
            if (ratio < 0.0f) ratio = 0.0f;
            if (ratio > 1.0f) ratio = 1.0f;
        }

        const float bri   = DaliHelper::arcToPercentFloat(_currentArc);
        const float wwPct = bri * (1.0f - ratio);
        const float cwPct = bri * ratio;
        // Values < 0.1 % underflow the log scale → clamp to off
        const uint8_t wwArc = (wwPct < 0.1f) ? 0 : DaliHelper::percentToArc(wwPct);
        const uint8_t cwArc = (cwPct < 0.1f) ? 0 : DaliHelper::percentToArc(cwPct);

        _master.sendArc(_wwAddress, wwArc, false);
        _master.sendArc(_cwAddress, cwArc, false);
    }

    Dali::Master& _master;
    uint8_t  _wwAddress;
    uint8_t  _cwAddress;
    uint16_t _tempMin;
    uint16_t _tempMax;
    uint16_t _currentTemp;
    uint8_t  _currentArc;
};
