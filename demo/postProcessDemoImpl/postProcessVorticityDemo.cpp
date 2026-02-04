// Revision: $Id: postProcessVorticityDemo.cpp,v 1.0 2025/04/24 15:32:20 Leon Exp $
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
#include <base/pfGroupData.h>
#include <postProcess/PFPostProcessBuilder.h>
#include <postprocess/pfPostProcess.h>
#include <postProcess/pfPostProcessVariable.h>
#include "../commonMethods.h"
#include "../renderDemo.h"
#include <postProcess/pfPostProcessVariableValues.h>
#include <postProcess/common/pfEnumeration.h>
#include <postProcess/pfVorticitySetting.h>
#include <postProcess/common/pfEnumeration.h>

void PostProcessDemo::vorticityDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_BASE_NAMESPACE::PFStatus status = createVorticity(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create vorticity." << std::endl;
        return;
    }
    status = renameVorticity(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename vorticity." << std::endl;
        return;
    }
    status = deleteVorticity(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete vorticity." << std::endl;
        return;
    }

    // since we have deleted the first vorticity that we created, it should be nothing when we try to get settings.
    status = getVorticitySetting(postProcess, "VorticityDemo1Rename");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get setting of vorticity." << std::endl;
        return;
    }
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createVorticity(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFVorticitySetting setting("VorticityDemo1");

    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariables allVariables;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createCustomVariable(&setting, allVariables);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create custom variable." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    printVariableInfo(allVariables);

    PREPRO_BASE_NAMESPACE::PFData pfData;
    status = postProcess->getObjectData("sliceDemo1", pfData);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get object data." << std::endl;
        return status;
    }
    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues variableValues;
    status = postProcess->getObjectVariableValues("sliceDemo1", "VorticityDemo1_x", variableValues);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get object variable values." << std::endl;
        return status;
    }
    RenderDemo::showResult(pfData, variableValues);
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameVorticity(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    std::string oldName = "VorticityDemo1";
    std::string newName = "VorticityDemo1Rename";
    //rename
    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariables allVariables;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->renameCustomVariable(oldName.c_str(), newName.c_str(), allVariables);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename custom variable." << std::endl;
        return status;
    }
    printVariableInfo(allVariables);
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::deleteVorticity(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{

	PREPRO_POSTPROCESS_NAMESPACE::PFVorticitySetting setting("VorticityDemo2");
	setting.setMethod(PREPRO_POSTPROCESS_NAMESPACE::PFVorticityMethod::EQCriteria);
	PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariables allVariables;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createCustomVariable(&setting, allVariables);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create custom variable." << setting.getName() << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	printVariableInfo(allVariables);
    status = postProcess->deleteCustomVariable("VorticityDemo2", allVariables);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete custom variable." << std::endl;
        return status;
    }
    printVariableInfo(allVariables);
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getVorticitySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get object info:" << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFVorticitySetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFVorticitySetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "vorticity setting name:" << currentSetting->getName() << std::endl;

    PREPRO_POSTPROCESS_NAMESPACE::PFVorticityMethod method = currentSetting->getMethod();
    if (method == PREPRO_POSTPROCESS_NAMESPACE::PFVorticityMethod::EVorticity)
    {
        std::cout << "vorticity setting method:  Vorticity.\n";
    }
    else if (method == PREPRO_POSTPROCESS_NAMESPACE::PFVorticityMethod::EQCriteria)
    {
        std::cout << "vorticity setting method:  Q Criteria.\n";
    }

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}