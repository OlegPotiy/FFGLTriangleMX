#include "FFGLGeometryMixer.h"

#define	FFPARAM_Blend		(0)


static CFFGLPluginInfo PluginInfo ( 
	FFGLGeometryMixer::CreateInstance,	// Create method
	"TriX",							// Plugin unique ID
	"Triangle MX",						// Plugin name											
	1,						   			// API major version number 											
	000,								// API minor version number	
	1,									// Plugin major version number
	000,								// Plugin minor version number
	FF_EFFECT,							// Plugin type
	"FFGL Geometry Mixer",				// Plugin description
	"by Oleg Potiy"						// About
	);

FFGLGeometryMixer::FFGLGeometryMixer(void):CFreeFrameGLPlugin()
{
	// Input properties
	SetMinInputs(2);
	SetMaxInputs(2);

	// parameters:
	SetParamInfo(FFPARAM_Blend, "Blend", FF_TYPE_STANDARD, 0.5f);	
	m_blend = 0.5f;
}


FFGLGeometryMixer::~FFGLGeometryMixer(void)
{
}


DWORD	FFGLGeometryMixer::SetParameter(const SetParameterStruct* pParam)
{
	float fNewValue = 0;

	if (pParam != NULL) 
	{
		fNewValue = *((float *)(unsigned)&(pParam->NewParameterValue));

		switch(pParam->ParameterNumber)
		{
		case 0:

			this->m_blend = fNewValue;

			this->pattern = (TexPattern)(unsigned char)(this->m_blend * (TexPattern::All + 0.9));

			break;

		default:
			return FF_FAIL;
		}
	};
	return FF_SUCCESS;
}

DWORD FFGLGeometryMixer::GetParameter(DWORD dwIndex)
{
	DWORD dwReturnValue;

	switch (dwIndex) 
	{
	case FFPARAM_Blend:
		*((float*)(unsigned)(&dwReturnValue)) = this->m_blend;
		return dwReturnValue;

	default:
		return FF_FAIL;
	}

	return FF_FAIL;
}

DWORD FFGLGeometryMixer::ProcessOpenGL(ProcessOpenGLStruct* pGL)
{
	if (pGL->numInputTextures < 2) 
		return FF_FAIL;

	if (pGL->inputTextures[0]==NULL || pGL->inputTextures[1]==NULL ) 
		return FF_FAIL;

	return this->ScalableTriangleMeshMix(pGL);
}



const static double divisor = 1.7320508075688772935274463415059L;

