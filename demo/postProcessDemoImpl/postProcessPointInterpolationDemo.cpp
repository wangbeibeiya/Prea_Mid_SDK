// Revision: $Id: postProcessPointInterpolationDemo.cpp,v 1.0 2025/06/24 10:55:04 liujing Exp $
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
#include "postProcessPointInterpolationDemo.h"
#include <iostream>
#include <postprocess/pfPostProcess.h>
#include <postProcess/pfPointInterpolationSetting.h>
#include "../renderDemo.h"
#include <base/pfGroupData.h>
#include <postProcess/pfPostProcessVariableValues.h>
#include <postProcess/pfVectorSetting.h>
#include <postProcess/pfStreamlineSetting.h>
#include <postProcess/common/pfEnumeration.h>

void PostProcessPointInterpolationDemo::pointInterpolationDemo(const std::string& examplePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return;
    }
    if (createPointInterpolation(postProcess) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create point collection." << std::endl;
        return;
    }
    if (editPointInterpolation(postProcess) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit point collection." << std::endl;
        return;
    }
    if (getPointInterpolationSetting(postProcess) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get point collection setting." << std::endl;
        return;
    }
    if (createVector(postProcess) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create vector from point collection." << std::endl;
        return;
    }
    if (createStreamline(postProcess) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create streamline from point collection." << std::endl;
        return;
    }
    if (exportCsv(examplePath, postProcess) != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to export csv file of point collection." << std::endl;
        return;
    }
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessPointInterpolationDemo::createPointInterpolation(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFPointInterpolationSetting setting("PointInterpolation1");
    double points[15] = {
        0.01,0.01,0.01,
        0.01,0.02,0.03,
        0.01,0.01,0.02,
        0.01,0.09,0.2,
        0.02,0.05,0.2,
    };
    setting.setPoints(points, 5);
    PREPRO_BASE_NAMESPACE::PFData pfData;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createObject(&setting, pfData);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create object:" << setting.getName() << std::endl;
        return status;
    }
    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues variableValues;
    status = postProcess->getObjectVariableValues(pfData.getName(), "Velocity_mag", variableValues);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get object variable values:" << setting.getName() << std::endl;
        return status;
    }
    RenderDemo::showResult(pfData, variableValues);
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessPointInterpolationDemo::editPointInterpolation(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFPointInterpolationSetting coordinateSetting("PointInterpolation1");
    double points[60] = {
        0.01561, -0.028576, -0.005184,
        0.001885, 0.022449, -0.032993,
        -0.012755, 0.011916, 0.035021,
        0.043087, -0.002853, 0.030736,
        0.042527, 0.007691, 7.5e-05,
        -0.033153, -0.014188, -0.013368,
        0.039359, 0.023954, 0.011912,
        0.031664, -0.005998, 0.024998,
        -0.003888, 0.047265, 0.010891,
        0.006733, -0.008291, 0.023446,
         0.001679, -0.002667, -0.031003,
         0.038176, 0.011051, 0.006309,
         0.05166, 0.01415, -0.014226,
         -0.008169, 0.032227, 0.011989,
         0.022054, 0.02169, -0.032607,
         0.011922, 0.045946, -0.02075,
         0.001005, -0.035714, -0.004009,
         -0.036874, 0.005536, -0.01522,
         0.020764, 0.004297, 0.015898,
         -0.015002, 0.011732, 0.030045,
    };
    coordinateSetting.setPoints(points, 20);
    PREPRO_BASE_NAMESPACE::PFData pfData;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editObject(&coordinateSetting, pfData);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit object:" << coordinateSetting.getName() << std::endl;
        return status;
    }
    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues variableValues;
    status = postProcess->getObjectVariableValues(pfData.getName(), "Velocity_mag", variableValues);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get object variable values:" << coordinateSetting.getName() << std::endl;
        return status;
    }
    RenderDemo::showResult(pfData, variableValues);
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessPointInterpolationDemo::getPointInterpolationSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    std::string name = "PointInterpolation1";
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get object setting:" << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    if(postSetting->getType() == PREPRO_POSTPROCESS_NAMESPACE::PFSettingType::EPointInterpolation)
    {
        const PREPRO_POSTPROCESS_NAMESPACE::PFPointInterpolationSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFPointInterpolationSetting*>(postSetting);
        if (nullptr == currentSetting)
        {
            return PREPRO_BASE_NAMESPACE::PFStatus::EError;
        }
        std::cout << "point collection name:" << currentSetting->getName() << std::endl;
        double* points = currentSetting->getPoints();
        unsigned int number = currentSetting->getPointsNumber();
        for (unsigned int i = 0; i < number - 2;)
        {
            std::cout << "point collection point: X:" << points[i++] << "  Y:" << points[i++] << "  Z:" << points[i++] << std::endl;
        }
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessPointInterpolationDemo::createVector(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    PREPRO_POSTPROCESS_NAMESPACE::PFVectorSetting setting("vectorFromPointInterpolation");
    setting.setDomains("volume,Pipetee");
    setting.setLocations("point,pointCollection1");
    setting.setVector("Velocity");
    setting.setScaleFactor(0.1);
    setting.setWidthSize(0.1);
    setting.setArrowHeadSize(0.35);
    PFPostProcess::VectorScaleVariable vectorScaleVariable = PFPostProcess::VectorScaleVariable::Magnitude;
    setting.setVectorScaleVariable(vectorScaleVariable);
    PREPRO_BASE_NAMESPACE::PFData pfData;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createObject(&setting, pfData);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create object." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues variableValues;
    status = postProcess->getObjectVariableValues(pfData.getName(), "Pressure", variableValues);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get object variable values." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    RenderDemo::showResult(pfData, variableValues);
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}
PREPRO_BASE_NAMESPACE::PFStatus PostProcessPointInterpolationDemo::createStreamline(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFStreamlineSetting setting("streamlineFromPointInterpolation");
    setting.setDomains("volume,Pipetee");
    setting.setLocations("point,PointInterpolation1");
    setting.setVector("Velocity");
    setting.setDirection(PREPRO_POSTPROCESS_NAMESPACE::StreamlineDirection::Forward);
    PREPRO_BASE_NAMESPACE::PFData pfData;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createObject(&setting, pfData);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create object:" << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues variableValues;
    status = postProcess->getObjectVariableValues(pfData.getName(), "Velocity_mag", variableValues);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get object variable values." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    RenderDemo::showResult(pfData, variableValues);
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessPointInterpolationDemo::exportCsv(const std::string& examplePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    //load result
    std::string filePath = examplePath + "\\postProcess\\pointCollctionData.csv";
    const char* locations[] = { "PointInterpolation1" };
    const char* variables[] = { "Density","Pressure","Velocity_mag","HeatFlux"};
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->exportCsvFile(filePath.c_str(), locations, 1, variables, 4, 8);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to export csv file" << std::endl;
    }
    return status;
}