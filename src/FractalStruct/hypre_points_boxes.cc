#include "libs.hh"
#include "classes.hh"
#include "headers.hh"
namespace FractalSpace
{
  void hypre_points_boxes(Fractal_Memory& mem,vector <vector <Point*> >& hypre_points,int spacing,
			  int VOLMIN,double FILLFACTOR,
			  vector < vector<int> >& SBoxes,vector < vector<Point*> >& SPoints)
  {
    static int _COUNTER=0;
    // ofstream& FHT=mem.p_file->DUMPS;
    const int SBstart=SBoxes.size();
    // int RANK=-1;
    // MPI_Comm_rank(MPI_COMM_WORLD,&RANK);
    // bool RANKY=RANK==21;
    int MAXY=Misc::pow(2,29);
    int MINY=-Misc::pow(2,29);
    for(vector <Point*>& hp : hypre_points)
      {
	vector <int> BOX(6);
	vector <int> pos(3);
	for(int B : {0,1,2,3,4,5})
	  BOX[B]=B%2 == 0 ? MAXY:MINY;
	for(Point* &p : hp)
	  {
	    p->get_pos_point(pos);
	    for(int B : {0,2,4})
	      {
		BOX[B]=min(BOX[B],pos[B/2]);
		BOX[B+1]=max(BOX[B+1],pos[B/2]);
	      }
	  }
	Misc::divide(BOX,spacing);
	for(int B : {1,3,5})
	  BOX[B]++;
	KdTree* pHypTree=new KdTree();
	pHypTree->LoadKdTree(BOX,hp,spacing,VOLMIN,FILLFACTOR);
	int TotalPoints=0;
	int TotalBoxes=0;
	pHypTree->DisplayTree(TotalPoints,TotalBoxes);
	pHypTree->CollectBoxesPoints(SBoxes,SPoints);
	delete pHypTree;
      }
    for(int SBcount=SBstart;SBcount<SBoxes.size();SBcount++)
      {
	for(int B : {1,3,5})
	  SBoxes[SBcount][B]--;
	Misc::times(SBoxes[SBcount],spacing);
      }
    _COUNTER++;
    if(VOLMIN > 1 || FILLFACTOR < 1.0)
      any_overlaps(mem,spacing,VOLMIN,FILLFACTOR,SBoxes,SPoints);
  }
}
