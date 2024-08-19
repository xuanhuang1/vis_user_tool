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


#include <vtkSmartPointer.h>
#include <vtkImageWriter.h>
#include <vtkPNGWriter.h>
#include <vtkPostScriptWriter.h>
#include <vtkWindowToImageFilter.h>


#include <fstream>
#include "../../loader.h"

using namespace visuser;
using namespace nlohmann_loader;

std::string getOutName(std::string str, uint32_t idx)
{
    std::string base_filename = str.substr(str.find_last_of("/\\") + 1);
    std::string outname = base_filename.substr(0, base_filename.find_last_of("."));
    return "img_"+outname+"_kf"+std::to_string(idx)+".png";
}


void loadTransferFunction(AniObjWidget &widget,
			  vtkVolumeProperty *volumeProperty)
{
    
    // Create transfer mapping scalar value to opacity
    vtkNew<vtkPiecewiseFunction> opacityTransferFunction;
    // Create transfer mapping scalar value to color
    vtkNew<vtkColorTransferFunction> colorTransferFunction;
    std::vector<float> colors;
    std::vector<float> opacities;
    widget.getFrameTF(colors, opacities);

    for (uint32_t i=0; i<colors.size()/3; i++){
	float current_val = widget.tfRange[0] + float(i)/(colors.size()/3) * (widget.tfRange[1] - widget.tfRange[0]);
        opacityTransferFunction->AddPoint(current_val, opacities[i]);
	colorTransferFunction->AddRGBPoint(current_val, colors[3*i], colors[i*3+1], colors[i*3+2]);
    }
    std::cout << "load tf sz= "<< colors.size()/3<<" \n";

    volumeProperty->SetColor(colorTransferFunction);
    volumeProperty->SetScalarOpacity(opacityTransferFunction);
    //volumeProperty->ShadeOn();
    volumeProperty->SetInterpolationTypeToLinear();
}


void loadCamera(AniObjWidget &widget, uint32_t kf, vtkRenderer *ren1)
{
    // load camera
    double world_scale_xy = 1;
    for (size_t i=0; i<2; i++)
	world_scale_xy = std::min(world_scale_xy, double(widget.world_bbox[i]/widget.dims[i]));
    double cam_scale = 1/world_scale_xy;
    
    auto c = widget.cameras[kf];
    auto camera = ren1->GetActiveCamera();
    double eyePos[3] = {c.pos[0]*cam_scale, c.pos[1]*cam_scale , c.pos[2]*cam_scale };
    double focalPoint[3] = {eyePos[0] + c.dir[0]*cam_scale,
	eyePos[1] + c.dir[1]*cam_scale,
	eyePos[2] + c.dir[2]*cam_scale};
  
    camera->SetPosition(eyePos[0], eyePos[1], eyePos[2]);
    camera->SetFocalPoint(focalPoint[0], focalPoint[1], focalPoint[2]);
    camera->SetViewUp(c.up[0], c.up[1], c.up[2]);
    ren1->ResetCameraClippingRange();
    ren1->ResetCamera();
}

void loadKF(AniObjWidget &widget,
	    vtkImageData *img,
	    vtkRenderer *ren1)
{
    int dim[3] = {widget.dims[0], widget.dims[1], widget.dims[2]};
    //vtkNew<vtkImageData> img;
    img->SetDimensions(dim);
    img->AllocateScalars(VTK_INT, 1);
  
    std::cout <<"Loading volume "<<widget.file_name <<" with dim "<<dim[0]<<" "<<dim[1]<<" "<<dim[2]<<"\n";
    std::fstream file;
    file.open(widget.file_name, std::fstream::in | std::fstream::binary);
    for (int z = 0; z < dim[2]; ++z)
	for (int y = 0; y < dim[1]; ++y)
	    for (int x = 0; x < dim[0]; ++x)
		{
		    float buff;
		    file.read((char*)(&buff), sizeof(buff));
		    img->SetScalarComponentFromDouble(x, y, z, 0, buff);
		}

    file.close();
    std::cout<< "End load\n";

    double spc[3] = {1.0, 1.0, 1.0};
    img->SetSpacing(spc);
      
    // The property describes how the data will look
    vtkNew<vtkVolumeProperty> volumeProperty;
    loadTransferFunction(widget, volumeProperty);

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

    // Create the standard renderer, render window
    // and interactor
    vtkNew<vtkNamedColors> colors;
    ren1->AddVolume(volume);
    ren1->SetBackground(colors->GetColor3d("Wheat").GetData());
    
    loadCamera(widget, 0, ren1);

}


void writeImage(std::string const& fileName, vtkRenderWindow* renWin, bool rgba)
{
    if (!fileName.empty())
	{
	    std::string fn = fileName;
	    auto writer = vtkSmartPointer<vtkPNGWriter>::New();
		
	    vtkNew<vtkWindowToImageFilter> window_to_image_filter;
	    window_to_image_filter->SetInput(renWin);
	    window_to_image_filter->SetScale(1); // image quality
	    if (rgba) window_to_image_filter->SetInputBufferTypeToRGBA();	
	    else window_to_image_filter->SetInputBufferTypeToRGB();
		
	    // Read from the front buffer.
	    window_to_image_filter->ReadFrontBufferOff();
	    window_to_image_filter->Update();

	    writer->SetFileName(fn.c_str());
	    writer->SetInputConnection(window_to_image_filter->GetOutputPort());
	    writer->Write();
	}
    else std::cerr << "No filename provided." << std::endl;

    return;
}

