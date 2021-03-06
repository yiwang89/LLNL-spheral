#include "libs.hh"
#include "classes.hh"
#include "headers.hh"
namespace FractalSpace
{
  void fix_memory(Fractal& frac,const int& ispace,const int& jfield)
  {
    for(int i=0;i<frac.get_number_particles();i++)
      {
	Particle* par=frac.particle_list[i];
	par->space_resize(ispace);
	par->field_resize(jfield);
      }
  }
}