DWORD FFGLGeometryMixer::ScalableTriangleMeshMix(ProcessOpenGLStruct* pGL)
{

	FFGLTextureStruct &TextureObject1 = *(pGL->inputTextures[0]);
	FFGLTextureStruct &TextureObject2 = *(pGL->inputTextures[1]);

	DWORD frameHeight = pGL->inputTextures[0]->HardwareHeight;
	DWORD frameWidth = pGL->inputTextures[0]->HardwareWidth;

	double triangleSide = frameHeight / divisor;
	double halfTriangleSide = frameHeight / (2. * divisor);

	int triNum = (int)((frameWidth / 2. - halfTriangleSide) / triangleSide) + 1;
	double nrmTriangleSide = triangleSide / frameWidth;
	double nrmHalfTriangleSide = halfTriangleSide / frameWidth;

	glEnable(GL_TEXTURE_2D);

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glHint(GL_POINT_SMOOTH, GL_NICEST);
	glHint(GL_LINE_SMOOTH, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH, GL_NICEST);

	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);


	double leftTexBorder = 0.5 - frameHeight/(frameWidth*divisor);
	double rightTexBorder = 0.5 + frameHeight/(frameWidth*divisor);


	if (this->pattern == TexPattern::SplitAsGeometry )
	{
		// Top central triangle
		glBindTexture(GL_TEXTURE_2D, TextureObject1.Handle);
		glBegin(GL_TRIANGLES);

		glTexCoord2f(0.5 - nrmHalfTriangleSide, 0.5);
		glVertex3f(-nrmTriangleSide, 0.0, 0.0);		

		glTexCoord2f(0.5, 1);
		glVertex3f(0.0, 1.0, 0.0);

		glTexCoord2f(0.5 + nrmHalfTriangleSide, 0.5);	
		glVertex3f(nrmTriangleSide, 0.0, 0.0);
		glEnd();

		// Bottom central triangle
		glBindTexture(GL_TEXTURE_2D, TextureObject2.Handle);
		glBegin(GL_TRIANGLES);

		glTexCoord2f(0.5 - (halfTriangleSide / (double)frameWidth), 0.5);		
		glVertex3f(-nrmTriangleSide, 0.0, 0.0);		

		glTexCoord2f(0.5 + (halfTriangleSide / (double)frameWidth), 0.5);		
		glVertex3f(nrmTriangleSide, 0.0, 0.0);

		glTexCoord2f(0.5, 0);
		glVertex3f(0.0, -1.0, 0.0);
		glEnd();
	}
	else
	{
		// Top central triangle
		glBindTexture(GL_TEXTURE_2D, TextureObject1.Handle);
		glBegin(GL_TRIANGLES);

		glTexCoord2f(leftTexBorder, 0.0);
		glVertex3f(-nrmTriangleSide, 0.0, 0.0);		

		glTexCoord2f(0.5, 1.0);
		glVertex3f(0.0, 1.0, 0.0);

		glTexCoord2f(rightTexBorder, 0.0);
		glVertex3f(nrmTriangleSide, 0.0, 0.0);

		glEnd();

		// Bottom central triangle
		glBindTexture(GL_TEXTURE_2D, TextureObject2.Handle);
		glBegin(GL_TRIANGLES);

		glTexCoord2f(leftTexBorder, 1.0);
		glVertex3f(-nrmTriangleSide, 0.0, 0.0);		

		glTexCoord2f(rightTexBorder, 1.0);
		glVertex3f(nrmTriangleSide, 0.0, 0.0);

		glTexCoord2f( 0.5, 0.0);
		glVertex3f(0.0, -1.0, 0.0);

		glEnd();
	}




	glBindTexture(GL_TEXTURE_2D, TextureObject1.Handle);
	glBegin(GL_TRIANGLES);

	double localShift = 0;

	for (int i=0 ; i < triNum ; i ++)
	{
		if (this->pattern == TexPattern::All )
		{
			// upper right triangles ->/
			localShift = nrmHalfTriangleSide + i*nrmTriangleSide; 
			glTexCoord2f(leftTexBorder, 0.0);
			glVertex3f( 2.*localShift , 0.0, 0.0);		

			localShift = (i+1)*nrmTriangleSide;
			glTexCoord2f(0.5, 1.0);
			glVertex3f(2.*localShift, 1.0, 0.0);

			localShift = nrmHalfTriangleSide + (i+1)*nrmTriangleSide; 
			glTexCoord2f(rightTexBorder, 0.0);
			glVertex3f(2.*localShift, 0.0, 0.0);


			// upper left triangles <-/
			localShift = nrmHalfTriangleSide + i*nrmTriangleSide; 
			glTexCoord2f(rightTexBorder, 0.0);
			glVertex3f( -2.*localShift, 0.0, 0.0);		

			localShift = (i+1)*nrmTriangleSide; 
			glTexCoord2f(0.5, 1.0);
			glVertex3f(-2.*localShift, 1.0, 0.0);

			localShift = nrmHalfTriangleSide + (i+1)*nrmTriangleSide; 
			glTexCoord2f(leftTexBorder, 0.0);
			glVertex3f(-2.*localShift, 0.0, 0.0);	
		}
		else
		{
			// upper right triangles ->/
			localShift = nrmHalfTriangleSide + i*nrmTriangleSide; 
			glTexCoord2f(0.5 + localShift, 0.5);
			glVertex3f( 2.*localShift , 0.0, 0.0);		

			localShift = (i+1)*nrmTriangleSide;
			glTexCoord2f(0.5  + localShift, 1.0);
			glVertex3f(2.*localShift, 1.0, 0.0);

			localShift = nrmHalfTriangleSide + (i+1)*nrmTriangleSide; 
			glTexCoord2f(0.5 + localShift, 0.5);
			glVertex3f(2.*localShift, 0.0, 0.0);


			// upper left triangles <-/
			localShift = nrmHalfTriangleSide + i*nrmTriangleSide; 
			glTexCoord2f(0.5 - localShift, 0.5);
			glVertex3f( -2.*localShift, 0.0, 0.0);		

			localShift = (i+1)*nrmTriangleSide; 
			glTexCoord2f(0.5  - localShift, 1.0);
			glVertex3f(-2.*localShift, 1.0, 0.0);

			localShift = nrmHalfTriangleSide + (i+1)*nrmTriangleSide; 
			glTexCoord2f(0.5 - localShift, 0.5);
			glVertex3f(-2.*localShift, 0.0, 0.0);		
		}

	}

	triNum = (double)frameWidth / 2. / triangleSide;
	triNum += 1;
	for (int i=0 ; i < triNum ; i ++)
	{
		if ((i == 0 && this->pattern == TexPattern::WithAdjacents) || this->pattern == TexPattern::All)
		{
			// lower right triangles /->
			localShift = i * nrmTriangleSide; 
			glTexCoord2f(leftTexBorder, 0.0);
			glVertex3f( 2.*localShift, -1., 0.0);		

			localShift = nrmHalfTriangleSide + i*nrmTriangleSide;
			glTexCoord2f(0.5, 1.);
			glVertex3f(2.*localShift, 0.0, 0.0);

			localShift = (i+1)*nrmTriangleSide;
			glTexCoord2f(rightTexBorder, 0.0);
			glVertex3f(2.*localShift, -1., 0.0);


			// lower left triangles /<-
			localShift = (i+1)*nrmTriangleSide; 
			glTexCoord2f(leftTexBorder, 0.0);
			glVertex3f(-2.*localShift, -1.0, 0.0);		

			localShift = nrmHalfTriangleSide + i*nrmTriangleSide;
			glTexCoord2f(0.5, 1.);
			glVertex3f(-2.*localShift, 0.0, 0.0);

			localShift = i*nrmTriangleSide;
			glTexCoord2f(rightTexBorder, 0.0);
			glVertex3f(-2.*localShift, -1.0, 0.0);
		}
		else
		{
			// lower right triangles /->
			localShift = i * nrmTriangleSide; 
			glTexCoord2f(0.5 + localShift, 0.0);
			glVertex3f( 2.*localShift, -1., 0.0);		

			localShift = nrmHalfTriangleSide + i*nrmTriangleSide;
			glTexCoord2f(0.5 + localShift, 0.5);
			glVertex3f(2.*localShift, 0.0, 0.0);

			localShift = (i+1)*nrmTriangleSide;
			glTexCoord2f(0.5 + localShift, 0.0);
			glVertex3f(2.*localShift, -1., 0.0);


			// lower left triangles /<-
			localShift = (i+1)*nrmTriangleSide; 
			glTexCoord2f(0.5 - localShift, 0.0);
			glVertex3f(-2.*localShift, -1.0, 0.0);		

			localShift = nrmHalfTriangleSide + i*nrmTriangleSide;
			glTexCoord2f(0.5 - localShift, 0.5);
			glVertex3f(-2.*localShift, 0.0, 0.0);

			localShift = i*nrmTriangleSide;
			glTexCoord2f(0.5 - localShift, 0.0);
			glVertex3f(-2.*localShift, -1.0, 0.0);		
		}
	};

	glEnd();

	// Bottom central triangle
	glBindTexture(GL_TEXTURE_2D, TextureObject2.Handle);
	glBegin(GL_TRIANGLES);

	for (int i=0 ; i < triNum ; i ++)
	{
		if (this->pattern == TexPattern::All )
		{
			// lower right triangles
			localShift = nrmHalfTriangleSide + i*nrmTriangleSide; 
			glTexCoord2f(leftTexBorder, 1.0);
			glVertex3f( 2.*localShift, 0.0, 0.0);		

			localShift = nrmHalfTriangleSide + (i+1)*nrmTriangleSide;
			glTexCoord2f(rightTexBorder, 1.0);
			glVertex3f(2.*localShift, 0.0, 0.0);

			localShift = (i+1)*nrmTriangleSide;
			glTexCoord2f(0.5, 0.0);
			glVertex3f(2.*localShift, -1.0, 0.0);		

			// lower left triangles
			localShift = nrmHalfTriangleSide + (i+1)*nrmTriangleSide; 
			glTexCoord2f(leftTexBorder, 1.0);
			glVertex3f(-2.*localShift , 0.0, 0.0);

			localShift = nrmHalfTriangleSide + i*nrmTriangleSide; 
			glTexCoord2f(rightTexBorder, 1.0);
			glVertex3f( -2.*localShift, 0.0, 0.0);		

			localShift = (i+1)*nrmTriangleSide; 
			glTexCoord2f(0.5, 0.0);
			glVertex3f(-2.*localShift, -1.0, 0.0);
		}
		else
		{
			// lower right triangles
			localShift = nrmHalfTriangleSide + i*nrmTriangleSide; 
			glTexCoord2f(0.5 + localShift, 0.5);
			glVertex3f( 2.*localShift, 0.0, 0.0);		

			localShift = nrmHalfTriangleSide + (i+1)*nrmTriangleSide;
			glTexCoord2f(0.5 + localShift, 0.5);
			glVertex3f(2.*localShift, 0.0, 0.0);

			localShift = (i+1)*nrmTriangleSide;
			glTexCoord2f(0.5  + localShift , 0.0);
			glVertex3f(2.*localShift, -1.0, 0.0);		

			// lower left triangles
			localShift = nrmHalfTriangleSide + (i+1)*nrmTriangleSide; 
			glTexCoord2f(0.5 - localShift, 0.5);
			glVertex3f(-2.*localShift , 0.0, 0.0);

			localShift = nrmHalfTriangleSide + i*nrmTriangleSide; 
			glTexCoord2f(0.5 - localShift, 0.5);
			glVertex3f( -2.*localShift, 0.0, 0.0);		

			localShift = (i+1)*nrmTriangleSide; 
			glTexCoord2f(0.5  - localShift, 0.0);
			glVertex3f(-2.*localShift, -1.0, 0.0);
		}
	}

	triNum = (double)frameWidth / 2. / triangleSide;
	triNum += 1;
	for (int i=0 ; i < triNum ; i ++)
	{
		if ( (i == 0 && this->pattern == TexPattern::WithAdjacents) || this->pattern == TexPattern::All)
		{
			// upper right triangles /->
			localShift = i*nrmTriangleSide;
			glTexCoord2f(leftTexBorder, 1.0);
			glVertex3f(2.*localShift , 1., 0.0);		

			localShift = (i+1)*nrmTriangleSide;
			glTexCoord2f(rightTexBorder, 1.0);
			glVertex3f(2.*localShift, 1., 0.0);

			localShift = nrmHalfTriangleSide + i*nrmTriangleSide;
			glTexCoord2f(0.5, 0.);
			glVertex3f(2.*localShift, 0.0, 0.0);

			// upper left triangles /<-
			localShift = (i+1)*nrmTriangleSide;
			glTexCoord2f(leftTexBorder, 1.0);
			glVertex3f(-2.*localShift, 1.0, 0.0);

			localShift = i*nrmTriangleSide;
			glTexCoord2f(rightTexBorder, 1.0);
			glVertex3f(-2.*localShift, 1.0, 0.0);

			localShift = nrmHalfTriangleSide + i*nrmTriangleSide;
			glTexCoord2f(0.5, 0.);
			glVertex3f(-2.*localShift, 0.0, 0.0);
		}
		else
		{
			// upper right triangles /->
			localShift = i*nrmTriangleSide;
			glTexCoord2f(0.5 + localShift , 1.0);
			glVertex3f(2.*localShift , 1., 0.0);		

			localShift = (i+1)*nrmTriangleSide;
			glTexCoord2f(0.5 + localShift, 1.0);
			glVertex3f(2.*localShift, 1., 0.0);

			localShift = nrmHalfTriangleSide + i*nrmTriangleSide;
			glTexCoord2f(0.5 + localShift, 0.5);
			glVertex3f(2.*localShift, 0.0, 0.0);

			// upper left triangles /<-
			localShift = (i+1)*nrmTriangleSide;
			glTexCoord2f(0.5 - localShift, 1.0);
			glVertex3f(-2.*localShift, 1.0, 0.0);

			localShift = i*nrmTriangleSide;
			glTexCoord2f(0.5 - localShift, 1.0);
			glVertex3f(-2.*localShift, 1.0, 0.0);

			localShift = nrmHalfTriangleSide + i*nrmTriangleSide;
			glTexCoord2f(0.5 - localShift, 0.5);
			glVertex3f(-2.*localShift, 0.0, 0.0);	
		}
	};

	glEnd();


	return FF_SUCCESS;
}

