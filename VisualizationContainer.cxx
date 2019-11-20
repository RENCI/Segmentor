#include "VisualizationContainer.h"

#include "MainWindow.h"

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkImageConnectivityFilter.h>
#include <vtkImageOpenClose3D.h>
#include <vtkImageThreshold.h>
#include <vtkIdTypeArray.h>
#include <vtkIntArray.h>
#include <vtkLookupTable.h>
#include <vtkNIFTIImageReader.h>
#include <vtkNIFTIImageWriter.h>
#include <vtkPointSource.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTIFFReader.h>
#include <vtkTIFFWriter.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>

#include <vtkInteractorStyleSlice.h>
#include <vtkInteractorStyleVolume.h>

#include "InteractionEnums.h"
#include "InteractionCallbacks.h"
#include "SegmentorMath.h"
#include "SlicePipeline.h"
#include "VolumePipeline.h"
#include "Region.h"
#include "RegionCollection.h"

VisualizationContainer::VisualizationContainer(vtkRenderWindowInteractor* volumeInteractor, vtkRenderWindowInteractor* sliceInteractor, MainWindow* mainWindow) {
	data = nullptr;
	labels = nullptr;
	regions = new RegionCollection();
	currentRegion = nullptr;

	// Qt main window
	qtWindow = mainWindow;

	// Lookup table
	labelColors = vtkSmartPointer<vtkLookupTable>::New();

	// Create rendering pipelines
	volumePipeline = new VolumePipeline(volumeInteractor);
	slicePipeline = new SlicePipeline(sliceInteractor, labelColors);

	// Set to edit mode, then toggle to propagate change
	interactionMode = EditMode;
	ToggleInteractionMode();

	// Callbacks

	// Camera
	vtkSmartPointer <vtkCallbackCommand> volumeCameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeCameraCallback->SetCallback(InteractionCallbacks::VolumeCameraChange);
	volumeCameraCallback->SetClientData(slicePipeline->GetRenderer());
	volumePipeline->GetRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, volumeCameraCallback);

	vtkSmartPointer <vtkCallbackCommand> sliceCameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceCameraCallback->SetCallback(InteractionCallbacks::SliceCameraChange);
	sliceCameraCallback->SetClientData(volumePipeline->GetRenderer());
	slicePipeline->GetRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, sliceCameraCallback);

	// Char
	vtkSmartPointer<vtkCallbackCommand> onCharCallback = vtkSmartPointer<vtkCallbackCommand>::New(); 
	onCharCallback->SetCallback(InteractionCallbacks::OnChar);
	onCharCallback->SetClientData(this);
	volumeInteractor->AddObserver(vtkCommand::CharEvent, onCharCallback);
	sliceInteractor->AddObserver(vtkCommand::CharEvent, onCharCallback);

	// Label select
	vtkSmartPointer<vtkCallbackCommand> volumeSelectLabelCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeSelectLabelCallback->SetCallback(InteractionCallbacks::VolumeSelectLabel);
	volumeSelectLabelCallback->SetClientData(this);
	volumePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::SelectLabelEvent, volumeSelectLabelCallback);

	vtkSmartPointer<vtkCallbackCommand> sliceSelectLabelCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceSelectLabelCallback->SetCallback(InteractionCallbacks::SliceSelectLabel);
	sliceSelectLabelCallback->SetClientData(this);
	slicePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::SelectLabelEvent, sliceSelectLabelCallback);

	// Paint
	vtkSmartPointer<vtkCallbackCommand> volumePaintCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumePaintCallback->SetCallback(InteractionCallbacks::VolumePaint);
	volumePaintCallback->SetClientData(this);
	volumePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::PaintEvent, volumePaintCallback);

	vtkSmartPointer<vtkCallbackCommand> slicePaintCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	slicePaintCallback->SetCallback(InteractionCallbacks::SlicePaint);
	slicePaintCallback->SetClientData(this);
	slicePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::PaintEvent, slicePaintCallback);

	// Erase
	vtkSmartPointer<vtkCallbackCommand> volumeEraseCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeEraseCallback->SetCallback(InteractionCallbacks::VolumeErase);
	volumeEraseCallback->SetClientData(this);
	volumePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::EraseEvent, volumeEraseCallback);

	vtkSmartPointer<vtkCallbackCommand> sliceEraseCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceEraseCallback->SetCallback(InteractionCallbacks::SliceErase);
	sliceEraseCallback->SetClientData(this);
	slicePipeline->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::EraseEvent, sliceEraseCallback);

	// Mouse move
	vtkSmartPointer<vtkCallbackCommand> volumeMouseMoveCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeMouseMoveCallback->SetCallback(InteractionCallbacks::VolumeMouseMove);
	volumeMouseMoveCallback->SetClientData(this);
	volumeInteractor->AddObserver(vtkCommand::MouseMoveEvent, volumeMouseMoveCallback);

	vtkSmartPointer<vtkCallbackCommand> sliceMouseMoveCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceMouseMoveCallback->SetCallback(InteractionCallbacks::SliceMouseMove);
	sliceMouseMoveCallback->SetClientData(this);
	sliceInteractor->AddObserver(vtkCommand::MouseMoveEvent, sliceMouseMoveCallback);
}

