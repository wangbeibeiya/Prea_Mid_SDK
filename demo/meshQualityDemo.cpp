// Revision: $Id: meshQualityDemo.cpp,v 1.0 2025/04/11 17:58:20 Leon Exp $
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

#include "meshDemo.h"
#include <iostream>
#include <base/pfApplication.h>
#include <base/pfDocument.h>
#include <base/common/pfEnumeration.h>
#include <base/pfGroupData.h>
#include <mesh/pfmesh.h>
#include <mesh/pfMeshBuilder.h>
#include <mesh/pfMeshData.h>
#include <mesh/pfQualityData.h>
#include <mesh/pfVolumeData.h>
#include <mesh/pfMeshQuality.h>

#include "documentDemo.h"
#include "commonMethods.h"
#include <geometry/common/pfGeometryMacro.h>
#include <geometry/pfGeometryBuilder.h>
#include <geometry/pfGeometry.h>
#include <mesh/common/pfConstDefinitions.h>
#include "renderDemo.h"


void MeshDemo::meshQualityDemo(const std::string& path)
{
    PREPRO_BASE_NAMESPACE::PFApplication pfApplication;
    PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder* geomertyBuilder = PREPRO_GEOMETRY_NAMESPACE::PFGeometryBuilder::getInstance();
    if (nullptr != geomertyBuilder)
    {
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(geomertyBuilder))
        {
            printf_s("No PERA SIM license");
            return;
        }
    }
    PREPRO_MESH_NAMESPACE::PFMeshBuilder* meshBuilder = PREPRO_MESH_NAMESPACE::PFMeshBuilder::getInstance();
    if (nullptr != meshBuilder)
    {
        if (PREPRO_BASE_NAMESPACE::PFStatus::ENoLicense == pfApplication.addEnvironment(meshBuilder))
        {
            printf_s("No PERA SIM license");
            return;
        }
    }

    std::string filePath = path + "mesh\\convert_polyhedral_mesh.ppcf";
    PFDocument* pfDocument = pfApplication.openDocument(filePath.c_str());
    if (nullptr == pfDocument)
    {
        std::cout << "!!! Document open failed." << std::endl;
        return;
    }

    PREPRO_GEOMETRY_NAMESPACE::PFGeometry* pfGeometry = dynamic_cast<PFGeometry*>(pfDocument->getGeometryEnvironment());
    if (nullptr == pfGeometry)
    {
        std::cout << "!!! Geometry is null." << std::endl;
        return;
    }

    PREPRO_MESH_NAMESPACE::PFMesh* pfMesh = dynamic_cast<PREPRO_MESH_NAMESPACE::PFMesh*>(pfDocument->getMeshEnvironment());
    if (nullptr == pfMesh)
    {
        return;
    }

    PREPRO_MESH_NAMESPACE::PFMeshQuality* pfMeshQuality = pfMesh->getMeshQuality();

    getOrthogonality(pfMeshQuality);

    getQuality(pfMeshQuality);

    getVolume(pfMeshQuality);

    getSkewness(pfMeshQuality);

    getAspectRatio(pfMeshQuality);

    getOrthogonalityFluent(pfMeshQuality);

    getAspectRatioFluent(pfMeshQuality);

    getOrthogonalitySkewFluent(pfMeshQuality);

    getJacobian(pfMeshQuality);

    PREPRO_BASE_NAMESPACE::PFData pfData;
    pfMesh->getAllData(pfData);
    RenderDemo::show(pfData, VisualizationType::EMesh);

    DocumentDemo::closeDocument(&pfApplication, pfDocument, false);
}

