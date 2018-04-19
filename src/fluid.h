#ifndef FLUID_H
#define FLUID_H

#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "CGL/CGL.h"
#include "CGL/misc.h"
#include "collision/collisionObject.h"
#include "collision/particle.h"

using namespace CGL;
using namespace std;

enum e_orientation { HORIZONTAL = 0, VERTICAL = 1 };

struct FluidParameters {
  FluidParameters() {}
  FluidParameters(double damping,
                  double density, double ks)
      : damping(damping), density(density), ks(ks) {}
  ~FluidParameters() {}

  // Global simulation parameters

  double damping;

  // Mass-spring parameters
  double density;
  double ks;
};

struct Fluid {
  Fluid() {}
  Fluid(double width, double length, double height, double particle_radius,
        int num_particles, int num_height_points,
        int num_width_points, int num_length_points);
  ~Fluid();

  void buildGrid();
  GLfloat* getBuffer();
  void simulate(double frames_per_sec, double simulation_steps, FluidParameters *fp,
                vector<Vector3D> external_accelerations,
                vector<CollisionObject *> *collision_objects);

  void reset();
  void saveVoxelsToMitsuba(Vector3D min, Vector3D max);

  // Fluid properties
  double width;
  double length;
  double height;
  double particle_radius;
  int num_width_points;
  int num_length_points;
  int num_height_points;
  int neighborhood_particle;
  int solver_iters = 3;
  
  Vector3D num_cells;
  bool firstFile = true;

  double radius;
  double friction;

  // Used to find neighboring particles
  double RHO_O = 25000;
  double mass = 1;

  double R=0.1;
  double W_CONSTANT =  315.0/(64.0*PI*pow(R,9));
  double W_DEL_CONSTANT = 45.0/(PI*pow(R,6)); //TODO this maybe negated

  // Fluid components
  vector<Particle> particles;

  // Spatial hashing
  unordered_map<string, vector<Particle *> *> map;
  
  // height, width, length
  std::vector<std::vector<std::vector<bool>>> voxelGrid;

  void build_spatial_map();
  void build_voxel_grid();
  string hash_position(Vector3D pos, int xOffset=0, int yOffset=0, int zOffset=0);
  std::vector<std::vector<Particle *>> generateNeighborArray();

  std::vector<Particle *> getNeighbors(Vector3D pos);

  double W(Vector3D r);
  Vector3D del_W(Vector3D r);
  double C_i(Particle p);
  void update_density(std::vector<std::vector<Particle *>>  neighborArray);
  void update_delta_p(std::vector<std::vector<Particle *>> neighborArray);
  double rho_i(Particle p);
  double lambda(Particle i, std::vector<std::vector<Particle *>>  neighborArray);
  void update_lambdas(std::vector<std::vector<Particle *>>  neighborArray);
  Vector3D del_ci_i(Particle i, std::vector<Particle *> neighbors);

  Vector3D del_ci_j(Particle i, Particle k);

  Vector3D f_vorticity(Particle p);
  void apply_viscosity(Particle p);
  void update_omega();

};

#endif /* FLUID_H */
