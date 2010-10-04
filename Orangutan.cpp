/*
    Orangutan
    ---------
    
    Copyright (c) 2010 Robin Southern
                                                                                  
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:
                                                                                  
    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.
                                                                                  
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE. 
    
*/

#include "Orangutan.h"

const Ogre::String Orangutan::Librarian::MOVABLE_OBJECT_NAME = "OrangutanGeometry";
const Ogre::String Orangutan::Geometry::DEFAULT_MATERIAL_NAME = "BaseWhiteNoLighting";
const Ogre::Vector3 Orangutan::Quad::QUAD_VERTICES[4] = 
           { Ogre::Vector3(-0.5f, 0,  0.5f),  // A
             Ogre::Vector3( 0.5f, 0,  0.5f),  // B
             Ogre::Vector3(-0.5f, 0, -0.5f),  // C
             Ogre::Vector3( 0.5f, 0, -0.5f)   // D
           };

const Ogre::Vector3 Orangutan::Block::BLOCK_VERTICES[8] = 
           {
             Ogre::Vector3(-0.5f, 0.5,  0.5f),  // A
             Ogre::Vector3( 0.5f, 0.5,  0.5f),  // B
             Ogre::Vector3(-0.5f, 0.5, -0.5f),  // C
             Ogre::Vector3( 0.5f, 0.5, -0.5f),  // D
             Ogre::Vector3(-0.5f,-0.5,  0.5f),  // E
             Ogre::Vector3( 0.5f,-0.5,  0.5f),  // F
             Ogre::Vector3(-0.5f,-0.5, -0.5f),  // G
             Ogre::Vector3( 0.5f,-0.5, -0.5f)   // H
           };

template<> Orangutan::Librarian* Ogre::Singleton<Orangutan::Librarian>::ms_Singleton = 0;

#define PUSH_VERTEX(VERTEX, NEW_VERTEX) VERTEX.position = NEW_VERTEX; vertices.push_back(VERTEX);

