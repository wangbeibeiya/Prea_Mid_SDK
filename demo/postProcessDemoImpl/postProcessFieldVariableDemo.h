// Revision: $Id: postProcessFieldVariableDemo.h,v 1.0 2025/05/07 12:55:04 liujing Exp $
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
#pragma once

#include <string>
#include <base/common/pfConstDefinitions.h>
#include <base/common/pfEnumeration.h>
#include <postProcess/common/pfConstDefinitions.h>

START_PREPRO_POSTPROCESS_NAMESPACE
class PFPostProcess;
class PFFieldVariablesSetting;
END_PREPRO_POSTPROCESS_NAMESPACE

class PostProcessFieldVariableDemo
{
public:

    static PREPRO_BASE_NAMESPACE::PFStatus fieldVariableDemo(const std::string& examplePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static void printFieldVariableSetting(const PREPRO_POSTPROCESS_NAMESPACE::PFFieldVariablesSetting& setting);
};