/*! \file MeshIO.h
*
*   SOP level entry point for multitracker implementation file
*	In this file houdini geometry is converted to a LosTopos surface, integrated
*	and the positions are copied back.
*
*	In every time step we delete and create the houdini geometry from scratch. This
*	will be an adaptive system in the future.
*/



#include "MeshIO.h"
#include "SimOptions.h"
#include <GU/GU_Detail.h>



VS3D * MeshIO::build_tracker(const GU_Detail *gdp, Options sim_options) {


	std::vector<LosTopos::Vec3d> vertices;
	std::vector<LosTopos::Vec3st> faces;
	std::vector<LosTopos::Vec2i> face_labels;
	std::vector<size_t> constrained_vertices;
	std::vector<Vec3d> constrained_positions;
	std::vector<Vec3d> constrained_velocities;


	GA_Range pt_range = gdp->getPointRange();
	if (pt_range.empty()) return NULL;

	GA_ROHandleV3 pt_pos(gdp->getP());
	GA_ROHandleI cons_pt(gdp, GA_ATTRIB_POINT, "constrained");
	GA_ROHandleV3 vel_h(gdp, GA_ATTRIB_POINT, "v");

	for (GA_Iterator it(pt_range.begin()); !it.atEnd(); ++it) {

		UT_Vector3F pos = pt_pos.get(it.getOffset());
		vertices.push_back(LosTopos::Vec3d(pos[0], pos[1], pos[2]));

		if (cons_pt.isValid()) {
			bool constrained = cons_pt.get(it.getOffset());

			if (constrained) {
				constrained_vertices.push_back(*it);
				constrained_positions.push_back(Vec3d(pos[0], pos[1], pos[2]));
				if (vel_h.isValid()) {
					UT_Vector3F vel = vel_h.get(it.getOffset());
					constrained_velocities.push_back(Vec3d(vel[0], vel[1], vel[2]));
				}

			}
		}
	}
	

	GA_OffsetArray neighbour_prims;
	GA_Offset prim_offset;
	const GA_Primitive *prim;
	GA_ROHandleIA face_label(gdp, GA_ATTRIB_PRIMITIVE, "label");

	for (GA_Iterator prim_it(gdp->getPrimitiveRange()); !prim_it.atEnd(); ++prim_it) {
		gdp->getEdgeAdjacentPolygons(neighbour_prims, prim_it.getOffset());
		prim = gdp->getPrimitive(prim_it.getOffset());
		GA_Range prim_pt_range = prim->getPointRange();

		std::vector<GA_Offset> pt_offsets;

		for (GA_Iterator it(prim_pt_range.begin()); !it.atEnd(); ++it) {
			pt_offsets.push_back(*it);
		}

		faces.push_back(LosTopos::Vec3st(pt_offsets[2], pt_offsets[1], pt_offsets[0]));

		if (face_label.isValid()) {
			UT_IntArray labels;
			face_label.get(prim_it.getOffset(), labels);
			face_labels.push_back(LosTopos::Vec2i(labels[0], labels[1]));
		}

		else face_labels.push_back(LosTopos::Vec2i(1, 0));
	}

	VS3D *m_vs = new VS3D(vertices, faces, face_labels, sim_options, constrained_vertices, constrained_positions, constrained_velocities);
	

	// Check if gamma values exist. If there is one, it means this is not the first frame 
	const GA_Attribute *gamma_attrib = gdp->findFloatArray(GA_ATTRIB_POINT, "Gamma", -1, -1);
	if(gamma_attrib){ 
		
		const GA_AIFNumericArray *gamma_aif = gamma_attrib->getAIFNumericArray();
		if (!gamma_aif)
		{
			//There is a gamma attribute but it's not a float array  
			return NULL;
		}



		for (GA_Iterator it(pt_range.begin()); !it.atEnd(); ++it) {

			UT_Array<fpreal64> data; 
			gamma_aif->get(gamma_attrib, it.getOffset(), data);

			size_t n = m_vs->nregion();
			for (int j = 0; j < m_vs->Gamma(*it).values.rows(); j++) {
				for (int k = 0; k < m_vs->Gamma(*it).values.cols(); k++) {
					fpreal64 val = data[(j*n)+k];
					m_vs->Gamma(*it).set(j, k, val);

				}
			}

		}
	
	}


	return m_vs;
}


