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

#ifndef ORANGUTAN_H
#define ORANGUTAN_H

#include "OGRE/Ogre.h"

namespace Orangutan
{
 
 class Librarian;
 //typedef Librarian DrHoraceWorblehat;
 class Geometry;
 class Brush;
 class Quad;
 class Plane;
 class Displacement;
 class Block;
 
 enum GeometryOperation
 {
  GeometryOp_Draw,           // Draw this
  GeometryOp_DrawInverse,    // Draw this but flip the faces.
  GeometryOp_NoDraw          // Don't draw this
 };
 
 /*! enum. buffer<T>
     desc.
         Internal container class that is similar to std::vector
 */
 template<typename T> class buffer
 {
   
  public:
   
   inline buffer() : mBuffer(0), mUsed(0), mCapacity(0)
   { // no code.
   }
  
   inline ~buffer()
   {
    if (mBuffer && mCapacity)
     OGRE_FREE(mBuffer, Ogre::MEMCATEGORY_GEOMETRY);
   }
   
   inline size_t size() const
   {
    return mUsed;
   }
   
   inline size_t capacity() const
   {
    return mCapacity;
   }
   
   inline T& operator[](size_t index)
   {
    return *(mBuffer + index);
   }

   inline const T& operator[](size_t index) const
   {
    return *(mBuffer + index);
   }

   inline T& at(size_t index)
   {
    return *(mBuffer + index);
   }

   inline const T& at(size_t index) const
   {
    return *(mBuffer + index);
   }
   
   inline void remove_all()
   {
    mUsed = 0;
   }
    
   inline void resize(size_t new_capacity)
   {
    T* new_buffer = (T*) OGRE_MALLOC(sizeof(T) * new_capacity, Ogre::MEMCATEGORY_GEOMETRY);
    
    if (mUsed != 0)
    {
     if (mUsed < new_capacity)  // Copy all
      std::copy(mBuffer, mBuffer + mUsed, new_buffer);
     else if (mUsed >= new_capacity) // Copy some
      std::copy(mBuffer, mBuffer + new_capacity, new_buffer);
    }
    
    OGRE_FREE(mBuffer, Ogre::MEMCATEGORY_GEOMETRY);
    mCapacity = new_capacity;
    mBuffer = new_buffer;
   }

   inline void destroy()
   {
    if (mBuffer && mCapacity)
     OGRE_FREE(mBuffer, Ogre::MEMCATEGORY_GEOMETRY);
    mBuffer = 0;
    mUsed = 0;
    mCapacity = 0;
   }

   inline void push_back(const T& value)
   {
    if (mUsed == mCapacity)
     resize(mUsed == 0 ? 1 : mUsed * 2);
    *(mBuffer + mUsed) = value;
    mUsed++;
   }
   
   inline void pop_back()
   {
    if (mUsed != 0)
     mUsed--;
   }
   
   inline void erase(size_t index)
   {
    *(mBuffer + index) = *(mBuffer + mUsed - 1);
    mUsed--;
   }
   
   inline  T* first()
   {
    return mBuffer;
   }
   
   inline T* last()
   {
    return mBuffer + mUsed;
   }
   
  protected:
   
   T*     mBuffer;
   size_t mUsed, mCapacity;
 };
 
 /*! struct. Vertex
     desc.
         Structure for a single vertex.
 */
 struct Vertex
 {
  Ogre::Vector3     position;
  Ogre::ColourValue colour;
  Ogre::Vector2     uv;
 };
 
 typedef Ogre::ushort Index;

 class Librarian : public Ogre::Singleton<Librarian>, public Ogre::MovableObjectFactory
 {
   
  public:
   
   static const Ogre::String MOVABLE_OBJECT_NAME;
   
   Librarian();
   
  ~Librarian();
   
   Ogre::MovableObject* createInstanceImpl(const Ogre::String& name, const Ogre::NameValuePairList* params);
   
   const Ogre::String& getType(void) const
   {
    return MOVABLE_OBJECT_NAME;
   }
   
  protected:
   
   void destroyInstance(Ogre::MovableObject* obj);
   
 };
 
 class GeometryRenderable : public Ogre::Renderable, public Ogre::GeneralAllocatedObject
 {
  public:
   
   friend class Geometry;
   
   GeometryRenderable(const Ogre::String& materialName, const Ogre::String& materialGroup, Geometry*, size_t index);
   
