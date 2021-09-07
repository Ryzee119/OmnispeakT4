//Copyright 2020, Ryan Wendland
//SPDX-License-Identifier: GPL-2.0
#include <Arduino.h>
extern "C"
{
#include "printf.h"
#include "ck_act.h"
#include "ck4_ep.h"
#include "ck5_ep.h"
#include "ck6_ep.h"

void CK_InitGame();
void CK_DemoLoop();
}

void _putchar(char character)
{
    Serial1.write(character);
    Serial1.flush();
}

CK_EpisodeDef *ck_currentEpisode;
void setup()
{
    Serial1.begin(115200);

    FS_Startup();
    MM_Startup();
    CFG_Startup();

    ck_currentEpisode = &ck4_episode;
    ck_currentEpisode->defineConstants();
    ck_currentEpisode->hasCreatureQuestion = false;

    CK_InitGame();
}

void loop()
{
    CK_DemoLoop();
    CK_ShutdownID();
}
