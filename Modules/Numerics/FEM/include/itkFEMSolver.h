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

#ifndef itkFEMSolver_h
#define itkFEMSolver_h

#include "itkProcessObject.h"
#include "itkFEMObject.h"

#include "itkFEMLinearSystemWrapper.h"
#include "itkFEMLinearSystemWrapperVNL.h"

#include "itkImage.h"

namespace itk
{
namespace fem
{
/**
 * \class Solver
 * \brief FEM solver used to generate a solution for a FE formulation.
 *
 * This class will solve the FE formulation provided in an FEMObject.
 * The FEMObject contains the Elements, Material properties, Loads,
 * and boundary conditions for the FE problem. The user can define
 * properties of the solver including the time step using the
 * SetTimeStep() method and the numerical solver via the
 * SetLinearSystemWrapper() method. The output of the filter is the deformed
 * FEMObject that also includes all of the loads and boundary conditions.
 *
 *
 * \par Inputs and Usage
 * The standard way to setup a FE problem in ITK is to use the following
 * approach.
 *
   \code
         using FEMObjectType = itk::fem::FEMObject<3>;
         auto fem = FEMObjectObjectType::New();
         ...
         using FEMSolverType = itk::fem::Solver<3>;
         auto solver = FEMSolverType::New();

         solver->SetInput( fem );
         solver->Update();
         FEMSolverType::Pointer defem = solver->GetOutput();
     ...
   \endcode
 *
 * The solution generated by the Solver can also be acquired using the
 * GetSolution() method. The FEM can be saved in a file using the
 * spatial objects and the Meta I/O library.
 *
 * \ingroup ITKFEM
 */

template <unsigned int VDimension = 3>
class ITK_TEMPLATE_EXPORT Solver : public ProcessObject
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(Solver);

  /** Standard class type aliases. */
  using Self = Solver;
  using Superclass = ProcessObject;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** \see LightObject::GetNameOfClass() */
  itkOverrideGetNameOfClassMacro(Solver);

  static constexpr unsigned int FEMDimension = VDimension;
  static constexpr unsigned int MaxDimensions = 3;

  /** Smart Pointer type to a DataObject. */
  using FEMObjectType = typename itk::fem::FEMObject<VDimension>;
  using FEMObjectPointer = typename FEMObjectType::Pointer;
  using FEMObjectConstPointer = typename FEMObjectType::ConstPointer;
  using DataObjectPointer = typename DataObject::Pointer;

  /** Some convenient type alias. */
  using Float = Element::Float;
  using VectorType = Element::VectorType;
  using NodeArray = Element::Node::ArrayType;
  using ElementArray = Element::ArrayType;
  using LoadArray = Load::ArrayType;
  using MaterialArray = Material::ArrayType;

  /**
   * Type used to store interpolation grid
   */
  using InterpolationGridType = typename itk::Image<Element::ConstPointer, VDimension>;
  using InterpolationGridPointerType = typename InterpolationGridType::Pointer;
  using InterpolationGridSizeType = typename InterpolationGridType::SizeType;
  using InterpolationGridRegionType = typename InterpolationGridType::RegionType;
  using InterpolationGridPointType = typename InterpolationGridType::PointType;
  using InterpolationGridSpacingType = typename InterpolationGridType::SpacingType;
  using InterpolationGridIndexType = typename InterpolationGridType::IndexType;
  using InterpolationGridDirectionType = typename InterpolationGridType::DirectionType;

  /** Get/Set the interpolation grid Origin. */
  itkSetMacro(Origin, InterpolationGridPointType);
  itkGetMacro(Origin, InterpolationGridPointType);

  /** Get/Set the interpolation grid spacing. */
  itkSetMacro(Spacing, InterpolationGridSpacingType);
  itkGetMacro(Spacing, InterpolationGridSpacingType);

  /** Get/Set the interpolation grid region. */
  itkSetMacro(Region, InterpolationGridRegionType);
  itkGetMacro(Region, InterpolationGridRegionType);

  /** Get/Set the interpolation grid direction. */
  itkSetMacro(Direction, InterpolationGridDirectionType);
  itkGetMacro(Direction, InterpolationGridDirectionType);

  /** Returns the time step used for dynamic problems. */
  virtual Float
  GetTimeStep() const;

  /**
   * Sets the time step used for dynamic problems.
   *
   * \param dt New time step.
   */
  virtual void
  SetTimeStep(Float dt);

  /** Returns the Solution for the specified nodal point. */
  Float
  GetSolution(unsigned int i, unsigned int which = 0);