namespace Orangutan
{
 
 
void writeOok(std::ofstream& stream, const Ogre::Vector3& vec, const Ogre::String& prefix = Ogre::StringUtil::BLANK)
{
 stream << prefix << vec.x << " " << vec.y << " " << vec.z << "\n";
}

void writeOok(std::ofstream& stream, const Ogre::Vector2& vec, const Ogre::String& prefix = Ogre::StringUtil::BLANK)
{
 stream << prefix << vec.x << " " << vec.y << "\n";
}

void writeOok(std::ofstream& stream, const Ogre::Quaternion& quat, const Ogre::String& prefix = Ogre::StringUtil::BLANK)
{
 stream << prefix << quat.w << " " << quat.x << " " << quat.y << " " << quat.z << "\n";
}

void writeOok(std::ofstream& stream, Ogre::ColourValue* colours, size_t count, const Ogre::String& prefix = Ogre::StringUtil::BLANK)
{
 stream << prefix;
 for (size_t i=0;i < count;i++)
 {
  stream << colours[i].r << " " << colours[i].g << " " << colours[i].b << " " << colours[i].a;
  if (i != count)
   stream << " ";
 }
 stream << "\n";
}

void writeOok(std::ofstream& stream, const Ogre::Radian& rad, const Ogre::String& prefix = Ogre::StringUtil::BLANK)
{
 stream << prefix << rad.valueRadians() << "\n";
}

void writeOok(std::ofstream& stream, bool val, const Ogre::String& prefix = Ogre::StringUtil::BLANK)
{
 stream << prefix << (val? "yes" : "no") << "\n";
}


void writeOok(std::ofstream& stream, buffer<float> heights, const Ogre::String& prefix = Ogre::StringUtil::BLANK)
{
 stream << prefix << heights.size() << " [\n";
 size_t i=0;
 while (i != heights.size())
 {
  stream << "\t\t";
  for (size_t j=0;j < 16;j++)
  {
   if (i == heights.size())
    break;
   stream << " " << heights[i++];
  }
  stream << "\n";
 }
 stream << "\t]\n";
}


void writeOok(std::ofstream& stream, buffer<Ogre::ColourValue> colours, const Ogre::String& prefix = Ogre::StringUtil::BLANK)
{
 stream << prefix << colours.size() << " [\n";
 size_t i=0;
 while (i != colours.size())
 {
  stream << "\t\t";
  for (size_t j=0;j < 4;j++)
  {
   if (i == colours.size())
    break;
   stream << " " << colours[i].r << " " << colours[i].g << " " << colours[i].b << " " << colours[i].a;
   i++;
  }
  stream << "\n";
 }
 stream << "\t]\n";
}
// ----------------------------------------------------------------------------------------




 
Librarian::Librarian()
{
 Ogre::Root::getSingletonPtr()->addMovableObjectFactory(this);
}

Librarian::~Librarian()
{
 Ogre::Root::getSingletonPtr()->removeMovableObjectFactory(this);
}

Ogre::MovableObject* Librarian::createInstanceImpl(const Ogre::String& name, const Ogre::NameValuePairList* params)
{
 return OGRE_NEW Geometry(name);
}

void Librarian::destroyInstance(Ogre::MovableObject* obj)
{
 OGRE_DELETE obj;
}




// ----------------------------------------------------------------------------------------




 
Geometry::Geometry(const Ogre::String& name)
: MovableObject(name), mRedrawNeeded(false)
{
 mAABB.setExtents(Ogre::Vector3(-1,-1,-1), Ogre::Vector3(1,1,1));
 // Push back the default geometry.
 mGeometries[0] = OGRE_NEW GeometryRenderable("BaseWhiteNoLighting", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, this, 0);
}

Geometry::~Geometry()
{
 // TODO: Delete Geometries.
 for (GeometryRenderables::iterator it = mGeometries.begin(); it != mGeometries.end();it++)
  OGRE_DELETE (*it).second;
 mGeometries.clear();
}

void Geometry::setMaterialName(size_t index, const Ogre::String& materialName, const Ogre::String& group)
{
 
 GeometryRenderables::iterator it = mGeometries.find(index);
 if (it != mGeometries.end())
 {
  (*it).second->setMaterialName(materialName, group);
  mRedrawNeeded = true;
  return;
 }

 GeometryRenderable* renderable = OGRE_NEW GeometryRenderable(materialName, group, this, index);
 mGeometries[index] = renderable;
}

GeometryRenderable* Geometry::getOrCreateRenderable(size_t index, const Ogre::String& materialName, const Ogre::String& groupName)
{
 GeometryRenderables::iterator it = mGeometries.find(index);
 if (it != mGeometries.end())
  return (*it).second;
 GeometryRenderable* renderable = OGRE_NEW GeometryRenderable(materialName, groupName, this, index);
 mGeometries[index] = renderable;
 return renderable;
}

Plane*  Geometry::createPlane(const Ogre::Vector3& position, const Ogre::Vector2& size, const Ogre::Quaternion& orientation, size_t materialIndex )
{
 
 Plane* plane = OGRE_NEW Orangutan::Plane(position, size, orientation, materialIndex, this);
 mPlanes.push_back(plane);
 GeometryRenderable* renderable = getOrCreateRenderable(materialIndex);
 renderable->pushBrush(plane);
 return plane;
}

void  Geometry::destroyPlane(Plane* Plane)
{
 GeometryRenderable* renderable = getOrCreateRenderable(Plane->getIndex());
 renderable->popBrush(Plane);
 mPlanes.erase(std::find(mPlanes.begin(), mPlanes.end(), Plane));
 OGRE_DELETE Plane;
}


Displacement*  Geometry::createDisplacement(const Ogre::Vector3& position, const Ogre::Vector3& scale, const Ogre::Quaternion& orientation, size_t materialIndex)
{
 Displacement* displacement = OGRE_NEW Orangutan::Displacement(position, scale, orientation, materialIndex, this);
 mDisplacements.push_back(displacement);
 GeometryRenderable* renderable = getOrCreateRenderable(materialIndex);
 renderable->pushBrush(displacement);
 return displacement;
}

void  Geometry::destroyDisplacement(Displacement* displacement)
{
 GeometryRenderable* renderable = getOrCreateRenderable(displacement->getIndex());
 renderable->popBrush(displacement);
 mDisplacements.erase(std::find(mDisplacements.begin(), mDisplacements.end(), displacement));
 OGRE_DELETE displacement;
}

Block*  Geometry::createBlock(const Ogre::Vector3& position, const Ogre::Vector3& size, const Ogre::Quaternion& orientation, size_t materialIndex)
{
 Block* block = OGRE_NEW Orangutan::Block(position, size, orientation, materialIndex, this);
 mBlocks.push_back(block);
 GeometryRenderable* renderable = getOrCreateRenderable(materialIndex);
 redrawNeeded(materialIndex);
 return block;
}

void   Geometry::destroyBlock(Block* block)
{
 mBlocks.erase(std::find(mBlocks.begin(), mBlocks.end(), block));
 
 for (GeometryRenderables::iterator it = mGeometries.begin(); it != mGeometries.end();it++)
  redrawNeeded((*it).first);
 
 OGRE_DELETE block;
}

void  Geometry::_renderVertices()
{
 mAABB.setNull();
 // Each GeometryRenderable, redraw (if needed) and then copy to mVertexBuffer.
 for (GeometryRenderables::iterator it = mGeometries.begin(); it != mGeometries.end();it++)
 {
  (*it).second->_renderVertices(false);
  mAABB.merge((*it).second->mAABB);
 }
 if (mParentNode)
  mParentNode->needUpdate();
}

void  Geometry::_updateRenderQueue(Ogre::RenderQueue* queue)
{
 
 if (mRedrawNeeded)
 {
  mRedrawNeeded = false;
  _renderVertices();
 }
 
 for (GeometryRenderables::iterator it = mGeometries.begin(); it != mGeometries.end();it++)
 {
  if ((*it).second->isEmpty())
   continue; // Avoid empty Geometries
  
  if (mRenderQueuePrioritySet)
  {
   assert(mRenderQueueIDSet == true);
   queue->addRenderable((*it).second, mRenderQueueID, mRenderQueuePriority);
  }
  else if (mRenderQueueIDSet)
   queue->addRenderable((*it).second, mRenderQueueID);
  else
   queue->addRenderable((*it).second);
 }
 
}

void  Geometry::visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables)
{
 for (GeometryRenderables::iterator it = mGeometries.begin(); it != mGeometries.end();it++)
  visitor->visit((*it).second, 0, false);
}


