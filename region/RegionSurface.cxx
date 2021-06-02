#include "RegionSurface.h"

#include <vtkActor.h>
#include <vtkDiscreteFlyingEdges3D.h>
#include <vtkLookupTable.h>
#include <vtkPlane.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkWindowedSincPolyDataFilter.h>

#include "Region.h"

RegionSurface::RegionSurface(Region* inputRegion, double color[3]) {
	smoothSurface = false;
	smoothShading = true;

	region = inputRegion;

	contour = vtkSmartPointer<vtkDiscreteFlyingEdges3D>::New();
	contour->SetValue(0, region->GetLabel());
	contour->ComputeNormalsOff();
	contour->ComputeGradientsOff();
	contour->SetInputConnection(region->GetOutput());

	// Smoother
	int smoothingIterations = 8;
	double passBand = 0.01;
	double featureAngle = 120.0;

	smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
	smoother->SetNumberOfIterations(smoothingIterations);
	//smoother->BoundarySmoothingOff();
	//smoother->FeatureEdgeSmoothingOff();
	//smoother->SetFeatureAngle(featureAngle);
	smoother->SetPassBand(passBand);
	//smoother->NonManifoldSmoothingOn();
	smoother->NormalizeCoordinatesOn();
	smoother->SetInputConnection(contour->GetOutputPort());

	normals = vtkSmartPointer<vtkPolyDataNormals>::New();
	normals->ComputePointNormalsOn();
	normals->SplittingOff();

	mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->ScalarVisibilityOff();

	actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(color);
	actor->GetProperty()->SetDiffuse(1.0);
	actor->GetProperty()->SetAmbient(0.1);
	actor->GetProperty()->SetSpecular(0.0);

	SetFrontfaceCulling(false);

	UpdatePipeline();
}
	
RegionSurface::~RegionSurface() {
	while (actor->GetNumberOfConsumers() > 0) {
		vtkRenderer::SafeDownCast(actor->GetConsumer(0))->RemoveActor(actor);
	}
}

vtkSmartPointer<vtkActor> RegionSurface::GetActor() {
	return actor;
}

void RegionSurface::SetSmoothSurface(bool smooth) {
	smoothSurface = smooth;

	UpdatePipeline();
}

void RegionSurface::SetSmoothShading(bool smooth) {
	smoothShading = smooth;

	UpdatePipeline();
}

void RegionSurface::SetFrontfaceCulling(bool cull) {
	actor->GetProperty()->SetFrontfaceCulling(cull);
}

bool RegionSurface::IntersectsPlane(double p[3], double n[3]) {
	if (!IntersectsBoundingBox(p, n)) return false;

	return IntersectsSurface(p, n);
}

void RegionSurface::UpdatePipeline() {
	vtkAlgorithm* surface;

	if (smoothSurface) {
		surface = smoother;
	}
	else {
		surface = contour;
	}

	if (smoothShading) {
		normals->SetInputConnection(surface->GetOutputPort());
		mapper->SetInputConnection(normals->GetOutputPort());
	}
	else {
		mapper->SetInputConnection(surface->GetOutputPort());
	}
}

bool RegionSurface::IntersectsBoundingBox(double p[3], double n[3]) {
	double bb[6];
	actor->GetBounds(bb);

	// Get points on bounding box
	double points[8][6] = {
		{ bb[0], bb[2], bb[4] },
		{ bb[0], bb[2], bb[5] },
		{ bb[0], bb[3], bb[4] },
		{ bb[0], bb[3], bb[5] },
		{ bb[1], bb[2], bb[4] },
		{ bb[1], bb[2], bb[5] },
		{ bb[1], bb[3], bb[4] },
		{ bb[1], bb[3], bb[5] }
	};

	// Check if first point is on the plane
	double d = vtkPlane::Evaluate(n, p, points[0]);
	if (d == 0.0) return true;

	// Check if any points lie on the other side of the plane
	for (int i = 1; i < 8; i++) {
		double e = vtkPlane::Evaluate(n, p, points[i]);

		if (e == 0.0 || (e < 0.0 && d > 0.0) || (e > 0.0 && d < 0.0)) {
			return true;
		}
	}

	return false;
}

bool RegionSurface::IntersectsSurface(double p[3], double n[3]) {
	// Check all points
	vtkPolyData* data = mapper->GetInput();

	// Check if first point is on the plane
	double d = vtkPlane::Evaluate(n, p, data->GetPoint(0));
	if (d == 0.0) return true;

	// Check if any points lie on the other side of the plane
	for (int i = 0; i < data->GetNumberOfPoints(); i++) {
		double e = vtkPlane::Evaluate(n, p, data->GetPoint(i));

		if (e == 0.0 || (e < 0.0 && d > 0.0) || (e > 0.0 && d < 0.0)) {
			return true;
		}
	}

	return false;
}