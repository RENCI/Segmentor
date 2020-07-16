#include "VisualizationContainer.h"

#include <algorithm>
#include <set>

#include "MainWindow.h"

#include <vtkBillboardTextActor3D.h>
#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkImageConnectivityFilter.h>
#include <vtkImageDilateErode3D.h>
#include <vtkImageOpenClose3D.h>
#include <vtkImageThreshold.h>
#include <vtkIdTypeArray.h>
#include <vtkImageCast.h>
#include <vtkImageChangeInformation.h>
#include <vtkImageToImageStencil.h>
#include <vtkImageStencil.h>
#include <vtkIntArray.h>
#include <vtkKMeansStatistics.h>
#include <vtkLookupTable.h>
#include <vtkNIFTIImageReader.h>
#include <vtkNIFTIImageWriter.h>
#include <vtkPointSource.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkTIFFReader.h>
#include <vtkTIFFWriter.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>

#include <vtkInteractorStyleSlice.h>
#include <vtkInteractorStyleVolume.h>

#include "History.h"
#include "InteractionEnums.h"
#include "InteractionCallbacks.h"
#include "LabelColors.h"
#include "SegmentorMath.h"
#include "SliceView.h"
#include "VolumeView.h"
#include "Region.h"
#include "RegionCollection.h"
#include "RegionMetadataIO.h"

#include <vtkExtractVOI.h>
#include <vtkImageGradientMagnitude.h>

VisualizationContainer::VisualizationContainer(vtkRenderWindowInteractor* volumeInteractor, vtkRenderWindowInteractor* sliceInteractor, MainWindow* mainWindow) {
	data = nullptr;
	labels = nullptr;
	history = new History(10);
	regions = new RegionCollection();
	currentRegion = nullptr;	
	hoverLabel = 0;

	brushRadius = 1;

	// Qt main window
	qtWindow = mainWindow;

	// Lookup table
	LabelColors::Initialize();
	labelColors = vtkSmartPointer<vtkLookupTable>::New();

	// Create rendering pipelines
	sliceInteractor->SetDolly(0);

	volumeView = new VolumeView(volumeInteractor);
	sliceView = new SliceView(sliceInteractor, labelColors);

	// Set to navigation mode
	SetInteractionMode(NavigationMode);

	// Set to no filter
	SetFilterMode(FilterNone);

	// Callbacks

	// Camera
	vtkSmartPointer <vtkCallbackCommand> volumeCameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeCameraCallback->SetCallback(InteractionCallbacks::VolumeCameraChange);
	volumeCameraCallback->SetClientData(sliceView->GetRenderer());
	volumeView->GetRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, volumeCameraCallback);

	vtkSmartPointer <vtkCallbackCommand> sliceCameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceCameraCallback->SetCallback(InteractionCallbacks::SliceCameraChange);
	sliceCameraCallback->SetClientData(volumeView->GetRenderer());
	sliceView->GetRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, sliceCameraCallback);

	vtkSmartPointer <vtkCallbackCommand> cameraCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	cameraCallback->SetCallback(InteractionCallbacks::CameraChange);
	cameraCallback->SetClientData(this);
	sliceView->GetRenderer()->GetActiveCamera()->AddObserver(vtkCommand::ModifiedEvent, cameraCallback);

	// Char
	vtkSmartPointer<vtkCallbackCommand> onCharCallback = vtkSmartPointer<vtkCallbackCommand>::New(); 
	onCharCallback->SetCallback(InteractionCallbacks::OnChar);
	onCharCallback->SetClientData(this);
	volumeInteractor->AddObserver(vtkCommand::CharEvent, onCharCallback);
	sliceInteractor->AddObserver(vtkCommand::CharEvent, onCharCallback);

	// Label select
	vtkSmartPointer<vtkCallbackCommand> selectLabelCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	selectLabelCallback->SetCallback(InteractionCallbacks::SelectLabel);
	selectLabelCallback->SetClientData(this);
	volumeView->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::SelectLabelEvent, selectLabelCallback);
	sliceView->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::SelectLabelEvent, selectLabelCallback);

	// Paint
	vtkSmartPointer<vtkCallbackCommand> paintCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	paintCallback->SetCallback(InteractionCallbacks::Paint);
	paintCallback->SetClientData(this);
	volumeView->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::PaintEvent, paintCallback);
	sliceView->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::PaintEvent, paintCallback);

	// Overwrite
	vtkSmartPointer<vtkCallbackCommand> overwriteCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	overwriteCallback->SetCallback(InteractionCallbacks::Overwrite);
	overwriteCallback->SetClientData(this);
//	volumeView->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::OverwriteEvent, overwriteCallback);
	sliceView->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::OverwriteEvent, overwriteCallback);

	// Erase
	vtkSmartPointer<vtkCallbackCommand> eraseCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	eraseCallback->SetCallback(InteractionCallbacks::Erase);
	eraseCallback->SetClientData(this);
	volumeView->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::EraseEvent, eraseCallback);
	sliceView->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::EraseEvent, eraseCallback);

	// Mouse move
	vtkSmartPointer<vtkCallbackCommand> mouseMoveCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	mouseMoveCallback->SetCallback(InteractionCallbacks::MouseMove);
	mouseMoveCallback->SetClientData(this);
	volumeInteractor->AddObserver(vtkCommand::MouseMoveEvent, mouseMoveCallback);
	sliceInteractor->AddObserver(vtkCommand::MouseMoveEvent, mouseMoveCallback);

	// Window/level callback
	vtkSmartPointer<vtkCallbackCommand> windowLevelCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	windowLevelCallback->SetCallback(InteractionCallbacks::WindowLevel);
	windowLevelCallback->SetClientData(this);
	sliceView->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, windowLevelCallback);
}

VisualizationContainer::~VisualizationContainer() {
	delete volumeView;
	delete sliceView;
	delete history;
	delete regions;
}

