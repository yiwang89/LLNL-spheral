//---------------------------------Spheral++----------------------------------//
// ConnectivityMap
//
// Stores the full set of significant neighbors for a set of NodeLists.
//
// Created by J. Michael Owen, Sun Oct 30 15:36:33 PST 2005
//----------------------------------------------------------------------------//
#include "ConnectivityMap.hh"
#include "NodeList/NodeList.hh"
#include "Neighbor/Neighbor.hh"
#include "DataBase/DataBase.hh"
#include "Field/FieldList.hh"
#include "Boundary/Boundary.hh"
#include "Utilities/globalNodeIDs.hh"
#include "Utilities/timingUtilities.hh"
#include "Utilities/mortonOrderIndices.hh"
#include "Utilities/PairComparisons.hh"
#include "Utilities/Timer.hh"

#include <algorithm>
#include <ctime>
using std::vector;
using std::map;
using std::string;
using std::pair;
using std::cout;
using std::cerr;
using std::endl;
using std::min;
using std::max;
using std::abs;

// Declare the timers.
extern Timer TIME_ConnectivityMap_patch;
extern Timer TIME_ConnectivityMap_cutConnectivity;
extern Timer TIME_ConnectivityMap_valid;
extern Timer TIME_ConnectivityMap_computeConnectivity;
extern Timer TIME_ConnectivityMap_computeOverlapConnectivity;