void Geometry::loadFromOokFile(const Ogre::String& filename, const Ogre::String& resourceGroup)
{

}


void Geometry::saveAsOokFile(const Ogre::String& filename)
{
 
 std::ofstream stream;
 stream.open(filename.c_str(), std::ios::out | std::ios::binary);
 stream << "OOK! 0.1\n";
 
 for (GeometryRenderables::iterator it = mGeometries.begin(); it != mGeometries.end();it++)
  stream << "uses \"" << (*it).second->getMaterial()->getName() << "\" as " << (*it).first << "\n";
 
 stream << "\n";
 size_t id = 0;

 // Planes
 // ----------------------------------------
 for (std::vector<Plane*>::iterator it = mPlanes.begin(); it != mPlanes.end();it++)
  (*it)->saveToOok(stream);
 
 // Displacements
 // ----------------------------------------
 for (std::vector<Displacement*>::iterator it = mDisplacements.begin(); it != mDisplacements.end();it++)
  (*it)->saveToOok(stream);

 stream << "\n";
 stream.close();
}

void Geometry::saveAsMesh(const Ogre::String& filename)
{

}

// ----------------------------------------------------------------------------------------




 
GeometryRenderable::GeometryRenderable(const Ogre::String& materialName, const Ogre::String& materialGroup, Geometry* parent, size_t index)
: mMaterialName(materialName),
  mMaterialGroup(materialGroup),
  mParent(parent),
  mVertexBufferSize(0),
  mIndexBufferSize(0),
  mIndex(index)
{
 _create();
}

GeometryRenderable::~GeometryRenderable()
{
 _destroy();
}

void GeometryRenderable::_renderVertices(bool force)
{
  
 if (mRedrawNeeded == false)
  if (!force)
   return;
 
 // Draw vertices and calculate AABB.
 mAABB.setNull();
 mVertices.remove_all(); // Reset
 mIndexes.remove_all();
 for (std::vector<Brush*>::iterator it = mBrushes.begin(); it != mBrushes.end();it++)
 {
  (*it)->_render(mVertices, mIndexes);
  mAABB.merge((*it)->getAABB());
 }

 // Multibrushes
  // --- Blocks
  for (std::vector<Block*>::iterator it = mParent->mBlocks.begin(); it != mParent->mBlocks.end();it++)
  {
   (*it)->_render(mVertices, mIndexes, mIndex);
   mAABB.merge((*it)->getAABB());
  }
 
 
 
 
 // Copy to VertexBuffer
 _resizeVertexBuffer(mVertices.size());
 Vertex* writeIterator = (Vertex*) mVertexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);

 for (size_t i=0;i < mVertices.size();i++)
  *writeIterator++ = mVertices[i];
 mVertexBuffer->unlock();
 mRenderOp.vertexData->vertexCount = mVertices.size();
 
 // Copy Indexes
 _resizeIndexBuffer(mIndexes.size());
 Index* indexWriteIterator = (Index*) mIndexBuffer->lock(Ogre::HardwareBuffer::HBL_DISCARD);
 std::cout << "++ Locking Index Buffer" << mIndexes.size() << "\n";
 for (size_t i=0;i < mIndexes.size();i++)
  *indexWriteIterator++ = mIndexes[i];
 mIndexBuffer->unlock();
 mRenderOp.indexData->indexCount = mIndexes.size();
 
}

