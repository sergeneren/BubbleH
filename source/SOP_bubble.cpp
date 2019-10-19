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
*	 Source repository: https://github.com/sergeneren/BubbleH
*/


/*! \file
*
*   SOP level entry point for multitracker implementation file
*	In this file houdini geometry is converted to a LosTopos surface, integrated    
*	and the positions are copied back. 
*
*	In every time step we delete and create the houdini geometry from scratch. This  
*	will be an adaptive system in the future.                                                                   
*/


//Local

#include "SOP_bubble.h"
#include "MeshIO.h"
#include "VS3D.h"
#include "SimOptions.h"

//Houdini
#include <GU/GU_Detail.h>
#include <OP/OP_Operator.h>
#include <OP/OP_OperatorTable.h>
#include <OP/OP_Director.h>
#include <OP/OP_AutoLockInputs.h>
#include <PRM/PRM_Include.h>
#include <UT/UT_DSOVersion.h>
#include <UT/UT_Interrupt.h>
#include <SOP/SOP_Node.h>

//C++

using namespace BUBBLE;



void newSopOperator(OP_OperatorTable *table) {

	table->addOperator(new OP_Operator (
		"sop_bubble",
		"Soap Film",
		SOP_bubble::myConstructor,
		SOP_bubble::myTemplateList,
		1, // min number of inputs 
		1, // max number of inputs
		0)); 
}


// Parameter defaults
static PRM_Name param_names[] = {

	PRM_Name("dt"			, "time step"),
	PRM_Name("implicit"		, "Implicit Integration"),
	PRM_Name("pbd_implicit"	, "PBD Integration"),
	PRM_Name("rk4"			, "RK4 Integration"),
	PRM_Name("sc"			, "Smoothing Coefficient"),
	PRM_Name("dc"			, "Damping Coefficient"),
	PRM_Name("sigma"		, "Sigma"),
	PRM_Name("bend"			, "Bending"),
	PRM_Name("strech"		, "Streching"),
	PRM_Name("g"			, "Gravity"),
	PRM_Name("radius"		, "Radius"),
	PRM_Name("remesh_res"	, "Remeshing Resolution"),
	PRM_Name("remesh_iter"	, "Remeshing Iterations"),
	PRM_Name("coll_eps"		, "Collision Epsilon Fraction"),
	PRM_Name("merge_eps"	, "Merge Epsilon Fraction"),
	PRM_Name("lt_smooth"	, "Perform Smoothing"),
	PRM_Name("vc_f"			, "Volume Change Fraction"),
	PRM_Name("min_tri_ang"	, "Min Triangle Angle"),
	PRM_Name("max_tri_ang"	, "Max Triangle Angle"),
	PRM_Name("large_tri"	, "Large Triangle Angle"),
	PRM_Name("min_tri_area"	, "Min Triangle Area"),
	PRM_Name("t1_trans"		, "T1 Transition"),
	PRM_Name("t1_pull"		, "T1 Pull Apart Distance Fraction"),
	PRM_Name("lt_sm_subd"	, "Smooth Subdivision"),
};

static PRM_Name         switcherName("shakeswitcher");

static PRM_Default      switcher[] = {
	PRM_Default(11, "Simulation"),   
	PRM_Default(2, "Remeshing"),
	PRM_Default(11, "LT Surface"),
};


