#include "VisualizationContainer.h"

#include <algorithm>

#include "MainWindow.h"

#include <vtkCallbackCommand.h>
#include <vtkCamera.h>
#include <vtkImageConnectivityFilter.h>
#include <vtkImageDilateErode3D.h>
#include <vtkImageOpenClose3D.h>
#include <vtkImageThreshold.h>
#include <vtkIdTypeArray.h>
#include <vtkImageCast.h>
#include <vtkImageChangeInformation.h>
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

#include "InteractionEnums.h"
#include "InteractionCallbacks.h"
#include "LabelColors.h"
#include "SegmentorMath.h"
#include "SliceView.h"
#include "VolumeView.h"
#include "Region.h"
#include "RegionCollection.h"
#include "RegionMetadataIO.h"

#include <vtkImageGradientMagnitude.h>

VisualizationContainer::VisualizationContainer(vtkRenderWindowInteractor* volumeInteractor, vtkRenderWindowInteractor* sliceInteractor, MainWindow* mainWindow) {
	data = nullptr;
	labels = nullptr;
	regions = new RegionCollection();
	currentRegion = nullptr;

	// Qt main window
	qtWindow = mainWindow;

	// Lookup table
	LabelColors::Initialize();
	labelColors = vtkSmartPointer<vtkLookupTable>::New();

	// Create rendering pipelines
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
	vtkSmartPointer<vtkCallbackCommand> volumeSelectLabelCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeSelectLabelCallback->SetCallback(InteractionCallbacks::VolumeSelectLabel);
	volumeSelectLabelCallback->SetClientData(this);
	volumeView->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::SelectLabelEvent, volumeSelectLabelCallback);

	vtkSmartPointer<vtkCallbackCommand> sliceSelectLabelCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceSelectLabelCallback->SetCallback(InteractionCallbacks::SliceSelectLabel);
	sliceSelectLabelCallback->SetClientData(this);
	sliceView->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::SelectLabelEvent, sliceSelectLabelCallback);

	// Paint
	vtkSmartPointer<vtkCallbackCommand> volumePaintCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumePaintCallback->SetCallback(InteractionCallbacks::VolumePaint);
	volumePaintCallback->SetClientData(this);
	volumeView->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::PaintEvent, volumePaintCallback);

	vtkSmartPointer<vtkCallbackCommand> slicePaintCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	slicePaintCallback->SetCallback(InteractionCallbacks::SlicePaint);
	slicePaintCallback->SetClientData(this);
	sliceView->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::PaintEvent, slicePaintCallback);

	// Erase
	vtkSmartPointer<vtkCallbackCommand> volumeEraseCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeEraseCallback->SetCallback(InteractionCallbacks::VolumeErase);
	volumeEraseCallback->SetClientData(this);
	volumeView->GetInteractorStyle()->AddObserver(vtkInteractorStyleVolume::EraseEvent, volumeEraseCallback);

	vtkSmartPointer<vtkCallbackCommand> sliceEraseCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceEraseCallback->SetCallback(InteractionCallbacks::SliceErase);
	sliceEraseCallback->SetClientData(this);
	sliceView->GetInteractorStyle()->AddObserver(vtkInteractorStyleSlice::EraseEvent, sliceEraseCallback);

	// Mouse move
	vtkSmartPointer<vtkCallbackCommand> volumeMouseMoveCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	volumeMouseMoveCallback->SetCallback(InteractionCallbacks::VolumeMouseMove);
	volumeMouseMoveCallback->SetClientData(this);
	volumeInteractor->AddObserver(vtkCommand::MouseMoveEvent, volumeMouseMoveCallback);

	vtkSmartPointer<vtkCallbackCommand> sliceMouseMoveCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	sliceMouseMoveCallback->SetCallback(InteractionCallbacks::SliceMouseMove);
	sliceMouseMoveCallback->SetClientData(this);
	sliceInteractor->AddObserver(vtkCommand::MouseMoveEvent, sliceMouseMoveCallback);

	// Window/level callback
	vtkSmartPointer<vtkCallbackCommand> windowLevelCallback = vtkSmartPointer<vtkCallbackCommand>::New();
	windowLevelCallback->SetCallback(InteractionCallbacks::WindowLevel);
	windowLevelCallback->SetClientData(this);
	sliceView->GetInteractorStyle()->AddObserver(vtkCommand::WindowLevelEvent, windowLevelCallback);
}