void  GeometryRenderable::_create(size_t initialSize)
{ 

 std::cout << __FUNCTION__ << "\n";
 mVertexBufferSize = initialSize * 3;
 mVertices.resize(mVertexBufferSize);
 mRenderOp.vertexData = OGRE_NEW Ogre::VertexData;
 mRenderOp.vertexData->vertexStart = 0;
 mRenderOp.vertexData->vertexCount = 0;
 
 Ogre::VertexDeclaration* vertexDecl = mRenderOp.vertexData->vertexDeclaration;
 size_t offset = 0;

 // Position
 vertexDecl->addElement(0,0, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
 offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
 
 // Colour
 vertexDecl->addElement(0, offset, Ogre::VET_FLOAT4, Ogre::VES_DIFFUSE);
 offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT4);
 
 // Texture Coordinates
 vertexDecl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES);
 
 mVertexBuffer = Ogre::HardwareBufferManager::getSingletonPtr()
     ->createVertexBuffer(
         vertexDecl->getVertexSize(0),
         mVertexBufferSize,
         Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
         false
     );
 
 mRenderOp.vertexData->vertexBufferBinding->setBinding(0, mVertexBuffer);
 
 mRenderOp.useIndexes = true;
 mRenderOp.indexData = OGRE_NEW Ogre::IndexData;
 mRenderOp.indexData->indexStart = 0;
 mRenderOp.indexData->indexCount = 0;
 
 mIndexBufferSize = mVertexBufferSize * 3;
 mIndexes.resize(mIndexBufferSize);
 mIndexBuffer = Ogre::HardwareBufferManager::getSingletonPtr()->createIndexBuffer(
   Ogre::HardwareIndexBuffer::IT_16BIT,
   mIndexBufferSize,
   Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE
  );
 mRenderOp.indexData->indexBuffer = mIndexBuffer;
 mRenderOp.operationType = Ogre::RenderOperation::OT_TRIANGLE_LIST;
 
}

void  GeometryRenderable::_destroy()
{
 
  std::cout << __FUNCTION__ << "\n";
  OGRE_DELETE mRenderOp.vertexData;
  OGRE_DELETE mRenderOp.indexData;
  mRenderOp.vertexData = 0;
  mVertexBuffer.setNull();
  mVertexBufferSize = 0;
  mIndexBuffer.setNull();
  mIndexBufferSize = 0;
  mVertices.destroy();
  mIndexes.destroy();
}

void  GeometryRenderable::_resizeVertexBuffer(size_t requestedSize)
{
 std::cout << __FUNCTION__ << "\n";
 
 if (mVertexBufferSize == 0)
 {
  std::cout << "\n\n\nVertex Buffer Size is zero, need to create it\n --> Pointer is " << this << "\n\n\n";
  _create();
 }
 
 if (requestedSize > mVertexBufferSize)
 {
  size_t newVertexBufferSize = 1;
  
  while(newVertexBufferSize < requestedSize)
   newVertexBufferSize <<= 1;
  
  mVertexBuffer = Ogre::HardwareBufferManager::getSingletonPtr()->createVertexBuffer(
    mRenderOp.vertexData->vertexDeclaration->getVertexSize(0),
    newVertexBufferSize,
    Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE,
    false
  );
  mVertexBufferSize = newVertexBufferSize;
  mRenderOp.vertexData->vertexStart = 0;
  mRenderOp.vertexData->vertexBufferBinding->setBinding(0, mVertexBuffer);
 }
  
}

void  GeometryRenderable::_resizeIndexBuffer(size_t requestedSize)
{
 
 std::cout << __FUNCTION__ << "\n";
 
 if (requestedSize > mIndexBufferSize)
 {
  size_t newIndexBufferSize = 1;
  
  while(newIndexBufferSize < requestedSize)
   newIndexBufferSize <<= 1;
  
  mIndexBuffer = Ogre::HardwareBufferManager::getSingletonPtr()->createIndexBuffer(
   Ogre::HardwareIndexBuffer::IT_16BIT,
   newIndexBufferSize,
   Ogre::HardwareBuffer::HBU_DYNAMIC_WRITE_ONLY_DISCARDABLE
  );
  mRenderOp.indexData->indexBuffer = mIndexBuffer;
  mIndexBufferSize = newIndexBufferSize;
 }
  
}

void GeometryRenderable::getWorldTransforms(Ogre::Matrix4* transform) const
{
 transform[0] = mParent->_getParentNodeFullTransform();
}

void GeometryRenderable::setMaterialName(const Ogre::String& materialName, const Ogre::String& materialGroup)
{
 std::cout << __FUNCTION__ << "\n";
 mMaterialName = materialName;
 mMaterialGroup = materialGroup;
 mMaterial = Ogre::MaterialManager::getSingletonPtr()->load(mMaterialName, mMaterialGroup);
}

Ogre::Real GeometryRenderable::getSquaredViewDepth(const Ogre::Camera* cam) const
{
 Ogre::Node* node = mParent->getParentNode();
 assert(node);
 return node->getSquaredViewDepth(cam);
}

