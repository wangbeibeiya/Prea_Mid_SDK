// Revision: $Id: postProcessFanOperatingPointDemo.cpp,v 1.0 2025/05/08 14:20:05 Leon Exp $
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

#include "../postProcessDemo.h"
#include <iostream>
#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <base/common/pfEnumeration.h>
#include <postProcess/PFPostProcessBuilder.h>
#include <postprocess/pfPostProcess.h>
#include <postProcess/common/pfEnumeration.h>
#include <postProcess/pfFanOperatingPointSetting.h>

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::fanOperatingPointDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& examplePath)
{
    std::string filePath = examplePath + "postProcess\\test_fanpoint.txt";
    PREPRO_BASE_NAMESPACE::PFStatus status = createFanOperatingPointBySetting(postProcess, filePath);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\nCreate fan operating point failed.\n";
        return status;
    }

    // edit fan operating point with new file setting.
    filePath = examplePath + "postProcess\\test_fanpoint_edit.txt";
    status = editFanOperatingPoint(postProcess, filePath);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\nEdit fan operating point failed.\n";
        return status;
    }
    else
    {
        std::cout << "\nEdit fan operating point with new data file path name is successful.\n\n";
    }

    // rename existing fan operating point with new name.
    status = renameFanOperatingPoint(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\nRename fan operating point failed.\n\n";
        return status;
    }
    else
    {
        std::cout << "\nRename fan operating point is successful.\n\n";
    }

    // delete existing fan operating point.
    status = deleteFanOperatingPoint(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\nDelete fan operating point failed.\n\n";
        return status;
    }
    else
    {
        std::cout << "\nDelete fan operating point is successful.\n\n";
    }

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createFanOperatingPointBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& filePath)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFFanOperatingPointSetting setting("fanOperatingPointSetting1");
    setting.setLocation("boundary,Pipetee/y-inlet");
    setting.setFileName(filePath.c_str());

    double pressureResult = 0.0;
    double volumeFlowResult = 0.0;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createFanOperatingPoint(&setting, volumeFlowResult, pressureResult);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create Fan Operating Point." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    std::cout << "\n\nPressure Jump Result value is: " << pressureResult << std::endl;
    std::cout << "Volume FLow Result value is: " << volumeFlowResult << "\n\n";

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editFanOperatingPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& filePath)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFFanOperatingPointSetting setting("fanOperatingPointSetting1");
    setting.setLocation("boundary,Pipetee/y-inlet");
    double pressureResult = 0.0;
    double volumeFlowResult = 0.0;

    setting.setFileName(filePath.c_str());

    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editFanOperatingPoint(&setting, volumeFlowResult, pressureResult);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit Operating Point." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    std::cout << "\n\n===============  After editing with new data file setting. =====================\n\n";
    std::cout << "Pressure Jump Result value is: " << pressureResult << std::endl;
    std::cout << "Volume FLow Result value is: " << volumeFlowResult << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getFanOperatingPointResult(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get Operating Point. " << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFFanOperatingPointSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFFanOperatingPointSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    std::cout << "\n\n===============  Get fan operating point info. =====================\n\n";
    std::cout << "Fan Operating Point name:" << currentSetting->getName() << std::endl;
    std::cout << "Fan Operating Point type:" << (int)currentSetting->getType() << std::endl;
    std::cout << "Fan Operating Point locations:" << currentSetting->getLocation() << std::endl;
    std::cout << "Fan Operating Point file name: " << currentSetting->getFileName() << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameFanOperatingPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->renameFanOperatingPoint("fanOperatingPointSetting1", "fanOperatingPointSetting1_rename");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return getFanOperatingPointResult(postProcess, "fanOperatingPointSetting1_rename");
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::deleteFanOperatingPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->deleteFanOperatingPoint("fanOperatingPointSetting1_rename");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting("fanOperatingPointSetting1_rename");
    if (nullptr != postSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}