#include "libs.hh"
#include "classes.hh"
#include "headers.hh"
namespace FractalSpace
{
  void high_pairs(Group& group)
  {
    //  cout << " entering high pairs " << endl;
    int num_high_points=group.get_number_high_points();
    group.list_high.resize(num_high_points);
    group.list_pair_1.reserve(3*num_high_points);
    group.list_pair_2.reserve(3*num_high_points);
    group.list_pair_1.clear();
    group.list_pair_2.clear();
    int n_h=0;
    for(vector <Point*>::const_iterator point_itr= group.list_points.begin();point_itr != group.list_points.end();++point_itr)
      {
	Point* p_point=*point_itr;
	if(p_point->get_it_is_high())
	  {
	    group.list_high[n_h]=p_point;
	    p_point->set_high_number(n_h);
	    n_h++;
	  }
	else
	  p_point->set_high_number(0);
      }
    group.set_number_high_points(n_h);
    int n_pairs=0;
    for(int n=0;n < num_high_points; ++n)
      {
	Point* p_point=group.list_high[n];
	assert(p_point);
	Point& point=*p_point;
	Point* p_up_x=point.get_point_up_x();
	if(p_up_x != 0 && p_up_x->get_it_is_high())
	  {
	    group.list_pair_1.push_back(n);
	    group.list_pair_2.push_back(p_up_x->get_high_number());
	    n_pairs++;
	  }
	Point* p_up_y=point.get_point_up_y();
	if(p_up_y != 0 && p_up_y->get_it_is_high())
	  {
	    group.list_pair_1.push_back(n);
	    group.list_pair_2.push_back(p_up_y->get_high_number());
	    n_pairs++;
	  }
	Point* p_up_z=point.get_point_up_z();
	if(p_up_z != 0 && p_up_z->get_it_is_high())
	  {
	    group.list_pair_1.push_back(n);
	    group.list_pair_2.push_back(p_up_z->get_high_number());
	    n_pairs++;
	  }
      }
    group.set_number_high_pairs(n_pairs);
  }
}