const Ogre::LightList& GeometryRenderable::getLights(void) const
{
 return mParent->queryLights();
}

void GeometryRenderable::pushBrush(Brush* brush)
{
 mBrushes.push_back(brush);
 mParent->redrawNeeded(brush->getIndex());
}
   
void GeometryRenderable::popBrush(Brush* brush)
{
 mBrushes.erase(std::find(mBrushes.begin(), mBrushes.end(), brush));
 mParent->redrawNeeded(brush->getIndex());
}




// ----------------------------------------------------------------------------------------




 
Quad::Quad(const Ogre::Vector3& position, const Ogre::Vector2& size, const Ogre::Quaternion& orientation, Ogre::AxisAlignedBox* aabb)
 : mPosition(position),
   mSize(Ogre::Vector3(size.x, 0, size.y)),
   mOrientation(orientation),
   mTextureZoom(2,2),
   mTextureOffset(0,0),
   mTextureFlipX(false),
   mTextureFlipY(false),
   mTextureAngle(Ogre::Degree(45)),
   mAABB(aabb)
{
 mColours[0] = Ogre::ColourValue::White;
 mColours[1] = Ogre::ColourValue::White;
 mColours[2] = Ogre::ColourValue::White;
 mColours[3] = Ogre::ColourValue::White;

 _update();
}

void Quad::_update()
{
  
 mTransform.makeTransform(mPosition, mSize, mOrientation);
 
 mVertices[0].position = mTransform * QUAD_VERTICES[0]; // A
 mVertices[0].uv = Ogre::Vector2(0,0);
 mVertices[0].colour = mColours[0];
 
 mVertices[1].position = mTransform * QUAD_VERTICES[1]; // B
 mVertices[1].uv = Ogre::Vector2(1,0);
 mVertices[1].colour = mColours[1];

 mVertices[2].position = mTransform * QUAD_VERTICES[2]; // C
 mVertices[2].uv = Ogre::Vector2(0,1);
 mVertices[2].colour = mColours[2];

 mVertices[3].position = mTransform * QUAD_VERTICES[3]; // D
 mVertices[3].uv = Ogre::Vector2(1,1);
 mVertices[3].colour = mColours[3];
 
 // Texture Rotation
 // ----------------
 if (mTextureAngle.valueAngleUnits() != 0.0f)
 {
  Ogre::Vector3 t(0,0,0);
  Ogre::Quaternion q;
  q.FromAngleAxis(mTextureAngle, Ogre::Vector3::UNIT_Y);
  for (size_t i=0;i < 4;i++)
  {
   t.x = mVertices[i].uv.x;
   t.z = mVertices[i].uv.y;
   t = q * t;
   mVertices[i].uv.x = t.x;
   mVertices[i].uv.y = t.z;
  }
 }
 
 // Flip X
 for (size_t i=0;i < 4;i++)
 {
  mVertices[i].uv.x = (mVertices[i].uv.x *  mTextureZoom.x) + mTextureOffset.x;
  mVertices[i].uv.y = (mVertices[i].uv.y *  mTextureZoom.y) + mTextureOffset.y;
 }
 
 if (!mTextureFlipX) // X has to be flipped for some reason.
  for (size_t i=0;i < 4;i++)
   mVertices[i].uv.x *= -1;
   
 if (mTextureFlipY)
  for (size_t i=0;i < 4;i++)
   mVertices[i].uv.y *= -1;
  
  
  // AABB
  // ----
  mAABB->merge(mVertices[0].position);
  mAABB->merge(mVertices[1].position);
  mAABB->merge(mVertices[2].position);
  mAABB->merge(mVertices[3].position);


}

void Quad::_render(buffer<Vertex>& vertices, buffer<Index>& indexes)
{
 
 size_t i = vertices.size();
 vertices.push_back(mVertices[0]);
 vertices.push_back(mVertices[1]);
 vertices.push_back(mVertices[2]);
 vertices.push_back(mVertices[3]);
 
 // TODO: GeometryOp_Draw/GeometryOp_DrawInverse switch/if in here?
 indexes.push_back(i+2); // C
 indexes.push_back(i);   // A
 indexes.push_back(i+1); // B
 indexes.push_back(i+2); // C
 indexes.push_back(i+1); // B
 indexes.push_back(i+3); // D
 
}





Plane::Plane(const Ogre::Vector3& position, const Ogre::Vector2& size, const Ogre::Quaternion& orientation, size_t materialIndex, Geometry* geometry)
 : Brush(geometry, materialIndex)
{
 mQuad = new Quad(position, size, orientation, &mAABB);
}

void Plane::_render(buffer<Vertex>& vertices, buffer<Index>& indexes)
{
 mQuad->_render(vertices, indexes);
}

