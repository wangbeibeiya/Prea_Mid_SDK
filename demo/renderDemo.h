// Revision: $Id: renderDemo.h,v 1.0 2025/02/26 11:01:36 xiaming Exp $
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

#include <base/common/pfConstDefinitions.h>
#include "commonEnumeration.h"
#include <postProcess/common/pfConstDefinitions.h>
#include <mesh/common/pfConstDefinitions.h>
#include <vector>
#include <set>
#include <string>


START_PREPRO_BASE_NAMESPACE
class PFData;
class PFGroup;
class PFVolume;
END_PREPRO_BASE_NAMESPACE
START_PREPRO_POSTPROCESS_NAMESPACE
class PFPostProcessVariableValues;
END_PREPRO_POSTPROCESS_NAMESPACE
START_PREPRO_MESH_NAMESPACE
class PFLeakPathContainer;
END_PREPRO_MESH_NAMESPACE

class RenderDemo
{
public:
    static void show(const PREPRO_BASE_NAMESPACE::PFData& shownData, VisualizationType  type, std::vector< PREPRO_BASE_NAMESPACE::PFGroup*> groupList = std::vector< PREPRO_BASE_NAMESPACE::PFGroup*>());
    static void show(const PREPRO_BASE_NAMESPACE::PFGroup& pfGroup, VisualizationType  type);
    static void showResult(const PREPRO_BASE_NAMESPACE::PFData& shownData, PREPRO_POSTPROCESS_NAMESPACE::PFPostProcessVariableValues& variableValues);
    static void showLeakPath(const PREPRO_MESH_NAMESPACE::PFLeakPathContainer& leakPath);

    static void show(const PREPRO_BASE_NAMESPACE::PFData& shownData, VisualizationType  type, std::set<std::string> groupNames);

    static void show(const PREPRO_BASE_NAMESPACE::PFVolume& pfGroup, VisualizationType type);
};