VisualizationContainer::FileErrorCode VisualizationContainer::OpenImageFile(const std::string& fileName) {
	// Get the file extension
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	// For setting origin
	vtkSmartPointer<vtkImageChangeInformation> info = vtkSmartPointer<vtkImageChangeInformation>::New();
	info->SetOutputExtentStart(0, 0, 0);
	info->SetOutputOrigin(0, 0, 0);

	// Load the data
	if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
		reader->SetFileName(fileName.c_str());

		info->SetInputConnection(reader->GetOutputPort());
	}
	else if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileName(fileName.c_str());
		
		info->SetInputConnection(reader->GetOutputPort());
	}
	else if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(fileName.c_str());
		
		info->SetInputConnection(reader->GetOutputPort());
	}
	else {
		return WrongFileType;
	}

	info->Update();

	SetImageData(info->GetOutput());
	
	double x, y, z;
	data->GetSpacing(x, y, z);

	qtWindow->setVoxelSize(x, y, z);

	return Success;
}

VisualizationContainer::FileErrorCode VisualizationContainer::OpenImageStack(const std::vector<std::string>& fileNames) {
	// Get the file extension
	std::string extension = fileNames[0].substr(fileNames[0].find_last_of(".") + 1);

	// Set file names to pass to VTK
	vtkSmartPointer<vtkStringArray> names = vtkSmartPointer<vtkStringArray>::New();
	names->SetNumberOfValues(fileNames.size());

	for (int i = 0; i < fileNames.size(); i++) {
		names->SetValue(i, fileNames[i].c_str());
	}

	// For setting origin
	vtkSmartPointer<vtkImageChangeInformation> info = vtkSmartPointer<vtkImageChangeInformation>::New();
	info->SetOutputExtentStart(0, 0, 0);
	info->SetOutputOrigin(0, 0, 0);

	// Load the data
	if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileNames(names);

		info->SetInputConnection(reader->GetOutputPort());
	}
	else {
		return WrongFileType;
	}

	info->Update();

	SetImageData(info->GetOutput());

	double x, y, z;
	data->GetSpacing(x, y, z);

	qtWindow->setVoxelSize(x, y, z);

	return Success;
}

VisualizationContainer::FileErrorCode VisualizationContainer::OpenSegmentationFile(const std::string& fileName) {
	if (data == nullptr) return NoImageData;

	// Get the file extension
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	// For setting origin
	vtkSmartPointer<vtkImageChangeInformation> info = vtkSmartPointer<vtkImageChangeInformation>::New();
	info->SetOutputExtentStart(0, 0, 0);
	info->SetOutputOrigin(0, 0, 0);

	// Load the data
	if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
		reader->SetFileName(fileName.c_str());

		info->SetInputConnection(reader->GetOutputPort());
	}
	else if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileName(fileName.c_str());

		info->SetInputConnection(reader->GetOutputPort());
	}
	else if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(fileName.c_str());

		info->SetInputConnection(reader->GetOutputPort());
	}
	else {
		return WrongFileType;
	}

	// For casting images
	vtkSmartPointer<vtkImageCast> cast = vtkSmartPointer<vtkImageCast>::New();
	cast->SetOutputScalarTypeToUnsignedShort();
	cast->SetInputConnection(info->GetOutputPort());
	cast->Update();

	if (SetLabelData(cast->GetOutput())) {
		LoadRegionMetadata(fileName + ".json");

		qtWindow->updateRegions(regions);

		segmentationDataFileName = fileName;

		return Success;
	}
	else {
		return VolumeMismatch;
	}
}

VisualizationContainer::FileErrorCode VisualizationContainer::OpenSegmentationStack(const std::vector<std::string>& fileNames) {
	if (data == nullptr) return NoImageData;

	// Get the file extension
	std::string extension = fileNames[0].substr(fileNames[0].find_last_of(".") + 1);

	// Set file names to pass to VTK
	vtkSmartPointer<vtkStringArray> names = vtkSmartPointer<vtkStringArray>::New();
	names->SetNumberOfValues(fileNames.size());

	for (int i = 0; i < fileNames.size(); i++) {
		names->SetValue(i, fileNames[i].c_str());
	}

	// For setting origin
	vtkSmartPointer<vtkImageChangeInformation> info = vtkSmartPointer<vtkImageChangeInformation>::New();
	info->SetOutputExtentStart(0, 0, 0);
	info->SetOutputOrigin(0, 0, 0);

	// Load the data
	if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileNames(names);

		info->SetInputConnection(reader->GetOutputPort());
	}
	else {
		return WrongFileType;
	}

	// For casting images
	vtkSmartPointer<vtkImageCast> cast = vtkSmartPointer<vtkImageCast>::New();
	cast->SetOutputScalarTypeToUnsignedShort();
	cast->SetInputConnection(info->GetOutputPort());
	cast->Update();

	if (SetLabelData(cast->GetOutput())) {
		LoadRegionMetadata(fileNames[0] + ".json");

		qtWindow->updateRegions(regions);

		segmentationDataFileName = fileNames[0];

		return Success;
	}
	else {
		return VolumeMismatch;
	}
}

VisualizationContainer::FileErrorCode VisualizationContainer::SaveImageData(const std::string& fileName) {
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataWriter> writer = vtkSmartPointer<vtkXMLImageDataWriter>::New();
		writer->SetFileName(fileName.c_str());
		writer->SetInputDataObject(data);
		writer->Update();
	}
	else if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageWriter> writer = vtkSmartPointer<vtkNIFTIImageWriter>::New();
		writer->SetFileName(fileName.c_str());
		writer->SetInputDataObject(data);
		writer->Update();
	}
	else if (extension == "tif" || extension == "tiff") {
		std::string prefix = fileName.substr(0, fileName.find_last_of("."));

		vtkSmartPointer<vtkTIFFWriter> writer = vtkSmartPointer<vtkTIFFWriter>::New();
		writer->SetFilePrefix(prefix.c_str());
		writer->SetFilePattern("%s_%04d.tif");
		writer->SetFileDimensionality(2);
		writer->SetInputDataObject(data);
		writer->Update();
	}
	else {
		return WrongFileType;
	}

	return Success;
}