void Plane::saveToOok(std::ofstream& stream)
{
 stream << "brush \"plane\" \n";
 writeOok(stream, mQuad->mPosition, "\tposition ");
 writeOok(stream, mQuad->mOrientation, "\torientation ");
 writeOok(stream, mQuad->mSize, "\tsize ");
 writeOok(stream, mQuad->mColours, 4, "\tcolours ");
 writeOok(stream, mQuad->mTextureAngle, "\ttexture_angle ");
 writeOok(stream, mQuad->mTextureFlipX, "\ttexture_flip_x ");
 writeOok(stream, mQuad->mTextureFlipY, "\ttexture_flip_y ");
 writeOok(stream, mQuad->mTextureOffset, "\ttexture_offset ");
 writeOok(stream, mQuad->mTextureZoom, "\ttexture_offset ");
 stream << ";\n\n";
}


Displacement::Displacement(const Ogre::Vector3& position, const Ogre::Vector3& scale, const Ogre::Quaternion& orientation, size_t materialIndex, Geometry* geometry)
 : Brush(geometry, materialIndex),
   mTextureZoom(2,2),
   mTextureOffset(0,0),
   mTextureFlipX(false),
   mTextureFlipY(false),
   mTextureAngle(Ogre::Degree(45)),
   mDescribing(false),
   mPosition(position),
   mScale(scale),
   mOrientation(orientation)
{
 mAABB.setNull();
}

Displacement::~Displacement()
{
}

void Displacement::saveToOok(std::ofstream& stream)
{
 stream << "brush \"displacement\" \n";
 writeOok(stream, mPosition, "\tposition ");
 writeOok(stream, mOrientation, "\torientation ");
 writeOok(stream, mScale, "\tscale ");
 writeOok(stream, mTextureAngle, "\ttexture_angle ");
 writeOok(stream, mTextureFlipX, "\ttexture_flip_x ");
 writeOok(stream, mTextureFlipY, "\ttexture_flip_y ");
 writeOok(stream, mTextureOffset, "\ttexture_offset ");
 writeOok(stream, mTextureZoom, "\ttexture_offset ");
 writeOok(stream, mHeights, "\theights ");
 writeOok(stream, mColours, "\tcolours ");
 stream << ";\n\n";
}

void Displacement::_render(buffer<Vertex>& vertices, buffer<Index>& indexes)
{
 
 size_t i=0, b = vertices.size();
 
 for (i=0;i < mVertices.size();i++)
  vertices.push_back(mVertices[i]);
 
 for (i=0;i < mIndexes.size();i++)
  indexes.push_back(b + mIndexes[i]);
 
}

void Displacement::_updateRequired()
{

 if (mDescribing)
  return;
 
 mAABB.setNull();
 
 Vertex vertex;
 mVertices.remove_all();
 
 size_t i=0;
 Ogre::Real texIncrementX = (1.0f / Ogre::Real(mLengthX-1)) * mTextureZoom.x,
            texIncrementY = (1.0f / Ogre::Real(mLengthY-1)) * mTextureZoom.y,
            texX = 0,
            texY = 0;

 if (mTextureFlipX)
  texIncrementX = -texIncrementX;
 
 if (mTextureFlipY)
  texIncrementY = -texIncrementY;
 
 for (size_t z=0;z < mLengthY;z++)
 {
  for (size_t x=0;x < mLengthX;x++)
  {
   vertex.position.x = x;
   vertex.position.y = mHeights[i];
   vertex.position.z = z;
   vertex.colour = mColours[i++];
   vertex.uv.x = texX;
   vertex.uv.y = texY;
   mVertices.push_back(vertex);

   texX += texIncrementX;
  }
  texX = 0;
  texY += texIncrementY;
 }
 
 if (mTextureAngle.valueAngleUnits() != 0.0f)
 {
  Ogre::Vector3 t(0,0,0);
  Ogre::Quaternion q;
  q.FromAngleAxis(mTextureAngle, Ogre::Vector3::UNIT_Y);
  for (size_t i=0;i < mVertices.size();i++)
  {
   t.x = mVertices[i].uv.x;
   t.z = mVertices[i].uv.y;
   t = q * t;
   mVertices[i].uv.x = t.x;
   mVertices[i].uv.y = t.z;
  }
 }
 
 // Transform:-
 mTransform.makeTransform(mPosition, mScale, mOrientation);
 Ogre::Vector2 halfSize(mLengthX, mLengthY);
 halfSize *= 0.5f;
 for (size_t i=0;i < mVertices.size();i++)
 {
  mVertices[i].position.x -= halfSize.x;
  mVertices[i].position.z -= halfSize.y;
  mVertices[i].position = mTransform * mVertices[i].position;
  mAABB.merge(mVertices[i].position);
 }
 
 mIndexes.remove_all();
 
 size_t y = 0;
 bool flip = false;
 bool flipRow = (mLengthX % 2) != 0;

 for (size_t j=0;j < mLengthY-1;j++)
 {
  y = j * mLengthX;
  for (size_t i=0;i < mLengthX-1;i++)
  {
    
   if (!flip)
   {
    mIndexes.push_back(i + y + mLengthX);
    mIndexes.push_back(i + 1 + y);
    mIndexes.push_back(i + y);
   
    mIndexes.push_back(i + y + mLengthX);
    mIndexes.push_back(i + 1 + y + mLengthX);
    mIndexes.push_back(i + 1 + y);
   }
   else
   {
    mIndexes.push_back(i + y + mLengthX);
    mIndexes.push_back(i + 1 + y + mLengthX);
    mIndexes.push_back(i + y);
   
    mIndexes.push_back(i + y);
    mIndexes.push_back(i + 1 + y + mLengthX);
    mIndexes.push_back(i + 1 + y);
    
   }
   
   flip = !flip;
  }
  if (flipRow)
   flip = !flip;
 }
 
}

