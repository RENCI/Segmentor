#include "Region.h"

#include <algorithm>

#include <vtkActor.h>
#include <vtkExtractVOI.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkPlane.h>
#include <vtkProperty.h>
#include <vtkTable.h>
#include <vtkTextActor.h>
#include <vtkRenderer.h>
#include <vtkTextProperty.h>
#include <vtkThreshold.h>

#include <vtkOutlineFilter.h>
#include <vtkPolyDataMapper.h>

#include "vtkImageDataCells.h"

#include "LabelColors.h"
#include "RegionInfo.h"
#include "RegionSurface.h"
#include "RegionOutline.h"
#include "RegionVoxelOutlines.h"
#include "RegionHighlight3D.h"

Region::Region(unsigned short regionLabel, double regionColor[3], vtkImageData* inputData, const int* regionExtent) {
	visible = false;
	modified = false;
	done = false;

	// Input data info
	data = inputData;
	
	// Label for this region
	label = regionLabel;

	// Color for this region
	color[0] = regionColor[0];
	color[1] = regionColor[1];
	color[2] = regionColor[2];

	// Coordinate system for text
	vtkSmartPointer<vtkCoordinate> coord = vtkSmartPointer<vtkCoordinate>::New();
	coord->SetCoordinateSystemToNormalizedViewport();
	coord->SetValue(0, 1);

	// Text
	text = vtkSmartPointer<vtkTextActor>::New();
	text->SetInput(LabelString().c_str());
	text->GetTextProperty()->SetFontSize(18);
	text->GetTextProperty()->SetColor(color);
	text->GetPositionCoordinate()->SetReferenceCoordinate(coord);
	text->SetPosition(10, -30);
	text->VisibilityOff();

	voi = vtkSmartPointer<vtkExtractVOI>::New();
	voi->SetInputDataObject(data);

	vtkSmartPointer<vtkImageDataCells> cells = vtkSmartPointer<vtkImageDataCells>::New();
	cells->SetInputConnection(voi->GetOutputPort());

	threshold = vtkSmartPointer<vtkThreshold>::New();
	threshold->ThresholdBetween(label, label);
	threshold->SetInputConnection(cells->GetOutputPort());

	// Initialize the extent for this region
	if (regionExtent) {
		InitializeExtent(regionExtent);
	}
	else {
		ComputeExtent();
	}

	voi->Update();

	surface = new RegionSurface(this, color);
	outline = new RegionOutline(this, color);
	voxelOutlines = new RegionVoxelOutlines(this, color);
	highlight3D = new RegionHighlight3D(this, color);

#ifdef SHOW_REGION_BOX
	// Outline for testing bounding box
	vtkSmartPointer<vtkOutlineFilter> boxFilter = vtkSmartPointer<vtkOutlineFilter>::New();
	boxFilter->SetInputConnection(voi->GetOutputPort());

	vtkSmartPointer<vtkPolyDataMapper> boxMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	boxMapper->SetInputConnection(boxFilter->GetOutputPort());

	box = vtkSmartPointer<vtkActor>::New();
	box->GetProperty()->SetColor(0.5, 0.5, 0.5);
	box->GetProperty()->LightingOff();
	box->SetMapper(boxMapper);
	box->PickableOff();
	box->VisibilityOff();
#endif
}

Region::Region(const RegionInfo& info, vtkImageData* inputData) {
	// Input data info
	data = inputData;

	voi = vtkSmartPointer<vtkExtractVOI>::New();
	voi->SetInputDataObject(data);

	// Text
	text = vtkSmartPointer<vtkTextActor>::New();

	SetInfo(info);

	text->SetInput(LabelString().c_str());
	text->GetTextProperty()->SetColor(color);
	text->VisibilityOff();

	vtkSmartPointer<vtkImageDataCells> cells = vtkSmartPointer<vtkImageDataCells>::New();
	cells->SetInputConnection(voi->GetOutputPort());

	threshold = vtkSmartPointer<vtkThreshold>::New();
	threshold->ThresholdBetween(label, label);
	threshold->SetInputConnection(cells->GetOutputPort());

	voi->Update();

	surface = new RegionSurface(this, color);
	outline = new RegionOutline(this, color);
	voxelOutlines = new RegionVoxelOutlines(this, color);
	highlight3D = new RegionHighlight3D(this, color);
}
	
