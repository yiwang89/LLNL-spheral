#ifndef __PBGWRAPS_ANEOSTYPES__
#define __PBGWRAPS_ANEOSTYPES__

#include <string>
#include <vector>

#include "Geometry/Dimension.hh"
#include "Material/EquationOfState.hh"
#include "SolidMaterial/ANEOSEquationOfState.hh"

// Fortran baby!
extern "C" {
  void aneos_initialize_(char* in_filename, char* out_filename, int* num, int* izetl);
}

//------------------------------------------------------------------------------
// Names!
//------------------------------------------------------------------------------
namespace Spheral {

namespace Material {
typedef EquationOfState<Dim<1> > EquationOfState1d;
typedef EquationOfState<Dim<2> > EquationOfState2d;
typedef EquationOfState<Dim<3> > EquationOfState3d;
}

namespace SolidMaterial {
typedef ANEOSEquationOfState<Dim<1> > ANEOSEquationOfState1d;
typedef ANEOSEquationOfState<Dim<2> > ANEOSEquationOfState2d;
typedef ANEOSEquationOfState<Dim<3> > ANEOSEquationOfState3d;

//------------------------------------------------------------------------------
// Wrap the ANEOS initialize function in something a bit more helpful.
//------------------------------------------------------------------------------
inline
void initializeANEOS(std::string in_filename, std::string out_filename, std::vector<int> izetl) {
  int numMaterials = izetl.size();
  VERIFY2(numMaterials <= 21,
          "ANEOS initialize error : can only specify at most 21 materials.");
  VERIFY2(in_filename.size() <= 256,
          "ANEOS intialize error : input file name cannot exceed 256 characters.");
  VERIFY2(out_filename.size() <= 256,
          "ANEOS intialize error : output file name cannot exceed 256 characters.");
  char in[256], out[256];
  strcpy(in, in_filename.c_str());
  std::fill(in + in_filename.size(), in + 256, ' ');
  strcpy(out, out_filename.c_str());
  std::fill(out + out_filename.size(), out + 256, ' ');
  izetl.resize(21, 0);
  aneos_initialize_(in, out, &numMaterials, &izetl.front());
}

}

}

#endif