  /** Set/Get the image input of this process object.
   * Connect one of the operands for pixel-wise addition. */
  using Superclass::SetInput;
  virtual void
  SetInput(FEMObjectType * fem);

  virtual void
  SetInput(unsigned int, FEMObjectType * fem);

  FEMObjectType *
  GetInput();

  FEMObjectType *
  GetInput(unsigned int idx);

  /**
   * Returns the pointer to the element which contains global point pt.
   *
   * \param pt Point in global coordinate system.
   *
   * \note Interpolation grid must be initializes before you can
   *       call this function.
   */
  const Element *
  GetElementAtPoint(const VectorType & pt) const;

  /** Get the total deformation energy using the chosen solution */
  Float
  GetDeformationEnergy(unsigned int SolutionIndex = 0);

  /**
   * Sets the LinearSystemWrapper object that will be used when solving
   * the master equation. If this function is not called, a default VNL linear
   * system representation will be used (class LinearSystemWrapperVNL).
   *
   * \param ls Pointer to an object of class which is derived from
   *           LinearSystemWrapper.
   *
   * \note Once the LinearSystemWrapper object is changed, it is used until
   *       the member function SetLinearSystemWrapper is called again. Since
   *       LinearSystemWrapper object was created outside the Solver class, it
   *       should also be destroyed outside. Solver class will not destroy it
   *       when the Solver object is destroyed.
   */
  void
  SetLinearSystemWrapper(LinearSystemWrapper::Pointer ls);

  /**
   * Gets the LinearSystemWrapper object.
   *
   * \sa SetLinearSystemWrapper
   */
  LinearSystemWrapper::Pointer
  GetLinearSystemWrapper()
  {
    return m_LinearSystem;
  }

  /**
   * Initialize the interpolation grid. The interpolation grid is used to
   * find elements that contains specific points in a mesh. The interpolation
   * grid stores pointers to elements for each point on a grid thereby providing
   * a fast way (lookup table) to perform interpolation of results.
   *
   * \note Interpolation grid must be reinitialized each time a mesh changes.
   *
   * \param size Vector that represents number of points on a grid in each dimension.
   * \param bb1 Lower limit of a bounding box of a grid.
   * \param bb2 Upper limit of a bounding box of a grid.
   *
   * \sa GetInterpolationGrid
   */
  void
  InitializeInterpolationGrid(const InterpolationGridSizeType & size,
                              const InterpolationGridPointType & bb1,
                              const InterpolationGridPointType & bb2);

  /** Same as InitializeInterpolationGrid(size, {0,0...}, size); */
  void
  InitializeInterpolationGrid(const InterpolationGridSizeType & size)
  {
    InterpolationGridPointType bb1{};

    InterpolationGridPointType bb2;
    for (unsigned int i = 0; i < FEMDimension; ++i)
    {
      bb2[i] = size[i] - 1.0;
    }
    InitializeInterpolationGrid(size, bb1, bb2);
  }

  /** Initialize the interpolation grid, over the domain specified by the
   * user. */
  void
  InitializeInterpolationGrid(const InterpolationGridRegionType & region,
                              const InterpolationGridPointType & origin,
                              const InterpolationGridSpacingType & spacing,
                              const InterpolationGridDirectionType & direction);

  /**
   * Returns pointer to interpolation grid, which is an itk::Image of pointers
   * to Element objects. Normally you would use physical coordinates to get
   * specific points (pointers to elements) from the image. You can then
   * use the Element::InterpolateSolution member function on the returned
   * element to obtain the solution at this point.
   *
   * \note Physical coordinates in an image correspond to the global
   *       coordinate system in which the mesh (nodes) are.
   */
  const InterpolationGridType *
  GetInterpolationGrid() const
  {
    return m_InterpolationGrid.GetPointer();
  }


  /** Make a DataObject of the correct type to be used as the specified
   * output. */
  using DataObjectPointerArraySizeType = ProcessObject::DataObjectPointerArraySizeType;
  using Superclass::MakeOutput;
  DataObjectPointer MakeOutput(DataObjectPointerArraySizeType) override;

  /** Get the output data of this process object.  The output of this
   * function is not valid until an appropriate Update() method has
   * been called, either explicitly or implicitly.  Both the filter
   * itself and the data object have Update() methods, and both
   * methods update the data.
   *
   * For Filters which have multiple outputs of different types, the
   * GetOutput() method assumes the output is of OutputImageType. For
   * the GetOutput(unsigned int) method, a dynamic_cast is performed
   * incase the filter has outputs of different types or image
   * types. Derived classes should have names get methods for these
   * outputs.
   */
  FEMObjectType *
  GetOutput();

