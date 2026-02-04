// Revision: $Id: postProcessDPMTrajectoryDemo.cpp,v 1.0 2025/05/14 13:50:05 Leon Exp $
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
#include <base/pfGroupData.h>
#include <base/common/pfEnumeration.h>
#include <postProcess/PFPostProcessBuilder.h>
#include <postprocess/pfPostProcess.h>
#include <postProcess/common/pfEnumeration.h>
#include <postProcess/pfDPMTrajectorySetting.h>
#include "../renderDemo.h"

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::dpmTrajectoryDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& examplePath)
{
    std::string cgnsPath = examplePath + "postProcess\\DPM-79.cgns";
    loadResult(cgnsPath, postProcess);

    std::string filePath = examplePath + "postProcess";

    PREPRO_BASE_NAMESPACE::PFStatus status = createDPMTrajectoryBySetting(postProcess, filePath);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\nCreate DPM Trajectory failed.\n";
        return status;
    }

    // edit DPM Trajectory with new file setting.
    status = editDPMTrajectory(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\nEdit DPM Trajectory failed.\n";
        return status;
    }
    else
    {
        std::cout << "\nEdit DPM Trajectory is successful.\n\n";
    }

    // rename existing DPM Trajectory with new name.
    status = renameDPMTrajectory(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\nRename DPM Trajectory failed.\n\n";
        return status;
    }
    else
    {
        std::cout << "\nRename DPM Trajectory is successful.\n\n";
    }

    // delete existing DPM Trajectory.
    status = deleteDPMTrajectory(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "\nDelete DPM Trajectory failed.\n\n";
        return status;
    }
    else
    {
        std::cout << "\nDelete DPM Trajectory is successful.\n\n";
    }

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createDPMTrajectoryBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& filePath)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFDPMTrajectorySetting setting("dpmTrajectorySetting1");

    PREPRO_POSTPROCESS_NAMESPACE::PFDPMData dpmData;
    loadDPMTrajectory(postProcess, filePath.c_str(), dpmData);

    int domainCount = dpmData.getDomainCount();
    PREPRO_POSTPROCESS_NAMESPACE::PFDPMDomain** domains = dpmData.getDomains();
    if (domainCount == 0 || domains == nullptr)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFDPMDomain* domain = domains[0];
    if (domain == nullptr)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    std::string domainParam = domain->getDomainName();
    setting.setDomains(domainParam.c_str());

    PREPRO_POSTPROCESS_NAMESPACE::PFDPMInjection* injection = domain->getInjection();
    int injectionCount = injection->getInjectionCount();
    if (injectionCount == 0)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    if (!dpmData.getIsSteady())
    {
        setting.setFlowTime(10.0);
    }
    
    PREPRO_POSTPROCESS_NAMESPACE::PFDPMInjectionData* injectionData = injection->getInjectionData(0);

    setting.setInjections(injectionData->getInjectionDataName());
    setting.setSamplingRate(1.0);

    PREPRO_BASE_NAMESPACE::PFData pfData;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createDPMTrajectory(&setting, pfData);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create DPM Trajectory." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    RenderDemo::show(pfData, VisualizationType::EPostProcess);

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editDPMTrajectory(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFDPMTrajectorySetting setting("dpmTrajectorySetting1");
    setting.setDomains("volume,volume1");
    setting.setInjections("injection1");
    setting.setSamplingRate(1.0);

    PREPRO_BASE_NAMESPACE::PFData pfData;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editDPMTrajectory(&setting, pfData);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit DPM Trajectory." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    RenderDemo::show(pfData, VisualizationType::EPostProcess);

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getDPMTrajectoryResult(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get DPM Trajectory. " << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFDPMTrajectorySetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFDPMTrajectorySetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    std::cout << "\n\n===============  Get DPM Trajectory info. =====================\n\n";
    std::cout << "DPM Trajectory name:" << currentSetting->getName() << std::endl;
    std::cout << "DPM Trajectory type:" << (int)currentSetting->getType() << std::endl;
    std::cout << "DPM Trajectory domains:" << currentSetting->getDomains() << std::endl;
    std::cout << "DPM Trajectory injections: " << currentSetting->getInjections() << std::endl;
    std::cout << "DPM Trajectory sampling rate: " << currentSetting->getSamplingRate() << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameDPMTrajectory(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->renameDPMTrajectory("dpmTrajectorySetting1", "dpmTrajectorySetting1_rename");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return getDPMTrajectoryResult(postProcess, "dpmTrajectorySetting1_rename");
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::deleteDPMTrajectory(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->deleteDPMTrajectory("dpmTrajectorySetting1_rename");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting("dpmTrajectorySetting1_rename");
    if (nullptr != postSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}