bool MeshIO::convert_to_houdini_geo(GU_Detail *gdp, VS3D *tracker) {

	std::vector<LosTopos::Vec3d> vertices;

	const LosTopos::SurfTrack &st = *(tracker->surfTrack());
	vertices = st.get_newpositions();
	

	gdp->clearAndDestroy();

	GA_Offset start_ptoff = gdp->appendPointBlock(tracker->mesh().nv());


	// We have deleted every attribute alongside primitives and points. Now we have to create them back 

	// Create triangle label attribute back  
	GA_Attribute *temp_face_labels = gdp->findIntArray(GA_ATTRIB_PRIMITIVE, "label", -1, -1);
	if (!temp_face_labels) temp_face_labels = gdp->addIntArray(GA_ATTRIB_PRIMITIVE, "label", 1);

	if (!temp_face_labels)
	{
		//Failed to create face label attribute
		return false;
	}

	const GA_AIFNumericArray *aif = temp_face_labels->getAIFNumericArray();
	if (!aif)
	{
		//Attribute is not a numeric array
		return false;
	}


	//Create Gamma float array attribute 
	GA_Attribute *gamma_attrib = gdp->findFloatArray(GA_ATTRIB_POINT, "Gamma", -1, -1); 
	if (!gamma_attrib) gamma_attrib = gdp->addFloatArray(GA_ATTRIB_POINT, "Gamma", 1); 

	if (!gamma_attrib)
	{
		//Failed to create gamma attribute
		return false;
	}

	const GA_AIFNumericArray *gamma_aif = gamma_attrib->getAIFNumericArray();
	if (!gamma_aif)
	{
		//Attribute is not a numeric array
		return false;
	}
	
	// Add vertex normal attribute if it doesn't exist
	GA_RWHandleV3 vertex_normals_h(gdp->addFloatTuple(GA_ATTRIB_VERTEX, "N", 3)); 



	// for each triangle add a primitive
	for (size_t pr = 0; pr < tracker->mesh().nt(); ++pr) {

		GA_Offset start_vtxoff;
		GA_Offset prim_off = gdp->appendPrimitivesAndVertices(GA_PRIMPOLY, 1, 3, start_vtxoff, true);

		LosTopos::Vec3st indices = tracker->mesh().get_triangle(pr);

		// Calculate area-weighted normals

		LosTopos::Vec3st t = tracker->surfTrack()->m_mesh.get_triangle(pr); 
		Vec3d x0 = tracker->pos(t[0]);
		Vec3d x1 = tracker->pos(t[1]);
		Vec3d x2 = tracker->pos(t[2]);

		Vec3d nt = (x1 - x0).cross(x2 - x0); 
		nt.normalize();

		if (tracker->surfTrack()->m_mesh.get_triangle_label(pr)[0] < tracker->surfTrack()->m_mesh.get_triangle_label(pr)[1]) nt = -nt;

		// Connect vertices
		for (size_t i = 0; i < 3; ++i) {

			gdp->getTopology().wireVertexPoint(start_vtxoff + i, start_ptoff + indices[2 - i]);
			vertex_normals_h.set(start_vtxoff + i, UT_Vector3F(nt[0], nt[1], nt[2]));
		}

		// Write back triangle labels
		LosTopos::Vec2i triangle_label = tracker->mesh().get_triangle_label(pr);
		UT_IntArray data;
		data.append(triangle_label[0]);
		data.append(triangle_label[1]);
		aif->set(temp_face_labels, prim_off, data);

	}
	temp_face_labels->bumpDataId();

	// Set the point positions and simulation attributes 

	GA_RWHandleH const_h(gdp->addIntTuple(GA_ATTRIB_POINT, "constrained", 1));
	GA_RWHandleV3 mass_h(gdp->addFloatTuple(GA_ATTRIB_POINT, "mass", 3));
	GA_RWHandleV3 vel_h(gdp->addFloatTuple(GA_ATTRIB_POINT, "v", 3));

	
	tracker->update_dbg_quantities();

	for (size_t i = 0; i < tracker->mesh().nv(); ++i) {

		GA_Offset ptoff = start_ptoff + i;

		LosTopos::Vec3d new_pos = vertices[i];
		gdp->setPos3(ptoff, UT_Vector3F(new_pos[0], new_pos[1], new_pos[2]));
		
		mass_h.set(ptoff, UT_Vector3F(st.m_masses[i][0], st.m_masses[i][1], st.m_masses[i][2]));		
		vel_h.set(ptoff, UT_Vector3F(tracker->get_velocity(i)[0], tracker->get_velocity(i)[1], tracker->get_velocity(i)[2]));
		if (st.vertex_is_all_solid(i)) const_h.set(ptoff, 1);

		
		//Write out gamma values
		UT_Array<fpreal64> data;
		for (int j = 0; j < tracker->Gamma(i).values.rows(); j++) {
			for (int k = 0; k < tracker->Gamma(i).values.cols(); k++) {
				
				data.append(tracker->Gamma(i).get(j, k));

			}
		}
		gamma_aif->set(gamma_attrib, ptoff,data);

	}
	gamma_attrib->bumpDataId();
	vel_h.bumpDataId();
	mass_h.bumpDataId();


	gdp->bumpDataIdsForAddOrRemove(true, true, true);

	return true;
}


MeshIO::~MeshIO() {}