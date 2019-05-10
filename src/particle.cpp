//
//  particle.cpp
//
//  Created by Yusuf Fateen on 4/30/19.
//

#include "particle.h"
#include <cmath>

void ParticleGrid::update_particles(uint8_t magnitude, uint8_t onset, uint8_t beat, int t) {
  init_boxes();
  float multiplier = magnitude / 255.0;
  // Apply some gravity!
  for (Particle &particle : particles) {
      particle.acceleration.y = 100 * multiplier - 20;
      particle.acceleration.x = 100 * multiplier - 20;
  }
//  for (Particle &particle : particles) {
//    for (Particle &neighbor : particles) {
//      interact(particle, neighbor);
//    }
//  }
    if ((int) beat != 0) {
            colorCount+= 1;
            colorVector = rainbow[colorCount % rainbow.size()];
        for (Particle &particle : particles) {
            //      float r = (float) rand() / (float) RAND_MAX;
            //      float g = (float) rand() / (float) RAND_MAX;
            //      float b = (float) rand() / (float) RAND_MAX;
            particle.color = colorVector[rand() % colorVector.size()];
        }
    }
  for (Particle &particle : particles) {
    move(particle);
    int natural_radius = (((uint64_t) &particle & 0xff0) >> 4) % 20;
    particle_collision(particle, particles);
    particle.radius += (natural_radius - particle.radius) * 0.01 + 3 * onset;
  }
  ts++;
}

// Placeholder!!
void apply_force(Particle &particle, Particle &neighbor);

void ParticleGrid::interact(Particle &particle, Particle &neighbor) {
  apply_force(particle, neighbor);
}

void ParticleGrid::add(const Particle& particle) {
  cout << "particle at x=" << particle.position.x << endl;
  particles.push_back(particle);
  Particle *p = &particles.back();
  // TODO: add p to grid!
}


bool ParticleGrid::circle_overlap(float x1, float y1, float r1, float x2, float y2, float r2) {
    return fabs((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2)) <= (r1 + r2)*(r1 + r2);
}


void ParticleGrid::particle_collision(Particle &particle, list<Particle> neighbors) {
    vector<pair<Particle*, Particle*>> vecCollidingPairs;
    
    for (auto &neighbor: neighbors) {
        if (particle.ballid != neighbor.ballid) {
            if (circle_overlap(particle.position[0], particle.position[1], particle.radius, neighbor.position[0], neighbor.position[1], neighbor.radius)) {
                
                vecCollidingPairs.push_back({ &particle, &neighbor });
                
                float distanceBetweenCenters = sqrtf((particle.position[0] - neighbor.position[0])*(particle.position[0] - neighbor.position[0]) +
                                                     (particle.position[1] - neighbor.position[1])*(particle.position[1] - neighbor.position[1]));
                float overlap = 0.5f * (distanceBetweenCenters - particle.radius - neighbor.radius);
                
                particle.position[0] -= overlap * (particle.position[0] - neighbor.position[0]) / distanceBetweenCenters;
                particle.position[1] -= overlap * (particle.position[1] - neighbor.position[1]) / distanceBetweenCenters;
                
                neighbor.position[0] -= overlap * (neighbor.position[0] - particle.position[0]) / distanceBetweenCenters;
                neighbor.position[1] -= overlap * (neighbor.position[1] - particle.position[1]) / distanceBetweenCenters;
            }
            
        }
    }
    colliding_pairs(vecCollidingPairs);
}

