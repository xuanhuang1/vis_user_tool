# vis_user_tool

## animation system for large scivis data
json based textfiles + loader for automated animation scripting and production

**build**
dependencies includes:
ospray v2.12 (for rendering)
pybind11 (for python interface)

build and install ospray following https://github.com/RenderKit/ospray/tree/release-2.12.x

set ospray_DIR and rkcommon_DIR (included in ospray build&install)

**examples/**

examples with opengl and ospray v2.12

**python/**
python interface with examples:
```
// to read, download and script by either preset templates or an interactive viewer
// produce a list of json files
python3 scripting.py 
// render with json file
python3 render.py path_to_json 
```
**jupter_notebook_example/**

remote data access through jupternotebook, see animationToolTutorial.ipynb.

**renderingApps/**

different rendering backends with raw data 

## Images

![This is an alt text.](/interactApp/demo_img/render_full_res_overview.png "This is a sample image.")
![This is an alt text.](/interactApp/demo_img/render_full_res_local.png "This is a sample image.")
