#include "RegionSurface.h"

#include <vtkActor.h>
#include <vtkDiscreteFlyingEdges3D.h>
#include <vtkLookupTable.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkWindowedSincPolyDataFilter.h>

#include "Region.h"

RegionSurface::RegionSurface(Region* region, vtkLookupTable* lut) {
	//		vtkSmartPointer<vtkContourFilter> contour = vtkSmartPointer<vtkContourFilter>::New();
	//		vtkSmartPointer<vtkFlyingEdges3D> contour = vtkSmartPointer<vtkFlyingEdges3D>::New();
	vtkSmartPointer<vtkDiscreteFlyingEdges3D> contour = vtkSmartPointer<vtkDiscreteFlyingEdges3D>::New();
	contour->SetValue(0, region->GetLabel());
	contour->ComputeNormalsOff();
	contour->ComputeGradientsOff();
	contour->SetInputConnection(region->GetOutput());
	//		contour->SetInputDataObject(data);

	// Smoother
	int smoothingIterations = 8;
	double passBand = 0.01;
	double featureAngle = 120.0;

	vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother = vtkSmartPointer<vtkWindowedSincPolyDataFilter>::New();
	smoother->SetNumberOfIterations(smoothingIterations);
	//smoother->BoundarySmoothingOff();
	//smoother->FeatureEdgeSmoothingOff();
	//smoother->SetFeatureAngle(featureAngle);
	smoother->SetPassBand(passBand);
	//smoother->NonManifoldSmoothingOn();
	smoother->NormalizeCoordinatesOn();
	smoother->SetInputConnection(contour->GetOutputPort());

	vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();
	normals->ComputePointNormalsOn();
	normals->SplittingOff();
	normals->SetInputConnection(contour->GetOutputPort());
	//		normals->SetInputConnection(smoother->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(normals->GetOutputPort());
	//		mapper->SetInputConnection(smoother->GetOutputPort());
	//mapper->SetInputConnection(contour->GetOutputPort());
	mapper->ScalarVisibilityOff();

	double color[3];
	lut->GetColor(region->GetLabel(), color);

	actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);
	actor->GetProperty()->SetColor(color);
	actor->GetProperty()->SetDiffuse(1.0);
	actor->GetProperty()->SetAmbient(0.1);
	actor->GetProperty()->SetSpecular(0.0);
}
	
RegionSurface::~RegionSurface() {
}

vtkSmartPointer<vtkActor> RegionSurface::GetActor() {
	return actor;
}