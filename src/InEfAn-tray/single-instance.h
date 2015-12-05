#pragma once


#include <windows.h>
#include <string>

// http://support.microsoft.com/kb/243953
//This code is from Q243953 in case you lose the article and wonder
//where this code came from.

class LimitSingleInstance
{
protected:
    DWORD  m_dwLastError;
    HANDLE m_hMutex;

public:
    LimitSingleInstance(std::wstring const& strMutexName): m_dwLastError(0), m_hMutex(NULL)
    {
        //Make sure that you use a name that is unique for this application otherwise
        //two apps may think they are the same if they are using same name for
        //3rd param to CreateMutex
        m_hMutex = CreateMutexW(NULL, FALSE, strMutexName.c_str()); //do early
        m_dwLastError = GetLastError(); //save for use later...
    }

    ~LimitSingleInstance()
    {
        if (m_hMutex) { //Do not forget to close handles.
            CloseHandle(m_hMutex); //Do as late as possible.
            m_hMutex = NULL; //Good habit to be in.
        }
    }

    bool Release()
    {
        return FALSE != ReleaseMutex(m_hMutex);
    }

    bool IsAnotherInstanceRunning()
    {
        return (ERROR_ALREADY_EXISTS == m_dwLastError);
    }
};