VisualizationContainer::~VisualizationContainer() {
	delete volumeView;
	delete sliceView;
	delete regions;
}

VisualizationContainer::FileErrorCode VisualizationContainer::OpenImageFile(const std::string& fileName) {
	// Get the file extension
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	// Load the data
	if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
		reader->SetFileName(fileName.c_str());
		reader->Update();

		SetImageData(reader->GetOutput());

		return Success;
	}
	else if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileName(fileName.c_str());
		reader->Update();

		SetImageData(reader->GetOutput());

		return Success;
	}
	else if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(fileName.c_str());
		reader->Update();

/*
		// For testing z scale
		vtkSmartPointer<vtkImageChangeInformation> info = vtkSmartPointer<vtkImageChangeInformation>::New();
		info->SetInputConnection(reader->GetOutputPort());
		info->SetSpacingScale(1, 1, 2);
		info->Update();
*/

/*
		// For testing gradient
		vtkSmartPointer<vtkImageGradientMagnitude> gradient = vtkSmartPointer<vtkImageGradientMagnitude>::New();
		gradient->SetInputConnection(reader->GetOutputPort());
		gradient->SetDimensionality(3);
		gradient->Update();
*/

		SetImageData(reader->GetOutput());
//		SetImageData(info->GetOutput());
//		SetImageData(gradient->GetOutput());

		return Success;
	}

	return WrongFileType;
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

	// Load the data
	if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileNames(names);
		reader->Update();

		SetImageData(reader->GetOutput());

		return Success;
	}
	
	return WrongFileType;
}

VisualizationContainer::FileErrorCode VisualizationContainer::OpenSegmentationFile(const std::string& fileName) {
	if (data == nullptr) return NoImageData;

	// Get the file extension
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);

	// Load the data
	if (extension == "nii") {
		vtkSmartPointer<vtkNIFTIImageReader> reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
		reader->SetFileName(fileName.c_str());

		vtkSmartPointer<vtkImageCast> cast = vtkSmartPointer<vtkImageCast>::New();
		cast->SetOutputScalarTypeToUnsignedShort();
		cast->SetInputConnection(reader->GetOutputPort());
		cast->Update();

		if (SetLabelData(cast->GetOutput())) {
			LoadRegionMetadata(fileName + ".json");

			qtWindow->updateRegions(regions);

			return Success;
		}
		else {
			return VolumeMismatch;
		}
	}
	else if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileName(fileName.c_str());

		vtkSmartPointer<vtkImageCast> cast = vtkSmartPointer<vtkImageCast>::New();
		cast->SetOutputScalarTypeToUnsignedShort();
		cast->SetInputConnection(reader->GetOutputPort());
		cast->Update();

		if (SetLabelData(cast->GetOutput())) {
			LoadRegionMetadata(fileName + ".json");

			qtWindow->updateRegions(regions);

			return Success;
		}
		else {
			return VolumeMismatch;
		}
	}
	else if (extension == "vti") {
		vtkSmartPointer<vtkXMLImageDataReader> reader = vtkSmartPointer<vtkXMLImageDataReader>::New();
		reader->SetFileName(fileName.c_str());

		vtkSmartPointer<vtkImageCast> cast = vtkSmartPointer<vtkImageCast>::New();
		cast->SetOutputScalarTypeToUnsignedShort();
		cast->SetInputConnection(reader->GetOutputPort());
		cast->Update();

		if (SetLabelData(cast->GetOutput())) {
			LoadRegionMetadata(fileName + ".json");

			qtWindow->updateRegions(regions);

			return Success;
		}
		else {
			return VolumeMismatch;
		}
	}

	return WrongFileType;
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

	// Load the data
	if (extension == "tif" || extension == "tiff") {
		vtkSmartPointer<vtkTIFFReader> reader = vtkSmartPointer<vtkTIFFReader>::New();
		reader->SetFileNames(names);

		vtkSmartPointer<vtkImageCast> cast = vtkSmartPointer<vtkImageCast>::New();
		cast->SetOutputScalarTypeToUnsignedShort();
		cast->SetInputConnection(reader->GetOutputPort());
		cast->Update();

		if (SetLabelData(cast->GetOutput())) {
			LoadRegionMetadata(fileNames[0] + ".json");

			qtWindow->updateRegions(regions);

			return Success;
		}
		else {
			return VolumeMismatch;
		}
	}

	return WrongFileType;
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
	
	return Success;
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
	
	vtkSmartPointer<vtkImageOpenClose3D> openClose = vtkSmartPointer<vtkImageOpenClose3D>::New();
	openClose->SetKernelSize(3, 3, 3);
	openClose->SetOpenValue(0);
	openClose->SetCloseValue(255);
	openClose->SetInputConnection(threshold->GetOutputPort());

	// Generate labels
	vtkSmartPointer<vtkImageConnectivityFilter> connectivity = vtkSmartPointer<vtkImageConnectivityFilter>::New();
	connectivity->SetScalarRange(255, 255);
	connectivity->SetLabelScalarTypeToUnsignedShort();
	connectivity->SetSizeRange(5, VTK_ID_MAX);
	connectivity->SetInputConnection(openClose->GetOutputPort());
	connectivity->Update();

	labels = connectivity->GetOutput();
	
	UpdateLabels();
	
	qtWindow->updateRegions(regions);

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

