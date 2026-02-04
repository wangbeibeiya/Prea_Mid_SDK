// Revision: $Id: postProcessPointInterpolationDemo.h,v 1.0 2025/06/24 10:55:04 liujing Exp $
/*
 * PeraGlobal CONFIDENTIAL
 * __________________
 *
 *  [2007] - [2025] PeraGlobal Technologies, Inc.
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

END_PREPRO_POSTPROCESS_NAMESPACE

class PostProcessPointInterpolationDemo
{
public:

    static void pointInterpolationDemo(const std::string& examplePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

private:
    static PREPRO_BASE_NAMESPACE::PFStatus createPointInterpolation(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus editPointInterpolation(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getPointInterpolationSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createVector(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createStreamline(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus exportCsv(const std::string& examplePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
};