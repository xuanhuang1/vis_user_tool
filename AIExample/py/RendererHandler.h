
enum RENDER_TYPE{
	SURFACE, VOLUME
};


struct RendererHandler{
    RendererHandler(){
	rotX = 0;
	rotY = 0;
	scale = -2;
	mode = SURFACE;
	isovalue = 180;
	isovalueVolRange = 256;
    }
    
    // renderer attribute access functions
    // need to be individually binded in PYBIND11_MODULE for python interface
    void rotateLeft (float x) { rotX = -x;}
    void rotateRight(float x) { rotX =  x;}
    void rotateUp   (float y) { rotY =  y;}
    void rotateDown (float y) { rotY = -y;}
    void zoom (float z) { scale = z;}
    void setRenderMode(std::string str) 
    { 
    	if (str == "surface") mode = SURFACE;
    	else if (str == "volume") mode = VOLUME;
    	else mode = VOLUME; // default to volume
    }
    void setIsoValue(float v) { isovalue = v;}
    void setIsoValueVolumeRange(float v) { isovalueVolRange = v;}

    float rotX;
    float rotY;
    float scale;
    RENDER_TYPE mode;
    float isovalue;
    float isovalueVolRange;
};