VisualizationContainer::FileErrorCode VisualizationContainer::SaveSegmentationData() {
	if (segmentationDataFileName.size() == 0) {
		return NoFileName;
	}
	else {
		return SaveSegmentationData(segmentationDataFileName);
	}
}

VisualizationContainer::FileErrorCode VisualizationContainer::SaveSegmentationData(const std::string& fileName) {
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
		return WrongFileType;
	}

	SaveRegionMetadata(fileName + ".json");

	segmentationDataFileName = fileName;
	
	return Success;
}

void VisualizationContainer::InitializeLabelData() {
	if (!data) return;

	// Threshold above maximum value
	vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
	threshold->ThresholdByUpper(0);
	threshold->SetInValue(0);
	threshold->SetOutValue(0);
	threshold->ReplaceInOn();
	threshold->ReplaceOutOn();
	threshold->SetOutputScalarTypeToUnsignedShort();
	threshold->SetInputDataObject(data);
	threshold->Update();

	labels = threshold->GetOutput();

	UpdateLabels();

	history->Clear();
	PushHistory();

	qtWindow->updateRegions(regions);
}

void VisualizationContainer::SegmentVolume() {
	if (!data) return;

	// Get Otsu threshold
	SegmentorMath::OtsuValues otsu = SegmentorMath::OtsuThreshold(data);

	// Filter
	vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
	threshold->ThresholdByUpper(otsu.threshold);
	threshold->SetInValue(255);
	threshold->SetOutValue(0);
	threshold->ReplaceInOn();
	threshold->ReplaceOutOn();
	threshold->SetOutputScalarTypeToUnsignedChar();
	threshold->SetInputDataObject(data);

/*	
	vtkSmartPointer<vtkImageOpenClose3D> openClose = vtkSmartPointer<vtkImageOpenClose3D>::New();
	openClose->SetKernelSize(10, 10, 10);
	openClose->SetOpenValue(0);
	openClose->SetCloseValue(255);
	openClose->SetInputConnection(threshold->GetOutputPort());
*/

	// Generate labels
	vtkSmartPointer<vtkImageConnectivityFilter> connectivity = vtkSmartPointer<vtkImageConnectivityFilter>::New();
	connectivity->SetScalarRange(255, 255);
	connectivity->SetLabelScalarTypeToUnsignedShort();
	connectivity->SetSizeRange(5, VTK_ID_MAX);
	//connectivity->SetInputConnection(openClose->GetOutputPort());
	connectivity->SetInputConnection(threshold->GetOutputPort());
	connectivity->Update();

	labels = connectivity->GetOutput();
	
	UpdateLabels();

/*
	std::vector<int> sizes;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		Region* region = regions->Get(it);

		std::cout << region->GetNumVoxels() << std::endl;

		sizes.push_back(region->GetNumVoxels());
	}

	std::cout << "************" << std::endl;

	std::sort(sizes.begin(), sizes.end());



	for (int i = 0; i < sizes.size(); i++) {
		std::cout << sizes[i] << std::endl;
	}


	int regionThreshold = 200;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		Region* region = regions->Get(it);

		int n = region->GetNumVoxels();

		if (n > regionThreshold) {
			int m = n / regionThreshold;

			SplitRegion(region, m);
		}
	}
*/

	qtWindow->updateRegions(regions);

	PushHistory();

	Render();
}

void VisualizationContainer::SetVoxelSize(double x, double y, double z) {
	// Get current bounds
	double bounds[6];
	data->GetBounds(bounds);

	// Get current focal point as a fraction of the volume
	vtkCamera* cam = sliceView->GetRenderer()->GetActiveCamera();
	double focus[3];

	cam->GetFocalPoint(focus);

	focus[0] = (focus[0] - bounds[0]) / (bounds[1] - bounds[0]);
	focus[1] = (focus[1] - bounds[2]) / (bounds[3] - bounds[2]);
	focus[2] = (focus[2] - bounds[4]) / (bounds[5] - bounds[4]);

	// Update voxel size
	if (data) {
		data->SetSpacing(x, y, z);

		sliceView->UpdateVoxelSize();
	}

	if (labels) {
		labels->SetSpacing(x, y, z);

		volumeView->UpdateVoxelSize(labels);
	}

	// Scale focal point to match in scaled volume
	data->GetBounds(bounds);

	focus[0] = bounds[0] + (bounds[1] - bounds[0]) * focus[0];
	focus[1] = bounds[2] + (bounds[3] - bounds[2]) * focus[1];
	focus[2] = bounds[4] + (bounds[5] - bounds[4]) * focus[2];

	cam->SetFocalPoint(focus);
	
	sliceView->GetRenderer()->ResetCameraClippingRange();
	volumeView->GetRenderer()->ResetCameraClippingRange();

	Render();
}

InteractionMode VisualizationContainer::GetInteractionMode() {
	return interactionMode;
}

void VisualizationContainer::SetInteractionMode(InteractionMode mode) {
	interactionMode = mode;

	sliceView->SetInteractionMode(interactionMode);
	volumeView->SetInteractionMode(interactionMode);

	Render();
}

void VisualizationContainer::ToggleInteractionMode() {
	SetInteractionMode(interactionMode == NavigationMode ? EditMode : NavigationMode);
}

FilterMode VisualizationContainer::GetFilterMode() {
	return filterMode;
}

void VisualizationContainer::SetFilterMode(FilterMode mode) {
	filterMode = mode;

	volumeView->SetFilterMode(filterMode);
	sliceView->SetFilterMode(filterMode);
}

void VisualizationContainer::PickLabel(double point[3]) {
	if (!labels) return;

	int ijk[3];
	PointToIndex(point, ijk);

	SetCurrentRegion(regions->Get(GetLabel(ijk[0], ijk[1], ijk[2])));
}