void MeshDemo::getOrthogonality(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality)
{
    //#################################
    // get orthogonality.
    std::cout << "\n\n**********************************************************\n";
    std::cout << " Get orthogonality. \n";
    std::cout << "**********************************************************\n";
    PREPRO_MESH_NAMESPACE::PFQualityDataCollection* pfMetricOrtho = pMeshQuality->getOrthogonalityData();
    if (pfMetricOrtho)
    {
        int metricOrthoCount = pfMetricOrtho->getQualityCount();
        for (int i = 0; i < metricOrthoCount; ++i)
        {
            const PREPRO_MESH_NAMESPACE::PFQualityData* quality = pfMetricOrtho->getQualityData(i);
            int type = quality->getType();
            int elementCount = quality->getElementCount();
            int invalidCount = quality->getInvalidCount();
            int qualityCount = quality->getQualityCount(); // 9 for number of quality. max range is 90. so each range is 10.
            const int* qualityCounts = quality->getCounts();
            std::cout << "Total elements : " << elementCount << std::endl;
            std::cout << "invalid count(out of range) : " << invalidCount << std::endl;
            std::cout << "quality number : " << qualityCount << "\n\n";

            for (int i = 0; i < qualityCount; ++i)
            {
                std::cout << i * 10 << "<= Q <=" << (i + 1) * 10 << " : ";
                std::cout << qualityCounts[i] << std::endl;
            }
            std::cout << std::endl;
            std::cout << "Best quality: " << quality->getQualityValue()[0] << std::endl;
            std::cout << "Worst quality: " << quality->getQualityValue()[1] << std::endl;
            std::cout << "Average quality: " << quality->getQualityValue()[2] << std::endl;

            std::cout << "\n------------------------------------------------\n";
        }
    }
}

void MeshDemo::getQuality(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality)
{
    //#################################
    // get quality
    std::cout << "\n\n**********************************************************\n";
    std::cout << " Get quality. \n";
    std::cout << "**********************************************************\n";
    PREPRO_MESH_NAMESPACE::PFQualityDataCollection* pfMetricQuality = pMeshQuality->getQualityData();
    if (pfMetricQuality)
    {
        int metricCount = pfMetricQuality->getQualityCount();
        for (int i = 0; i < metricCount; ++i)
        {
            const PREPRO_MESH_NAMESPACE::PFQualityData* quality = pfMetricQuality->getQualityData(i);
            int type = quality->getType();
            int elementCount = quality->getElementCount();
            int invalidCount = quality->getInvalidCount();
            int qualityCount = quality->getQualityCount(); // 10 for total number, max range is 1.0, so each range is 0.1.
            const int* qualityCounts = quality->getCounts();
            std::cout << "Total elements : " << elementCount << std::endl;
            std::cout << "invalid count(out of range) : " << invalidCount << std::endl;
            std::cout << "quality number : " << qualityCount << "\n\n";

            for (int i = qualityCount - 1; i >= 0; --i)
            {
                std::cout << i * 0.1 << "< Q <=" << (i + 1) * 0.1 << " : ";
                std::cout << qualityCounts[i] << std::endl;
            }
            std::cout << std::endl;
            std::cout << "Worst quality: " << quality->getQualityValue()[0] << std::endl;
            std::cout << "Best quality: " << quality->getQualityValue()[1] << std::endl;
            std::cout << "Average quality: " << quality->getQualityValue()[2] << std::endl;

            std::cout << "\n------------------------------------------------\n";
        }
    }
}

