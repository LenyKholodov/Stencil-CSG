#include <stdio.h>

#include "model.h"

/*
    Удаление / копирование
*/

//отладочная статистика
static size_t total_primitives_count = 0; //полное количество примитивов
static size_t total_booleans_count   = 0; //полное количество булевых операций

ModelNode* csgInstance (ModelNode* node,size_t add_ref_count)
{
  if (!node)
    return 0;

  node->reference_counter += add_ref_count;
  
  return node;
}

static Transform* csgCopy (Transform* first_transform)
{
  if (!first_transform)
    return 0;

  Transform* new_transform = new Transform;  

  *new_transform      = *first_transform;
  new_transform->next = csgCopy (first_transform->next);

  return new_transform;
}

ModelNode* csgCopy (ModelNode* node)
{
  if (!node)
    return 0;

  if (csgIsBoolean (node))
  {
    ModelNodeBoolean *new_node = new ModelNodeBoolean,
                     *op       = (ModelNodeBoolean*)node;

    new_node->type              = node->type;
    new_node->a                 = csgCopy (op->a);
    new_node->b                 = csgCopy (op->b);    
    new_node->reference_counter = 1;    
    
    total_booleans_count++;    
    
    return new_node;
  }
  
  total_primitives_count++;
  
  switch (node->type)
  {
    case NODE_BOX:
    {  
      ModelNodeBox *primitive = (ModelNodeBox*)node,
                   *new_node  = new ModelNodeBox;

      *new_node                   = *primitive;
      new_node->first_transform   = csgCopy (primitive->first_transform);
      new_node->reference_counter = 1;      

      return new_node;
    }
    case NODE_SPHERE:
    {  
      ModelNodeSphere *primitive = (ModelNodeSphere*)node,
                      *new_node  = new ModelNodeSphere;

      *new_node                   = *primitive;
      new_node->first_transform   = csgCopy (primitive->first_transform);
      new_node->reference_counter = 1;            

      return new_node;
    }
    case NODE_CYLINDER:
    {  
      ModelNodeCylinder *primitive = (ModelNodeCylinder*)node,
                        *new_node  = new ModelNodeCylinder;

      *new_node                   = *primitive;
      new_node->first_transform   = csgCopy (primitive->first_transform);
      new_node->reference_counter = 1;

      return new_node;
    }      
    case NODE_CONE:
    {  
      ModelNodeCone *primitive = (ModelNodeCone*)node,
                    *new_node  = new ModelNodeCone;

      *new_node                   = *primitive;
      new_node->first_transform   = csgCopy (primitive->first_transform);
      new_node->reference_counter = 1;

      return new_node;
    }
    default:
      return 0;
  }
}

void csgRelease (ModelNode* node)
{
  if (!node)
    return;    
    
  if (!--node->reference_counter) //если на данный узел больше никто не ссылается - удаляем его
  {    
    if (csgIsBoolean (node))
    {
      ModelNodeBoolean* op = (ModelNodeBoolean*)node;

      csgRelease (op->a);
      csgRelease (op->b);
      
      total_booleans_count--;
    }

    if (csgIsPrimitive (node))
    {
      ModelNodePrimitive* primitive = (ModelNodePrimitive*)node;
      
      for (Transform* transform=primitive->first_transform;transform;)
      {
        Transform* next = transform->next;
        
        delete transform;
        
        transform = next;
      }
      
      total_primitives_count--;
    }
    
    delete node;
  }
}

/*
    Создание примитивов
*/

static void csgInitPrimitive (ModelNodePrimitive* primitive)
{
  primitive->first_transform = 0;
  primitive->red             = 1.0;
  primitive->green           = 1.0;
  primitive->blue            = 1.0;
  
  total_primitives_count++;
}

ModelNode* csgCreateBox (float width,float height,float depth)
{
  ModelNodeBox* node = new ModelNodeBox;

  node->type              = NODE_BOX;
  node->width             = width;
  node->height            = height;
  node->depth             = depth;
  node->reference_counter = 1;
  
  csgInitPrimitive (node);
  
  return node;
}