  ~GeometryRenderable();
   
   void setMaterialName(const Ogre::String& materialName, const Ogre::String& materialGroup); 
   
   inline void pushBrush(Brush* brush);
   
   inline void popBrush(Brush* brush);
   
   void _renderVertices(bool force);
   
   /*! function. _create
       desc.
           Create the vertex buffer
   */
   void _create(size_t initialSize = 100);
   
   /*! function. _destroy
       desc.
           Destroy the vertex buffer
   */
   void _destroy();
   
   /*! function. _resizeVertexBuffer
       desc.
           Resize the vertex buffer to the greatest nearest power 
           of 2 of requestedSize.
   */
   void _resizeVertexBuffer(size_t requestedSize);
   
   /*! function. _resizeVertexBuffer
       desc.
           Resize the vertex buffer to the greatest nearest power 
           of 2 of requestedSize.
   */
   void _resizeIndexBuffer(size_t requestedSize);

   const Ogre::MaterialPtr& getMaterial(void) const
   {
    if (mMaterial.isNull())
     mMaterial = Ogre::MaterialManager::getSingletonPtr()->load(mMaterialName, mMaterialGroup);
    return mMaterial;
   }
    
   void Ogre::Renderable::getRenderOperation(Ogre::RenderOperation& op)
   {
    op = mRenderOp;
   }
   
   inline const bool isEmpty() const
   {
    return (mRenderOp.vertexData == 0 || mRenderOp.vertexData->vertexCount == 0);
   }
   
   void getWorldTransforms(Ogre::Matrix4* transform) const;
   
   Ogre::Real getSquaredViewDepth(const Ogre::Camera* cam) const;
   
   const Ogre::LightList& getLights(void) const;
   
  protected:
   
   /// mRedrawNeeded -- If all Brushes need to be copied into the VertexBuffer.
   bool                                mRedrawNeeded;
   // Copy of pointers to Brushes assigned to this GeometryRenderable
   std::vector<Brush*>                 mBrushes;
   // Temporary Vertex buffer
   buffer<Vertex>                      mVertices;
   // Temporary Index buffer
   buffer<Index>                       mIndexes;
   // Vertex buffer size
   size_t                              mVertexBufferSize;
   // Index buffer size
   size_t                              mIndexBufferSize;
   // Render Operation
   Ogre::RenderOperation               mRenderOp;
   // Master vertex buffer
   Ogre::HardwareVertexBufferSharedPtr mVertexBuffer;
   // Master index buffer
   Ogre::HardwareIndexBufferSharedPtr  mIndexBuffer;
   // Material
   mutable Ogre::MaterialPtr           mMaterial;
   // Material name and group
   Ogre::String                        mMaterialName, mMaterialGroup;
   // Parent geometry
   Geometry*                           mParent;
   // AABB
   Ogre::AxisAlignedBox                mAABB;
   // Index
   size_t                              mIndex;
 };
 
 class Geometry : public Ogre::MovableObject
 {
   
  public:
   
   friend class GeometryRenderable;
   
   static const Ogre::String DEFAULT_MATERIAL_NAME;
   
   typedef std::map<size_t, GeometryRenderable*> GeometryRenderables;
   
   friend class Librarian;
   
   inline const Ogre::String& getMovableType(void) const
   {
    return Librarian::MOVABLE_OBJECT_NAME;
   }
   