PRM_Template SOP_bubble::myTemplateList[] = {

	PRM_Template(PRM_SWITCHER,  sizeof(switcher) / sizeof(PRM_Default), &switcherName, switcher),

	PRM_Template(PRM_FLT, 1 , &param_names[0], PRMpointOneDefaults),		// dt
	PRM_Template(PRM_TOGGLE, 1 , &param_names[1], PRMzeroDefaults),			// implicit
	PRM_Template(PRM_TOGGLE, 1 , &param_names[2], PRMzeroDefaults),			// pbd
	PRM_Template(PRM_TOGGLE, 1 , &param_names[3], PRMzeroDefaults),			// rk4
	PRM_Template(PRM_FLT, 1 , &param_names[4], PRMoneDefaults),				// sc
	PRM_Template(PRM_FLT, 1 , &param_names[5], PRMoneDefaults),				// dc
	PRM_Template(PRM_FLT, 1 , &param_names[6], PRMoneDefaults),				// sigma
	PRM_Template(PRM_FLT, 1 , &param_names[7], PRM100Defaults),				// bend
	PRM_Template(PRM_FLT, 1 , &param_names[8], PRM100Defaults),				// strech
	PRM_Template(PRM_FLT, 3 , &param_names[9], PRMzeroDefaults),			// g
	PRM_Template(PRM_FLT, 1 , &param_names[10], PRMpointOneDefaults),		// radius
	PRM_Template(PRM_FLT, 1 , &param_names[11], PRMpointOneDefaults),		// remesh res
	PRM_Template(PRM_INT, 1 , &param_names[12], PRMtwoDefaults),			// remesh iter
	PRM_Template(PRM_FLT, 1 , &param_names[13], PRMpointOneDefaults),		// Collision Epsilon Fraction
	PRM_Template(PRM_FLT, 1 , &param_names[14], PRMpointOneDefaults),		// Merge Epsilon Fraction
	PRM_Template(PRM_TOGGLE, 1 , &param_names[15], PRMzeroDefaults),		// Perform Smoothing
	PRM_Template(PRM_FLT, 1 , &param_names[16], PRMpointOneDefaults),		// Volume Change Fraction
	PRM_Template(PRM_FLT, 1 , &param_names[17], PRMthreeDefaults),			// Min Triangle Angle
	PRM_Template(PRM_FLT, 1 , &param_names[18], PRM180Defaults),			// Max Triangle Angle
	PRM_Template(PRM_FLT, 1 , &param_names[19], PRM180Defaults),			// Large Triangle Angle
	PRM_Template(PRM_FLT, 1 , &param_names[20], PRMpointOneDefaults),		// Min Triangle Area
	PRM_Template(PRM_TOGGLE, 1 , &param_names[21], PRMoneDefaults),			// T1 Transition
	PRM_Template(PRM_FLT, 1 , &param_names[22], PRMpointOneDefaults),		// T1 Pull Apart Distance Fraction
	PRM_Template(PRM_TOGGLE, 1 , &param_names[23], PRMzeroDefaults),		// Smooth Subdivision
	PRM_Template()
};



OP_Node * SOP_bubble::myConstructor(OP_Network *net, const char *name, OP_Operator *op)
{
	return new SOP_bubble(net, name, op);
}

SOP_bubble::SOP_bubble(OP_Network *net, const char *name, OP_Operator *op) : SOP_Node(net, name, op)
{
	flags().setTimeDep(1);
	mySopFlags.setManagesDataIDs(true); 

}

SOP_bubble::~SOP_bubble() {}