void MeshDemo::getVolume(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality)
{
    //#################################
    // get volume
    std::cout << "\n\n**********************************************************\n";
    std::cout << " Get volume (mesh quality). \n";
    std::cout << "**********************************************************\n";
    PREPRO_MESH_NAMESPACE::PFVolumeDataCollection* pfMetricVolume = pMeshQuality->getVolumeData();
    if (pfMetricVolume)
    {
        int metricCount = pfMetricVolume->getVolumeCount();
        for (int i = 0; i < metricCount; ++i)
        {
            const PREPRO_MESH_NAMESPACE::PFVolumeData* pfVolume = pfMetricVolume->getVolumeData(i);
            int type = pfVolume->getType();
            int elementCount = pfVolume->getElementCount();
            int invalidCount = pfVolume->getInvalidCount();
            int volumeCount = pfVolume->getNumVolume();
            const int* volumeCounts = pfVolume->getCounts();
            std::cout << "Total elements : " << elementCount << std::endl;
            std::cout << "invalid count(out of range) : " << invalidCount << std::endl;
            std::cout << "\n\n";

            float minRange = 0;
            float maxRange = 0.0001;
            for (int i = 0; i < volumeCount; ++i)
            {
                std::cout << minRange << "< Q <=" << maxRange << " : ";
                minRange = maxRange;
                maxRange = minRange * 10;
                std::cout << volumeCounts[i] << std::endl;
            }
            std::cout << std::endl;
            std::cout << "Minimum Volume(m^3): " << pfVolume->getVolumeValue()[0] << std::endl;
            std::cout << "Maximum Volume(m^3): " << pfVolume->getVolumeValue()[1] << std::endl;
            std::cout << "Average volume(m^3): " << pfVolume->getVolumeValue()[2] << std::endl;

            std::cout << "\n------------------------------------------------\n";
        }
    }
}

void MeshDemo::getSkewness(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality)
{
    //#################################
    // get skewness.
    std::cout << "\n\n**********************************************************\n";
    std::cout << " Get skewness. \n";
    std::cout << "**********************************************************\n";
    PREPRO_MESH_NAMESPACE::PFQualityDataCollection* pfSkewness = pMeshQuality->getSkewnessData();
    if (pfSkewness)
    {
        int metricCount = pfSkewness->getQualityCount();
        for (int i = 0; i < metricCount; ++i)
        {
            const PREPRO_MESH_NAMESPACE::PFQualityData* quality = pfSkewness->getQualityData(i);
            int type = quality->getType();
            int elementCount = quality->getElementCount();
            int invalidCount = quality->getInvalidCount();
            int qualityCount = quality->getQualityCount(); // 8 for skewness. max for this range is 4, so each range is 0.5.
            const int* qualityCounts = quality->getCounts();
            std::cout << "Total elements : " << elementCount << std::endl;
            std::cout << "invalid count(out of range) : " << invalidCount << std::endl;
            std::cout << "quality number : " << qualityCount << "\n\n";

            for (int i = qualityCount - 1; i >= 0; --i)
            {
                std::cout << i * 0.5 << "< Q <=" << (i + 1) * 0.5 << " : ";
                std::cout << qualityCounts[i] << std::endl;
            }
            std::cout << std::endl;
            std::cout << "Best quality: " << quality->getQualityValue()[0] << std::endl;
            std::cout << "Worst quality: " << quality->getQualityValue()[1] << std::endl;
            std::cout << "Average quality: " << quality->getQualityValue()[2] << std::endl;

            std::cout << "\n------------------------------------------------\n";
        }
    }
}

void MeshDemo::getAspectRatio(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality)
{
    //#################################
    // get aspect ratio.
    std::cout << "\n\n**********************************************************\n";
    std::cout << " Get aspect ratio. \n";
    std::cout << "**********************************************************\n";
    PREPRO_MESH_NAMESPACE::PFVolumeDataCollection* pfAspectRatio = pMeshQuality->getAspectRatioData();
    if (pfAspectRatio)
    {
        int metricCount = pfAspectRatio->getVolumeCount();
        for (int i = 0; i < metricCount; ++i)
        {
            const PREPRO_MESH_NAMESPACE::PFVolumeData* pfVolume = pfAspectRatio->getVolumeData(i);
            int type = pfVolume->getType();
            int elementCount = pfVolume->getElementCount();
            int invalidCount = pfVolume->getInvalidCount();
            int volumeCount = pfVolume->getNumVolume();
            const int* volumeCounts = pfVolume->getCounts();
            std::cout << "Total elements : " << elementCount << std::endl;
            std::cout << "invalid aspect ratio : " << invalidCount << std::endl;

            float minRange = 0;
            float maxRange = 10;
            for (int i = 0; i < volumeCount; ++i)
            {
                std::cout << minRange << "<= A <=" << maxRange << " : ";
                minRange = maxRange;
                maxRange = minRange * 2;
                std::cout << volumeCounts[i] << std::endl;
            }
            std::cout << std::endl;
            std::cout << "Minimum aspect ratio: " << pfVolume->getVolumeValue()[0] << std::endl;
            std::cout << "Maximum aspect ratio: " << pfVolume->getVolumeValue()[1] << std::endl;
            std::cout << "Average aspect ratio: " << pfVolume->getVolumeValue()[2] << std::endl;

            std::cout << "\n------------------------------------------------\n";
        }
    }
}

