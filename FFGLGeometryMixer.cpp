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
	"FFGL Triangle MX",				// Plugin description
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

