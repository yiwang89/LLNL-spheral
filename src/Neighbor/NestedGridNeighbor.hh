//---------------------------------Spheral++----------------------------------//
// NestedGridNeighbor -- A reimplementation of the nested AMR like neighboring 
// method used in the original fortran version of Spheral.  Described in
// Owen, Villumsen, Shapiro, & Martel 1998, ApJS, 116, 155
//
// Created by JMO, Wed Dec 22 14:11:43 PST 1999
//----------------------------------------------------------------------------//
#ifndef __Spheral_NestedGridNeighbor_hh__
#define __Spheral_NestedGridNeighbor_hh__

namespace Spheral {
  template<typename Dimension> class GeomPlane;
  namespace NodeSpace {
    template<typename Dimension> class NodeList;
  }
}

#ifndef __GCCXML__
#include <vector>
#include "Utilities/SafeIndexMap.hh"
#else
#include "fakestl.hh"
#endif

#include "Neighbor.hh"
#include "GridCellIndex.hh"

namespace Spheral {
namespace NeighborSpace {

template<typename Dimension>
class NestedGridNeighbor: public Neighbor<Dimension> {

public:
  //--------------------------- Public Interface ---------------------------//
  typedef typename Dimension::Scalar Scalar;
  typedef typename Dimension::Vector Vector;
  typedef typename Dimension::Tensor Tensor;
  typedef typename Dimension::SymTensor SymTensor;

  typedef GeomPlane<Dimension> Plane;
  typedef GridCellIndex<Dimension> GC;

  // Constructors and destructors.
  NestedGridNeighbor(NodeSpace::NodeList<Dimension>& nodeList, 
                     const NeighborSearchType searchType,
                     const int numGridLevels, 
                     const double topGridCellSize,
                     const Vector origin,
                     const double kernelExtent,
		     const int gridCellInfluenceRadius);
  virtual ~NestedGridNeighbor();

  // Set or refine the neighbor lists for a given node ID.
  virtual void setMasterList(const int nodeID);
  virtual void setRefineNeighborList(const int nodeID);

  // Define the required methods for all Neighbor objects.
  virtual void setMasterList(const Vector& position,
                             const Scalar& H);
  virtual void setMasterList(const Vector& position,
                             const SymTensor& H);

  virtual void setMasterList(const Vector& position);
  virtual void setRefineNeighborList(const Vector& position);

  virtual void setRefineNeighborList(const Vector& position,
                                     const Scalar& H);
  virtual void setRefineNeighborList(const Vector& position,
                                     const SymTensor& H);

  virtual void setMasterList(const Plane& enterPlane,
                             const Plane& exitPlane);

  // Reassign the grid cell info for a given set of nodes.
  virtual void updateNodes();
  virtual void updateNodes(const std::vector<int>& nodeIDs);

  // Function to determine the appropriate gridlevel for a node.
  int gridLevel(const int nodeID) const;
  template<typename OtherHType> int gridLevel(const OtherHType& H) const;

  // Determine the gridcell which contains a given position on a given
  // gridlevel.
  GC gridCellIndex(const int nodeID,
                   const int gridLevel) const;
  GC gridCellIndex(const Vector& position,
                   const int gridLevel) const;

  // Find the corresponding grid cell range on a different grid level.
  void translateGridCellRange(const GC& gridCellMin,
                              const GC& gridCellMax,
                              const int gridLevel,
                              const int targetGridLevel,
                              GC& targetMin,
                              GC& targetMax) const;

  // The number of grid levels.
  int numGridLevels() const;
  void numGridLevels(const int numGridLevels);

//   // The first grid level with daughters.
//   int firstParentGridLevel() const;

  // Access the occupied grid levels.
  int numOccupiedGridLevels() const;
  std::vector<int> occupiedGridLevels() const;

//   // Is the given grid cell in the tree?
//   bool cellInTree(const GC& gridCell,
//                   const int gridLevel) const;

  // Is the given grid cell occupied?
  bool cellOccupied(const GC& gridCell,
                    const int gridLevel) const;

//   // Access the internal oct-tree structure.
//   const std::vector<GC>& daughterCells(const GC& gridCell,
//                                        const int gridLevel) const;

  // Access the occupied grid cells.
  const std::vector< std::vector<GC> >& occupiedGridCells() const;
  const std::vector<GC>& occupiedGridCells(const int gridLevelID) const;

