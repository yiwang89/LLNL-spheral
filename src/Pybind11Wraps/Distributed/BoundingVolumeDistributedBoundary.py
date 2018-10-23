#-------------------------------------------------------------------------------
# BoundingVolumeDistributedBoundary
#-------------------------------------------------------------------------------
from PYB11Generator import *

from DistributedBoundary import *

@PYB11template("Dimension")
@PYB11singleton
class BoundingVolumeDistributedBoundary(DistributedBoundary):

    typedefs = """
    typedef typename %(Dimension)s::Scalar Scalar;
    typedef typename %(Dimension)s::Vector Vector;
    typedef typename %(Dimension)s::Tensor Tensor;
    typedef typename %(Dimension)s::SymTensor SymTensor;
    typedef typename %(Dimension)s::ThirdRankTensor ThirdRankTensor;

    typedef typename DistributedBoundary<%(Dimension)s>::DomainBoundaryNodes DomainBoundaryNodes;
    typedef std::map<int, DomainBoundaryNodes> DomainBoundaryNodeMap;
    typedef std::map<NodeList<%(Dimension)s>*, DomainBoundaryNodeMap> NodeListDomainBoundaryNodeMap;
"""

    # The instance attribute.  We expose this as a property of the class.
    @PYB11static
    @PYB11cppname("instancePtr")
    @PYB11returnpolicy("take_ownership")
    def instance(self):
        return "RestartRegistrar*"

    #...........................................................................
    # Virtual methods
    @PYB11pure_virtual
    def setAllGhostNodes(self,
                         dataBase = "DataBase<%(Dimension)s>&"):
        "Descendent Distributed Neighbors are required to provide the setGhostNodes method for DataBases."
        return "void"
