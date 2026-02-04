// Revision: $Id: postProcessFieldVariableDemo.cpp,v 1.0 2025/05/07 12:55:04 liujing Exp $
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
#include "postProcessFieldVariableDemo.h"
#include <iostream>
#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <postProcess/PFPostProcessBuilder.h>
#include <postprocess/pfPostProcess.h>
#include <postProcess/pfFieldVariablesSetting.h>
#include "../postProcessDemo.h"


PREPRO_BASE_NAMESPACE::PFStatus PostProcessFieldVariableDemo::fieldVariableDemo(const std::string& examplePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    //load result
    std::string filePath = examplePath + "postProcess\\hdf5_node_10.cgns";
    PostProcessDemo::loadResult(filePath, postProcess);

    //get info after loading result
    PREPRO_POSTPROCESS_NAMESPACE::PFFieldVariablesSetting setting;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->getFieldVariable(setting);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get field variable." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    printFieldVariableSetting(setting);
    PostProcessDemo::getAllVariables(postProcess);

    //import fps
    filePath = examplePath + "postProcess\\fieldVariable.fps";
    PostProcessDemo::importFpsFile(filePath, postProcess);
    //get info after importing result
    status = postProcess->getFieldVariable(setting);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get field variable." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    printFieldVariableSetting(setting);

    //modify
    setting.setVelocityFieldXComponentByVariable("WallShearStress_x");
    setting.setVelocityFieldYComponentByVariable("WallShearStress_y");
    setting.setVelocityFieldZComponentByVariable("WallShearStress_z");
    setting.setTemperatureFieldByVariable("TurbulentDissipation");
    status = postProcess->setFieldVariable(setting);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to set field variable." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    status = postProcess->getFieldVariable(setting);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get field variable." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    printFieldVariableSetting(setting);
    filePath = examplePath + "postProcess\\fieldVariable_export.fps";
    PostProcessDemo::exportFpsFile(filePath, postProcess);  
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

void PostProcessFieldVariableDemo::printFieldVariableSetting(const PREPRO_POSTPROCESS_NAMESPACE::PFFieldVariablesSetting& setting)
{
    std::cout << "Field variable setting VelocityX:" << setting.getVariableForVelocityFieldXComponent() << std::endl;
    std::cout << "Field variable setting VelocityY:" << setting.getVariableForVelocityFieldYComponent() << std::endl;
    std::cout << "Field variable setting VelocityZ:" << setting.getVariableForVelocityFieldZComponent() << std::endl;
    std::cout << "Field variable setting Pressure:" << setting.getVariableForPressureField() << std::endl;
    std::cout << "Field variable setting Temperature:" << setting.getVariableForTemperatureField() << std::endl;
    std::cout << "Field variable setting Density:" << setting.getVariableForDensityField() << std::endl;
}