void VisualizationContainer::Paint(double point[3], bool overwrite) {
	int ijk[3];
	PointToIndex(point, ijk);

	Paint(ijk[0], ijk[1], ijk[2], overwrite);

	PushHistory();
}

void VisualizationContainer::Paint(int i, int j, int k, bool overwrite, bool useBrush) {
	if (!labels || !currentRegion) return;

	int extent[6];
	data->GetExtent(extent);

	int r = useBrush ? brushRadius - 1 : 0;
	int r2 = r * r;

	int i1 = std::max(extent[0], i - r);
	int i2 = std::min(extent[1], i + r);

	int j1 = std::max(extent[2], j - r);
	int j2 = std::min(extent[3], j + r);

	std::set<Region*> update;

	for (int m = i1; m <= i2; m++) {
		for (int n = j1; n <= j2; n++) {
			int md = i - m;
			int nd = j - n;
			int d2 = md * md + nd * nd;

			if (d2 > r2) continue;

			int value = SetLabel(m, n, k, currentRegion->GetLabel(), overwrite);

			if (value != -1) {
				currentRegion->UpdateExtent(m, n, k);

				update.insert(currentRegion);

				if (value != 0 && value != currentRegion->GetLabel()) {
					Region* previous = regions->Get(value);

					if (previous) {
						// TODO: SHRINK EXTENT

						previous->UpdateExtent(m, n, k);

						update.insert(previous);
					}
				}
			}
		}
	}

	for (auto region : update) {
		region->SetModified(true);
		qtWindow->updateRegion(region, regions);
	}
}

void VisualizationContainer::Erase(double point[3]) {
	int ijk[3];
	PointToIndex(point, ijk);

	Erase(ijk[0], ijk[1], ijk[2]);

	PushHistory();
}

void VisualizationContainer::Erase(int i, int j, int k, bool useBrush) {
	if (!labels || !currentRegion) return;

	int extent[6];
	data->GetExtent(extent);

	int r = useBrush ? brushRadius - 1 : 0;
	int r2 = r * r;

	int i1 = std::max(extent[0], i - r);
	int i2 = std::min(extent[1], i + r);

	int j1 = std::max(extent[2], j - r);
	int j2 = std::min(extent[3], j + r);

	bool update = false;

	for (int m = i1; m <= i2; m++) {
		for (int n = j1; n <= j2; n++) {
			int md = i - m;
			int nd = j - n;
			int d2 = md * md + nd * nd;

			if (d2 > r2) continue;

			int value = SetLabel(m, n, k, 0);

			if (value != -1) {				
				// TODO: SHRINK EXTENT

				update = true;
			}
		}
	}

	if (update) {
		currentRegion->SetModified(true);
		qtWindow->updateRegion(currentRegion, regions);
	}
}

void VisualizationContainer::MouseMove() {
	volumeView->SetShowProbe(false);
	sliceView->SetShowProbe(false);

	if (hoverLabel > 0) {
		regions->Get(hoverLabel)->ShowText(false);
	}

	hoverLabel = 0;
}

void VisualizationContainer::MouseMove(double point[3]) {
	int ijk[3];
	PointToIndex(point, ijk);
	IndexToPoint(ijk, point);

	// Update probes
	volumeView->SetProbePosition(point[0], point[1], point[2]);
	sliceView->SetProbePosition(point[0], point[1], point[2]);
	volumeView->SetShowProbe(true);
	sliceView->SetShowProbe(true);

	// Show label for region
	unsigned short label = GetLabel(ijk[0], ijk[1], ijk[2]);

	// Hide current hover label if different
	if (hoverLabel > 0 && hoverLabel != label) {
		if (regions->Has(hoverLabel)) {
			regions->Get(hoverLabel)->ShowText(false);
		}
	}
	
	// Show label
	if (label > 0) {		
		regions->Get(label)->ShowText(true);
	}

	hoverLabel = label;
}

/*
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
*/

void VisualizationContainer::SetCurrentRegion(Region* region) {
	currentRegion = region;
	volumeView->SetCurrentRegion(region);
	sliceView->SetCurrentRegion(region);

	qtWindow->selectRegion(region ? region->GetLabel() : 0);
}

void VisualizationContainer::CreateNewRegion(double point[3]) {
	int ijk[3];
	PointToIndex(point, ijk);
	int x = ijk[0];
	int y = ijk[1];
	int z = ijk[2];

	// Create new label
	unsigned short newLabel = regions->GetNewLabel();

	UpdateColors(newLabel);

	// Add first voxel
	unsigned short* labelData = static_cast<unsigned short*>(labels->GetScalarPointer(x, y, z));
	*labelData = newLabel;

	// Create new region
	Region* newRegion = new Region(newLabel, labelColors->GetTableValue(newLabel), labels);
	regions->Add(newRegion);
	volumeView->AddRegion(newRegion);
	sliceView->AddRegion(newRegion);

	newRegion->SetModified(true);

	qtWindow->updateRegions(regions);

	SetCurrentRegion(newRegion);

	labels->Modified();
	Render();
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
		currentRegion->SetModified(true);

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
				volumeView->AddRegion(newRegion);
				sliceView->AddRegion(newRegion);
				
				newRegion->SetModified(true);
			}			
		}

		qtWindow->updateRegions(regions);

		labels->Modified();
		Render();
	}

	PushHistory();
}

void VisualizationContainer::MergeWithCurrentRegion(double point[3]) {
	if (!currentRegion) return;

	int ijk[3];
	PointToIndex(point, ijk);
	int x = ijk[0];
	int y = ijk[1];
	int z = ijk[2];

	unsigned short currentLabel = currentRegion->GetLabel();

	// Get the region at the point
	unsigned short label = GetLabel(x, y, z);

	if (label == currentLabel) return;

	Region* region = regions->Get(label);

	if (!region) return;

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
	for (int i = 0; i < 3; i++) {
		int i1 = 2 * i;
		int i2 = i1 + 1;
		newExtent[i1] = std::min(extent[i1], currentExtent[i1]);
		newExtent[i2] = std::max(extent[i2], currentExtent[i2]);
	}

	currentRegion->SetExtent(newExtent);
	currentRegion->SetModified(true);

	// Remove region
	RemoveRegion(label);

	PushHistory();
}