Region::~Region() {
	ClearLabels();

	while (text->GetNumberOfConsumers() > 0) {
		vtkRenderer::SafeDownCast(text->GetConsumer(0))->RemoveActor(text);
	}

#ifdef SHOW_REGION_BOX
	while (box->GetNumberOfConsumers() > 0) {
		vtkRenderer::SafeDownCast(box->GetConsumer(0))->RemoveActor(box);
	}
#endif

	delete surface;
	delete outline;
	delete voxelOutlines;
	delete highlight3D;
}

vtkAlgorithmOutput* Region::GetOutput() {
	return voi->GetOutputPort();
}

vtkAlgorithmOutput* Region::GetCells() {
	return threshold->GetOutputPort();
}

vtkSmartPointer<vtkTable> Region::GetPointTable() {	
	vtkSmartPointer<vtkTable> table = vtkSmartPointer<vtkTable>::New();

	vtkSmartPointer<vtkIntArray> x = vtkSmartPointer<vtkIntArray>::New();
	x->SetNumberOfComponents(1);
	x->SetName("x");

	vtkSmartPointer<vtkIntArray> y = vtkSmartPointer<vtkIntArray>::New();
	y->SetNumberOfComponents(1);
	y->SetName("y");

	vtkSmartPointer<vtkIntArray> z = vtkSmartPointer<vtkIntArray>::New();
	z->SetNumberOfComponents(1);
	z->SetName("z");

	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			for (int k = extent[4]; k <= extent[5]; k++) {
				unsigned short* p = static_cast<unsigned short*>(data->GetScalarPointer(i, j, k));
				if (*p == label) {
					x->InsertNextValue(i);
					y->InsertNextValue(j);
					z->InsertNextValue(k);
				}
			}
		}
	}

	table->AddColumn(x);
	table->AddColumn(y);
	table->AddColumn(z);

	return table;
}

RegionSurface* Region::GetSurface() {
	return surface;
}

RegionOutline* Region::GetOutline() {
	return outline;
}

RegionVoxelOutlines* Region::GetVoxelOutlines() {
	return voxelOutlines;
}

RegionHighlight3D* Region::GetHighlight3D() {
	return highlight3D;
}

vtkSmartPointer<vtkTextActor> Region::GetText() {
	return text;
}

vtkSmartPointer<vtkImageData> Region::GetZSlice(int z) {
	int extent[6];
	voi->GetOutput()->GetExtent(extent);
	extent[4] = extent[5] = z;

	vtkSmartPointer<vtkExtractVOI> slice = vtkSmartPointer<vtkExtractVOI>::New();
	slice->SetVOI(extent);
	slice->SetInputConnection(voi->GetOutputPort());
	slice->Update();

	return slice->GetOutput();
}

#ifdef SHOW_REGION_BOX
vtkSmartPointer<vtkActor> Region::GetBox() {
	return box;
}
#endif

void Region::SetExtent(int newExtent[6]) {
	for (int i = 0; i < 6; i++) {
		extent[i] = newExtent[i];
	}

	UpdateExtent();
}

void Region::UpdateExtent(int x, int y, int z) {
	bool update = false;

	if (x < extent[0]) {
		extent[0] = x;
		update = true;
	}
	else if (x > extent[1]) {
		extent[1] = x;
		update = true;
	}

	if (y < extent[2]) {
		extent[2] = y;
		update = true;
	}
	else if (y > extent[3]) {
		extent[3] = y;
		update = true;
	}

	if (z < extent[4]) {
		extent[4] = z;
		update = true;
	}
	else if (z > extent[5]) {
		extent[5] = z;
		update = true;
	}

	if (update) {
		UpdateExtent();
	}
}

void Region::ShrinkExtent() {
	// Copy extent before passing in
	int tempExtent[6];
	for (int i = 0; i < 6; i++) {
		tempExtent[i] = extent[i];
	}

	ShrinkExtent(tempExtent);
}

