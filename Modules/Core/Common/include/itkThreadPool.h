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
#ifndef itkThreadPool_h
#define itkThreadPool_h

#include "itkConfigure.h"
#include "itkIntTypes.h"

#include <deque>
#include <functional>
#include <future>
#include <condition_variable>
#include <thread>

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkSingletonMacro.h"


namespace itk
{

/**
 * \class ThreadPool
 * \brief Thread pool maintains a constant number of threads.
 *
 * Thread pool is called and initialized from within the PoolMultiThreader.
 * Initially the thread pool is started with GlobalDefaultNumberOfThreads.
 * The jobs are submitted via AddWork method.
 *
 * This implementation heavily borrows from:
 * https://github.com/progschj/ThreadPool
 *
 * \ingroup OSSystemObjects
 * \ingroup ITKCommon
 */

struct ThreadPoolGlobals;

class ITKCommon_EXPORT ThreadPool : public Object
{
public:
  ITK_DISALLOW_COPY_AND_MOVE(ThreadPool);

  /** Standard class type aliases. */
  using Self = ThreadPool;
  using Superclass = Object;
  using Pointer = SmartPointer<Self>;
  using ConstPointer = SmartPointer<const Self>;

  /** Run-time type information (and related methods). */
  itkTypeMacro(ThreadPool, Object);

  /** Returns the global instance */
  static Pointer
  New();

  /** Returns the global singleton instance of the ThreadPool */
  static Pointer
  GetInstance();

  /** Add this job to the thread pool queue.
   *
   * This method returns an std::future, and calling get()
   * will block until the result is ready. Example usage:
\code
auto result = pool->AddWork([](int param) { return param; }, 7);
\endcode
   * std::cout << result.get() << std::endl; */
  template <class Function, class... Arguments>
  auto
  AddWork(Function && function, Arguments &&... arguments) -> std::future<std::invoke_result_t<Function, Arguments...>>
  {
    using return_type = std::invoke_result_t<Function, Arguments...>;

    std::packaged_task<return_type()> task(
      [function, arguments...]() -> return_type { return function(arguments...); });

    std::future<return_type> res = task.get_future();
    {
      const std::lock_guard<std::mutex> lockGuard(this->GetMutex());
      m_WorkQueue.emplace_back(std::move(task));
    }
    m_Condition.notify_one();
    return res;
  }

  /** Can call this method if we want to add extra threads to the pool. */
  void
  AddThreads(ThreadIdType count);

  ThreadIdType
  GetMaximumNumberOfThreads() const
  {
    const std::lock_guard<std::mutex> lockGuard(this->GetMutex());
    return static_cast<ThreadIdType>(m_Threads.size());
  }

  /** The approximate number of idle threads. */
  int
  GetNumberOfCurrentlyIdleThreads() const;

  /** Set/Get wait for threads.
  This function should be used carefully, probably only during static
  initialization phase to disable waiting for threads when ITK is built as a
  static library and linked into a shared library (Windows only). */
  static bool
  GetDoNotWaitForThreads();
  static void
  SetDoNotWaitForThreads(bool doNotWaitForThreads);

protected:
  /** We need access to the mutex in AddWork, and the variable is only
   * visible in the .cxx file, so this method returns it. */
  std::mutex &
  GetMutex() const;

  ThreadPool();

  /** Stop the pool and release threads. To be called by the destructor and atfork. */
  void
  CleanUp();

  ~ThreadPool() override { this->CleanUp(); }

  static void
  PrepareForFork();
  static void
  ResumeFromFork();

private:
  /** Only used to synchronize the global variable across static libraries.*/
  itkGetGlobalDeclarationMacro(ThreadPoolGlobals, PimplGlobals);

  // Move-only function object class (similar to C++23 `std::move_only_function<void()>`) that wraps a
  // `std::packaged_task<T()>`. Internal, for implementation only.
  class PackagedTaskFunction
  {
  private:
    class AbstractTaskHolder
    {
    public:
      ITK_DISALLOW_COPY_AND_MOVE(AbstractTaskHolder);

      virtual void
      Call() = 0;

      virtual ~AbstractTaskHolder() = default;

    protected:
      AbstractTaskHolder() = default;
    };

    template <typename TResult>
    class TaskHolder : public AbstractTaskHolder
    {
    public:
      ITK_DISALLOW_COPY_AND_MOVE(TaskHolder);

      explicit TaskHolder(std::packaged_task<TResult()> && task)
        : m_Task(std::move(task))
      {}

      ~TaskHolder() final = default;

    private:
      void
      Call() final
      {
        m_Task();
      }

      std::packaged_task<TResult()> m_Task{};
    };

  public:
    // Explicit constructor.
    template <typename TResult>
    explicit PackagedTaskFunction(std::packaged_task<TResult()> && task)
      : m_TaskHolder(std::make_unique<TaskHolder<TResult>>(std::move(task)))
    {}

    // Function-call operator.
    void
    operator()()
    {
      return m_TaskHolder->Call();
    }

    // Default-constructor and destructor.
    PackagedTaskFunction() = default;
    ~PackagedTaskFunction() = default;

    // Allow move, but disallow copying:
    PackagedTaskFunction(const PackagedTaskFunction &) = delete;
    PackagedTaskFunction(PackagedTaskFunction &&) = default;
    PackagedTaskFunction &
    operator=(const PackagedTaskFunction &) = delete;
    PackagedTaskFunction &
    operator=(PackagedTaskFunction &&) = default;

  private:
    std::unique_ptr<AbstractTaskHolder> m_TaskHolder{};
  };


  /** This is a list of jobs submitted to the thread pool.
   * This is the only place where the jobs are submitted.
   * Filled by AddWork, emptied by ThreadExecute. */
  std::deque<PackagedTaskFunction> m_WorkQueue; // guarded by m_PimplGlobals->m_Mutex

  /** When a thread is idle, it is waiting on m_Condition.
   * AddWork signals it to resume a (random) thread. */
  std::condition_variable m_Condition;

  /** Vector to hold all thread handles.
   * Thread handles are used to delete (join) the threads. */
  std::vector<std::thread> m_Threads; // guarded by m_PimplGlobals->m_Mutex

  /* Has destruction started? */
  bool m_Stopping{ false }; // guarded by m_PimplGlobals->m_Mutex

  /** To lock on the internal variables */
  static ThreadPoolGlobals * m_PimplGlobals;

  /** The continuously running thread function */
  static void
  ThreadExecute();
};

} // namespace itk
#endif