int main(int argc, char* argv[])
{
    std::vector<std::string> args(argv, argv + argc);
    
    json config;
    std::string prefix;
    std::string overwrite_inputf = "";

  // which kf file to render
  // -2 = render all frames, -1 = render all kfs, >0 = render selected kf
  int header_sel = -2; 
  for (int i = 1; i < argc; ++i) {
      if (args[i] == "-h") {
	  std::cout << "./vtk_vistool <config.json> [options] \n"
		    << "  [options]: \n"
		    << "  -f img_output_name"
		    << "  -header-sel # ([default] -2 = render all frames, -1 = render all kfs, >0 = render selected k)"
		    << "\n";
	  return 0;
      } else {
	  if (i == 1){
	      std::ifstream cfg_file(args[i].c_str());
	      if (!cfg_file) {
		  std::cerr << "[error]: Failed to open config file " << args[i] << "\n";
		  throw std::runtime_error("Failed to open input config file");
	      }
	      cfg_file >> config;
	  }else{
	      if (args[i] == "-f"){
		  overwrite_inputf = args[++i];
	      }else if (args[i] == "-header-sel"){
		  header_sel = std::stoi(args[++i]);
	      }else if (args[i] == "-header")
		  header_sel = -1;
	  }
      }
  }
	
  AniObjWidget widget;
  AniObjHandler h(argv[1]);
  if (header_sel < 0)
      widget.init_from_json(h.widgets[0].config);
  else widget.init_from_json(h.widgets[header_sel].config);

  if (overwrite_inputf != ""){
      widget.file_name = overwrite_inputf;
  }

  // This is a simple volume rendering example that
  // uses a vtkFixedPointVolumeRayCastMapper

  vtkNew<vtkRenderer> ren1;

  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren1);

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  // Create the reader for the data
  //vtkNew<vtkStructuredPointsReader> reader;
  //vtkSmartPointer<vtkImageData> input;
  //reader->SetFileName(argv[1]);
  vtkNew<vtkImageData> img;
  loadKF(widget, img, ren1);
  
  renWin->SetSize(600, 600);
  renWin->SetWindowName("SimpleRayCast");
  renWin->Render();

  if (header_sel >= 0){ // render selected keyframe
      // save file
      writeImage(getOutName(h.widgets[header_sel].file_name, header_sel), renWin, false);
  }else{
      // reload widget for each key frame
      for (int kf_idx=0; kf_idx<h.widgets.size(); kf_idx++){
	  loadKF(h.widgets[kf_idx], img, ren1);
	  if (header_sel == -1){// render key frames
	      renWin->Render();
	      // save file
	      writeImage(getOutName(h.widgets[kf_idx].file_name, kf_idx), renWin, false);
	  }else if (header_sel == -2){//renderAllFrames
	      for (int f = h.widgets[kf_idx].frameRange[0]; f <= h.widgets[kf_idx].frameRange[1]; f++){
		  renWin->Render();
		  std::string base_filename = h.widgets[kf_idx].file_name.substr(h.widgets[kf_idx].file_name.find_last_of("/\\") + 1);
		  std::string outname = base_filename.substr(0, base_filename.find_last_of("."));
		  outname = "img_"+outname+"_f"+std::to_string(f)+".png";
		  writeImage(outname, renWin, false);
		  if (f < h.widgets[kf_idx].frameRange[1]){
		      // advance frame 
		      h.widgets[kf_idx].advanceFrame();
		      // load camera
		      double world_scale_xy = 1;
		      for (size_t i=0; i<2; i++)
			  world_scale_xy = std::min(world_scale_xy, double(widget.world_bbox[i]/widget.dims[i]));
		      double cam_scale = 1/world_scale_xy;
    
		      auto c = Camera();
		      h.widgets[kf_idx].getFrameCam(c);  
		      auto camera = ren1->GetActiveCamera();
		      double eyePos[3] = {c.pos[0]*cam_scale, c.pos[1]*cam_scale , c.pos[2]*cam_scale };
		      double focalPoint[3] = {eyePos[0] + c.dir[0]*cam_scale,
			  eyePos[1] + c.dir[1]*cam_scale,
			  eyePos[2] + c.dir[2]*cam_scale};
  
		      camera->SetPosition(eyePos[0], eyePos[1], eyePos[2]);
		      camera->SetFocalPoint(focalPoint[0], focalPoint[1], focalPoint[2]);
		      camera->SetViewUp(c.up[0], c.up[1], c.up[2]);
		      ren1->ResetCameraClippingRange();
		      ren1->ResetCamera();
		  }
	      }

	  }

      }
      

  }
  
  //iren->Start();

  return EXIT_SUCCESS;
}