Block::Block(const Ogre::Vector3& position, const Ogre::Vector3& size, const Ogre::Quaternion& orientation, size_t index, Geometry* geometry)
 : MultiBrush(geometry),
   mPosition(position),
   mSize(size),
   mOrientation(orientation)
{
 
 for (size_t i=0; i < 6;i++)
 {
  mHasQuads[i] = true;
  mQuadMaterial[i] = index;
  mQuadTextureScale[i] = Ogre::Vector2(1,1);
  mQuadTextureOffset[i] = Ogre::Vector2(0,0);
  mQuadTextureFlipX[i] = false;
  mQuadTextureFlipY[i] = false;
  mQuadTextureColour[i] = Ogre::ColourValue(1.0f, 1.0f, 1.0f, 1.0f);
 }

 _updateRequired();
}

Block::~Block()
{

}

void Block::_render(buffer<Vertex>& vertices, buffer<Index>& indexes, size_t index)
{
 size_t j = 0;
 for (size_t i=0; i < 6;i++)
 {
   std::cout << "Block::_render @ " << i << "\n";
  if (mHasQuads[i] == false)
   continue;
  
  if (mQuadMaterial[i] != index)
   continue;
  
  j = vertices.size();
  vertices.push_back( mQuadVertexData[i].mVertices[0] );
  vertices.push_back( mQuadVertexData[i].mVertices[1] );
  vertices.push_back( mQuadVertexData[i].mVertices[2] );
  vertices.push_back( mQuadVertexData[i].mVertices[3] );

  indexes.push_back( j + mQuadVertexData[i].mIndexes[0] );
  indexes.push_back( j + mQuadVertexData[i].mIndexes[1] );
  indexes.push_back( j + mQuadVertexData[i].mIndexes[2] );  // Remove mIndexes, and just hardcode it.
  indexes.push_back( j + mQuadVertexData[i].mIndexes[3] );
  indexes.push_back( j + mQuadVertexData[i].mIndexes[4] );
  indexes.push_back( j + mQuadVertexData[i].mIndexes[5] );
  
  std::cout << "Block::_render == " << i << "\n";
  
 }
#if 0
 size_t base = vertices.size();
 
 for(size_t i=0;i < mVertices.size();i++)
 {
  std::cout << mVertices[i].position << "\n";
  vertices.push_back(mVertices[i]);
 }
 
 for(size_t i=0;i < mIndexes.size();i++)
 {
  indexes.push_back(base + mIndexes[i]);
 }
#endif
}

