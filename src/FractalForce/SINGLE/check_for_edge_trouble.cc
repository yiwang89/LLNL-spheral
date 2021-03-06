#include "libs.hh"
#include "classes.hh"
#include "headers.hh"
namespace FractalSpace
{
  void check_for_edge_trouble(Fractal& fractal)
  { 
    fractal.timing(-1,2);
    //--------------------------------------------------------------------------------------------------------------------------------
    // Round off errors can cause trouble at the edge, move points a little
    //--------------------------------------------------------------------------------------------------------------------------------
    double eps=DBL_EPSILON;
    vector <double>pos(3);
    for(int part=0; part < fractal.get_number_particles();part++)
      {
	Particle* p=fractal.particle_list[part];
	p->get_pos(pos);
	bool outside=pos[0] >= 1.0 || pos[0] <=0.0 ||
	  pos[1] >= 1.0 || pos[1] <=0.0 ||
	  pos[2] >= 1.0 || pos[2] <=0.0;
	if(!outside) break;
	if(pos[0] >= 1.0)
	  pos[0]-=eps;
	else if(pos[0] <= 0.0) 
	  pos[0]+=eps;
	if(pos[1] >= 1.0) 
	  pos[1]-=eps;
	else if(pos[1] <= 0.0) 
	  pos[1]+=eps;
	if(pos[2] >= 1.0) 
	  pos[2]-=eps;
	else if(pos[2] <= 0.0) 
	  pos[2]+=eps;
	p->set_pos(pos);
      }
    fractal.timing(1,2);
  }
}
