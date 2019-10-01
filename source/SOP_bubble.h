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
			return "Input Mesh";
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
		
		fpreal	   DT(fpreal t)				{ return evalFloat("dt", 0, t); }
		size_t	   IMP(fpreal t)			{ return evalInt("implicit", 0, t); }
		size_t	   PBD(fpreal t)			{ return evalInt("pbd_implicit", 0, t); }
		size_t	   RK4(fpreal t)			{ return evalInt("rk4", 0, t); }
		fpreal	   SC(fpreal t)				{ return evalFloat("sc", 0, t); }
		fpreal	   DC(fpreal t)				{ return evalFloat("dc", 0, t); }
		fpreal	   SIGMA(fpreal t)			{ return evalFloat("sigma", 0, t); }
		fpreal	   BEND(fpreal t)			{ return evalFloat("bend", 0, t); }
		fpreal	   STRECH(fpreal t)			{ return evalFloat("strech", 0, t); }
		UT_Vector3 G(fpreal t)				{ return evalVector3("g", t); }
		fpreal	   RAD(fpreal t)			{ return evalFloat("radius", 0, t); }
		fpreal	   REMESH_RES(fpreal t)		{ return evalFloat("remesh_res", 0, t); }
		size_t	   REMESH_ITE(fpreal t)		{ return evalInt("remesh_iter", 0, t); }
		fpreal	   COLL_EPS(fpreal t)		{ return evalFloat("coll_eps", 0, t); }
		fpreal	   MERGE_EPS(fpreal t)		{ return evalFloat("merge_eps", 0, t); }
		size_t	   SMOOTH(fpreal t)			{ return evalInt("lt_smooth", 0, t); }
		fpreal	   VC_F(fpreal t)			{ return evalFloat("vc_f", 0, t); }
		fpreal	   MIN_TRI_ANG(fpreal t)	{ return evalFloat("min_tri_ang", 0, t); }
		fpreal	   MAX_TRI_ANG(fpreal t)	{ return evalFloat("max_tri_ang", 0, t); }
		fpreal	   LAR_TRI_ANG(fpreal t)	{ return evalFloat("large_tri", 0, t); }
		fpreal	   MIN_TRI_AREA(fpreal t)	{ return evalFloat("min_tri_area", 0, t); }
		size_t	   T1_TRANS(fpreal t)		{ return evalInt("t1_trans", 0, t); }
		fpreal	   T1_PULL(fpreal t)		{ return evalFloat("t1_pull", 0, t); }
		size_t	   LT_SM_SBD(fpreal t)		{ return evalInt("lt_sm_subd", 0, t); }


	};
} // End namespace

#endif // !__BUBBLE__