  FEMObjectType *
  GetOutput(unsigned int idx);

protected:
  Solver();
  ~Solver() override;
  void
  PrintSelf(std::ostream & os, Indent indent) const override;

  /** Method invoked by the pipeline in order to trigger the computation of
   * the registration. */
  void
  GenerateData() override;


  /**
   * System solver functions. Call all six functions below (in listed order) to solve system.
   */

  /**
   * Assign a global freedom numbers to each DOF in a system.
   * This must be done before any other solve function can be called.
   */
  // void GenerateGFN();

  /** Assemble the master stiffness matrix (also apply the MFCs to K). */
  void
  AssembleK();

  /**
   * This function is called before assembling the matrices. You can
   * override it in a derived class to account for special needs.
   *
   * \param N Size of the matrix.
   */
  virtual void
  InitializeMatrixForAssembly(unsigned int N);

  /**
   * This function is called after the assembly has been completed.
   * In this class it is only used to apply the BCs. You may however
   * use it to perform other stuff in derived solver classes.
   */
  virtual void
  FinalizeMatrixAfterAssembly()
  {
    // Apply the boundary conditions to the K matrix
    this->ApplyBC();
  }

  /**
   * Copy the element stiffness matrix into the correct position in the
   * master stiffness matrix. Since more complex Solver classes may need to
   * assemble many matrices and may also do some funky stuff to them, this
   * function is and can be overridden in a derived solver class.
   */
  virtual void
  AssembleElementMatrix(Element::Pointer e);

  /**
   * Add the contribution of the landmark-containing elements to the
   * correct position in the master stiffness matrix. Since more
   * complex Solver classes may need to assemble many matrices and may
   * also do some funky stuff to them, this function is virtual and
   * can be overridden in a derived solver class.
   */
  virtual void
  AssembleLandmarkContribution(Element::ConstPointer e, float);

  /**
   * Apply the boundary conditions to the system.
   *
   * \note This function must be called after AssembleK().
   *
   * \param matrix Index of a matrix, to which the BCs should be
   *               applied (master stiffness matrix). Normally this
   *               is zero, but in derived classes many matrices may
   *               be used and this index must be specified.
   * \param dim This is a parameter that can be passed to the function and is
   *            normally used with isotropic elements to specify the
   *            dimension in which the DOF is fixed.
   */
  void
  ApplyBC(int dim = 0, unsigned int matrix = 0);

  /**
   * Assemble the master force vector.
   *
   * \param dim This is a parameter that can be passed to the function and is
   *            normally used with isotropic elements to specify the
   *            dimension for which the master force vector should be assembled.
   */
  void
  AssembleF(int dim = 0);

  /** Decompose matrix using svd, qr, etc. if needed. */
  void
  DecomposeK();

  /** Solve for the displacement vector u. May be overridden in derived
   * classes. */
  virtual void
  RunSolver();

  /**
   * Copy solution vector u to the corresponding nodal values, which are
   * stored in node objects). This is standard post processing of the solution.
   */
  void
  UpdateDisplacements();

  /** Fill the interpolation grid based on the current deformed grid. */
  void
  FillInterpolationGrid();

  /**
   * Performs any initialization needed for LinearSystemWrapper
   * object i.e. sets the maximum number of matrices and vectors.
   */
  virtual void
  InitializeLinearSystemWrapper();

  /** Number of global degrees of freedom in a system. */
  unsigned int m_NGFN{};

  /**
   * Number of multi freedom constraints in a system.
   * This member is set in a AssembleK function.
   */
  unsigned int m_NMFC{};

  /** Pointer to LinearSystemWrapper object. */
  LinearSystemWrapper::Pointer m_LinearSystem{};

  /** LinearSystemWrapperVNL object that is used by default in Solver class. */
  LinearSystemWrapperVNL m_LinearSystemVNL{};

  /**
   * An Image of pointers to Element objects that represents a grid used
   * for interpolation of solution. Each Pixel in an image is a pointer to
   * an Element object in which that pixel is located.
   */
  InterpolationGridPointerType m_InterpolationGrid{};

  FEMObjectPointer m_FEMObject{};

private:
  /** Properties of the interpolation grid. */
  InterpolationGridRegionType m_Region{};
  InterpolationGridPointType m_Origin{};
  InterpolationGridSpacingType m_Spacing{};
  InterpolationGridDirectionType m_Direction{};
};
} // end namespace fem
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#  include "itkFEMSolver.hxx"
#endif

#endif // itkFEMSolver_h