void VisualizationContainer::SplitCurrentRegion(int numRegions) {
	if (!currentRegion) return;
	
	SplitRegion2(currentRegion, numRegions);

	PushHistory();

	Render();
}

void VisualizationContainer::SplitRegion(Region* region, int numRegions) {
	vtkSmartPointer<vtkTable> table = region->GetPointTable();

	vtkSmartPointer<vtkKMeansStatistics> kMeans = vtkSmartPointer<vtkKMeansStatistics>::New();

	kMeans->SetInputData(vtkStatisticsAlgorithm::INPUT_DATA, table);
	kMeans->SetColumnStatus(table->GetColumnName(0), 1);
	kMeans->SetColumnStatus(table->GetColumnName(1), 1);
	kMeans->SetColumnStatus(table->GetColumnName(2), 1);
	kMeans->RequestSelectedColumns();
	kMeans->SetAssessOption(true);
	kMeans->SetDefaultNumberOfClusters(numRegions);
	kMeans->Update();

	vtkTable* output = kMeans->GetOutput();

	std::vector<std::vector<int>> newRegionRows(numRegions - 1);

	for (int i = 0; i < output->GetNumberOfRows(); i++) {
		int c = output->GetValue(i, output->GetNumberOfColumns() - 1).ToInt();

		if (c > 0) newRegionRows[c - 1].push_back(i);
	}

	for (int i = 1; i < numRegions; i++) {
		// Get label for new region
		unsigned short newLabel = regions->GetNewLabel();

		// Update label data
		for (int j = 0; j < (int)newRegionRows[i - 1].size(); j++) {
			int row = newRegionRows[i - 1][j];

			int x = output->GetValue(row, 0).ToInt();
			int y = output->GetValue(row, 1).ToInt();
			int z = output->GetValue(row, 2).ToInt();

			unsigned short* labelData = static_cast<unsigned short*>(labels->GetScalarPointer(x, y, z));

			*labelData = newLabel;
		}

		UpdateColors(newLabel);

		// Create new region
		Region* newRegion = new Region(newLabel, labelColors->GetTableValue(newLabel), labels);
		regions->Add(newRegion);
		volumeView->AddRegion(newRegion);
		sliceView->AddRegion(newRegion);

		newRegion->SetModified(true);
	}

	// Update current region
	region->SetModified(true);
	region->ComputeExtent();

	qtWindow->updateRegions(regions);

	labels->Modified();
}