VisualizationContainer::~VisualizationContainer() {
	delete volumePipeline;
	delete slicePipeline;
	delete regions;
}

bool VisualizationContainer::OpenImageFile(const std::string& fileName) {
	// Get the file extension
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	// Load the data
	if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(fileName.c_str());
		reader->Update();

		data = reader->GetOutput();
	}
	else if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
		reader->SetFileName(fileName.c_str());
		reader->Update();

		data = reader->GetOutput();
	}
	else {
		return false;
	}	

	slicePipeline->SetImageData(data);

	return true;
}

bool VisualizationContainer::OpenImageStack(const std::vector<std::string>& fileNames) {
	// Get the file extension
	std::string extension = fileNames[0].substr(fileNames[0].find_last_of(".") + 1);

	// Set file names to pass to VTK
	vtkSmartPointer<vtkStringArray> names = vtkSmartPointer<vtkStringArray>::New();
	names->SetNumberOfValues(fileNames.size());

	for (int i = 0; i < fileNames.size(); i++) {
		names->SetValue(i, fileNames[i].c_str());
	}

	// Load the data
	if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileNames(names);
		reader->Update();

		data = reader->GetOutput();
	}
	else {
		return false;
	}

	slicePipeline->SetImageData(data);

	return true;
}

bool VisualizationContainer::OpenSegmentationFile(const std::string& fileName) {
	// Get the file extension
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	// Load the data
	if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(fileName.c_str());
		reader->Update();

		labels = reader->GetOutput();
	}
	else if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
		reader->SetFileName(fileName.c_str());
		reader->Update();

		labels = reader->GetOutput();
	}
	else {
		return false;
	}

	UpdateLabels();

	return true;
}

bool VisualizationContainer::OpenSegmentationStack(const std::vector<std::string>& fileNames) {
	// Get the file extension
	std::string extension = fileNames[0].substr(fileNames[0].find_last_of(".") + 1);

	// Set file names to pass to VTK
	vtkSmartPointer<vtkStringArray> names = vtkSmartPointer<vtkStringArray>::New();
	names->SetNumberOfValues(fileNames.size());

	for (int i = 0; i < fileNames.size(); i++) {
		names->SetValue(i, fileNames[i].c_str());
	}

	// Load the data
	if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileNames(names);
		reader->Update();

		labels = reader->GetOutput();
	}
	else {
		return false;
	}

	UpdateLabels();

	return true;
}

bool VisualizationContainer::SaveSegmentationData(const std::string& fileName) {
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
		writer->SetFileName(fileName.c_str());
		writer->SetInputDataObject(labels);
		writer->Update();
	}
	else if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageWriter> writer = vtkSmartPointer<vtkNIFTIImageWriter>::New();
		writer->SetFileName(fileName.c_str());
		writer->SetInputDataObject(labels);
		writer->Update();
	}
	else if (extension == "tif" || extension == "tiff") {
		std::string prefix = fileName.substr(0, fileName.find_last_of("."));

		vtkSmartPointer<vtkTIFFWriter> writer = vtkSmartPointer<vtkTIFFWriter>::New();
		writer->SetFilePrefix(prefix.c_str());
		writer->SetFilePattern("%s_%04d.tif");
		writer->SetFileDimensionality(2);
		writer->SetInputDataObject(labels);
		writer->Update();
	}
	else {
		return false;
	}
	
	return true;
}

