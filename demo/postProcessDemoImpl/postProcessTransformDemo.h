// Revision: $Id: postProcessFieldVariableDemo.h,v 1.0 2025/05/07 12:55:04 liujing Exp $
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

START_PREPRO_POSTPROCESS_NAMESPACE
class PFPostProcess;

END_PREPRO_POSTPROCESS_NAMESPACE

class PostProcessTransformDemo
{
public:


    static void transformDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static void mirrorDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createMirror(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus editMirror(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getMirrorSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameMirror(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& oldName, const std::string& newName);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteMirror(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

    static void rotateDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createRotate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus editRotate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getRotateSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameRotate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& oldName, const std::string& newName);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteRotate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);

    static void translateDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus createTranslate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus editTranslate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
    static PREPRO_BASE_NAMESPACE::PFStatus getTranslateSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name);
    static PREPRO_BASE_NAMESPACE::PFStatus renameTranslate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& oldName, const std::string& newName);
    static PREPRO_BASE_NAMESPACE::PFStatus deleteTranslate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess);
};