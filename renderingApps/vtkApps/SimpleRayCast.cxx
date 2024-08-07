#include <vtkCamera.h>
#include <vtkColorTransferFunction.h>
#include <vtkFixedPointVolumeRayCastMapper.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPiecewiseFunction.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkStructuredPointsReader.h>
#include <vtkImageData.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>

#include <fstream>

int main(int argc, char* argv[])
{
  if (argc < 5)
  {
    std::cout << "Usage: " << argv[0] << " <filename> x y z" << std::endl;
    return EXIT_FAILURE;
  }

  // This is a simple volume rendering example that
  // uses a vtkFixedPointVolumeRayCastMapper

  // Create the standard renderer, render window
  // and interactor
  vtkNew<vtkNamedColors> colors;

  vtkNew<vtkRenderer> ren1;

  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren1);

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  // Create the reader for the data
  //vtkNew<vtkStructuredPointsReader> reader;
  //vtkSmartPointer<vtkImageData> input;
  //reader->SetFileName(argv[1]);
  
  int dim[3] = {atoi(argv[2]), atoi(argv[3]), atoi(argv[4])};
  double spc[3] = {1.0, 1.0, 1.0};
  vtkNew<vtkImageData> img;
  img->SetDimensions(dim);
  img->AllocateScalars(VTK_INT, 1);
  img->SetSpacing(spc);
  
  std::cout <<"Loading volume with dim "<<dim[0]<<" "<<dim[1]<<" "<<dim[2]<<"\n";
  std::fstream file;
  file.open(argv[1], std::fstream::in | std::fstream::binary);
  for (int x = 0; x < dim[0]; ++x)
    for (int y = 0; y < dim[1]; ++y)
      for (int z = 0; z < dim[2]; ++z)
      {
        float buff;
        file.read((char*)(&buff), sizeof(buff));
        img->SetScalarComponentFromDouble(x, y, z, 0, buff);
      }

  file.close();
  std::cout<< "End load\n";
  
  // Create transfer mapping scalar value to opacity
  vtkNew<vtkPiecewiseFunction> opacityTransferFunction;
  opacityTransferFunction->AddPoint(0, 0.0);
  opacityTransferFunction->AddPoint(100, 0.1);

  // Create transfer mapping scalar value to color
  vtkNew<vtkColorTransferFunction> colorTransferFunction;
  colorTransferFunction->AddRGBPoint(0.0, 0.0, 0.0, 0.0);
  //colorTransferFunction->AddRGBPoint(dim[0]*0.25f, 1.0, 0.0, 0.0);
  //colorTransferFunction->AddRGBPoint(dim[0]*0.5f, 0.0, 0.0, 1.0);
  //colorTransferFunction->AddRGBPoint(dim[0]*0.75f, 0.0, 1.0, 0.0);
  colorTransferFunction->AddRGBPoint(100, 1.0, 1.0, 1.0);

  // The property describes how the data will look
  vtkNew<vtkVolumeProperty> volumeProperty;
  volumeProperty->SetColor(colorTransferFunction);
  volumeProperty->SetScalarOpacity(opacityTransferFunction);
  //volumeProperty->ShadeOn();
  volumeProperty->SetInterpolationTypeToLinear();

  // The mapper / ray cast function know how to render the data
  vtkNew<vtkFixedPointVolumeRayCastMapper> volumeMapper;
  //volumeMapper->SetInputConnection(reader->GetOutputPort());
  volumeMapper->SetInputData(img);

  // The volume holds the mapper and the property and
  // can be used to position/orient the volume
  vtkNew<vtkVolume> volume;
  volume->SetMapper(volumeMapper);
  volume->SetProperty(volumeProperty);
  volume->SetPosition(0, 0, 0);

  ren1->AddVolume(volume);
  ren1->SetBackground(colors->GetColor3d("Wheat").GetData());
  ren1->GetActiveCamera()->Azimuth(45);
  ren1->GetActiveCamera()->Elevation(30);
  ren1->ResetCameraClippingRange();
  ren1->ResetCamera();

  renWin->SetSize(600, 600);
  renWin->SetWindowName("SimpleRayCast");
  renWin->Render();

  iren->Start();

  return EXIT_SUCCESS;
}