void VisualizationContainer::PickLabel(int x, int y, int z) {
	if (!labels) return;
	
	SetCurrentRegion(regions->Get(GetLabel(x, y, z)));
}

void VisualizationContainer::Paint(int x, int y, int z) {
	if (!labels || !currentRegion) return;

	currentRegion->UpdateExtent(x, y, z);

	SetLabel(x, y, z, currentRegion->GetLabel());

	currentRegion->SetModified(true);
	qtWindow->updateRegion(currentRegion);
}

void VisualizationContainer::Erase(int x, int y, int z) {	
	if (!labels || !currentRegion) return;

	// TODO: SHRINK EXTENT

	SetLabel(x, y, z, 0);

	currentRegion->SetModified(true);
	qtWindow->updateRegion(currentRegion);
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
	volumeView->SetCurrentRegion(currentRegion);
	sliceView->SetCurrentRegion(currentRegion); 

	qtWindow->selectRegion(region ? region->GetLabel() : 0);
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
	currentRegion->SetModified(true);

	// Remove region
	RemoveRegion(label);
}

void VisualizationContainer::SplitCurrentRegion(int numRegions) {
	if (!currentRegion) return;
	
	vtkSmartPointer<vtkTable> table = currentRegion->GetPointTable();

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
	currentRegion->SetModified(true);
	currentRegion->ComputeExtent();

	qtWindow->updateRegions(regions);

	labels->Modified();
	Render();
}

