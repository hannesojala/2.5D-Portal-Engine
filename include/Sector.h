#pragma once

#include <linalg.h>
#include <vector>
#include <util.h>

using namespace linalg::aliases;

struct Wall;
struct Sector;

struct Wall {
    float2 p1, p2;
    int next_sector;

    bool rayIntersect(Ray ray, float2* point) const {
        float2 p3(ray.origin.x, ray.origin.y);
        float2 p4(ray.origin.x - ray.direction.x, ray.origin.y - ray.direction.y);
        float d = (p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x);
        if (d == 0.0f)
        {
            return false;
        }
        float t = ((p1.x-p3.x)*(p3.y-p4.y)-(p1.y-p3.y)*(p3.x-p4.x))/d;
        float u = ((p1.x-p2.x)*(p1.y-p3.y)-(p1.y-p2.y)*(p1.x-p3.x))/d;

        if (t > 0.0f && t < 1.0f && u > 0)
        {
            *point = float2(p1.x + t * (p2.x-p1.x), p1.y + t * (p2.y-p1.y));
            return true;
        }
        return false;
    }

    bool facingFront(Ray camera_ray) {
        float2 p2p1 = p2 - p1;
        float p = (camera_ray.direction.x * p2p1.y) - (camera_ray.direction.y * p2p1.x);
        return p > 0 ? false : true;
    }
};

struct Sector {
    float floor, ceil;
    int walls_begin, walls_end;

    bool containsPoint(float2 point, const std::vector<Wall>& walls) {
        Ray testRay = {point, {1, 1}}; // any direction works
        int numIntersections = 0;
        for (auto it = walls.begin() + walls_begin; it <= walls.begin() + walls_end; it++) {
            float2 obligatory_point;
            if ((*it).rayIntersect(testRay, &obligatory_point))
                numIntersections++;
        }
        if (numIntersections%2 > 0) return true;
        return false;
    }
};