void Block::_updateRequired()
{
 
 // Transform
 mTransform.makeTransform(mPosition, mSize, mOrientation);
 mAABB.setNull();

#define BLOCK_VERTEX(REF_ID, ID, QUAD, U, V)                                \
 mQuadVertexData[QUAD].mVertices[REF_ID].position = BLOCK_VERTICES[ID];     \
 mQuadVertexData[QUAD].mVertices[REF_ID].uv.x = U;                          \
 mQuadVertexData[QUAD].mVertices[REF_ID].uv.y = V;                          \
 mQuadVertexData[QUAD].mVertices[REF_ID].colour = mQuadTextureColour[QUAD];  

#define BLOCK_UV(QUAD)                                                \
  for (size_t j=0; j < 4;j++)                                         \
  {                                                                   \
   mQuadVertexData[QUAD].mVertices[j].uv *= mQuadTextureScale[QUAD];  \
   mQuadVertexData[QUAD].mVertices[j].uv += mQuadTextureOffset[QUAD]; \
   if (mQuadTextureFlipX[QUAD])                                       \
    mQuadVertexData[QUAD].mVertices[j].uv.x = -mQuadVertexData[QUAD].mVertices[j].uv.x;     \
   if (mQuadTextureFlipY[QUAD])                                       \
    mQuadVertexData[QUAD].mVertices[j].uv.y = -mQuadVertexData[QUAD].mVertices[j].uv.y;     \
  }                                                                    

#define BLOCK_TRANSFORM(QUAD)                                   \
 for (size_t j=0;j < 4;j++)                                     \
 {                                                              \
  mQuadVertexData[QUAD].                                        \
     mVertices[j].position =                                    \
      mTransform * mQuadVertexData[QUAD].mVertices[j].position; \
  mAABB.merge(mQuadVertexData[QUAD].mVertices[j].position);     \
 }                                                               

#define BLOCK_TRANGLES(QUAD)              \
  mQuadVertexData[QUAD].mIndexes[0] = 2;  \
  mQuadVertexData[QUAD].mIndexes[1] = 0;  \
  mQuadVertexData[QUAD].mIndexes[2] = 1;  \
  mQuadVertexData[QUAD].mIndexes[3] = 2;  \
  mQuadVertexData[QUAD].mIndexes[4] = 1;  \
  mQuadVertexData[QUAD].mIndexes[5] = 3;   
  
 if (mHasQuads[Quad_Top])
 {
  BLOCK_VERTEX(0, 0, Quad_Top,  0,0)
  BLOCK_VERTEX(1, 1, Quad_Top, -1,0)
  BLOCK_VERTEX(2, 2, Quad_Top,  0,1)
  BLOCK_VERTEX(3, 3, Quad_Top, -1,1)
  BLOCK_UV(Quad_Top)
  BLOCK_TRANSFORM(Quad_Top)
  BLOCK_TRANGLES(Quad_Top)
 }
  
 if (mHasQuads[Quad_Bottom])
 {
  BLOCK_VERTEX(0, 5, Quad_Bottom,  0,0)
  BLOCK_VERTEX(1, 4, Quad_Bottom, -1,0)
  BLOCK_VERTEX(2, 7, Quad_Bottom,  0,1)
  BLOCK_VERTEX(3, 6, Quad_Bottom, -1,1)
  BLOCK_UV(Quad_Bottom)
  BLOCK_TRANSFORM(Quad_Bottom)
  BLOCK_TRANGLES(Quad_Bottom)
 }
  
 if (mHasQuads[Quad_Left])
 {
  BLOCK_VERTEX(0, 0, Quad_Left,  0,0)
  BLOCK_VERTEX(1, 2, Quad_Left, -1,0)
  BLOCK_VERTEX(2, 4, Quad_Left,  0,1)
  BLOCK_VERTEX(3, 6, Quad_Left, -1,1)
  BLOCK_UV(Quad_Left)
  BLOCK_TRANSFORM(Quad_Left)
  BLOCK_TRANGLES(Quad_Left)
 }
  
 if (mHasQuads[Quad_Right])
 {
  BLOCK_VERTEX(0, 3, Quad_Right,  0,0)
  BLOCK_VERTEX(1, 1, Quad_Right, -1,0)
  BLOCK_VERTEX(2, 7, Quad_Right,  0,1)
  BLOCK_VERTEX(3, 5, Quad_Right, -1,1)
  BLOCK_UV(Quad_Right)
  BLOCK_TRANSFORM(Quad_Right)
  BLOCK_TRANGLES(Quad_Right)
 }
  
 if (mHasQuads[Quad_Front])
 {
  BLOCK_VERTEX(0, 1, Quad_Front,  0,0)
  BLOCK_VERTEX(1, 0, Quad_Front, -1,0)
  BLOCK_VERTEX(2, 5, Quad_Front,  0,1)
  BLOCK_VERTEX(3, 4, Quad_Front, -1,1)
  BLOCK_UV(Quad_Front)
  BLOCK_TRANSFORM(Quad_Front)
  BLOCK_TRANGLES(Quad_Front)
 }
  
 if (mHasQuads[Quad_Back])
 {
  BLOCK_VERTEX(0, 2, Quad_Back,  0,0)
  BLOCK_VERTEX(1, 3, Quad_Back, -1,0)
  BLOCK_VERTEX(2, 6, Quad_Back,  0,1)
  BLOCK_VERTEX(3, 7, Quad_Back, -1,1)
  BLOCK_UV(Quad_Back)
  BLOCK_TRANSFORM(Quad_Back)
  BLOCK_TRANGLES(Quad_Back)
 }
 
 std::cout << mAABB << "\n";
#undef BLOCK_VERTEX
#undef BLOCK_UV
#undef BLOCK_TRANGLES
 
 
}


} // namespace Orangutan
