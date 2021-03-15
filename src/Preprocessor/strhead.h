/******************************************************************************
*
* project name:    TIGCC Tools Suite 
* file name:       strhead.h
* initial date:    13/08/2000
* author:          thomas.nussbaumer@gmx.net 
* description:     header structure of a TI string or OTH variable stored on
*                  the PC
*
* $Id: strhead.h,v 1.1 2000/08/23 20:31:13 Thomas Nussbaumer Exp $
*
******************************************************************************/

#ifndef __STRHEAD_H__
#define __STRHEAD_H__

typedef struct {
    char          signature[8]; // "**TI92P*" or "**TI89**"
    unsigned char fill1[2];     // 01 00
    char          folder[8];    // folder name
    char          desc[40];     // ---- not used ----
    unsigned char fill2[6];     // 01 00 52 00 00 00
    char          name[8];      // varname
    unsigned char type[4];      // 0C 00 00 00
    unsigned char size[4];      // complete file size (including checksum)
    unsigned char fill3[6];     // A5 5A 00 00 00 00
    unsigned char datasize[2];  // data size
}
StrHeader;

#endif

//#############################################################################
//###################### NO MORE FAKES BEYOND THIS LINE #######################
//#############################################################################
//
//=============================================================================
// Revision History
//=============================================================================
//
// $Log: strhead.h,v $
// Revision 1.1  2000/08/23 20:31:13  Thomas Nussbaumer
// renamed from pc_strdef.h
//
//
// ---------------------------------------------------------------------------
// revision info before file was renamed
// ---------------------------------------------------------------------------
// Revision 1.4  2000/08/23 01:05:00  Thomas Nussbaumer
// minor comment added
//
// Revision 1.3  2000/08/20 15:25:24  Thomas Nussbaumer
// bug fixed: size[] and datasize[] must be unsigned
//
// Revision 1.2  2000/08/13 20:24:51  Thomas Nussbaumer
// some definitions moved to tt.h
//
// Revision 1.1  2000/08/13 16:02:13  Thomas Nussbaumer
// initial version
//
//
//
