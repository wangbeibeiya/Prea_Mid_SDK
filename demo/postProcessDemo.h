// Revision: $Id: postProcessDemo.h,v 1.0 2025/03/04 13:15:20 liujing Exp $
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
#pragma once

#include <string>
#include <base/common/pfConstDefinitions.h>
#include <base/common/pfEnumeration.h>
#include <postProcess/common/pfConstDefinitions.h>
#include <postProcess/pfDPMData.h>

START_PREPRO_BASE_NAMESPACE
class PFDocument;
class PFData;
END_PREPRO_BASE_NAMESPACE

START_PREPRO_POSTPROCESS_NAMESPACE
class PFPostProcess;
class PFPostProcessVariables;

END_PREPRO_POSTPROCESS_NAMESPACE


class PostProcessDemo
{
public:

    static void postProcessDemo(const std::string& examplePath);

    static void loadResult(const std::string& filePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

    static void exportFpsFile(const std::string& filePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

    static void importFpsFile(const std::string& filePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

    static void getAllVariables(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
  
private:
    static void getAllBoundriesData(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static void deleteObejct(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);



	static void pointDemo(const std::string& examplePath, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus createPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus editPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus getPointSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);

	static PREPRO_BASE_NAMESPACE::PFStatus getObjectData(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name,
		const std::string& variable);

	static PREPRO_BASE_NAMESPACE::PFStatus renameObject(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static void sliceDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus createSliceBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus editSlice(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus getSliceSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);

	static void  calculatorDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus createCalculator(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus editCalculator(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus renameCalculator(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus deleteCalculator(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus getCalculatorSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);

	static void printVariableInfo(const PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariables& variables);

	static void streamlineDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus createStreamlineBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus editStreamline(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus getStreamlineSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
   
	static void vectorDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
	
    static PREPRO_BASE_NAMESPACE::PFStatus   createVectorBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
	
	static PREPRO_BASE_NAMESPACE::PFStatus editVector(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
	
	static PREPRO_BASE_NAMESPACE::PFStatus getVectorSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);

	static void contourLineDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus createContourLineBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus editContourLine(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus getContourLineSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);

	static void contourSurfaceDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus createContourSurfaceBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus editContourSurface(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus getContourSurfaceSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);

	static void clipWithPlaneDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus   createClipWithPlaneBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus editClipWithPlane(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus getClipWithPlaneSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);

	static void clipWithBoxDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus   createClipWithBoxBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus editClipWithBox(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus getClipWithBoxSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);

	// iso clip demo.
	static void isoClipDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus createISOClipBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus editISOClip(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus getISOClipSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    
	static void lineFromPointDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
	
	static PREPRO_BASE_NAMESPACE::PFStatus   createLineFromPointBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus editLineFromPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
	
	static PREPRO_BASE_NAMESPACE::PFStatus editLineFromTwoPointsToIntersection(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus getLineFromPointSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);

	static void lineFromIntersectionDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus  createLineFromIntersectionBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus editLineFromIntersection(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	static PREPRO_BASE_NAMESPACE::PFStatus getLineFromIntersectionSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);

    static void integralDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createAreaWeightedAverageIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus editAreaWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getAreaWeightedAverageIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameAreaWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteAreaWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createVolumeWeightedAverageIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus editVolumeWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getVolumeWeightedAverageIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameVolumeWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteVolumeWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createMassWeightedAverageIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus editMassWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getMassWeightedAverageIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameMassWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteMassWeightedAverageIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createMassFlowRateIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus editMassFlowRateIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getMassFlowRateIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameMassFlowRateIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteMassFlowRateIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createForceIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus editForceIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getForceIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameForceIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteForceIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createCentroidIntegralBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus editCentroidIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getCentroidIntegralSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameCentroidIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteCentroidIntegral(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

	// vorticity demo.
	static void vorticityDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
	static PREPRO_BASE_NAMESPACE::PFStatus createVorticity(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
	static PREPRO_BASE_NAMESPACE::PFStatus renameVorticity(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
	static PREPRO_BASE_NAMESPACE::PFStatus deleteVorticity(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
	static PREPRO_BASE_NAMESPACE::PFStatus getVorticitySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);

    static void accumulatedCurveDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createAccumulatedCurveBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus editAccumulatedCurve(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getAccumulatedCurveSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameAccumulatedCurve(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteAccumulatedCurve(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

    // fan operting point.
    static PREPRO_BASE_NAMESPACE::PFStatus fanOperatingPointDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& examplePath);
    static PREPRO_BASE_NAMESPACE::PFStatus createFanOperatingPointBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& examplePath);
    static PREPRO_BASE_NAMESPACE::PFStatus editFanOperatingPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& examplePath);
    static PREPRO_BASE_NAMESPACE::PFStatus getFanOperatingPointResult(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameFanOperatingPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteFanOperatingPoint(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

    // DPM Trajectory
    static PREPRO_BASE_NAMESPACE::PFStatus dpmTrajectoryDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& examplePath);
    static PREPRO_BASE_NAMESPACE::PFStatus createDPMTrajectoryBySetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& examplePath);
    static PREPRO_BASE_NAMESPACE::PFStatus editDPMTrajectory(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getDPMTrajectoryResult(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameDPMTrajectory(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteDPMTrajectory(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static void loadDPMTrajectory(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const char* dpmFilePath, PREPRO_POSTPROCESS_NAMESPACE::PFDPMData& dpmData);
};