void VisualizationContainer::SplitRegion2(Region* region, int numRegions) {
	// Region label
	unsigned short label = region->GetLabel();

	// Extract data for region
	int extent[6];
	region->GetExtent(extent);

	vtkSmartPointer<vtkExtractVOI> voi = vtkSmartPointer<vtkExtractVOI>::New();
	voi->SetVOI(extent[0], extent[1], extent[2], extent[3], extent[4], extent[5]);
	voi->SetInputDataObject(data);
	voi->Update();

	// Data range
	double range[2];
	voi->GetOutput()->GetScalarRange(range);

	// Stencil for current region
	vtkSmartPointer<vtkImageToImageStencil> stencil = vtkSmartPointer<vtkImageToImageStencil>::New();
	stencil->ThresholdBetween(label, label);
	stencil->SetInputConnection(region->GetOutput());

	// Threshold
	vtkSmartPointer<vtkImageThreshold> threshold = vtkSmartPointer<vtkImageThreshold>::New();
	threshold->ReplaceInOn();
	threshold->ReplaceOutOn();
	threshold->SetInValue(1);
	threshold->SetOutValue(0);
	threshold->SetInputConnection(voi->GetOutputPort());

	// Connectivity
	int minSize = 3;

	vtkSmartPointer<vtkImageConnectivityFilter> connectivity = vtkSmartPointer<vtkImageConnectivityFilter>::New();
	connectivity->SetLabelScalarTypeToUnsignedShort();
	connectivity->SetLabelModeToSizeRank();
	connectivity->GenerateRegionExtentsOff();
	connectivity->SetScalarRange(1, 1);
	connectivity->SetSizeRange(minSize, VTK_ID_MAX);
	connectivity->SetStencilConnection(stencil->GetOutputPort());
	connectivity->SetExtractionModeToAllRegions();
	connectivity->SetInputConnection(threshold->GetOutputPort());

	// Threshold until we have the right number of regions
/*
	double step = 1;

	int closestCount = 0;
	double closestThreshold = 0.0;

	for (double t = range[0]; t <= range[1]; t += step) {
		threshold->ThresholdByUpper(t);
		connectivity->Update();

		int numComponents = connectivity->GetNumberOfExtractedRegions();

		if (numComponents > closestCount && numComponents <= numRegions) {
			closestCount = numComponents;
			closestThreshold = t;
		}

		if (numComponents >= numRegions) break;
	}
*/
	// XXX: TRY ALL THRESHOLDS, KEEP ONE WITH LARGEST REGIONS FOR CORRECT NUMBER?
	double step = 1;

	int closestCount = 0;
	double closestThreshold = 0.0;
	int maxMinSize = VTK_INT_MIN;

	for (double t = range[0]; t <= range[1]; t += step) {
		threshold->ThresholdByUpper(t);
		connectivity->Update();

		int numComponents = connectivity->GetNumberOfExtractedRegions();

		if (numComponents > closestCount && numComponents <= numRegions) {
			// Use this threshold
			vtkIdTypeArray* sizes = connectivity->GetExtractedRegionSizes();

			vtkIdType minSize = VTK_INT_MAX;
			for (vtkIdType i = 0; i < sizes->GetSize(); i++) {
				vtkIdType size = sizes->GetValue(i);

				if (size < minSize) minSize = size;
			}

			maxMinSize = minSize;

			closestCount = numComponents;
			closestThreshold = t;
		}
		else if (numComponents == closestCount) {
			// Check min size
			vtkIdTypeArray* sizes = connectivity->GetExtractedRegionSizes();

			vtkIdType minSize = VTK_INT_MAX;
			for (vtkIdType i = 0; i < sizes->GetSize(); i++) {
				vtkIdType size = sizes->GetValue(i);

				if (size < minSize) minSize = size;
			}

			if (minSize > maxMinSize) {
				maxMinSize = minSize;

				closestCount = numComponents;
				closestThreshold = t;
			}
		}
	}

	threshold->ThresholdByUpper(closestThreshold);
	connectivity->Update();

	int numComponents = connectivity->GetNumberOfExtractedRegions();

	if (numComponents <= 1) {
		return;
	}

	vtkIdTypeArray* componentLabels = connectivity->GetExtractedRegionLabels();
	vtkImageData* connectivityOutput = connectivity->GetOutput();	

	for (double t = closestThreshold - 1; t >= range[0]; t -= step) {
		// Assign voxels by growing each component
		std::vector<vtkSmartPointer<vtkImageConnectivityFilter>> grow;

		for (int i = 0; i < numComponents; i++) {
			// Label from the connectivity filter
			unsigned short componentLabel = (unsigned short)componentLabels->GetTuple1(i);

			// Dilate
			int kernelSize = 3;

			vtkSmartPointer<vtkImageDilateErode3D> dilate = vtkSmartPointer<vtkImageDilateErode3D>::New();
			dilate->SetDilateValue(componentLabel);
			dilate->SetErodeValue(0);
			dilate->SetKernelSize(kernelSize, kernelSize, kernelSize);
			dilate->SetInputDataObject(connectivityOutput);

			// Stencil based on dilated component and original region
			vtkSmartPointer<vtkImageStencil> dilateStencil = vtkSmartPointer<vtkImageStencil>::New();
			dilateStencil->SetStencilConnection(stencil->GetOutputPort());
			dilateStencil->SetInputConnection(dilate->GetOutputPort());

			vtkSmartPointer<vtkImageToImageStencil> finalStencil = vtkSmartPointer<vtkImageToImageStencil>::New();
			finalStencil->ThresholdBetween(componentLabel, componentLabel);
			finalStencil->SetInputConnection(dilateStencil->GetOutputPort());

			// Grow this region
			vtkSmartPointer<vtkImageConnectivityFilter> regionGrow = vtkSmartPointer<vtkImageConnectivityFilter>::New();
			regionGrow->SetStencilConnection(finalStencil->GetOutputPort());
			regionGrow->SetScalarRange(t, range[1]);
			regionGrow->SetLabelScalarTypeToUnsignedShort();
			regionGrow->SetLabelModeToConstantValue();
			regionGrow->SetLabelConstantValue(componentLabel);
			regionGrow->GenerateRegionExtentsOn();
			regionGrow->SetInputConnection(voi->GetOutputPort());
			regionGrow->Update();

			grow.push_back(regionGrow);
		}

		// Assign voxels
		// XXX: Could potentially speed things up here by looking at component extents?
		for (int i = extent[0]; i <= extent[1]; i++) {
			for (int j = extent[2]; j <= extent[3]; j++) {
				for (int k = extent[4]; k <= extent[5]; k++) {
					// Current label
					unsigned short* currentLabel = static_cast<unsigned short*>(connectivityOutput->GetScalarPointer(i, j, k));

					if (*currentLabel != 0) continue;

					std::vector<unsigned short> labels;
					
					for (int c = 0; c < numComponents; c++) {
						// Label for this component
						unsigned short componentLabel = (unsigned short)componentLabels->GetTuple1(c);

						// Check voxel in the region grow for this component
						unsigned short growLabel = *static_cast<unsigned short*>(grow[c]->GetOutput()->GetScalarPointer(i, j, k));

						if (growLabel == componentLabel) {
							labels.push_back(componentLabel);
						}
					}

					if (labels.size() == 1) {
						*currentLabel = labels[0];
					}
				}
			}
		}

		connectivityOutput->Modified();
	}

	// Use current region for first component
	unsigned short componentLabel = (unsigned short)componentLabels->GetTuple1(0);

	for (int i = extent[0]; i <= extent[1]; i++) {
		for (int j = extent[2]; j <= extent[3]; j++) {
			for (int k = extent[4]; k <= extent[5]; k++) {
				unsigned short* labelData = static_cast<unsigned short*>(labels->GetScalarPointer(i, j, k));
				unsigned short* connectivityData = static_cast<unsigned short*>(connectivityOutput->GetScalarPointer(i, j, k));

				if (*labelData == label) {
					*labelData = *connectivityData == componentLabel ? label : 0;
				}				
			}
		}
	}

	currentRegion->ComputeExtent();
	currentRegion->SetModified(true);

	// Create new regions for other components
	for (int i = 1; i < numComponents; i++) {
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
		volumeView->AddRegion(newRegion);
		sliceView->AddRegion(newRegion);

		newRegion->SetModified(true);
	}

	qtWindow->updateRegions(regions);

	labels->Modified();
}

