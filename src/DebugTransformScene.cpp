#include "DebugTransformScene.hpp"

DebugTransformScene::DebugTransformScene(MapState& map) : Scene(map, SceneType::Base, SceneLifetime::Manual),
    _label1(map),
    _label2(map),
    _label3(map),
    _label4(map),
    _label5(map),
    _label6(map),
    _label7(map),
    _label8(map),
    projection(map),
    fullscreen_rect_vertex_buffer_data
    { 
      0.0f, (float)map.height, 0.0f,
      0.0f,  0.0f, 0.0f,
      (float)map.width, (float)map.height, 0.0f,
      (float)map.width,  0.0f, 0.0f
    }
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
  LonLatLookupTexture = LoadImageToTexture(lut);

  // Load and compile the shaders into a glsl program
  program = loadGraphicsProgram(vertShader, fragShader);
  program.SetCameraFromPixelTransform(map.width,map.height);
}

void DebugTransformScene::drawMapRect()
{
  glVertexAttribPointer(
                        0, //vertexPosition_modelspaceID, // The attribute we want to configure
                        3,                  // size
                        GL_FLOAT,           // type
                        GL_FALSE,           // normalized?
                        0,                  // stride
                        fullscreen_rect_vertex_buffer_data // (void*)0            // array buffer offset
                );

   // see above glEnableVertexAttribArray(vertexPosition_modelspaceID);
   glEnableVertexAttribArray ( 0 );

  // Draw the triangles!
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void DebugTransformScene::drawOverride()
{
	// Select our shader program
	glUseProgram(program.GetId());
	
	// Assign the transform LUT to texture 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, LonLatLookupTexture);
  
  // Tell our shader which units to look for each texture on
  program.SetUniform("uLonLatLut", 0);
  
  // Tell the frag shader the size of the map in pixels
  program.SetUniform("uScale", map.width, map.height);
  
  // Draw a full map-sized rectagle using the current shader
	drawMapRect();
	
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
