// SPDX-License-Identifier: GPL-2.0+

/*
  pimount.h

  INDI driver for 'pimount'.  This is based on the
  telescope_simulation driver in the indi tree.
*/

#pragma once

#include "indiguiderinterface.h"
#include "inditelescope.h"

class PiMount : public INDI::Telescope, public INDI::GuiderInterface
{
  public:
    PiMount();
    virtual ~PiMount() = default;

    virtual const char *getDefaultName() override;
    virtual bool Connect() override;
    virtual bool Disconnect() override;
    virtual bool ReadScopeStatus() override;
    virtual bool initProperties() override;
    virtual void ISGetProperties(const char *dev) override;
    virtual bool updateProperties() override;

    virtual bool ISNewNumber(const char *dev, const char *name, double values[], char *names[], int n) override;
    virtual bool ISNewSwitch(const char *dev, const char *name, ISState *states, char *names[], int n) override;

  protected:
    virtual bool MoveNS(INDI_DIR_NS dir, TelescopeMotionCommand command) override;
    virtual bool MoveWE(INDI_DIR_WE dir, TelescopeMotionCommand command) override;
    virtual bool Abort() override;

    virtual IPState GuideNorth(uint32_t ms) override;
    virtual IPState GuideSouth(uint32_t ms) override;
    virtual IPState GuideEast(uint32_t ms) override;
    virtual IPState GuideWest(uint32_t ms) override;

    virtual bool SetTrackMode(uint8_t mode) override;
    virtual bool SetTrackEnabled(bool enabled) override;
    virtual bool SetTrackRate(double raRate, double deRate) override;

    virtual bool Goto(double, double) override;
    virtual bool Park() override;
    virtual bool UnPark() override;
    virtual bool Sync(double ra, double dec) override;

    // Parking
    virtual bool SetCurrentPark() override;
    virtual bool SetDefaultPark() override;

private:
    double currentRA { 0 };
    double currentDEC { 90 };
    double targetRA { 0 };
    double targetDEC { 0 };

    /// used by GoTo and Park
    void StartSlew(double ra, double dec, TelescopeStatus status);

    bool forceMeridianFlip { false };
    unsigned int DBG_SCOPE { 0 };

    double guiderEWTarget[2];
    double guiderNSTarget[2];

    INumber GuideRateN[2];
    INumberVectorProperty GuideRateNP;

#ifdef USE_EQUATORIAL_PE
    INumberVectorProperty EqPENV;
    INumber EqPEN[2];

    ISwitch PEErrNSS[2];
    ISwitchVectorProperty PEErrNSSP;

    ISwitch PEErrWES[2];
    ISwitchVectorProperty PEErrWESP;
#endif
};
