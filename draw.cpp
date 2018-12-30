#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <functional>

#include "gl/glew.h"
#include "gl/glut.h"
#include "gl/freeglut_ext.h"
#include "opencsg.h"

#include "model.h"

typedef std::vector<OpenCSG::Primitive*> PrimitiveList; //массив примитивов
typedef std::vector<PrimitiveList>       SubmodelList;  //список подмоделей

///////////////////////////////////////////////////////////////////////////////////////////////////
///Визуализируемая модель
///////////////////////////////////////////////////////////////////////////////////////////////////
struct RenderModel
{
  SubmodelList submodels;  //подмодели (объединяемые через union)  
};

/*
    Рисование базовых примитивов
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
///Базовый класс для визуализируемых примитивов
///////////////////////////////////////////////////////////////////////////////////////////////////
class CSGPrimitive: public OpenCSG::Primitive
{
  public:
    CSGPrimitive (ModelNodePrimitive* node,OpenCSG::Operation operation) : 
      Primitive (operation,1), primitive (*(ModelNodePrimitive*)csgInstance (node)) {}
      
    ~CSGPrimitive () { csgRelease (&primitive); }
    
    void render ()
    {
      glPushMatrix          ();
      apply_transformations ();
      render_primitive      ();
      glPopMatrix           ();
    }
    
    void colored_render ()
    {
      glColor3f (primitive.red,primitive.green,primitive.blue);
      render    ();
    }
    
  private:
    void apply_transformations () {
      for (Transform* t=primitive.first_transform;t;t=t->next)
      {
        glTranslatef (t->dx,t->dy,t->dz);
        glRotatef    (t->angle,t->axis_x,t->axis_y,t->axis_z);
        glScalef     (t->sx,t->sy,t->sz);        
      }
    }
    
    virtual void render_primitive () = 0;
  
  protected:
    ModelNodePrimitive& primitive;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///Параллелипиппед
///////////////////////////////////////////////////////////////////////////////////////////////////
class BoxPrimitive: public CSGPrimitive
{
  public:
    BoxPrimitive (ModelNodePrimitive& node,OpenCSG::Operation operation) : CSGPrimitive (&node,operation) {}
    
    void render_primitive ()
    {
      ModelNodeBox& box = (ModelNodeBox&)primitive;
      
      glPushMatrix  ();
      glScalef      (box.width,box.height,box.depth);
      glutSolidCube (1.0);
      glPopMatrix   ();
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///Сфера
///////////////////////////////////////////////////////////////////////////////////////////////////
class SpherePrimitive: public CSGPrimitive
{
  public:
    SpherePrimitive (ModelNodePrimitive& node,OpenCSG::Operation operation) : CSGPrimitive (&node,operation) {}
    
    void render_primitive ()
    {
      ModelNodeSphere& sphere = (ModelNodeSphere&)primitive;
      
      glutSolidSphere (sphere.radius,sphere.slices,sphere.slices);
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///Конус
///////////////////////////////////////////////////////////////////////////////////////////////////
class ConePrimitive: public CSGPrimitive
{
  public:
    ConePrimitive (ModelNodePrimitive& node,OpenCSG::Operation operation) : CSGPrimitive (&node,operation) {}
    
    void render_primitive ()
    {
      ModelNodeCone& cone = (ModelNodeCone&)primitive;
      
      glPushMatrix  ();      
      glRotatef     (-90,1,0,0);      
      glutSolidCone (cone.radius,cone.height,cone.slices,cone.slices);
      glPopMatrix   ();

      return; 
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///Цилиндр
///////////////////////////////////////////////////////////////////////////////////////////////////
class CylinderPrimitive: public CSGPrimitive
{
  public:
    CylinderPrimitive (ModelNodePrimitive& node,OpenCSG::Operation operation) : CSGPrimitive (&node,operation) {}
    
    void render_primitive ()
    {
      ModelNodeCylinder& cylinder = (ModelNodeCylinder&)primitive;

      glPushMatrix      ();      
      glRotatef         (-90,1,0,0);
      glutSolidCylinder (cylinder.radius,cylinder.height,cylinder.slices,1);
      glPopMatrix       ();
      
      return;
    }
};

/*
    Построение визуализируемой модели по нормализованному дереву CSG
*/

static void csgBuildPrimitivesList (ModelNode* node,OpenCSG::Operation operation,PrimitiveList& primitives)
{
  if (csgIsPrimitive (node))
  {
    OpenCSG::Primitive* primitive = 0;
    
    switch (node->type)
    {
      case NODE_BOX:      primitive = new BoxPrimitive (*(ModelNodeBox*)node,operation); break;
      case NODE_SPHERE:   primitive = new SpherePrimitive (*(ModelNodeSphere*)node,operation); break;
      case NODE_CYLINDER: primitive = new CylinderPrimitive (*(ModelNodeCylinder*)node,operation); break;
      case NODE_CONE:     primitive = new ConePrimitive (*(ModelNodeCone*)node,operation); break;
      default:            return;
    }    
    
    primitives.push_back (primitive);
    
    return;
  }

  ModelNodeBoolean* op = (ModelNodeBoolean*)node;
  
  csgBuildPrimitivesList (op->a,operation,primitives);
    
  switch (op->type)
  {
    case NODE_SUBTRACTION:  csgBuildPrimitivesList (op->b,OpenCSG::Subtraction,primitives); break;
    case NODE_INTERSECTION: csgBuildPrimitivesList (op->b,OpenCSG::Intersection,primitives); break;
  }    
}

static void csgBuildSubmodels (ModelNode* node,RenderModel& model)
{
  if (node->type == NODE_UNION)
  {
    ModelNodeBoolean* op = (ModelNodeBoolean*)node;
    
    csgBuildSubmodels (op->a,model);
    csgBuildSubmodels (op->b,model);    
  }
  else
  { 
      //строим список примитивов модели
    
    model.submodels.push_back (PrimitiveList ());
      
    csgBuildPrimitivesList (node,OpenCSG::Intersection,model.submodels.back ());
  }
}

RenderModel* csgCreateRenderModel (ModelNode* node)
{
  RenderModel* model           = new RenderModel;    
  ModelNode*   normalized_node = csgNormalize (node);    
  
//  csgDump (normalized_node);

  csgBuildSubmodels (normalized_node,*model);
  csgRelease        (normalized_node);

  return model;
}

void csgRelease (RenderModel* model)
{
  if (!model)
    return;

  for (SubmodelList::iterator i=model->submodels.begin ();i!=model->submodels.end ();++i)
    for (PrimitiveList::iterator j=i->begin ();j!=i->end ();++j)
      delete *j;

  delete model;
}

/*
    Рисование визуализируемой модели
*/

void csgDraw (RenderModel* model,bool draw_csg)
{
  if (!model)
    return;

  for (SubmodelList::iterator i=model->submodels.begin ();i!=model->submodels.end ();++i)
  {
    PrimitiveList& primitives = *i;
    
    if (draw_csg)
    {
      OpenCSG::render (primitives);
      glDepthFunc     (GL_EQUAL);    
    }
    
    for (PrimitiveList::iterator j=primitives.begin ();j!=primitives.end ();++j)
      ((CSGPrimitive*)(*j))->colored_render ();          

    if (draw_csg) 
      glDepthFunc (GL_LESS);
  }  
}
