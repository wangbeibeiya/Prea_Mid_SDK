// Revision: $Id: geometryDemo.h,v 1.0 2025/02/24 13:55:04 zhiqiang.chen Exp $
/*
 * PeraGlobal CONFIDENTIAL
 * __________________
 *
 *  [2007] - [2021] PeraGlobal Technologies, Inc.
 *  All Rights Reserved.
 *
 * NOTICE:  All information contained herein is, and remains
 * the property of PeraGlobal Technoglies, Inc.
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
#include <utility>
#include <geometry/common/pfGeometryMacro.h>
#include <base/common/pfConstDefinitions.h>

START_PREPRO_BASE_NAMESPACE
class PFApplication;
END_PREPRO_BASE_NAMESPACE

START_PREPRO_GEOMETRY_NAMESPACE
class PFGeometry;
END_PREPRO_GEOMETRY_NAMESPACE
typedef void (*func)(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
class GeometryDemo
{
public:
	static void geometryDemo(const std::string& path);

private:
	static PREPRO_GEOMETRY_NAMESPACE::PFGeometry* getPFGeometryPointer(PREPRO_BASE_NAMESPACE::PFApplication& pfApplication, const std::string& exePath, const std::string& fileName);
	static std::pair<std::string, func> getFunctionAndFileName();
	static void quickRepair(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void findVolumes(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void stitch(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void defeature(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void stitchHole(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void intersect(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void fillHole(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void fillAllHoles(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void fillFacesHoles(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void addEntitiesToGroup(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void createEdge(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void splitEdge(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void mergeEdges(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void createFaceByEdges(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void splitFaceByEdge(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void createFaceByPoints(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void project(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void mergeFaces(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void extrude(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void createCube(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void createCylinder(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void createHemisphere(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void createSphere(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void deleteEntities(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void imprint(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void scale(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void mirror(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void rotate(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void translate(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void createVolume(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void detachVolumes(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void deleteFaceGroups(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void rename(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static void deleteVolumes(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
    static void convertMeshToGeometry(PREPRO_GEOMETRY_NAMESPACE::PFGeometry* geometry);
	static PREPRO_GEOMETRY_NAMESPACE::PFGeometry* importMeshBeforeConverttingMeshToGeometry(PREPRO_BASE_NAMESPACE::PFApplication& pfApplication, const std::string& examplePath);
};