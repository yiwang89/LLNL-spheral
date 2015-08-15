from SpheralModules.Spheral.CRKSPHSpace import *
from SpheralModules.Spheral.NodeSpace import *
from SpheralModules.Spheral.PhysicsSpace import *
from SpheralModules.Spheral.PhysicsSpace import *

#-------------------------------------------------------------------------------
# The generic SolidCRKSPHHydro pattern.
#-------------------------------------------------------------------------------
SolidCRKSPHHydroFactoryString = """
class %(classname)s%(dim)s(SolidCRKSPHHydroBase%(dim)s):

    def __init__(self,
                 W,
                 WPi,
                 Q,
                 filter = 0.0,
                 cfl = 0.25,
                 useVelocityMagnitudeForDt = False,
                 compatibleEnergyEvolution = True,
                 XSPH = True,
                 densityUpdate = RigorousSumDensity,
                 HUpdate = IdealH,
                 epsTensile = 0.0,
                 nTensile = 4.0):
        self._smoothingScaleMethod = %(smoothingScaleMethod)s%(dim)s()
        SolidCRKSPHHydroBase%(dim)s.__init__(self,
                                             self._smoothingScaleMethod,
                                             W,
                                             WPi,
                                             Q,
                                             filter,
                                             cfl,
                                             useVelocityMagnitudeForDt,
                                             compatibleEnergyEvolution,
                                             XSPH,
                                             densityUpdate,
                                             HUpdate,
                                             epsTensile,
                                             nTensile)
        return
"""

#-------------------------------------------------------------------------------
# SolidCRKSPH
#-------------------------------------------------------------------------------
exec(SolidCRKSPHHydroFactoryString % {"dim"                  : "1d",
                                   "classname"            : "SolidCRKSPHHydro",
                                   "smoothingScaleMethod" : "SPHSmoothingScale"})
exec(SolidCRKSPHHydroFactoryString % {"dim"                  : "2d",
                                   "classname"            : "SolidCRKSPHHydro",
                                   "smoothingScaleMethod" : "SPHSmoothingScale"})
exec(SolidCRKSPHHydroFactoryString % {"dim"                  : "3d",
                                   "classname"            : "SolidCRKSPHHydro",
                                   "smoothingScaleMethod" : "SPHSmoothingScale"})

#-------------------------------------------------------------------------------
# SolidASPH
#-------------------------------------------------------------------------------
exec(SolidCRKSPHHydroFactoryString % {"dim"                  : "1d",
                                   "classname"            : "SolidACRKSPHHydro",
                                   "smoothingScaleMethod" : "ASPHSmoothingScale"})
exec(SolidCRKSPHHydroFactoryString % {"dim"                  : "2d",
                                   "classname"            : "SolidACRKSPHHydro",
                                   "smoothingScaleMethod" : "ASPHSmoothingScale"})
exec(SolidCRKSPHHydroFactoryString % {"dim"                  : "3d",
                                   "classname"            : "SolidACRKSPHHydro",
                                   "smoothingScaleMethod" : "ASPHSmoothingScale"})