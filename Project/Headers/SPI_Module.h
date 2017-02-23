/****************************************************************************
 Module
   SPI_Module.h

 Revision
   1.0.1

 Description
   Header file for the SPI module

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/15/12 11:12 jec      revisions for Gen2 framework
 11/07/11 11:26 jec      made the queue static
 10/30/11 17:59 jec      fixed references to CurrentEvent in RunTemplateSM()
 10/23/11 18:20 jec      began conversion from SMTemplate.c (02/20/07 rev)
****************************************************************************/

#ifndef SPI_MODULE_H
#define SPI_MODULE_H


void InitSPI_Comm(void);

void QueryLOC(uint8_t QueryVal);


#endif