  // Origin of the gridcell coordinates.
  Vector origin() const;
  void origin(const Vector& origin);

  // The top level grid size.
  double topGridSize() const;
  void topGridSize(const double gridSize);

  // The radius in gridcells a node can touch on it's home grid level.
  int gridCellInfluenceRadius() const;
  void gridCellInfluenceRadius(const int x);

  // Find the head node for the link list of a grid cell (if it exists).
  int headOfGridCell(const GC& gridCell,
                     const int gridLevel) const;

  // Find the next node in the linked list for a cell.
  int nextNodeInCell(const int nodeID) const;

  // Return the set of internal nodes in the given cell.
  std::vector<int> internalNodesInCell(const GC& gridCell,
                                       const int gridLevel) const;

  // Return the set of nodes in the given cell.
  std::vector<int> nodesInCell(const GC& gridCell,
                               const int gridLevel) const;

  // Read out the nodes in the given cell and append them to the 
  // given list.
  void appendNodesInCell(const GC& gridCell,
                         const int gridLevel,
                         std::vector<int>& nodes) const;

  // Return the list of occupied grid cells in the given range.
  void occupiedGridCellsInRange(std::vector<GC>& gridCells,
                                const GC& minGridCell,
                                const GC& maxGridCell,
                                const int gridLevelID) const;

  // Convert the normal of a plane to an equivalent integer GridCellIndex
  // representation.
  GC gridNormal(const Vector& normal) const;

  // Map a grid cell through an enter/exit plane combo.
  GC mapGridCell(const GC& gridCell,
                 const int gridLevel,
                 const Plane& enterPlane,
                 const Plane& exitPlane) const;

  // Allow read only access to some of the more interesting member data.
  const std::vector<double>& gridCellSizeInv() const;
  const std::vector< std::vector<GC > >& nodeInCell() const;
  int masterGridLevel() const;
  const GC& masterGridCellIndex() const;

  // The flag indicating the end of a linked list of nodes.
  int endOfLinkList() const;

  // Determine if the NeighborList is in a valid, ready to use state.
  virtual bool valid() const;

  // Allow outside users to be able to directly set master info for a given
  // grid cell and grid level.
  void setNestedMasterList(const GC& gridCell,
                           const int gridLevel);

  // Find the nodes that affect the given grid cell.
  std::vector<int> findNestedNeighbors(const GC& gridCell,
                                       const int gridLevel) const;
private:
  //--------------------------- Private Interface ---------------------------//
  // Private functions.
  bool readyToAssignNodes() const;
  void rebuildOccupiedGridCells();
//   void rebuildOctTree();
  void linkNode(const int nodeID,
                const int gridLevelID,
                const GC& gridCellID);
  void unlinkNode(const int nodeID,
                  const int gridLevelID,
                  const GC& gridCellID);
  int determineGridLevel(const double& h) const;

  // Private templated functions to do the work of setMasterList and 
  // setRefineList above.
  template<typename OtherHType>
  void setNestedMasterList(const Vector& position,
                           const OtherHType& H);

  template<typename OtherHType>
  void setNestedRefineNeighborList(const Vector& position,
                                   const OtherHType& H);

  // Private data.
  int mMaxGridLevels, mFirstParentGridLevel, mGridCellInfluenceRadius;
  Vector mGridOrigin;
  std::vector<int> mGridLevelOccupied;

  double mGridLevelConst0;
  std::vector<double> mGridCellSizeInv;
  std::vector< SafeIndexMap<GC, int> > mGridCellHead;

  std::vector< std::vector< GC > > mNodeInCell;
  std::vector<int> mNextNodeInCell;
  std::vector<int> mNodeOnGridLevel;

  int mMasterGridLevel;
  GC mMasterGridCellIndex;

  // The oct-tree structure of daughter cells.
//   typedef SafeIndexMap<GC, std::vector<GC> > OctTree;
//   std::vector<OctTree> mDaughterCells;
//   std::vector<GC> mEmptyNest;

  // The set of grid cells that are actually occupied.
  std::vector< std::vector<GC> > mOccupiedGridCells;

  static const double ln2inverse;
  static const int mEndOfLinkList;
  static const int mGridNormalMagnitude;

};

}
}

#ifndef __GCCXML__
#include "NestedGridNeighborInline.hh"
#endif

#else

// Forward declaration.
namespace Spheral {
  namespace NeighborSpace {
    template<typename Dimension> class NestedGridNeighbor;
  }
}

#endif