namespace Spheral {

namespace {
//------------------------------------------------------------------------------
// Append v2 to the end of v1
//------------------------------------------------------------------------------
template<typename T>
inline
void
appendSTLvectors(std::vector<T>& v1, std::vector<T>& v2) {
  v1.reserve(v1.size() + v2.size());
  v1.insert(v1.end(), v2.begin(), v2.end());
}

//------------------------------------------------------------------------------
// Helper to insert into a sorted list of IDs.
//------------------------------------------------------------------------------
inline
bool
insertUnique(const std::vector<int>& offsets,
             std::vector<std::vector<std::vector<int>>>& indices,
             const int jN1, const int j1,
             const int jN2, const int j2) {
  if (jN1 != jN2 or j1 != j2) {
    auto& overlap = indices[offsets[jN1] + j1][jN2];
    auto itr = std::lower_bound(overlap.begin(), overlap.end(), j2);
    if (itr == overlap.end() or *itr != j2) {
      overlap.insert(itr, j2);
      return true;
    } else {
      return false;
    }
  }
  return false;
}

}

//------------------------------------------------------------------------------
// Constructor.
//------------------------------------------------------------------------------
template<typename Dimension>
inline
ConnectivityMap<Dimension>::
ConnectivityMap():
  mNodeLists(),
  mBuildGhostConnectivity(false),
  mBuildOverlapConnectivity(false),
  mConnectivity(),
  mNodeTraversalIndices(),
  mKeys(FieldStorageType::CopyFields) {
}

//------------------------------------------------------------------------------
// Destructor.
//------------------------------------------------------------------------------
template<typename Dimension>
ConnectivityMap<Dimension>::
~ConnectivityMap() {
}

//------------------------------------------------------------------------------
// Internal method to build the connectivity for the requested set of NodeLists.
//------------------------------------------------------------------------------
template<typename Dimension>
void
ConnectivityMap<Dimension>::
patchConnectivity(const FieldList<Dimension, int>& flags,
                  const FieldList<Dimension, int>& old2new) {
  TIME_ConnectivityMap_patch.start();

  const auto domainDecompIndependent = NodeListRegistrar<Dimension>::instance().domainDecompositionIndependent();

  // We have to recompute the keys to sort nodes by excluding the 
  // nodes that are being removed.
  const auto numNodeLists = mNodeLists.size();
  if (domainDecompIndependent) {
// #pragma omp parallel for collapse(2)
    for (auto iNodeList = 0; iNodeList < numNodeLists; ++iNodeList) {
      for (auto i = 0; i < mNodeLists[iNodeList]->numNodes(); ++i) {
        if (flags(iNodeList, i) == 0) mKeys(iNodeList, i) = KeyTraits::maxKey;
      }
    }
  }

  // Iterate over the Connectivity (NodeList).
  for (auto iNodeList = 0; iNodeList != numNodeLists; ++iNodeList) {
    const auto ioff = mOffsets[iNodeList];
    const auto numNodes = ((domainDecompIndependent or mBuildGhostConnectivity) ? 
                           mNodeLists[iNodeList]->numNodes() :
                           mNodeLists[iNodeList]->numInternalNodes());

    vector<size_t> iNodesToKill;
    vector<pair<int, Key>> keys;
#pragma omp parallel
    {
      vector<size_t> iNodesToKill_thread;
      vector<pair<int, Key>> keys_thread;

      // Patch the traversal ordering and connectivity for this NodeList.
#pragma omp for schedule(dynamic)
      for (auto i = 0; i < numNodes; ++i) {

        // Should we patch this set of neighbors?
        if (flags(iNodeList, i) == 0) {
          iNodesToKill_thread.push_back(i);
        } else {
          if (domainDecompIndependent) keys_thread.push_back(std::make_pair(old2new(iNodeList, i), mKeys(iNodeList, i)));
          mNodeTraversalIndices[iNodeList][i] = old2new(iNodeList, i);
          auto& neighbors = mConnectivity[ioff + i];
          CHECK(neighbors.size() == numNodeLists);
          for (auto jNodeList = 0; jNodeList < numNodeLists; ++jNodeList) {
            vector<pair<int, Key>> nkeys;
            vector<size_t> jNodesToKill;
            for (auto k = 0; k < neighbors[jNodeList].size(); ++k) {
              const auto j = neighbors[jNodeList][k];
              if (flags(jNodeList, j) == 0) {
                jNodesToKill.push_back(k);
              } else {
                if (domainDecompIndependent) nkeys.push_back(std::make_pair(old2new(jNodeList, j), mKeys(jNodeList, j)));
                neighbors[jNodeList][k] = old2new(jNodeList, j);
              }
            }
            removeElements(neighbors[jNodeList], jNodesToKill);

            // Recompute the ordering of the neighbors.
            if (domainDecompIndependent) {
              sort(nkeys.begin(), nkeys.end(), ComparePairsBySecondElement<pair<int, Key> >());
              for (size_t k = 0; k != neighbors[jNodeList].size(); ++k) {
                CHECK2(k == 0 or nkeys[k].second > nkeys[k-1].second,
                       "Incorrect neighbor ordering:  "
                       << i << " "
                       << k << " "
                       << nkeys[k-1].second << " "
                       << nkeys[k].second);
                neighbors[jNodeList][k] = nkeys[k].first;
              }
            } else {
              sort(neighbors[jNodeList].begin(), neighbors[jNodeList].end());
            }
          }
        }
      }

#pragma omp critical
      appendSTLvectors(iNodesToKill, iNodesToKill_thread);
      appendSTLvectors(keys, keys_thread);
    }
    removeElements(mNodeTraversalIndices[iNodeList], iNodesToKill);

    // Recompute the ordering for traversing the nodes.
    {
      const auto numNodes = mNodeTraversalIndices[iNodeList].size();
      if (domainDecompIndependent) {
        // keys = vector<pair<int, Key> >();
        // for (size_t k = 0; k != numNodes; ++k) {
        //   const int i = mNodeTraversalIndices[iNodeList][k];
        //   keys.push_back(std::make_pair(i, mKeys(iNodeList, i)));
        // }
        sort(keys.begin(), keys.end(), ComparePairsBySecondElement<pair<int, Key> >());
#pragma omp parallel for
        for (auto k = 0; k < numNodes; ++k) {
          mNodeTraversalIndices[iNodeList][k] = keys[k].first;
        }
      } else {
#pragma omp parallel for
        for (auto i = 0; i < numNodes; ++i) {
          mNodeTraversalIndices[iNodeList][i] = i;
        }
      }
    }
  }
  
  // We also need to patch the node pair structure
  NodePairList culledPairs;
#pragma omp parallel
  {
    NodePairList culledPairs_thread;
    const auto npairs = mNodePairList.size();
#pragma omp for
    for (auto k = 0; k < npairs; ++k) {
      const auto iNodeList = mNodePairList[k].i_list;
      const auto jNodeList = mNodePairList[k].j_list;
      const auto i = mNodePairList[k].i_node;
      const auto j = mNodePairList[k].j_node;
      if (flags(iNodeList, i) != 0 and flags(jNodeList, j) != 0) {
        culledPairs_thread.push_back(NodePairIdxType(old2new(iNodeList, i), iNodeList,
                                                     old2new(jNodeList, j), jNodeList));
      }
    }
#pragma omp critical
    {
      culledPairs.insert(culledPairs.end(), culledPairs_thread.begin(), culledPairs_thread.end());
    }
  }
  mNodePairList = culledPairs;

  // You can't check valid yet 'cause the NodeLists have not been resized
  // when we call patch!  The valid method should be checked by whoever called
  // this method after that point.
  //ENSURE(valid());
  TIME_ConnectivityMap_patch.stop();
}

//------------------------------------------------------------------------------
// Compute the common neighbors for a pair of nodes.  Note this method 
// returns by value since this information is not stored by ConnectivityMap.
//------------------------------------------------------------------------------
template<typename Dimension>
vector<vector<int> >
ConnectivityMap<Dimension>::
connectivityIntersectionForNodes(const int nodeListi, const int i,
                                 const int nodeListj, const int j) const {

  // Pre-conditions.
  const auto numNodeLists = mNodeLists.size();
  REQUIRE(nodeListi < numNodeLists and
          nodeListj < numNodeLists);
  const auto firstGhostNodei = mNodeLists[nodeListi]->firstGhostNode();
  const auto firstGhostNodej = mNodeLists[nodeListj]->firstGhostNode();
  REQUIRE(i < firstGhostNodei or j < firstGhostNodej);

  // Prepare the result.
  vector<vector<int>> result(numNodeLists);

  // If both nodes are internal, we simply intersect their neighbor lists.
  if (i < firstGhostNodei and j < firstGhostNodej) {
    vector<vector<int>> neighborsi = this->connectivityForNode(nodeListi, i);
    vector<vector<int>> neighborsj = this->connectivityForNode(nodeListj, j);
    CHECK(neighborsi.size() == numNodeLists);
    CHECK(neighborsj.size() == numNodeLists);
    neighborsi[nodeListi].push_back(i);
    neighborsj[nodeListj].push_back(j);
    for (unsigned k = 0; k != numNodeLists; ++k) {
      sort(neighborsi[k].begin(), neighborsi[k].end());
      sort(neighborsj[k].begin(), neighborsj[k].end());
      set_intersection(neighborsi[k].begin(), neighborsi[k].end(),
                       neighborsj[k].begin(), neighborsj[k].end(),
                       back_inserter(result[k]));
    }
  } else if (i < firstGhostNodei) {
    result = this->connectivityForNode(nodeListi, i);
  } else {
    result = this->connectivityForNode(nodeListj, j);
  }

  // That's it.
  return result;
}

//------------------------------------------------------------------------------
// Remove connectivity between neighbors.
// NOTE: this method assumes you are passing the indices of the neighbors to
//       remove!
//------------------------------------------------------------------------------
template<typename Dimension>
void
ConnectivityMap<Dimension>::
removeConnectivity(const FieldList<Dimension, vector<vector<int>>>& neighborsToCut) {
  TIME_ConnectivityMap_cutConnectivity.start();

  const auto numNodeLists = mNodeLists.size();
  REQUIRE(neighborsToCut.numFields() == numNodeLists);

  for (auto nodeListi = 0; nodeListi < numNodeLists; ++nodeListi) {
    const auto n = mNodeLists[nodeListi]->numNodes();
    for (auto i = 0; i < n; ++i) {
      const auto& allneighbors = neighborsToCut(nodeListi, i);
      CHECK(allneighbors.size() == 0 or allneighbors.size() == numNodeLists);
      for (auto nodeListj = 0; nodeListj < allneighbors.size(); ++nodeListj) {
        auto& neighborsi = mConnectivity[mOffsets[nodeListi] + i][nodeListj];
        removeElements(neighborsi, allneighbors[nodeListj]);
      }
    }
  }

  TIME_ConnectivityMap_cutConnectivity.stop();
}

//------------------------------------------------------------------------------
// Compute the union of neighbors for a pair of nodes.  Note this method 
// returns by value since this information is not stored by ConnectivityMap.
//------------------------------------------------------------------------------
template<typename Dimension>
vector<vector<int> >
ConnectivityMap<Dimension>::
connectivityUnionForNodes(const int nodeListi, const int i,
                          const int nodeListj, const int j) const {

  typedef typename Dimension::Scalar Scalar;
  typedef typename Dimension::Vector Vector;
  typedef typename Dimension::Tensor Tensor;
  typedef typename Dimension::SymTensor SymTensor;

  // Pre-conditions.
  const unsigned numNodeLists = mNodeLists.size();
  REQUIRE(nodeListi < numNodeLists and
          nodeListj < numNodeLists);
  const unsigned firstGhostNodei = mNodeLists[nodeListi]->firstGhostNode();
  const unsigned firstGhostNodej = mNodeLists[nodeListj]->firstGhostNode();
  REQUIRE(i < firstGhostNodei or j < firstGhostNodej);

  // Do the deed.
  vector<vector<int> > result(numNodeLists);
  vector<vector<int> > neighborsi = this->connectivityForNode(nodeListi, i);
  vector<vector<int> > neighborsj = this->connectivityForNode(nodeListj, j);
  CHECK(neighborsi.size() == numNodeLists);
  CHECK(neighborsj.size() == numNodeLists);
  for (unsigned k = 0; k != numNodeLists; ++k) {
    sort(neighborsi[k].begin(), neighborsi[k].end());
    sort(neighborsj[k].begin(), neighborsj[k].end());
    set_union(neighborsi[k].begin(), neighborsi[k].end(),
              neighborsj[k].begin(), neighborsj[k].end(),
              back_inserter(result[k]));
  }

  // That's it.
  return result;
}

//------------------------------------------------------------------------------
// Return the connectivity in terms of global node IDs.
//------------------------------------------------------------------------------
template<typename Dimension>
map<int, vector<int> > 
ConnectivityMap<Dimension>::
globalConnectivity(vector<Boundary<Dimension>*>& boundaries) const {

  // Get the set of global node IDs.
  FieldList<Dimension, int> globalIDs = globalNodeIDs<Dimension, typename vector<const NodeList<Dimension>*>::const_iterator>
    (mNodeLists.begin(), mNodeLists.end());

  // Make sure all ghost nodes have the appropriate global IDs.
  for (typename vector<Boundary<Dimension>*>::iterator boundItr = boundaries.begin();
       boundItr != boundaries.end();
       ++boundItr) (*boundItr)->applyFieldListGhostBoundary(globalIDs);
  for (typename vector<Boundary<Dimension>*>::iterator boundItr = boundaries.begin();
       boundItr != boundaries.end();
       ++boundItr) (*boundItr)->finalizeGhostBoundary();

  // Now convert our connectivity to global IDs.
  map<int, vector<int> > result;
  const size_t numNodeLists = mNodeLists.size();
  for (size_t nodeListi = 0; nodeListi != numNodeLists; ++nodeListi) {

    const NodeList<Dimension>* nodeListPtr = mNodeLists[nodeListi];
    for (int i = 0; i != nodeListPtr->numInternalNodes(); ++i) {
      const int gid = globalIDs(nodeListi, i);
      result[gid] = vector<int>();

      const vector< vector<int> >& fullConnectivity = connectivityForNode(nodeListPtr, i);
      CHECK(fullConnectivity.size() == numNodeLists);
      for (size_t nodeListj = 0; nodeListj != numNodeLists; ++nodeListj) {
        const vector<int>& connectivity = fullConnectivity[nodeListj];

        for (typename vector<int>::const_iterator jItr = connectivity.begin();
             jItr != connectivity.end();
             ++jItr) result[gid].push_back(globalIDs(nodeListj, *jItr));

      }
      ENSURE(result[gid].size() == numNeighborsForNode(nodeListPtr, i));
    }
  }

  // That's it.
  return result;
}

//------------------------------------------------------------------------------
// Compute the index for the given NodeList in our known set.
//------------------------------------------------------------------------------
template<typename Dimension>
unsigned
ConnectivityMap<Dimension>::
nodeListIndex(const NodeList<Dimension>* nodeListPtr) const {
  return distance(mNodeLists.begin(), 
                  find(mNodeLists.begin(), mNodeLists.end(), nodeListPtr));
}

//------------------------------------------------------------------------------
// Valid test.
//------------------------------------------------------------------------------
template<typename Dimension>
bool
ConnectivityMap<Dimension>::
valid() const {
  TIME_ConnectivityMap_valid.start();

  const bool domainDecompIndependent = NodeListRegistrar<Dimension>::instance().domainDecompositionIndependent();

  // Check the offsets.
  const int numNodeLists = mNodeLists.size();
  if (mOffsets.size() != numNodeLists) {
    cerr << "ConnectivityMap::valid: Failed mOffsets.size() == numNodeLists" << endl;
    return false;
  }
  {
    const int numNodes = ((domainDecompIndependent or mBuildGhostConnectivity) ? 
                          mNodeLists.back()->numNodes() : 
                          mNodeLists.back()->numInternalNodes());
    if (mConnectivity.size() != mOffsets.back() + numNodes) {
      cerr << "ConnectivityMap::valid: Failed offset bounding." << endl;
    }
  }

  // Make sure that the NodeLists are listed in the correct sequence, and are
  // FluidNodeLists.
  {
    const NodeListRegistrar<Dimension>& registrar = NodeListRegistrar<Dimension>::instance();
    const vector<string> names = registrar.registeredNames();
    int lastPosition = -1;
    for (typename vector<const NodeList<Dimension>*>::const_iterator itr = mNodeLists.begin();
         itr != mNodeLists.end();
         ++itr) {
      const int newPosition = distance(names.begin(),
                                       find(names.begin(), names.end(), (*itr)->name()));
      if (newPosition <= lastPosition) {
        cerr << "ConnectivityMap::valid: Failed ordering of NodeLists" << endl;
        return false;
      }
      lastPosition = newPosition;
    }
  }

  // Iterate over each NodeList entered.
  int nodeListIDi = 0;
  for (unsigned nodeListIDi = 0; nodeListIDi != numNodeLists; ++nodeListIDi) {

    // Are all internal nodes represented?
    const NodeList<Dimension>* nodeListPtri = mNodeLists[nodeListIDi];
    const int numNodes = ((domainDecompIndependent or mBuildGhostConnectivity) ? 
                          nodeListPtri->numNodes() : 
                          nodeListPtri->numInternalNodes());
    const int firstGhostNodei = nodeListPtri->firstGhostNode();
    if (((nodeListIDi < numNodeLists - 1) and (mOffsets[nodeListIDi + 1] - mOffsets[nodeListIDi] != numNodes)) or
        ((nodeListIDi == numNodeLists - 1) and (mConnectivity.size() - mOffsets[nodeListIDi] != numNodes))) {
      cerr << "ConnectivityMap::valid: Failed test that all nodes set for NodeList "
           << mNodeLists[nodeListIDi]->name()
           << endl;
      return false;
    }

    // Iterate over the nodes for this NodeList.
    const int ioff = mOffsets[nodeListIDi];
    for (int i = 0; i != numNodes; ++i) {

      // The set of neighbors for this node.  This has to be sized as the number of
      // NodeLists.
      const vector< vector<int> >& allNeighborsForNode = mConnectivity[ioff + i];
      if (allNeighborsForNode.size() != numNodeLists) {
        cerr << "ConnectivityMap::valid: Failed allNeighborsForNode.size() == numNodeLists" << endl;
        return false;
      }

      // Iterate over the sets of NodeList neighbors for this node.
      for (int nodeListIDj = 0; nodeListIDj != numNodeLists; ++nodeListIDj) {
        const NodeList<Dimension>* nodeListPtrj = mNodeLists[nodeListIDj];
        const int firstGhostNodej = nodeListPtrj->firstGhostNode();
        const vector<int>& neighbors = allNeighborsForNode[nodeListIDj];

        // We require that the node IDs be sorted, unique, and of course in a valid range.
        if (neighbors.size() > 0) {
          const int minNeighbor = *min_element(neighbors.begin(), neighbors.end());
          const int maxNeighbor = *max_element(neighbors.begin(), neighbors.end());

          if (minNeighbor < 0 or maxNeighbor >= nodeListPtrj->numNodes()) {
            cerr << "ConnectivityMap::valid: Failed test that neighbors must be valid IDs" << endl;
            return false;
          }

          // When enforcing domain independence the ith node may be a ghost, but all of it's neighbors should
          // be internal.
          if (domainDecompIndependent and (i >= firstGhostNodei) and (maxNeighbor > firstGhostNodej)) {
            cerr << "ConnectivityMap::valid: Failed test that all neighbors of a ghost node should be internal." << endl;
            return false;
          }

          for (int k = 1; k < neighbors.size(); ++k) {
            if (domainDecompIndependent) {
              // In the case of domain decomposition reproducibility, neighbors are sorted
              // by hashed IDs.
              if (mKeys(nodeListIDj, neighbors[k]) < mKeys(nodeListIDj, neighbors[k - 1])) {
                cerr << "ConnectivityMap::valid: Failed test that neighbors must be sorted for node "
                     << i << endl;
                for (vector<int>::const_iterator itr = neighbors.begin();
                     itr != neighbors.end();
                     ++itr) cerr << "(" << *itr << " " << mKeys(nodeListIDj, *itr) << ") ";
                cerr << endl;
                return false;
              }

            } else {
              // Otherwise they should be sorted by local ID.
              if (neighbors[k] <= neighbors[k - 1]) {
                cerr << "ConnectivityMap::valid: Failed test that neighbors must be sorted" << endl;
                for (vector<int>::const_iterator itr = neighbors.begin();
                     itr != neighbors.end();
                     ++itr) cerr << " " << *itr;
                cerr << endl;
                return false;
              }
            }
          }
        }

        // Check that the connectivity is symmetric.
        for (vector<int>::const_iterator jItr = neighbors.begin();
             jItr != neighbors.end();
             ++jItr) {
          if (domainDecompIndependent or mBuildGhostConnectivity or (*jItr < nodeListPtrj->numInternalNodes())) {
            const vector< vector<int> >& otherNeighbors = connectivityForNode(nodeListPtrj, *jItr);
            if (find(otherNeighbors[nodeListIDi].begin(),
                     otherNeighbors[nodeListIDi].end(),
                     i) == otherNeighbors[nodeListIDi].end()) {
              cerr << "ConnectivityMap::valid: Failed test that neighbors must be symmetric: " 
                   << i << " <> " << *jItr 
                   << "  numneigbors(i)=" << neighbors.size() 
                   << "  numneigbors(j)=" << otherNeighbors[nodeListIDi].size() 
                   << endl;
              cerr << "   " << i << " : ";
              std::copy(neighbors.begin(), neighbors.end(), std::ostream_iterator<int>(std::cerr, " "));
              cerr << endl
                   << "   " << *jItr << " : ";
              std::copy(otherNeighbors[nodeListIDi].begin(), otherNeighbors[nodeListIDi].end(), std::ostream_iterator<int>(std::cerr, " "));
              cerr << endl;
              return false;
            }
          }
        }

      }
    }
  }

  // Check that if we are using domain decompostion independence then the keys 
  // have been calculated.
  if (domainDecompIndependent) {
    for (typename vector<const NodeList<Dimension>*>::const_iterator itr = mNodeLists.begin();
         itr != mNodeLists.end();
         ++itr) {
      if (not mKeys.haveNodeList(**itr)) {
        cerr << "ConnectivityMap::valid: missing information from Keys." << endl;
        return false;
      }
    }
  }

  // Make sure all nodes are listed in the node index traversal stuff.
  if (mNodeTraversalIndices.size() != mNodeLists.size()) {
    cerr << "ConnectivityMap::valid: mNodeTraversalIndices wrong size!" << endl;
    return false;
  }
  for (int nodeList = 0; nodeList != numNodeLists; ++nodeList) {
    const int numExpected = domainDecompIndependent ? mNodeLists[nodeList]->numNodes() : mNodeLists[nodeList]->numInternalNodes();
    bool ok = mNodeTraversalIndices[nodeList].size() == numExpected;
    for (int i = 0; i != numExpected; ++i) {
      ok = ok and (count(mNodeTraversalIndices[nodeList].begin(),
                         mNodeTraversalIndices[nodeList].end(),
                         i) == 1);
    }
    if (not ok) {
      cerr << "ConnectivityMap::valid: mNodeTraversalIndices elements messed up!" << endl;
      return false;
    }
  }

  // // Check that the node traversal is ordered correctly.
  // for (int nodeList = 0; nodeList != numNodeLists; ++nodeList) {
  //   if ((domainDecompIndependent and mNodeLists[nodeList]->numNodes() > 0) or
  //       (not domainDecompIndependent and mNodeLists[nodeList]->numInternalNodes() > 0)) {
  //     const int firstGhostNode = mNodeLists[nodeList]->firstGhostNode();
  //     for (const_iterator itr = begin(nodeList); itr < end(nodeList) - 1; ++itr) {
  //       if (not calculatePairInteraction(nodeList, *itr,
  //                                        nodeList, *(itr + 1), 
  //                                        firstGhostNode)) {
  //         cerr << "ConnectivityMap::valid: mNodeTraversalIndices ordered incorrectly." << endl;
  //         cerr << *itr << " "
  //              << *(itr + 1) << " "
  //              << mKeys(nodeList, *itr) << " "
  //              << mKeys(nodeList, *(itr + 1)) << " "
  //              << mNodeLists[nodeList]->positions()(*itr) << " "
  //              << mNodeLists[nodeList]->positions()(*(itr + 1)) << " "
  //              << endl;
  //         for (int i = 0; i != 100; ++i) cerr << mKeys(nodeList, i) << " " << mNodeLists[nodeList]->positions()(i) << " ";
  //         cerr << endl;
  //         return false;
  //       }
  //     }
  //   }
  // }

  // Everything must be OK.
  TIME_ConnectivityMap_valid.stop();
  return true;
}

//------------------------------------------------------------------------------
// Internal method to build the connectivity for the requested set of NodeLists.
//------------------------------------------------------------------------------
template<typename Dimension>
void
ConnectivityMap<Dimension>::
computeConnectivity() {
  TIME_ConnectivityMap_computeConnectivity.start();

  typedef typename Dimension::Scalar Scalar;
  typedef typename Dimension::Vector Vector;
  typedef typename Dimension::Tensor Tensor;
  typedef typename Dimension::SymTensor SymTensor;
  typedef Timing::Time Time;

  // Pre-conditions.
  BEGIN_CONTRACT_SCOPE
  {
    for (typename vector<const NodeList<Dimension>*>::const_iterator itr = mNodeLists.begin();
         itr != mNodeLists.end();
         ++itr) {
      REQUIRE((**itr).neighbor().valid());
    }
    REQUIRE(mOffsets.size() == mNodeLists.size());
  }
  END_CONTRACT_SCOPE

  const bool domainDecompIndependent = NodeListRegistrar<Dimension>::instance().domainDecompositionIndependent();
  // std::clock_t tpre = std::clock();

  // Build ourselves a temporary DataBase with the set of NodeLists.
  // Simultaneously find the maximum kernel extent.
  DataBase<Dimension> dataBase;
  double kernelExtent = 0.0;
  for (typename vector<const NodeList<Dimension>*>::const_iterator itr = mNodeLists.begin();
       itr != mNodeLists.end();
       ++itr) {
    dataBase.appendNodeList(const_cast<NodeList<Dimension>&>(**itr));
    kernelExtent = max(kernelExtent, (**itr).neighbor().kernelExtent());
  }
  const double kernelExtent2 = kernelExtent*kernelExtent;

  // Erase any prior information.
  const unsigned numNodeLists = dataBase.numNodeLists(),
             connectivitySize = mOffsets.back() + 
                                ((domainDecompIndependent or mBuildGhostConnectivity) ? mNodeLists.back()->numNodes() : mNodeLists.back()->numInternalNodes());
  bool ok = (connectivitySize > 0 and mConnectivity.size() == connectivitySize);
  if (ok) {
    CHECK(mNodeTraversalIndices.size() == numNodeLists);
    for (typename ConnectivityStorageType::iterator itr = mConnectivity.begin();
         itr != mConnectivity.end();
         ++itr) {
      CHECK(itr->size() == numNodeLists);
      for (unsigned k = 0; k != numNodeLists; ++k) {
        (*itr)[k].clear();
      }
    }
  } else {
    mConnectivity = ConnectivityStorageType(connectivitySize, vector<vector<int> >(numNodeLists));
    mNodeTraversalIndices = vector<vector<int> >(numNodeLists);
  }
  mNodePairList.clear();

  // If we're trying to be domain decomposition independent, we need a key to sort
  // by that will give us a unique ordering regardless of position.  The Morton ordered
  // hash fills the bill.
  typedef typename KeyTraits::Key Key;
  if (domainDecompIndependent) mKeys = mortonOrderIndices(dataBase);

  // Fill in the ordering for walking the nodes.
  CHECK(mNodeTraversalIndices.size() == numNodeLists);
  if (domainDecompIndependent) {
    for (int iNodeList = 0; iNodeList != numNodeLists; ++iNodeList) {
      const NodeList<Dimension>& nodeList = *mNodeLists[iNodeList];
      mNodeTraversalIndices[iNodeList].resize(nodeList.numNodes());
      vector<pair<int, Key> > keys;
      keys.reserve(nodeList.numNodes());
      for (int i = 0; i != nodeList.numNodes(); ++i) keys.push_back(pair<int, Key>(i, mKeys(iNodeList, i)));
      sort(keys.begin(), keys.end(), ComparePairsBySecondElement<pair<int, Key> >());
      for (int i = 0; i != nodeList.numNodes(); ++i) mNodeTraversalIndices[iNodeList][i] = keys[i].first;
      CHECK(mNodeTraversalIndices[iNodeList].size() == nodeList.numNodes());
      // std::cerr << "Traversal: ";
      // std::copy(mNodeTraversalIndices[iNodeList].begin(), mNodeTraversalIndices[iNodeList].end(), std::ostream_iterator<int>(std::cerr, " "));
      // std::cerr << std::endl;
    }
  } else {
    for (int iNodeList = 0; iNodeList != numNodeLists; ++iNodeList) {
      const NodeList<Dimension>& nodeList = *mNodeLists[iNodeList];
      mNodeTraversalIndices[iNodeList].resize(nodeList.numInternalNodes());
      for (int i = 0; i != nodeList.numInternalNodes(); ++i) mNodeTraversalIndices[iNodeList][i] = i;
    }
  }

  // Create a list of flags to keep track of which nodes have been completed thus far.
  FieldList<Dimension, int> flagNodeDone = dataBase.newGlobalFieldList(int());
  flagNodeDone = 0;

  // Get the position and H fields.
  const auto position = dataBase.globalPosition();
  const auto H = dataBase.globalHfield();

  // Iterate over the NodeLists.
  // std::clock_t t0, 
  //   tmaster = std::clock_t(0), 
  //   trefine = std::clock_t(0), 
  //   twalk = std::clock_t(0);
  CHECK(mConnectivity.size() == connectivitySize);
  for (auto iiNodeList = 0; iiNodeList != numNodeLists; ++iiNodeList) {
    const auto etaMax = mNodeLists[iiNodeList]->neighbor().kernelExtent();

    // Iterate over the nodes in this NodeList, and look for any that are not done yet.
    const auto nii = (false ? // domainDecompIndependent or mBuildGhostConnectivity ? 
                      mNodeLists[iiNodeList]->numNodes() :
                      mNodeLists[iiNodeList]->numInternalNodes());
    for (auto ii = 0; ii < nii; ++ii) {
      if (flagNodeDone(iiNodeList, ii) == 0) {

        // Set the master nodes.
        // t0 = std::clock();
        vector<vector<int>> masterLists, coarseNeighbors;
        Neighbor<Dimension>::setMasterNeighborGroup(position(iiNodeList, ii),
                                                    H(iiNodeList, ii),
                                                    mNodeLists.begin(),
                                                    mNodeLists.end(),
                                                    etaMax,
                                                    masterLists,
                                                    coarseNeighbors);

        // Iterate over the full of NodeLists again to work on the master nodes.
        for (auto iNodeList = 0; iNodeList != numNodeLists; ++iNodeList) {
          const auto nmaster = masterLists[iNodeList].size();
#pragma omp parallel 
          {
            NodePairList nodePairs_private;
#pragma omp for schedule(dynamic)
            for (auto k = 0; k < nmaster; ++k) {
              const auto i = masterLists[iNodeList][k];
              CHECK2(flagNodeDone(iNodeList, i) == 0, "(" << iNodeList << " " << i << ") (" << iNodeList << " " << i << ")");

              // Get the state for this node.
              const auto& ri = position(iNodeList, i);
              const auto& Hi = H(iNodeList, i);
              auto&       worki = mNodeLists[iNodeList]->work();
              CHECK(mOffsets[iNodeList] + i < mConnectivity.size());
              const auto start = Timing::currentTime();

              // Get the neighbor set we're building for this node.
              auto& neighbors = mConnectivity[mOffsets[iNodeList] + i];
              CHECK2(neighbors.size() == numNodeLists, neighbors.size() << " " << numNodeLists << " " << i);

              // We keep track of the Morton indices.
              vector<vector<pair<int, Key>>> keys(numNodeLists);

              // Iterate over the neighbor NodeLists.
              for (auto jNodeList = 0; jNodeList != numNodeLists; ++jNodeList) {
                const auto firstGhostNodej = mNodeLists[jNodeList]->firstGhostNode();

                // Iterate over the coarse neighbors in this NodeList.
                // t0 = std::clock();
                for (const auto j:  coarseNeighbors[jNodeList]) {
                  const auto& rj = position(jNodeList, j);
                  const auto& Hj = H(jNodeList, j);

                  // Compute the normalized distance between this pair.
                  const auto rij = ri - rj;
                  const auto eta2i = (Hi*rij).magnitude2();
                  const auto eta2j = (Hj*rij).magnitude2();

                  // If this pair is significant, add it to the list.
                  if (eta2i <= kernelExtent2 or eta2j <= kernelExtent2) {

                    // We don't include self-interactions.
                    if ((iNodeList != jNodeList) or (i != j)) {
                      neighbors[jNodeList].push_back(j);
                      if (calculatePairInteraction(iNodeList, i, jNodeList, j, firstGhostNodej)) 
                        nodePairs_private.push_back(NodePairIdxType(i, iNodeList, j, jNodeList));
                      if (domainDecompIndependent) 
                        keys[jNodeList].push_back(pair<int, Key>(j, mKeys(jNodeList, j)));
                    }
                  }
                }
                // twalk += std::clock() - t0;
              }
              CHECK(neighbors.size() == numNodeLists);
              CHECK(keys.size() == numNodeLists);
        
              // We have a few options for how to order the neighbors for this node.
              for (auto jNodeList = 0; jNodeList != numNodeLists; ++jNodeList) {

                if (domainDecompIndependent) {
                  // Sort in a domain independent manner.
                  CHECK(keys[jNodeList].size() == neighbors[jNodeList].size());
                  sort(keys[jNodeList].begin(), keys[jNodeList].end(), ComparePairsBySecondElement<pair<int, Key>>());
                  for (auto j = 0; j != neighbors[jNodeList].size(); ++j) neighbors[jNodeList][j] = keys[jNodeList][j].first;
                } else {
                  // Sort in an attempt to be cache friendly.
                  sort(neighbors[jNodeList].begin(), neighbors[jNodeList].end());
                }
              }

              // Flag this master node as done.
              flagNodeDone(iNodeList, i) = 1;
              worki(i) += Timing::difference(start, Timing::currentTime());
            }
            
            // Merge the NodePairList
#pragma omp critical
            mNodePairList.insert(mNodePairList.end(), nodePairs_private.begin(), nodePairs_private.end());
          } // end OMP parallel
        }
      }
    }
  }

  // If necessary add ghost->internal connectivity.
  if (mBuildGhostConnectivity or domainDecompIndependent) {
    for (auto iNodeList = 0; iNodeList < numNodeLists; ++iNodeList) {
      for (auto i = 0; i < mNodeLists[iNodeList]->numInternalNodes(); ++i) {
        const auto& neighborsi = mConnectivity[mOffsets[iNodeList] + i];
        CHECK(neighborsi.size() == numNodeLists);
        for (auto jNodeList = 0; jNodeList < numNodeLists; ++jNodeList) {
          const auto firstGhostNodej = mNodeLists[jNodeList]->firstGhostNode();
          for (auto jItr = neighborsi[jNodeList].begin(); jItr < neighborsi[jNodeList].end(); ++jItr) {
            const auto j = *jItr;
            if (j >= firstGhostNodej) {
              auto& neighborsj = mConnectivity[mOffsets[jNodeList] + j];
              CHECK(neighborsj.size() == numNodeLists);
              neighborsj[iNodeList].push_back(i);
              // mNodePairList.push_back(NodePairIdxType(i, iNodeList, j, jNodeList));
            }
          }
        }
      }
    }

    // Flag ghost nodes as done if at least one neighbor was found
    for (auto iNodeList = 0; iNodeList < numNodeLists; ++iNodeList) {
      for (auto i = mNodeLists[iNodeList]->numInternalNodes(); i < mNodeLists[iNodeList]->numNodes(); ++i) {
        const auto& neighborsi = mConnectivity[mOffsets[iNodeList] + i];
        if (neighborsi.size() > 0) {
          flagNodeDone(iNodeList, i) = 1;
        }
      }
    }
  }

  // In the domain decompostion independent case, we need to sort the neighbors for ghost
  // nodes as well.
  if (domainDecompIndependent) {
    for (auto iNodeList = 0; iNodeList != numNodeLists; ++iNodeList) {
      const auto* nodeListPtr = mNodeLists[iNodeList];
      for (auto i = nodeListPtr->firstGhostNode();
           i != nodeListPtr->numNodes();
           ++i) {
        auto& neighbors = mConnectivity[mOffsets[iNodeList] + i];
        CHECK(neighbors.size() == numNodeLists);
        for (auto jNodeList = 0; jNodeList != numNodeLists; ++jNodeList) {
          vector<pair<int, Key>> keys;
          keys.reserve(neighbors[jNodeList].size());
          for (auto itr = neighbors[jNodeList].begin();
               itr != neighbors[jNodeList].end();
               ++itr) keys.push_back(pair<int, Key>(*itr, mKeys(jNodeList, *itr)));
          CHECK(keys.size() == neighbors[jNodeList].size());
          sort(keys.begin(), keys.end(), ComparePairsBySecondElement<pair<int, Key> >());
          for (auto k = 0; k != keys.size(); ++k) neighbors[jNodeList][k] = keys[k].first;
        }
      }
    }
  }

  // Sort the NodePairList in order to enforce domain decomposition independence.
  if (domainDecompIndependent) {
    sort(mNodePairList.begin(), mNodePairList.end(), [this](const NodePairIdxType& a, const NodePairIdxType& b) { return (mKeys(a.i_list, a.i_node) + mKeys(a.j_list, a.j_node)) < (mKeys(b.i_list, b.i_node) + mKeys(b.j_list, b.j_node)); });
  }

  // Do we need overlap connectivity?
  if (mBuildOverlapConnectivity) {
    // VERIFY2(mBuildGhostConnectivity, "ghost connectivity is required for overlap connectivity");
    TIME_ConnectivityMap_computeOverlapConnectivity.start();

    // To start out, *all* neighbors of a node (gather and scatter) are overlap neighbors.  Therefore we
    // first just copy the neighbor connectivity.
    mOverlapConnectivity = mConnectivity;

    for (auto iNodeList = 0; iNodeList < numNodeLists; ++iNodeList) {
      const auto* nodeListPtr = mNodeLists[iNodeList];
      for (auto i = 0; i < nodeListPtr->numNodes(); ++i) {
        const auto& neighborsi = mConnectivity[mOffsets[iNodeList] + i];
        CHECK(neighborsi.size() == numNodeLists);
        const auto& ri = position(iNodeList, i);
        const auto& Hi = H(iNodeList, i);

        // Find all the gather neighbors of i.
        for (auto jN1 = 0; jN1 < numNodeLists; ++jN1) {
          for (const auto j1: neighborsi[jN1]) {
            const auto& rj1 = position(jN1, j1);
            const auto& Hj1 = H(jN1, j1);
            if ((Hi*(rj1 - ri)).magnitude2() <= kernelExtent2) {                           // Is j1 a gather neighbor of i?

              // Check if i and j1 have overlap directly.
              if ((Hj1*(rj1 - ri)).magnitude2() <= kernelExtent2) {
                insertUnique(mOffsets, mOverlapConnectivity,
                             iNodeList, i, jN1, j1);
                insertUnique(mOffsets, mOverlapConnectivity,
                             jN1, j1, iNodeList, i);
              }

              // Find the gather neighbors of j1, all of which share overlap with i.
              const auto& neighborsj1 = mConnectivity[mOffsets[jN1] + j1];
              for (auto jN2 = 0; jN2 < numNodeLists; ++jN2) {
                for (const auto j2: neighborsj1[jN2]) {
                  const auto& rj2 = position(jN2, j2);
                  const auto& Hj2 = H(jN2, j2);
                  if ((Hj2*(rj2 - rj1)).magnitude2() <= kernelExtent2) {                   // Is j2 a scatter neighbor of j1?
                    insertUnique(mOffsets, mOverlapConnectivity,
                                 iNodeList, i, jN2, j2);
                    insertUnique(mOffsets, mOverlapConnectivity,
                                 jN2, j2, iNodeList, i);
                  }
                }
              }
            }
          }
        }
      }
    }
    TIME_ConnectivityMap_computeOverlapConnectivity.stop();
  }

  // {
  //   tpre = allReduce(unsigned(tpre), MPI_SUM, Communicator::communicator()) / Process::getTotalNumberOfProcesses() / CLOCKS_PER_SEC;
  //   tmaster = allReduce(unsigned(tmaster), MPI_SUM, Communicator::communicator()) / Process::getTotalNumberOfProcesses() / CLOCKS_PER_SEC;
  //   trefine = allReduce(unsigned(trefine), MPI_SUM, Communicator::communicator()) / Process::getTotalNumberOfProcesses() / CLOCKS_PER_SEC;
  //   twalk = allReduce(unsigned(twalk), MPI_SUM, Communicator::communicator()) / Process::getTotalNumberOfProcesses() / CLOCKS_PER_SEC;
  //   if (Process::getRank() == 0) {
  //     std::cerr << "ConnectivityMap timings (pre, master, refine, walk) = " << tpre << " " << tmaster << " " << trefine << " " << twalk << std::endl;
  //   }
  // }

  // Post conditions.
  BEGIN_CONTRACT_SCOPE
  // Make sure that the correct number of nodes have been completed.
  for (auto iNodeList = 0; iNodeList != numNodeLists; ++iNodeList) {
    const auto n = (mBuildGhostConnectivity ? 
                    mNodeLists[iNodeList]->numNodes() :
                    mNodeLists[iNodeList]->numInternalNodes());
    for (auto i = 0; i != n; ++i) {
      ENSURE2(flagNodeDone(iNodeList, i) == 1,
              "Missed connnectivity for (" << iNodeList << " " << i << ")");
    }
  }
  // Make sure we're ready to be used.
  ENSURE(valid());
  END_CONTRACT_SCOPE

  TIME_ConnectivityMap_computeConnectivity.stop();
}

}
