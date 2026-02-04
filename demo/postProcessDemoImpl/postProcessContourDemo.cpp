// Revision: $Id: postProcessContourDemo.cpp,v 1.0 2025/04/23 11:15:20 Leon Exp $
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
#include <postProcess/pfContourLineSetting.h>
#include <postProcess/pfContourSurfaceSetting.h>

void PostProcessDemo::contourLineDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_BASE_NAMESPACE::PFStatus status = createContourLineBySetting(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create contour line." << std::endl;
		return;
	}
	status = editContourLine(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit contour line." << std::endl;
		return;
	}
	status = getContourLineSetting(postProcess, "contourLineDemo1");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of contour line." << std::endl;
		return;
	}
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createContourLineBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	if (nullptr == postProcess)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	PREPRO_POSTPROCESS_NAMESPACE::PFContourLineSetting setting("contourLineDemo1");
	setting.setDomains("volume,Pipetee");
	setting.setLocations("boundary,Pipetee/walls,slice,sliceDemo1");
	setting.setVariable("Pressure");
	double values[5] = { -30,0,50,100,130 };
	setting.setValues(5, values);

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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editContourLine(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_POSTPROCESS_NAMESPACE::PFContourLineSetting setting("contourLineDemo1");
	setting.setDomains("volume,Pipetee");
	setting.setLocations("boundary,Pipetee/z-outlet,slice,sliceDemo1");
	setting.setVariable("Pressure");
	double values[2] = { 0.318048, 1.58023 };
	setting.setValues(2, values);

	PREPRO_BASE_NAMESPACE::PFData pfData;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editObject(&setting, pfData);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit object." << setting.getName() << std::endl;
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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getContourLineSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	if (nullptr == postSetting)
	{
		std::cout << "Failed to get object info:" << name << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	const PREPRO_POSTPROCESS_NAMESPACE::PFContourLineSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFContourLineSetting*>(postSetting);
	if (nullptr == currentSetting)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	std::cout << "contour line name:" << currentSetting->getName() << std::endl;
	std::cout << "contour line domains:" << currentSetting->getDomains() << std::endl;
	if (currentSetting->getNumberOfValues() > 1)
	{
		std::cout << "contour line number:" << currentSetting->getNumberOfValues() << std::endl;
	}
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

void PostProcessDemo::contourSurfaceDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_BASE_NAMESPACE::PFStatus status = createContourSurfaceBySetting(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create contour surface." << std::endl;
		return;
	}
	status = editContourSurface(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit contour surface." << std::endl;
		return;
	}
	status = getContourSurfaceSetting(postProcess, "contourSurfaceDemo1");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of contour surface." << std::endl;
		return;
	}
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createContourSurfaceBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	if (nullptr == postProcess)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	PREPRO_POSTPROCESS_NAMESPACE::PFContourSurfaceSetting setting("contourSurfaceDemo1");
	setting.setDomains("volume,Pipetee");
	setting.setVariable("Pressure");
	double values[5] = { -30,0,50,100,130 };
	setting.setValues(5, values);

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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editContourSurface(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_POSTPROCESS_NAMESPACE::PFContourSurfaceSetting setting("contourSurfaceDemo1");
	setting.setDomains("volume,Pipetee");
	setting.setVariable("Pressure");
	double values[2] = { 0.318048, 1.58023 };
	setting.setValues(2, values);

	PREPRO_BASE_NAMESPACE::PFData pfData;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editObject(&setting, pfData);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit object." << setting.getName() << std::endl;
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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getContourSurfaceSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	if (nullptr == postSetting)
	{
		std::cout << "Failed to get object info:" << name << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	const PREPRO_POSTPROCESS_NAMESPACE::PFContourSurfaceSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFContourSurfaceSetting*>(postSetting);
	if (nullptr == currentSetting)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	std::cout << "contour surface name:" << currentSetting->getName() << std::endl;
	std::cout << "contour surface domains:" << currentSetting->getDomains() << std::endl;
	if (currentSetting->getNumberOfValues() > 1)
	{
		std::cout << "contour surface number:" << currentSetting->getNumberOfValues() << std::endl;
	}
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}