void VisualizationContainer::SegmentVolume() {
	if (!data) return;

	// Get Otsu threshold
	double value = SegmentorMath::OtsuThreshold(data);

	// Filter
	vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
	threshold->ThresholdByUpper(value);
	threshold->SetInValue(255);
	threshold->SetOutValue(0);
	threshold->ReplaceInOn();
	threshold->ReplaceOutOn();
	threshold->SetOutputScalarTypeToUnsignedChar();
	threshold->SetInputDataObject(data);

	vtkSmartPointer<vtkImageOpenClose3D> openClose = vtkSmartPointer<vtkImageOpenClose3D>::New();
	openClose->SetKernelSize(10, 10, 10);
	openClose->SetOpenValue(0);
	openClose->SetCloseValue(255);
	openClose->SetInputConnection(threshold->GetOutputPort());

	// Generate labels
	vtkSmartPointer<vtkImageConnectivityFilter> connectivity = vtkSmartPointer<vtkImageConnectivityFilter>::New();
	connectivity->SetScalarRange(255, 255);
	connectivity->SetLabelScalarTypeToUnsignedShort();
	connectivity->SetInputConnection(openClose->GetOutputPort());
	connectivity->Update();

	labels = connectivity->GetOutput();

	UpdateLabels();
}

void VisualizationContainer::ToggleInteractionMode() {
	interactionMode = interactionMode == NavigationMode ? EditMode : NavigationMode;

	slicePipeline->SetInteractionMode(interactionMode);
	volumePipeline->SetInteractionMode(interactionMode);

	Render();
}

void VisualizationContainer::PickLabel(int x, int y, int z) {
	if (!labels) return;
	
	SetCurrentRegion(regions->Get(GetLabel(x, y, z)));
}

void VisualizationContainer::Paint(int x, int y, int z) {
	if (!labels || !currentRegion) return;

	currentRegion->UpdateExtent(x, y, z);

	SetLabel(x, y, z, currentRegion->GetLabel());
}

void VisualizationContainer::Erase(int x, int y, int z) {	
	if (!labels || !currentRegion) return;

	// TODO: SHRINK EXTENT

	SetLabel(x, y, z, 0);
}

void VisualizationContainer::PickPointLabel(double x, double y, double z) {
	if (!labels) return;

	double p[3] = { x, y, z };
	int s[3];
	PointToStructured(p, s);

	PickLabel(s[0], s[1], s[2]);
}

void VisualizationContainer::PaintPoint(double x, double y, double z) {
	if (!labels) return;

	double p[3] = { x, y, z };
	int s[3];
	PointToStructured(p, s);

	Paint(s[0], s[1], s[2]);
}

void VisualizationContainer::ErasePoint(double x, double y, double z) {
	if (!labels) return;

	double p[3] = { x, y, z };
	int s[3];
	PointToStructured(p, s);

	Erase(s[0], s[1], s[2]);
}

void VisualizationContainer::SetCurrentRegion(Region* region) {
	currentRegion = region;
	volumePipeline->SetCurrentRegion(currentRegion);
	slicePipeline->SetCurrentRegion(currentRegion);

	if (currentRegion) {
		slicePipeline->SetCurrentLabel(currentRegion->GetLabel());
	}
	else {
		slicePipeline->SetCurrentLabel(0);
	}
}

void VisualizationContainer::RelabelCurrentRegion() {
	if (!currentRegion) return;

	unsigned short label = currentRegion->GetLabel();

	vtkSmartPointer<vtkImageConnectivityFilter> connectivity = vtkSmartPointer<vtkImageConnectivityFilter>::New();
	connectivity->SetScalarRange(label, label);
	connectivity->SetLabelScalarTypeToUnsignedShort();
	connectivity->SetLabelModeToSizeRank();
	connectivity->GenerateRegionExtentsOn();
	connectivity->SetInputDataObject(labels);
	connectivity->Update();

	int numComponents = connectivity->GetNumberOfExtractedRegions();
	vtkIdTypeArray* componentLabels = connectivity->GetExtractedRegionLabels();
	vtkIntArray* componentExtents = connectivity->GetExtractedRegionExtents();
	vtkImageData* connectivityOutput = connectivity->GetOutput();

	if (numComponents == 0) {
		RemoveRegion(label);
	}
	else {
		for (int i = 0; i < numComponents; i++) {
			// Get the extent for this component
			double* componentExtent = componentExtents->GetTuple(i);
			int extent[6];
			for (int j = 0; j < 6; j++) {
				extent[j] = (int)componentExtent[j];
			}

			if (i == 0) {
				// Use current region
				currentRegion->SetExtent(extent);
			}
			else {
				// Label from the connectivity filter
				unsigned short componentLabel = (unsigned short)componentLabels->GetTuple1(i);

				// Get label for new region
				unsigned short newLabel = regions->GetNewLabel();

				// Update label data
				for (int i = extent[0]; i <= extent[1]; i++) {
					for (int j = extent[2]; j <= extent[3]; j++) {
						for (int k = extent[4]; k <= extent[5]; k++) {
							unsigned short* labelData = static_cast<unsigned short*>(labels->GetScalarPointer(i, j, k));
							unsigned short* connectivityData = static_cast<unsigned short*>(connectivityOutput->GetScalarPointer(i, j, k));

							if (*connectivityData == componentLabel) *labelData = newLabel;
						}
					}
				}

				UpdateColors(newLabel);

				// Create new region
				Region* newRegion = new Region(newLabel, labelColors->GetTableValue(newLabel), labels);
				regions->Add(newRegion);
			}			
		}

		qtWindow->UpdateRegionTable(regions);

		labels->Modified();
		Render();
	}
}

