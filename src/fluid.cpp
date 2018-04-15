#include <iostream>
#include <math.h>
#include <random>
#include <vector>

#include "fluid.h"
#include "collision/plane.h"
#include "collision/particle.h"

using namespace std;

double RHO_O = -1; //TODO in future, this should be the density function evaluated at t=0
double mass = 0.01;
#define EPSILON 1e-6

Fluid::Fluid(double width, double length, double height, double particle_radius,
             int num_particles, int num_height_points,
             int num_width_points, int num_length_points) {
  this->width = width;
  this->height = height;
  this->num_width_points = num_width_points;
  this->num_height_points = num_height_points;

  buildGrid();
}

Fluid::~Fluid() {
  particles.clear();
}

void Fluid::buildGrid() {
  // TODO (Part 1.1): Build a grid of masses.
  double w_offset = width / ((double) num_width_points);
  double l_offset = length / ((double) num_length_points);
  double h_offset = height / ((double) num_height_points);

  for (int i = 0; i < num_width_points; i++) {
    for (int j = 0; j < num_length_points; j++) {
      for (int k = 0; k < num_height_points; k++) {
        Vector3D pos = Vector3D(i * w_offset + ((double)(rand()%10 - 5))/100.,
                                j * l_offset + ((double)(rand()%10 - 5))/100.,
                                k * h_offset + ((double)(rand()%10 - 5))/100.);
        Particle p = Particle(pos, radius, friction);
        particles.emplace_back(p);
      }
    }
  }

}

GLfloat* Fluid::getBuffer() {
    GLfloat* data = (GLfloat*) malloc(sizeof(GLfloat)*this->particles.size()*7);
    int count = 0;
    for (auto particle : particles) {
        data[count * 7] = particle.origin.x;
        data[count * 7+1] = particle.origin.y;
        data[count * 7+2] = particle.origin.z;
        // data[count * 7+3] = particle.color[0];
        // data[count * 7+4] = particle.color[1];
        // data[count * 7+5] = particle.color[2];
        // data[count * 7+6] = particle.color[3];
        data[count * 7+3] = 1.0f;
        data[count * 7+4] = 1.0f;
        data[count * 7+5] = 1.0f;
        data[count * 7+6] = 1.0f;
        count += 1;
    }
    return data;
}

void Fluid::build_spatial_map() {
  for (const auto &entry : map) {
    delete(entry.second);
  }
  map.clear();

  for (Particle &particle : this->particles){
    string key = hash_position(particle.origin);
    if (map.find(key) == map.end()){
      map[key] = new std::vector<Particle *>();
    }
    map[key]->push_back(&particle);
  }
}

string Fluid::hash_position(Vector3D pos, int xOffset, int yOffset, int zOffset) {
  // TODO (Part 4.1): Hash a 3D position into a unique float identifier that represents
  // membership in some uniquely identified 3D box volume.
  int xVol = floor(pos.x / R);
  int yVol = floor(pos.y / R);
  int zVol = floor(pos.z / R);

  if (pos.x < 0){
    xVol = ceil(pos.x / R);
  }
  if (pos.y < 0){
    yVol = ceil(pos.y / R);
  }
  if (pos.z < 0){
    zVol = ceil(pos.z / R);
  }

  xVol += xOffset;
  yVol += yOffset;
  zVol += zOffset;

  return std::to_string(xVol) + "/" + std::to_string(yVol) + "/" +  std::to_string(zVol);
}

std::vector<Particle *> Fluid::getNeighbors(Vector3D pos){
  std::vector<Particle *> * neighbors = new std::vector<Particle *>();
  // Get the location of all neighboring cells in the hashmap.
  std::vector<string> neighborCellsHashes = std::vector<string>();
  neighborCellsHashes.push_back(hash_position(pos));
  neighborCellsHashes.push_back(hash_position(pos, 1, 0, 0));
  neighborCellsHashes.push_back(hash_position(pos, -1, 0, 0));
  neighborCellsHashes.push_back(hash_position(pos, 0, 1, 0));
  neighborCellsHashes.push_back(hash_position(pos, 0, -1, 0));
  neighborCellsHashes.push_back(hash_position(pos, 0, 0, 1));
  neighborCellsHashes.push_back(hash_position(pos, 0, 0, -1));


  // Iterate through the neighbor cell and check if within R distance.
  for (string neighborCellsHash : neighborCellsHashes){
    if (map[neighborCellsHash] != NULL){
      vector<Particle *> currCell = *map[neighborCellsHash];
      for (Particle* particle : currCell){
        if ((pos-particle->origin).norm() < R && ((pos-particle->origin).norm() != 0)) {
          neighbors->push_back(particle);
        }
      }
    }
  }
  return *neighbors;
}