ModelNode* csgCreateSphere (float radius,size_t slices)
{
  ModelNodeSphere* node = new ModelNodeSphere;

  node->type              = NODE_SPHERE;
  node->radius            = radius;
  node->slices            = slices;
  node->reference_counter = 1;  
  
  csgInitPrimitive (node);
  
  return node;  
}

ModelNode* csgCreateCylinder (float radius,float height,size_t slices)
{
  ModelNodeCylinder* node = new ModelNodeCylinder;

  node->type              = NODE_CYLINDER;
  node->radius            = radius;
  node->height            = height;
  node->slices            = slices;
  node->reference_counter = 1;  
  
  csgInitPrimitive (node);  

  return node;
}

ModelNode* csgCreateCone (float radius,float height,size_t slices)
{
  ModelNodeCone* node = new ModelNodeCone;

  node->type              = NODE_CONE;
  node->radius            = radius;
  node->height            = height;
  node->slices            = slices;
  node->reference_counter = 1;  
  
  csgInitPrimitive (node);

  return node;
}

/*
    Создание булевых операций
*/

static ModelNode* csgCreateBoolean (ModelNodeType type,ModelNode* a,ModelNode* b)
{
  ModelNodeBoolean* node = new ModelNodeBoolean;

  node->type              = type;
  node->a                 = csgInstance (a);
  node->b                 = csgInstance (b);
  node->reference_counter = 1;
  
  total_booleans_count++;

  return node;
}

ModelNode* csgCreateUnion (ModelNode* a,ModelNode* b)
{
  return csgCreateBoolean (NODE_UNION,a,b);
}

ModelNode* csgCreateIntersection (ModelNode* a,ModelNode* b)
{
  return csgCreateBoolean (NODE_INTERSECTION,a,b);
}

ModelNode* csgCreateSubtraction (ModelNode* a,ModelNode* b)
{
  return csgCreateBoolean (NODE_SUBTRACTION,a,b);
}

/*
    Проверка класса узла
*/

bool csgIsBoolean (ModelNode* node)
{
  switch (node->type)
  {
    case NODE_INTERSECTION:
    case NODE_UNION:
    case NODE_SUBTRACTION:  return true;
    default:                return false;
  }
}

bool csgIsPrimitive (ModelNode* node)
{
  switch (node->type)
  {
    case NODE_BOX:
    case NODE_SPHERE:
    case NODE_CYLINDER:
    case NODE_CONE:     return true;
    default:            return false;
  }
}

/*
    Изменение свойств примитива (для непримитивов игниорируются)
*/

static Transform* csgCreateTranform (ModelNodePrimitive* owner)
{
  Transform* transform = new Transform;
  
  transform->dx     = 0.0;
  transform->dy     = 0.0;
  transform->dz     = 0.0;
  transform->axis_x = 0.0;
  transform->axis_y = 0.0;
  transform->axis_z = 1.0;  
  transform->angle  = 0.0;
  transform->sx     = 1.0;
  transform->sy     = 1.0;
  transform->sz     = 1.0;
  transform->next   = owner->first_transform;
  
  owner->first_transform = transform;
  
  return transform;
}

void csgTranslate (ModelNode* node,float dx,float dy,float dz)
{
  if (!csgIsPrimitive (node))
    return;

  Transform* transform = csgCreateTranform ((ModelNodePrimitive*)node);

  transform->dx = dx;
  transform->dy = dy;
  transform->dz = dz;
}

void csgRotate (ModelNode* node,float angle,float axis_x,float axis_y,float axis_z)
{
  if (!csgIsPrimitive (node))
    return;

  Transform* transform = csgCreateTranform ((ModelNodePrimitive*)node);

  transform->axis_x = axis_x;
  transform->axis_y = axis_y;
  transform->axis_z = axis_z;
  transform->angle  = angle;
}

