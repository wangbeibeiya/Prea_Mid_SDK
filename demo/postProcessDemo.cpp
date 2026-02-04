// Revision: $Id: postProcessDemo.cpp,v 1.0 2025/03/04 13:15:20 liujing Exp $
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

#include "postProcessDemo.h"
#include <iostream>
#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <base/common/pfEnumeration.h>
#include <base/pfGroupData.h>
#include <postProcess/PFPostProcessBuilder.h>
#include <postprocess/pfPostProcess.h>
#include <postProcess/pfPostProcessVariable.h>
#include "commonMethods.h"
#include "renderDemo.h"
#include <postProcess/pfPostProcessVariableValues.h>
#include <postProcess/common/pfEnumeration.h>
#include <postProcess/pfCoordinatePointSetting.h>
#include <postProcess/pfExtremaPointSetting.h>
#include <postProcess/pfSliceSetting.h>
#include <postProcess/pfStreamlineSetting.h>
#include <postProcess/pfCalculatorSetting.h>
#include <postProcess/pfVectorSetting.h>
#include <postProcess/pfContourLineSetting.h>
#include <postProcess/pfClipWithPlaneSetting.h>
#include <postProcess/pfClipWithBoxSetting.h>
#include <postProcess/pfPostProcessData.h>
#include <postProcess/pfLineFromTwoPointsSetting.h>
#include <postProcess/pfLineFromIntersectionSetting.h>
#include "postProcessDemoImpl/postProcessFieldVariableDemo.h"
#include "postProcessDemoImpl/postProcessExportDataDemo.h"
#include "postProcessDemoImpl/postProcessTransformDemo.h"
#include <postProcess/pfDPMData.h>
#include "postProcessDemoImpl/postProcessPointInterpolationDemo.h"

void PostProcessDemo::postProcessDemo(const std::string& examplePath)
{
    /*!
        1.new document,add post process environment
    */
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessBuilder* postProcessBuilder = PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessBuilder::getInstance();
    if (nullptr != postProcessBuilder)
    {
        //add to PFApplication first
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(postProcessBuilder))
        {
            printf_s("No PERA SIM license");
            return;
        }
    }

    PREPRO_BASE_NAMESPACE::PFDocument* pfDocument = pfApplication.newDocument();
    if (nullptr == pfDocument)
    {
        return;
    }
    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess = dynamic_cast<PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess*>(pfDocument->getPostProcessEnvironment());
    if (nullptr == postProcess)
    {
        return;
    }
    int type = 1;
    std::cout << "Select one post process demo type: " << std::endl;
    std::cout << "#### 1. Field variables demo. " << std::endl;
    std::cout << "#### 2. DPM Trajectory demo.\n";
    std::cout << "#### 3. others \n";

    std::cin >> type;
    if (type == 1)
    {
        PostProcessFieldVariableDemo::fieldVariableDemo(examplePath, postProcess);
        return;
    }
    else if (type == 2)
    {
        dpmTrajectoryDemo(postProcess, examplePath);
        return;
    }

    /*!
        2.load result
    */
    std::string filePath = examplePath + "postProcess\\test-06-300.cgns";
    loadResult(filePath, postProcess);

    /*!
        3.get all variables
    */
    getAllVariables(postProcess);

    /*!
        4.get all variables
    */
    getAllBoundriesData(postProcess);
    /*!
        import fps file
    */
    filePath = examplePath + "postProcess\\postProcess.fps";
    importFpsFile(filePath, postProcess);
    /*!
        export data 
    */
    PostProcessExportDataDemo::exportDataDemo(examplePath, postProcess);
	/*!
	    transform demo :create,edit,get setting,rename, delete
	*/
    PostProcessTransformDemo::transformDemo(postProcess);
    /*!
    point interpolation demo:create,edit,get setting,create vector streamline,export
    */
    PostProcessPointInterpolationDemo::pointInterpolationDemo(examplePath, postProcess);
	/*!
        delete object
    */
    deleteObejct(postProcess);
	/*!
		point demo :create,edit,get setting
	*/
	pointDemo(examplePath, postProcess);
	/*!
		rename object
	*/
	renameObject(postProcess);
	/*!
		slice demo :create,edit,get setting
	*/
	sliceDemo(postProcess);
    /*!
		vector demo :create,edit,get setting
	*/
    vectorDemo(postProcess);
	/*!
		calculator demo :create,edit,get setting,delete
	*/
	calculatorDemo(postProcess);
	/*!
	 streamline demo :create,edit,get setting
	*/
	streamlineDemo(postProcess);
	/*!
		contour line demo: create, edit, get setting
		contour surface demo: create, edit, get setting
	*/
	contourLineDemo(postProcess);
	contourSurfaceDemo(postProcess);

    /*!
      line from point demo :create,edit,get setting
     */
    lineFromPointDemo(postProcess);
    /*!
     line from intersection demo :create,edit,get setting
    */
    lineFromIntersectionDemo(postProcess);

	/*!
		ISO Clip demo: create, edit, get setting
	*/
	isoClipDemo(postProcess);

	/*!
	 clip with plane demo :create,edit,get setting
	*/
	clipWithPlaneDemo(postProcess);
	/*!
	 clip with box demo :create,edit,get setting
	*/
	clipWithBoxDemo(postProcess);

	vorticityDemo(postProcess);

    /*!
        Integral demo: creat, edit, get setting.
    */
    integralDemo(postProcess);

    accumulatedCurveDemo(postProcess);

    fanOperatingPointDemo(postProcess, examplePath);
    /*!
        export fps file
    */
    filePath = examplePath + "postprocess.fps";
    exportFpsFile(filePath, postProcess);
}

