#-------------------------------------------------------------------------------
# EquationOfState abstract class
#-------------------------------------------------------------------------------
from PYB11Generator import *

@PYB11template("Dimension")
class EquationOfState:

    typedefs = """
    typedef typename %(Dimension)s::Scalar Scalar;
    typedef Field<%(Dimension)s, Scalar> ScalarField;
"""

    #...........................................................................
    # Constructors
    def pyinit(self,
               constants = "const PhysicalConstants&",
               minimumPressure = "const double",
               maximumPressure = "const double",
               minPressureType = "const MaterialPressureMinType"):
        "EOS base constructor"

    #...........................................................................
    # Abstract interface
    @PYB11pure_virtual
    @PYB11const
    def setPressure(self,
                    Pressure = "ScalarField&",
                    massDensity = "const ScalarField&",
                    specificThermalEnergy = "const ScalarField&"):
        return "void"

    @PYB11pure_virtual
    @PYB11const
    def setTemperature(self,
                       temperature = "ScalarField&",
                       massDensity = "const ScalarField&",
                       specificThermalEnergy = "const ScalarField&"):
        return "void"

    @PYB11pure_virtual
    @PYB11const
    def setSpecificThermalEnergy(self,
                                 specificThermalEnergy = "ScalarField&",
                                 massDensity = "const ScalarField&",
                                 temperature = "const ScalarField&"):
        return "void"

    @PYB11pure_virtual
    @PYB11const
    def setSpecificHeat(self,
                        specificHeat = "ScalarField&",
                        massDensity = "const ScalarField&",
                        temperature = "const ScalarField&"):
        return "void"

    @PYB11pure_virtual
    @PYB11const
    def setSoundSpeed(self,
                      soundSpeed = "ScalarField&",
                      massDensity = "const ScalarField&",
                      specificThermalEnergy = "const ScalarField&"):
        return "void"

    @PYB11pure_virtual
    @PYB11const
    def setGammaField(self,
                      gamma = "ScalarField&",
                      massDensity = "const ScalarField&",
                      specificThermalEnergy = "const ScalarField&"):
        return "void"

    @PYB11pure_virtual
    @PYB11const
    def setBulkModulus(self,
                       bulkModulus = "ScalarField&",
                       massDensity = "const ScalarField&",
                       specificThermalEnergy = "const ScalarField&"):
        return "void"

    @PYB11pure_virtual
    @PYB11const
    def setEntropy(self,
                   entropy = "ScalarField&",
                   massDensity = "const ScalarField&",
                   specificThermalEnergy = "const ScalarField&"):
        return "void"

    @PYB11pure_virtual
    @PYB11const
    def valid(self):
        return "bool"

    #...........................................................................
    # Virtual methods
    @PYB11virtual
    @PYB11const
    def specificThermalEnergyForPressure(self,
                                         Ptarget = "const Scalar",
                                         rho = "const Scalar",
                                         epsMin = "const Scalar",
                                         epsMax = "const Scalar",
                                         epsTol = "const Scalar",
                                         Ptol = "const Scalar",
                                         maxIterations = "const unsigned"):
        return "Scalar"

    #...........................................................................
    # Methods
    @PYB11const
    def applyPressureLimits(self, P="const Scalar"):
        "Return the limited pressure"
        return "Scalar"

    #...........................................................................
    # Properties
    constants = PYB11property("const PhysicalConstants&", "constants", doc="The units choice")
    minimumPressure = PYB11property("double", "minimumPressure", "minimumPressure", doc="The minimum allowed pressure")
    maximumPressure = PYB11property("double", "maximumPressure", "maximumPressure", doc="The maximum allowed pressure")
    minimumPressureType = PYB11property("MaterialPressureMinType", "minimumPressureType", "minimumPressureType", doc="The algorithm for enforcing the minimum pressure")
    
