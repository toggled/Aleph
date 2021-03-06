#ifndef ALEPH_GEOMETRY_NEAREST_NEIGHBOURS_HH__
#define ALEPH_GEOMETRY_NEAREST_NEIGHBOURS_HH__

#include <vector>

namespace aleph
{

namespace geometry
{

template <class Wrapper, class ElementType, class IndexType> class NearestNeighbours
{
public:
  void radiusSearch( ElementType radius,
                     std::vector< std::vector<IndexType> >& indices,
                     std::vector< std::vector<ElementType> >& distances )
  {
    static_cast<const Wrapper&>( *this ).radiusSearch( radius,
                                                       indices,
                                                       distances );
  }

  void neighbourSearch( unsigned k,
                        std::vector< std::vector<IndexType> >& indices,
                        std::vector< std::vector<ElementType> >& distances )
  {
    static_cast<const Wrapper&>( *this ).neighbourSearch( k,
                                                          indices,
                                                          distances );
  }

  std::size_t size() const noexcept
  {
    return static_cast<const Wrapper&>( *this ).size();
  }
};

}

}

#endif
