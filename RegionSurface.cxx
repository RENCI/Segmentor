#include "RegionSurface.h"

#include <vtkActor.h>
#include <vtkDiscreteFlyingEdges3D.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkWindowedSincPolyDataFilter.h>

#include "Region.h"

RegionSurface::RegionSurface(Region* inputRegion, vtkLookupTable* lut) {
	smoothSurfaces = false;
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

	double color[3];
	lut->GetColor(region->GetLabel(), color);

	actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(color);
	actor->GetProperty()->SetDiffuse(1.0);
	actor->GetProperty()->SetAmbient(0.1);
	actor->GetProperty()->SetSpecular(0.0);

	UpdatePipeline();
}
	
RegionSurface::~RegionSurface() {
}

Region* RegionSurface::GetRegion() {
	return region;
}

vtkSmartPointer<vtkActor> RegionSurface::GetActor() {
	return actor;
}

void RegionSurface::SetSmoothSurfaces(bool smooth) {
	smoothSurfaces = smooth;

	UpdatePipeline();
}

void RegionSurface::SetSmoothShading(bool smooth) {
	smoothShading = smooth;

	UpdatePipeline();
}

void RegionSurface::UpdatePipeline() {
	vtkAlgorithm* surface;

	if (smoothSurfaces) {
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