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
#include "itkArray2D.h"
#include <gtest/gtest.h>
#include <limits>

namespace
{
// Checks that an object created by the "filling" constructor is equal to an object created by Array2D(numberOfRows,
// numberOfCols) that is filled afterwards.
template <typename TValue>
void
Check_filling_constructor(const unsigned int numberOfRows, const unsigned int numberOfCols)
{
  for (const TValue fillValue : { TValue{}, TValue{ 1 }, std::numeric_limits<TValue>::max() })
  {
    const itk::Array2D<TValue> actual(numberOfRows, numberOfCols, fillValue);

    itk::Array2D<TValue> expected(numberOfRows, numberOfCols);
    expected.Fill(fillValue);

    EXPECT_EQ(actual, expected);
  }
}
} // namespace


// Tests the "filling" constructor Array2D(numberOfRows, numberOfCols, fillValue)
TEST(Array2D, FillingConstructor)
{
  Check_filling_constructor<int>(2, 3);
  Check_filling_constructor<double>(3, 2);
}