DWORD FFGLGeometryMixer::HorizontalSlide(ProcessOpenGLStruct* pGL)
{

	FFGLTextureStruct &TextureObject1 = *(pGL->inputTextures[0]);
	FFGLTextureStruct &TextureObject2 = *(pGL->inputTextures[1]);


	FFGLTexCoords maxCoordsTO1 = GetMaxGLTexCoords(TextureObject1);
	FFGLTexCoords maxCoordsTO2 = GetMaxGLTexCoords(TextureObject2);


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-this->m_blend * 2.0 ,0,0);

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, TextureObject1.Handle);


	float f_verticalBorder = - 2.0 * this->m_blend + 1.0;

	glBegin(GL_QUADS);

	glTexCoord2f(0,0);
	glVertex3f(-1.0, -1.0, 0.0);		

	glTexCoord2f(0, maxCoordsTO1.t);
	glVertex3f(-1.0, 1.0, 0.0);

	glTexCoord2f(maxCoordsTO1.s, maxCoordsTO1.t);		
	glVertex3f( 1.0, 1.0, 0.0);

	glTexCoord2f(maxCoordsTO1.s, 0);
	glVertex3f( 1.0, -1.0, 0.0);

	glEnd();


	glBindTexture(GL_TEXTURE_2D, TextureObject2.Handle);

	glBegin(GL_QUADS);

	//lower left
	glTexCoord2f(0.0, 0.0);
	glVertex3f(1.0, -1.0, 0.0);

	//upper left
	glTexCoord2f(0.0, maxCoordsTO2.t);
	glVertex3f(1.0, 1.0, 0.0);

	//upper right
	glTexCoord2f(maxCoordsTO2.s, maxCoordsTO2.t);
	glVertex3f(3.0, 1.0, 0.0);

	//lower right
	glTexCoord2f(maxCoordsTO2.s, 0);
	glVertex3f(3.0, -1.0, 0.0);

	glEnd();




	glDisable(GL_TEXTURE_2D);

	return FF_SUCCESS;
}

