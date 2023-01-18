/*=========================================================================
 *
 *  Copyright NumFOCUS
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         https://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkGrayscaleMorphologicalOpeningImageFilter_h
#define itkGrayscaleMorphologicalOpeningImageFilter_h

#include "itkKernelImageFilter.h"
#include "itkMathematicalMorphologyEnums.h"
#include "itkMovingHistogramDilateImageFilter.h"
#include "itkMovingHistogramErodeImageFilter.h"
#include "itkBasicDilateImageFilter.h"
#include "itkBasicErodeImageFilter.h"
#include "itkAnchorOpenImageFilter.h"
#include "itkVanHerkGilWermanErodeImageFilter.h"
#include "itkVanHerkGilWermanDilateImageFilter.h"
#include "itkConstantBoundaryCondition.h"
#include "itkNeighborhood.h"

namespace itk
{
/**
 * \class GrayscaleMorphologicalOpeningImageFilter
 * \brief Grayscale opening of an image.
 *
 * Open an image using grayscale morphology.
 *
 * The structuring element is assumed to be composed of binary
 * values (zero or one). Only elements of the structuring element
 * having values > 0 are candidates for affecting the center pixel.
 *
 * \sa MorphologyImageFilter, GrayscaleFunctionDilateImageFilter, BinaryDilateImageFilter
 * \ingroup ImageEnhancement  MathematicalMorphologyImageFilters
 * \ingroup ITKMathematicalMorphology
 */

template <typename TInputImage, typename TOutputImage, typename TKernel>
class ITK_TEMPLATE_EXPORT GrayscaleMorphologicalOpeningImageFilter
  : public KernelImageFilter<TInputImage, TOutputImage, TKernel>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(GrayscaleMorphologicalOpeningImageFilter);

  /** Standard class type aliases. */
  using Self = GrayscaleMorphologicalOpeningImageFilter;
  using Superclass = KernelImageFilter<TInputImage, TOutputImage, TKernel>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Standard New method. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(GrayscaleMorphologicalOpeningImageFilter, KernelImageFilter);

  /** Image related type alias. */
  static constexpr unsigned int ImageDimension = TInputImage::ImageDimension;

  /** Image related type alias. */
  using InputImageType = TInputImage;
  using OutputImageType = TOutputImage;
  using RegionType = typename TInputImage::RegionType;
  using SizeType = typename TInputImage::SizeType;
  using IndexType = typename TInputImage::IndexType;
  using PixelType = typename TInputImage::PixelType;
  using OffsetType = typename TInputImage::OffsetType;
  using typename Superclass::OutputImageRegionType;

  using FlatKernelType = FlatStructuringElement<(Self::ImageDimension)>;
  using HistogramDilateFilterType = MovingHistogramDilateImageFilter<TInputImage, TOutputImage, TKernel>;
  using HistogramErodeFilterType = MovingHistogramErodeImageFilter<TInputImage, TOutputImage, TKernel>;
  using BasicErodeFilterType = BasicErodeImageFilter<TInputImage, TInputImage, TKernel>;
  using BasicDilateFilterType = BasicDilateImageFilter<TInputImage, TOutputImage, TKernel>;
  using AnchorFilterType = AnchorOpenImageFilter<TInputImage, FlatKernelType>;
  using VanHerkGilWermanErodeFilterType = VanHerkGilWermanErodeImageFilter<TInputImage, FlatKernelType>;
  using VanHerkGilWermanDilateFilterType = VanHerkGilWermanDilateImageFilter<TInputImage, FlatKernelType>;
  using SubtractFilterType = CastImageFilter<TInputImage, TOutputImage>;

  /** Kernel type alias. */
  using KernelType = TKernel;
  //   using KernelSuperclass = typename KernelType::Superclass;
  //   using KernelSuperclass = Neighborhood< typename KernelType::PixelType, ImageDimension >;

  using AlgorithmEnum = MathematicalMorphologyEnums::Algorithm;

#if !defined(ITK_LEGACY_REMOVE)
  /** Backwards compatibility for enum values */
  using AlgorithmType = AlgorithmEnum;
  // We need to expose the enum values at the class level
  // for backwards compatibility
  static constexpr AlgorithmType BASIC = AlgorithmEnum::BASIC;
  static constexpr AlgorithmType HISTO = AlgorithmEnum::HISTO;
  static constexpr AlgorithmType ANCHOR = AlgorithmEnum::ANCHOR;
  static constexpr AlgorithmType VHGW = AlgorithmEnum::VHGW;
#endif

  /** Set kernel (structuring element). */
  void
  SetKernel(const KernelType & kernel) override;

  /** Set/Get the backend filter class. */
  void
  SetAlgorithm(AlgorithmEnum algo);
  itkGetConstMacro(Algorithm, AlgorithmEnum);

  /** GrayscaleMorphologicalOpeningImageFilter need to set its internal filters
    as modified */
  void
  Modified() const override;

  /** A safe border is added to input image to avoid borders effects
   * and remove it once the closing is done */
  itkSetMacro(SafeBorder, bool);
  itkGetConstReferenceMacro(SafeBorder, bool);
  itkBooleanMacro(SafeBorder);

protected:
  GrayscaleMorphologicalOpeningImageFilter();
  ~GrayscaleMorphologicalOpeningImageFilter() override = default;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  void
  GenerateData() override;

private:
  // the filters used internally
  typename HistogramDilateFilterType::Pointer m_HistogramDilateFilter{};

  typename HistogramErodeFilterType::Pointer m_HistogramErodeFilter{};

  typename BasicDilateFilterType::Pointer m_BasicDilateFilter{};

  typename BasicErodeFilterType::Pointer m_BasicErodeFilter{};

  typename VanHerkGilWermanDilateFilterType::Pointer m_VanHerkGilWermanDilateFilter{};

  typename VanHerkGilWermanErodeFilterType::Pointer m_VanHerkGilWermanErodeFilter{};

  typename AnchorFilterType::Pointer m_AnchorFilter{};

  // and the name of the filter
  AlgorithmEnum m_Algorithm{ AlgorithmEnum::HISTO };

  bool m_SafeBorder{ true };
}; // end of class
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkGrayscaleMorphologicalOpeningImageFilter.hxx"
#endif

#endif
