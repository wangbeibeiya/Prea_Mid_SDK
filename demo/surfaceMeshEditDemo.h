// Revision: $Id: surfaceMeshEditDemo.h,v 1.0 2025/04/02 13:15:20 cuianzhu Exp $
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
#include <mesh/common/pfConstDefinitions.h>

START_PREPRO_BASE_NAMESPACE
class PFDocument;
END_PREPRO_BASE_NAMESPACE

START_PREPRO_MESH_NAMESPACE
class PFMesh;
END_PREPRO_MESH_NAMESPACE


class SurfaceMeshEditDemo
{
public:
    static void surfaceMeshEditDemo(const std::string& path);
private:
    static void createDemo(const std::string& path);
    static void createSurfaceMeshBy3Points(const std::string& path);
    static void createSurfaceMeshBy3Edges(const std::string& path);
    static void projectNodesDemo(const std::string& path);
    static void projectNodesToPlaneDemo(const std::string& path);
    static void projectNodesToEdgeDemo(const std::string& path);
    static void projectNodesToPointDemo(const std::string& path);
    static void projectNodesToLineDemo(const std::string& path);
    static void projectNodesMoreToPlaneDemo(const std::string& path);
    static void moveToGroupDemo(const std::string& path);
    static void intersectDemo(const std::string& path);
    static void fillHolesDemo(const std::string& path);
    static void splitDemo(const std::string& path);
    static void splitEdgesDemo(const std::string& path);
    static void splitCellsDemo(const std::string& path);
    static void smoothCellsDemo(const std::string& path);
    static void swapEdgeDemo(const std::string& path);
    static void remeshCellsDemo(const std::string& path);
    static void collapseEdgesDemo(const std::string& path);
    static void deleteCellsDemo(const std::string& path);

private:
};
