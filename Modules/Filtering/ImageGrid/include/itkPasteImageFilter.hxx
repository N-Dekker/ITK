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
#ifndef itkPasteImageFilter_hxx
#define itkPasteImageFilter_hxx

#include "itkObjectFactory.h"
#include "itkTotalProgressReporter.h"
#include "itkImageAlgorithm.h"

namespace itk
{


template <typename TInputImage, typename TSourceImage, typename TOutputImage>
PasteImageFilter<TInputImage, TSourceImage, TOutputImage>::PasteImageFilter()
{
  // #0 "FixedImage" required
  Self::SetPrimaryInputName("DestinationImage");

  // "SourceImage"
  Self::AddOptionalInputName("SourceImage");
  Self::AddOptionalInputName("Constant");

  this->m_DestinationIndex.Fill(0);
  for (unsigned int i = 0; i < InputImageDimension; ++i)
  {
    this->m_DestinationSkipAxes[i] = (i >= SourceImageDimension);
  }

  this->InPlaceOff();
  this->DynamicMultiThreadingOn();
  this->ThreaderUpdateProgressOff();
}


template <typename TInputImage, typename TSourceImage, typename TOutputImage>
void
PasteImageFilter<TInputImage, TSourceImage, TOutputImage>::GenerateInputRequestedRegion()
{
  // Call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();

  // Get the pointers for the inputs and output
  const InputImagePointer destPtr = const_cast<InputImageType *>(this->GetInput());
  const SourceImagePointer sourcePtr = const_cast<SourceImageType *>(this->GetSourceImage());
  const OutputImagePointer outputPtr = this->GetOutput();

  if (!destPtr || !outputPtr)
  {
    return;
  }

  if (sourcePtr)
  {
    // Second input must include the SourceRegion
    sourcePtr->SetRequestedRegion(m_SourceRegion);
  }

  // First input must match the output requested region
  destPtr->SetRequestedRegion(outputPtr->GetRequestedRegion());
}


template <typename TInputImage, typename TSourceImage, typename TOutputImage>
void
PasteImageFilter<TInputImage, TSourceImage, TOutputImage>::VerifyPreconditions() const
{
  Superclass::VerifyPreconditions();


  auto sourceInput = this->GetSourceImage();
  auto constantInput = this->GetConstantInput();

  if (sourceInput == nullptr && constantInput == nullptr)
  {
    itkExceptionMacro("The Source or the Constant input are required.");
  }


  auto numberSkippedAxis = std::accumulate(m_DestinationSkipAxes.begin(), m_DestinationSkipAxes.end(), 0);
  if (numberSkippedAxis != (InputImageDimension - SourceImageDimension))
  {
    itkExceptionMacro("Number of skipped axes " << m_DestinationSkipAxes << " does not match the difference in"
                                                << " destination and source image dimensions.");
  }
}


template <typename TInputImage, typename TSourceImage, typename TOutputImage>
bool
PasteImageFilter<TInputImage, TSourceImage, TOutputImage>::CanRunInPlace() const
{
  auto destinationImage = static_cast<const DataObject *>(this->GetDestinationImage());
  auto sourceImage = static_cast<const DataObject *>(this->GetSourceImage());

  return (destinationImage != sourceImage) && Superclass::CanRunInPlace();
}

template <typename TInputImage, typename TSourceImage, typename TOutputImage>
void
PasteImageFilter<TInputImage, TSourceImage, TOutputImage>::DynamicThreadedGenerateData(
  const OutputImageRegionType & outputRegionForThread)
{
  // Get the input and output pointers
  const InputImageType * destPtr = this->GetInput();
  const SourceImageType * sourcePtr = this->GetSourceImage();
  OutputImageType * outputPtr = this->GetOutput();

  TotalProgressReporter progress(this, outputPtr->GetRequestedRegion().GetNumberOfPixels());

  // What is the region on the destination image would be overwritten by the
  // source?
  // Do we need to use the source image at all for the region generated by this
  // thread?

  const InputImageSizeType destinationSize = this->GetPresumedDestinationSize();

  InputImageRegionType destinationRegion(this->GetDestinationIndex(), destinationSize);

  // if there is not intersection then the source is not needed
  const bool useSource = destinationRegion.Crop(outputRegionForThread);

  // If the source image needs to be used to generate the output image, does the
  // destination image need to be used? i.e. will the source region completely
  // overlap the destination region for this thread?
  const bool useOnlySource = useSource && (destinationRegion == outputRegionForThread);

  // If the source needs to be used, what part of the source needs to copied
  // by this thread?
  SourceImageRegionType sourceRegionInSourceImageCropped;
  if (useSource)
  {
    // What is the proposed shift from destination to source?
    Offset<SourceImageDimension> originalOffsetFromDestinationToSource;

    for (unsigned int d = 0, s = 0; s < SourceImageDimension; ++d, ++s)
    {
      while (m_DestinationSkipAxes[d])
      {
        ++d;
        assert(d < InputImageDimension);
      }
      originalOffsetFromDestinationToSource[s] = m_SourceRegion.GetIndex()[s] - m_DestinationIndex[d];
    }

    // Transform the cropped index back into the source image
    SourceImageIndexType sourceIndexInSourceImageCropped;
    SourceImageSizeType sourceSizeCropped;

    for (unsigned int d = 0, s = 0; s < SourceImageDimension; ++d, ++s)
    {
      while (m_DestinationSkipAxes[d])
      {
        ++d;
        assert(d < InputImageDimension);
      }
      sourceIndexInSourceImageCropped[s] = destinationRegion.GetIndex()[d] + originalOffsetFromDestinationToSource[s];
      sourceSizeCropped[s] = destinationRegion.GetSize()[d];
    }

    // Set the values in the region
    sourceRegionInSourceImageCropped.SetIndex(sourceIndexInSourceImageCropped);
    sourceRegionInSourceImageCropped.SetSize(sourceSizeCropped);
  }


  // There are three cases that we need to consider:
  //
  // 1. Source region does not impact this thread, so copy data from
  //    from the destination image to the output
  //
  // 2. Source region completely overlaps the output region for this
  //    thread, so copy data from the source image to the output
  //
  // 3. Source region partially overlaps the output region for this
  //    thread, so copy data as needed from both the source and
  //    destination.
  //
  if (!useSource && !(this->GetInPlace() && this->CanRunInPlace()))
  {
    // Paste region is outside this thread, so just copy the destination
    // input to the output
    ImageAlgorithm::Copy(destPtr, outputPtr, outputRegionForThread, outputRegionForThread);
    progress.Completed(outputRegionForThread.GetNumberOfPixels());
  }
  else if (useOnlySource)
  {

    if (sourcePtr)
    {
      // Paste region completely overlaps the output region
      // for this thread, so copy data from the second input
      // to the output
      ImageAlgorithm::Copy(sourcePtr, outputPtr, sourceRegionInSourceImageCropped, outputRegionForThread);
      progress.Completed(outputRegionForThread.GetNumberOfPixels());
    }
    else
    {
      const SourceImagePixelType sourceValue = this->GetConstant();

      for (ImageScanlineIterator outputIt(outputPtr, destinationRegion); !outputIt.IsAtEnd(); outputIt.NextLine())
      {
        while (!outputIt.IsAtEndOfLine())
        {
          outputIt.Set(sourceValue);
          ++outputIt;
        }
        progress.Completed(outputRegionForThread.GetSize()[0]);
      }
    }
  }
  else
  {
    // Paste region partially overlaps the output region for the
    // thread, so we need copy data from both inputs as necessary. The
    // following code could be optimized. This case could be
    // decomposed further such the output is broken into a set of
    // regions where each region would get data from either the
    // destination or the source images (but not both). But for the
    // sake of simplicity and running under the assumption that the
    // source image is smaller than the destination image, we'll just
    // copy the destination to the output then overwrite the
    // appropriate output pixels with the source.

    if (!(this->GetInPlace() && this->CanRunInPlace()))
    {
      // Copy destination to output
      ImageAlgorithm::Copy(destPtr, outputPtr, outputRegionForThread, outputRegionForThread);

      progress.Completed(outputRegionForThread.GetNumberOfPixels() - destinationRegion.GetNumberOfPixels());
    }

    if (sourcePtr)
    {
      // copy the cropped source region to output
      ImageAlgorithm::Copy(sourcePtr, outputPtr, sourceRegionInSourceImageCropped, destinationRegion);

      progress.Completed(destinationRegion.GetNumberOfPixels());
    }
    else
    {
      const SourceImagePixelType sourceValue = this->GetConstant();

      for (ImageScanlineIterator outputIt(outputPtr, destinationRegion); !outputIt.IsAtEnd(); outputIt.NextLine())
      {
        while (!outputIt.IsAtEndOfLine())
        {
          outputIt.Set(sourceValue);
          ++outputIt;
        }
        progress.Completed(outputRegionForThread.GetSize()[0]);
      }
    }
  }
}


template <typename TInputImage, typename TSourceImage, typename TOutputImage>
auto
PasteImageFilter<TInputImage, TSourceImage, TOutputImage>::GetPresumedDestinationSize() const -> InputImageSizeType
{
  auto numberSkippedAxis = std::accumulate(m_DestinationSkipAxes.begin(), m_DestinationSkipAxes.end(), 0);

  if (numberSkippedAxis != (InputImageDimension - SourceImageDimension))
  {
    itkExceptionMacro("Number of skipped axis " << m_DestinationSkipAxes);
  }

  InputImageSizeType ret;
  for (unsigned int d = 0, s = 0; d < InputImageDimension; ++d)
  {
    if (m_DestinationSkipAxes[d])
    {
      ret[d] = 1;
    }
    else
    {
      ret[d] = this->m_SourceRegion.GetSize()[s++];
    }
  }
  return ret;
}

template <typename TInputImage, typename TSourceImage, typename TOutputImage>
void
PasteImageFilter<TInputImage, TSourceImage, TOutputImage>::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "SourceRegion: " << m_SourceRegion << std::endl;
  os << indent
     << "DestinationIndex: " << static_cast<typename NumericTraits<InputImageIndexType>::PrintType>(m_DestinationIndex)
     << std::endl;
  os << indent << "DestinationSkipAxes: " << m_DestinationSkipAxes << std::endl;
}
} // end namespace itk

#endif
