#-------------------------------------------------------------------------------
# ConstantVelocityBoundary
#-------------------------------------------------------------------------------
from PYB11Generator import *
from Boundary import *
from PlanarBoundary import *
from BoundaryAbstractMethods import *
from RestartMethods import *

@PYB11template("Dimension")
class ConstantVelocityBoundary(Boundary):
    """ConstantVelocityBoundary -- A boundary condition to enforce a constant 
velocity on a given set of nodes.

This boundary is very specialized -- it explicitly works on only one 
NodeList.
"""

    typedefs = """
    typedef typename %(Dimension)s::Scalar Scalar;
    typedef typename %(Dimension)s::Vector Vector;
    typedef typename %(Dimension)s::Tensor Tensor;
    typedef typename %(Dimension)s::SymTensor SymTensor;
    typedef typename %(Dimension)s::ThirdRankTensor ThirdRankTensor;
    typedef GeomPlane<%(Dimension)s> Plane;
"""

    #...........................................................................
    # Constructors
    def pyinit(self,
               nodeList = "const NodeList<%(Dimension)s>&",
               nodeIndices = "const std::vector<int>&"):
        "Construct a periodic boundary mapping between the two (enter/exit) planes"

    #...........................................................................
    # Methods
    @PYB11virtual
    @PYB11const
    def valid(self):
        return "bool"

    #...........................................................................
    # Properties
    nodeList = PYB11property("const NodeList<%(Dimension)s>&", "nodeList", doc="The NodeList this boundary applies to")
    nodeIndices = PYB11property("std::vector<int>", "nodeIndices", doc="The nodes this boundary is in control of")
    velocityCondition = PYB11property("std::vector<Vector>", "velocityCondition", doc="The velocities for the nodes we control")

#-------------------------------------------------------------------------------
# Inject methods
#-------------------------------------------------------------------------------
PYB11inject(BoundaryAbstractMethods, ConstantVelocityBoundary, virtual=True, pure_virtual=False)
PYB11inject(RestartMethods, ConstantVelocityBoundary)
