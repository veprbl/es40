/* ES40 emulator.
 * Copyright (C) 2007-2008 by the ES40 Emulator Project
 *
 * WWW    : http://sourceforge.net/projects/es40
 * E-mail : camiel@camicom.com
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 * Although this is not required, the author would appreciate being notified of, 
 * and receiving any modifications you may make to the source code that might serve
 * the general public.
 */

/**
 * \file 
 * Contains the definitions for the different locking structures for multi-threading.
 *
 * $Id: Lock.h,v 1.6 2008/03/14 19:20:38 iamcamiel Exp $
 *
 * X-1.6        Camiel Vanderhoeven                             14-MAR-2008
 *      2-second timeouts on simple mutexes.
 *
 * X-1.5        Camiel Vanderhoeven                             14-MAR-2008
 *      Formatting.
 *
 * X-1.4        Camiel Vanderhoeven                             14-MAR-2008
 *   1. More meaningful exceptions replace throwing (int) 1.
 *   2. U64 macro replaces X64 macro.
 *
 * X-1.3        Camiel Vanderhoeven                             12-MAR-2007
 *      Print an error message on locking failures.
 *
 * X-1.2        Brian Wheeler                                   11-MAR-2007
 *      Fixed macro concatenation syntax error.
 *
 * X-1.1        Camiel Vanderhoeven                             11-MAR-2007
 *      File created to support named, debuggable mutexes.
 **/

//#define DEBUG_LOCKS
#include <Poco/Mutex.h>
#include <Poco/RWLock.h>

/**
 * This Macro finds out what the current Poco::Thread object is, and returns it's name. If there's no
 * Poco::Thread object associated with this thread, it's assumed to be the main thread, and "main"
 * is returned.
 **/
#define CURRENT_THREAD_NAME Poco::Thread::current() ? Poco::Thread::current() \
    ->getName().c_str() : "main"

/**
 * \brief A class that simplifies thread synchronization with a mutex or fastmutex.
 *
 * The constructor accepts a Mutex and locks it.
 * The destructor unlocks the mutex.
 **/
template<class M>
class CScopedLock
{
  public:
    inline  CScopedLock(M* mutex) { _mutex = mutex; _mutex->lock(2000); }
    inline  ~CScopedLock()        { _mutex->unlock(); }
  private:
    M*  _mutex;
};

class CMutex : public Poco::MutexImpl
{
  public:
    typedef CScopedLock<CMutex> ScopedLock;
    CMutex(char* lName)                   { lockName = strdup(lName); };
    ~                           CMutex()  { free(lockName); };
    void                        lock();
    void                        lock(long milliseconds);
    bool                        tryLock();
    bool                        tryLock(long milliseconds);
    void                        unlock();

    char*                       lockName;
};

class CFastMutex : public Poco::FastMutexImpl
{
  public:
    typedef CScopedLock<CFastMutex> ScopedLock;
    CFastMutex(char* lName)                       { lockName = strdup(lName); };
    ~                               CFastMutex()  { free(lockName); };
    void                            lock();
    void                            lock(long milliseconds);
    bool                            tryLock();
    bool                            tryLock(long milliseconds);
    void                            unlock();
    char*                           lockName;
};

class CScopedRWLock;

class CRWMutex : public Poco::RWLockImpl
{
  public:
    typedef CScopedRWLock ScopedLock;
    CRWMutex(char* lName)             { lockName = strdup(lName); };
    ~                     CRWMutex()  { free(lockName); };

    void                  readLock();
    bool                  tryReadLock();
    void                  writeLock();
    bool                  tryWriteLock();
    void                  unlock();

    char*                 lockName;
};

class CScopedRWLock
{
  public:
    CScopedRWLock(CRWMutex* rwl, bool write = false);
    ~ CScopedRWLock();
  private:
    CRWMutex*   _rwl;
};