void csgScale (ModelNode* node,float sx,float sy,float sz)
{
  if (!csgIsPrimitive (node))
    return;

  Transform* transform = csgCreateTranform ((ModelNodePrimitive*)node);

  transform->sx = sx;
  transform->sy = sy;
  transform->sz = sz;
}

void csgSetColor (ModelNode* node,float red,float green,float blue)
{
  if (!csgIsPrimitive (node))
    return;
    
  ModelNodePrimitive* primitive = (ModelNodePrimitive*)node;

  primitive->red   = red;
  primitive->green = green;
  primitive->blue  = blue;
}

/*
    Преобразование булевой операции
*/

static void csgReplace (ModelNodeBoolean* node,ModelNodeType new_type,ModelNode* new_a,ModelNode* new_b)
{
  ModelNode *old_a = node->a, *old_b = node->b;

  node->type = new_type;
  node->a    = new_a;
  node->b    = new_b;

  csgRelease (old_a);
  csgRelease (old_b);
}

/*
    Нормализация CSG-дерева
*/

static bool csgNormalizeNode (ModelNode* node)
{
  ModelNodeBoolean* op = (ModelNodeBoolean*)node;
  bool conversion = false;

  switch (node->type)
  {
    case NODE_SUBTRACTION:
    {
      if (op->a->type == NODE_UNION) //rule #8
      {
        ModelNodeBoolean *a = (ModelNodeBoolean*)op->a;
        ModelNode        *x = a->a, *y = a->b, *z = op->b;
                          
        conversion = true;
        
        csgReplace (op,NODE_UNION,csgCreateSubtraction (x,z),csgCreateSubtraction (y,z));
      }
      
      if (csgIsBoolean (op->b))
      {                
        ModelNodeBoolean *b = (ModelNodeBoolean*)op->b;
        ModelNode        *x = op->a,
                         *y = b->a,
                         *z = b->b;
        
        switch (b->type)
        {
          case NODE_UNION: //rule #1
            conversion = true;
            
            csgReplace (op,NODE_SUBTRACTION,csgCreateSubtraction (x,y),csgInstance (z));
            
            break;
          case NODE_INTERSECTION: //rule #3
            conversion = true;            
            
            csgReplace (op,NODE_UNION,csgCreateSubtraction (x,y),csgCreateSubtraction (x,z));

            break;
          case NODE_SUBTRACTION: //rule #5
            conversion = true;            
            
            csgReplace (op,NODE_UNION,csgCreateSubtraction (x,y),csgCreateIntersection (x,z));

            break;
        }
      }
      
      break;
    }
    case NODE_INTERSECTION:
    {      
      if (csgIsBoolean (op->a))
      {
        ModelNodeBoolean *a = (ModelNodeBoolean*)op->a;
        ModelNode        *x = a->a,
                         *y = a->b,
                         *z = op->b;
        
        switch (a->type)
        {
          case NODE_SUBTRACTION: //rule #7
            conversion = true;            
            
            csgReplace (op,NODE_SUBTRACTION,csgCreateIntersection (x,z),csgInstance (y));
            
            break;
          case NODE_UNION: //rule #9
            conversion = true;
            
            csgReplace (op,NODE_UNION,csgCreateIntersection (x,z),csgCreateIntersection (y,z));
            
            break;
        }
      }
      
      if (csgIsBoolean (op->b))
      {
        ModelNodeBoolean *b = (ModelNodeBoolean*)op->b;
        ModelNode        *x = op->a,
                         *y = b->a,
                         *z = b->b;
                         
        switch (b->type)
        {
          case NODE_UNION: //rule #2
            conversion = true;            
            
            csgReplace (op,NODE_UNION,csgCreateIntersection (x,y),csgCreateIntersection (x,z));
            break;
          case NODE_INTERSECTION: //rule #4
            conversion = true;            
            
            csgReplace (op,NODE_INTERSECTION,csgCreateIntersection (x,y),csgInstance (z));
            break;
          case NODE_SUBTRACTION: //rule #6
            conversion = true;            
            
            csgReplace (op,NODE_SUBTRACTION,csgCreateIntersection (x,y),csgInstance (z));
            break;
        }
      }
    }
  }
  
  return conversion;
}

