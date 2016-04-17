/*
 * This file is part of eVic SDK.
 *
 * eVic SDK is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * eVic SDK is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with eVic SDK.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2016 ReservedField
 * Copyright (C) 2016 kfazz
 */
#include <stdio.h>
#include <M451Series.h>
#include <Atomizer.h>
#include <Button.h>
#include <TimerUtils.h>
#include <Display.h>

#include "main.h"
#include "mode_watt.h"
#include "mode_volt.h"
#include "mode_temp.h"

struct globals g = {};
volatile struct globalVols gv = {
    .fireButtonPressed = 0,
    .screenState = 1,
    .screenOffTimer = -1,
};
struct settings s = {};

struct vapeMaterials vapeMaterialList[] = {
    {
        .typeMask = KANTHAL,
        .name = "KA",
        .tcr = 0,
    },
    {
        .typeMask = NICKEL,
        .name = "NI",
        .tcr = 620,
    },
    {
        .typeMask = TITANIUM,
        .name = "TI",
        .tcr = 350,
    },
    {
        .typeMask = STAINLESS,
        .name = "SS",
        .tcr = 105,
    },
};

// ALWAYS init it a sane mode
void (*__init)(void);
void (*__vape)(void);
void (*__up)(void);
void (*__down)(void);

void setVapeMode(int newMode) {
    if(newMode >= MODE_COUNT)
        return;

    s.mode = newMode;

    __vape = g.vapeModes[newMode]->fire;
    __up = g.vapeModes[newMode]->increase;
    __down = g.vapeModes[newMode]->decrease;
    if(g.vapeModes[newMode]->init) {
        __init = g.vapeModes[newMode]->init;
	__init();
    }
}

void setVapeMaterial(struct vapeMaterials *material) {
    s.material = material;
    g.atomInfo.tcr = material->tcr;
}

inline void __screenOff(void);

void screenOffTimeout(uint32_t c) {
    gv.screenState--;
    if(gv.screenState >= 1)
        __screenOff();
    return 0;
}

inline void screenOn() {
    gv.screenState = s.screenTimeout;
}

inline void __screenOff() {
    if(gv.screenOffTimer >= 0)
        Timer_DeleteTimer(gv.screenOffTimer);
    gv.screenOffTimer = Timer_CreateTimeout(100, 0, screenOffTimeout, 9);
}

void startVaping(uint32_t counterIndex) {
   if(gv.buttonCnt < 3) {
       if(Button_GetState() & BUTTON_MASK_FIRE) {
          gv.fireButtonPressed = 1;
          gv.buttonCnt = 0;
       }
   } else {
       gv.shouldShowMenu = 1;
       gv.buttonCnt = 0;
   }
   return 0;
}

void buttonFire(uint8_t state) {
   screenOn();
   g.whatever++;
   if (state & BUTTON_MASK_FIRE) {
       if(g.fireTimer)
           Timer_DeleteTimer(g.fireTimer);
       g.fireTimer = Timer_CreateTimeout(200, 0, startVaping, 3);
       gv.buttonCnt++;
   } else {
       __screenOff();
       gv.fireButtonPressed = 0;
   }
}

void buttonRight(uint8_t state) {
    screenOn();
    if(state & BUTTON_MASK_RIGHT) {
        __up();
        Atomizer_SetOutputVoltage(g.volts);
    } else {
        __screenOff();
    }
}

void buttonLeft(uint8_t state) {
   screenOn();
    if (state & BUTTON_MASK_LEFT) {
        __down();
	Atomizer_SetOutputVoltage(g.volts);
    } else {
        __screenOff();
    }
}


void setupButtons() {
    g.fire = Button_CreateCallback(buttonFire, BUTTON_MASK_FIRE);
    g.plus = Button_CreateCallback(buttonRight, BUTTON_MASK_RIGHT);
    g.minus = Button_CreateCallback(buttonLeft, BUTTON_MASK_LEFT);
}

#define REGISTER_MODE(X) g.vapeModes[X.index] = &X

int main() {
    int i = 0;
    int c_screenState = -1;
    load_settings();
    setupButtons();

    REGISTER_MODE(variableVoltage);
    REGISTER_MODE(variableWattage);
    REGISTER_MODE(variableTemp);

    setVapeMode(s.mode);
    setVapeMaterial(s.material);

    // Let's start with 15.0W as the initial value
    // We keep g.watts as mW
    Atomizer_ReadInfo(&g.atomInfo);
    g.watts = 15000;
    g.volts = wattsToVolts(g.watts, g.atomInfo.resistance);
    Atomizer_SetOutputVoltage(g.volts);

    // Initialize atomizer info
    do {
        Atomizer_ReadInfo(&g.atomInfo);
        updateScreen(&g);
        i++;
    } while(i < 100 && g.atomInfo.resistance == 0) ;
        
    while(g.atomInfo.resistance - g.atomInfo.base_resistance > 10) {
        Atomizer_ReadInfo(&g.atomInfo);
        updateScreen(&g);
    }

    __screenOff();

    while(1) {

        if (gv.fireButtonPressed) {
            __vape();
        }
        if (gv.shouldShowMenu) {
            showMenu();
        } else if (gv.screenState) {
            Display_SetOn(1);
            updateScreen(&g);
        } else if (gv.screenState <= 1) {
            Timer_DelayUs(100);
            Display_Clear();
            Display_SetOn(0);
            c_screenState = gv.screenState;
        }

    }
}