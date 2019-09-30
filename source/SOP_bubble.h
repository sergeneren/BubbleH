/*
*    Copyright 2019, Sergen Eren <sergeneren@gmail.com>.
*
*    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* 
*    1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
*
*    2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer 
*       in the documentation and/or other materials provided with the distribution.
*
*    3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived 
*       from this software without specific prior written permission.
* 
*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
*    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
*    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
*    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
*    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
*    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* Source repository: https://github.com/sergeneren/BubbleH
*/


/*! \file 
* 
*   SOP level entry point for multitracker header file 
*	this is the header for SOP_bubble node 
*
*/

#pragma once


#ifndef __SOP_bubble_h__
#define __SOP_bubble_h__

#include <SOP/SOP_Node.h>
#include <GU/GU_Detail.h>
#include <GA/GA_PageHandle.h>

namespace BUBBLE {

	class SOP_bubble : public SOP_Node
	{
	public:

		//Constructors and destructor

		static OP_Node *myConstructor(OP_Network *, const char *, OP_Operator *); 

		static PRM_Template myTemplateList[];
		

	protected: 

		SOP_bubble(OP_Network *net, const char *name, OP_Operator *op);
		virtual ~SOP_bubble(); 

		virtual OP_ERROR	cookMySop(OP_Context &context); 
		
		virtual const char *inputLabel(unsigned idx) const override 
		{
			switch (idx)
			{
			case 0:     return "Rendering Grid";
			case 1:     return "Camera";
			case 2:     return "Primitives";
			case 3:     return "Volumes";
			case 4:     return "Lights";
			default:    return "Invalid Source";
			}
		}

	private: // Parameter accessors

		// Vector evaluaters
		UT_Vector3 evalVector3(const char *str, fpreal t) {
		
			return UT_Vector3(evalFloat(str, 0, t), 
							  evalFloat(str, 1, t), 
							  evalFloat(str, 2, t));
		}

		UT_Vector2 evalVector2(const char *str, fpreal t) {

			return UT_Vector2(evalFloat(str, 0, t), 
							  evalFloat(str, 1, t));
		}
		// End vector evaluaters

		UT_Vector2 RES(fpreal t)		{ return evalVector2("res", t); }
		size_t	   SPP(fpreal t)		{ return evalInt("spp", 0, t); }
		size_t	   DEPTH(fpreal t)		{ return evalInt("depth", 0, t); }
		size_t	   VS(fpreal t)			{ return evalInt("vs", 0, t); }
		fpreal	   SIZEX(fpreal t)		{ return evalFloat("sizex", 0, t); }
		UT_Vector3 COLOR(fpreal t)		{ return evalVector3("color", t); }
		UT_Vector3 ATTN(fpreal t)		{ return evalVector3("attenuation", t); }
		fpreal	   DENSITY(fpreal t)	{ return evalFloat("density", 0, t); }
		fpreal	   SDM(fpreal t)		{ return evalFloat("sdm", 0, t); }
		fpreal	   MAXDIST(fpreal t)	{ return evalFloat("max_dist", 0, t); }
		fpreal	   STEPRATE(fpreal t)	{ return evalFloat("steprate", 0, t); }


	};
} // End namespace

#endif // !__BUBBLE__