void VisualizationContainer::GrowCurrentRegion(int x, int y, int z) {
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

void VisualizationContainer::DilateCurrentRegion() {
	if (!currentRegion) return;

	DoDilateErode(true);
}

void VisualizationContainer::ErodeCurrentRegion() {
	if (!currentRegion) return;

	DoDilateErode(false);
}

void VisualizationContainer::DoDilateErode(bool doDilate) {
	int kernelSize = 3;

	unsigned short label = currentRegion->GetLabel();

	vtkSmartPointer<vtkImageDilateErode3D> dilate = vtkSmartPointer<vtkImageDilateErode3D>::New();
	dilate->SetDilateValue(doDilate ? label : 0);
	dilate->SetErodeValue(doDilate ? 0 : label);
	dilate->SetKernelSize(kernelSize, kernelSize, kernelSize);
	dilate->SetInputDataObject(labels);
	dilate->Update();

	int dataExtent[6];
	labels->GetExtent(dataExtent);

	const int* extent = currentRegion->GetExtent();

	int padExtent[6];
	padExtent[0] = std::max(dataExtent[0], extent[0] - kernelSize);
	padExtent[1] = std::min(dataExtent[1], extent[1] + kernelSize);
	padExtent[2] = std::max(dataExtent[2], extent[2] - kernelSize);
	padExtent[3] = std::min(dataExtent[3], extent[3] + kernelSize);
	padExtent[4] = std::max(dataExtent[4], extent[4] - kernelSize);
	padExtent[5] = std::min(dataExtent[5], extent[5] + kernelSize);

	for (int i = padExtent[0]; i <= padExtent[1]; i++) {
		for (int j = padExtent[2]; j <= padExtent[3]; j++) {
			for (int k = padExtent[4]; k <= padExtent[5]; k++) {
				unsigned short* dilateData = static_cast<unsigned short*>(dilate->GetOutput()->GetScalarPointer(i, j, k));
				unsigned short* labelData = static_cast<unsigned short*>(labels->GetScalarPointer(i, j, k));

				*labelData = *dilateData;
			}
		}
	}

	// Update current region
	currentRegion->SetModified(true);
	currentRegion->ComputeExtent();

	qtWindow->updateRegions(regions);

	labels->Modified();
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

	qtWindow->updateRegion(region);

	Render();
}

void VisualizationContainer::RemoveRegion(unsigned short label) {
	Region* region = regions->Get(label);

	if (region == currentRegion) SetCurrentRegion(nullptr);

	regions->Remove(label);

	Render();
}

void VisualizationContainer::HighlightRegion(unsigned short label) {
	Region* region = regions->Get(label);

	volumeView->HighlightRegion(region);
	
	Render();
}


void VisualizationContainer::SelectRegion(unsigned short label) {
	Region* region = regions->Get(label);

	SetCurrentRegion(region);

	volumeView->GetInteractorStyle()->GetInteractor()->FlyTo(volumeView->GetRenderer(), region->GetCenter());
}

void VisualizationContainer::SetWindowLevel(double window, double level) {
	qtWindow->setWindowLevel(window, level);
}

void VisualizationContainer::SliceUp() {
	SliceStep(1);
}

void VisualizationContainer::SliceDown() {
	SliceStep(-1);
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

void VisualizationContainer::Render() {
	volumeView->Render();
	sliceView->Render();
}

VolumeView* VisualizationContainer::GetVolumeView() {
	return volumeView;
}

SliceView* VisualizationContainer::GetSliceView() {
	return sliceView;
}

void VisualizationContainer::SetImageData(vtkImageData* imageData) {	
	regions->RemoveAll();
	qtWindow->updateRegions(regions);
	
	data = imageData;

	sliceView->Reset();
	volumeView->Reset();

	sliceView->SetImageData(data);	

	// Update GUI
	qtWindow->setWindowLevel(sliceView->GetWindow(), sliceView->GetLevel());

	Render();
}

bool VisualizationContainer::SetLabelData(vtkImageData* labelData) {
	// Check that volumes match
	int* dataDims = data->GetDimensions();
	int* labelDims = data->GetDimensions();

	for (int i = 0; i < 3; i++) {
		if (dataDims[i] != labelDims[i]) {
			std::cout << dataDims[i] << " != " << labelDims[i] << std::endl;
			return false;
		}
	}

	// Check that bounds match
	double* dataBounds = data->GetBounds();
	double* labelBounds = labelData->GetBounds();

	double epsilon = 1e-6;

	for (int i = 0; i < 6; i++) {
		if (std::abs(dataBounds[i] - labelBounds[i]) > epsilon) {
			std::cout << dataBounds[i] << " != " << labelBounds[i] << std::endl;
			return false;
		}
	}

	labels = labelData;

	UpdateLabels();

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
	regions->RemoveAll();

	for (int label = 1; label <= maxLabel; label++) {
		Region* region = new Region(label, labelColors->GetTableValue(label), labels);

		if (region->GetNumVoxels() > 0) {
			regions->Add(new Region(label, labelColors->GetTableValue(label), labels));
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

/*
void VisualizationContainer::PointToStructured(double p[3], int s[3]) {
	// Find closest non-zero cell
	vtkSmartPointer<vtkThreshold> threshold = vtkSmartPointer<vtkThreshold>::New();
	threshold->ThresholdByUpper(1);
	threshold->SetInputData(labels);
	threshold->Update();

	vtkSmartPointer<vtkCellLocator> locator = vtkSmartPointer<vtkCellLocator>::New();
	locator->SetDataSet(threshold->GetOutput());
	locator->BuildLocator();

	double p2[3];
	double d2;
	vtkIdType cellId;
	int subId;
	locator->FindClosestPoint(p, p2, cellId, subId, d2);

	// Convert to structured coordinates
	double pc[3];
	labels->ComputeStructuredCoordinates(p2, s, pc);
}
*/