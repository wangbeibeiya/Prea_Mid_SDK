// Revision: $Id: common.cpp,v 1.0 2025/02/25 13:15:20 zhiqiang.chen Exp $
/*
 * PeraGlobal CONFIDENTIAL
 * __________________
 *
 *  [2007] - [2021] PeraGlobal Technologies, Inc.
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of PeraGlobal Technologies, Inc.
 * The intellectual and technical concepts contained
 * herein are proprietary to PeraGlobal Technologies, Inc.
 * and may be covered by China. and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from PeraGlobal Technologies, Inc.
 */

#include "commonMethods.h"

std::string CommonMethods::getCurrentProgramPath(const std::string& path)
{
    if (path.empty())
    {
        return "";
    }
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos)
    {
        pos = path.find_last_of('\\');
        if (pos == std::string::npos)
        {
            return "";
        }
    }
    return path.substr(0, pos);
}