# Segmentor
Segmentation tool for 3D tissue cleared microscopy images of nuclei.

## Data Formats:

The following data formats are supported for loading and saving image and segmentation data:

1. NIfTI (Neuroimaging Informatics Technology Initiative)
2. VTI (VTK'S XML Image Data format)
3. Single multipage TIFF file (loading only)
4. Stack of TIFFs

When loading data as a stack of TIFFs, it is assumed that all files in the directory belong to the stack, and that the file names are in alphanumeric order (e.g. image_001.tif, image_002.tif, etc.).

## Image Data Loading:

To load image data in NIfTI, VTI, or multipage TIFF format select `File->Open Image File...` and choose the desired file.
To load image data in stack of TIFFs format, select `File->Open Image Stack...` and choose a single file from the directory in which the image stack is stored (the other files will be loaded automatically).

After loading data, the 2D slice view on the right will display the current slice.

## Creating an Initial Segmentation:

To generate an initial segmentation, select `Analyze->Segment Volume...`. The segmentation method currently consists of an Otsu thresholding followed by morphological followed by calculating the connected components.

Once the initial segmentation is generated, the 3D view on the left will display surfaces of the segmentations, and the 2D slice view will show 2D overlays and outlines of the segmented regions on the current slice.

## Segmentation Loading:

To load segmentation data in NIfTI, VTI, or multipage TIFF format select `File->Open Segmentation File...` and choose the desired file.
To load segmentation data in stack of TIFFs format, select `File->Open Segmentation Stack...` and choose a single file from the directory in which the segmentation stack is stored (the other files will be loaded automatically).

## Image Data Saving:

To save the image (useful for file format conversion) select `File->Save Image Data...`. The format will be based on the file extension  provided `(.nii, .vti, or .tif)`. 

## Segmentation Saving:

To overwrite the most recently loaded or saved segmentation file, select `File->Save Segmentation Data`. To save as a new segmentation file, select `File->Save Segmentation Data As...`. The format will be based on the file extension  provided `(.nii, .vti, or .tif)`. 

## Controls:

Switch between **Navigation** and **Edit** modes by pressing the space bar, or with the `Mode` (N or E) GUI control.

### Navigation mode:

* Translate: `Middle-click and drag`
* Zoom: `Right-click and drag`
* Rotate: `Left-click and drag`
* Rotate around view up vector: `Alt + left-click and drag`
* Rotate around view right vector: `Ctrl + left-click and drag`

### Edit mode:

* Paint (with current label): `Left-click or left-click and drag`
* Overwrite (enables painting over other labels): `Alt + left-click or left-click and drag`
* Erase (voxels with current label) : `Right-click or right-click and drag`

### Both modes:

#### General 

* Select current label: `Middle-click or Ctrl + left-click` on segmentation region
* Move clipping plane: `Ctrl + right-click and drag` or `up arrow` and `down arrow`
* Zoom to point: `f`
****
* Relabel current region (applies new labels if a region has been manually split): `u`
* Merge with current region (applied to region under the cursor): `m`
* Split current region: `2-9` (for number of split regions)
****
* Region grow or shrink (based on voxel intensity of current cursor position): `g` (2D slice view only)
****
* Toggle slice plane filtering: `p`
* Toggle neighbor filtering: `k`
* Toggle region filtering: `l`

#### 2D Visualization:

* Window/level: `Shift + left-click and drag` (2D slice view only)
* Set window/level to full intensity range of current slice: `=`
* Set window/level to partial intensity range of current slice (higher contrast): `-`
****
* Toggle voxel overlay: `q`
* Toggle region outline: `e`
****
* Adjust overlay opacity: `right arrow` or `left arrow`

#### 3D Visualization

* Toggle surface smoothing: `s`
* Toggle surface normals: `n`
* Toggle slice plane visualization: `o`
****
* Adjust neighbor opacity (in neighbor filter mode): `Shift + right arrow` or `Shift + left arrow`

## Region Table:

* Mouse-over cells in the *Id*, *Color*, or *Size* columns to highlight that region in the 3D view
* Click cells in the *Id*, *Color*, or *Size* columns to select and fly to that region
* Regions edited by the user are indicated in the *Modified* column
* Click the check box in the *Done* column to indicate regions for which editing is complete, which will render them as grey in the 3D and 2D slice views
* Click the button in the *Remove* column to remove that region
