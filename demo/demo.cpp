// Revision: $Id: demo.cpp,v 1.0 2025/01/22 16:21:04 xiaming Exp $
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
#include "importDemo.h"
#include "documentDemo.h"
#include "meshDemo.h"
#include "geometryDemo.h"
#include "commonMethods.h"
#include "postProcessDemo.h"

#include <iostream>

#ifdef WIN32
#include <Windows.h>
#endif

using namespace std;

/*-------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
    std::cout << "##### Demos, please select a number for category: " << std::endl;
    std::cout << "#####   1 - import." << std::endl;
    std::cout << "#####   2 - geometry." << std::endl;
    std::cout << "#####   3 - mesh." << std::endl;
    std::cout << "#####   4 - document." << std::endl;
    std::cout << "#####   5 - post process." << std::endl;
    std::cout << "#####   6 - only load post process(cgns)." << std::endl;
    std::cout << "#####   any other number for misc demo.\n" << std::endl;
    std::cout << "input number ->  " << std::endl;

    int type = 0;
    std::cin >> type;
    
    //get the real module file name.
    char buff[256];
    if (GetModuleFileName(nullptr, buff, 256) == 0)
    {
        return -1;
    }
    else
    {
        std::string executablePath(buff);
        std::string examplePath = CommonMethods::getCurrentProgramPath(executablePath) + "\\..\\examples\\";

        switch (type)
        {
        case 1:
            ImportDemo::importDemo(examplePath);
            break;
        case 2:
            GeometryDemo::geometryDemo(examplePath);
            break;
        case 3:
            MeshDemo::meshDemo(examplePath);
            break;
        case 4:
            DocumentDemo::documentDemo(examplePath);
            break;
        case 5:
            PostProcessDemo::postProcessDemo(examplePath);
            break;
        case 6:
            ImportDemo::importPostProcessResult(examplePath);
            break;
        default:
            break;
        }
    }
}