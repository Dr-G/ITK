/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    itkNeighborhoodIterator.h
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#ifndef __itkNeighborhoodIterator_h
#define __itkNeighborhoodIterator_h

#include <vector>
#include <string.h>
#include <iostream>
#include "itkConstNeighborhoodIterator.h"

namespace itk {

/**
 * \class NeighborhoodIterator
 * \brief  Defines iteration of a local N-dimensional neighborhood of pixels
 * across an itk::Image.
 *

 I. What NeighborhoodIterators are.

 II. What NeighborhoodIterators can be used for.

 III. How NeighborhoodIterators are used.

 IV. How NeighborhoodIterators are implemented.

 V. Examples?

 I. What are NeighborhoodIterators?
 
 This class is an extension of the Standard Template Library bi-directional
 iterator concept to neighborhoods of pixels within itk::Image objects.  The
 class allows simple forward and reverse iteration of a N-dimensional 
 neighborhood "mask" across an image.  A pixel neighborhood is defined as
 a central pixel location and an N-dimensional radius extending from that
 location.  For iteration, a neighborhood mask is
 constructed as a container of pointers to a neighborhood of image pixels.  As
 the central pixel position in the mask is moved around the image, the
 neighboring pixel pointers are moved accordingly. 
 
 NeighborhoodIterators are "bidirectional iterators". They move only in two
 directions through the data set.  These directions correspond to the layout of
 the image data in memory and not to the spatial directions of the
 N-dimensional itk::Image.  Iteration always proceeds along the
 fastest increasing dimension (as defined by the layout of the image data) .
 For itk::Image this is the first dimension specified (i.e. for 3-dimensional
 (x,y,z) NeighborhoodIterator proceeds along the x-dimension)  

 For true, random access iteration within an itk::Image, use
 RandomAccessNeighborhoodIterator. 

 II. What are NeighborhoodIterators used for?

 NeighborhoodIterators can be used to simplify writing algorithms that
 perform local image processing.  Convolution filtering and morphological
 operations on images are two typical use cases.  
 

*/
template<class TImage>
class ITK_EXPORT NeighborhoodIterator
  :  public ConstNeighborhoodIterator<TImage>
{
public:
  /** 
   * Standard "Self" & Superclass typedef support.
   */
  typedef NeighborhoodIterator Self;
  typedef ConstNeighborhoodIterator<TImage> Superclass;

  /**
   * Extract typedefs from superclass
   */
  typedef typename Superclass::InternalPixelType InternalPixelType;
  typedef typename Superclass::PixelType  PixelType;
  typedef typename Superclass::SizeType   SizeType;
  typedef typename Superclass::ImageType  ImageType;
  typedef typename Superclass::RegionType RegionType;
  typedef typename Superclass::IndexType  IndexType;
  typedef typename Superclass::OffsetType OffsetType;
  typedef typename Superclass::RadiusType RadiusType;
  typedef typename Superclass::NeighborhoodType NeighborhoodType;
  enum {Dimension = Superclass::Dimension };
  typedef typename Superclass::Iterator      Iterator;
  typedef typename Superclass::ConstIterator ConstIterator;
  typedef typename Superclass::ImageBoundaryConditionPointerType
   ImageBoundaryConditionPointerType;
  typedef typename Superclass::ScalarValueType ScalarValueType;
  
  /**
   * Default constructor.
   */
  NeighborhoodIterator(): Superclass() {}
  
  /**
   * Copy constructor
   */
  NeighborhoodIterator( const NeighborhoodIterator &n )
    : Superclass(n) {}
  
  /**
   * Assignment operator
   */
  Self &operator=(const Self& orig)
    {
      Superclass::operator=(orig);
      return *this;
    }
  
  /**
   * Constructor which establishes the region size, neighborhood, and image
   * over which to walk.
   */
  NeighborhoodIterator(const SizeType &radius,
                       ImageType * ptr,
                       const RegionType &region
                       )
    : Superclass(radius, ptr, region)
    { }

  /**
   * Standard print method
   */
  virtual void PrintSelf(std::ostream &, Indent) const;

  /**
   * Returns the central memory pointer of the neighborhood.
   */
  InternalPixelType *GetCenterPointer()
    {    return (this->operator[]((this->Size())>>1));  }


  virtual void SetCenterPixel(const PixelType &p)
    {    *( this->GetCenterPointer() ) = p;  }
  
  /**
   * Virtual function that replaces the pixel values in the image
   * neighborhood that are pointed to by this NeighborhoodIterator with
   * the pixel values contained in a Neighborhood.
   */
  virtual void SetNeighborhood(const NeighborhoodType &);

  /**
   *
   */
  virtual void SetPixel(const unsigned long i, const PixelType &v)
    { *(this->operator[](i)) = v; }
    
};

} // namespace itk


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkNeighborhoodIterator.txx"
#endif

#endif
