// Revision: $Id: postProcessTransformDemo.cpp,v 1.0 2025/04/25 13:15:20 liujing Exp $
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

#include "postProcessTransformDemo.h"
#include <iostream>
#include <base/pfDocument.h>
#include <base/common/pfEnumeration.h>
#include <base/pfGroupData.h>
#include <postprocess/pfPostProcess.h>
#include <postProcess/pfPostProcessVariable.h>
#include "../commonMethods.h"
#include "../renderDemo.h"
#include <postProcess/pfPostProcessVariableValues.h>
#include <postProcess/common/pfEnumeration.h>
#include <postProcess/pfMirrorSetting.h>
#include <postProcess/pfRotateSetting.h>
#include <postProcess/pfTranslateSetting.h>
#include <postProcess/pfVolumeInfo.h>
void PostProcessTransformDemo::transformDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{   
    /*!
    mirror demo :create,edit,get setting,rename, delete
    */
    PostProcessTransformDemo::mirrorDemo(postProcess);
    /*!
    rotate demo :create,edit,get setting,rename, delete
    */
    PostProcessTransformDemo::rotateDemo(postProcess);
    /*!
    translate demo :create,edit,get setting,rename, delete
    */
    PostProcessTransformDemo::translateDemo(postProcess);

}
void PostProcessTransformDemo::mirrorDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_BASE_NAMESPACE::PFStatus status = createMirror(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create mirror." << std::endl;
        return;
    }

    status = editMirror(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit mirror." << std::endl;
        return;
    }
    status = getMirrorSetting(postProcess, "mirrorDemo1");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get setting of mirror." << std::endl;
        return;
    }
    std::string newName = "mirrorDemo1Rename";
    status = renameMirror(postProcess, "mirrorDemo1", newName);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename mirror." << std::endl;
        return;
    }

    status = deleteMirror(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete mirror." << std::endl;
        return;
    }
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::createMirror(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFMirrorSetting setting("mirrorDemo1");
    setting.setSourceVolumeName("Pipetee");
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo targetVolume;
    targetVolume.setTargetVolumeName("mirror1");
    PREPRO_POSTPROCESS_NAMESPACE::NamePair pairs[4] = {
    {"z-inlet","mirrorBoundary1" },
    {"walls","mirrorBoundary2"},
    {"z-outlet","mirrorBoundary3" },
    {"y-inlet","mirrorBoundary4"},
    };
    targetVolume.setBoundaryNameMappings(pairs, 4);
    setting.setTargetVolume(targetVolume);
    
    double origin[3] = { 0.,0.1,0. };
    setting.setOriginPoint(origin);
    double normal[3] = { 1.,1.,0. };
    setting.setNormal(normal);
    PREPRO_BASE_NAMESPACE::PFData data;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createTransform(&setting, data);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create object." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    RenderDemo::show(data, VisualizationType::EPostProcess);
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::editMirror(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFMirrorSetting setting("mirrorDemo1");
    setting.setSourceVolumeName("Pipetee");
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo targetVolume;
    targetVolume.setTargetVolumeName("mirror1Edit");
    PREPRO_POSTPROCESS_NAMESPACE::NamePair pairs[4] = 
    {
    {"z-inlet","mirrorBoundary1" },
    {"walls","mirrorBoundary2"},
    {"z-outlet","mirrorBoundary3" },
    {"y-inlet","mirrorBoundary4"},
    };
    targetVolume.setBoundaryNameMappings(pairs, 4);
    setting.setTargetVolume(targetVolume);

    double origin[3] = { 0.1,0.1,0. };
    setting.setOriginPoint(origin);
    double normal[3] = { 1.,1.,1. };
    setting.setNormal(normal);
    PREPRO_BASE_NAMESPACE::PFData data;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editTransform(&setting, data);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit transform." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    RenderDemo::show(data, VisualizationType::EPostProcess);
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}
PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::getMirrorSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get setting of mirror:" << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFMirrorSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFMirrorSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "mirror name:" << currentSetting->getName() << std::endl;
    std::cout << "mirror source volume:" << currentSetting->getSourceVolumeName() << std::endl;
    const PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo& targetVolume = currentSetting->getTargetVolume();
    std::cout << "mirror target volume:" << targetVolume.getTargetVolumeName() << std::endl;
    double point[3] = { 0. };
    currentSetting->getOriginPoint(point);
    std::cout << "mirror origin: X:" << point[0] << "  Y:" << point[1] << "  Z:" << point[2] << std::endl;
    double normal[3] = { 0. };
    currentSetting->getNormal(normal);
    std::cout << "mirror normal: X:" << normal[0] << "  Y:" << normal[1] << "  Z:" << normal[2] << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}
PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::renameMirror(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& oldName,
    const std::string& newName)
{
    //rename
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->renameTransform(oldName.c_str(), newName.c_str());
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename custom variable." << std::endl;
        return status;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::deleteMirror(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFMirrorSetting setting("mirrorDemo2");
    setting.setSourceVolumeName("mirror1Edit");
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo targetVolume;
    targetVolume.setTargetVolumeName("mirror2");
    PREPRO_POSTPROCESS_NAMESPACE::NamePair pairs[4] =
    {
    {"mirrorBoundary1","Boundary1" },
    {"mirrorBoundary2","Boundary2"},
    {"mirrorBoundary3","Boundary3" },
    {"mirrorBoundary4","Boundary4"},
    };
    targetVolume.setBoundaryNameMappings(pairs, 4);
    setting.setTargetVolume(targetVolume);
    double origin[3] = { 0.1,0.1,0. };
    setting.setOriginPoint(origin);
    double normal[3] = { 1.,1.,1. };
    setting.setNormal(normal);
    PREPRO_BASE_NAMESPACE::PFData data;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createTransform(&setting, data);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create object." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    RenderDemo::show(data, VisualizationType::EPostProcess);

    status = postProcess->deleteTransform("mirrorDemo2");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete transform." << std::endl;
        return status;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}


void PostProcessTransformDemo::rotateDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_BASE_NAMESPACE::PFStatus status = createRotate(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create rotate." << std::endl;
        return;
    }

    status = editRotate(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit rotate." << std::endl;
        return;
    }
    status = getRotateSetting(postProcess, "rotateDemo1");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get setting of rotate." << std::endl;
        return;
    }
    std::string newName = "rotateDemo1Rename";
    status = renameRotate(postProcess, "rotateDemo1", newName);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename rotate." << std::endl;
        return;
    }

    status = deleteRotate(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete rotate." << std::endl;
        return;
    }
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::createRotate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    PREPRO_POSTPROCESS_NAMESPACE::PFRotateSetting setting("rotateDemo1");
    setting.setSourceVolumeName("Pipetee");
    
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo volume1;
    volume1.setTargetVolumeName("rotate1");
    PREPRO_POSTPROCESS_NAMESPACE::NamePair boundaries[4] =
    {
    {"z-inlet","rotate1z-inlet" },
    {"walls","rotate1walls"},
    {"z-outlet","rotate1z-outlet" },
    {"y-inlet","rotate1y-inlet"},
    };
    volume1.setBoundaryNameMappings(boundaries, 4);
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo volume2;
    volume2.setTargetVolumeName("rotate2");
    PREPRO_POSTPROCESS_NAMESPACE::NamePair boundaries2[4] =
    {
    {"z-inlet","rotate2z-inlet" },
    {"walls","rotate2walls"},
    {"z-outlet","rotate2z-outlet" },
    {"y-inlet","rotate2y-inlet"},
    };
    volume2.setBoundaryNameMappings(boundaries2, 4);
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo volumes[2] = { volume1 ,volume2 };
    setting.setTargetVolumes(volumes, 2);
    double center[3] = { 0.2,0. ,0. };
    setting.setCenterPoint(center);
    double axis[3] = { 0.,0. ,1. };
    setting.setRotateAxis(axis);
    setting.setStartAngle(90);
    setting.setAngleIncrement(90);
    PREPRO_BASE_NAMESPACE::PFData data;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createTransform(&setting, data);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create rotate." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    RenderDemo::show(data, VisualizationType::EPostProcess);
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::editRotate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting("rotateDemo1");
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get setting of rotate." << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFRotateSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFRotateSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    PREPRO_POSTPROCESS_NAMESPACE::PFRotateSetting setting(*currentSetting);
    double axis[3] = { 0.,0. ,1. };
    setting.setRotateAxis(axis);
    setting.setStartAngle(60);
    setting.setAngleIncrement(180);
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo volume1;
    volume1.setTargetVolumeName("rotate1Eidt");
    PREPRO_POSTPROCESS_NAMESPACE::NamePair boundaries[4] =
    {
    {"z-inlet","rotate1z-inlet" },
    {"walls","rotate1walls"},
    {"z-outlet","rotate1z-outlet" },
    {"y-inlet","rotate1y-inlet"},
    };
    volume1.setBoundaryNameMappings(boundaries, 4);
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo volume2;
    volume2.setTargetVolumeName("rotate2Edit");
    PREPRO_POSTPROCESS_NAMESPACE::NamePair boundaries2[4] =
    {
    {"z-inlet","rotate2z-inlet" },
    {"walls","rotate2walls"},
    {"z-outlet","rotate2z-outlet" },
    {"y-inlet","rotate2y-inlet"},
    };
    volume2.setBoundaryNameMappings(boundaries2, 4);
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo volumes[2] = { volume1 ,volume2 };
    setting.setTargetVolumes(volumes, 2);

    PREPRO_BASE_NAMESPACE::PFData data;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editTransform(&setting, data);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit rotate." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    RenderDemo::show(data, VisualizationType::EPostProcess);
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::getRotateSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get setting of rotate:" << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFRotateSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFRotateSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "rotate name:" << currentSetting->getName() << std::endl;
    std::cout << "rotate source volume:" << currentSetting->getSourceVolumeName() << std::endl;
    int rotatedVolumeCount = currentSetting->getNumberOfTargetVolumes();
    std::cout << "rotate target volume count:" << rotatedVolumeCount << std::endl;
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo* volumes = new PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo[rotatedVolumeCount];
    PREPRO_BASE_NAMESPACE::PFStatus status = currentSetting->getTargetVolumes(volumes, rotatedVolumeCount);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get target volume of rotate" << name << std::endl;
        return status;
    }
    for (int index = 0;index<rotatedVolumeCount;++index)
    {
        std::cout << "rotated volume name:" << volumes[index].getTargetVolumeName() << std::endl;
        unsigned int boundaryCount = volumes[index].getBoundaryMappingCount();
        const PREPRO_POSTPROCESS_NAMESPACE::NamePair* names = volumes[index].getBoundaryNameMappings();
        for (unsigned int boundaryIndex = 0; boundaryIndex < boundaryCount; ++boundaryIndex)
        {
            std::cout << "rotated boundary source name:" << names[boundaryIndex].sourceName << "  rotated boundary target name:" << names[boundaryIndex].targetName << std::endl;
        }
    }
    delete[]volumes;
    volumes = nullptr;
    double center[3] = { 0. };
    currentSetting->getCenterPoint(center);
    std::cout << "rotate center: X:" << center[0] << "  Y:" << center[1] << "  Z:" << center[2] << std::endl;
    double axis[3] = { 0. };
    currentSetting->getRotateAxis(axis);
    std::cout << "rotate axis: X:" << axis[0] << "  Y:" << axis[1] << "  Z:" << axis[2] << std::endl;
    std::cout << "rotate start angle:" << currentSetting->getStartAngle()<< std::endl;
    std::cout << "rotate angle increment:" << currentSetting->getAngleIncrement() << std::endl;
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}
PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::renameRotate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& oldName,
    const std::string& newName)
{
    //rename
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->renameTransform(oldName.c_str(), newName.c_str());
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename rotate." << std::endl;
        return status;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::deleteRotate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFRotateSetting setting("rotateDemo2");
    setting.setSourceVolumeName("rotate1Eidt");

    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo volume1;
    volume1.setTargetVolumeName("rotateDemo2Volume");
    PREPRO_POSTPROCESS_NAMESPACE::NamePair pairs[4] =
    {
    {"rotate1z-inlet","Boundary1" },
    {"rotate1walls","Boundary2"},
    {"rotate1z-outlet","Boundary3" },
    {"rotate1y-inlet","Boundary4"},
    };
    volume1.setBoundaryNameMappings(pairs, 4);
    setting.setTargetVolumes(&volume1, 1);
    double center[3] = { 0.2,0. ,0. };
    setting.setCenterPoint(center);
    double axis[3] = { 0.,0. ,1. };
    setting.setRotateAxis(axis);
    setting.setStartAngle(90);
    setting.setAngleIncrement(90);
    PREPRO_BASE_NAMESPACE::PFData data;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createTransform(&setting, data);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create rotate." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    RenderDemo::show(data, VisualizationType::EPostProcess);

    status = postProcess->deleteTransform("rotateDemo2");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete transform." << std::endl;
        return status;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

void PostProcessTransformDemo::translateDemo(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_BASE_NAMESPACE::PFStatus status = createTranslate(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create translate." << std::endl;
        return;
    }
    status = editTranslate(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit translate." << std::endl;
        return;
    }

    status = getTranslateSetting(postProcess, "translateDemo1");
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to get setting of translate." << std::endl;
        return;
    }

    std::string newName = "translateDemo1Rename";
    status = renameTranslate(postProcess, "translateDemo1", newName);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename translate." << std::endl;
        return;
    }
    status = deleteTranslate(postProcess);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete translate." << std::endl;
        return;
    }
}
struct BoundingBox 
{
    double min[3] = { std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max() };
    double max[3] = { std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest(), std::numeric_limits<double>::lowest() };
    void update(const double* coord) 
    {
        for (int i = 0; i < 3; ++i) 
        {
            if (coord[i] < min[i])
            {
                min[i] = coord[i];
            }
            if (coord[i] > max[i])
            {
                max[i] = coord[i];
            }
        }
    }
};
BoundingBox computeBoundingBox(const PREPRO_BASE_NAMESPACE::PFData& pfData)
{
    BoundingBox boundingbox;
    unsigned int volumeSize = pfData.getVolumeSize();
    for (unsigned int i = 0; i < volumeSize; ++i) 
    {
        PREPRO_BASE_NAMESPACE::PFVolume* volume = pfData.getVolumes()[i];
        if (nullptr == volume)
        {
            continue;
        }
        unsigned int groupSize = volume->getGroupSize();

        for (unsigned int j = 0; j < groupSize; ++j)
        {
            PREPRO_BASE_NAMESPACE::PFGroup* group = volume->getGroups()[j];
            if (nullptr == group)
            {
                continue;
            }
            size_t vertexSize = group->getVertexSize();
            double* vertexes = group->getVertexes();
            for (size_t k = 0; k < vertexSize; ++k) 
            {
                const double* coord = vertexes + 3 * k;
                boundingbox.update(coord);
            }
        }
    }
    return boundingbox;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::createTranslate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    if (nullptr == postProcess)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    PREPRO_POSTPROCESS_NAMESPACE::PFTranslateSetting setting("translateDemo1");
    setting.setSourceVolumeName("Pipetee");
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo targetVolume;
    targetVolume.setTargetVolumeName("translate1");
    PREPRO_POSTPROCESS_NAMESPACE::NamePair boundaries[4] =
    {
    {"z-inlet","z-inlet" },
    {"walls","walls"},
    {"z-outlet","z-outlet" },
    {"y-inlet","y-inlet"},
    };
    targetVolume.setBoundaryNameMappings(boundaries, 4);
    setting.setTargetVolume(targetVolume);
    double translateVector[3] = { 10.,10,100 };
    setting.setTranslateVector(translateVector);
    PREPRO_BASE_NAMESPACE::PFData data;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createTransform(&setting, data);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create object." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    RenderDemo::show(data, VisualizationType::EPostProcess);
    //check data
    BoundingBox bbox = computeBoundingBox(data);
    std::cout << "bounding box min: (" << bbox.min[0] << ", " << bbox.min[1] << ", " << bbox.min[2] << ")\n";
    std::cout << "bounding box max: (" << bbox.max[0] << ", " << bbox.max[1] << ", " << bbox.max[2] << ")\n";
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::editTranslate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    PREPRO_POSTPROCESS_NAMESPACE::PFTranslateSetting setting("translateDemo1");
    setting.setSourceVolumeName("Pipetee");
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo targetVolume;
    targetVolume.setTargetVolumeName("translate1");
    PREPRO_POSTPROCESS_NAMESPACE::NamePair boundaries[4] =
    {
    {"z-inlet","translate1z-inlet" },
    {"walls","translate1walls"},
    {"z-outlet","translate1z-outlet" },
    {"y-inlet","translate1y-inlet"},
    };
    targetVolume.setBoundaryNameMappings(boundaries, 4);
    setting.setTargetVolume(targetVolume);
    double translateVector[3] = { -10.,-5,-20};
    setting.setTranslateVector(translateVector);
    PREPRO_BASE_NAMESPACE::PFData data;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->editTransform(&setting, data);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to edit transform." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    RenderDemo::show(data, VisualizationType::EPostProcess);
    BoundingBox bbox = computeBoundingBox(data);
    std::cout << "bounding box min: (" << bbox.min[0] << ", " << bbox.min[1] << ", " << bbox.min[2] << ")\n";
    std::cout << "bounding box max: (" << bbox.max[0] << ", " << bbox.max[1] << ", " << bbox.max[2] << ")\n";
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}
PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::getTranslateSetting(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& name)
{
    const PREPRO_POSTPROCESS_NAMESPACE::IPFPostProcessSetting* postSetting = postProcess->getSetting(name.c_str());
    if (nullptr == postSetting)
    {
        std::cout << "Failed to get setting of translate:" << name << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }

    const PREPRO_POSTPROCESS_NAMESPACE::PFTranslateSetting* currentSetting = dynamic_cast<const PREPRO_POSTPROCESS_NAMESPACE::PFTranslateSetting*>(postSetting);
    if (nullptr == currentSetting)
    {
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    std::cout << "translate name:" << currentSetting->getName() << std::endl;
    const PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo& targetVolume = currentSetting->getTargetVolume();
    std::cout << "translate source volume:" << currentSetting->getSourceVolumeName() << std::endl;
    std::cout << "translate target volume:" << targetVolume.getTargetVolumeName() << std::endl;

    double translateVector[3] = { 0. };
    currentSetting->getTranslateVector(translateVector);
    std::cout << "translate vector : X:" << translateVector[0] << "  Y:" << translateVector[1] << "  Z:" << translateVector[2] << std::endl;

    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::renameTranslate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess, const std::string& oldName,
    const std::string& newName)
{
    //rename
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->renameTransform(oldName.c_str(), newName.c_str());
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to rename translate." << std::endl;
        return status;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}

PREPRO_BASE_NAMESPACE::PFStatus PostProcessTransformDemo::deleteTranslate(PREPRO_POSTPROCESS_NAMESPACE::PFPostProcess* postProcess)
{
    std::string translateName = "translateDemo2";
    PREPRO_POSTPROCESS_NAMESPACE::PFTranslateSetting setting(translateName.c_str());
    setting.setSourceVolumeName("translate1");
    PREPRO_POSTPROCESS_NAMESPACE::PFVolumeInfo targetVolume;
    targetVolume.setTargetVolumeName("translate2");
    PREPRO_POSTPROCESS_NAMESPACE::NamePair boundaries[4] =
    {
    {"translate1z-inlet","z-inlet" },
    {"translate1walls","walls"},
    {"translate1z-outlet","z-outlet" },
    {"translate1y-inlet","y-inlet"},
    };
    targetVolume.setBoundaryNameMappings(boundaries, 4);
    setting.setTargetVolume(targetVolume);
    double translateVector[3] = { 1.,-1, 0 };
    setting.setTranslateVector(translateVector);
    PREPRO_BASE_NAMESPACE::PFData data;
    PREPRO_BASE_NAMESPACE::PFStatus status = postProcess->createTransform(&setting, data);
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to create translate." << setting.getName() << std::endl;
        return PREPRO_BASE_NAMESPACE::PFStatus::EError;
    }
    RenderDemo::show(data, VisualizationType::EPostProcess);

    status = postProcess->deleteTransform(translateName.c_str());
    if (status != PREPRO_BASE_NAMESPACE::PFStatus::EOkay)
    {
        std::cout << "Failed to delete translate." << setting.getName() << std::endl;
        return status;
    }
    return PREPRO_BASE_NAMESPACE::PFStatus::EOkay;
}