inline void CMutex::lock()
{
#if defined(DEBUG_LOCKS)
  printf("        LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    lockImpl();
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to lock mutex %s from thread %s.\n", e.message().c_str(),
                  lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("      LOCKED mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
}

inline void CMutex::lock(long milliseconds)
{
#if defined(DEBUG_LOCKS)
  printf("        LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    if(!tryLockImpl(milliseconds))
      FAILURE(Timeout, "Timeout");
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to lock mutex %s from thread %s.\n", e.message().c_str(),
                  lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("      LOCKED mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
}

inline bool CMutex::tryLock()
{
  bool  res;
#if defined(DEBUG_LOCKS)
  printf("    TRY LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    res = tryLockImpl();
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to lock mutex %s from thread %s.\n", e.message().c_str(),
                  lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("  %s mutex %s from thread %s.   \n", res ? "    LOCKED" : "CAN'T LOCK",
         lockName, CURRENT_THREAD_NAME);
#endif
  return res;
}

inline bool CMutex::tryLock(long milliseconds)
{
  bool  res;
#if defined(DEBUG_LOCKS)
  printf("    TRY LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    res = tryLockImpl(milliseconds);
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to lock mutex %s from thread %s.\n", e.message().c_str(),
                  lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("  %s mutex %s from thread %s.   \n", res ? "    LOCKED" : "CAN'T LOCK",
         lockName, CURRENT_THREAD_NAME);
#endif
  return res;
}

inline void CMutex::unlock()
{
#if defined(DEBUG_LOCKS)
  printf("      UNLOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    unlockImpl();
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to unlock mutex %s from thread %s.\n",
              e.message().c_str(), lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("    UNLOCKED mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
}

inline void CFastMutex::lock()
{
#if defined(DEBUG_LOCKS)
  printf("        LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    lockImpl();
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to lock mutex %s from thread %s.\n", e.message().c_str(),
                  lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("      LOCKED mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
}

inline void CFastMutex::lock(long milliseconds)
{
#if defined(DEBUG_LOCKS)
  printf("        LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    if(!tryLockImpl(milliseconds))
      FAILURE(Timeout, "Timeout");
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to lock mutex %s from thread %s.\n", e.message().c_str(),
                  lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("      LOCKED mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
}

inline bool CFastMutex::tryLock()
{
  bool  res;
#if defined(DEBUG_LOCKS)
  printf("    TRY LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    res = tryLockImpl();
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to lock mutex %s from thread %s.\n", e.message().c_str(),
                  lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("  %s mutex %s from thread %s.   \n", res ? "    LOCKED" : "CAN'T LOCK",
         lockName, CURRENT_THREAD_NAME);
#endif
  return res;
}

inline bool CFastMutex::tryLock(long milliseconds)
{
  bool  res;
#if defined(DEBUG_LOCKS)
  printf("    TRY LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    res = tryLockImpl(milliseconds);
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to lock mutex %s from thread %s.\n", e.message().c_str(),
                  lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("  %s mutex %s from thread %s.   \n", res ? "    LOCKED" : "CAN'T LOCK",
         lockName, CURRENT_THREAD_NAME);
#endif
  return res;
}

inline void CFastMutex::unlock()
{
#if defined(DEBUG_LOCKS)
  printf("      UNLOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    unlockImpl();
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to unlock mutex %s from thread %s.\n",
              e.message().c_str(), lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("    UNLOCKED mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
}

inline void CRWMutex::readLock()
{
#if defined(DEBUG_LOCKS)
  printf("   READ LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    readLockImpl();
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to read-lock mutex %s from thread %s.\n",
              e.message().c_str(), lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf(" READ LOCKED mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
}

inline bool CRWMutex::tryReadLock()
{
  bool  res;
#if defined(DEBUG_LOCKS)
  printf(" TRY RD LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    res = tryReadLockImpl();
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to read-lock mutex %s from thread %s.\n",
              e.message().c_str(), lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("  %s mutex %s from thread %s.   \n", res ? "    LOCKED" : "CAN'T LOCK",
         lockName, CURRENT_THREAD_NAME);
#endif
  return res;
}

inline void CRWMutex::writeLock()
{
#if defined(DEBUG_LOCKS)
  printf("  WRITE LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    writeLockImpl();
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to write-lock mutex %s from thread %s.\n",
              e.message().c_str(), lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("WRITE LOCKED mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
}

inline bool CRWMutex::tryWriteLock()
{
  bool  res;
#if defined(DEBUG_LOCKS)
  printf(" TRY WR LOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    res = tryWriteLockImpl();
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to write-lock mutex %s from thread %s.\n",
              e.message().c_str(), lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("  %s mutex %s from thread %s.   \n", res ? "    LOCKED" : "CAN'T LOCK",
         lockName, CURRENT_THREAD_NAME);
#endif
  return res;
}

inline void CRWMutex::unlock()
{
#if defined(DEBUG_LOCKS)
  printf("      UNLOCK mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
  try
  {
    unlockImpl();
  }

  catch(Poco::Exception & e)
  {
    FAILURE_3(Thread,
              "Locking error (%s) trying to unlock mutex %s from thread %s.\n",
              e.message().c_str(), lockName, CURRENT_THREAD_NAME);
  }

#if defined(DEBUG_LOCKS)
  printf("    UNLOCKED mutex %s from thread %s.   \n", lockName,
         CURRENT_THREAD_NAME);
#endif
}

inline CScopedRWLock::CScopedRWLock(CRWMutex* rwl, bool write) : _rwl(rwl)
{
  _rwl = rwl;
  if(write)
    _rwl->writeLock();
  else
    _rwl->readLock();
}

inline CScopedRWLock::~CScopedRWLock()
{
  _rwl->unlock();
}

#define MUTEX_LOCK(mutex)         mutex->lock(2000)
#define MUTEX_READ_LOCK(mutex)    mutex->readLock()
#define MUTEX_WRITE_LOCK(mutex)   mutex->writeLock()
#define MUTEX_UNLOCK(mutex)       mutex->unlock()
#define SCOPED_M_LOCK(mutex)      CMutex::ScopedLock L_##__LINE__(mutex)
#define SCOPED_FM_LOCK(mutex)     CFastMutex::ScopedLock L_##__LINE__(mutex)
#define SCOPED_READ_LOCK(mutex)   CRWMutex::ScopedLock L_##__LINE__(mutex, false)
#define SCOPED_WRITE_LOCK(mutex)  CRWMutex::ScopedLock L_##__LINE__(mutex, true)