void MeshDemo::getOrthogonalityFluent(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality)
{
    //#################################
    // get orthogonality fluent.
    std::cout << "\n\n**********************************************************\n";
    std::cout << " Get orthogonality fluent. \n";
    std::cout << "**********************************************************\n";
    PREPRO_MESH_NAMESPACE::PFQualityDataCollection* pfOrthoFluent = pMeshQuality->getOrthogonalityFluentData();
    if (pfOrthoFluent)
    {
        int metricCount = pfOrthoFluent->getQualityCount();
        for (int i = 0; i < metricCount; ++i)
        {
            const PREPRO_MESH_NAMESPACE::PFQualityData* quality = pfOrthoFluent->getQualityData(i);
            int type = quality->getType();
            int elementCount = quality->getElementCount();
            int invalidCount = quality->getInvalidCount();
            int qualityCount = quality->getQualityCount(); // 10 for ortho(fluent). max for this range is 1, so each range is 0.1.
            const int* qualityCounts = quality->getCounts();
            std::cout << "Total elements : " << elementCount << std::endl;
            std::cout << "invalid count(out of range) : " << invalidCount << std::endl;
            std::cout << "quality number : " << qualityCount << "\n\n";

            for (int i = qualityCount - 1; i >= 0; --i)
            {
                std::cout << i * 0.1 << "< Q <=" << (i + 1) * 0.1 << " : ";
                std::cout << qualityCounts[i] << std::endl;
            }
            std::cout << std::endl;
            std::cout << "Worst quality: " << quality->getQualityValue()[0] << std::endl;
            std::cout << "Best quality: " << quality->getQualityValue()[1] << std::endl;
            std::cout << "Average quality: " << quality->getQualityValue()[2] << std::endl;

            std::cout << "\n------------------------------------------------\n";
        }
    }
}

void MeshDemo::getAspectRatioFluent(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality)
{
    //#################################
    // get aspect ratio (fluent).
    std::cout << "\n\n**********************************************************\n";
    std::cout << " Get aspect ratio (fluent). \n";
    std::cout << "**********************************************************\n";
    PREPRO_MESH_NAMESPACE::PFVolumeDataCollection* pfAspectRatioFluent = pMeshQuality->getAspectRatioFluentData();
    if (pfAspectRatioFluent)
    {
        int metricCount = pfAspectRatioFluent->getVolumeCount();
        for (int i = 0; i < metricCount; ++i)
        {
            const PREPRO_MESH_NAMESPACE::PFVolumeData* pfVolume = pfAspectRatioFluent->getVolumeData(i);
            int type = pfVolume->getType();
            int elementCount = pfVolume->getElementCount();
            int invalidCount = pfVolume->getInvalidCount();
            int volumeCount = pfVolume->getNumVolume();
            const int* volumeCounts = pfVolume->getCounts();
            std::cout << "Total elements : " << elementCount << std::endl;
            std::cout << "invalid aspect ratio : " << invalidCount << std::endl;
            std::cout << "aspect ratio (fluent) number : " << volumeCount << "\n\n";

            float minRange = 0;
            float maxRange = 10;
            for (int i = 0; i < volumeCount; ++i)
            {
                std::cout << minRange << "<= A <=" << maxRange << " : ";
                minRange = maxRange;
                maxRange = minRange * 2;
                std::cout << volumeCounts[i] << std::endl;
            }
            std::cout << std::endl;
            std::cout << "Minimum aspect ratio: " << pfVolume->getVolumeValue()[0] << std::endl;
            std::cout << "Maximum aspect ratio: " << pfVolume->getVolumeValue()[1] << std::endl;
            std::cout << "Average aspect ratio: " << pfVolume->getVolumeValue()[2] << std::endl;

            std::cout << "\n------------------------------------------------\n";
        }
    }
}

