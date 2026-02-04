// Revision: $Id: fileMonitor.h 1.0 2025/05/20 17:39:05 Leon Exp $
/*
 * PeraGlobal CONFIDENTIAL
 * __________________
 *
 *  [2007] - [2025] PeraGlobal Technologies, Inc.
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of PeraGlobal Technoglies, Inc.
 * The intellectual and technical concepts contained
 * herein are proprietary to PeraGlobal Technologies, Inc.
 * and may be covered by China and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from PeraGlobal Technologies, Inc.
 */
#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <atomic>
#include <thread>
#include <chrono>
#include <mutex>
#include <condition_variable>
#include <functional>

#ifdef WIN32
#include <windows.h>

 /*!
    \brief This is one monitor for data by file mapping updated from SDK library.
    Client program can use this monitor to work with file mapping method.
    If client program wants to use this file mapping monitor, you should
    set file mapping inforation before this class.
    The method is setDocumentLogMappingInfo() in the PFDocument class. \see setDocumentLogMappingInfo().
    Client should provide file mapping name and the event name for the use of sharing data with library.
    Also client need to provide the log file name with the full path. If the path is not present, library
    will create those folders for you.
    You can refer to the meshDemo example codes.
 */
class FileMappingMonitor
{
public:
    FileMappingMonitor(std::string mapName, std::string eventName)
        :myMapFileName(mapName),
        myEventName(eventName)
    {
    }

    ~FileMappingMonitor()
    {
        stop();
    }

    void start()
    {
        if (isRunning)
        {
            return;
        }
        isRunning = true;
        monitorThread = std::thread(&FileMappingMonitor::monitorLoop, this);
    }

    void stop()
    {
        if (!isRunning)
        {
            return;
        }

        {
            std::lock_guard<std::mutex> lock(myMutex);
            isRunning = false;
        }

        myConditionVar.notify_all();

        if (monitorThread.joinable())
        {
            monitorThread.join();
        }
        if (hEvent)
        {
            CloseHandle(hEvent);
        }
        if (hMapFile)
        {
            CloseHandle(hMapFile);
        }
    }

private:
    void monitorLoop()
    {
        while (isRunning)
        {
            HANDLE tempMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, myMapFileName.c_str());
            if (tempMapFile)
            {
                std::lock_guard<std::mutex> lock(myMutex);
                hMapFile = tempMapFile;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (hMapFile)
        {
            hEvent = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, myEventName.c_str());
            LPVOID p_buf = MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, 4096);

            while (isRunning)
            {
                DWORD wait_result = WaitForSingleObject(hEvent, 100);
                if (!isRunning) break;

                if (wait_result == WAIT_OBJECT_0) {
                    std::cout << "\n[Monitor]: " << (const char*)p_buf << std::endl;
                }
            }

            if (p_buf)
            {
                UnmapViewOfFile(p_buf);
            }
        }
    }

    std::atomic<bool> isRunning{ false };
    std::thread monitorThread;
    std::mutex myMutex;
    std::condition_variable myConditionVar;
    HANDLE hMapFile{ nullptr };
    HANDLE hEvent{ nullptr };
    std::string myMapFileName;
    std::string myEventName;
};
#endif

/*!
    \brief This is one monitor for the log file updated from SDK library.
    Client program can use this monitor to work with log file directly.
    Client need to provide the log file name with the full path. If the path is not present, library
    will create those folders for you.
    You can refer to the meshDemo example codes.
*/
class LogMonitor
{
public:
    LogMonitor(const std::string& filepath,
        std::function<void(const std::string&)> callback)
        : logFileName(filepath), myCallback(std::move(callback)) {}

    void start()
    {
        if (isRunning)
        {
            return;
        }
        isRunning = true;
        monitorThread = std::thread(&LogMonitor::watch, this);
    }

    void stop()
    {
        isRunning = false;
        if (monitorThread.joinable())
        {
            monitorThread.join();
        }
    }

    ~LogMonitor()
    {
        stop();
    }

private:

    void watch()
    {
        while (isRunning)
        {
            std::ifstream file;
            file.open(logFileName, std::ios::ate);

            if (file.is_open())
            {
                std::streampos last_pos = file.tellg();
                std::string line;

                while (isRunning)
                {
                    file.clear();
                    file.seekg(last_pos);

                    bool new_data = false;
                    while (std::getline(file, line))
                    {
                        myCallback(line);
                        last_pos = file.tellg();
                        new_data = true;
                    }

                    if (!new_data)
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                }
            }

            if (isRunning)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        }
    }

    std::string logFileName;
    std::function<void(const std::string&)> myCallback;
    std::atomic<bool> isRunning{ false };
    std::thread monitorThread;
};