#include "DebugTransformScene.hpp"

DebugTransformScene::DebugTransformScene(ConfigService& map) : Scene(map, SceneType::Base, SceneLifetime::Manual),
    _label1(map),
    _label2(map),
    _label3(map),
    _label4(map),
    _label5(map),
    _label6(map),
    _label7(map),
    _label8(map),
    projection(map)
{
    _label1.SetText("DEBUG");
    _label1.SetPosition(0,0);
    
    _label2.SetText("DEBUG");
    _label2.SetPosition(0,0);
    _label2.SetFlowDirection(FlowDirection::Vertical);
    
    _label3.SetText("TRANSFORMATION");
    _label3.SetPosition(186,0);
    _label3.SetFlowDirection(FlowDirection::Vertical);
    
    _label4.SetText("TEST");
    _label4.SetPosition(168,0);
    
    _label5.SetText("DEBUG");
    _label5.SetPosition(0,0);
    
    _label6.SetText("DEBUG");
    _label6.SetPosition(0,0);
    
    _label7.SetText("DEBUG");
    _label7.SetPosition(0,0);
    
    _label8.SetText("DEBUG");
    _label8.SetPosition(0,0);
}

DebugTransformScene::~DebugTransformScene()
{
}

const char* DebugTransformScene::SceneName()
{
  return "Transfrom Debug Scene";
}

const char* DebugTransformScene::SceneResourceDir()
{
  return "Debug";
}

void DebugTransformScene::initGLOverride()
{
    // Create the LonLatLookupTexture
    ImageRGBA lut = projection.getInvLookupTable();
    LonLatLookupTexture = std::make_unique<GfxTexture>(lut);

    // Load and compile the shaders into a glsl program
    program = loadProgram("vertshader.glsl", "debugfragshader.glsl", 
    {
        ShaderFeature::PixelSnap,
        ShaderFeature::Texture
    });

    // Create the mesh for the image view
    //       X                  Y                          Z       U       V
    mesh = { 0.0f,                0.0f,                   0.0f,   0.0f,   0.0f,
            (float)LonLatLookupTexture->GetWidth(), 0.0f,                   0.0f,   1.0f,   0.0f, 
            0.0f,                       (float)LonLatLookupTexture->GetHeight(),  0.0f,   0.0f,   1.0f,
            (float)LonLatLookupTexture->GetWidth(), (float)LonLatLookupTexture->GetHeight(),  0.0f,   1.0f,   1.0f  };
}

void DebugTransformScene::drawOverride()
{
    program->Use();
        
    // Assign the transform LUT to texture 0
    program->SetTexture0(*LonLatLookupTexture);
  
    // Draw a full map-sized rectagle using the current shader
    glVertexAttribPointer(
                  program->Attrib("aPosition"),      // The attribute ID
                  3,                  // size
                  GL_FLOAT,           // type
                  GL_FALSE,           // normalized?
                  5*sizeof(float),                  // stride
                  mesh.data()         // underlying data
          );

    glEnableVertexAttribArray ( program->Attrib("aPosition") );
    
    glVertexAttribPointer(
                        program->Attrib("aTexCoord"), // The attribute ID
                        2,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        5*sizeof(float),   // stride
                        mesh.data()+3      // underlying data
                );
                
    glEnableVertexAttribArray(program->Attrib("aTexCoord"));

    // Draw the triangles!
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	// Draw the label
	_label1.Draw();
	_label2.Draw();
	_label3.Draw();
	_label4.Draw();
	_label5.Draw();
    _label6.Draw();
    _label7.Draw();
    _label8.Draw();
}
