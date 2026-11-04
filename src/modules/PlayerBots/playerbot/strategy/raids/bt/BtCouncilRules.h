#pragma once

#include <vector>

namespace ai
{
    namespace bt
    {
        // Illidari Council target, pull and interrupt rules, kept free of game types for the offline tests
        namespace council
        {
            enum class Member
            {
                Gathios,
                Malande,
                Veras,
                Zerevor,
                None
            };

            // With a single hunter the one Misdirection goes to Gathios, the hardest hitter: the whole council
            // links onto whoever hits first, and Gathios on the wrong player killed the main tank 2 s in.
            // The mage tank gets Zerevor himself through Spellsteal and damage.
            inline Member MisdirectTarget(int hunterIndex)
            {
                switch (hunterIndex)
                {
                case 0: return Member::Gathios;
                case 1: return Member::Malande;
                case 2: return Member::Veras;
                case 3: return Member::Zerevor;
                default: return Member::None;
                }
            }

            // Arcane Explosion (41524, 10 yard radius) only goes off while someone stands that close to Zerevor
            constexpr float ARCANE_EXPLOSION_RADIUS = 10.0f;
            constexpr float ZEREVOR_TOO_CLOSE = ARCANE_EXPLOSION_RADIUS + 3.0f;
            constexpr float ZEREVOR_SAFE_DISTANCE = ARCANE_EXPLOSION_RADIUS + 6.0f;

            // Before the pull all four stand together on the dais, and the melee tanks have to walk in to pick up
            // their member; they only step out once the pull window (dps wait) is over. secondsSincePull is
            // negative before the pull.
            constexpr long long PULL_WINDOW_SECONDS = 8;

            inline bool MustLeaveZerevor(float distance, bool zerevorInCombat, bool meleeTank, long long secondsSincePull)
            {
                if (!zerevorInCombat || distance >= ZEREVOR_TOO_CLOSE)
                    return false;

                return !meleeTank || secondsSincePull >= PULL_WINDOW_SECONDS;
            }

            // The health pool is shared, so the target only decides where the damage is safe. Everybody on Gathios;
            // the Malande group (rogues and the dps with an interrupt) stays at Malande for Circle of Healing and
            // Divine Wrath. Veras wipes his threat after every Deadly Poison: the dps on him took him over each
            // time, so he is only hit while his tank holds him and nothing else is left.
            inline std::vector<Member> DpsOrder(bool malandeGroup, bool malandeInZerevorAoe, bool verasHeldByTank)
            {
                std::vector<Member> order;
                if (malandeGroup && !malandeInZerevorAoe)
                    order.push_back(Member::Malande);
                order.push_back(Member::Gathios);
                if (!malandeGroup && !malandeInZerevorAoe)
                    order.push_back(Member::Malande);
                if (verasHeldByTank)
                    order.push_back(Member::Veras);
                return order;
            }

            // The main tank drags Gathios to a corner once and then stands still: stepping towards the exact point
            // every tick kept the paladin tank walking, Gathios hit the casters (1.4k tank damage in 3 minutes).
            // Only Consecration / Blizzard / Flamestrike under the tank moves him on to the next corner.
            constexpr float GATHIOS_CORNER_REACHED = 2.0f;
            constexpr float GATHIOS_DRAG_DISTANCE = 10.0f;

            enum class TankMove
            {
                Stay,
                StepToCorner,
                NextCorner
            };

            inline TankMove MainTankMove(float distanceToCorner, bool standsInDanger)
            {
                if (standsInDanger)
                    return distanceToCorner <= GATHIOS_CORNER_REACHED ? TankMove::NextCorner : TankMove::StepToCorner;
                return distanceToCorner > GATHIOS_DRAG_DISTANCE ? TankMove::StepToCorner : TankMove::Stay;
            }

            // Consecration (8 y) and Thunder Clap (10 y) of a tank only touch his own member when no other council
            // member stands within this range; blocking them always left the tanks without threat
            constexpr float AOE_THREAT_RANGE = 12.0f;

            inline bool AoeThreatAllowed(bool otherMemberInRange)
            {
                return !otherMemberInRange;
            }

            // Blessing of Protection stops Kick / Shield Bash / Pummel, Blessing of Spell Warding stops
            // Counterspell / Earth Shock / Silence
            inline bool InterruptLands(bool physicalInterrupt, bool blessingOfProtection, bool blessingOfSpellWarding)
            {
                return physicalInterrupt ? !blessingOfProtection : !blessingOfSpellWarding;
            }
        }
    }
}
