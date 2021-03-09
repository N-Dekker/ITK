/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#ifndef itkRegionalConstantBoundaryImageNeighborhoodPixelAccessPolicy_h
#define itkRegionalConstantBoundaryImageNeighborhoodPixelAccessPolicy_h

#include "itkIndex.h"
#include "itkOffset.h"
#include "itkSize.h"

namespace itk
{

/**
 * \class RegionalConstantBoundaryImageNeighborhoodPixelAccessPolicy
 * ImageNeighborhoodPixelAccessPolicy class for ShapedImageNeighborhoodRange.
 * Allows getting and setting the value of a pixel, located in a specified
 * neighborhood location, at a specified offset. Uses a constant as value
 * for pixels outside the region border.
 *
 * \see ShapedNeighborhoodIterator
 * \see ConstantBoundaryCondition
 * \see ConstantBoundaryImageNeighborhoodPixelAccessPolicy
 * \ingroup ImageIterators
 * \ingroup ITKCommon
 */
template <typename TImage>
class RegionalConstantBoundaryImageNeighborhoodPixelAccessPolicy final
{
private:
  using NeighborhoodAccessorFunctorType = typename TImage::NeighborhoodAccessorFunctorType;
  using PixelType = typename TImage::PixelType;
  using InternalPixelType = typename TImage::InternalPixelType;

  using ImageDimensionType = typename TImage::ImageDimensionType;
  static constexpr ImageDimensionType ImageDimension = TImage::ImageDimension;

  using IndexType = Index<ImageDimension>;
  using OffsetType = Offset<ImageDimension>;
  using ImageSizeType = Size<ImageDimension>;

  // Index value to the image buffer, indexing the current pixel. -1 is used to indicate out-of-bounds.
  const IndexValueType m_PixelIndexValue;

  // A reference to the accessor of the image.
  const NeighborhoodAccessorFunctorType & m_NeighborhoodAccessor;

  // The constant whose value is returned a pixel value outside the region is queried.
  const PixelType m_Constant;


  // Private helper function. Tells whether the pixel at 'pixelIndex' is inside the region.
  static bool
  IsInside(const IndexType & pixelIndex, const IndexType & regionIndex, const ImageSizeType & regionSize) ITK_NOEXCEPT
  {
    bool result = true;

    for (ImageDimensionType i = 0; i < ImageDimension; ++i)
    {
      const IndexValueType indexValue = pixelIndex[i];
      const IndexValueType regionIndexValue = regionIndex[i];

      // Note: Do not 'quickly' break or return out of the for-loop when the
      // result is false! For performance reasons (loop unrolling, etc.) it
      // appears preferable to complete the for-loop iteration in this case!
      result = result && (indexValue >= regionIndexValue) &&
               (static_cast<SizeValueType>(indexValue - regionIndexValue) < regionSize[i]);
    }
    return result;
  }


  // Private helper function. Calculates and returns the index value of the
  // current pixel within the image buffer.
  static IndexValueType
  CalculatePixelIndexValue(const OffsetType & offsetTable, const IndexType & pixelIndex) ITK_NOEXCEPT
  {
    IndexValueType result = 0;

    for (ImageDimensionType i = 0; i < ImageDimension; ++i)
    {
      result += pixelIndex[i] * offsetTable[i];
    }
    return result;
  }

public:
  /** This type is necessary to tell the ShapedImageNeighborhoodRange that the
   * constructor accepts a pixel value as (optional) extra parameter. */
  struct PixelAccessParameterType
  {
    IndexType     RegionIndex;
    ImageSizeType RegionSize;
    PixelType     Constant;
  };

  // Deleted member functions:
  RegionalConstantBoundaryImageNeighborhoodPixelAccessPolicy() = delete;
  RegionalConstantBoundaryImageNeighborhoodPixelAccessPolicy &
  operator=(const RegionalConstantBoundaryImageNeighborhoodPixelAccessPolicy &) = delete;

  // Explicitly-defaulted functions:
  ~RegionalConstantBoundaryImageNeighborhoodPixelAccessPolicy() = default;
  RegionalConstantBoundaryImageNeighborhoodPixelAccessPolicy(
    const RegionalConstantBoundaryImageNeighborhoodPixelAccessPolicy &) = default;

  /** Constructor called directly by the pixel proxy of
   * ShapedImageNeighborhoodRange. */
  RegionalConstantBoundaryImageNeighborhoodPixelAccessPolicy(
    const ImageSizeType &                   itkNotUsed(imageSize),
    const OffsetType &                      offsetTable,
    const NeighborhoodAccessorFunctorType & neighborhoodAccessor,
    const IndexType &                       pixelIndex,
    const PixelAccessParameterType          pixelAccessParameter) ITK_NOEXCEPT
    : m_PixelIndexValue{ IsInside(pixelIndex, pixelAccessParameter.RegionIndex, pixelAccessParameter.RegionSize)
                           ? CalculatePixelIndexValue(offsetTable, pixelIndex)
                           : -1 }
    , m_NeighborhoodAccessor(neighborhoodAccessor)
    , m_Constant{ pixelAccessParameter.Constant }
  {}


  /** Retrieves the pixel value from the image buffer, at the current
   * index. When the index is out of bounds, it returns the constant
   * value specified during construction. */
  PixelType
  GetPixelValue(const InternalPixelType * const imageBufferPointer) const ITK_NOEXCEPT
  {
    return (m_PixelIndexValue < 0) ? m_Constant : m_NeighborhoodAccessor.Get(imageBufferPointer + m_PixelIndexValue);
  }

  /** Sets the value of the image buffer at the current index value to the
   * specified value.  */
  void
  SetPixelValue(InternalPixelType * const imageBufferPointer, const PixelType & pixelValue) const ITK_NOEXCEPT
  {
    if (m_PixelIndexValue >= 0)
    {
      m_NeighborhoodAccessor.Set(imageBufferPointer + m_PixelIndexValue, pixelValue);
    }
  }
};


} // namespace itk

#endif
