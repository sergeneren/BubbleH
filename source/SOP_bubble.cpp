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
*	In this file houdini geometry is sent to MeshIO for being processed as a LosTopos surface   
*
*/


//Local

#include "SOP_bubble.h"
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
		"SOP_BUBBLE",
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
	

	// Parse options
	Options::addStringOption("scene", "T1");
	Options::addStringOption("load-dir", "");
	Options::addDoubleOption("timestep", DT(t));
	Options::addDoubleOption("simulation-time", 1.0);
	Options::addBooleanOption("implicit-integration", IMP(t));
	Options::addBooleanOption("pbd-implicit", PBD(t));
	Options::addBooleanOption("RK4-velocity-integration", RK4(t));
	Options::addDoubleOption("smoothing-coef", SC(t));
	Options::addDoubleOption("damping-coef", DC(t));
	Options::addDoubleOption("sigma", SIGMA(t));
	Options::addDoubleOption("gravity", 0.0);
	Options::addBooleanOption("fmmtl", false);
	Options::addBooleanOption("looped", true);
	Options::addDoubleOption("radius", RAD(t));
	Options::addDoubleOption("density", 1.32e3);
	Options::addDoubleOption("stretching", STRECH(t));
	Options::addDoubleOption("bending", BEND(t));


	Options::addDoubleOption("remeshing-resolution", REMESH_RES(t));
	Options::addIntegerOption("remeshing-iterations", REMESH_ITE(t));


	Options::addDoubleOption("lostopos-collision-epsilon-fraction", COLL_EPS(t));			// lostopos collision epsilon (fraction of mean edge length)
	Options::addDoubleOption("lostopos-merge-proximity-epsilon-fraction", MERGE_EPS(t));	// lostopos merge proximity epsilon (fraction of mean edge length)
	Options::addBooleanOption("lostopos-perform-smoothing", SMOOTH(t));						// whether or not to perform smoothing
	Options::addDoubleOption("lostopos-max-volume-change-fraction", VC_F(t));				// maximum allowed volume change during a remeshing operation (fraction of mean edge length cubed)
	Options::addDoubleOption("lostopos-min-triangle-angle", MIN_TRI_ANG(t));                // min triangle angle (in degrees)
	Options::addDoubleOption("lostopos-max-triangle-angle", MAX_TRI_ANG(t));				// max triangle angle (in degrees)
	Options::addDoubleOption("lostopos-large-triangle-angle-to-split", LAR_TRI_ANG(t));		// threshold for large angles to be split
	Options::addDoubleOption("lostopos-min-triangle-area-fraction", MIN_TRI_AREA(t));       // minimum allowed triangle area (fraction of mean edge length squared)
	Options::addBooleanOption("lostopos-t1-transition-enabled", T1_TRANS(t));				// whether t1 is enabled
	Options::addDoubleOption("lostopos-t1-pull-apart-distance-fraction", T1_PULL(t));		// t1 pull apart distance (fraction of mean edge legnth)
	Options::addBooleanOption("lostopos-smooth-subdivision", LT_SM_SBD(t));					// whether to use smooth subdivision during remeshing
	Options::addBooleanOption("lostopos-allow-non-manifold", true);							// whether to allow non-manifold geometry in the mesh
	Options::addBooleanOption("lostopos-allow-topology-changes", true);						// whether to allow topology changes

	Options::addIntegerOption("mesh-size-n", 2);
	Options::addIntegerOption("mesh-size-m", 2);



	// Create surface tracker


	std::vector<LosTopos::Vec3d> vertices; 
	std::vector<LosTopos::Vec3st> faces;
	std::vector<LosTopos::Vec2i> face_labels;
	std::vector<size_t> constrained_vertices;
	std::vector<Vec3d> constrained_positions;


	GA_Range pt_range = gdp->getPointRange();
	if (pt_range.empty()) return error(); 

	GA_RWHandleV3 pt_pos(gdp->getP());
	GA_ROHandleI cons_pt(gdp, GA_ATTRIB_POINT, "const"); 

	for (GA_Iterator it(pt_range.begin()); !it.atEnd(); ++it) {
	
		UT_Vector3F pos = pt_pos.get(it.getOffset());
		vertices.push_back(LosTopos::Vec3d(pos[0], pos[1], pos[2]));

		if (cons_pt.isValid()) {
			bool constrained = cons_pt.get(it.getOffset());

			if (constrained) {
				constrained_vertices.push_back(*it);
				constrained_positions.push_back(Vec3d(pos[0], pos[1], pos[2]));
			}
		}
	}
	
	GA_OffsetArray neighbour_prims;
	GA_Offset prim_offset;
	GA_Primitive *prim;
	GA_Offset goff;
	GA_RWHandleIA face_label(gdp, GA_ATTRIB_PRIMITIVE, "label"); 
	
	for (GA_Iterator prim_it(gdp->getPrimitiveRange()); !prim_it.atEnd(); ++prim_it) {
		gdp->getEdgeAdjacentPolygons(neighbour_prims, prim_it.getOffset());
		prim = gdp->getPrimitive(prim_it.getOffset());
		pt_range = prim->getPointRange();

		std::vector<GA_Offset> pt_offsets;

		for (GA_Iterator it(pt_range.begin()); !it.atEnd(); ++it) {
			pt_offsets.push_back(*it);
		}
		 
		faces.push_back(LosTopos::Vec3st(pt_offsets[2], pt_offsets[1], pt_offsets[0]));
		
		if (face_label.isValid()) { 
			UT_Int32Array labels;
			face_label.get(prim_it.getOffset(), labels);			
			face_labels.push_back(LosTopos::Vec2i(labels[0], labels[1]));
		}

		else face_labels.push_back(LosTopos::Vec2i(1, 0));
	}

	VS3D *m_vs = new VS3D(vertices, faces, face_labels, constrained_vertices, constrained_positions);

	m_vs->step(DT(t));


	LosTopos::SurfTrack &st =  *(m_vs->surfTrack());
	vertices = st.get_positions();

	

	pt_range = gdp->getPointRange();
	for (GA_Iterator it(pt_range.begin()); !it.atEnd(); ++it) {
		LosTopos::Vec3d new_pos = vertices[*it];
		pt_pos.set(it.getOffset(), UT_Vector3F(new_pos[0], new_pos[1], new_pos[2]));
	}

	pt_pos.bumpDataId();





	delete m_vs;

	return error();
}


