#pragma once

#include <GU/GU_Detail.h>
#include "VS3D.h"


class MeshIO {

public:
	virtual ~MeshIO();

	virtual VS3D* build_tracker(const GU_Detail *gdp, Options sim_options);
	virtual bool convert_to_houdini_geo(GU_Detail *gdp, VS3D *tracker);

};