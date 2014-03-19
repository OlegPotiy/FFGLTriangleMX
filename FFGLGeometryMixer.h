#ifndef FFGLGEOMETRYMIXER_H
#define FFGLGEOMETRYMIXER_H

#include "FFGL/FFGLPluginSDK.h"
#include "FFGL/FFGLLib.h"


class FFGLGeometryMixer : public CFreeFrameGLPlugin
{
public:

	enum TexPattern:unsigned char {
		SplitAsGeometry = 0, // All triangles use splitted texcoords
		OnlyTwoCentral = 1, // Only two central triangles use central texmapping
		WithAdjacents = 2, // Two central triangles and four adjacent triangles use central texmapping
		All = 3 // All triangles use central texmapping
	};

	FFGLGeometryMixer(void);
	~FFGLGeometryMixer(void);


	///////////////////////////////////////////////////
	// FreeFrame plugin methods
	///////////////////////////////////////////////////

	DWORD	SetParameter(const SetParameterStruct* pParam);		
	DWORD	GetParameter(DWORD dwIndex);					

	DWORD	ProcessOpenGL(ProcessOpenGLStruct* pGL);

	DWORD	InitGL(const FFGLViewportStruct *vp);
	DWORD	DeInitGL();



	///////////////////////////////////////////////////
	// Factory method
	///////////////////////////////////////////////////

	static DWORD __stdcall CreateInstance(CFreeFrameGLPlugin **ppInstance)
	{
		*ppInstance = new FFGLGeometryMixer();
		if (*ppInstance != NULL) return FF_SUCCESS;
		return FF_FAIL;
	}

private:
	// Parameters
	float m_blend;
	TexPattern pattern;

	// Effects implementation
	DWORD TriangleMeshMix(ProcessOpenGLStruct* pGL);

	DWORD ScalableTriangleMeshMix(ProcessOpenGLStruct* pGL);


};

#endif
