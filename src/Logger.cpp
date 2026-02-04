#include "../include/Logger.h"
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <ctime>
#else
#include <ctime>
#endif

Logger::Logger()
    : m_logFilePath()
{
}

Logger::~Logger()
{
    closeLogFile();
}

bool Logger::initializeLogFile(const std::string& logFilePath)
{
    try
    {
        // 如果已有日志文件打开，先关闭
        if (m_logFile.is_open())
        {
            m_logFile.close();
        }
        
        // 创建目录（如果不存在）
        std::filesystem::path logPath(logFilePath);
        std::filesystem::path logDir = logPath.parent_path();
        if (!logDir.empty() && !std::filesystem::exists(logDir))
        {
            std::filesystem::create_directories(logDir);
        }
        
        // 打开日志文件（重写模式，每次执行都清空旧内容）
        m_logFile.open(logFilePath, std::ios::out | std::ios::trunc);
        if (!m_logFile.is_open())
        {
            std::cerr << "警告: 无法打开日志文件: " << logFilePath << std::endl;
            return false;
        }
        
        m_logFilePath = logFilePath;
        
#ifdef _DEBUG
        std::cout << "[Logger] 日志文件已打开: " << logFilePath << std::endl;
#endif
        
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "警告: 初始化日志文件失败: " << e.what() << std::endl;
        return false;
    }
}

void Logger::closeLogFile()
{
    if (m_logFile.is_open())
    {
        m_logFile.close();
#ifdef _DEBUG
        std::cout << "[Logger] 日志文件已关闭: " << m_logFilePath << std::endl;
#endif
        m_logFilePath.clear();
    }
}

void Logger::logOutput(const std::string& message) const
{
#ifdef _DEBUG
    // Debug模式下同时输出到控制台和日志文件
    std::cout << message << std::endl;
    if (m_logFile.is_open())
    {
        m_logFile << message << std::endl;
        m_logFile.flush();
    }
#else
    // Release模式下只输出到日志文件
    if (m_logFile.is_open())
    {
        m_logFile << message << std::endl;
        m_logFile.flush();
    }
    else
    {
        // 如果日志文件未打开，输出到控制台
        std::cout << message << std::endl;
    }
#endif
}

void Logger::logOutputLine(const std::string& message) const
{
#ifdef _DEBUG
    // Debug模式下同时输出到控制台和日志文件
    std::cout << message << std::endl;
    if (m_logFile.is_open())
    {
        m_logFile << message << std::endl;
        m_logFile.flush();
    }
#else
    // Release模式下只输出到日志文件
    if (m_logFile.is_open())
    {
        m_logFile << message << std::endl;
        m_logFile.flush();
    }
    else
    {
        // 如果日志文件未打开，输出到控制台
        std::cout << message << std::endl;
    }
#endif
}

std::string Logger::getCurrentTimeString() const
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    
#ifdef _WIN32
    localtime_s(&tm_buf, &time_t);
#else
    localtime_r(&time_t, &tm_buf);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

