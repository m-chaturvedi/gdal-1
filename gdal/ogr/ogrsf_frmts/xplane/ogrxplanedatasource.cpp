/******************************************************************************
 *
 * Project:  X-Plane aeronautical data reader
 * Purpose:  Implements OGRXPlaneDataSource class
 * Author:   Even Rouault, even dot rouault at mines dash paris dot org
 *
 ******************************************************************************
 * Copyright (c) 2008-2010, Even Rouault <even dot rouault at mines-paris dot org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

#include "ogr_xplane.h"
#include "ogr_xplane_reader.h"

CPL_CVSID("$Id$")

/************************************************************************/
/*                          OGRXPlaneDataSource()                          */
/************************************************************************/

OGRXPlaneDataSource::OGRXPlaneDataSource() :
    pszName(nullptr),
    papoLayers(nullptr),
    nLayers(0),
    poReader(nullptr),
    bReadWholeFile(true),
    bWholeFiledReadingDone(false)
{}

/************************************************************************/
/*                         ~OGRXPlaneDataSource()                       */
/************************************************************************/

OGRXPlaneDataSource::~OGRXPlaneDataSource()

{
    Reset();
}

/************************************************************************/
/*                              Reset()                                 */
/************************************************************************/

void OGRXPlaneDataSource::Reset()
{
    if ( poReader != nullptr)
    {
        delete poReader;
        poReader = nullptr;
    }

    CPLFree( pszName );
    pszName = nullptr;

    for( int i = 0; i < nLayers; i++ )
        delete papoLayers[i];
    CPLFree( papoLayers );
    papoLayers = nullptr;
    nLayers = 0;
}

/************************************************************************/
/*                              GetLayer()                              */
/************************************************************************/

OGRLayer *OGRXPlaneDataSource::GetLayer( int iLayer )

{
    if( iLayer < 0 || iLayer >= nLayers )
        return nullptr;
    else
        return papoLayers[iLayer];
}

/************************************************************************/
/*                           RegisterLayer()                            */
/************************************************************************/

void OGRXPlaneDataSource::RegisterLayer(OGRXPlaneLayer* poLayer)
{
    poLayer->SetDataSource(this);

    papoLayers = (OGRXPlaneLayer**) CPLRealloc(papoLayers,
                                    (nLayers + 1) * sizeof(OGRXPlaneLayer*));
    papoLayers[nLayers++] = poLayer;
}

/************************************************************************/
/*                                Open()                                */
/************************************************************************/

int OGRXPlaneDataSource::Open( const char * pszFilename, int bReadWholeFileIn )

{
    Reset();

    bReadWholeFile = CPL_TO_BOOL(bReadWholeFileIn);

    const char* pszShortFilename = CPLGetFilename(pszFilename);
    if (EQUAL(pszShortFilename, "nav.dat") ||
        EQUAL(pszShortFilename, "earth_nav.dat"))
    {
        poReader = OGRXPlaneCreateNavFileReader(this);
    }
    else if (EQUAL(pszShortFilename, "apt.dat"))
    {
        poReader = OGRXPlaneCreateAptFileReader(this);
    }
    else if (EQUAL(pszShortFilename, "fix.dat") ||
             EQUAL(pszShortFilename, "earth_fix.dat"))
    {
        poReader = OGRXPlaneCreateFixFileReader(this);
    }
    else if (EQUAL(pszShortFilename, "awy.dat") ||
             EQUAL(pszShortFilename, "earth_awy.dat"))
    {
        poReader = OGRXPlaneCreateAwyFileReader(this);
    }

    if( poReader && !poReader->StartParsing(pszFilename) )
    {
        delete poReader;
        poReader = nullptr;
    }
    if( poReader )
    {
        pszName = CPLStrdup(pszFilename);

        if( !bReadWholeFile )
        {
            for( int i = 0; i < nLayers; i++ )
                papoLayers[i]->SetReader(poReader->CloneForLayer(papoLayers[i]));
        }
        return true;
    }

    return false;
}

/************************************************************************/
/*                           TestCapability()                           */
/************************************************************************/

int OGRXPlaneDataSource::TestCapability( CPL_UNUSED const char * pszCap )
{
    return FALSE;
}

/************************************************************************/
/*                     ReadWholeFileIfNecessary()                       */
/************************************************************************/

void OGRXPlaneDataSource::ReadWholeFileIfNecessary()
{
    if( bReadWholeFile && !bWholeFiledReadingDone )
    {
        poReader->ReadWholeFile();
        for( int i = 0; i < nLayers; i++ )
            papoLayers[i]->AutoAdjustColumnsWidth();
        bWholeFiledReadingDone = true;
    }
}