void Region::ShrinkExtent(const int startExtent[6]) {
	bool update = false;

	extent[0] = startExtent[1];
	extent[1] = startExtent[0];
	extent[2] = startExtent[3];
	extent[3] = startExtent[2];
	extent[4] = startExtent[5];
	extent[5] = startExtent[4];

	bool hasVoxel = false;

	for (int i = startExtent[0]; i <= startExtent[1]; i++) {
		for (int j = startExtent[2]; j <= startExtent[3]; j++) {
			for (int k = startExtent[4]; k <= startExtent[5]; k++) {
				unsigned short* p = static_cast<unsigned short*>(data->GetScalarPointer(i, j, k));

				if (*p == label) {
					if (i < extent[0]) extent[0] = i;
					if (i > extent[1]) extent[1] = i;
					if (j < extent[2]) extent[2] = j;
					if (j > extent[3]) extent[3] = j;
					if (k < extent[4]) extent[4] = k;
					if (k > extent[5]) extent[5] = k;

					hasVoxel = true;
				}
			}
		}
	}

	// Fix extent if no voxels with this label
	if (!hasVoxel) {
		extent[0] = extent[1] = startExtent[0];
		extent[2] = extent[3] = startExtent[2];
		extent[4] = extent[5] = startExtent[4];
	}

	UpdateExtent();
}


void Region::UpdateExtent() {
	int dataExtent[6];
	data->GetExtent(dataExtent);

	// Add a buffer around VOI
	const int padding = 2;

	int padExtent[6];
	padExtent[0] = std::max(dataExtent[0], extent[0] - padding);
	padExtent[1] = std::min(dataExtent[1], extent[1] + padding);
	padExtent[2] = std::max(dataExtent[2], extent[2] - padding);
	padExtent[3] = std::min(dataExtent[3], extent[3] + padding);
	padExtent[4] = std::max(dataExtent[4], extent[4] - padding);
	padExtent[5] = std::min(dataExtent[5], extent[5] + padding);

	voi->SetVOI(padExtent);

	//text->SetPosition(
//		(padExtent[1] - padExtent[0]) / 2,
//		(padExtent[3] - padExtent[2]) / 2
//	);

	/*
	double bounds[6];
	voi->Update();
	voi->GetOutput()->GetBounds(bounds);
	
	text->SetPosition(
		(bounds[1] - bounds[0]) / 2,
		(bounds[3] - bounds[2]) / 2,
		(bounds[5] - bounds[4]) / 2
	);
	*/
}

void Region::InitializeExtent(const int* regionExtent) {
	// Initialize extent for this region
	extent[0] = regionExtent[0];
	extent[1] = regionExtent[1];
	extent[2] = regionExtent[2];
	extent[3] = regionExtent[3];
	extent[4] = regionExtent[4];
	extent[5] = regionExtent[5];

	UpdateExtent();
}

void Region::ComputeExtent() {
	int dataExtent[6];
	data->GetExtent(dataExtent);

	ShrinkExtent(dataExtent);
}

bool Region::GetVisible() {
	return visible;
}

void Region::SetVisible(bool isVisible) {
	visible = isVisible;
}

bool Region::GetModified() {
	return modified;
}

void Region::SetModified(bool isModified) {
	modified = isModified;
}

bool Region::GetDone() {
	return done;
}

void Region::SetDone(bool isDone) {
	done = isDone;

	UpdateColor();
}

void Region::SetColor(double r, double g, double b) {
	color[0] = r;
	color[1] = g;
	color[2] = b;

	UpdateColor();	
}

void Region::UpdateColor() {
	if (done) {
		text->GetTextProperty()->SetColor(LabelColors::doneColor);
		surface->GetActor()->GetProperty()->SetColor(LabelColors::doneColor);
		outline->GetActor()->GetProperty()->SetColor(LabelColors::doneColor);
		voxelOutlines->GetActor()->GetProperty()->SetColor(LabelColors::doneColor);
		highlight3D->GetActor()->GetProperty()->SetColor(LabelColors::doneColor);
	}
	else {
		text->GetTextProperty()->SetColor(color);
		surface->GetActor()->GetProperty()->SetColor(color);
		outline->GetActor()->GetProperty()->SetColor(color);
		voxelOutlines->GetActor()->GetProperty()->SetColor(color);
		highlight3D->GetActor()->GetProperty()->SetColor(color);
	}
}