void PostProcessDemo::loadResult(const std::string& filePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->loadResult(filePath.c_str());
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Result file is loaded successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to load result file." << std::endl;
    }
}


void PostProcessDemo::loadDPMTrajectory(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const char* dpmFilePath, PREPRO_POSTPROCESS_NAMESPACE::PFDPMData& dpmData)
{
    if (nullptr == postProcess)
    {
        return;
    }
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->loadDPMTrajectoryFile(dpmFilePath, dpmData);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "DPM data is loaded successfully." << std::endl;
    }
    else
    {
        std::cout << "Failed to load DPM data file." << std::endl;
        return;
    }

    int domainCount = dpmData.getDomainCount();
    PREPRO_POSTPROCESS_NAMESPACE::PFDPMDomain** domains = dpmData.getDomains();
    if (domains == nullptr)
    {
        std::cout << "\n\nDPM data has wrong data inside.\n";
        return;
    }

    for (int i = 0; i < domainCount; ++i)
    {
        PREPRO_POSTPROCESS_NAMESPACE::PFDPMDomain* domain = domains[i];
        if (domain == nullptr)
        {
            std::cout << "\n\nDPM data has wrong data inside.\n";
            return;
        }
        std::cout << "\n\n===============================================\n";
        std::cout << "Domain Name: " << domain->getDomainName() << std::endl;

        PREPRO_POSTPROCESS_NAMESPACE::PFDPMInjection* injection = domain->getInjection();
        if (injection)
        {
            int injectionCount = injection->getInjectionCount();
            std::cout << "Injection Count: " << injectionCount << std::endl;

            for (int j = 0; j < injectionCount; ++j)
            {
                PREPRO_POSTPROCESS_NAMESPACE::PFDPMInjectionData* injectionData = injection->getInjectionData(j);
                if (injectionData)
                {
                    std::cout << "Injection Data [" << j << "] : " << injectionData->getInjectionDataName() << std::endl;
                }
            }
            std::cout << "\n\n===============================================\n";
        }
    }
}

void PostProcessDemo::getAllVariables(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return;
    }
    /*!
        get all variables
    */
    PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariables variables;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->getAllVariables(variables);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EError)
    {
        std::cout << "Failed to get variables." << std::endl;
        return;
    }

	printVariableInfo(variables);
}

void PostProcessDemo::getAllBoundriesData(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return;
    }
    /*!
        get boundaries data
    */
    PREPRO_BASE_NAMESPACE::PFData pfData;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->getAllBoundriesData(pfData);
    if (status == PREPRO_BASE_NAMESPACE::PFStatus::EError)
    {
        std::cout << "Failed to get boundaries data." << std::endl;
        return;
    }
    RenderDemo::show(pfData, VisualizationType::EPostProcess);
}

void PostProcessDemo::deleteObejct(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->deleteObject("point_1");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete object." << std::endl;
    }
}

void PostProcessDemo::exportFpsFile(const std::string& filePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{

    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->exportFpsFile(filePath.c_str());
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to save script." << std::endl;
    }
}

void PostProcessDemo::importFpsFile(const std::string& filePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    unsigned int objectCount = 0;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->importFpsFile(filePath.c_str(), objectCount);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to import fps file." << std::endl;
        return;
    }
    PREPRO_POSTPROCESS_NAMESPACE::ObjectGeneralInfo* info = new PREPRO_POSTPROCESS_NAMESPACE::ObjectGeneralInfo[objectCount];
    status = postProcess->getObjectsGeneralInfo(info);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get objects' info." << std::endl;
        return;
    }
	std::cout << "Select whether to show the results after importing file? \n";
	std::cout << "#####  0.No " << std::endl;
	std::cout << "#####  1.Yes " << std::endl;
	int showResult = 1;
	std::cin >> showResult;
    for (unsigned int i = 0; i < objectCount; ++i)
    {
        std::cout << "object name:" << info[i].objectName << "      color variable:" << info[i].variableName << std::endl;
		if (showResult)
		{
			getObjectData(postProcess, info[i].objectName, info[i].variableName);
		}
	}
	delete[]info;
	info = nullptr;
}


