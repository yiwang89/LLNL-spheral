#include "libs.hh"
#include "classes.hh"
#include "headers.hh"
namespace FractalSpace
{
  void poisson_solver_struct(Fractal& fractal,Fractal_Memory& mem,const int& level)
  {
    double timea,timeb,time0,time1,time2,time3,time4,time5,time6,time7,time8;
    timea=mem.p_mess->Clock();
    static int _COUNTER=0;
    int RANK=-1;
    MPI_Comm_rank(MPI_COMM_WORLD,&RANK);
    int spacing=Misc::pow(2,fractal.get_level_max()-level);
    // cerr << "POISSON A " << RANK << " " << level << " " << " " << _COUNTER << "\n";
    for(int ni=0;ni<2;ni++)
      {
	vector <vector <Point*> >hypre_points;
	vector <vector <Point*> >SPoints;
	vector <vector <int> >SBoxes;
	time0=mem.p_mess->Clock();
	hypre_points_struct(mem,mem.all_groups[level],hypre_points,ni == 1,level);
	time1=mem.p_mess->Clock();
	hypre_points_boxes(hypre_points,spacing,ni==1,SBoxes,SPoints);
	time2=mem.p_mess->Clock();
	hypre_points.clear();
	hypre_world_create(mem,level,SBoxes,ni == 1);
	time3=mem.p_mess->Clock();
	if(mem.p_mess->IAmAHypreNode)
	  {
	    box_stats(mem,level,ni,SBoxes,SPoints);
	    time4=mem.p_mess->Clock();
	    hypre_solve_struct(mem,level,SBoxes,SPoints);
	    time5=mem.p_mess->Clock();
	    if(ni == 1)
	      add_buffer_values(mem,level,SBoxes,SPoints);
	    time6=mem.p_mess->Clock();
	  }
	time7=mem.p_mess->Clock();
	mem.p_mess->HypreGroupFree();
	time8=mem.p_mess->Clock();
	SBoxes.clear();
	SPoints.clear();
	if(mem.p_mess->HypreRank == 0)
	  {
	    cerr << " HYPRE RES B " <<  RANK << " " << ni << " " << level << " " << _COUNTER;
	    cerr << " " << time1-time0 << " " << time2-time1 << " " << time3-time2 << " " << time5-time4 << " " << time6-time5 <<  " " << time8-time7 << "\n";
	  }
	_COUNTER++;
      }
    timeb=mem.p_mess->Clock();
    // cerr << " HYPRE RES C " <<  RANK << " " << level << " " << _COUNTER << " " << timeb-timea << "\n";
  }
}
