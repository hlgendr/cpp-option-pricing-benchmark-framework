// ExcelImports.cpp
//
// (C) Datasim Education BV 2005
//
// Import necessary typelib's for Excel 

#ifndef ExcelImports_CPP
#define ExcelImports_CPP

// Define these macros from the project or environment when building the Excel demo line.
// The main benchmark framework does not depend on Office COM.
#ifndef DATASIM_MSO_DLL_PATH
#define DATASIM_MSO_DLL_PATH "MSO.DLL"
#endif

#ifndef DATASIM_VBE6EXT_OLB_PATH
#define DATASIM_VBE6EXT_OLB_PATH "VBE6EXT.OLB"
#endif

#ifndef DATASIM_EXCEL_EXE_PATH
#define DATASIM_EXCEL_EXE_PATH "EXCEL.EXE"
#endif

#import DATASIM_MSO_DLL_PATH \
    rename("DocumentProperties", "DocumentPropertiesXL") \
    rename("RGB", "RGBXL")

#import DATASIM_VBE6EXT_OLB_PATH

#import DATASIM_EXCEL_EXE_PATH \
    rename("DialogBox", "DialogBoxXL") \
    rename("RGB", "RGBXL") \
    rename("DocumentProperties", "DocumentPropertiesXL") \
    rename("ReplaceText", "ReplaceTextXL") \
    rename("CopyFile", "CopyFileXL") \
    no_dual_interfaces \
    exclude("IFont", "IPicture")

#endif