void VisualizationContainer::GrowCurrentRegion(double point[3]) {
	if (!currentRegion) return;

	int ijk[3];
	PointToIndex(point, ijk);
	int x = ijk[0];
	int y = ijk[1];
	int z = ijk[2];
	
	double value = GetValue(x, y, z);
	double label = GetLabel(x, y, z);

	bool grow = label == 0;

	double growValue = 255;

	vtkSmartPointer<vtkPointSource> seed = vtkSmartPointer<vtkPointSource>::New();
	seed->SetNumberOfPoints(1);
	seed->SetCenter(point);
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

	// Extent for extracting
	int extractExtent[6];
	data->GetExtent(extractExtent);
	extractExtent[4] = extractExtent[5] = z;

	// Stencil for growing
	vtkSmartPointer<vtkImageToImageStencil> stencil;

	if (grow) {
		// Compute distance to region
		int distance = round(currentRegion->GetXYDistance(x, y, z));

		if (distance < 0) {
			extractExtent[0] = extractExtent[1] = x;
			extractExtent[2] = extractExtent[3] = y;
		}
		else {
			const int* regionExtent = currentRegion->GetExtent();

			extractExtent[0] = std::max(extractExtent[0], regionExtent[0] - distance);
			extractExtent[1] = std::min(extractExtent[1], regionExtent[1] + distance);
			extractExtent[2] = std::max(extractExtent[2], regionExtent[2] - distance);
			extractExtent[3] = std::min(extractExtent[3], regionExtent[3] + distance);
		}

		// Extract z slice around region with space to dilate
		vtkSmartPointer<vtkExtractVOI> voi = vtkSmartPointer<vtkExtractVOI>::New();
		voi->SetVOI(extractExtent[0], extractExtent[1], extractExtent[2], extractExtent[3], extractExtent[4], extractExtent[5]);
		voi->SetInputDataObject(labels);

		// Dilate
		int kernelSize = distance * 2 + 1;

		unsigned short label = currentRegion->GetLabel();

		vtkSmartPointer<vtkImageDilateErode3D> dilate = vtkSmartPointer<vtkImageDilateErode3D>::New();
		dilate->SetDilateValue(label);
		dilate->SetErodeValue(0);
		dilate->SetKernelSize(kernelSize, kernelSize, 1);
		dilate->SetInputConnection(voi->GetOutputPort());

		// Stencil
		stencil = vtkSmartPointer<vtkImageToImageStencil>::New();
		stencil->ThresholdBetween(label, label);
		stencil->SetInputConnection(dilate->GetOutputPort());
	}

	// Extract z slice around region
	vtkSmartPointer<vtkExtractVOI> extract = vtkSmartPointer<vtkExtractVOI>::New();
	extract->SetVOI(extractExtent[0], extractExtent[1], extractExtent[2], extractExtent[3], extractExtent[4], extractExtent[5]);
	extract->SetInputDataObject(data);

	// Grow region
	vtkSmartPointer<vtkImageConnectivityFilter> regionGrow = vtkSmartPointer<vtkImageConnectivityFilter>::New();
	regionGrow->SetExtractionModeToSeededRegions();
	regionGrow->SetSeedConnection(seed->GetOutputPort());
	if (grow) regionGrow->SetStencilConnection(stencil->GetOutputPort());
	regionGrow->SetScalarRange(min, max);
	regionGrow->SetLabelModeToConstantValue();
	regionGrow->SetLabelConstantValue(growValue);
	regionGrow->GenerateRegionExtentsOn();
	//regionGrow->SetInputDataObject(data);
	regionGrow->SetInputConnection(extract->GetOutputPort());
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
						Paint(i, j, k, false, false);
					}
					else {
						Erase(i, j, k, false);
					}
				}
			}
		}
	}

	labels->Modified();

	PushHistory();

	Render();
}

Region* VisualizationContainer::SetRegionDone(unsigned short label, bool done) {
	Region* region = regions->Get(label);

	if (!region) return nullptr;

	if (done) {
		// Test to make sure the region is contiguous
		vtkSmartPointer<vtkImageConnectivityFilter> connectivity = vtkSmartPointer<vtkImageConnectivityFilter>::New();
		connectivity->SetScalarRange(label, label);
		connectivity->SetInputDataObject(labels);
		connectivity->Update();

		if (connectivity->GetNumberOfExtractedRegions() > 1) {
			return region;
		}
	}

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

	qtWindow->updateRegion(region, regions);

	Render();

	return region;
}

void VisualizationContainer::RemoveRegion(unsigned short label) {
	Region* region = regions->Get(label);

	if (region == currentRegion) SetCurrentRegion(nullptr);

	regions->Remove(label);

	qtWindow->updateRegions(regions);

	Render();

	PushHistory();
}

void VisualizationContainer::HighlightRegion(unsigned short label) {
	Region* region = regions->Get(label);

	volumeView->HighlightRegion(region);
	
	Render();
}


void VisualizationContainer::SelectRegion(unsigned short label) {
	Region* region = regions->Get(label);

	SetCurrentRegion(region);

	volumeView->GetInteractorStyle()->FlyTo(region->GetCenter());
}

void VisualizationContainer::SetWindowLevel(double window, double level) {
	qtWindow->setWindowLevel(window, level);
}

void VisualizationContainer::SliceUp() {
	double* spacing = data->GetSpacing();
	SliceStep(spacing[2]);
}

void VisualizationContainer::SliceDown() {
	double* spacing = data->GetSpacing();
	SliceStep(-spacing[2]);
}

void VisualizationContainer::SliceStep(double amount) {
	vtkCamera* cam = sliceView->GetRenderer()->GetActiveCamera();

	double distance = cam->GetDistance();

	distance += amount;

	cam->SetDistance(distance);

	Render();
}

void VisualizationContainer::SetFocalPoint(double x, double y, double z) {
	qtWindow->setSlicePosition(x, y, z);
}

void VisualizationContainer::SetBrushRadius(int radius) {
	brushRadius = radius;
	sliceView->SetBrushRadius(radius);
	volumeView->SetBrushRadius(radius);

	Render();
}

void VisualizationContainer::Render() {
	volumeView->Render();
	sliceView->Render();
}

void VisualizationContainer::Undo() {
	history->Undo(labels, regions);
	volumeView->SetRegions(labels, regions);
	sliceView->SetSegmentationData(labels, regions);
	qtWindow->updateRegions(regions);
	Render();
}

void VisualizationContainer::Redo() {
	history->Redo(labels, regions);
	volumeView->SetRegions(labels, regions);
	sliceView->SetSegmentationData(labels, regions);
	qtWindow->updateRegions(regions);
	Render();
}

VolumeView* VisualizationContainer::GetVolumeView() {
	return volumeView;
}

SliceView* VisualizationContainer::GetSliceView() {
	return sliceView;
}