OP_ERROR SOP_bubble::cookMySop(OP_Context & context)
{
	
	OP_AutoLockInputs inputs(this); 
	if (inputs.lock(context) >= UT_ERROR_ABORT) return error(); 
	duplicateSource(0, context); 
	addMessage(SOP_MESSAGE, "Soap Bubble Simulator"); 
	
	fpreal t = context.getTime();
	flags().setTimeDep(1);
	MeshIO meshio; 

	fpreal dt = DT(t); 
	size_t imp = IMP(t);
	size_t pbd = PBD(t); 
	size_t rk = RK4(t); 
	fpreal sc = SC(t); 
	fpreal dc = DC(t); 
	fpreal sigma = SIGMA(t); 
	fpreal rad = RAD(t); 
	fpreal strech = STRECH(t); 
	fpreal bend = BEND(t); 
	UT_Vector3F grav = G(t); 
	
	fpreal rem_res = REMESH_RES(t); 
	size_t rem_iter = REMESH_ITE(t);


	fpreal coll_eps = COLL_EPS(t); 
	fpreal merge_eps = MERGE_EPS(t);
	size_t smooth = SMOOTH(t);
	fpreal vc = VC_F(t);
	fpreal min_tri_ang = MIN_TRI_ANG(t);
	fpreal max_tri_ang = MAX_TRI_ANG(t);
	fpreal lar_tri_ang = LAR_TRI_ANG(t);
	fpreal min_tri_area = MIN_TRI_AREA(t);
	size_t t1_trans = T1_TRANS(t);
	fpreal t1_pull = T1_PULL(t);
	size_t lt_sm_sbd = LT_SM_SBD(t);
	fpreal frame = context.getFloatFrame();

	// Parse options

	Options sim_options;
	
	sim_options.addStringOption("scene", "T1");
	sim_options.addStringOption("load-dir", "");
	sim_options.addDoubleOption("timestep",dt);
	sim_options.addDoubleOption("simulation-time", 1.0);
	sim_options.addBooleanOption("implicit-integration", imp);
	sim_options.addBooleanOption("pbd-implicit", pbd);
	sim_options.addBooleanOption("RK4-velocity-integration", rk);
	sim_options.addDoubleOption("smoothing-coef",sc);
	sim_options.addDoubleOption("damping-coef", dc);
	sim_options.addDoubleOption("sigma", sigma);
	sim_options.addVectorOption("gravity", Vector3s(grav[0], grav[1], grav[2]));
	sim_options.addBooleanOption("fmmtl", false);
	sim_options.addBooleanOption("looped", true);
	sim_options.addDoubleOption("radius",rad);
	sim_options.addDoubleOption("density", 1.32e3);
	sim_options.addDoubleOption("stretching", strech);
	sim_options.addDoubleOption("bending", bend);
	
	sim_options.addDoubleOption("frame", frame);

	sim_options.addDoubleOption("remeshing-resolution", rem_res);
	sim_options.addIntegerOption("remeshing-iterations", rem_iter);


	sim_options.addDoubleOption("lostopos-collision-epsilon-fraction", coll_eps);				// lostopos collision epsilon (fraction of mean edge length)
	sim_options.addDoubleOption("lostopos-merge-proximity-epsilon-fraction", merge_eps);		// lostopos merge proximity epsilon (fraction of mean edge length)
	sim_options.addBooleanOption("lostopos-perform-smoothing", smooth);							// whether or not to perform smoothing
	sim_options.addDoubleOption("lostopos-max-volume-change-fraction", vc);						// maximum allowed volume change during a remeshing operation (fraction of mean edge length cubed)
	sim_options.addDoubleOption("lostopos-min-triangle-angle", min_tri_ang);					// min triangle angle (in degrees)
	sim_options.addDoubleOption("lostopos-max-triangle-angle", max_tri_ang);					// max triangle angle (in degrees)
	sim_options.addDoubleOption("lostopos-large-triangle-angle-to-split", lar_tri_ang);			// threshold for large angles to be split
	sim_options.addDoubleOption("lostopos-min-triangle-area-fraction", min_tri_area);			// minimum allowed triangle area (fraction of mean edge length squared)
	sim_options.addBooleanOption("lostopos-t1-transition-enabled", t1_trans);					// whether t1 is enabled
	sim_options.addDoubleOption("lostopos-t1-pull-apart-distance-fraction", t1_pull);			// t1 pull apart distance (fraction of mean edge legnth)
	sim_options.addBooleanOption("lostopos-smooth-subdivision", lt_sm_sbd);						// whether to use smooth subdivision during remeshing
	sim_options.addBooleanOption("lostopos-allow-non-manifold", true);							// whether to allow non-manifold geometry in the mesh
	sim_options.addBooleanOption("lostopos-allow-topology-changes", true);						// whether to allow topology changes


	// Create surface tracker

	VS3D *m_vs = meshio.build_tracker(gdp, sim_options); 
	if (!m_vs) {
		UT_WorkBuffer buf;
		buf.sprintf("Unable to create surface tracker!");
		addError(SOP_MESSAGE, buf.buffer());
		return error();
	}

	
	// Integrate positions
	m_vs->step(dt);
	
	
	// Convert surface tracker mesh back to houdini geo
	bool success = meshio.convert_to_houdini_geo(gdp, m_vs); 

	if (!success) {
		UT_WorkBuffer buf;
		buf.sprintf("Unable to convert surface tracker mesh back to houdini geometry!");
		addError(SOP_MESSAGE, buf.buffer());
		return error();
	}

	delete m_vs;
	return error();
}


