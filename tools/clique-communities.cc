#include <algorithm>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <string>
#include <sstream>

#include <cmath>

#include "filtrations/Data.hh"

#include "geometry/RipsExpander.hh"

#include "persistentHomology/ConnectedComponents.hh"

#include "topology/CliqueGraph.hh"
#include "topology/ConnectedComponents.hh"
#include "topology/Simplex.hh"
#include "topology/SimplicialComplex.hh"

#include "topology/io/EdgeLists.hh"
#include "topology/io/GML.hh"

#include "utilities/Filesystem.hh"

using DataType           = double;
using VertexType         = unsigned;
using Simplex            = aleph::topology::Simplex<DataType, VertexType>;
using SimplicialComplex  = aleph::topology::SimplicialComplex<Simplex>;

template <class Simplex> std::string formatSimplex( const Simplex& s )
{
  std::ostringstream stream;
  stream << "{";

  for( auto it = s.begin(); it != s.end(); ++it )
  {
    if( it != s.begin() )
      stream << ",";
    stream << *it;
  }

  stream << "}";

  return stream.str();
}

void usage()
{
  std::cerr << "Usage: clique_communities FILE THRESHOLD K\n"
            << "\n"
            << "Extracts clique communities from FILE, which is supposed to be\n"
            << "a weighted graph. In the subsequent calculation, an edge whose\n"
            << "weight is larger than THRESHOLD will be ignored. K denotes the\n"
            << "maximum dimension of a simplex for the clique graph extraction\n"
            << "and the clique community calculation. This does not correspond\n"
            << "to the dimensionality of the clique. Hence, a parameter of K=2\n"
            << "will result in calculating 3-clique communities because all of\n"
            << "the 2-simplices have 3 vertices.\n\n";
}

int main( int argc, char** argv )
{
  if( argc <= 3 )
  {
    usage();
    return -1;
  }

  std::string filename = argv[1];
  double threshold     = std::stod( argv[2] );
  unsigned maxK        = static_cast<unsigned>( std::stoul( argv[3] ) );

  SimplicialComplex K;

  // Input -------------------------------------------------------------
  //
  // TODO: This is copied from the clique persistence diagram
  // calculation. It would make sense to share some functions
  // between the two applications.

  std::cerr << "* Reading '" << filename << "'...";

  if( aleph::utilities::extension( filename ) == ".gml" )
  {
    aleph::topology::io::GMLReader reader;
    reader( filename, K );
  }
  else
  {
    aleph::io::EdgeListReader reader;
    reader.setReadWeights( true );
    reader.setTrimLines( true );

    reader( filename, K );
  }

  std::cerr << "finished\n";

  // Thresholding ------------------------------------------------------

  {
    std::cerr << "* Filtering input data to threshold epsilon=" << threshold << "...";

    std::vector<Simplex> simplices;

    std::remove_copy_if( K.begin(), K.end(), std::back_inserter( simplices ),
                         [&threshold] ( const Simplex& s )
                         {
                           return s.data() > threshold;
                         } );

    K = SimplicialComplex( simplices.begin(), simplices.end() );

    std::cerr << "finished\n";
  }

  // Expansion ---------------------------------------------------------

  aleph::geometry::RipsExpander<SimplicialComplex> ripsExpander;
  K = ripsExpander( K, maxK );
  K = ripsExpander.assignMaximumWeight( K );

  K.sort( aleph::filtrations::Data<Simplex>() );

  for( unsigned k = 1; k <= maxK; k++ )
  {
    std::cerr << "* Extracting " << k << "-cliques graph...";

    auto C
        = aleph::topology::getCliqueGraph( K, k );

    C.sort( aleph::filtrations::Data<Simplex>() );

    std::cerr << "finished\n";

    std::cerr << "* " << k << "-cliques graph has " << C.size() << " simplices\n";

    auto uf = aleph::topology::calculateConnectedComponents( C );

    std::set<VertexType> roots;
    uf.roots( std::inserter( roots, roots.begin() ) );

    std::cerr << "* " << k << "-cliques graph has " << roots.size() << " connected components\n";

    for( auto&& root : roots )
    {
      // The vertex IDs stored in the Union--Find data structure
      // correspond to the indices of the simplicial complex. It
      // thus suffices to map them back.
      std::set<VertexType> vertices;
      uf.get( root, std::inserter( vertices, vertices.begin() ) );

      std::vector<Simplex> simplices;

      std::transform( vertices.begin(), vertices.end(), std::back_inserter( simplices ),
                      [&K] ( VertexType v )
                      {
                        return K.at(v);
                      } );

      std::sort( simplices.begin(), simplices.end() );

      std::cout << "[";

      for( auto it = simplices.begin(); it != simplices.end(); ++it )
      {
        if( it != simplices.begin() )
          std::cout << ",";

        std::cout << formatSimplex( *it );
      }

      std::cout << "]\n";
    }

    std::cout << "\n\n";
  }
}