# ==========================================================================
#
#   Copyright NumFOCUS
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#          https://www.apache.org/licenses/LICENSE-2.0.txt
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
# ==========================================================================*/
import numpy as np
import itk

itk.auto_progress(2)

img = itk.image_from_array(np.zeros((10, 10, 10)))
transform = itk.IdentityTransform[itk.D, 3].New()
field = itk.transform_to_displacement_field_filter(
    transform, reference_image=img, use_reference_image=True
)
print(field)
