// SPDX-License-Identifier: GPL-2.0
#include <Arduino.h>
#include <SPI.h>

extern "C"
{
#include "printf.h"
#include "id_sd.h"
#include "ck_cross.h"
void SDL_t0Service(void);
}

static const int PC_PIT_RATE = 1193182;
static const int SD_SFX_PART_RATE = 140;
static const int SD_SOUND_PART_RATE_BASE = 1192030;
static const int BITRATE = 9600;
uint16_t ints_per_sec;

static bool SD_t4_IsLocked = false;
static bool SD_t4_AudioSubsystem_Up = false;

static const int OPL_PIN_RESET = 8;
static const int OPL_PIN_A0 = 9;
//These are for SPI0:
static const int OPL_PIN_LATCH = 10;
static const int OPL_PIN_DATA = 11;
static const int OPL_PIN_SHIFT = 13;

//Timing backend for the gamelogic which uses the sound system
static IntervalTimer t0_timer;

//Audio interrupts
static void _t0service()
{
    SDL_t0Service();
}

static void SD_t4_SetTimer0(int16_t int_8_divisor)
{
    //Create an interrupt that occurs at a certain frequency.
    ints_per_sec = PC_PIT_RATE / int_8_divisor;
    t0_timer.end();
    t0_timer.begin(_t0service, 1000000 / ints_per_sec);
}

static void SD_t4_alOut(uint8_t reg, uint8_t val)
{
    //Write the register
    digitalWrite(OPL_PIN_A0, LOW);
    SPI.transfer(reg);
    digitalWrite(OPL_PIN_LATCH, LOW);
    delayMicroseconds(16);
    digitalWrite(OPL_PIN_LATCH, HIGH);
    delayMicroseconds(16);

    //Write the value
    digitalWrite(OPL_PIN_A0, HIGH);
    SPI.transfer(val);
    digitalWrite(OPL_PIN_LATCH, LOW);
    delayMicroseconds(4);
    digitalWrite(OPL_PIN_LATCH, HIGH);
    //delayMicroseconds(92);
}

static void SD_t4_PCSpkOn(bool on, int freq)
{
}

static void SD_t4_Startup(void)
{
    if (SD_t4_AudioSubsystem_Up)
    {
        return;
    }

    //Using SPI0.
    //Latch should be connected to 10
    //Data pin should be connected to 11
    //Shift should be connected to 13
    SPI.begin();
    SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));

    pinMode(OPL_PIN_LATCH, OUTPUT);
    pinMode(OPL_PIN_A0, OUTPUT);
    pinMode(OPL_PIN_RESET, OUTPUT);

    digitalWrite(OPL_PIN_LATCH, HIGH);
    digitalWrite(OPL_PIN_RESET, HIGH);
    digitalWrite(OPL_PIN_A0, LOW);

    digitalWrite(OPL_PIN_RESET, LOW);
    delay(1);
    digitalWrite(OPL_PIN_RESET, HIGH);
}

static void SD_t4_Shutdown(void)
{
    if (SD_t4_AudioSubsystem_Up)
    {
        t0_timer.end();
        SD_t4_AudioSubsystem_Up = false;
    }
}

static void SD_t4_Lock()
{
    if (SD_t4_IsLocked)
    {
        CK_Cross_LogMessage(CK_LOG_MSG_ERROR, "Tried to lock the audio system when it was already locked!\n");
        return;
    }
    SD_t4_IsLocked = true;
}

static void SD_t4_Unlock()
{
    if (!SD_t4_IsLocked)
    {
        CK_Cross_LogMessage(CK_LOG_MSG_ERROR, "Tried to unlock the audio system when it was already unlocked!\n");
        return;
    }
    SD_t4_IsLocked = false;
}

static SD_Backend sd_t4_backend = {
    .startup = SD_t4_Startup,
    .shutdown = SD_t4_Shutdown,
    .lock = SD_t4_Lock,
    .unlock = SD_t4_Unlock,
    .alOut = SD_t4_alOut,
    .pcSpkOn = SD_t4_PCSpkOn,
    .setTimer0 = SD_t4_SetTimer0};

SD_Backend *SD_Impl_GetBackend()
{
    return &sd_t4_backend;
}
