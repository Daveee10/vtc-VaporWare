#ifndef __SETTINGS_H
#define __SETTINGS_H

// 10/second 0.5-60s range, 3s default
#define SCREENMINTIMEOUT 5
#define SCREENMAXTIMEOUT 600
#define SCREENDEFAUTLTIMEOUT 30

// After screen off
#define SYSSLEEPDEFAULT 180 * 10
// After screen off, may block sys sleep
#define SETTINGSWRITEDEFAULT 300 * 10

// Kanthal Default
#define DEFAULTMATERIAL 0

// Default mode: Wattage
#define DEFAULTMODE 0

// Temp Scale, Default F (Sorry Europe)
#define DEFAULTTEMPSCALE 1

// Pid Vals
#define MINPID 0
#define MAXPID 30000
// TODO: Move these to material struct
#define DEFPIDP 17000
#define DEFPIDI 5500
#define DEFPIDD 0

// Temp mode watts (and wattage mode default?)
#define MINWATTS 0
#define MAXWATTS 60000
#define DEFWATTS 15000

// Temp mode pid handoff temp differential
#define STEMPMIN -600
#define STEMPMAX  600
#define STEMPDEF  600

// Invert display pixel default
#define INVERTDEF 0

// Flip display while vaping
#define FLIPDEF 1

// TCR
#define TCRMIN 0
#define TCRMAX 1000
#define TCRDEF 0

// Base Temp degrees C
#define BTEMPMIN -20
#define BTEMPMAX 50
#define BTEMPDEF -20

// Base Res, mOhm
#define BRESMIN 50
#define BRESMAX 3450
#define BRESDEF 0


struct settings {
    uint8_t fromRom;
    uint8_t mode;
    uint16_t screenTimeout;
    volatile uint32_t displayTemperature;
    volatile uint32_t targetTemperature;
    uint8_t materialIndex;
    uint8_t tempScaleTypeIndex;
    uint32_t pidP;
    uint32_t pidI;
    uint32_t pidD;
    int32_t initWatts;
    int32_t pidSwitch;
    uint8_t dumpPids;
    uint8_t tunePids;
/* Adding in V2 of dataflash */
    uint8_t invertDisplay;
    uint8_t flipOnVape;
    uint16_t tcr;
    int16_t baseTemp;
    uint16_t baseRes;
    int screenBrightness;
};

extern struct settings s;

struct tempScale {
    char display[2];
    uint32_t max;
    uint32_t min;
    uint32_t def;
};
// Definition in communication.c
extern struct tempScale tempScaleType[];
extern uint8_t tempScaleCount;


int load_settings(void);
void saveSettings();

/* These all set the value passed in if validated, and make any other changes that
   this change might need.  If the passed value is not sane, it sets the default */
void modeSet(uint8_t mode);
void screenTimeoutSet(uint16_t screenTimeout);
void displayTemperatureSet(uint32_t displayTemperature);
void targetTemperatureSet(uint32_t targetTemperature);
void materialIndexSet(uint32_t materialIndex);
void tempScaleTypeIndexSet(uint8_t tempScaleTypeIndex);
void pidPSet(uint32_t pidP);
void pidISet(uint32_t pidI);
void pidDSet(uint32_t pidD);
void initWattsSet(int32_t initWatts);
void pidSwitchSet(int32_t pidSwitch);

void invertDisplaySet(uint8_t invertDisplay);
void flipOnVapeSet(uint8_t flipOnVape);

void tcrSet(uint16_t tcr);
void baseTempSet(int16_t baseTemp);
void baseResSet(uint16_t baseRes);
#endif
