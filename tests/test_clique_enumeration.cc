#include <tests/Base.hh>

#include <aleph/geometry/RipsExpanderTopDown.hh>

#include <aleph/topology/MaximalCliques.hh>

#include <aleph/topology/Simplex.hh>
#include <aleph/topology/SimplicialComplex.hh>

#include <aleph/topology/filtrations/Data.hh>

#include <algorithm>
#include <vector>

using namespace aleph::topology;
using namespace aleph;

template <class Data, class Vertex> void triangles()
{
  ALEPH_TEST_BEGIN( "Triangles [connected & unconnected]" );

  using Simplex           = Simplex<Data, Vertex>;
  using SimplicialComplex = SimplicialComplex<Simplex>;

  // 2---1
  // |  /|
  // | / |
  // |/  |
  // 0---3
  //
  // Expected cliques: {0,1,2}, {0,3,1}
  std::vector<Simplex> trianglesConnected
    = {
        {0}, {1}, {2}, {3},
        {0,1}, {0,2}, {0,3}, {1,2}, {1,3},
        {0,1,2}, {0,1,3}
    };

  // 2---1   5
  // |  /   /|
  // | /   / |
  // |/   /  |
  // 0---3---4
  //
  // Expected cliques: {0,3}, {0,1,2}, {3,4,5}
  std::vector<Simplex> trianglesDisconnected
    = {
        {0}, {1}, {2}, {3}, {4}, {5},
        {0,1}, {0,2}, {0,3}, {1,2}, {3,4}, {3,5}, {4,5},
        {0,1,2}, {3,4,5}
    };

  SimplicialComplex K1( trianglesConnected.begin()   , trianglesConnected.end() );
  SimplicialComplex K2( trianglesDisconnected.begin(), trianglesDisconnected.end() );

  auto C11 = maximalCliquesBronKerbosch( K1 );
  auto C12 = maximalCliquesKoch( K1 );
  auto C21 = maximalCliquesBronKerbosch( K2 );
  auto C22 = maximalCliquesKoch( K2 );

  ALEPH_ASSERT_THROW( C11.empty() == false );
  ALEPH_ASSERT_THROW( C12.empty() == false );
  ALEPH_ASSERT_THROW( C21.empty() == false );
  ALEPH_ASSERT_THROW( C22.empty() == false );

  ALEPH_ASSERT_EQUAL( C11.size(), C12.size() );
  ALEPH_ASSERT_EQUAL( C21.size(), C22.size() );

  ALEPH_ASSERT_EQUAL( C11.size(), 2 );
  ALEPH_ASSERT_EQUAL( C21.size(), 3 );

  ALEPH_ASSERT_THROW( std::find( C11.begin(), C11.end(), std::set<Vertex>( {0,1,2} ) ) != C11.end() );
  ALEPH_ASSERT_THROW( std::find( C11.begin(), C11.end(), std::set<Vertex>( {0,1,3} ) ) != C11.end() );
  ALEPH_ASSERT_THROW( std::find( C12.begin(), C12.end(), std::set<Vertex>( {0,1,2} ) ) != C12.end() );
  ALEPH_ASSERT_THROW( std::find( C12.begin(), C12.end(), std::set<Vertex>( {0,1,3} ) ) != C12.end() );

  ALEPH_ASSERT_THROW( std::find( C21.begin(), C21.end(), std::set<Vertex>( {0,3  } ) ) != C21.end() );
  ALEPH_ASSERT_THROW( std::find( C21.begin(), C21.end(), std::set<Vertex>( {0,1,2} ) ) != C21.end() );
  ALEPH_ASSERT_THROW( std::find( C21.begin(), C21.end(), std::set<Vertex>( {3,4,5} ) ) != C21.end() );
  ALEPH_ASSERT_THROW( std::find( C22.begin(), C22.end(), std::set<Vertex>( {0,3  } ) ) != C22.end() );
  ALEPH_ASSERT_THROW( std::find( C22.begin(), C22.end(), std::set<Vertex>( {0,1,2} ) ) != C22.end() );
  ALEPH_ASSERT_THROW( std::find( C22.begin(), C22.end(), std::set<Vertex>( {3,4,5} ) ) != C22.end() );

  aleph::geometry::RipsExpanderTopDown<SimplicialComplex> expander;

  auto expandedK1 = expander( K1, 3 );
  auto expandedK2 = expander( K2, 3 );

  expandedK1 = expander.assignMaximumWeight( expandedK1, K1 );
  expandedK2 = expander.assignMaximumWeight( expandedK2, K2 );

  expandedK1.sort( aleph::topology::filtrations::Data<Simplex>() );
  expandedK2.sort( aleph::topology::filtrations::Data<Simplex>() );

  ALEPH_ASSERT_THROW( expandedK1.empty() == false );
  ALEPH_ASSERT_THROW( expandedK2.empty() == false );

  ALEPH_ASSERT_THROW( expandedK1 != expandedK2 );

  ALEPH_TEST_END();
}

template <class Data, class Vertex> void trianglesNonZeroBasedIndices()
{
  ALEPH_TEST_BEGIN( "Triangles with non-zero-based indices" );

  using Simplex           = Simplex<Data, Vertex>;
  using SimplicialComplex = SimplicialComplex<Simplex>;

  // 3---2
  // |  /|
  // | / |
  // |/  |
  // 1---4
  //
  // Expected cliques: {1,2,3}, {1,4,2}
  std::vector<Simplex> triangles
    = {
        {1}, {2}, {3}, {4},
        {1,2}, {1,3}, {1,4}, {2,3}, {2,4},
        {1,2,3}, {1,2,4}
    };

  SimplicialComplex K( triangles.begin(), triangles.end() );

  auto C1 = maximalCliquesBronKerbosch( K );
  auto C2 = maximalCliquesKoch( K );

  ALEPH_ASSERT_THROW( C1.empty() == false );
  ALEPH_ASSERT_THROW( C2.empty() == false );

  ALEPH_ASSERT_EQUAL( C1.size(), C2.size() );
  ALEPH_ASSERT_EQUAL( C1.size(), 2 );

  ALEPH_ASSERT_THROW( std::find( C1.begin(), C1.end(), std::set<Vertex>( {1,2,3} ) ) != C1.end() );
  ALEPH_ASSERT_THROW( std::find( C1.begin(), C1.end(), std::set<Vertex>( {1,2,4} ) ) != C1.end() );
  ALEPH_ASSERT_THROW( std::find( C2.begin(), C2.end(), std::set<Vertex>( {1,2,3} ) ) != C2.end() );
  ALEPH_ASSERT_THROW( std::find( C2.begin(), C2.end(), std::set<Vertex>( {1,2,4} ) ) != C2.end() );

  aleph::geometry::RipsExpanderTopDown<SimplicialComplex> expander;

  auto L = expander( K, 3 );
  L      = expander.assignMaximumWeight( L, K );

  L.sort( aleph::topology::filtrations::Data<Simplex>() );

  ALEPH_ASSERT_THROW( L.empty() == false );

  ALEPH_TEST_END();
}



int main()
{
  triangles<double, unsigned>();
  triangles<float,  unsigned>();

  trianglesNonZeroBasedIndices<double, unsigned>();
  trianglesNonZeroBasedIndices<float,  unsigned>();
}