void csgNormalizeTree (ModelNode* node)
{
  csgNormalizeNode (node);
  
  do
  {
    if (csgIsBoolean (node))
    {
      ModelNodeBoolean* op = (ModelNodeBoolean*)node;
      
      csgNormalizeTree (op->a);
      csgNormalizeTree (op->b);
    }
  } while (csgNormalizeNode (node));  
}

ModelNode* csgNormalize (ModelNode* node)
{
  ModelNode* new_node = csgCopy (node);
    
  csgNormalizeTree (new_node);  

  return new_node;
}

/*
    Средства отладки
*/

static void csgDumpNode (ModelNode* node)
{
  switch (node->type)
  {
    case NODE_BOX:
    {
      ModelNodeBox* box = (ModelNodeBox*)node;
      
      printf ("box(%g,%g,%g)",box->width,box->height,box->depth); 
      break;
    }
    case NODE_SPHERE:
    {
      ModelNodeSphere* sphere = (ModelNodeSphere*)node;
      
      printf ("sphere(%g,%d)",sphere->radius,sphere->slices); 
      break;
    }
    case NODE_CYLINDER:
    {
      ModelNodeCylinder* cylinder = (ModelNodeCylinder*)node;
      
      printf ("cylinder(%g,%g,%d)",cylinder->radius,cylinder->height,cylinder->slices); 
      break;
    }
    case NODE_CONE:
    {
      ModelNodeCone* cone = (ModelNodeCone*)node;
      
      printf ("cone(%g,%g,%d)",cone->radius,cone->height,cone->slices);
      break;
    }
    case NODE_UNION:
    case NODE_INTERSECTION:
    case NODE_SUBTRACTION:
    {
      ModelNodeBoolean* op = (ModelNodeBoolean*)node;      
      
      bool brackets = false;
      
      switch (op->a->type)
      {
        case NODE_UNION:
        case NODE_INTERSECTION:
        case NODE_SUBTRACTION:
          brackets = true;
          break;        
      }
      
      if (brackets)
      {
        printf ("(");
        csgDumpNode (op->a);
        printf (")");
      }
      else csgDumpNode (op->a);
            
      switch (node->type)
      {
        case NODE_UNION:        printf ("|"); break;
        case NODE_INTERSECTION: printf ("&"); break;
        case NODE_SUBTRACTION:  printf ("-"); break;
      }
      
      brackets = false;
      
      switch (op->b->type)
      {
        case NODE_UNION:
        case NODE_INTERSECTION:
        case NODE_SUBTRACTION:
          brackets = true;
          break;        
      }
      
      if (brackets)
      {
        printf ("(");
        csgDumpNode (op->b);
        printf (")");
      }
      else csgDumpNode (op->b);      
      break;
    }
    default:
      printf ("UNKNOWN");
      break;
  }
  
//  printf ("<ref=%d>",node->reference_counter);
}

void csgDump (ModelNode* node)
{
  csgDumpNode (node);
}

size_t csgTotalPrimitivesCount (ModelNode* node)
{
  if (csgIsBoolean (node))
  {
    ModelNodeBoolean* op = (ModelNodeBoolean*)node;        
    
    return csgTotalPrimitivesCount (op->a) + csgTotalPrimitivesCount (op->b);
  }
  
  return 1;
}

size_t csgTotalBooleansCount (ModelNode* node)
{
  if (csgIsBoolean (node))
  {
    ModelNodeBoolean* op = (ModelNodeBoolean*)node;        
    
    return csgTotalBooleansCount (op->a) + csgTotalBooleansCount (op->b) + 1;
  }
  
  return 0;
}
