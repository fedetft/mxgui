//Forced include file for Visual Studio
//It will be automatically included into each .cpp file in the project

#ifndef _FORCED_INCLUDE_H_
#define _FORCED_INCLUDE_H_

//__attribute__() is not supported by MSVC
#define __attribute__(x)

//Set 1-byte struct member alignment instead of __attribute__((packed))
#pragma pack(push,1)

//Override QT include files: they conflict with windows ones.
#define QTBACKEND_H
#define DISPLAY_QT_H
#define EVENT_QT_H
#define EVENT_TYPES_QT_H

#endif //_FORCED_INCLUDE_H_
