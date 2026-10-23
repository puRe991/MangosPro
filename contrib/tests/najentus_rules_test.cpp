#include "../../src/modules/PlayerBots/playerbot/strategy/raids/bt/BtNajentusRules.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>

using namespace ai::bt::najentus;

namespace
{
    struct Point { float x, y; };

    std::vector<Point> RangedRing(int count)
    {
        std::vector<Point> points;
        for (int i = 0; i < count; ++i)
        {
            float radius, angle;
            RangedSlot(i, count, radius, angle);
            points.push_back({ radius * std::cos(angle), radius * std::sin(angle) });
        }
        return points;
    }

    float MinDistance(const std::vector<Point>& points)
    {
        float best = 1e9f;
        for (size_t i = 0; i < points.size(); ++i)
            for (size_t j = i + 1; j < points.size(); ++j)
                best = std::fmin(best, std::hypot(points[i].x - points[j].x, points[i].y - points[j].y));
        return best;
    }
}

int main()
{
    // Ranged never stand within one Needle Spine splash (6 yards) of each other, up to a full 25 man raid
    for (int count = 2; count <= 25; ++count)
        assert(MinDistance(RangedRing(count)) > 6.0f);

    // Small groups stay on the inner ring, a crowded raid uses both rings within spell range
    float radius, angle;
    RangedSlot(14, 15, radius, angle);
    assert(radius == INNER_RING);
    RangedSlot(24, 25, radius, angle);
    assert(radius == OUTER_RING);

    // Melee groups: behind the boss, out of the frontal arc, more than a splash apart
    const float orientation = 0.0f;   // boss faces +x, tank stands there
    const float meleeRadius = MeleeRadius(1.5f);
    Point a { meleeRadius * std::cos(MeleeGroupAngle(0, orientation)), meleeRadius * std::sin(MeleeGroupAngle(0, orientation)) };
    Point b { meleeRadius * std::cos(MeleeGroupAngle(1, orientation)), meleeRadius * std::sin(MeleeGroupAngle(1, orientation)) };
    assert(a.x < 0.0f && b.x < 0.0f);
    assert(std::hypot(a.x - b.x, a.y - b.y) > 6.0f);
    assert(MeleeGroupAngle(0, orientation) == MeleeGroupAngle(2, orientation));

    // Tidal Shield: wait for the raid to be topped up, but never longer than the time limit
    assert(!ThrowSpineNow(60.0f, 0));
    assert(!ThrowSpineNow(84.9f, 9));
    assert(ThrowSpineNow(85.0f, 0));
    assert(ThrowSpineNow(40.0f, 10));

    std::cout << "PASS: Naj'entus ranged rings, melee flanks and Tidal Shield timing\n";
}