void PostProcessDemo::pointDemo(const std::string& examplePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_BASE_NAMESPACE::PFStatus status = createPoint(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create point." << std::endl;
		return;
	}
	status = getPointSetting(postProcess, "pointDemo1");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of point." << std::endl;
		return;
	}
	status = editPoint(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit point." << std::endl;
		return;
	}
	status = getPointSetting(postProcess, "pointDemo1");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of point." << std::endl;
		return;
	}

}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_POSTPROCESS_NAMESPACE::PFCoordinatePointSetting coordinateSetting("pointDemo1");
	coordinateSetting.setDomains("volume,Pipetee");
	coordinateSetting.setOption(PREPRO_POSTPROCESS_NAMESPACE::PointCoordinateOption::EPoint);

	PREPRO_BASE_NAMESPACE::PFData pfData;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createObject(&coordinateSetting, pfData);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create object:" << coordinateSetting.getName() << std::endl;
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

	//create extrema point
	PREPRO_POSTPROCESS_NAMESPACE::PFExtremaPointSetting extremaSetting("pointDemo2");
	extremaSetting.setDomains("volume,Pipetee");
	extremaSetting.setLocations("volume,Pipetee");
	extremaSetting.setOption(PREPRO_POSTPROCESS_NAMESPACE::ExtremaPointOption::EMinimum);
	extremaSetting.setVariable("Temperature");
	PREPRO_BASE_NAMESPACE::PFData pfExtrmePointData;
	status = postProcess->createObject(&extremaSetting, pfExtrmePointData);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create object:" << pfExtrmePointData.getName() << std::endl;
		return status;
	}
	PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues extremaPointVariableValues;
	status = postProcess->getObjectVariableValues(pfExtrmePointData.getName(), "Velocity_mag", extremaPointVariableValues);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get object variable values:" << pfExtrmePointData.getName() << std::endl;
		return status;
	}
	RenderDemo::showResult(pfExtrmePointData, extremaPointVariableValues);
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}


PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_POSTPROCESS_NAMESPACE::PFCoordinatePointSetting coordinateSetting("pointDemo1");
	coordinateSetting.setDomains("volume,Pipetee");
	coordinateSetting.setOption(PREPRO_POSTPROCESS_NAMESPACE::PointCoordinateOption::EPointCloud);
	coordinateSetting.setNumberOfPoint(100);
	coordinateSetting.setRadius(0.2);
	double point[3] = { 0.01,0.02,0.03 };
	coordinateSetting.setPoint(point);
	PREPRO_BASE_NAMESPACE::PFData coordinateData;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editObject(&coordinateSetting, coordinateData);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit object:" << coordinateSetting.getName() << std::endl;
		return status;
	}
	PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues variableValues;
	status = postProcess->getObjectVariableValues(coordinateSetting.getName(), "Velocity_mag", variableValues);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get object variable values:" << coordinateSetting.getName() << std::endl;
		return status;
	}
	RenderDemo::showResult(coordinateData, variableValues);

	//change point type
	PREPRO_POSTPROCESS_NAMESPACE::PFExtremaPointSetting extremaSetting("pointDemo1");
	extremaSetting.setDomains("volume,Pipetee");
	extremaSetting.setLocations("volume,Pipetee");
	extremaSetting.setOption(PREPRO_POSTPROCESS_NAMESPACE::ExtremaPointOption::EMaximum);
	extremaSetting.setVariable("Pressure");
	PREPRO_BASE_NAMESPACE::PFData pfExtrmePointData;
	status = postProcess->editObject(&extremaSetting, pfExtrmePointData);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit object:" << coordinateSetting.getName() << std::endl;
		return status;
	}
	PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues extremaPointVariableValues;
	status = postProcess->getObjectVariableValues(pfExtrmePointData.getName(), "Velocity_mag", extremaPointVariableValues);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get object variable values:" << coordinateSetting.getName() << std::endl;
		return status;
	}
	RenderDemo::showResult(pfExtrmePointData, extremaPointVariableValues);

	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}


PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getPointSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	if (nullptr == postSetting)
	{
		std::cout << "Failed to get object info:" << name << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	switch (postSetting->getType())
	{
	case PREPRO_POSTPROCESS_NAMESPACE::PFSettingType::ECoordinatePoint:
	{
		const PREPRO_POSTPROCESS_NAMESPACE::PFCoordinatePointSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFCoordinatePointSetting*>(postSetting);
		if (nullptr == currentSetting)
		{
			return PREPRO_BASE_NAMESPACE::PFStatus::EError;
		}
		std::cout << "coordinate point name:" << currentSetting->getName() << std::endl;
		std::cout << "coordinate point domains:" << currentSetting->getDomains() << std::endl;
		double point[3] = { 0.01,0.02,0.03 };
		currentSetting->getPoint(point);
		std::cout << "coordinate point point: X:" << point[0] << "  Y:" << point[1] << "  Z:" << point[2] << std::endl;
		std::cout << "coordinate point option:" << (int)currentSetting->getOption() << std::endl;
		if (currentSetting->getOption() == PREPRO_POSTPROCESS_NAMESPACE::PointCoordinateOption::EPointCloud)
		{
			std::cout << "coordinate point cloud number:" << currentSetting->getNumberOfPoint() << std::endl;
			std::cout << "coordinate point cloud radius:" << currentSetting->getRadius() << std::endl;
		}
		break;
	}
	case PREPRO_POSTPROCESS_NAMESPACE::PFSettingType::EExtremaPoint:
	{
		const PREPRO_POSTPROCESS_NAMESPACE::PFExtremaPointSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFExtremaPointSetting*>(postSetting);
		if (nullptr == currentSetting)
		{
			std::cout << "get object info error." << std::endl;
			return PREPRO_BASE_NAMESPACE::PFStatus::EError;
		}
		std::cout << "extrema point name:" << currentSetting->getName() << std::endl;
		std::cout << "extrema point domains:" << currentSetting->getDomains() << std::endl;
		std::cout << "extrema point locations:" << currentSetting->getLocations() << std::endl;
		std::cout << "extrema point option:" << (int)currentSetting->getOption() << std::endl;
		break;
	}
	default:
		break;
	}
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}


PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getObjectData(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name,
	const std::string& variable)
{
	PREPRO_BASE_NAMESPACE::PFData pfData;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->getObjectData(name.c_str(), pfData);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get object data:" << name << std::endl;
		return status;
	}
	if (variable.empty())
	{
		RenderDemo::show(pfData, VisualizationType::EPostProcess);
	}
	else
	{
		PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues variableValues;
		status = postProcess->getObjectVariableValues(name.c_str(), variable.c_str(), variableValues);
		if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
		{
			std::cout << "Failed to get object variable values:." << name << std::endl;
			return status;
		}
		RenderDemo::showResult(pfData, variableValues);
	}
	return status;
}


PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameObject(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	std::string oldName = "pointDemo1";
	std::string newName = "pointDemo1Rename";

	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->renameObject(oldName.c_str(), newName.c_str());
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to rename object:" << newName.c_str() << std::endl;
		return status;
	}
	return getPointSetting(postProcess, newName);
}


void PostProcessDemo::sliceDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_BASE_NAMESPACE::PFStatus status = createSliceBySetting(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create slice." << std::endl;
		return;
	}
	status = editSlice(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit slice." << std::endl;
		return;
	}
	status = getSliceSetting(postProcess, "sliceDemo1");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of point." << std::endl;
		return;
	}
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createSliceBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	if (nullptr == postProcess)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	PREPRO_POSTPROCESS_NAMESPACE::PFSliceSetting setting("sliceDemo1");
	setting.setDomains("volume,Pipetee");
	double origin[3] = { 0.,0.1,0. };
	setting.setOriginPoint(origin);
	double normal[3] = { 0.,1.,0. };
	setting.setNormal(normal);
	setting.setOffset(0.02);
	setting.setNumberOfSlice(5);
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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editSlice(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_POSTPROCESS_NAMESPACE::PFSliceSetting setting("sliceDemo1");
	setting.setDomains("volume,Pipetee");
	double origin[3] = { 0.05,0.1,0.2 };
	setting.setOriginPoint(origin);
	double normal[3] = { 1.,0.,1. };
	setting.setNormal(normal);
	setting.setNumberOfSlice(1);
	setting.setOffset(0.);
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
PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getSliceSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	if (nullptr == postSetting)
	{
		std::cout << "Failed to get object info:" << name << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	const PREPRO_POSTPROCESS_NAMESPACE::PFSliceSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFSliceSetting*>(postSetting);
	if (nullptr == currentSetting)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	std::cout << "slice name:" << currentSetting->getName() << std::endl;
	std::cout << "slice domains:" << currentSetting->getDomains() << std::endl;
	double point[3] = { 0. };
	currentSetting->getOriginPoint(point);
	std::cout << "slice origin: X:" << point[0] << "  Y:" << point[1] << "  Z:" << point[2] << std::endl;
	double normal[3] = { 0. };
	currentSetting->getNormal(normal);
	std::cout << "slice normal: X:" << normal[0] << "  Y:" << normal[1] << "  Z:" << normal[2] << std::endl;
	if (currentSetting->getNumberOfSlice() > 1)
	{
		std::cout << "slice number:" << currentSetting->getNumberOfSlice() << std::endl;
		std::cout << "slice offset:" << currentSetting->getOffset() << std::endl;
	}
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

void PostProcessDemo::calculatorDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_BASE_NAMESPACE::PFStatus status = createCalculator(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create calculator." << std::endl;
		return;
	}
	status = editCalculator(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit calculator." << std::endl;
		return;
	}
	status = renameCalculator(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to rename calculator." << std::endl;
		return;
	}
	status = deleteCalculator(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to delete calculator." << std::endl;
		return;
	}
	status = getCalculatorSetting(postProcess, "calculateDemo1Rename");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of calculator." << std::endl;
		return;
	}
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createCalculator(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	if (nullptr == postProcess)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	PREPRO_POSTPROCESS_NAMESPACE::PFCalculatorSetting scalarSetting("calculateDemo1");
	scalarSetting.setExpression("<Pressure>/<Temperature>");

	PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariables allVariables;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createCustomVariable(&scalarSetting, allVariables);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create custom variable." << scalarSetting.getName() << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	PREPRO_POSTPROCESS_NAMESPACE::PFCalculatorSetting vectorSetting("calculateDemo2");
	vectorSetting.setExpression("<Velocity>*<Density>");

	status = postProcess->createCustomVariable(&vectorSetting, allVariables);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create custom variable." << vectorSetting.getName() << std::endl;
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
	status = postProcess->getObjectVariableValues("sliceDemo1", "calculateDemo1", variableValues);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get object variable values." << std::endl;
		return status;
	}
	RenderDemo::showResult(pfData, variableValues);
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editCalculator(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	if (nullptr == postProcess)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	PREPRO_POSTPROCESS_NAMESPACE::PFCalculatorSetting setting("calculateDemo1");
	setting.setExpression("<Velocity>*100");

	PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariables allVariables;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editCustomVariable(&setting, allVariables);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create custom variable." << setting.getName() << std::endl;
		return status;
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
	status = postProcess->getObjectVariableValues("sliceDemo1", "calculateDemo1_mag", variableValues);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get object variable values." << std::endl;
		return status;
	}
	RenderDemo::showResult(pfData, variableValues);
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}
PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::renameCalculator(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	std::string oldName = "calculateDemo1";
	std::string newName = "calculateDemo1Rename";
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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::deleteCalculator(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariables allVariables;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->deleteCustomVariable("calculateDemo2", allVariables);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to delete custom variable." << std::endl;
		return status;
	}
	printVariableInfo(allVariables);
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getCalculatorSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	if (nullptr == postSetting)
	{
		std::cout << "Failed to get object info:" << name << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	const PREPRO_POSTPROCESS_NAMESPACE::PFCalculatorSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFCalculatorSetting*>(postSetting);
	if (nullptr == currentSetting)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	std::cout << "calculator name:" << currentSetting->getName() << std::endl;
	std::cout << "calculator expression:" << currentSetting->getExpression() << std::endl;

	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

void PostProcessDemo::printVariableInfo(const PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariables& variables)
{
	unsigned int variableSize = variables.getVariableSize();
	PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariable** variable = variables.getVariables();
	std::cout << "\n****************************** all variables ****************************\n";
	for (unsigned int i = 0; i < variableSize; ++i)
	{
		std::cout << "variable name: " << variable[i]->getName() << "   dimension: " << variable[i]->getDimension() << std::endl;
	}
	std::cout << "\n";
}
void PostProcessDemo::streamlineDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_BASE_NAMESPACE::PFStatus status = createStreamlineBySetting(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create streamline." << std::endl;
		return;
	}
	status = editStreamline(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit streamline." << std::endl;
		return;
	}
	status = getStreamlineSetting(postProcess, "streamlineDemo1");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of streamline." << std::endl;
		return;
	}
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createStreamlineBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	if (nullptr == postProcess)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	PREPRO_POSTPROCESS_NAMESPACE::PFStreamlineSetting setting("streamlineDemo1");
	setting.setDomains("volume,Pipetee");
	setting.setLocations("boundary,Pipetee/y-inlet");
	setting.setVector("Velocity");
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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editStreamline(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	if (nullptr == postProcess)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	PREPRO_POSTPROCESS_NAMESPACE::PFStreamlineSetting setting("streamlineDemo1");
	setting.setDomains("volume,Pipetee");
	setting.setLocations("slice,sliceDemo1");
	setting.setVector("calculateDemo1Rename");
	setting.setMaximumNumberOfSteps(5000);
	setting.setStepSize(0.0015);
	setting.setNumberOfStreamline(100);
	PREPRO_BASE_NAMESPACE::PFData pfData;
	PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editObject(&setting, pfData);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit object:" << setting.getName() << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues variableValues;
	status = postProcess->getObjectVariableValues(pfData.getName(), "calculateDemo1Rename_mag", variableValues);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get object variable values." << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	RenderDemo::showResult(pfData, variableValues);
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getStreamlineSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	if (nullptr == postSetting)
	{
		std::cout << "Failed to get object info:" << name << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	const PREPRO_POSTPROCESS_NAMESPACE::PFStreamlineSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFStreamlineSetting*>(postSetting);
	if (nullptr == currentSetting)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	std::cout << "streamline name:" << currentSetting->getName() << std::endl;
	std::cout << "streamline domains:" << currentSetting->getDomains() << std::endl;
	std::cout << "streamline locations:" << currentSetting->getLocations() << std::endl;
	std::cout << "streamline vector:" << currentSetting->getVector() << std::endl;
	std::cout << "streamline direction:" << (int)currentSetting->getDirection() << std::endl;
	std::cout << "streamline maximum number of steps:" << currentSetting->getmaximumNumberOfSteps() << std::endl;
	std::cout << "streamline step size:" << currentSetting->getStepSize() << std::endl;
	std::cout << "streamline number:" << currentSetting->getNumberOfStreamline() << std::endl;
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

void PostProcessDemo::vectorDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_BASE_NAMESPACE::PFStatus status = createVectorBySetting(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create vector." << std::endl;
		return;
	}
	status = editVector(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit vector." << std::endl;
		return;
	}
	status = getVectorSetting(postProcess, "vectorDemo1");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of vector." << std::endl;
		return;
	}
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createVectorBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	if (nullptr == postProcess)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	PREPRO_POSTPROCESS_NAMESPACE::PFVectorSetting setting("vectorDemo1");
	setting.setDomains("volume,Pipetee");
	setting.setLocations("boundary,Pipetee/y-inlet,boundary,Pipetee/z-outlet,slice,sliceDemo1");
	setting.setVector("Velocity");
	setting.setSampleNumber(100);
	setting.setScaleFactor(0.1);
	setting.setWidthSize(0.1);
	setting.setArrowHeadSize(0.35);
	PFPostProcess::VectorScaleVariable vectorScaleVariable = PFPostProcess::VectorScaleVariable::Magnitude;
	setting.setVectorScaleVariable(vectorScaleVariable);
	PFPostProcess::VectorProjectComponent vectorProjectComponent = PFPostProcess::VectorProjectComponent::Normal;
	setting.setProjectComponent(vectorProjectComponent);
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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editVector(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_POSTPROCESS_NAMESPACE::PFVectorSetting setting("vectorDemo1");
	setting.setDomains("volume,Pipetee");
	setting.setLocations("boundary,Pipetee/y-inlet,boundary,Pipetee/z-outlet");
	setting.setVector("Velocity");
	setting.setSampleNumber(200);
	setting.setScaleFactor(0.5);
	setting.setWidthSize(0.5);
	setting.setArrowHeadSize(0.5);
	PFPostProcess::VectorScaleVariable vectorScaleVariable = PFPostProcess::VectorScaleVariable::Constant;
	setting.setVectorScaleVariable(vectorScaleVariable);
	PFPostProcess::VectorProjectComponent vectorProjectComponent = PFPostProcess::VectorProjectComponent::Normal;
	setting.setProjectComponent(vectorProjectComponent);

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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getVectorSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	if (nullptr == postSetting)
	{
		std::cout << "Failed to get object info:" << name << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	const PREPRO_POSTPROCESS_NAMESPACE::PFVectorSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFVectorSetting*>(postSetting);
	if (nullptr == currentSetting)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	std::cout << "vector name:" << currentSetting->getName() << std::endl;
	std::cout << "vector domains:" << currentSetting->getDomains() << std::endl;
	std::cout << "vector setLocations:" << currentSetting->getLocations() << std::endl;

	std::cout << "vector vector:" << currentSetting->getVector() << std::endl;
	std::cout << "vector sample number:" << currentSetting->getSampleNumber() << std::endl;
	std::cout << "vector scale factor:" << currentSetting->getScaleFactor() << std::endl;

	std::cout << "vector width size:" << currentSetting->getWidthSize() << std::endl;
	std::cout << "vector arrow head size:" << currentSetting->getArrowHeadSize() << std::endl;
	std::cout << "vector scale factor:" << currentSetting->getScaleFactor() << std::endl;
	PFPostProcess::VectorScaleVariable vectorScaleVariable = currentSetting->getVectorScaleVariable();
	const char* variable = vectorScaleVariable == PFPostProcess::VectorScaleVariable::Magnitude ? "Magnitude" : "Constant";
	std::cout << "vector scale variable:" << variable << std::endl;

	PFPostProcess::VectorProjectComponent vectorProjectComponent = currentSetting->getProjectComponent();
	const char* component = "None";
	switch (vectorProjectComponent)
	{
	case PFPostProcess::VectorProjectComponent::None:
		component = "None";
		break;
	case PFPostProcess::VectorProjectComponent::Normal:
		component = "Normal";
		break;
	case PFPostProcess::VectorProjectComponent::Tangent:
		component = "Tangent";
		break;
	default:
		break;
	}
	std::cout << "vector project component:" << component << std::endl;
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

void PostProcessDemo::clipWithPlaneDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_BASE_NAMESPACE::PFStatus status = createClipWithPlaneBySetting(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create clip with plane." << std::endl;
		return;
	}
	status = editClipWithPlane(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit clip with plane." << std::endl;
		return;
	}
	status = getClipWithPlaneSetting(postProcess, "clipWithPlaneDemo1");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of clip with plane." << std::endl;
		return;
	}
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createClipWithPlaneBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	if (nullptr == postProcess)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	PREPRO_POSTPROCESS_NAMESPACE::PFClipWithPlaneSetting setting("clipWithPlaneDemo1");
	setting.setDomains("volume,Pipetee");
	double point[3] = { 0,0.1,0 };
	setting.setOriginPoint(point);
	double normal[3] = { 0,1,0 };
	setting.setNormal(normal);
	setting.setInvertOption(PFPostProcess::ClipInvertOption::ENoInvert);
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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editClipWithPlane(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_POSTPROCESS_NAMESPACE::PFClipWithPlaneSetting setting("clipWithPlaneDemo1");
	setting.setDomains("volume,Pipetee");
	double point[3] = { 0.1,0,0 };
	setting.setOriginPoint(point);
	double normal[3] = { 1,1,1 };
	setting.setNormal(normal);
	setting.setInvertOption(PFPostProcess::ClipInvertOption::EInvert);
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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getClipWithPlaneSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	if (nullptr == postSetting)
	{
		std::cout << "Failed to get object info:" << name << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}

	const PREPRO_POSTPROCESS_NAMESPACE::PFClipWithPlaneSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFClipWithPlaneSetting*>(postSetting);
	if (nullptr == currentSetting)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	std::cout << "clip with plane name:" << currentSetting->getName() << std::endl;
	std::cout << "clip with plane domains:" << currentSetting->getDomains() << std::endl;
	double point[3] = { 0. };
	currentSetting->getOriginPoint(point);
	std::cout << "clip with plane origin: X:" << point[0] << "  Y:" << point[1] << "  Z:" << point[2] << std::endl;
	double normal[3] = { 0. };
	currentSetting->getNormal(normal);
	std::cout << "clip with plane normal: X:" << normal[0] << "  Y:" << normal[1] << "  Z:" << normal[2] << std::endl;
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

void PostProcessDemo::clipWithBoxDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_BASE_NAMESPACE::PFStatus status = createClipWithBoxBySetting(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create clip with box." << std::endl;
		return;
	}
	status = editClipWithBox(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit clip with box." << std::endl;
		return;
	}
	status = getClipWithBoxSetting(postProcess, "clipWithBoxDemo1");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of clip with box." << std::endl;
		return;
	}
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createClipWithBoxBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	if (nullptr == postProcess)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	PREPRO_POSTPROCESS_NAMESPACE::PFClipWithBoxSetting setting("clipWithBoxDemo1");
	setting.setDomains("volume,Pipetee");
	double minimumPoint[3] = { 0,0,0 };
	double maximumPoint[3] = { 0.05,0.1,0.2 };
	setting.setBoxPoint(minimumPoint, maximumPoint);
	setting.setInvertOption(PFPostProcess::ClipInvertOption::ENoInvert);
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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editClipWithBox(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_POSTPROCESS_NAMESPACE::PFClipWithBoxSetting setting("clipWithBoxDemo1");
	setting.setDomains("volume,Pipetee");
	double minimumPoint[3] = { 0,0,0 };
	double maximumPoint[3] = { 0.2,0.2,0.2 };
	setting.setBoxPoint(minimumPoint,maximumPoint);
	setting.setInvertOption(PFPostProcess::ClipInvertOption::EInvert);
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

PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getClipWithBoxSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	if (nullptr == postSetting)
	{
		std::cout << "Failed to get object info:" << name << std::endl;
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	const PREPRO_POSTPROCESS_NAMESPACE::PFClipWithBoxSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFClipWithBoxSetting*>(postSetting);
	if (nullptr == currentSetting)
	{
		return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	}
	std::cout << "clip with box name:" << currentSetting->getName() << std::endl;
	std::cout << "clip with box domains:" << currentSetting->getDomains() << std::endl;
	double minimumPoint[3] = { 0. };
	double maximumPoint[3] = { 0. };
	currentSetting->getBoxPoint(minimumPoint, maximumPoint);
	std::cout << "clip with box point1: X:" << minimumPoint[0] << "  Y:" << minimumPoint[1] << "  Z:" << minimumPoint[2] << std::endl;
	std::cout << "clip with box point2: X:" << maximumPoint[0] << "  Y:" << maximumPoint[1] << "  Z:" << maximumPoint[2] << std::endl;
	return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

void PostProcessDemo::lineFromPointDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	PREPRO_BASE_NAMESPACE::PFStatus status = createLineFromPointBySetting(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to create line from point." << std::endl;
		return;
	}
	status = editLineFromPoint(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit line from point." << std::endl;
		return;
	}

	status = editLineFromTwoPointsToIntersection(postProcess);
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to edit line from point." << std::endl;
		return;
	}

	status = getLineFromPointSetting(postProcess, "lineFromPointDemo");
	if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	{
		std::cout << "Failed to get setting of line from point." << std::endl;
		return;
	}
}

 PREPRO_BASE_NAMESPACE::PFStatus   PostProcessDemo::createLineFromPointBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	 if (nullptr == postProcess)
	 {
		 return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	 }
	 PREPRO_POSTPROCESS_NAMESPACE::PFLineFromTwoPointsSetting setting("lineFromPointDemo");
	 setting.setDomains("volume,Pipetee");
	 setting.setNumberOfSamples(10);
	 double startPoint[3] = { 0,0,0 };
	 double endPoint[3] = { 0.1,0.1,0 };
	 setting.setEndPoints(startPoint, endPoint);
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

 PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editLineFromPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	 if (nullptr == postProcess)
	 {
		 return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	 }
	 PREPRO_POSTPROCESS_NAMESPACE::PFLineFromTwoPointsSetting setting("lineFromPointDemo");
	 setting.setDomains("volume,Pipetee");
	 setting.setNumberOfSamples(100);
	 double startPoint[3] = { 0,0,0 };
	 double endPoint[3] = { -0.1,-0.1,0 };
	 setting.setEndPoints(startPoint, endPoint);
	 PREPRO_BASE_NAMESPACE::PFData pfData;
	 PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editObject(&setting, pfData);
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

 PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editLineFromTwoPointsToIntersection(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
 {
	 if (nullptr == postProcess)
	 {
		 return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	 }
	 PREPRO_POSTPROCESS_NAMESPACE::PFLineFromIntersectionSetting setting("lineFromPointDemo");
	 setting.setDomains("volume,Pipetee");
	 setting.setCuttingSurface("slice,sliceDemo1");
	 setting.setIntersectedSurfaces("boundary,Pipetee/walls,boundary,Pipetee/z-outlet");
	 setting.setNumberOfSamples(100);
	 PREPRO_BASE_NAMESPACE::PFData pfData;
	 PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editObject(&setting, pfData);
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

 PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getLineFromPointSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	 const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	 if (nullptr == postSetting)
	 {
		 std::cout << "Failed to get object info:" << name << std::endl;
		 return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	 }
	 const PREPRO_POSTPROCESS_NAMESPACE::PFLineFromTwoPointsSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFLineFromTwoPointsSetting*>(postSetting);
	 if (nullptr == currentSetting)
	 {
		 return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	 }
	 std::cout << "line from point name:" << currentSetting->getName() << std::endl;
	 std::cout << "line from point name domains:" << currentSetting->getDomains() << std::endl;
	 double startPoint[3] = { 0. };
	 double endPoint[3] = { 0. };
	 currentSetting->getEndPoints(startPoint, endPoint);
	 std::cout << "line from point  startPoint: X:" << startPoint[0] << "  Y:" << startPoint[1] << "  Z:" << startPoint[2] << std::endl;
	 std::cout << "line from point endPoint: X:" << endPoint[0] << "  Y:" << endPoint[1] << "  Z:" << endPoint[2] << std::endl;
	 unsigned int samplesNumber = currentSetting->getNumberOfSamples();
	 std::cout << "samples mumber" << samplesNumber << std::endl;
	 return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

 void PostProcessDemo::lineFromIntersectionDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	 PREPRO_BASE_NAMESPACE::PFStatus status = createLineFromIntersectionBySetting(postProcess);
	 if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	 {
		 std::cout << "Failed to create line from intersection ." << std::endl;
		 return;
	 }
	 status = editLineFromIntersection(postProcess);
	 if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	 {
		 std::cout << "Failed to edit line from intersection." << std::endl;
		 return;
	 }
	 status = getLineFromIntersectionSetting(postProcess, "lineFromIntersectionDemo");
	 if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
	 {
		 std::cout << "Failed to get setting of line from intersection." << std::endl;
		 return;
	 }
}

 PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::createLineFromIntersectionBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	 if (nullptr == postProcess)
	 {
		 return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	 }
	 PREPRO_POSTPROCESS_NAMESPACE::PFLineFromIntersectionSetting setting("lineFromIntersectionDemo");
	 setting.setDomains("volume,Pipetee");
	 setting.setCuttingSurface("slice,sliceDemo1");
	 setting.setIntersectedSurfaces("boundary,Pipetee/walls,boundary,Pipetee/z-outlet");
	 setting.setNumberOfSamples(10);
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

 PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::editLineFromIntersection(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
	 if (nullptr == postProcess)
	 {
		 return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	 }
	 PREPRO_POSTPROCESS_NAMESPACE::PFLineFromIntersectionSetting setting("lineFromIntersectionDemo");
	 setting.setDomains("volume,Pipetee");
	 setting.setCuttingSurface("slice,sliceDemo1");
	 setting.setIntersectedSurfaces("boundary,Pipetee/walls,boundary,Pipetee/z-outlet");
	 setting.setNumberOfSamples(100);
	 PREPRO_BASE_NAMESPACE::PFData pfData;
	 PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editObject(&setting, pfData);
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

 PREPRO_BASE_NAMESPACE::PFStatus PostProcessDemo::getLineFromIntersectionSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
	 const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
	 if (nullptr == postSetting)
	 {
		 std::cout << "Failed to get object info:" << name << std::endl;
		 return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	 }
	 const PREPRO_POSTPROCESS_NAMESPACE::PFLineFromIntersectionSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFLineFromIntersectionSetting*>(postSetting);
	 if (nullptr == currentSetting)
	 {
		 return PREPRO_BASE_NAMESPACE::PFStatus::EError;
	 }
	 std::cout << "line from intersection name:" << currentSetting->getName() << std::endl;
	 std::cout << "line from intersection name domains:" << currentSetting->getDomains() << std::endl;
	 std::cout << "line from intersection name cutting surface:" << currentSetting->getCuttingSurface() << std::endl;
	 std::cout << "line from intersection name intersected surfaces:" << currentSetting->getIntersectedSurfaces() << std::endl;
	 unsigned int samplesNumber = currentSetting->getNumberOfSamples();
	 std::cout << "samples mumber" << samplesNumber << std::endl;
	 return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

