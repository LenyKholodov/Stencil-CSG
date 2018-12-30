#include <stddef.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
///��� 㧫�
///////////////////////////////////////////////////////////////////////////////////////////////////
enum ModelNodeType
{
  NODE_BOX,          //��ࠫ����������
  NODE_SPHERE,       //���
  NODE_CYLINDER,     //樫����
  NODE_CONE,         //�����
  NODE_UNION,        //�㫥�� ��ꥤ������
  NODE_INTERSECTION, //�㫥�� ����祭��
  NODE_SUBTRACTION   //�㫥�� ���⠭��
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///���� ������
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNode
{
  ModelNodeType type;              //⨯ 㧫�  
  size_t        reference_counter; //������⢮ ��뫮� �� ����� 㧥� ������ (��� ���४⭮�� 㤠�����)
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///�८�ࠧ������ ⥫� � ����࠭�⢥
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Transform
{
  float      sx, sy, sz;             //�����樥��� ����⠡�஢���� �� ���
  float      axis_x, axis_y, axis_z; //��� ������
  float      angle;                  //㣮� ������ � �ࠤ���
  float      dx, dy, dz;             //���न���� ����� ��६�饭��
  Transform* next;                   //᫥���饥 �८�ࠧ������ (�ᯮ������ �� ࠡ�� CSG-��ࠦ����)
};

/*
    ���� 㫮� ������
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
///�ਬ�⨢
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodePrimitive: ModelNode
{
  Transform* first_transform;  //ᯨ᮪ �䨭��� �८�ࠧ������ ��� �ਬ�⨢��
  float      red, green, blue; //梥� �ਬ�⨢�
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///��ࠫ����������
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodeBox: ModelNodePrimitive
{
  float width;  //�ਭ�
  float height; //����
  float depth;  //��㡨��
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///���
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodeSphere: ModelNodePrimitive
{
  float  radius; //ࠤ��� ����
  size_t slices; //������⢮ ࠧ������
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///�������
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodeCylinder: ModelNodePrimitive
{
  float  radius; //ࠤ��� 樫����
  float  height; //���� 樫����
  size_t slices; //������⢮ ࠧ������
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///�����
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodeCone: ModelNodePrimitive
{
  float  radius; //ࠤ��� �����
  float  height; //���� �����
  size_t slices; //������⢮ ࠧ������
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///�㫥�� ������
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodeBoolean: ModelNode
{
  ModelNode *a, *b; //���࠭��
};

/*
    �믮������ ����権 ��� 㧫��� ������
*/

ModelNode* csgCreateBox          (float width,float height,float depth);
ModelNode* csgCreateSphere       (float radius,size_t slices);
ModelNode* csgCreateCylinder     (float radius,float height,size_t slices);
ModelNode* csgCreateCone         (float radius,float height,size_t slices);
ModelNode* csgCreateUnion        (ModelNode* a,ModelNode* b);
ModelNode* csgCreateIntersection (ModelNode* a,ModelNode* b);
ModelNode* csgCreateSubtraction  (ModelNode* a,ModelNode* b);

/*
    ����஢���� 㧫� ������
*/

ModelNode* csgCopy     (ModelNode*);
ModelNode* csgInstance (ModelNode*,size_t add_ref_count=1); //㢥��祭�� �᫠ ��뫮� (��� ������� ����஢����)
void       csgRelease  (ModelNode*);

/*
    �஢�ઠ ����� 㧫�: �ਬ�⨢ ��� �㫥�� ������
*/

bool csgIsPrimitive (ModelNode*);
bool csgIsBoolean   (ModelNode*);

/*
    �८�ࠧ������ ��� �ਬ�⨢�� (��� �㫥��� 㧫�� �����������)
*/

void csgTranslate (ModelNode*,float dx,float dy,float dz);
void csgRotate    (ModelNode*,float angle,float axis_x,float axis_y,float axis_z);
void csgScale     (ModelNode*,float sx,float sy,float sz);
void csgSetColor  (ModelNode*,float red,float green,float blue);

/*
    ��ଠ������ CSG-��ॢ�
*/

ModelNode* csgNormalize (ModelNode* tree);

/*
    �������� CSG-��ॢ� �� ⥪�⮢��� ��ࠦ���� �� �몥 Lua
*/

ModelNode* csgParseExpression (const char* expression,const char* name=0);
ModelNode* csgParseFile       (const char* file_name);

/*
    ��ᮢ���� ��ॢ�
*/

struct RenderModel; //���㠫����㥬�� ������, ����஥���� �� CSG ��ॢ�

RenderModel* csgCreateRenderModel (ModelNode*);
void         csgRelease           (RenderModel*);
void         csgDraw              (RenderModel*,bool draw_csg=true);

/*
    �।�⢠ �⫠���
*/

void   csgDump (ModelNode*);
size_t csgTotalPrimitivesCount (ModelNode*);
size_t csgTotalBooleansCount   (ModelNode*);
