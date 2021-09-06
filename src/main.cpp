//Copyright 2020, Ryan Wendland
//SPDX-License-Identifier: GPL-2.0

#include <Arduino.h>
extern "C"
{
#include "id_ca.h"
#include "id_fs.h"
#include "id_in.h"
#include "id_mm.h"
#include "id_rf.h"
#include "id_us.h"
#include "id_vl.h"
#include "ck_act.h"
#include "ck_cross.h"
#include "ck_def.h"
#include "ck_game.h"
#include "ck_play.h"
#include "ck4_ep.h"
#include "ck5_ep.h"
#include "ck6_ep.h"

void CK_InitGame();
void CK_DemoLoop();
}

CK_EpisodeDef *ck_currentEpisode;
void setup()
{
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
