#ifndef THIRDPARTYSDKWRAPPER_H
#define THIRDPARTYSDKWRAPPER_H

#include <string>
#include <vector>

// 这里包含第三方SDK的头文件
// 根据实际的SDK头文件路径进行调整
// #include "third_party_sdk_header.h"

/**
 * @brief 第三方SDK包装器类
 * 
 * 提供对第三方SDK功能的封装和集成
 */
class ThirdPartySDKWrapper {
public:
    /**
     * @brief 构造函数
     */
    ThirdPartySDKWrapper();
    
    /**
     * @brief 析构函数
     */
    ~ThirdPartySDKWrapper();
    
    /**
     * @brief 初始化第三方SDK
     * @param configPath 配置文件路径
     * @return 是否初始化成功
     */
    bool initialize(const std::string& configPath = "");
    
    /**
     * @brief 处理数据
     * @param inputData 输入数据
     * @return 处理结果
     */
    std::vector<double> processData(const std::vector<double>& inputData);
    
    /**
     * @brief 获取SDK版本信息
     * @return 版本字符串
     */
    std::string getVersion() const;
    
    /**
     * @brief 检查SDK是否可用
     * @return 是否可用
     */
    bool isAvailable() const;
    
    /**
     * @brief 清理资源
     */
    void cleanup();

private:
    bool m_initialized;
    std::string m_version;
    
    // 这里可以添加第三方SDK的句柄或对象
    // ThirdPartySDKHandle* m_sdkHandle;
};

#endif // THIRDPARTYSDKWRAPPER_H 