void VisualizationContainer::SetImageData(vtkImageData* imageData) {	
/*
	vtkSmartPointer<vtkImageGradientMagnitude> gradient = vtkSmartPointer<vtkImageGradientMagnitude>::New();
	gradient->SetInputData(imageData);
	gradient->SetDimensionality(2);
	gradient->Update();
*/

	regions->RemoveAll();
	qtWindow->updateRegions(regions);
	
	data = imageData;

	sliceView->Reset();
	volumeView->Reset();

	sliceView->SetImageData(data);	
	volumeView->SetImageData(data);

	// Update GUI
	qtWindow->setWindowLevel(sliceView->GetWindow(), sliceView->GetLevel());

	InitializeLabelData();

	Render();
}

bool VisualizationContainer::SetLabelData(vtkImageData* labelData) {
	// Check that volumes match
	int* dataDims = data->GetDimensions();
	int* labelDims = labelData->GetDimensions();

	for (int i = 0; i < 3; i++) {
		std::cout << dataDims[i] << ", " << labelDims[i] << std::endl;


		if (dataDims[i] != labelDims[i]) {
			std::cout << dataDims[i] << " != " << labelDims[i] << std::endl;
			return false;
		}
	}

	labelData->SetSpacing(data->GetSpacing());

	// Check that bounds match
	double* dataBounds = data->GetBounds();
	double* labelBounds = labelData->GetBounds();

	double epsilon = 1e-6;

	for (int i = 0; i < 6; i++) {

		std::cout << dataBounds[i] << ", " << labelBounds[i] << std::endl;


		if (std::abs(dataBounds[i] - labelBounds[i]) > epsilon) {
			std::cout << dataBounds[i] << " != " << labelBounds[i] << std::endl;
			return false;
		}
	}

	labels = labelData;

	UpdateLabels();

	history->Clear();
	PushHistory();

	return true;
}

void VisualizationContainer::UpdateLabels() {
	UpdateColors();
	ExtractRegions();

	volumeView->SetRegions(labels, regions);
	sliceView->SetSegmentationData(labels, regions);
}

void VisualizationContainer::UpdateColors(unsigned short label) {
	if (label >= labelColors->GetNumberOfTableValues()) {
		labelColors->SetNumberOfTableValues(label + 1);
		labelColors->SetRange(0, label);
	}

	double* c = LabelColors::GetColor(label - 1);
	labelColors->SetTableValue(label, c[0], c[1], c[2]);
	labelColors->Build();
}

void VisualizationContainer::UpdateColors() {
	// Get label info
	int maxLabel = labels->GetScalarRange()[1];

	// Label colors
	labelColors->SetNumberOfTableValues(maxLabel + 1);
	labelColors->SetRange(0, maxLabel);
	labelColors->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
	for (int i = 1; i <= maxLabel; i++) {
		double* c = LabelColors::GetColor(i - 1);
		labelColors->SetTableValue(i, c[0], c[1], c[2]);
	}
	labelColors->Build();
}

void VisualizationContainer::ExtractRegions() {
	// Get label info
	int maxLabel = labels->GetScalarRange()[1];

	// Clear current regions

	// XXX: THIS IS CLEARING ALL VOXELS IN THE LABEL DATA
	regions->RemoveAll();

	for (int label = 1; label <= maxLabel; label++) {
		Region* region = new Region(label, labelColors->GetTableValue(label), labels);

		if (region->GetNumVoxels() > 0) {
			regions->Add(region);
		}		
		else {
			delete region;
		}
	}

	currentRegion = nullptr;
}

void VisualizationContainer::LoadRegionMetadata(std::string fileName) {
	std::vector<RegionMetadataIO::Region> metadata = RegionMetadataIO::Read(fileName);

	for (int i = 0; i < (int)metadata.size(); i++) {
		Region* region = regions->Get(metadata[i].label);

		if (region) {
			region->SetModified(metadata[i].modified);
			region->SetDone(metadata[i].done);
		}
	}
}

void VisualizationContainer::SaveRegionMetadata(std::string fileName) {
	std::vector<RegionMetadataIO::Region> metadata;

	for (RegionCollection::Iterator it = regions->Begin(); it != regions->End(); it++) {
		RegionMetadataIO::Region regionMetadata;

		Region* region = regions->Get(it);

		regionMetadata.label = region->GetLabel();
		regionMetadata.modified = region->GetModified();
		regionMetadata.done = region->GetDone();

		metadata.push_back(regionMetadata);
	}

	RegionMetadataIO::Write(fileName, metadata);
}

int VisualizationContainer::SetLabel(int x, int y, int z, unsigned short label, bool overwrite) {
	unsigned short* p = static_cast<unsigned short*>(labels->GetScalarPointer(x, y, z));

	unsigned short old = *p;

	// Restrict painting to no label and erasing to current label, unless overwrite is true
	if (overwrite ||
		(label != 0 && old == 0) ||
		(label == 0 && old == currentRegion->GetLabel())) {
		*p = label;
		labels->Modified();

		return old;
	}

	return -1;
}

unsigned short VisualizationContainer::GetLabel(int x, int y, int z) {
	return *(static_cast<unsigned short*>(labels->GetScalarPointer(x, y, z)));
}

double VisualizationContainer::GetValue(int x, int y, int z) {
	return data->GetScalarComponentAsDouble(x, y, z, 0);
}

void VisualizationContainer::PointToIndex(double point[3], int ijk[3]) {
	double pc[3];
	data->ComputeStructuredCoordinates(point, ijk, pc);

	if (pc[0] > 0.5) ijk[0]++;
	if (pc[1] > 0.5) ijk[1]++;
	if (pc[2] > 0.5) ijk[2]++;
}

void VisualizationContainer::IndexToPoint(int ijk[3], double point[3]) {
	vtkIdType id = data->ComputePointId(ijk);

	data->GetPoint(id, point);
}

void VisualizationContainer::PushHistory() {
	history->Push(labels, regions);
}