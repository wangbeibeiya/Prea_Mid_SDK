// Revision: $Id: postProcessISOClipDemo.cpp,v 1.0 2025/04/23 17:35:20 Leon Exp $
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
#include <postProcess/pfISOClipSetting.h>

void PostProcessDemo::isoClipDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_BASE_NAMESPACE::PFStatus status = createISOClipBySetting(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create ISO Clip." << std::endl;
		return;
	}
	status = editISOClip(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit ISO Clip." << std::endl;
		return;
	}
	status = getISOClipSetting(postProcess, "ISOClipDemo1");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of ISO Clip." << std::endl;
		return;
	}
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createISOClipBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	if (nullptr == postProcess)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	PREPRO_POSTPROCESS_NAMESPACE::PFISOClipSetting setting("ISOClipDemo1");
	setting.setDomains("volume,Pipetee");
	setting.setLocations("boundary,Pipetee/z-outlet,slice,sliceDemo1");
	setting.setMaxClipRangeY(0.1);
	setting.setMaxClipRangeZ(0.1);

	PREPRO_BASE_NAMESPACE::PFData pfData;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createObject(&setting, pfData);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create object." << setting.getName() << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues variableValues;
	status = postProcess->getObjectVariableValues(pfData.getName(), "Velocity_mag", variableValues);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get object variable values." << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	RenderDemo::showResult(pfData, variableValues);
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editISOClip(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_POSTPROCESS_NAMESPACE::PFISOClipSetting setting("ISOClipDemo1");
	setting.setDomains("volume,Pipetee");
	setting.setLocations("boundary,Pipetee/z-outlet,slice,sliceDemo1");
	setting.setMaxClipRangeY(0.3);
	setting.setMaxClipRangeZ(0.3);

	PREPRO_BASE_NAMESPACE::PFData pfData;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editObject(&setting, pfData);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit object." << setting.getName() << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues variableValues;
	status = postProcess->getObjectVariableValues(pfData.getName(), "Velocity_mag", variableValues);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get object variable values." << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	RenderDemo::showResult(pfData, variableValues);
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getISOClipSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	if (nullptr == postSetting)
	{
		std::cout << "Failed to get object info:" << name << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	const PREPRO_POSTPROCESS_NAMESPACE::PFISOClipSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFISOClipSetting*>(postSetting);
	if (nullptr == currentSetting)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	std::cout << "ISO Clip name:" << currentSetting->getName() << std::endl;
	std::cout << "ISO Clip domains:" << currentSetting->getDomains() << std::endl;
	std::cout << "ISO Clip Range X: [" << currentSetting->getMinClipRangeX() << ", " << currentSetting->getMaxClipRangeX() << "]\n";
	std::cout << "ISO Clip Range Y: [" << currentSetting->getMinClipRangeY() << ", " << currentSetting->getMaxClipRangeY() << "]\n";
	std::cout << "ISO Clip Range Z: [" << currentSetting->getMinClipRangeZ() << ", " << currentSetting->getMaxClipRangeZ() << "]\n";

	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}