DWORD FFGLGeometryMixer::TriangleMeshMix(ProcessOpenGLStruct* pGL)
{

	float  triangleDefs [3][2] = {{0.25,0.5}, {0.5, 1.0}, {0.75, 0.5}};

	FFGLTextureStruct &TextureObject1 = *(pGL->inputTextures[0]);
	FFGLTextureStruct &TextureObject2 = *(pGL->inputTextures[1]);


	FFGLTexCoords maxCoordsTO1 = GetMaxGLTexCoords(TextureObject1);
	FFGLTexCoords maxCoordsTO2 = GetMaxGLTexCoords(TextureObject2);

	glEnable(GL_TEXTURE_2D);

	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glHint(GL_POINT_SMOOTH, GL_NICEST);
	glHint(GL_LINE_SMOOTH, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH, GL_NICEST);

	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);






	glBindTexture(GL_TEXTURE_2D, TextureObject2.Handle);

	glBegin(GL_TRIANGLES);

	// Central top triangle	
	glTexCoord2f(0.25,0.5);
	glVertex3f(-0.5, 0.0, 0.0);		

	glTexCoord2f(0.5, 1.0);
	glVertex3f(0.0, 1.0, 0.0);

	glTexCoord2f(0.75, 0.5);		
	glVertex3f( 0.5, 0.0, 0.0);

	// Left bottom triangle
	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, -1.0, 0.0);		

	glTexCoord2f(0.25, 0.5);
	glVertex3f(-0.5, 0.0, 0.0);

	glTexCoord2f(0.5, 0.0);		
	glVertex3f( 0.0, -1.0, 0.0);


	// Right bottom triangle
	glTexCoord2f(0.5, 0.0);
	glVertex3f(0.0, -1.0, 0.0);		

	glTexCoord2f(0.75, 0.5);
	glVertex3f(0.5, 0.0, 0.0);

	glTexCoord2f(1.0, 0.0);		
	glVertex3f( 1.0, -1.0, 0.0);

	// Left top semi-triangle
	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0, 1.0, 0.0);		

	glTexCoord2f(0.25, 0.5);
	glVertex3f(-0.5, 0.0, 0.0);

	glTexCoord2f(0.0, 0.5);		
	glVertex3f( -1.0, 0.0, 0.0);

	// Right top semi-triangle
	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0, 1.0, 0.0);		

	glTexCoord2f(1.0, 0.5);
	glVertex3f(1.0, 0.0, 0.0);

	glTexCoord2f(0.75, 0.5);		
	glVertex3f( 0.5, 0.0, 0.0);

	glEnd();


	glBindTexture(GL_TEXTURE_2D, TextureObject1.Handle);

	glBegin(GL_TRIANGLES);

	glTexCoord2f(0.5, 0.0);
	glVertex3f(0.0, -1.0, 0.0);

	glTexCoord2f(0.25, 0.5);
	glVertex3f(-0.5, 0.0, 0.0);

	glTexCoord2f(0.75, 0.5);
	glVertex3f(0.5, 0.0, 0.0);


	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0, 1.0, 0.0);

	glTexCoord2f(0.5, 1.0);
	glVertex3f(0.0, 1.0, 0.0);

	glTexCoord2f(0.25, 0.5);
	glVertex3f(-0.5, 0.0, 0.0);


	glTexCoord2f(0.5, 1.0);
	glVertex3f(0.0, 1.0, 0.0);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0, 1.0, 0.0);

	glTexCoord2f(0.75, 0.5);
	glVertex3f(0.5, 0.0, 0.0);


	// Left bottom semi-triangle
	glTexCoord2f(0.0, 0.5);
	glVertex3f(-1.0, 0.0, 0.0);		

	glTexCoord2f(0.25, 0.5);
	glVertex3f(-0.5, 0.0, 0.0);

	glTexCoord2f(0.0, 0.0);		
	glVertex3f( -1.0, -1.0, 0.0);

	// Right bottom semi-triangle
	glTexCoord2f(0.75, 0.5);
	glVertex3f(0.5, 0.0, 0.0);		

	glTexCoord2f(1.0, 0.5);
	glVertex3f(1.0, 0.0, 0.0);

	glTexCoord2f(1.0, 0.0);		
	glVertex3f( 1.0, -1.0, 0.0);


	glEnd();

	glDisable(GL_TEXTURE_2D);

	return FF_SUCCESS;
}



DWORD	FFGLGeometryMixer::InitGL(const FFGLViewportStruct *vp)
{
	return FF_SUCCESS;
}

DWORD	FFGLGeometryMixer::DeInitGL()
{
	return FF_SUCCESS;
}

