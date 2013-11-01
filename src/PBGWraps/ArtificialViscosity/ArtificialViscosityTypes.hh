#ifndef __PBGWRAPS_ARTIFICIALVISCOSITYTYPES__
#define __PBGWRAPS_ARTIFICIALVISCOSITYTYPES__

#include "Geometry/Dimension.hh"
#include "ArtificialViscosity/ArtificialViscosity.hh"
#include "ArtificialViscosity/MonaghanGingoldViscosity.hh"
#include "ArtificialViscosity/TensorMonaghanGingoldViscosity.hh"
#include "ArtificialViscosity/FiniteVolumeViscosity.hh"
#include "ArtificialViscosity/TensorSVPHViscosity.hh"

namespace Spheral {
namespace ArtificialViscositySpace {

//------------------------------------------------------------------------------
// Names!
//------------------------------------------------------------------------------
typedef ArtificialViscosity<Dim<1> > ArtificialViscosity1d;
typedef ArtificialViscosity<Dim<2> > ArtificialViscosity2d;
typedef ArtificialViscosity<Dim<3> > ArtificialViscosity3d;

typedef MonaghanGingoldViscosity<Dim<1> > MonaghanGingoldViscosity1d;
typedef MonaghanGingoldViscosity<Dim<2> > MonaghanGingoldViscosity2d;
typedef MonaghanGingoldViscosity<Dim<3> > MonaghanGingoldViscosity3d;

typedef TensorMonaghanGingoldViscosity<Dim<1> > TensorMonaghanGingoldViscosity1d;
typedef TensorMonaghanGingoldViscosity<Dim<2> > TensorMonaghanGingoldViscosity2d;
typedef TensorMonaghanGingoldViscosity<Dim<3> > TensorMonaghanGingoldViscosity3d;

typedef FiniteVolumeViscosity<Dim<1> > FiniteVolumeViscosity1d;
typedef FiniteVolumeViscosity<Dim<2> > FiniteVolumeViscosity2d;
typedef FiniteVolumeViscosity<Dim<3> > FiniteVolumeViscosity3d;

typedef TensorSVPHViscosity<Dim<1> > TensorSVPHViscosity1d;
typedef TensorSVPHViscosity<Dim<2> > TensorSVPHViscosity2d;
typedef TensorSVPHViscosity<Dim<3> > TensorSVPHViscosity3d;

}
}

#endif
