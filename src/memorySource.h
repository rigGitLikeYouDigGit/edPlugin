

#ifndef MEMORYSOURCE_H
#define MEMORYSOURCE_H

#include "lib/api.h"

class MemorySource : public MPxNode {
    public:
        MemorySource();
        virtual ~MemorySource();

        virtual MStatus compute(
				const MPlug& plug, MDataBlock& data);

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
	static MObject aSinkConnection;
	static MObject aData;
    

};
#endif
	