void Fluid::simulate(double frames_per_sec, double simulation_steps, FluidParameters *fp,
                     vector<Vector3D> external_accelerations,
                      vector<CollisionObject *> *collision_objects) {
  double delta_t = 1.0f / frames_per_sec / simulation_steps;
  Particle temp(this->particles[0].origin, this->particles[0].radius, this->particles[0].friction); // just a placeholder
  // TODO (Part 2.1): Compute total force acting on each point mass.
  if (RHO_O == -1) {
    this->update_density();
    for (Particle p: particles) {
      p.rest_density = p.density;
    }
  }

  for (auto &p: particles) {
    for (auto ea: external_accelerations){
      p.velocity += delta_t*ea;
    }
    p.x_star = p.origin+delta_t*p.velocity;
  }

  int solver_iters = 5;

  for(int iter=0; iter<solver_iters; iter++) {
    build_spatial_map();
  //   this->update_density();
  //   this->update_lambdas();
  //   // TODO stuff is confusing from this point.
    for (Particle &p: this->particles) {
  //     // p.delta_p = this->delta_p(p);
  //     // p.x_star += p.delta_p;
  //     // Collision detection
      temp.origin = p.x_star;
      temp.last_origin = p.origin;
      for (CollisionObject *co : *collision_objects) {
        co->collide_particle(temp);
      }
      p.x_star = temp.origin;
    }
  }

  for (Particle &p: this->particles) {
    p.velocity = (p.x_star-p.origin)/delta_t;
     //vorticity confinement
    // Vector3D corrective_force = f_vorticity(p);
    //TODO don't know what to do with the corrective force.
    // p.velocity += corrective_force*delta_t/mass;
    //viscosity
    // this->apply_viscosity(p);
    p.last_origin = p.origin;
    p.origin = p.x_star;
  }

}


///////////////////////////////////////////////////////
/// YOU DO NOT NEED TO REFER TO ANY CODE BELOW THIS ///
///////////////////////////////////////////////////////

void Fluid::reset() {
  Particle *pm = &particles[0];
  for (int i = 0; i < particles.size(); i++) {
    pm->origin = pm->start_origin;
    pm->last_origin = pm->start_origin;
    pm++;
  }
}

/*
* Simulation Physics Code
*/

double Fluid::W(Vector3D r) { //density kernel
  double r_norm = r.norm();
  if ((r_norm < 0) || (r_norm > R)) return 0;
  return pow(R*R-r_norm*r_norm, 3.0)*W_CONSTANT;
}

Vector3D Fluid::del_W(Vector3D r) { // gradient of density kernel
  double r_norm = r.norm();
  if ((r_norm < 1e-5) || (r_norm > R)) return 0;
  return r.unit()*pow(R-r_norm,2.0)*W_DEL_CONSTANT;
}


double Fluid::C_i(Particle p){
  return p.density/p.rest_density - 1;
}


double Fluid::del_ci_pk_sq_norm(Particle i, Particle k){
  Vector3D pi = i.origin;
  Vector3D pk = k.origin;
  Vector3D diff = pi - pk;
  if (diff.norm() > R) return 0;
  if (diff.norm() > 1e-9) {
    Vector3D v = del_W(diff)/i.rest_density;
    return CGL::dot(v,v);
  }
  Vector3D accum;
  for (Particle * &pj : getNeighbors(i.origin)) {
    accum += del_W(pi-pj->origin);
  }
  accum /= i.rest_density;
  return CGL::dot(accum,accum);
}


void Fluid::update_density() {
  double ci = 0;
  for (Particle &p: this->particles) {
    double r = 0;
    Vector3D pi = p.origin;
    for (Particle * &pj: getNeighbors(pi)) {
      r += W(pi-pj->origin);
    }
    p.density = r*mass;
    ci += p.density/p.rest_density - 1;
  }
  //cout << ci << endl;
}

void Fluid::update_lambdas() {
  for (Particle &p: this->particles) {
    double accum;
    Vector3D pi = p.origin;
    for (Particle * &k :  getNeighbors(pi)) {
      accum += del_ci_pk_sq_norm(p, *k);
    }
    p.lambda = C_i(p)/(-accum-EPSILON);
  }
}

Vector3D Fluid::delta_p(Particle p){
  Vector3D accum; //TODO: might be incorrect
  Vector3D pi = p.origin;
  for (Particle * &pj: getNeighbors(pi)) {
    accum += (p.lambda+pj->lambda)*del_W(pi-pj->origin);
  }
  return accum/p.rest_density;
}

Vector3D Fluid::f_vorticity(Particle p){
  Vector3D pi = p.origin;
  Vector3D omega_i = p.omega;
  double epsilon = EPSILON;
  Vector3D N;
  for (Particle * &j: getNeighbors(pi)) {
    N += (mass/j->density)*(j->omega).norm()*del_W(pi-j->origin);
  }
  if (N.norm() > 1e-6) N.normalize();
  return epsilon*CGL::cross(N, p.omega);
}

void Fluid::update_omega(){
  for (Particle &p: this->particles){
      Vector3D accum;
      Vector3D pi = p.origin;
      for (Particle * &j: getNeighbors(pi)) {
        Vector3D vij = p.velocity - j->velocity;
        accum += CGL::cross(vij,del_W(pi-j->origin)); //TODO delW may be negative
      }
      p.omega = accum;
  }
}


void Fluid::apply_viscosity(Particle p) {
  Vector3D accum;
  Vector3D pi = p.origin;
  for (Particle * &j: getNeighbors(pi)) {
    Vector3D vij = p.velocity - j->velocity;
    accum += W(pi-j->origin)*vij;
  }
  p.velocity += 0.01*accum;
}