void VisualizationContainer::MergeWithCurrentRegion(int x, int y, int z) {
	if (!currentRegion) return;

	unsigned short currentLabel = currentRegion->GetLabel();

	// Get the region at the point
	unsigned short label = GetLabel(x, y, z);
	Region* region = regions->Get(label);

	const int* extent = region->GetExtent();

	// Update label data
	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			for (int k = extent[4]; k <= extent[5]; k++) {
				unsigned short* labelData = static_cast<unsigned short*>(labels->GetScalarPointer(i, j, k));

				if (*labelData == label) *labelData = currentLabel;
			}
		}
	}

	// Update current region extent
	const int* currentExtent = currentRegion->GetExtent();

	int newExtent[6];
	for (int i = 0; i < 6; i++) {
		newExtent[i] = i % 2 == 0 ? 
			std::min(extent[i], currentExtent[i]) : 
			std::max(extent[i], currentExtent[i]);
	}

	currentRegion->SetExtent(newExtent);

	// Remove region
	RemoveRegion(label);
}

void VisualizationContainer::GrowRegion(int x, int y, int z) {
	if (!currentRegion) return;

	double value = GetValue(x, y, z);
	double label = GetLabel(x, y, z);

	bool grow = label == 0;

	double growValue = 255;

	vtkSmartPointer<vtkPointSource> seed = vtkSmartPointer<vtkPointSource>::New();
	seed->SetNumberOfPoints(1);
	seed->SetCenter(x, y, z);
	seed->SetRadius(0);

	// Variables for growing vs. shrinking
	double min, max;
	if (grow) {
		min = value;
		max = VTK_DOUBLE_MAX;
	}
	else {
		min = VTK_DOUBLE_MIN;
		max = value;
	}

	vtkSmartPointer<vtkImageConnectivityFilter> regionGrow = vtkSmartPointer<vtkImageConnectivityFilter>::New();
	regionGrow->SetExtractionModeToSeededRegions();
	regionGrow->SetSeedConnection(seed->GetOutputPort());
	regionGrow->SetScalarRange(min, max);
	regionGrow->SetLabelModeToConstantValue();
	regionGrow->SetLabelConstantValue(growValue);
	regionGrow->GenerateRegionExtentsOn();
	regionGrow->SetInputDataObject(data);
	regionGrow->Update();

	if (regionGrow->GetNumberOfExtractedRegions() != 1) {
		std::cout << "Invalid number of extracted regions: " << regionGrow->GetNumberOfExtractedRegions() << std::endl;
		return;
	}

	int extent[6];
	regionGrow->GetExtractedRegionExtents()->GetTypedTuple(0, extent);

	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			for (int k = extent[4]; k <= extent[5]; k++) {
				double v = regionGrow->GetOutput()->GetScalarComponentAsDouble(i, j, k, 0);

				if (v >= growValue) {
					if (grow) {
						Paint(i, j, k);
					}
					else {
						Erase(i, j, k);
					}
				}
			}
		}
	}

	Render();
}

void VisualizationContainer::SetRegionDone(unsigned short label, bool done) {
	Region* region = regions->Get(label);

	if (!region) return;

	region->SetDone(done);

	if (done) {
		// Set to grey
		labelColors->SetTableValue(label, 0.5, 0.5, 0.5);
		labelColors->Build();
	}
	else {
		// Set to color map
		UpdateColors(label);
	}

	Render();
}

