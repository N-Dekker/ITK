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

// First include the header file to be tested:
#include "itkHausdorffDistanceImageFilter.h"
#include "itkImageRegionRange.h"

#include "itkGTest.h"

#include <algorithm> // For fill.
#include <numeric>   // For iota.

TEST(HausdorffDistanceImageFilter, Test)
{
  constexpr unsigned int ImageDimension{ 3 };

  using IndexType = itk::Index<ImageDimension>;
  using SizeType = itk::Size<ImageDimension>;
  using RegionType = itk::ImageRegion<ImageDimension>;

  using Pixel1Type = unsigned int;
  using Pixel2Type = float;

  using Image1Type = itk::Image<Pixel1Type, ImageDimension>;
  using Image2Type = itk::Image<Pixel2Type, ImageDimension>;

  auto image1 = Image1Type::New();
  auto image2 = Image2Type::New();

  constexpr auto size = SizeType::Filled(50);

  image1->SetRegions(size);
  image2->SetRegions(size);

  image1->AllocateInitialized();
  image2->AllocateInitialized();

  const RegionType region1{ IndexType::Filled(10), SizeType::Filled(20) };
  const RegionType region2{ IndexType::Filled(20), SizeType::Filled(15) };

  const itk::ImageRegionRange<Image1Type> imageRegionRange1(*image1, region1);
  std::iota(imageRegionRange1.begin(), imageRegionRange1.end(), Pixel1Type{ 1 });

  const itk::ImageRegionRange<Image2Type> imageRegionRange2(*image2, region2);
  std::fill(imageRegionRange2.begin(), imageRegionRange2.end(), Pixel2Type{ 7.2 });

  // Compute the Hausdorff distance H(image1,image2)
  {
    using FilterType = itk::HausdorffDistanceImageFilter<Image1Type, Image2Type>;
    auto filter = FilterType::New();

    ITK_GTEST_EXERCISE_BASIC_OBJECT_METHODS(filter, HausdorffDistanceImageFilter, ImageToImageFilter);

    filter->SetInput1(image1);
    filter->SetInput2(image2);
    filter->Update();

    // Check results
    const FilterType::RealType trueDistance = 10 * std::sqrt(double{ ImageDimension });
    const FilterType::RealType distance = filter->GetHausdorffDistance();

    EXPECT_NEAR(distance, trueDistance, 0.1);
    EXPECT_NEAR(filter->GetAverageHausdorffDistance(), 4.5, 0.1);
  }

  // Compute the Hausdorff distance H(image2,image1)
  {
    using FilterType = itk::HausdorffDistanceImageFilter<Image2Type, Image1Type>;
    auto filter = FilterType::New();

    filter->SetInput1(image2);
    filter->SetInput2(image1);
    filter->Update();

    // Check results
    const FilterType::RealType trueDistance = 10 * std::sqrt(double{ ImageDimension });
    const FilterType::RealType distance = filter->GetHausdorffDistance();

    EXPECT_NEAR(distance, trueDistance, 0.1);
    EXPECT_NEAR(filter->GetAverageHausdorffDistance(), 4.5, 0.1);
  }

  // Compute the Hausdorff distance H(image2,image1)
  {
    const Image1Type::SpacingType spacing1 = image1->GetSpacing() / 2.0;
    image1->SetSpacing(spacing1);
    const Image2Type::SpacingType spacing2 = image2->GetSpacing() / 2.0;
    image2->SetSpacing(spacing2);

    using FilterType = itk::HausdorffDistanceImageFilter<Image2Type, Image1Type>;
    auto filter = FilterType::New();

    filter->SetUseImageSpacing(true);
    EXPECT_TRUE(filter->GetUseImageSpacing());

    filter->SetInput1(image2);
    filter->SetInput2(image1);
    filter->Update();

    // Check results
    const FilterType::RealType trueDistance =
      10 * std::sqrt(spacing1[0] * spacing1[0] + spacing1[1] * spacing1[1] + spacing1[2] * spacing1[2]);
    // Note that the following is only correct because spacing is the same across dimensions:
    const FilterType::RealType trueAverageDistance = 4.5 * spacing1[0];
    const FilterType::RealType distance = filter->GetHausdorffDistance();

    EXPECT_NEAR(distance, trueDistance, 0.1);
    EXPECT_NEAR(filter->GetAverageHausdorffDistance(), trueAverageDistance, 0.1);
  }
}