void MeshDemo::getOrthogonalitySkewFluent(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality)
{
    //#################################
    // get orthogonality skew fluent.
    std::cout << "\n\n**********************************************************\n";
    std::cout << " Get orthogonality skew fluent. \n";
    std::cout << "**********************************************************\n";
    PREPRO_MESH_NAMESPACE::PFQualityDataCollection* pfOrthoSkewFluent = pMeshQuality->getOrthogonalitySkewFluentData();
    if (pfOrthoSkewFluent)
    {
        int metricCount = pfOrthoSkewFluent->getQualityCount();
        for (int i = 0; i < metricCount; ++i)
        {
            const PREPRO_MESH_NAMESPACE::PFQualityData* quality = pfOrthoSkewFluent->getQualityData(i);
            int type = quality->getType();
            int elementCount = quality->getElementCount();
            int invalidCount = quality->getInvalidCount();
            int qualityCount = quality->getQualityCount(); // 10 for ortho skew(fluent). max for this range is 1, so each range is 0.1.
            const int* qualityCounts = quality->getCounts();
            std::cout << "Total elements : " << elementCount << std::endl;
            std::cout << "invalid count(out of range) : " << invalidCount << std::endl;
            std::cout << "quality number : " << qualityCount << "\n\n";

            for (int i = qualityCount - 1; i >= 0; --i)
            {
                std::cout << i * 0.1 << "< Q <=" << (i + 1) * 0.1 << " : ";
                std::cout << qualityCounts[i] << std::endl;
            }
            std::cout << std::endl;
            std::cout << "Worst quality: " << quality->getQualityValue()[0] << std::endl;
            std::cout << "Best quality: " << quality->getQualityValue()[1] << std::endl;
            std::cout << "Average quality: " << quality->getQualityValue()[2] << std::endl;

            std::cout << "\n------------------------------------------------\n";
        }
    }
}

void MeshDemo::getJacobian(PREPRO_MESH_NAMESPACE::PFMeshQuality* pMeshQuality)
{
    //#################################
    // get Jacobian.
    std::cout << "\n\n**********************************************************\n";
    std::cout << " Get Jacobian. \n";
    std::cout << "**********************************************************\n";
    PREPRO_MESH_NAMESPACE::PFQualityDataCollection* pfJacobian = pMeshQuality->getJacobianData();
    if (pfJacobian)
    {
        int metricCount = pfJacobian->getQualityCount();
        for (int i = 0; i < metricCount; ++i)
        {
            const PREPRO_MESH_NAMESPACE::PFQualityData* quality = pfJacobian->getQualityData(i);
            int type = quality->getType();
            int elementCount = quality->getElementCount();
            int invalidCount = quality->getInvalidCount();
            int qualityCount = quality->getQualityCount();
            const int* qualityCounts = quality->getCounts();
            std::cout << "Total elements : " << elementCount << std::endl;
            std::cout << "invalid count(out of range) : " << invalidCount << std::endl;

            std::cout << std::endl;
            std::cout << "Minimum Jacobian: " << quality->getQualityValue()[0] << std::endl;
            std::cout << "Maximum Jacobian: " << quality->getQualityValue()[1] << std::endl;
            std::cout << "Average Jacobian: " << quality->getQualityValue()[2] << std::endl;

            std::cout << "\n------------------------------------------------\n";
        }
    }
}