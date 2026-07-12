#pragma once

/**
 * Abstract interface for sending commands to one or more DALI lamps.
 */
class IDaliOutput
{
public:
    virtual ~IDaliOutput() = default;

    /** Set absolute arc brightness (0-254). Off = 0. */
    virtual void sendArc(uint8_t arc) = 0;

    /** Turn the lamp(s) off. */
    virtual void sendOff() = 0;

    /** Set colour temperature in Kelvin. */
    virtual void setTemperature(uint16_t kelvin) = 0;

    /**
     * Called once per relative-dimming interval while brightness-dimming.
     */
    virtual void sendDimCommand(bool up) = 0;

    /**
     * Called after each arc increment/decrement during brightness-dimming.
     */
    virtual void onBrightnessDimStep(uint8_t arc) = 0;

    /**
     * Called when brightness-dimming starts from off.
     */
    virtual void recallMin(uint8_t minArc) = 0;

    /**
     * Called when brightness-dimming reaches the lower limit.
     */
    virtual void sendDimOffCommand() = 0;

    /**
     * True if this output operates in Tunable-White mode.
     * Used by DaliChannel to force colorType = TW in koHandleColor().
     */
    virtual bool isTunableWhiteMode() const = 0;

    /**
     * True if a DALI QUERY_ACTUAL_LEVEL is meaningful for this output.
     */
    virtual bool supportsQueryLevel() const = 0;

    /**
     * Recall a DALI scene (0-15) on all lamps of this output.
     * VirtualTWOutput sends the command to both WW and CW addresses.
     */
    virtual void sendScene(uint8_t sceneNumber) = 0;
};
