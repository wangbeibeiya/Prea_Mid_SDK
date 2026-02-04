// Revision: $Id: saveDocumentDemo.h,v 1.0 2025/02/21 14:55:04 Leon Exp $
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

#include <base/common/pfConstDefinitions.h>
#include <base/common/pfEnumeration.h>
#include <mesh/common/pfConstDefinitions.h>
#include <geometry/common/pfGeometryMacro.h>

#include <string>

START_PREPRO_BASE_NAMESPACE
class PFDocument;
class PFApplication;
END_PREPRO_BASE_NAMESPACE

START_PREPRO_GEOMETRY_NAMESPACE
class PFGeometry;
END_PREPRO_GEOMETRY_NAMESPACE

using namespace PREPRO_BASE_NAMESPACE;
using namespace PREPRO_GEOMETRY_NAMESPACE;

class DocumentDemo
{
public:
	static void documentDemo(const std::string& path);
	static PREPRO_BASE_NAMESPACE::PFStatus saveDocument(PFApplication* pfApplication, PFDocument* pfDocument);
	static PREPRO_BASE_NAMESPACE::PFStatus closeDocument(PFApplication* pfApplication, PFDocument* pfDocument, bool needSave = true);
    static PREPRO_BASE_NAMESPACE::PFStatus testSaveAsNewGeometryAlone(const std::string& examplePath);
    static PREPRO_BASE_NAMESPACE::PFStatus testSaveAsNewMeshAlone(const std::string& examplePath);
    static PREPRO_BASE_NAMESPACE::PFStatus testSaveAsNewGeometryAndMesh(const std::string& examplePath);
    static PREPRO_BASE_NAMESPACE::PFStatus testSaveAsNewEmpty(const std::string& examplePath);
};