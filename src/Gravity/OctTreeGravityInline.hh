//---------------------------------Spheral++----------------------------------//
// OctTreeGravity -- An implementation of the tree n-body gravity solver.
//
// Created by JMO, 2012-02-28
//----------------------------------------------------------------------------//

namespace Spheral {
namespace GravitySpace {

//------------------------------------------------------------------------------
// Build a cell key from coordinate indices.
//------------------------------------------------------------------------------
//inline
void
OctTreeGravity::
buildCellKey(const OctTreeGravity::LevelKey ilevel,
             const OctTreeGravity::Vector& xi,
             OctTreeGravity::CellKey& key,
             OctTreeGravity::CellKey& ix,
             OctTreeGravity::CellKey& iy,
             OctTreeGravity::CellKey& iz) const {
  REQUIRE(xi.x() >= mXmin.x() and xi.x() <= mXmax.x());
  REQUIRE(xi.y() >= mXmin.y() and xi.y() <= mXmax.y());
  REQUIRE(xi.z() >= mXmin.z() and xi.z() <= mXmax.z());
  const CellKey ncell = (1U << ilevel);
  const CellKey maxcell = ncell - 1U;
  ix = std::min(maxcell, CellKey((xi.x() - mXmin.x())/mBoxLength * ncell));
  iy = std::min(maxcell, CellKey((xi.y() - mXmin.y())/mBoxLength * ncell));
  iz = std::min(maxcell, CellKey((xi.z() - mXmin.z())/mBoxLength * ncell));
  key = ((std::max(CellKey(0), std::min(max1dKey, iz)) << 2*num1dbits) +
         (std::max(CellKey(0), std::min(max1dKey, iy)) <<   num1dbits) +
         (std::max(CellKey(0), std::min(max1dKey, ix))));
}

//------------------------------------------------------------------------------
// Extract the individual coordinate indices from a cell index.
//------------------------------------------------------------------------------
//inline
void
OctTreeGravity::
extractCellIndices(const OctTreeGravity::CellKey& key,
                   OctTreeGravity::CellKey& ix,
                   OctTreeGravity::CellKey& iy,
                   OctTreeGravity::CellKey& iz) const {
  ix = key & xkeymask;
  iy = (key & ykeymask) >> num1dbits;
  iz = (key & zkeymask) >> 2*num1dbits;
}

//------------------------------------------------------------------------------
// Add a daughter to a cell if not present.
//------------------------------------------------------------------------------
//inline
void
OctTreeGravity::
addDaughter(OctTreeGravity::Cell& cell,
            const OctTreeGravity::TreeKey& daughterKey) const {
  if (std::find(cell.daughters.begin(), cell.daughters.end(), daughterKey) == cell.daughters.end())
    cell.daughters.push_back(daughterKey);
  ENSURE(cell.daughters.size() <= 8);
}

//------------------------------------------------------------------------------
// Add a node to the internal Tree structure.
//------------------------------------------------------------------------------
//inline
void
OctTreeGravity::
addNodeToTree(OctTreeGravity::MapTree& tree,
              const double mi,
              const OctTreeGravity::Vector& xi) {
  LevelKey ilevel = 0;
  bool terminated = false;
  CellKey cellKey, otherCellKey, ix, iy, iz;;
  TreeKey key, parentKey, otherKey;
  MapTree::iterator itr;
  while (ilevel < OctTreeGravity::num1dbits and not terminated) {

    // Create the key for the cell containing this particle on this level.
    buildCellKey(ilevel, xi, cellKey, ix, iy, iz);
    key = make_pair(ilevel, cellKey);
    itr = tree.find(key);

    if (itr == tree.end()) {
      // If this is an unregistered cell, add it with this node as the sole leaf
      // and we're done.
      terminated = true;
      tree[key] = Cell(mi, xi);

    } else {
      Cell& cell = itr->second;

      // Is this cell a single leaf?
      if (cell.masses.size() > 0) {
        CHECK(cell.masses.size() == cell.positions.size());
        CHECK(cell.daughters.size() == 0);

        // Yep, so we need to split it unless we're at the maximum refinement.
        if (ilevel < OctTreeGravity::num1dbits - 1) {
          CHECK(cell.masses.size() == 1);
          const LevelKey ilevel1 = ilevel + 1;
          CHECK(ilevel1 < OctTreeGravity::num1dbits);
          buildCellKey(ilevel1, cell.xcm, otherCellKey, ix, iy, iz);
          otherKey = make_pair(ilevel1, otherCellKey);
          tree[otherKey] = Cell(cell.M, cell.xcm);
          cell.daughters = std::vector<TreeKey>(1, otherKey);
          cell.masses = std::vector<double>();
          cell.positions = std::vector<Vector>();

        } else {
          // If we've maxed out the levels, then we just huck this node in 
          // the members of this cell.
          cell.masses.push_back(mi);
          cell.positions.push_back(xi);
        }
      }

      // Increment the cell moments.
      cell.xcm = (cell.M*cell.xcm + mi*xi)/(cell.M + mi);
      cell.M += mi;
      cell.Mglobal = cell.M;
    }

    // Link this cell as a daughter of its parent.
    if (ilevel > 0) {
      CHECK(tree.find(parentKey) != tree.end());
      addDaughter(tree[parentKey], key);
    }

    parentKey = key;
    ++ilevel;
  }

}

}
}
