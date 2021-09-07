// SPDX-License-Identifier: GPL-2.0
#include <Arduino.h>
#include "USBHost_t36.h"

extern "C"
{
#include "printf.h"
#include "id_in.h"
#include "ck_cross.h"
}

USBHost usbh;
USBHub hub1(usbh);
JoystickController joy1(usbh);
JoystickController joy2(usbh);

static uint32_t new_b = 0;
static uint32_t delta_b = 0;
static int16_t x_axis = 0, y_axis = 0;

static void IN_T4_PumpEvents()
{
    if (joy1 == false)
        return;

    //There's a few joystick buttons that need to be injected as keypressed
    delta_b = (joy1.getButtons() ^ new_b);
    new_b = joy1.getButtons();
    x_axis = joy1.getAxis(0);
    y_axis = joy1.getAxis(1);
    joy1.joystickDataClear();

    //Start (Main menu)
    if (((new_b & (1 << 12)) && (delta_b & (1 << 12))))  IN_HandleKeyDown(IN_SC_Escape, 0);
    if ((!(new_b & (1 << 12)) && (delta_b & (1 << 12)))) IN_HandleKeyUp(IN_SC_Escape, 0);

    //Back (Status menu)
    if (((new_b & (1 << 13)) && (delta_b & (1 << 13))))  IN_HandleKeyDown(IN_SC_Enter, 0);
    if ((!(new_b & (1 << 13)) && (delta_b & (1 << 13)))) IN_HandleKeyUp(IN_SC_Enter, 0);

    //B (Make the B button work in the main menu)
    if (((new_b & (1 << 5)) && (delta_b & (1 << 5))))  IN_HandleKeyDown(IN_SC_B, 0);
    if ((!(new_b & (1 << 5)) && (delta_b & (1 << 5)))) IN_HandleKeyUp(IN_SC_B, 0);
}

static void IN_T4_WaitKey()
{
    return;
}

static void IN_T4_Startup(bool disableJoysticks)
{
    usbh.begin();
    IN_SetControlType(0, IN_ctrl_Joystick1);
    IN_SetJoyConf(IN_joy_jump, IN_joy_jump);
    IN_SetJoyConf(IN_joy_pogo, IN_joy_pogo);
    IN_SetJoyConf(IN_joy_fire, IN_joy_fire);
    IN_SetJoyConf(IN_joy_deadzone, 60);
}

static bool IN_T4_StartJoy(int joystick)
{
    return true;
}

static void IN_T4_StopJoy(int joystick)
{
    return;
}

static bool IN_T4_JoyPresent(int joystick)
{
    if (joystick == 0)
        return joy1 == true;
    if (joystick == 1)
        return joy2 == true;
    return false;
}

static void IN_T4_JoyGetAbs(int joystick, int *x, int *y)
{
    *x = x_axis;
    *y = -y_axis + 1;
    if (new_b & (1 << 8))
        *y = INT16_MIN;
    if (new_b & (1 << 9))
        *y = INT16_MAX;
    if (new_b & (1 << 10))
        *x = INT16_MIN;
    if (new_b & (1 << 11))
        *x = INT16_MAX;
}

static uint16_t IN_T4_JoyGetButtons(int joystick)
{
    uint16_t mask = 0;
    if (IN_T4_JoyPresent(joystick) == false)
    {
        return mask;
    }

    if (new_b & (1 << 4))
        mask |= (1 << IN_joy_jump);
    if (new_b & (1 << 5))
        mask |= (1 << IN_joy_fire);
    if (new_b & (1 << 6))
        mask |= (1 << IN_joy_pogo);
    if (new_b & (1 << 12))
        mask |= (1 << IN_joy_menu);
    if (new_b & (1 << 13))
        mask |= (1 << IN_joy_status);

    return mask;
}

static const char *IN_T4_JoyGetName(int joystick)
{
    return "USB Joystick";
}

static IN_Backend in_t4_backend = {
    .startup = IN_T4_Startup,
    .shutdown = 0,
    .pumpEvents = IN_T4_PumpEvents,
    .waitKey = IN_T4_WaitKey,
    .joyStart = IN_T4_StartJoy,
    .joyStop = IN_T4_StopJoy,
    .joyPresent = IN_T4_JoyPresent,
    .joyGetAbs = IN_T4_JoyGetAbs,
    .joyGetButtons = IN_T4_JoyGetButtons,
    .joyGetName = IN_T4_JoyGetName,
    .joyAxisMin = -32767,
    .joyAxisMax = 32768,
};

IN_Backend *IN_Impl_GetBackend()
{
    return &in_t4_backend;
}
