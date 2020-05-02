

#ifndef MEMORYSINK_H
#define MEMORYSINK_H

#include "lib/api.h"

class MemorySink : public MPxNode {
    public:
        MemorySink();
        virtual ~MemorySink();

        virtual MStatus compute(
				const MPlug& plug, MDataBlock& data);

        static void* creator();
        static MStatus initialize();

public:
    static MTypeId kNODE_ID;
    static MString kNODE_NAME;
    
    // attribute MObjects
	static MObject aSourceConnection;
	static MObject aData;
	static MObject aTime;

    

};
#endif
	