#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <iostream>

/**
 * @brief 日志管理类
 * 
 * 负责日志文件的创建、写入和管理
 * - Debug模式下：同时输出到控制台和日志文件
 * - Release模式下：只输出到日志文件
 */
class Logger {
public:
    /**
     * @brief 构造函数
     */
    Logger();
    
    /**
     * @brief 析构函数
     */
    ~Logger();
    
    /**
     * @brief 初始化日志文件
     * @param logFilePath 日志文件路径
     * @return 是否成功初始化
     */
    bool initializeLogFile(const std::string& logFilePath);
    
    /**
     * @brief 关闭日志文件
     */
    void closeLogFile();
    
    /**
     * @brief 输出日志消息（带换行）
     * @param message 消息内容
     */
    void logOutput(const std::string& message) const;
    
    /**
     * @brief 输出日志消息（带换行，统一输出函数）
     * @param message 消息内容
     */
    void logOutputLine(const std::string& message) const;
    
    /**
     * @brief 获取当前时间字符串
     * @return 格式化的时间字符串
     */
    std::string getCurrentTimeString() const;
    
    /**
     * @brief 检查日志文件是否已打开
     * @return 是否已打开
     */
    bool isLogFileOpen() const { return m_logFile.is_open(); }
    
    /**
     * @brief 获取日志文件路径
     * @return 日志文件路径
     */
    std::string getLogFilePath() const { return m_logFilePath; }

private:
    mutable std::ofstream m_logFile;    ///< 日志文件输出流
    std::string m_logFilePath;          ///< 日志文件路径
};

#endif // LOGGER_H