void Region::ShowText(bool show) {
	if (!text) return;

	if (show) {
/*
		double bounds[6];
		voi->GetOutput()->GetBounds(bounds);
		
		text->SetPosition(
			bounds[0] + (bounds[1] - bounds[0]) / 2,
			bounds[3],
			bounds[4] + (bounds[5] - bounds[4]) / 2
		);
*/
		text->VisibilityOn();

#ifdef SHOW_REGION_BOX
		box->VisibilityOn();
#endif
	}
	else {
		text->VisibilityOff();

#ifdef SHOW_REGION_BOX
		box->VisibilityOff();
#endif
	}
}

unsigned short Region::GetLabel() {
	return label;
}

const double* Region::GetColor() {
	return color;
}

int Region::GetNumVoxels() {
	voi->Update();

	vtkImageData* voiData = voi->GetOutput();
	unsigned short* scalars = static_cast<unsigned short*>(voiData->GetScalarPointer());
	int numPoints = voiData->GetNumberOfPoints();

	int numVoxels = 0;
	for (int i = 0; i < numPoints; i++) {
		if (scalars[i] == label) numVoxels++;
	}

	return numVoxels;
}

int Region::GetNumVoxels(int slice) {
	int numVoxels = 0;

	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			unsigned short* p = static_cast<unsigned short*>(data->GetScalarPointer(i, j, slice));
			if (*p == label) numVoxels++;
		}
	}

	return numVoxels;
}

const int* Region::GetExtent() {
	return extent;
}

void Region::GetExtent(int outExtent[6]) {
	for (int i = 0; i < 6; i++) {
		outExtent[i] = extent[i];
	}
}

double* Region::GetCenter() {
	return voi->GetOutput()->GetCenter();
}

double Region::GetLength() {
	return voi->GetOutput()->GetLength();
}

double Region::GetXYDistance(int x, int y, int z) {
	double distance2 = VTK_DOUBLE_MAX;

	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			unsigned short* p = static_cast<unsigned short*>(data->GetScalarPointer(i, j, z));
			if (*p == label) {
				double dx = x - i;
				double dy = y - j;
				double d2 = dx * dx + dy * dy;

				if (d2 < distance2) distance2 = d2;
			}
		}
	}

	return sqrt(distance2);
}

bool Region::GetSeed(double point[3]) {
	int extent[6];
	voi->GetOutput()->GetExtent(extent);

	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			for (int k = extent[4]; k <= extent[5]; k++) {
				unsigned short* p = static_cast<unsigned short*>(data->GetScalarPointer(i, j, k));
				if (*p == 0) {
					int ijk[3] = { i, j, k };

					vtkIdType id = voi->GetOutput()->ComputePointId(ijk);
					voi->GetOutput()->GetPoint(id, point);

					return true;
				}
			}
		}
	}

	return false;
}

bool Region::GetSeed(double point[3], int z) {
	int extent[6];
	voi->GetOutput()->GetExtent(extent);

	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			unsigned short* p = static_cast<unsigned short*>(data->GetScalarPointer(i, j, z));
			if (*p == 0) {
				int ijk[3] = { i, j, z };

				vtkIdType id = voi->GetOutput()->ComputePointId(ijk);
				voi->GetOutput()->GetPoint(id, point);

				return true;
			}
		}
	}

	return false;
}

void Region::SetInfo(const RegionInfo& info) {
	label = info.label;

	for (int i = 0; i < 3; i++) {
		color[i] = info.color[i];
	}

	if (info.extent[0] >= 0) {
		InitializeExtent(info.extent);
	}
	else {
		ComputeExtent();
	}

	visible = info.visible;
	modified = info.modified;
	done = info.done;
	comment = info.comment;
}

bool Region::HasComment() {
	return comment.size() > 0;
}

const std::string& Region::GetComment() {
	return comment;
}

void Region::SetComment(const std::string& commentString) {
	comment = commentString;

	text->SetInput(LabelString().c_str());
}

void Region::ClearLabels() {
	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			for (int k = extent[4]; k <= extent[5]; k++) {
				unsigned short* p = static_cast<unsigned short*>(data->GetScalarPointer(i, j, k));
				if (*p == label) *p = 0;
			}
		}
	}

	data->Modified();
}

std::string Region::LabelString() {
	std::string s = std::to_string(label).c_str();

	if (comment.length() > 0) {
		s += ": " + comment;
	}

	return s;
}