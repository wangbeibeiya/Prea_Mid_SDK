// Revision: $Id: postProcessAccumulatedCurveDemo.cpp,v 1.0 2025/05/08 14:20:05 Leon Exp $
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
#include <postProcess/pfAccumulatedCurveCoefficientSetting.h>

void PostProcessDemo::accumulatedCurveDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_BASE_NAMESPACE::PFStatus status = createAccumulatedCurveBySetting(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Create accumulated curve failed.\n";
        std::cout << "==============================================================\n";
        return;
    }
    else
    {
        std::cout << "Create accumulated curve is successful.\n";
        std::cout << "==============================================================\n";
    }

    status = editAccumulatedCurve(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Edit accumulated curve failed.\n";
        return;
    }
    else
    {
        std::cout << "Edit accumulated curve is successful.\n";
        std::cout << "==============================================================\n";
    }

    status = renameAccumulatedCurve(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Rename accumulated curve failed.\n";
        return;
    }
    else
    {
        std::cout << "Rename accumulated curve is successful.\n";
        std::cout << "==============================================================\n";
    }

    status = deleteAccumulatedCurve(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Delete accumulated curve failed.\n";
        return;
    }
    else
    {
        std::cout << "Delete accumulated curve is successful.\n";
        std::cout << "==============================================================\n";
    }
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createAccumulatedCurveBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFAccumulatedCurveCoefficientSetting setting("acCoefSetting1");
    setting.setLocations("boundary,Pipetee/walls");
    setting.setDirection(1.0, 0.0, 0.0);
    setting.setDivisionCount(10);
    setting.setFunctionType(PREPRO_POSTPROCESS_NAMESPACE::PFAccumulatedCurveFunctionType::EForceCoefficient);
    setting.setReferenceArea(1.1357);
    setting.setReferenceDensity(1.225);
    setting.setReferenceVelocity(0.97694);

    PREPRO_POSTPROCESS_NAMESPACE::PFAccumulatedCurveData result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createAccumulatedCurve(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create Accumulated Curve (Coefficient).. ---> " << setting.getName() << std::endl;
        std::cout << "==============================================================\n";
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    else
    {
        std::cout << "Create Accumulated Curve (Coefficient) is successful. ---> " << setting.getName() << std::endl;
        std::cout << "==============================================================\n";
    }

    int divCount = result.getDivisionCount();
    std::cout << "Result list size: " << divCount << std::endl;
    std::cout << "Result of Force Data: ";
    for (int i = 0; i < divCount; ++i)
    {
        std::cout << result.getCurveForceData()[i];
        if (i < divCount - 1)
        {
           std::cout << ", ";
        }
    }
    std::cout << std::endl;

    std::cout << "Result of Length Data: ";
    for (int i = 0; i < divCount; ++i)
    {
        std::cout << result.getCurveLengthData()[i];
        if (i < divCount - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;

    // ###############################
    // create curve of Force type.
    PREPRO_POSTPROCESS_NAMESPACE::PFAccumulatedCurveForceSetting settingFoce("acForceSetting1");
    settingFoce.setLocations("boundary,Pipetee/walls");
    settingFoce.setDirection(1.0, 0.0, 0.0);
    settingFoce.setDivisionCount(10);
    settingFoce.setFunctionType(PREPRO_POSTPROCESS_NAMESPACE::PFAccumulatedCurveFunctionType::EForce);

    PREPRO_POSTPROCESS_NAMESPACE::PFAccumulatedCurveData resultForce;
    status = postProcess->createAccumulatedCurve(&settingFoce, resultForce);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create Accumulated Curve (Force). ---> " << settingFoce.getName() << std::endl;
        std::cout << "==============================================================\n";
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    else
    {
        std::cout << "Create Accumulated Curve (Force) is successful. ---> " << settingFoce.getName() << std::endl;
        std::cout << "==============================================================\n";
    }

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editAccumulatedCurve(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFAccumulatedCurveCoefficientSetting setting("acCoefSetting1");
    setting.setLocations("boundary,Pipetee/walls");
    setting.setDivisionCount(15);
    setting.setFunctionType(PREPRO_POSTPROCESS_NAMESPACE::PFAccumulatedCurveFunctionType::EPressureForceCoefficient);
    setting.setReferenceArea(1.21357);
    setting.setReferenceDensity(1.3225);
    setting.setReferenceVelocity(0.897694);

    PREPRO_POSTPROCESS_NAMESPACE::PFAccumulatedCurveData result;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editAccumulatedCurve(&setting, result);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit Accumulated Curve (Coefficient). ---> " << setting.getName() << std::endl;
        std::cout << "==============================================================\n";
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    else
    {
        std::cout << "Edit Accumulated Curve (Coefficient) is successful. ---> " << setting.getName() << std::endl;
        std::cout << "==============================================================\n";
    }

    int divCount = result.getDivisionCount();
    std::cout << "Result list size: " << divCount << std::endl;
    std::cout << "Result of Force Data: ";
    for (int i = 0; i < divCount; ++i)
    {
        std::cout << result.getCurveForceData()[i];
        if (i < divCount - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;

    std::cout << "Result of Length Data: ";
    for (int i = 0; i < divCount; ++i)
    {
        std::cout << result.getCurveLengthData()[i];
        if (i < divCount - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getAccumulatedCurveSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get Accumulated Curve (Coefficient): ---> " << name << std::endl;
        std::cout << "==============================================================\n";
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    else
    {
        std::cout << "Get Accumulated Curve (Coefficient) is successful. ---> " << name << std::endl;
        std::cout << "==============================================================\n";
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFAccumulatedCurveCoefficientSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFAccumulatedCurveCoefficientSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    std::cout << "Accumulated Curve (Coefficient)  name:" << currentSetting->getName() << std::endl;
    std::cout << "Accumulated Curve (Coefficient)  type:" << (int)currentSetting->getType() << std::endl;
    std::cout << "Accumulated Curve (Coefficient)  locations:" << currentSetting->getLocations() << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameAccumulatedCurve(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->renameAccumulatedCurve("acCoefSetting1", "acCoefSetting1_rename");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return getAccumulatedCurveSetting(postProcess, "acCoefSetting1_rename");
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::deleteAccumulatedCurve(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    auto status = postProcess->deleteAccumulatedCurve("acCoefSetting1_rename");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting("acCoefSetting1_rename");
    if (nullptr != postSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}