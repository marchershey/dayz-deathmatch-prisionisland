
#include "C:/Program Files (x86)/Steam/steamapps/common/DayZServer/mpmissions/prisonisland-deathmatch.chernarusplus/inc/spawnitems/clothes.c";
// #include "C:/Program Files (x86)/Steam/steamapps/common/DayZServer/mpmissions/prisonisland-deathmatch.chernarusplus/crates.c";
#include "C:/Program Files (x86)/Steam/steamapps/common/DayZServer/mpmissions/prisonisland-deathmatch.chernarusplus/inc/spawnitems/weapons.c";

// #include "inc/spawnitems/clothes.c";
// #include "inc/spawnitems/weapons.c";

void main() {
    //INIT WEATHER BEFORE ECONOMY INIT------------------------
    Weather weather = g_Game.GetWeather();

    // weather.MissionWeather(true); // false = use weather controller from Weather.c

    weather.GetOvercast().Set(0, 0, 0);
    weather.GetRain().Set(0, 0, 0);
    weather.GetFog().Set(0, 0, 0);

    //INIT ECONOMY--------------------------------------
    Hive ce = CreateHive();
    if (ce)
        ce.InitOffline();

    //DATE RESET AFTER ECONOMY INIT-------------------------
    int year, month, day, hour, minute;
    int reset_month = 8, reset_day = 10;
    GetGame().GetWorld().GetDate(year, month, day, hour, minute);

    if ((month == reset_month) && (day < reset_day)) {
        GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
    } else {
        if ((month == reset_month + 1) && (day > reset_day)) {
            GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
        } else {
            if ((month < reset_month) || (month > reset_month + 1)) {
                GetGame().GetWorld().SetDate(year, reset_month, reset_day, hour, minute);
            }
        }
    }
}

class CustomMission : MissionServer {
    static private int DEFAULT_ROUND_DURATION = 10;
    static private int COUNTDOWN_DURATION_MS = 10000;
    static private const vector LIMBO_POSITON = "2613 24 1312";
    static private const vector PLAYAREA_CENTER = "2685 0 1311";
    static private const int CLEANUP_RADIUS = 600;
    static private const int KILL_RADIUS = 300;
    static private const int PLAYAREA_RADIUS = 250;

    autoptr TStringStringMap m_Identities = new TStringStringMap();

    autoptr Clothes clothes = new Clothes();
    autoptr Weapons weapons = new Weapons();

    int m_round_duration;

    void CustomMission() {
        // Check if the round duration is set in the config.
        m_round_duration = GetGame().ServerConfigGetInt("deathmatchRoundMinutes");

        // if it is, then set it
        if (m_round_duration < 1) {
            m_round_duration = DEFAULT_ROUND_DURATION;
        }

        // Spawn creates
        // Crates.SpawnCrates(game); // disabled due to not being finished

        // Make sure players do not leave the play area.
        // Checked location every 10 seconds (true = repeat)
        GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.StartBoundarySystem, 10000, true);

