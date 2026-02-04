// Revision: $Id: commonEnumeration.h 1.0 2025/02/26 13:17:05 liujing Exp $
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
 * and may be covered by China and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from PeraGlobal Technologies, Inc.
 */
#pragma once

 /*! \enum VisualizationType
 * options for visualization.
 */

enum class VisualizationType
{
    EGeomerty = 0, /*!< geometry type. */
    EMesh = 1,		/*!< mesh type. */
    EPostProcess = 2,		/*!< post process type. */
};