void VisualizationContainer::RemoveRegion(unsigned short label) {
	Region* region = regions->Get(label);

	if (region == currentRegion) SetCurrentRegion(nullptr);

	regions->Remove(label);
	qtWindow->UpdateRegionTable(regions);

	Render();
}

void VisualizationContainer::Render() {
	volumePipeline->Render();
	slicePipeline->Render();
}

VolumePipeline* VisualizationContainer::GetVolumePipeline() {
	return volumePipeline;
}

SlicePipeline* VisualizationContainer::GetSlicePipeline() {
	return slicePipeline;
}

void VisualizationContainer::UpdateLabels() {
	UpdateColors();
	ExtractRegions();

	volumePipeline->SetRegions(labels, regions);
	slicePipeline->SetSegmentationData(labels, regions);

	qtWindow->UpdateRegionTable(regions);
}

void VisualizationContainer::UpdateColors(unsigned short label) {
	// XXX: Copied from UpdateColors
	//		Move to separate color management class
	// Colors from ColorBrewer
	const int numColors = 12;
	double colors[numColors][3] = {
		{ 166,206,227 },
		{ 31,120,180 },
		{ 178,223,138 },
		{ 51,160,44 },
		{ 251,154,153 },
		{ 227,26,28 },
		{ 253,191,111 },
		{ 255,127,0 },
		{ 202,178,214 },
		{ 106,61,154 },
		{ 255,255,153 },
		{ 177,89,40 }
	};

	for (int i = 0; i < numColors; i++) {
		for (int j = 0; j < 3; j++) {
			colors[i][j] /= 255.0;
		}
	}

	if (label >= labelColors->GetNumberOfTableValues()) {
		labelColors->SetNumberOfTableValues(label + 1);
		labelColors->SetRange(0, label);
	}

	double* c = colors[(label - 1) % numColors];
	labelColors->SetTableValue(label, c[0], c[1], c[2]);
	labelColors->Build();
}

void VisualizationContainer::UpdateColors() {
	// Get label info
	int maxLabel = labels->GetScalarRange()[1];

	// Colors from ColorBrewer
	const int numColors = 12;
	double colors[numColors][3] = {
		{ 166,206,227 },
		{ 31,120,180 },
		{ 178,223,138 },
		{ 51,160,44 },
		{ 251,154,153 },
		{ 227,26,28 },
		{ 253,191,111 },
		{ 255,127,0 },
		{ 202,178,214 },
		{ 106,61,154 },
		{ 255,255,153 },
		{ 177,89,40 }
	};

	for (int i = 0; i < numColors; i++) {
		for (int j = 0; j < 3; j++) {
			colors[i][j] /= 255.0;
		}
	}

	// Label colors
	labelColors->SetNumberOfTableValues(maxLabel + 1);
	labelColors->SetRange(0, maxLabel);
	labelColors->SetTableValue(0, 0.0, 0.0, 0.0);
	for (int i = 1; i <= maxLabel; i++) {
		double* c = colors[(i - 1) % numColors];
		labelColors->SetTableValue(i, c[0], c[1], c[2]);
	}
	labelColors->Build();
}

void VisualizationContainer::ExtractRegions() {
	// Get label info
	int maxLabel = labels->GetScalarRange()[1];

	// Clear current regions
	regions->RemoveAll();

	for (int label = 1; label <= maxLabel; label++) {
		regions->Add(new Region(label, labelColors->GetTableValue(label), labels));
	}
}

void VisualizationContainer::SetLabel(int x, int y, int z, unsigned short label) {
	unsigned short* p = static_cast<unsigned short*>(labels->GetScalarPointer(x, y, z));

	// Restrict painting to no label, and erasing to current label
	if ((label != 0 && *p == 0) ||
		(label == 0 && *p == currentRegion->GetLabel())) {
		*p = label;
		labels->Modified();
	}
}

unsigned short VisualizationContainer::GetLabel(int x, int y, int z) {
	return *(static_cast<unsigned short*>(labels->GetScalarPointer(x, y, z)));
}

double VisualizationContainer::GetValue(int x, int y, int z) {
	return data->GetScalarComponentAsDouble(x, y, z, 0);
}

void VisualizationContainer::PointToStructured(double p[3], int s[3]) {
	vtkIdType point = labels->FindPoint(p);
	double* p2 = labels->GetPoint(point);
	double c[3];
	
	labels->ComputeStructuredCoordinates(p2, s, c);
}