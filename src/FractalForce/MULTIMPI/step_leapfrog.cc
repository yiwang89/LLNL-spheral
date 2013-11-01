#include "libs.hh"
#include "classes.hh"
#include "headers.hh"
namespace FractalSpace
{
  string Fractal::integrator("leapfrog");
  //
  template <class M> void step_simple(M& mem, Fractal& fractal)
  {
    fractal.timing(-2,0);
    fractal.timing(-1,49);
    mem.p_file->FileFractal << " Made It step a " << endl;
    fractal_force(fractal,mem);
    mem.p_file->FileFractal << " Made It step b " << endl;
    fractal.timing(1,49);
    fractal.timing(0,0);
    fractal.timing_lev(0,0);
    //    ofstream& FP=mem.p_file->FileParticle;
    vector <double> pos(3);
    vector <double> vel(3);
    vector <double> force(3);
    if(mem.periodic)
      {
	double parad=pow(mem.arad,mem.pexp);
	double dpda=mem.pexp*parad/mem.arad;
	double parad_half=parad+0.5*mem.step_length;
	double arad_half=pow(parad_half,1.0/mem.pexp);
	double dadt=arad_half*M::hubble(arad_half,mem.omega_start,mem.lambda_start);
	double v_const=1.0-2.0*mem.step_length/mem.arad/dpda;
	double f_const=mem.step_length/pow(mem.arad,4)/M::hubble(arad_half,mem.omega_start,mem.lambda_start)/dpda;
	double dt=mem.step_length/dadt/dpda;
	for(int n=0;n < fractal.get_number_particles();++n)
	  {
	    Particle* p=fractal.particle_list[n];
	    if(!p->get_real_particle())
	       continue;
	    p->get_field(pos,vel,force);
	    vel[0]=vel[0]*v_const+force[0]*f_const;
	    vel[1]=vel[1]*v_const+force[1]*f_const;
	    vel[2]=vel[2]*v_const+force[2]*f_const;
	    pos[0]+=vel[0]*dt;
	    pos[1]+=vel[1]*dt;
	    pos[2]+=vel[2]*dt;
	    p->set_phase(pos,vel);
	  }
	mem.time+=mem.step_length/dadt/dpda;
	if(!mem.start_up)
	  parad+=mem.step_length;
	mem.arad=pow(parad,1.0/mem.pexp);
      }
    else
      {
	double dt=mem.step_length;
	mem.time+=dt;
	for(int n=0;n < fractal.get_number_particles();++n)
	  {
	    Particle* p=fractal.particle_list[n];
	    if(!p->get_real_particle())
	       continue;
	    p->get_field(pos,vel,force);
	    //	    p->dump(FP);
	    vel[0]+=force[0]*dt;
	    vel[1]+=force[1]*dt;
	    vel[2]+=force[2]*dt;
	    pos[0]+=vel[0]*dt;
	    pos[1]+=vel[1]*dt;
	    pos[2]+=vel[2]*dt;
	    p->set_phase(pos,vel);
	    //	    p->dump(FP);
	  }
      }
  }
}
namespace FractalSpace
{
  template void step_simple(Fractal_Memory& mem, Fractal& fractal);
}
