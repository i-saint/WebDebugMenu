// created by i-saint
// distributed under Creative Commons Attribution (CC BY) license.
// https://github.com/i-saint/DynamicObjLoader

#include "stdafx.h"
#include "Test2.h"
#include <algorithm>

int g_num_particles;
float g_pradius;
float g_accel;
float g_deccel;
float g_gravity;
XMFLOAT3 g_gravity_center;

inline XMFLOAT3& operator+=(XMFLOAT3 &l, const XMFLOAT3 &r) { l.x+=r.x; l.y+=r.y; l.z+=r.z; return l; }
inline XMFLOAT3& operator*=(XMFLOAT3 &l, float r) { l.x*=r; l.y*=r; l.z*=r; return l; }
inline XMFLOAT3 operator-(const XMFLOAT3 &l, const XMFLOAT3 &r) { return XMFLOAT3(l.x-r.x, l.y-r.y, l.z-r.z); }
inline XMFLOAT3 operator*(const XMFLOAT3 &l, float r) { return XMFLOAT3(l.x*r, l.y*r, l.z*r); }
inline XMFLOAT3 operator/(const XMFLOAT3 &l, float r) { return XMFLOAT3(l.x/r, l.y/r, l.z/r); }

inline float Dot(const XMFLOAT3 a, const XMFLOAT3 b)
{
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline float Len(const XMFLOAT3 a)
{
    return sqrt(Dot(a, a));
}

inline XMFLOAT3 Normalize(XMFLOAT3 a)
{
    return a / Len(a);
}

float GetParticleRadius()
{
    return g_pradius;
}

void SetParticleRadius(float r)
{
    g_pradius = r;
    Particle *particles = g_particles;
    size_t num_particles = _countof(g_particles);
    for(size_t i=0; i<num_particles; ++i) {
        particles[i].radius = r;
    }
}

void InitializeParticles()
{
    wdmAddNode("Particle/num", &g_num_particles, 0, MAX_PARTICLES);
    wdmAddNode("Particle/radius", &GetParticleRadius, &SetParticleRadius, 0.0f, 0.25f);
    wdmAddNode("Particle/deccel", &g_deccel, 0.9f, 1.0f);
    wdmAddNode("Particle/accel", &g_accel, 0.0001f, 0.01f);
    wdmAddNode("Particle/gravity_center", &g_gravity_center, -20.0f, 20.0f);
    wdmAddNode("Particle/gravity_strength", &g_gravity, 0.0f, 0.005f);

    g_num_particles = MAX_PARTICLES/2;
    g_pradius = 0.015f;
    g_deccel = 0.998f;
    g_accel = 0.0022f;
    g_gravity = 0.002f;
    g_gravity_center = XMFLOAT3(0.0f, 1.0f, 0.0f);

    Particle *particles = g_particles;
    size_t num_particles = _countof(g_particles);
    for(size_t i=0; i<num_particles; ++i) {
        particles[i].position = XMFLOAT3(GenRand()*3.0f, GenRand()*3.0f, GenRand()*3.0f);
        particles[i].velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
        particles[i].radius = g_pradius;
    }
}

void FinalizeParticles()
{

}

void UpdateParticles()
{
    // 相互に押し返し
    Particle *particles = g_particles;
    int num_particles = g_num_particles;
    #pragma omp parallel for
    for(int ri=0; ri<num_particles; ++ri) {
        Particle &rp = particles[ri];
        XMFLOAT3 rpos = rp.position;
        float rradius = rp.radius;
        for(int si=0; si<num_particles; ++si) {
            Particle &sp = particles[si];
            float uradius = rradius + sp.radius;
            XMFLOAT3 diff = sp.position - rpos;
            float len = Len(diff);
            if(len==0.0f) { continue; } // 自分自身との衝突なので無視
            float d = len - uradius;
            if(d < 0.0f) {
                XMFLOAT3 dir = diff / len;
                rp.velocity += dir * (d * 0.2f);
            }
        }
    }

    // 中心に吸い寄せる
    {
        #pragma omp parallel for
        for(int ri=0; ri<num_particles; ++ri) {
            Particle &rp = particles[ri];
            XMFLOAT3 dir = Normalize(g_gravity_center - rp.position);
            rp.velocity += dir * g_accel;
        }
    }

    // 床とのバウンド

    #pragma omp parallel for
    for(int ri=0; ri<num_particles; ++ri) {
        Particle &rp = particles[ri];
        rp.velocity.y -= g_gravity;
        float bottom = rp.position.y - rp.radius;
        float d = bottom + 3.0f;
        rp.velocity.y += std::min<float>(0.0f, d) * -0.2f;
    }

    // 速度を適用
    #pragma omp parallel for
    for(int ri=0; ri<num_particles; ++ri) {
        Particle &rp = particles[ri];
        rp.position += rp.velocity;
        rp.velocity *= g_deccel;
    }
}
