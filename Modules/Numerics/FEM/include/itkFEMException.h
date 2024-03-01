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
#ifndef itkFEMException_h
#define itkFEMException_h

#include <typeinfo>
#include <string>

#include "itkMacro.h"

namespace itk
{
namespace fem
{
/**
 * \file itkFEMException.h
 * \brief Declaration of several exception classes that are used
    within the FEM code.
 */

/**
 * \class FEMException
 * \brief Base class for all exception's that can occur within FEM classes.
 * \ingroup ITKFEM
 */
class ITK_ABI_EXPORT FEMException : public itk::ExceptionObject
{
public:
  /**
   * Constructor. Must provide a string, that specifies name of
   * the file where the exception occurred and an integer for the
   * line number. An optional argument specifies the location
   * (usually the name of the class and member function). Normally
   * you should use __FILE__ and __LINE__ macros to specify file name
   * and line number.
   */
  FEMException(const char * file, unsigned int lineNumber, std::string location = "Unknown");

  /** Virtual destructor needed for subclasses. Has to have empty throw(). */
  ~FEMException() noexcept override;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(FEMException);
};

/**
 * \class FEMExceptionIO
 * \brief Base class for all IO exception's that can occur within FEM classes.
 *
 * This class is normally used when reading or writing objects from/to stream.
 * \ingroup ITKFEM
 */
class ITK_ABI_EXPORT FEMExceptionIO : public FEMException
{
public:
  /**
   * Constructor. In order to construct this exception object, four parameters
   * must be provided: file, lineNumber, location and a detailed description
   * of the exception.
   */
  FEMExceptionIO(const char * file, unsigned int lineNumber, std::string location, std::string moreDescription);

  /** Virtual destructor needed for subclasses. Has to have empty throw(). */
  ~FEMExceptionIO() noexcept override;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(FEMExceptionIO);
};

/**
 * \class FEMExceptionWrongClass
 * \brief Bad object exception.
 *
 * This exception occurs, when a the pointer that was passed to a
 * function or member, was pointing to the wrong class of object.
 * Usually this means that the dynamic_cast operator failed.
 * This exception object is normally  generated by catching the
 * std::bad_cast exception.
 *
 * FIXME: Note that there are big differences between compilers
 * when it comes to catching standard exceptions. MSVC, for
 * example DOESN'T properly catch std::bad_cast generated by
 * a failed dynamic_cast operator. It does, however catch the
 * std:exception. Update the bad_cast in ALL files to
 * accommodate this differences. Currently they are ignored.
 * \ingroup ITKFEM
 */
class ITK_ABI_EXPORT FEMExceptionWrongClass : public FEMException
{
public:
  FEMExceptionWrongClass(const char * file, unsigned int lineNumber, std::string location);

  /** Virtual destructor needed for subclasses. Has to have empty throw(). */
  ~FEMExceptionWrongClass() noexcept override;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(FEMExceptionWrongClass);
};

/**
 * \class FEMExceptionObjectNotFound
 * \brief Object not found exception.
 *
 * This exception occurs, when a search for an object with given
 * global number was unsuccessful.
 * \ingroup ITKFEM
 */
class ITK_ABI_EXPORT FEMExceptionObjectNotFound : public FEMException
{
public:
  FEMExceptionObjectNotFound(const char * file,
                             unsigned int lineNumber,
                             std::string  location,
                             std::string  baseClassName,
                             int          GN);

  /** Virtual destructor needed for subclasses. Has to have empty throw(). */
  ~FEMExceptionObjectNotFound() noexcept override;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(FEMExceptionObjectNotFound);

  /**
   * Base class of the searched object.
   */
  std::string m_baseClassName{};
  int         m_GlobalNumber{};
};

/**
 * \class FEMExceptionSolution
 * \brief Base class for all exceptions that can occur when solving FEM problem.
 *
 * This class is normally used when an error occurs while the problem is
 * already in memory and something went wrong while trying to solve it.
 * \ingroup ITKFEM
 */
class ITK_ABI_EXPORT FEMExceptionSolution : public FEMException
{
public:
  /**
   * Constructor. In order to construct this exception object, four parameters
   * must be provided: file, lineNumber, location and a detailed description
   * of the exception.
   */
  FEMExceptionSolution(const char * file, unsigned int lineNumber, std::string location, std::string moreDescription);

  /** Virtual destructor needed for subclasses. Has to have empty throw(). */
  ~FEMExceptionSolution() noexcept override;

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(FEMExceptionSolution);
};
} // end namespace fem
} // end namespace itk

#endif // itkFEMException_h
