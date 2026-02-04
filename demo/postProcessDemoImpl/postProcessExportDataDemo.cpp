// Revision: $Id: postProcessExportDataDemo.cpp,v 1.0 2025/05/09 12:55:04 liujing Exp $
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
#include "postProcessExportDataDemo.h"
#include <iostream>
#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <postProcess/PFPostProcessBuilder.h>
#include <postprocess/pfPostProcess.h>
#include "../postProcessDemo.h"

void PostProcessExportDataDemo::exportDataDemo(const std::string& examplePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{ 
    exportCsvFileDemo(examplePath, postProcess);
    exportDatFileDemo(examplePath, postProcess);
}
PREPRO_BASE_NAMESPACE::PFStatus PostProcessExportDataDemo::exportCsvFileDemo(const std::string& examplePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    //load result
    std::string filePath = examplePath + "\\postProcess\\data.csv";
    const char* locations[] = { "Pipetee", "Pipetee/z-inlet","Pipetee/y-inlet","Pipetee/walls","Pipetee/z-outlet",
        "line_1","line_2","point_1","point_2","AccCurve1","slice_2","clip_1" ,"contour_1","streamline_1","vector_1" };
    const char* variables[] = { "Pressure","Velocity_mag" };
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->exportCsvFile(filePath.c_str(), locations, 15, variables, 2, 8);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to export csv file" << std::endl;
    }
    return status;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessExportDataDemo::exportDatFileDemo(const std::string& examplePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    //load result
    std::string filePath = examplePath + "\\postProcess\\data.dat";
    const char* locations[] = { "Pipetee", "Pipetee/z-inlet","Pipetee/y-inlet","Pipetee/walls","Pipetee/z-outlet"};
    const char* variables[] = { "Pressure","Velocity_mag" };
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->exportDatFile(filePath.c_str(), locations, 5, variables, 2, 8, PREPRO_POSTPROCESS_NAMESPACE::PFResultValueType::ECellCenterValue);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to export dat file" << std::endl;
    }
    return status;
}