        // Start the round
        this.StartRound();
    }

  private void StartRound() {
        // set GetGame as a var
        CGame game = GetGame();

        // set countdown delay.
        // this just takes the round duration and sets it -10 seconds to start the "10 second timer"
        int delay = (m_round_duration * 60000) - COUNTDOWN_DURATION_MS;
        // start the queue for the countdown timer, set the delay as the queue time
        game.GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.EndRoundCountdown, delay, false, COUNTDOWN_DURATION_MS);

        // notify all players that the round has started
        this.NotifyAllPlayers("The round has started. Good luck!", 10);
    }

  private void EndRound() {

        // set the queue in a shorter var
        ScriptCallQueue queue = GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY);
        queue.CallLater(this.StartRound, 5000, false);

        // Notify all the players that the round has ended
        this.NotifyAllPlayers("The round has ended!");

        // create men array
        array<Man> men = new array<Man>();
        // Get all players and put them in array
        GetGame().GetPlayers(men);
        // place each player in lobby
        foreach (Man man : men) {
            PlayerBase player = PlayerBase.Cast(man);
            if (player != null) {
                player.ClearInventory();
                player.RemoveAllItems();
                // this.PlaceInLobby(player);
            }
        }

        this.KillAllPlayers();
        // delete all items on the ground
        this.CleanUpItems();
        // delete all dead bodies
        this.CleanUpDeadBodies();
    }

  private void StartBoundarySystem() {
        array<Man> players = new array<Man>();
        GetGame().GetPlayers(players);
        foreach (Man player : players) {
            if (player.IsAlive()) {
                float distance = this.DistanceFromCenter(player.GetPosition());

                if (distance > PLAYAREA_RADIUS) {
                    NotificationSystem.SendNotificationToPlayerExtended(player, 5.0, "You are outside the zone!", "You will continue to lose health until you return to the zone.");
                    if (distance > KILL_RADIUS) {
                        player.SetHealth("", "", 0.0);
                    } else {
                        player.SetHealth("", "", player.GetHealth() - 33);
                    }
                }
            }
        }
    }

  private float DistanceFromCenter(vector pos) {
        vector adjusted = Vector(pos[0], 0, pos[2]);

        return vector.Distance(PLAYAREA_CENTER, adjusted);
    }

  private void EndRoundCountdown(int duration) {
        if (duration <= 0) {
            this.EndRound();
        } else {
            int timeLeft = duration / 1000;
            this.NotifyAllPlayers("Round ends in " + timeLeft + " seconds", 0.1);
            GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(this.EndRoundCountdown, 900, false, duration - 1000);
        }
    }

  private void PlaceInLobby(EntityAI player) {
        player.SetPosition(LIMBO_POSITON);
    }

  private void KillAllPlayers() {
        array<Man> men = new array<Man>();
        GetGame().GetPlayers(men);
        foreach (Man man : men) {
            Print("Checking man " + man);
            PlayerBase playerBase = PlayerBase.Cast(man);
            if (playerBase != null) {
                playerBase.SetHealth("", "", 0.0);
            }
        }
    }

  private void CleanUpDeadBodies() {
        array<Man> men = new array<Man>();
        GetGame().GetPlayers(men);
        foreach (Man man : men) {
            PlayerBase playerBase = PlayerBase.Cast(man);
            if (playerBase != null) {
                playerBase.ClearInventory();
                playerBase.RemoveAllItems();
                playerBase.SetHealth("", "", 0.0);
                GetGame().GetCallQueue(CALL_CATEGORY_GAMEPLAY).CallLater(playerBase.Delete, 1000, false);
            }
        }
    }

  private void CleanUpItems() {
        CGame game = GetGame();
        array<Object> objects = new array<Object>();
        array<CargoBase> cargos = new array<CargoBase>();

        game.GetObjectsAtPosition(PLAYAREA_CENTER, CLEANUP_RADIUS, objects, cargos);

        foreach (Object obj : objects) {
            ItemBase itemBase = ItemBase.Cast(obj);
            if (itemBase != null && itemBase.GetHierarchyParent() == null) {
                Print("Cleaning up object " + itemBase);
                itemBase.Delete();
            }
        }
        Print("Done cleaning up objects");
    }

  private void NotifyAllPlayers(string message, float time = 1.0) {
        // This should use chat messaging but, because chat is broken in v1.07, we're using
        // NotificationSystem, instead.
        NotificationSystem.SendNotificationToPlayerIdentityExtended(null, time, message);
    }

    // -----------------------------------------------------------------------------------------

    override PlayerBase CreateCharacter(PlayerIdentity identity, vector pos, ParamsReadContext ctx, string characterName) {
        Entity playerEnt;
        playerEnt = GetGame().CreatePlayer(identity, characterName, pos, 0, "NONE"); //Creates random player
        Class.CastTo(m_player, playerEnt);

        GetGame().SelectPlayer(identity, m_player);

        return m_player;
    }

    void EquipPlayerForSurvival(PlayerBase player) {
        HumanInventory inventory = player.GetHumanInventory();
        EntityAI bandage = inventory.CreateInInventory("BandageDressing");
        inventory.CreateInInventory("SalineBagIV");
        inventory.CreateInInventory("TacticalBaconCan_Opened");
        inventory.CreateInInventory("WaterBottle");

        player.SetQuickBarEntityShortcut(bandage, 3);
    }

    void StartFedAndWatered(PlayerBase player) {
        player.GetStatWater().Set(player.GetStatWater().GetMax());
        player.GetStatEnergy().Set(player.GetStatEnergy().GetMax());
    }

    override void StartingEquipSetup(PlayerBase player, bool clothesChosen) {
        player.RemoveAllItems();

        EntityAI sheath = clothes.EquipPlayerClothes(player);
        this.EquipPlayerForSurvival(player);
        weapons.EquipPlayerWeapons(player, sheath);
        this.StartFedAndWatered(player);

        player.GetInventory().CreateInInventory("M67Grenade");
        player.GetInventory().CreateInInventory("M67Grenade");
        player.GetInventory().CreateInInventory("M67Grenade");
        player.GetInventory().CreateInInventory("M67Grenade");
        player.GetInventory().CreateInInventory("M67Grenade");
    }

    override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity) {

        string uid = identity.GetId();

        // Unfortunately, InvokeOnConnect gets called when players respawn, so we have to keep
        // track of connects and disconnects in order to know if this is being called for a
        // player's initial spawn.
        if (!m_Identities.Contains(uid)) {
            string name = identity.GetName();

            m_Identities.Set(uid, name);

            this.NotifyAllPlayers(name + " has entered ");
        }

        super.InvokeOnConnect(player, identity);
    }

    override void PlayerDisconnected(PlayerBase player, PlayerIdentity identity, string uid) {
        string name;
        if (m_Identities.Find(uid, name)) {
            m_Identities.Remove(uid);
            this.NotifyAllPlayers(name + " has left the arena");
        }
        super.PlayerDisconnected(player, identity, uid);
    }

    override void HandleBody(PlayerBase player) {
        // Kill character so that players start fresh every time they connect
        player.SetHealth("", "", 0.0);
    }

    // override void HandleBody(PlayerBase player) {
    //     player.DropAllItems();

    //     if (player.IsAlive() && !player.IsRestrained() && !player.IsUnconscious()) {
    //         // remove the body
    //         player.Delete();
    //     } else if (player.IsUnconscious() || player.IsRestrained()) {
    //         // kill character
    //         player.SetHealth("", "", 0.0);
    //     }
    // }

    // override void OnClientRespawnEvent(PlayerIdentity identity, PlayerBase player) {
    //     if (player) {
    //         if (player.IsUnconscious() || player.IsRestrained()) {
    //             // kill character
    //             player.SetHealth("", "", 0.0);
    //         }
    //     }
    // }

    // when a player dies
    override bool InsertCorpse(Man player) {

        PlayerIdentity identity = player.GetIdentity();
        if (identity) {
            string name = identity.GetName();
            KillerData data = player.m_KillerData;
            if (data) {
                Man killerMan = Man.Cast(data.m_Killer);
                PlayerIdentity killerIdentity;
                if (killerMan)
                    killerIdentity = killerMan.GetIdentity();
                if (killerIdentity) {
                    string killer = killerIdentity.GetName();
                    string message = killer + " killed " + name;
                    if (data.m_KillerHiTheBrain) {
                        message += " with a headshot";
                    }
                    if (data.m_MurderWeapon) {
                        message += " using a " + data.m_MurderWeapon.GetDisplayName();
                    }
                    if (killer == name) {
                        message = name + " commited suicide";
                    }
                    this.NotifyAllPlayers(message);
                } else {
                    this.NotifyAllPlayers(name + " was killed");
                }

            } else {

                this.NotifyAllPlayers(name + " has died");
            }
        }

        return super.InsertCorpse(player);
    }
};

Mission CreateCustomMission(string path) {
    return new CustomMission();
}