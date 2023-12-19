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
#ifndef itkMeanSquaresPointSetToImageMetric_h
#define itkMeanSquaresPointSetToImageMetric_h

#include "itkPointSetToImageMetric.h"
#include "itkCovariantVector.h"
#include "itkPoint.h"

namespace itk
{
/** \class MeanSquaresPointSetToImageMetric
 * \brief Computes similarity between pixel values of a point set and
 * intensity values of an image.
 *
 * This metric computes the average squared differences between pixels
 * in the point set and the transformed set of pixels.
 *
 * Spatial correspondence between both images is established through a
 * Transform.
 *
 * \ingroup RegistrationMetrics
 * \ingroup ITKRegistrationCommon
 */
template <typename TFixedPointSet, typename TMovingImage>
class ITK_TEMPLATE_EXPORT MeanSquaresPointSetToImageMetric : public PointSetToImageMetric<TFixedPointSet, TMovingImage>
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(MeanSquaresPointSetToImageMetric);

  /** Standard class type aliases. */
  using Self = MeanSquaresPointSetToImageMetric;
  using Superclass = PointSetToImageMetric<TFixedPointSet, TMovingImage>;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkOverrideGetNameOfClassMacro(MeanSquaresPointSetToImageMetric);

  /** Types transferred from the base class */
  using typename Superclass::RealType;
  using typename Superclass::TransformType;
  using typename Superclass::TransformPointer;
  using typename Superclass::TransformParametersType;
  using typename Superclass::TransformJacobianType;
  using typename Superclass::GradientPixelType;
  using typename Superclass::InputPointType;
  using typename Superclass::OutputPointType;

  using typename Superclass::MeasureType;
  using typename Superclass::DerivativeType;
  using typename Superclass::FixedPointSetType;
  using typename Superclass::MovingImageType;
  using typename Superclass::FixedPointSetConstPointer;
  using typename Superclass::MovingImageConstPointer;

  using typename Superclass::PointIterator;
  using typename Superclass::PointDataIterator;

  /** Get the derivatives of the match measure. */
  void
  GetDerivative(const TransformParametersType & parameters, DerivativeType & derivative) const override;

  /**  Get the value for single valued optimizers. */
  MeasureType
  GetValue(const TransformParametersType & parameters) const override;

  /**  Get value and derivatives for multiple valued optimizers. */
  void
  GetValueAndDerivative(const TransformParametersType & parameters,
                        MeasureType &                   value,
                        DerivativeType &                derivative) const override;

protected:
  MeanSquaresPointSetToImageMetric() = default;
  ~MeanSquaresPointSetToImageMetric() override = default;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkMeanSquaresPointSetToImageMetric.hxx"
#endif

#endif