   void setMaterialName(size_t index, const Ogre::String& materialName = DEFAULT_MATERIAL_NAME, const Ogre::String& group = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
   
   GeometryRenderable* getOrCreateRenderable(size_t index, const Ogre::String& materialName = DEFAULT_MATERIAL_NAME, const Ogre::String& group = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
   
   Plane*  createPlane(const Ogre::Vector3& position, const Ogre::Vector2& size, const Ogre::Quaternion& orientation = Ogre::Quaternion::IDENTITY, size_t materialIndex = 0);
   
   void   destroyPlane(Plane*);

   Ogre::VectorIterator< std::vector<Plane*> >  getPlanes()
   {
    return Ogre::VectorIterator< std::vector<Plane*> >(mPlanes.begin(), mPlanes.end());
   }
   
   Displacement*  createDisplacement(const Ogre::Vector3& position, const Ogre::Vector3& scale, const Ogre::Quaternion& orientation = Ogre::Quaternion::IDENTITY, size_t materialIndex = 0);
   
   void   destroyDisplacement(Displacement*);
   
   Ogre::VectorIterator< std::vector<Displacement*> >  getDisplacements()
   {
    return Ogre::VectorIterator< std::vector<Displacement*> >(mDisplacements.begin(), mDisplacements.end());
   }
   
   Block*  createBlock(const Ogre::Vector3& position, const Ogre::Vector3& size, const Ogre::Quaternion& orientation = Ogre::Quaternion::IDENTITY, size_t materialIndex = 0);
   
   void   destroyBlock(Block*);
   
   Ogre::VectorIterator< std::vector<Block*> >  getBlock()
   {
    return Ogre::VectorIterator< std::vector<Block*> >(mBlocks.begin(), mBlocks.end());
   }

   /*! function. _renderVertices
       desc.
           Bundle up mIndexData (redraw any if needed) then copy them
           into mVertexBuffer, and update mRenderOpPtr with the new 
           vertex count.
   */
   void _renderVertices();
   
   /*! function. getBoundingBox
   */
   const Ogre::AxisAlignedBox& getBoundingBox() const
   {
    return mAABB;
   }
   
   /*! function. getBoundingRadius
   */
   Ogre::Real getBoundingRadius() const
   {
    return 0; // TODO
   }
   
   /*! function. _updateRenderQueue
   */
   void _updateRenderQueue(Ogre::RenderQueue* queue);
   
   /*! function. visitRenderables
   */
   void visitRenderables(Ogre::Renderable::Visitor *,bool);
   
   void redrawNeeded(size_t index)
   {
    mRedrawNeeded = true;
    mGeometries[index]->mRedrawNeeded = true;
    if (mParentNode)
     mParentNode->needUpdate();
   }
   
   void loadFromOokFile(const Ogre::String& filename, const Ogre::String& resourceGroup = Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
   
   void saveAsOokFile(const Ogre::String& filename);
   
   void saveAsMesh(const Ogre::String& filename);
   
  protected:
   
   Geometry(const Ogre::String& name);
   
  ~Geometry();
   
   /// mSubRenderables -- All SubRenderables organised by material index.
   GeometryRenderables  mGeometries;
   
   /// mPlanes -- Master copy of all Planes.
   std::vector<Plane*>  mPlanes;
   
   /// mPlanes -- Master copy of all Displacements.
   std::vector<Displacement*>  mDisplacements;
   
   /// mBlocks -- Master copy of all Blocks.
   std::vector<Block*>  mBlocks;

   /// mRedrawNeeded -- One of the Geometry renderables need a redraw
   bool mRedrawNeeded;
   
   Ogre::AxisAlignedBox mAABB;
 };
 
 class Brush
 {
   
  public:
   
   Brush(Geometry* geom, size_t index) : mGeometry(geom), mIndex(index) {}
   
   virtual ~Brush() {}

   size_t getIndex() const { return mIndex; }

   virtual void _render(buffer<Vertex>&, buffer<Index>&) {}
   
   void redrawNeeded() { mGeometry->redrawNeeded(mIndex); }
   
   inline const Ogre::AxisAlignedBox& getAABB() const { return mAABB; }
   
  protected:
   
   void _setIndex(size_t newIndex) { mIndex = newIndex; redrawNeeded(); }
   
   Geometry*            mGeometry;
   size_t               mIndex;
   Ogre::Matrix4        mTransform;
   Ogre::AxisAlignedBox mAABB;
   
 };
 
 class MultiBrush
 {
   
  public:
   
   MultiBrush(Geometry* geom) : mGeometry(geom) {}
   
   virtual ~MultiBrush() {}

   virtual void _render(buffer<Vertex>&, buffer<Index>&, size_t materialIndex) {}
   
   void redrawNeeded(size_t index) { mGeometry->redrawNeeded(index); }
   
   inline const Ogre::AxisAlignedBox& getAABB() const { return mAABB; }
   
  protected:
   
   Geometry*            mGeometry;
   Ogre::Matrix4        mTransform;
   Ogre::AxisAlignedBox mAABB;
   
 };
 
 class Quad
 {
    
  public:
    
    Quad(const Ogre::Vector3& position, const Ogre::Vector2& size, const Ogre::Quaternion& orientation, Ogre::AxisAlignedBox* aabb);
    
   ~Quad() {}
    
    void _render(buffer<Vertex>&, buffer<Index>&);
    
    void _update();
    
    static const Ogre::Vector3  QUAD_VERTICES[4];
    
    Ogre::Matrix4      mTransform;
    Ogre::Vector3      mSize;
    Ogre::Quaternion   mOrientation;
    Ogre::Vector3      mPosition;
    Ogre::Vector2      mTextureZoom;
    Ogre::Vector2      mTextureOffset;
    bool               mTextureFlipX, mTextureFlipY;
    Ogre::Radian       mTextureAngle;
    Ogre::ColourValue  mColours[4];
    Ogre::AxisAlignedBox* mAABB;
    /* variable. mVertices
       desc.
           Pre-cached vertices to draw on to screen.
            
           Quad Structure
           --------------
           
             A------B
             | 1  / |   A = 0, B = 1, C = 2, D = 3
             |  /   |
             |/   2 |
             C------D
             
             0 = C \
             1 = A  | -- Triangle 1 
             2 = B /
             3 = C \
             4 = B  | -- Triangle 2
             5 = D /
    */
    Vertex             mVertices[4];
 };
 
 class Plane : public Brush, public Ogre::GeneralAllocatedObject
 {
   
   public:
   
  friend class Geometry;
   
   Plane(const Ogre::Vector3& position, const Ogre::Vector2& size, const Ogre::Quaternion& orientation, size_t materialIndex, Geometry*);
    
  ~Plane() {delete mQuad;}
   
   void _render(buffer<Vertex>&, buffer<Index>&);
   
   void _updateRequired()
   {
    mAABB.setNull();
    mQuad->_update();
    redrawNeeded();
   }
   
   void  position(const Ogre::Vector3& position)
   {
    mQuad->mPosition = position;
    _updateRequired();
   }
   
   void saveToOok(std::ofstream& stream);
   
  protected:
    
    Quad* mQuad;
    
 };
 
 /* class. Displacement
    desc.
        A heightfield made up of "samples" which are various points on the heightfield
        grid consisting of a height and colour.
 */
 class Displacement : public Brush, public Ogre::GeneralAllocatedObject
 {
   
  public:
   
   Displacement(const Ogre::Vector3& position, const Ogre::Vector3& scale, const Ogre::Quaternion& orientation, size_t materialIndex, Geometry*);
   
  ~Displacement();
   
   void saveToOok(std::ofstream& stream);
   
   void _render(buffer<Vertex>&, buffer<Index>&);
   
   void _updateRequired();
   
   /*! function. setHeight
       desc.
            Set a height directly.
       note.
            If your setting heights in bulk, use the begin/sample/end functions as each call of
            this will redraw the displacement. begin/sample/end will only redraw at the end
            of defining the terrain.
   */
   void setHeight(size_t x, size_t y, float height)
   {
    if (x > mLengthX || y > mLengthY)
     return;
    mHeights[x + (y * mLengthX)] = height;
    _updateRequired();
   }
   
   /*! function. setHeight
       desc.
            Set a height directly.
       note.
            If your setting heights in bulk, use the begin/sample/end functions as each call of
            this will redraw the displacement. begin/sample/end will only redraw at the end
            of defining the terrain.
   */
   void setHeight(size_t x, size_t y, float height, const Ogre::ColourValue& colour)
   {
    if (x > mLengthX || y > mLengthY)
     return;
    mHeights[x + (y * mLengthX)] = height;
    mColours[x + (y * mLengthX)] = colour;
    _updateRequired();
   }
   
   /*! function. setColour
       desc.
            Set a colour directly.
       note.
            If your setting heights in bulk, use the begin/sample/end functions as each call of
            this will redraw the displacement. begin/sample/end will only redraw at the end
            of defining the terrain.
   */
   void setColour(size_t x, size_t y, const Ogre::ColourValue& colour)
   {
    if (x > mLengthX || y > mLengthY)
     return;
    mColours[x + (y * mLengthX)] = colour;
    _updateRequired();
   }
   
   /*! function. getHeight
       desc.
            get a vertex Height
   */
   float getHeight(size_t x, size_t y) const
   {
    if (x > mLengthX || y > mLengthY)
     return 0.0f;
    return mHeights[x + (y * mLengthX)];
   }
   
   /*! function. getColour
       desc.
            Get a colour.
   */
   Ogre::ColourValue getColour(size_t x, size_t y) const
   {
    if (x > mLengthX || y > mLengthY)
     return Ogre::ColourValue::White;
    return mColours[x + (y * mLengthX)];
   }
   
   /*! function. getHeights
       desc.
            Get all heights.
   */
   void getHeights(buffer<float>& copy_to)
   {
    copy_to.remove_all();
    for (size_t i=0;i < mHeights.size();i++)
    {
     copy_to.push_back(mHeights[i]);
    }
   }
   
   /*! desc. getColours
            Get all colours.
   */
   void getColours(buffer<Ogre::ColourValue>& copy_to)
   {
    copy_to.remove_all();
    for (size_t i=0;i < mColours.size();i++)
    {
     copy_to.push_back(mColours[i]);
    }
   }


   /*! desc. begin
             Start (or restart) describing the displacement.
       args.
            lengthX -- Number of quads - 1 across the terrain.
            lengthY -- Number of quads - 1 down the terrain.
   */
   void begin(size_t lengthX, size_t lengthY)
   {
    mHeights.remove_all();
    mLengthX = lengthX;
    mLengthY = lengthY;
    mDescribing = true;
   }

   void sample(float height)
   {
    if (!mDescribing)
     return;
    mHeights.push_back(height);
    mColours.push_back(Ogre::ColourValue::White);
   }
   
   void sample(float height, const Ogre::ColourValue& colour)
   {
    if (!mDescribing)
     return;
    mHeights.push_back(height);
    mColours.push_back(colour);
   }

   void end()
   {
    
    if (mHeights.size() < (mLengthX * mLengthY))
    {
     size_t diff = (mLengthX * mLengthY) - mHeights.size();
     for (size_t i=0;i < diff;i++)
      mHeights.push_back(0.0f);
    }
    
    mDescribing = false;
    _updateRequired();
   }
   
  protected:
   
   buffer<float>               mHeights;
   buffer<Ogre::ColourValue>  mColours;
   Ogre::uint                 mLengthX, mLengthY;
   Ogre::Vector3              mPosition;
   Ogre::Vector3              mScale;
   Ogre::Quaternion           mOrientation;
   Ogre::Vector2              mTextureZoom;
   Ogre::Vector2              mTextureOffset;
   Ogre::Radian               mTextureAngle;
   bool                       mTextureFlipX, mTextureFlipY;
   buffer<Vertex>             mVertices;
   buffer<Index>              mIndexes;
   bool                       mDescribing;
 };
 
class Block : public MultiBrush, public Ogre::GeneralAllocatedObject
{
  
 public:
   
   enum QuadID
   {
    Quad_Top,
    Quad_Bottom,
    Quad_Front,
    Quad_Back,
    Quad_Left,
    Quad_Right
   };
   
   Block(const Ogre::Vector3& position, const Ogre::Vector3& size, const Ogre::Quaternion& orientation, size_t materialIndex, Geometry*);
   
  ~Block();
   
   void _render(buffer<Vertex>&, buffer<Index>&, size_t index);
   
   void _updateRequired();
   
   void quad_show(QuadID id)
   {
    mHasQuads[id] = true;
    _updateRequired();
    redrawNeeded(mQuadMaterial[id]);
   }
   
   void quad_hide(QuadID id)
   {
    mHasQuads[id] = false;
    _updateRequired();
    redrawNeeded(mQuadMaterial[id]);
   }

   void quad_index(QuadID id, size_t index)
   {
    mHasQuads[id] = true;
   size_t old_index = mQuadMaterial[id];
    mQuadMaterial[id] = index;
    _updateRequired();
    redrawNeeded(index);
    //redrawNeeded(old_index);
   }

 protected:
   
   bool                          mHasQuads[6];
   Ogre::Vector3                 mPosition, mSize;
   Ogre::Quaternion              mOrientation;
   struct QuadVertexData
   {
    Vertex                       mVertices[4];
    Index                        mIndexes[6];
   };
   QuadVertexData                mQuadVertexData[6];
   size_t                        mQuadMaterial[6];
   Ogre::Vector2                 mQuadTextureScale[6];
   Ogre::Vector2                 mQuadTextureOffset[6];
   bool                          mQuadTextureFlipX[6];
   bool                          mQuadTextureFlipY[6];
   Ogre::ColourValue             mQuadTextureColour[6];
   
   static const Ogre::Vector3    BLOCK_VERTICES[8];
   
};

} // namespace Orangutan

#endif
