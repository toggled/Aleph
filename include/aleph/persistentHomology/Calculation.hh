#ifndef ALEPH_PERSISTENT_HOMOLOGY_CALCULATION_HH__
#define ALEPH_PERSISTENT_HOMOLOGY_CALCULATION_HH__

#include <aleph/config/Defaults.hh>

#include <aleph/persistenceDiagrams/PersistenceDiagram.hh>
#include <aleph/persistenceDiagrams/Calculation.hh>

#include <aleph/persistentHomology/PersistencePairing.hh>

#include <aleph/topology/Conversions.hh>
#include <aleph/topology/SimplicialComplex.hh>

#include <algorithm>
#include <tuple>
#include <unordered_set>
#include <vector>

namespace aleph
{

/**
  Given a boundary matrix, reduces it and reads off the resulting
  persistence pairing. An optional parameter can be used to force
  the algorithm to stop processing a part of the pairing. This is
  especially relevant for intersection homology, which sets upper
  limits for the validity of an index in the matrix.

  @param M                          Boundary matrix to reduce

  @param includeAllUnpairedCreators Flag indicating whether all unpaired creators should
                                    be included (regardless of their dimension). If set,
                                    this increases the size of the resulting pairing, as
                                    the highest-dimensional columns of the matrix cannot
                                    be reduced any more. The flag is useful, however, in
                                    case one wants to calculate ordinary homology, where
                                    high-dimensional simplices are used for Betti number
                                    calculations.

  @param max                        Optional maximum index after which simplices are not
                                    considered any more. If the pairing of a simplex has
                                    an index larger than the maximum one, such simplices
                                    will not be considered in the pairing.

  @tparam ReductionAlgorithm Specifies a reduction algorithm to use for reducing
                             the input matrix. Aleph provides a default value in
                             order to simplify the usage of this function.

  @tparam Representation     The representation of the boundary matrix, i.e. how
                             columns are stored. This parameter is automatically
                             determined from the input data.
*/

template <
  class ReductionAlgorithm = aleph::defaults::ReductionAlgorithm,
  class Representation
> PersistencePairing<typename Representation::Index> calculatePersistencePairing( const topology::BoundaryMatrix<Representation>& M,
                                                                                  bool includeAllUnpairedCreators    = false,
                                                                                  typename Representation::Index max = typename Representation::Index() )
{
  using namespace topology;

  using Index              = typename Representation::Index;
  using PersistencePairing = PersistencePairing<Index>;

  BoundaryMatrix<Representation> B = M;

  ReductionAlgorithm reductionAlgorithm;
  reductionAlgorithm( B );

  PersistencePairing pairing;

  auto numColumns = max ? max : B.getNumColumns();

  std::unordered_set<Index> creators;

  for( Index j = Index(0); j < numColumns; j++ )
  {
    Index i;
    bool valid;

    std::tie( i, valid ) = B.getMaximumIndex( j );
    if( valid )
    {
      auto u = i;
      auto v = j;
      auto w = u;

      // Column j is non-zero. It destroys the feature created by its
      // lowest 1. Hence, i does not remain a creator.
      creators.erase( i );

      if( B.isDualized() )
      {
        u  = numColumns - 1 - v;
        v  = numColumns - 1 - w; // Yes, this is correct!
      }

      if( !max || i < max )
        pairing.add( u, v );
    }

    // An invalid maximum index indicates that the corresponding column
    // is empty. Hence, we need to think about whether it signifies one
    // feature with infinite persistence.
    else
    {
      // Only add creators that do not belong to the largest dimension
      // of the boundary matrix. Else, there will be a lot of spurious
      // features that cannot be destroyed due to their dimensions. If
      // the client wants to have them, however, we let them.
      if(    ( !B.isDualized() && B.getDimension(j) != B.getDimension() )
          || (  B.isDualized() && B.getDimension(j) != Index(0) )
          || includeAllUnpairedCreators )
      {
        creators.insert( j );
      }
    }
  }

  for( auto&& creator : creators )
  {
    if( B.isDualized() )
      pairing.add( numColumns - 1 - creator );
    else
      pairing.add( creator );
  }

  std::sort( pairing.begin(), pairing.end() );
  return pairing;
}

template <
  class ReductionAlgorithm = defaults::ReductionAlgorithm,
  class Representation     = defaults::Representation,
  class Simplex
> std::vector< PersistenceDiagram<typename Simplex::DataType> > calculatePersistenceDiagrams( const topology::SimplicialComplex<Simplex>& K, bool dualize = true, bool includeAllUnpairedCreators = false )
{
  using namespace topology;

  auto boundaryMatrix = makeBoundaryMatrix<Representation>( K );
  auto pairing        = calculatePersistencePairing<ReductionAlgorithm>( dualize ? boundaryMatrix.dualize() : boundaryMatrix, includeAllUnpairedCreators );

  return makePersistenceDiagrams( pairing, K );
}

template <
  class ReductionAlgorithm = defaults::ReductionAlgorithm,
  class Representation     = defaults::Representation,
  class DataType
> PersistenceDiagram<DataType> calculatePersistenceDiagram( const topology::BoundaryMatrix<Representation>& boundaryMatrix,
                                                            const std::vector<DataType>& functionValues )
{
  auto pairing = calculatePersistencePairing<ReductionAlgorithm>( boundaryMatrix );
  return makePersistenceDiagram( pairing, functionValues );
}

}

#endif
