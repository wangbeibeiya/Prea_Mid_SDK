// Revision: $Id: postProcessIntegralDemo.cpp,v 1.0 2025/04/24 11:15:20 cuianzhu Exp $
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
#include <postProcess/pfAreaWeightedAverageIntegralSetting.h>
#include <postProcess/pfVolumeWeightedAverageIntegralSetting.h>
#include <postProcess/pfMassWeightedAverageIntegralSetting.h>
#include <postProcess/pfMassFlowRateIntegralSetting.h>
#include <postProcess/pfForceIntegralSetting.h>
#include <postProcess/pfCentroidIntegralSetting.h>

static const char* areaWeightedAverageIntegralName = "areaWeightedAverageIntegral1";
static const char* areaWeightedAverageIntegralName2 = "areaWeightedAverageIntegral2";
static const char* volumeWeightedAverageIntegralName = "volumeWeightedAverageIntegral1";
static const char* volumeWeightedAverageIntegralName2 = "volumeWeightedAverageIntegral2";
static const char* massWeightedAverageIntegralName = "massWeightedAverageIntegral1";
static const char* massWeightedAverageIntegralName2 = "massWeightedAverageIntegral2";
static const char* massFlowRateIntegralName = "massFlowRateIntegral1";
static const char* massFlowRateIntegralName2 = "massFlowRateIntegral2";
static const char* forceIntegralName = "forceIntegral1";
static const char* forceIntegralName2 = "forceIntegral2";
static const char* centroidIntegralName = "centroidIntegral1";
static const char* centroidIntegralName2 = "centroidIntegral2";


