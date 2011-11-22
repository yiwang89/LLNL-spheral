//---------------------------------Spheral++----------------------------------//
// packElement
// Methods to pack/unpack sets of values into vector<char>.  Useful for 
// constructing MPI buffers of data.
//
// Created by J. Michael Owen, Tue Oct 10 22:16:29 PDT 2006
//----------------------------------------------------------------------------//
#ifndef __Spheral_packElement__
#define __Spheral_packElement__

#include <stdint.h>
#include <vector>
#include <iterator>
#include "DataTypeTraits.hh"

#ifdef USE_MPI
extern "C" {
#include "mpi.h"
}
#endif

namespace Spheral {
  namespace FieldSpace {
    template<typename Dimension, typename DataType> class Field;
  }
}

namespace Spheral {

//------------------------------------------------------------------------------
// A standalone method to pack a given DataType into a vector
// The generic version assumes the DataType has a begin/end iterator pair, and
// a fixed size.
//------------------------------------------------------------------------------
template<typename DataType>
inline
void
packElement(const DataType& value,
            std::vector<char>& buffer) {
  const int packSize = sizeof(typename DataTypeTraits<DataType>::ElementType);
  for (typename DataType::const_iterator valueItr = value.begin();
       valueItr != value.end();
       ++valueItr) {
    char* data = reinterpret_cast<char*>(const_cast<typename DataTypeTraits<DataType>::ElementType*>(&(*valueItr)));
    for (int i = 0; i != packSize; ++i) {
      buffer.push_back(*(data + i));
    }
  }
}

// Specialization for an int type.
template<>
inline
void
packElement<int>(const int& value, 
                 std::vector<char>& buffer) {
  const int packSize = sizeof(int);
  char* data = reinterpret_cast<char*>(const_cast<int*>(&value));
  for (int i = 0; i != packSize; ++i) {
    buffer.push_back(*(data + i));
  }
}

// // Specialization for an unsigned int type.
// template<>
// inline
// void
// packElement<unsigned>(const unsigned& value, 
//                       std::vector<char>& buffer) {
//   const int packSize = sizeof(unsigned);
//   char* data = reinterpret_cast<char*>(const_cast<unsigned*>(&value));
//   for (int i = 0; i != packSize; ++i) {
//     buffer.push_back(*(data + i));
//   }
// }

// // Specialization for an unsigned long int type.
// template<>
// inline
// void
// packElement<unsigned long>(const unsigned long& value, 
//                            std::vector<char>& buffer) {
//   const int packSize = sizeof(unsigned long);
//   char* data = reinterpret_cast<char*>(const_cast<unsigned long*>(&value));
//   for (int i = 0; i != packSize; ++i) {
//     buffer.push_back(*(data + i));
//   }
// }

// // Specialization for an unsigned long long int type.
// template<>
// inline
// void
// packElement<unsigned long long>(const unsigned& value, 
//                                 std::vector<char>& buffer) {
//   const int packSize = sizeof(unsigned long long);
//   char* data = reinterpret_cast<char*>(const_cast<unsigned long long*>(&value));
//   for (int i = 0; i != packSize; ++i) {
//     buffer.push_back(*(data + i));
//   }
// }

// Specialization for an uint32_t int type.
template<>
inline
void
packElement<uint32_t>(const uint32_t& value, 
                      std::vector<char>& buffer) {
  const int packSize = sizeof(uint32_t);
  char* data = reinterpret_cast<char*>(const_cast<uint32_t*>(&value));
  for (int i = 0; i != packSize; ++i) {
    buffer.push_back(*(data + i));
  }
}

// Specialization for an uint64_t int type.
template<>
inline
void
packElement<uint64_t>(const uint64_t& value, 
                                std::vector<char>& buffer) {
  const int packSize = sizeof(uint64_t);
  char* data = reinterpret_cast<char*>(const_cast<uint64_t*>(&value));
  for (int i = 0; i != packSize; ++i) {
    buffer.push_back(*(data + i));
  }
}

// Specialization for a scalar (float) type.
template<>
inline
void
packElement<float>(const float& value, 
                   std::vector<char>& buffer) {
  const int packSize = sizeof(float);
  char* data = reinterpret_cast<char*>(const_cast<float*>(&value));
  for (int i = 0; i != packSize; ++i) {
    buffer.push_back(*(data + i));
  }
}

// Specialization for a scalar (double) type.
template<>
inline
void
packElement<double>(const double& value, 
                    std::vector<char>& buffer) {
  const int packSize = sizeof(double);
  char* data = reinterpret_cast<char*>(const_cast<double*>(&value));
  for (int i = 0; i != packSize; ++i) {
    buffer.push_back(*(data + i));
  }
}

// Specialization for a boost::tuple of three common elements.
template<typename T>
inline
void
packElement(const boost::tuple<T, T, T>& value,
            std::vector<char>& buffer) {
  packElement(boost::get<0>(value), buffer);
  packElement(boost::get<1>(value), buffer);
  packElement(boost::get<2>(value), buffer);
}

// Specialize for a std::vector<DataType>.
// Assumes the elements of the vector<> are of a type we already know how to pack.
template<typename DataType>
inline
void
packElement(const std::vector<DataType>& value, 
            std::vector<char>& buffer) {

  // First push the size of the vector onto the buffer.
  const unsigned size = value.size();
  packElement(size, buffer);

  // Now push each of the elements on.
  for (typename std::vector<DataType>::const_iterator itr = value.begin();
       itr != value.end();
       ++itr) {
    packElement(*itr, buffer);
  }
}

//------------------------------------------------------------------------------
// A standalone method to unpack an encoded value from the given
// vector<*>::iterator to the dataType.
// Note these methods increment the buffer iterator as they go, which alters 
// the value in the calling method.
//------------------------------------------------------------------------------
template<typename DataType>
inline
void
unpackElement(DataType& value, 
              std::vector<char>::const_iterator& itr,
              const std::vector<char>::const_iterator& endPackedVector) {
  const int packSize = sizeof(typename DataTypeTraits<DataType>::ElementType);
  for (typename DataType::iterator valueItr = value.begin();
       valueItr != value.end();
       ++valueItr) {
    char* data = reinterpret_cast<char*>(&(*valueItr));
    for (int i = 0; i != packSize; ++i, ++itr) {
      CHECK(itr < endPackedVector);
      *(data + i) = *itr;
    }
  }
  ENSURE(itr <= endPackedVector);
}

// Specialization for an int type.
template<>
inline
void
unpackElement<int>(int& value,
                   std::vector<char>::const_iterator& itr,
                   const std::vector<char>::const_iterator& endPackedVector) {
  const int packSize = sizeof(int);
  char* data = reinterpret_cast<char*>(&value);
  for (int i = 0; i != packSize; ++i, ++itr) {
    CHECK(itr < endPackedVector);
    *(data + i) = *itr;
  }
  ENSURE(itr <= endPackedVector);
}

// // Specialization for an unsigned int type.
// template<>
// inline
// void
// unpackElement<unsigned>(unsigned& value,
//                         std::vector<char>::const_iterator& itr,
//                         const std::vector<char>::const_iterator& endPackedVector) {
//   const int packSize = sizeof(unsigned);
//   char* data = reinterpret_cast<char*>(&value);
//   for (int i = 0; i != packSize; ++i, ++itr) {
//     CHECK(itr < endPackedVector);
//     *(data + i) = *itr;
//   }
//   ENSURE(itr <= endPackedVector);
// }

// // Specialization for an unsigned long int type.
// template<>
// inline
// void
// unpackElement<unsigned long>(unsigned long& value,
//                         std::vector<char>::const_iterator& itr,
//                         const std::vector<char>::const_iterator& endPackedVector) {
//   const int packSize = sizeof(unsigned long);
//   char* data = reinterpret_cast<char*>(&value);
//   for (int i = 0; i != packSize; ++i, ++itr) {
//     CHECK(itr < endPackedVector);
//     *(data + i) = *itr;
//   }
//   ENSURE(itr <= endPackedVector);
// }

// Specialization for an uint32_t int type.
template<>
inline
void
unpackElement<uint32_t>(uint32_t& value,
                             std::vector<char>::const_iterator& itr,
                             const std::vector<char>::const_iterator& endPackedVector) {
  const int packSize = sizeof(uint32_t);
  char* data = reinterpret_cast<char*>(&value);
  for (int i = 0; i != packSize; ++i, ++itr) {
    CHECK(itr < endPackedVector);
    *(data + i) = *itr;
  }
  ENSURE(itr <= endPackedVector);
}

// Specialization for an uint64_t int type.
template<>
inline
void
unpackElement<uint64_t>(uint64_t& value,
                                  std::vector<char>::const_iterator& itr,
                                  const std::vector<char>::const_iterator& endPackedVector) {
  const int packSize = sizeof(uint64_t);
  char* data = reinterpret_cast<char*>(&value);
  for (int i = 0; i != packSize; ++i, ++itr) {
    CHECK(itr < endPackedVector);
    *(data + i) = *itr;
  }
  ENSURE(itr <= endPackedVector);
}

// Specialization for a scalar (float) type.
template<>
inline
void
unpackElement<float>(float& value,
                     std::vector<char>::const_iterator& itr,
                     const std::vector<char>::const_iterator& endPackedVector) {
  const int packSize = sizeof(float);
  char* data = reinterpret_cast<char*>(&value);
  for (int i = 0; i != packSize; ++i, ++itr) {
    CHECK(itr < endPackedVector);
    *(data + i) = *itr;
  }
  ENSURE(itr <= endPackedVector);
}

// Specialization for a scalar (double) type.
template<>
inline
void
unpackElement<double>(double& value,
                      std::vector<char>::const_iterator& itr,
                      const std::vector<char>::const_iterator& endPackedVector) {
  const int packSize = sizeof(double);
  char* data = reinterpret_cast<char*>(&value);
  for (int i = 0; i != packSize; ++i, ++itr) {
    CHECK(itr < endPackedVector);
    *(data + i) = *itr;
  }
  ENSURE(itr <= endPackedVector);
}

// boost::tuple<T,T,T>
template<typename DataType>
inline
void
unpackElement(boost::tuple<DataType, DataType, DataType>& value,
              std::vector<char>::const_iterator& itr,
              const std::vector<char>::const_iterator& endPackedVector) {
  DataType x, y, z;
  unpackElement(x, itr, endPackedVector);
  unpackElement(y, itr, endPackedVector);
  unpackElement(z, itr, endPackedVector);
  boost::get<0>(value) = x;
  boost::get<1>(value) = y;
  boost::get<2>(value) = z;
}

// Handle the vector<DataType> case, so long as DataType is one of the types
// we know how to unpack.
template<typename DataType>
inline
void
unpackElement(std::vector<DataType>& value,
              std::vector<char>::const_iterator& itr,
              const std::vector<char>::const_iterator& endPackedVector) {

  // Read the size of the vector.
  unsigned size;
  unpackElement(size, itr, endPackedVector);
  CHECK2(size <= std::distance(itr, endPackedVector),
         "Crazy buffer size:  " << size << " " << std::distance(itr, endPackedVector));

  // Now iterate over the number of elements we will unpack, and push them onto
  // the value.
  for (int i = 0; i != size; ++i) {
    DataType element;
    unpackElement(element, itr, endPackedVector);
    value.push_back(element);
  }

  ENSURE(itr <= endPackedVector);
}

//------------------------------------------------------------------------------
// Compute the size of a buffer necessary to hold the given field data.
//------------------------------------------------------------------------------
template<typename Dimension, typename DataType>
int
computeBufferSize(const FieldSpace::Field<Dimension, DataType>& field,
                  const std::vector<int>& packIndicies,
                  const int sendProc,
                  const int recvProc) {
  return (packIndicies.size() * 
          DataTypeTraits<DataType>::numElements(DataType()) * 
          sizeof(typename DataTypeTraits<DataType>::ElementType));
}

// Specialization for a std::vector
template<typename Dimension, typename DataType>
int
computeBufferSize(const FieldSpace::Field<Dimension, std::vector<DataType> >& field,
                  const std::vector<int>& packIndicies,
                  const int sendProc,
                  const int recvProc) {

  // Byte size of the primitive type of the vector.
  const int elementSize = (DataTypeTraits<DataType>::numElements(DataType()) * 
                           sizeof(typename DataTypeTraits<DataType>::ElementType));

  // Find the rank of this processor.
  int rank = 0;
#ifdef USE_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
#endif
  REQUIRE(rank == sendProc || rank == recvProc);

  // The send proc can compute the necessary size.
  int bufSize;
  if (rank == sendProc) {
    for (std::vector<int>::const_iterator itr = packIndicies.begin();
         itr != packIndicies.end();
         ++itr) {
      bufSize += field(*itr).size();
    }
    bufSize *= elementSize;
  }

  // Communicate the result to the receiving processor.
#ifdef USE_MPI
  if (rank == recvProc) {
    MPI_Status status;
    MPI_Recv(&bufSize, 1, MPI_INT, sendProc, 103, MPI_COMM_WORLD, &status);
  } else if (rank == sendProc) {
    MPI_Send(&bufSize, 1, MPI_INT, recvProc, 103, MPI_COMM_WORLD);
  }
#endif
  return bufSize;
}

//------------------------------------------------------------------------------
// Pack the given Field values into a 1-D array for transmission.
//------------------------------------------------------------------------------
template<typename Dimension, typename DataType>
std::vector<char>
packFieldValues(const FieldSpace::Field<Dimension, DataType>& field,
                const std::vector<int>& packIndicies) {

  // Prepare the return vector.
  std::vector<char> result;

  // Loop over the elements of the Field we are packing, and push the 
  // packed elements onto the result.
  for (std::vector<int>::const_iterator elementItr = packIndicies.begin();
       elementItr != packIndicies.end();
       ++elementItr) {
    CHECK(*elementItr >= 0 && *elementItr < field.numElements());
    packElement(field(*elementItr), result);
  }

  return result;
}

//------------------------------------------------------------------------------
// Unpack encoded values from the given vector to the Field at the indicated
// indicies.
//------------------------------------------------------------------------------
template<typename Dimension, typename DataType>
void
unpackFieldValues(FieldSpace::Field<Dimension, DataType>& field,
                  const std::vector<int>& packIndicies,
                  const std::vector<char>& packedValues) {

  // Loop over the elements of the Field we are unpacking.
  typename std::vector<char>::const_iterator bufItr = packedValues.begin();
  for (std::vector<int>::const_iterator elementItr = packIndicies.begin();
       elementItr != packIndicies.end();
       ++elementItr) {
    CHECK(*elementItr >= 0 && *elementItr < field.numElements());
    CHECK(bufItr < packedValues.end());
    unpackElement(field(*elementItr), bufItr, packedValues.end());
    CHECK(bufItr <= packedValues.end());
  }

  ENSURE(bufItr == packedValues.end());
}

//------------------------------------------------------------------------------
// Pack the Volume types.
//------------------------------------------------------------------------------
template<>
inline
void
packElement(const Dim<1>::Box& value,
            std::vector<char>& buffer) {
  packElement(value.center(), buffer);
  packElement(value.extent(), buffer);
}

template<>
inline
void
packElement(const GeomPolygon& value,
            std::vector<char>& buffer) {
  packElement(value.vertices(), buffer);
  packElement(value.facetVertices(), buffer);
}

template<>
inline
void
packElement(const GeomPolyhedron& value,
            std::vector<char>& buffer) {
  packElement(value.vertices(), buffer);
  packElement(value.facetVertices(), buffer);
  packElement(value.facetNormals(), buffer);
}

//------------------------------------------------------------------------------
// Unpack the Volume types.
//------------------------------------------------------------------------------
template<>
inline
void
unpackElement(Dim<1>::Box& value,
              std::vector<char>::const_iterator& itr,
              const std::vector<char>::const_iterator& endPackedVector) {
  Dim<1>::Vector center;
  double extent;
  unpackElement(center, itr, endPackedVector);
  unpackElement(extent, itr, endPackedVector);
  value.center(center);
  value.extent(extent);
}              

template<>
inline
void
unpackElement(GeomPolygon& value,
              std::vector<char>::const_iterator& itr,
              const std::vector<char>::const_iterator& endPackedVector) {
  std::vector<Dim<2>::Vector> vertices;
  std::vector<std::vector<unsigned> > facetVertices;
  unpackElement(vertices, itr, endPackedVector);
  unpackElement(facetVertices, itr, endPackedVector);
  value.reconstruct(vertices, facetVertices);
}

template<>
inline
void
unpackElement(GeomPolyhedron& value,
              std::vector<char>::const_iterator& itr,
              const std::vector<char>::const_iterator& endPackedVector) {
  std::vector<Dim<3>::Vector> vertices, facetNormals;
  std::vector<std::vector<unsigned> > facetVertices;
  unpackElement(vertices, itr, endPackedVector);
  unpackElement(facetVertices, itr, endPackedVector);
  unpackElement(facetNormals, itr, endPackedVector);
  value.reconstruct(vertices, facetVertices, facetNormals);
}

}

// To be helpful we also load up the WildMagic element types.
#include "packWMElement.hh"

#endif
