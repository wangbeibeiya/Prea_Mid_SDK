// Revision: $Id: importDemo.h,v 1.0 2025/02/10 16:55:04 xiaming Exp $
/*
 * PeraGlobal CONFIDENTIAL
 * __________________
 *
 *  [2007] - [2021] PeraGlobal Technologies, Inc.
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of PeraGlobal Technoglies, Inc.
 * The intellectual and technical concepts contained
 * herein are proprietary to PeraGlobal Technologies, Inc.
 * and may be covered by China. and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from PeraGlobal Technologies, Inc.
 */
#pragma once

#include <string>

class ImportDemo
{
public:
    static void importDemo(const std::string& demoPath);
	static void importGeometry(const std::string& geometryFilePath);
    static void importPostProcessResult(const std::string& examplePath);

private:
    static void checkSetImportGroupMode(const std::string& examplePath);
    static void setImportSTLOptions(const std::string& examplePath);
};