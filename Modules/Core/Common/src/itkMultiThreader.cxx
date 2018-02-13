/*=========================================================================
 *
 *  Copyright Insight Software Consortium
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
/*=========================================================================
 *
 *  Portions of this file are subject to the VTK Toolkit Version 3 copyright.
 *
 *  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
 *
 *  For complete copyright, license and disclaimer of warranty information
 *  please refer to the NOTICE file at the top of the ITK source tree.
 *
 *=========================================================================*/
#include "itkMultiThreader.h"
#include "itkNumericTraits.h"
#include <iostream>
#include <string>
#if !defined( ITK_LEGACY_FUTURE_REMOVE )
# include "vcl_algorithm.h"
#endif
#include <algorithm>

#if defined(ITK_USE_PTHREADS)
#include "itkMultiThreaderPThreads.cxx"
#elif defined(ITK_USE_WIN32_THREADS)
#include "itkMultiThreaderWinThreads.cxx"
#else
#include "itkMultiThreaderNoThreads.cxx"
#endif

namespace itk
{

MultiThreader::MultiThreader()
{
  for( ThreadIdType i = 0; i < ITK_MAX_THREADS; ++i )
    {
    m_ThreadInfoArray[i].ThreadID           = i;
    m_ThreadInfoArray[i].ActiveFlag         = nullptr;
    m_ThreadInfoArray[i].ActiveFlagLock     = nullptr;

    m_MultipleMethod[i]                     = nullptr;
    m_MultipleData[i]                       = nullptr;

    m_SpawnedThreadActiveFlag[i]            = 0;
    m_SpawnedThreadActiveFlagLock[i]        = nullptr;
    m_SpawnedThreadInfoArray[i].ThreadID    = i;
    }

  m_SingleMethod = nullptr;
  m_SingleData = nullptr;
}

MultiThreader::~MultiThreader()
{
}

void MultiThreader::SetSingleMethod(ThreadFunctionType f, void *data)
{
  m_SingleMethod = f;
  m_SingleData   = data;
}

// Set one of the user defined methods that will be run on NumberOfThreads
// threads when MultipleMethodExecute is called. This method should be
// called with index = 0, 1, ..,  NumberOfThreads-1 to set up all the
// required user defined methods
void MultiThreader::SetMultipleMethod(ThreadIdType index, ThreadFunctionType f, void *data)
{
  // You can only set the method for 0 through NumberOfThreads-1
  if( index >= m_NumberOfThreads )
    {
    itkExceptionMacro(<< "Can't set method " << index << " with a thread count of " << m_NumberOfThreads);
    }
  else
    {
    m_MultipleMethod[index] = f;
    m_MultipleData[index]   = data;
    }
}

void MultiThreader::SingleMethodExecute()
{
  ThreadIdType        thread_loop = 0;
  ThreadProcessIdType process_id[ITK_MAX_THREADS];

  if( !m_SingleMethod )
    {
    itkExceptionMacro(<< "No single method set!");
    }

  // obey the global maximum number of threads limit
  m_NumberOfThreads = std::min( MultiThreaderBase::GetGlobalMaximumNumberOfThreads(), m_NumberOfThreads );

  // Init process_id table because a valid process_id (i.e., non-zero), is
  // checked in the WaitForSingleMethodThread loops
  for( thread_loop = 1; thread_loop < m_NumberOfThreads; ++thread_loop )
    {
    process_id[thread_loop] = 0;
    }

  // Spawn a set of threads through the SingleMethodProxy. Exceptions
  // thrown from a thread will be caught by the SingleMethodProxy. A
  // naive mechanism is in place for determining whether a thread
  // threw an exception.
  //
  // Thanks to Hannu Helminen for suggestions on how to catch
  // exceptions thrown by threads.
  bool        exceptionOccurred = false;
  std::string exceptionDetails;
  try
    {
    for( thread_loop = 1; thread_loop < m_NumberOfThreads; ++thread_loop )
      {
      m_ThreadInfoArray[thread_loop].UserData = m_SingleData;
      m_ThreadInfoArray[thread_loop].NumberOfThreads = m_NumberOfThreads;
      m_ThreadInfoArray[thread_loop].ThreadFunction = m_SingleMethod;

      process_id[thread_loop] =
        this->SpawnDispatchSingleMethodThread(&m_ThreadInfoArray[thread_loop]);
      }
    }
  catch( std::exception & e )
    {
    // get the details of the exception to rethrow them
    exceptionDetails = e.what();
    // If creation of any thread failed, we must make sure that all
    // threads are correctly cleaned
    exceptionOccurred = true;
    }
  catch( ... )
    {
    // If creation of any thread failed, we must make sure that all
    // threads are correctly cleaned
    exceptionOccurred = true;
    }

  // Now, the parent thread calls this->SingleMethod() itself
  //
  //
  try
    {
    m_ThreadInfoArray[0].UserData = m_SingleData;
    m_ThreadInfoArray[0].NumberOfThreads = m_NumberOfThreads;
    m_SingleMethod( (void *)( &m_ThreadInfoArray[0] ) );
    }
  catch( ProcessAborted & )
    {
    // Need cleanup and rethrow ProcessAborted
    // close down other threads
    for( thread_loop = 1; thread_loop < m_NumberOfThreads; ++thread_loop )
      {
      try
        {

        this->SpawnWaitForSingleMethodThread(process_id[thread_loop]);

        }
      catch( ... )
        {
        }
      }
    // rethrow
    throw;
    }
  catch( std::exception & e )
    {
    // get the details of the exception to rethrow them
    exceptionDetails = e.what();
    // if this method fails, we must make sure all threads are
    // correctly cleaned
    exceptionOccurred = true;
    }
  catch( ... )
    {
    // if this method fails, we must make sure all threads are
    // correctly cleaned
    exceptionOccurred = true;
    }
  // The parent thread has finished this->SingleMethod() - so now it
  // waits for each of the other processes to exit
  for( thread_loop = 1; thread_loop < m_NumberOfThreads; ++thread_loop )
    {
    try
      {

      this->SpawnWaitForSingleMethodThread(process_id[thread_loop]);

      if( m_ThreadInfoArray[thread_loop].ThreadExitCode
          != ThreadInfoStruct::SUCCESS )
        {
        exceptionOccurred = true;
        }
      }
    catch( std::exception & e )
      {
      // get the details of the exception to rethrow them
      exceptionDetails = e.what();
      exceptionOccurred = true;
      }
    catch( ... )
      {
      exceptionOccurred = true;
      }
    }

  if( exceptionOccurred )
    {
    if( exceptionDetails.empty() )
      {
      itkExceptionMacro("Exception occurred during SingleMethodExecute");
      }
    else
      {
      itkExceptionMacro(<< "Exception occurred during SingleMethodExecute" << std::endl << exceptionDetails);
      }
    }
}

// Print method for the multithreader
void MultiThreader::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

}
