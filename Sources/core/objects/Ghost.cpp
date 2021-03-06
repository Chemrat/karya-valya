#include "Ghost.h"

#include "../Map.h"

#include "net/MagicStrings.h"
#include "representation/Chat.h"
#include "../ObjectFactory.h"
#include "Lobby.h"
#include "representation/Text.h"
#include "LoginMob.h"
#include "../Game.h"

#include "Human.h"

Ghost::Ghost(size_t id) : IMob(id)
{
    tickSpeed = 1;
    pixSpeed = 2;

    v_level = 11;

    SetSprite("icons/mob.dmi");
    SetState("ghost_grey");

    passable_level = Passable::EMPTY;

    name = "Ghost";

    seconds_until_respawn_ = 15;
}

void Ghost::AfterWorldCreation()
{
    IMob::AfterWorldCreation();
    SetFreq(10);
}

bool Ghost::IsMobGhost()
{
    static size_t mob_id = 0;
    static bool draw = true;
    if (!GetGame().GetMob())
    {
        return false;
    }
    if (mob_id != GetGame().GetMob().ret_id())
    {
        if (id_ptr_on<Ghost> g = GetGame().GetMob())
        {
            draw = true;
        }
        else
        {
            draw = false;
        }
        mob_id = GetGame().GetMob().ret_id();
    }
    return draw;
}

void Ghost::Represent()
{
    if (IsMobGhost())
    {
        IMob::Represent();
    }
}

void Ghost::CalculateVisible(std::list<PosPoint>* visible_list)
{
    visible_list->clear();
    PosPoint p;
    p.posz = GetZ();
    int x_low_border = std::max(0, GetX() - SIZE_H_SQ - 1);
    int x_high_border = std::min(GetGame().GetMap().GetWidth(), GetX() + SIZE_H_SQ);
    int y_low_border = std::max(0, GetY() - SIZE_W_SQ - 2);
    int y_high_border = std::min(GetGame().GetMap().GetHeight(), GetY() + SIZE_W_SQ);
    for (int i = x_low_border; i < x_high_border; ++i)
    {
        for (int j = y_low_border; j < y_high_border; ++j)
        {
            p.posx = i;
            p.posy = j;
            visible_list->push_back(p);
        }
    }
}

void Ghost::processGUImsg(const Message2& msg)
{
    IMob::processGUImsg(msg);
}

void Ghost::InitGUI()
{
    GetGame().GetTexts()["RespawnCount"].SetUpdater
    ([this](std::string* str)
    {
        std::stringstream conv;
        conv << "Until respawn: " << seconds_until_respawn_;
        *str = conv.str();
    }).SetFreq(250);
}

void Ghost::DeinitGUI()
{
    GetGame().GetTexts().Delete("RespawnCount");
}

void Ghost::Process()
{
    --seconds_until_respawn_;
    if (seconds_until_respawn_ < 0)
    {
        size_t net_id = GetGame().GetFactory().GetNetId(GetId());
        if (net_id)
        {
            auto login_mob = Create<IMob>(LoginMob::T_ITEM_S(), 0);

            GetGame().GetFactory().SetPlayerId(net_id, login_mob.ret_id());
            if (GetId() == GetGame().GetMob().ret_id())
            {
                GetGame().ChangeMob(login_mob);
            }
            Delete();
            //qDebug() << "Ghost deleted: net_id: " << net_id;
        }
    }
}