void PostProcessDemo::integralDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_BASE_NAMESPACE::PFStatus status = PREPRO_BASE_NAMESPACE::PFStatus::EError;
    std::cout << "-----------------------------------------Start Integral Demo." << std::endl;
    std::cout << "-----------------------------------------Start AreaWeightedAverageIntegral Demo." << std::endl;
    std::cout << "1. create area weighted average integral." << std::endl;
    status = createAreaWeightedAverageIntegralBySetting(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create area weighted average integral." << std::endl;
        return;
    }
    std::cout << "2. edit area weighted average integral." << std::endl;
    status = editAreaWeightedAverageIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit area weighted average integral." << std::endl;
        return;
    }
    std::cout << "3. get area weighted average integral." << std::endl;
    status = getAreaWeightedAverageIntegralSetting(postProcess, areaWeightedAverageIntegralName);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get area weighted average integral." << std::endl;
        return;
    }
    std::cout << "4. rename area weighted average integral." << std::endl;
    status = renameAreaWeightedAverageIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename area weighted average integral." << std::endl;
        return;
    }
    std::cout << "5. delete area weighted average integral." << std::endl;
    status = deleteAreaWeightedAverageIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete area weighted average integral." << std::endl;
        return;
    }
    std::cout << "-----------------------------------------End AreaWeightedAverageIntegral Demo." << std::endl;

    std::cout << "-----------------------------------------Start VolumeWeightedAverageIntegral Demo." << std::endl;
    std::cout << "1. create volume weighted average integral." << std::endl;
    status = createVolumeWeightedAverageIntegralBySetting(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create volume weighted average integral." << std::endl;
        return;
    }
    std::cout << "2. edit volume weighted average integral." << std::endl;
    status = editVolumeWeightedAverageIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit volume weighted average integral." << std::endl;
        return;
    }
    std::cout << "3. get volume weighted average integral." << std::endl;
    status = getVolumeWeightedAverageIntegralSetting(postProcess, volumeWeightedAverageIntegralName);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get volume weighted average integral." << std::endl;
        return;
    }
    std::cout << "4. rename volume weighted average integral." << std::endl;
    status = renameVolumeWeightedAverageIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename volume weighted average integral." << std::endl;
        return;
    }
    std::cout << "5. delete volume weighted average integral." << std::endl;
    status = deleteVolumeWeightedAverageIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete volume weighted average integral." << std::endl;
        return;
    }
    std::cout << "-----------------------------------------End VolumeWeightedAverageIntegral Demo." << std::endl;

    std::cout << "-----------------------------------------Start MassWeightedAverageIntegral Demo." << std::endl;
    std::cout << "1. create mass weighted average integral." << std::endl;
    status = createMassWeightedAverageIntegralBySetting(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create mass weighted average integral." << std::endl;
        return;
    }
    std::cout << "2. edit mass weighted average integral." << std::endl;
    status = editMassWeightedAverageIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit mass weighted average integral." << std::endl;
        return;
    }
    std::cout << "3. get mass weighted average integral." << std::endl;
    status = getMassWeightedAverageIntegralSetting(postProcess, massWeightedAverageIntegralName);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get mass weighted average integral." << std::endl;
        return;
    }
    std::cout << "4. rename mass weighted average integral." << std::endl;
    status = renameMassWeightedAverageIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename mass weighted average integral." << std::endl;
        return;
    }
    std::cout << "5. delete mass weighted average integral." << std::endl;
    status = deleteMassWeightedAverageIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete mass weighted average integral." << std::endl;
        return;
    }
    std::cout << "-----------------------------------------End MassWeightedAverageIntegral Demo." << std::endl;

    std::cout << "-----------------------------------------Start MassFlowRateIntegral Demo." << std::endl;
    std::cout << "1. create mass flow rate integral." << std::endl;
    status = createMassFlowRateIntegralBySetting(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create mass flow rate integral." << std::endl;
        return;
    }
    std::cout << "2. edit mass flow rate integral." << std::endl;
    status = editMassFlowRateIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit mass flow rate integral." << std::endl;
        return;
    }
    std::cout << "3. get mass flow rate integral." << std::endl;
    status = getMassFlowRateIntegralSetting(postProcess, massFlowRateIntegralName);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get mass flow rate integral." << std::endl;
        return;
    }
    std::cout << "4. rename mass flow rate integral." << std::endl;
    status = renameMassFlowRateIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename mass flow rate integral." << std::endl;
        return;
    }
    std::cout << "5. delete mass flow rate integral." << std::endl;
    status = deleteMassFlowRateIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete mass flow rate integral." << std::endl;
        return;
    }
    std::cout << "-----------------------------------------End MassFlowRateIntegral Demo." << std::endl;

    std::cout << "-----------------------------------------Start ForceIntegral Demo." << std::endl;
    std::cout << "1. create force integral." << std::endl;
    status = createForceIntegralBySetting(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create force integral." << std::endl;
        return;
    }
    std::cout << "2. edit force integral." << std::endl;
    status = editForceIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit force integral." << std::endl;
        return;
    }
    std::cout << "3. get force integral." << std::endl;
    status = getForceIntegralSetting(postProcess, forceIntegralName);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get force integral." << std::endl;
        return;
    }
    std::cout << "4. rename force integral." << std::endl;
    status = renameForceIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename force integral." << std::endl;
        return;
    }
    std::cout << "5. delete force integral." << std::endl;
    status = deleteForceIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete force integral." << std::endl;
        return;
    }
    std::cout << "-----------------------------------------End ForceIntegral Demo." << std::endl;

    std::cout << "-----------------------------------------Start CentroidIntegral Demo." << std::endl;
    std::cout << "1. create centroid integral." << std::endl;
    status = createCentroidIntegralBySetting(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create centroid integral." << std::endl;
        return;
    }
    std::cout << "2. edit centroid integral." << std::endl;
    status = editCentroidIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit centroid integral." << std::endl;
        return;
    }
    std::cout << "3. get centroid integral." << std::endl;
    status = getCentroidIntegralSetting(postProcess, centroidIntegralName);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get centroid integral." << std::endl;
        return;
    }
    std::cout << "4. rename centroid integral." << std::endl;
    status = renameCentroidIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename centroid integral." << std::endl;
        return;
    }
    std::cout << "5. delete centroid integral." << std::endl;
    status = deleteCentroidIntegral(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete centroid integral." << std::endl;
        return;
    }
    std::cout << "-----------------------------------------End CentroidIntegral Demo." << std::endl;

    std::cout << "-----------------------------------------End Integral Demo." << std::endl;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createAreaWeightedAverageIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFAreaWeightedAverageIntegralSetting setting(areaWeightedAverageIntegralName);
    setting.setLocations("boundary,Pipetee/walls");
    setting.setVariable("Pressure");
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(areaWeightedAverageIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:"<< result2.result << std::endl;
    
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editAreaWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFAreaWeightedAverageIntegralSetting setting(areaWeightedAverageIntegralName);
    setting.setLocations("boundary,Pipetee/z-outlet");
    setting.setVariable("Temperature");
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(areaWeightedAverageIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:" << result2.result << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getAreaWeightedAverageIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get integral info:" << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFAreaWeightedAverageIntegralSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFAreaWeightedAverageIntegralSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Integral name:" << currentSetting->getName() << std::endl;
    std::cout << "Integral type:" << (int)currentSetting->getType() << std::endl;
    std::cout << "Integral locations:" << currentSetting->getLocations() << std::endl;
    std::cout << "Integral variable:" << currentSetting->getVariable() << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameAreaWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->renameIntegral(areaWeightedAverageIntegralName, areaWeightedAverageIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return getAreaWeightedAverageIntegralSetting(postProcess, areaWeightedAverageIntegralName2);
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::deleteAreaWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->deleteIntegral(areaWeightedAverageIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(areaWeightedAverageIntegralName2);
    if (nullptr != postSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createVolumeWeightedAverageIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeWeightedAverageIntegralSetting setting(volumeWeightedAverageIntegralName);
    setting.setLocations("volume,Pipetee");
    setting.setVariable("Pressure");
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(volumeWeightedAverageIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:" << result2.result << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editVolumeWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeWeightedAverageIntegralSetting setting(volumeWeightedAverageIntegralName);
    setting.setLocations("volume,Pipetee");
    setting.setVariable("Temperature");
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(volumeWeightedAverageIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:" << result2.result << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getVolumeWeightedAverageIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get integral info:" << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFVolumeWeightedAverageIntegralSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFVolumeWeightedAverageIntegralSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Integral name:" << currentSetting->getName() << std::endl;
    std::cout << "Integral type:" << (int)currentSetting->getType() << std::endl;
    std::cout << "Integral locations:" << currentSetting->getLocations() << std::endl;
    std::cout << "Integral variable:" << currentSetting->getVariable() << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameVolumeWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->renameIntegral(volumeWeightedAverageIntegralName, volumeWeightedAverageIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return getVolumeWeightedAverageIntegralSetting(postProcess, volumeWeightedAverageIntegralName2);
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::deleteVolumeWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->deleteIntegral(volumeWeightedAverageIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(volumeWeightedAverageIntegralName2);
    if (nullptr != postSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createMassWeightedAverageIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFMassWeightedAverageIntegralSetting setting(massWeightedAverageIntegralName);
    setting.setLocations("boundary,Pipetee/walls");
    setting.setVariable("Pressure");
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(massWeightedAverageIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:" << result2.result << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editMassWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFMassWeightedAverageIntegralSetting setting(massWeightedAverageIntegralName);
    setting.setLocations("boundary,Pipetee/z-outlet");
    setting.setVariable("Temperature");
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(massWeightedAverageIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:" << result2.result << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getMassWeightedAverageIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get integral info:" << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFMassWeightedAverageIntegralSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFMassWeightedAverageIntegralSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Integral name:" << currentSetting->getName() << std::endl;
    std::cout << "Integral type:" << (int)currentSetting->getType() << std::endl;
    std::cout << "Integral locations:" << currentSetting->getLocations() << std::endl;
    std::cout << "Integral variable:" << currentSetting->getVariable() << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameMassWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->renameIntegral(massWeightedAverageIntegralName, massWeightedAverageIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return getMassWeightedAverageIntegralSetting(postProcess, massWeightedAverageIntegralName2);
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::deleteMassWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->deleteIntegral(massWeightedAverageIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(massWeightedAverageIntegralName2);
    if (nullptr != postSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createMassFlowRateIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFMassFlowRateIntegralSetting setting(massFlowRateIntegralName);
    setting.setLocations("boundary,Pipetee/z-inlet");
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(massFlowRateIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:" << result2.result << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editMassFlowRateIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFMassFlowRateIntegralSetting setting(massFlowRateIntegralName);
    setting.setLocations("boundary,Pipetee/z-outlet");
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(massFlowRateIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:" << result2.result << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getMassFlowRateIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get integral info:" << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFMassFlowRateIntegralSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFMassFlowRateIntegralSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Integral name:" << currentSetting->getName() << std::endl;
    std::cout << "Integral type:" << (int)currentSetting->getType() << std::endl;
    std::cout << "Integral locations:" << currentSetting->getLocations() << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameMassFlowRateIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->renameIntegral(massFlowRateIntegralName, massFlowRateIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return getMassFlowRateIntegralSetting(postProcess, massFlowRateIntegralName2);
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::deleteMassFlowRateIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->deleteIntegral(massFlowRateIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(massFlowRateIntegralName2);
    if (nullptr != postSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createForceIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFForceIntegralSetting setting(forceIntegralName);
    setting.setLocations("boundary,Pipetee/z-inlet");
    double direction[3] = { 0,0,1 };
    setting.setDirection(direction);
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(forceIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:" << result2.result << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editForceIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFForceIntegralSetting setting(forceIntegralName);
    setting.setLocations("boundary,Pipetee/z-outlet");
    double direction[3] = { 0,0,1 };
    setting.setDirection(direction);
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(forceIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:" << result2.result << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getForceIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get integral info:" << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFForceIntegralSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFForceIntegralSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Integral name:" << currentSetting->getName() << std::endl;
    std::cout << "Integral type:" << (int)currentSetting->getType() << std::endl;
    std::cout << "Integral locations:" << currentSetting->getLocations() << std::endl;
    double direction[3] = { 0 };
    currentSetting->getDirection(direction);
    std::cout << "Integral force direction:" << "["<<direction[0]<<"," << direction[1] << "," << direction[2] << "]" << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameForceIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->renameIntegral(forceIntegralName, forceIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return getForceIntegralSetting(postProcess, forceIntegralName2);
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::deleteForceIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->deleteIntegral(forceIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(forceIntegralName2);
    if (nullptr != postSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createCentroidIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFCentroidIntegralSetting setting(centroidIntegralName);
    setting.setLocations("volume,Pipetee");
    setting.setVariable("Pressure");
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    std::cout << "Result centroid:" << "[" << result.centroid[0] << "," << result.centroid[1] << "," << result.centroid[2] << "]" << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(centroidIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:" << result2.result << std::endl;
    std::cout << "Get result centroid:" << "[" << result2.centroid[0] << "," << result2.centroid[1] << "," << result2.centroid[2] << "]" << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editCentroidIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFCentroidIntegralSetting setting(centroidIntegralName);
    setting.setLocations("clip,clip_2");
    setting.setVariable("Temperature");
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editIntegral(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit integral." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Result:" << result.result << std::endl;
    std::cout << "Result centroid:" << "[" << result.centroid[0] << "," << result.centroid[1] << "," << result.centroid[2] << "]" << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::IntegralResult result2;
    status = postProcess->getIntegralResult(centroidIntegralName, result2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get integral results." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Get result:" << result2.result << std::endl;
    std::cout << "Get result centroid:" << "[" << result2.centroid[0] << "," << result2.centroid[1] << "," << result2.centroid[2] << "]" << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getCentroidIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get integral info:" << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFCentroidIntegralSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFCentroidIntegralSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "Integral name:" << currentSetting->getName() << std::endl;
    std::cout << "Integral type:" << (int)currentSetting->getType() << std::endl;
    std::cout << "Integral locations:" << currentSetting->getLocations() << std::endl;
    std::cout << "Integral variable:" << currentSetting->getVariable() << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameCentroidIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->renameIntegral(centroidIntegralName, centroidIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return getCentroidIntegralSetting(postProcess, centroidIntegralName2);
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::deleteCentroidIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->deleteIntegral(centroidIntegralName2);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(centroidIntegralName2);
    if (nullptr != postSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}