void ParticleGrid::colliding_pairs(vector<pair<Particle*, Particle*>> vecCollidingPairs) {
    
    for (auto pair: vecCollidingPairs) {
        Particle *p1 = pair.first;
        Particle *p2 = pair.second;
        
        // Distance between balls
        float fDistance = sqrtf((p1->position[0] - p2->position[0])*(p1->position[0] - p2->position[0]) + (p1->position[1] - p2->position[1])*(p1->position[1] - p2->position[1]));
        
        float nx = (p2->position[0] - p1->position[0]) / fDistance;
        float ny = (p2->position[1] - p1->position[1]) / fDistance;
        float tx = -ny;
        float ty = nx;
        
        float dpTan1 = p1->velocity[0] * tx + p1->velocity[1] * ty;
        float dpTan2 = p2->velocity[0] * tx + p2->velocity[1] * ty;
        float dpNorm1 = p1->velocity[0] * nx + p1->velocity[1] * ny;
        float dpNorm2 = p2->velocity[0] * nx + p2->velocity[0] * ny;
        
        float m1 = (dpNorm1 * (p1->mass - p2->mass) + 2.0f * p2->mass * dpNorm2) / (p1->mass + p2->mass);
        float m2 = (dpNorm2 * (p2->mass - p1->mass) + 2.0f * p1->mass * dpNorm1) / (p1->mass + p2->mass);
        
        //        p1->velocity[0] = tx * dpTan1 + nx * m1;
        //        p1->velocity[1] = ty * dpTan1 + ny * m1;
        //        p2->velocity[0] = tx * dpTan2 + nx * m2;
        //        p1->velocity[1] = ty * dpTan2 + ny * m2;
        //
        float p1vx = tx * dpTan1 + nx * m1;
        float p1vy = ty * dpTan1 + ny * m1;
        float p2vx = tx * dpTan2 + nx * m2;
        float p2vy = ty * dpTan2 + ny * m2;
        
        //        Dampen the velocities
        //        p1->velocity[0] = (p1vx > 100.0) ? (p1vx*(3.0/4.0)) : (p1vx);
        p1->velocity[1] = (p1vy > 100.0) ? (p1vy*(3.0/4.0)) : (p1vy);
        //        p2->velocity[0] = (p2vx > 100.0) ? (p2vx*(3.0/4.0)) : (p2vx);
        p2->velocity[1] = (p2vy > 100.0) ? (p2vy*(3.0/4.0)) : (p2vy);
        
    }
}


// FOLLOWING IS *PLACEHOLDER* INTERACTION CODE FROM CS 267 HOMEWORK

void ParticleGrid::move(Particle &particle) {
  //
  //
  //  slightly simplified Velocity Verlet integration
  //  conserves energy better than explicit Euler method
  //
  particle.velocity += particle.acceleration * dt;
  particle.position += particle.velocity * dt;

  //
  //  bounce from walls
  //
  double *x = &(particle.position.x);
  double *y = &(particle.position.y);
  if (*x < particle.radius || *x > width-particle.radius)
  {
    *x = *x < particle.radius ? particle.radius : width-particle.radius;
    particle.velocity.x *= -1;
  }
  if (*y < particle.radius || *y > height-particle.radius)
  {
    *y = *y < particle.radius ? particle.radius : height-particle.radius;
    particle.velocity.y *= -1;
  }
  *x = max(particle.radius, min((double) (width-particle.radius), *x));
  *y = max(particle.radius, min((double) (height-particle.radius), *y));
}

void ParticleGrid::compute_density(Particle &particle, list<Particle> &neighbors) {

}

void ParticleGrid::compute_pressure(Particle &particle, list<Particle> &neighbors) {

}

vector<Particle *>* ParticleGrid::get_grid_box(Particle &particle) {
  int x = (int) (grid_width * particle.position.x / width);
  int y = (int) (grid_height * particle.position.y / width);
  return &grid[y * grid_height + x];
}

void ParticleGrid::resize(size_t w, size_t h) {
  width = w;
  height = h;
  grid_width = width / BOX_SIZE;
  grid_height = height / BOX_SIZE;
  grid.clear();
  grid.resize(grid_width * grid_height);
  cout << width << " " << height << endl;
  cout << grid_width << "x" << grid_height << endl;
}

void ParticleGrid::init_boxes() {
  for (vector<Particle*> &v : grid) {
    v.clear();
  }
  for (Particle &p : particles) {
    get_grid_box(p)->push_back(&p);
  }
}

void apply_force(Particle &particle, Particle &neighbor) {
  if (&particle == &neighbor) return;
  Vector2D dPosition = neighbor.position - particle.position;
  particle.acceleration -= 0.00000001 * dPosition.unit() * dPosition.norm2() / particle.mass;
  if (dPosition.norm() < particle.radius + neighbor.radius) {
    particle.velocity = -1 * dPosition;
  }
}
