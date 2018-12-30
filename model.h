#include <stddef.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
///Тип узла
///////////////////////////////////////////////////////////////////////////////////////////////////
enum ModelNodeType
{
  NODE_BOX,          //параллелипиппед
  NODE_SPHERE,       //сфера
  NODE_CYLINDER,     //цилиндр
  NODE_CONE,         //конус
  NODE_UNION,        //булево объединение
  NODE_INTERSECTION, //булево пересечение
  NODE_SUBTRACTION   //булево вычитание
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///Узел модели
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNode
{
  ModelNodeType type;              //тип узла  
  size_t        reference_counter; //количество ссылок на данный узел модели (для корректного удаления)
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///Преобразование тела в пространстве
///////////////////////////////////////////////////////////////////////////////////////////////////
struct Transform
{
  float      sx, sy, sz;             //коэффициенты масштабирования по осям
  float      axis_x, axis_y, axis_z; //ось поворота
  float      angle;                  //угол поворота в градусах
  float      dx, dy, dz;             //координаты вектора перемещения
  Transform* next;                   //следующее преобразование (используется при раборе CSG-выражений)
};

/*
    Виды улов модели
*/

///////////////////////////////////////////////////////////////////////////////////////////////////
///Примитив
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodePrimitive: ModelNode
{
  Transform* first_transform;  //список афинных преобразований над примитивом
  float      red, green, blue; //цвет примитива
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///Параллелипиппед
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodeBox: ModelNodePrimitive
{
  float width;  //ширина
  float height; //высота
  float depth;  //глубина
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///Сфера
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodeSphere: ModelNodePrimitive
{
  float  radius; //радиус сферы
  size_t slices; //количество разбиений
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///Цилиндр
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodeCylinder: ModelNodePrimitive
{
  float  radius; //радиус цилиндра
  float  height; //высота цилиндра
  size_t slices; //количество разбиений
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///Конус
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodeCone: ModelNodePrimitive
{
  float  radius; //радиус конуса
  float  height; //высота конуса
  size_t slices; //количество разбиений
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///Булева операция
///////////////////////////////////////////////////////////////////////////////////////////////////
struct ModelNodeBoolean: ModelNode
{
  ModelNode *a, *b; //операнды
};

/*
    Выполнение операций над узлами модели
*/

ModelNode* csgCreateBox          (float width,float height,float depth);
ModelNode* csgCreateSphere       (float radius,size_t slices);
ModelNode* csgCreateCylinder     (float radius,float height,size_t slices);
ModelNode* csgCreateCone         (float radius,float height,size_t slices);
ModelNode* csgCreateUnion        (ModelNode* a,ModelNode* b);
ModelNode* csgCreateIntersection (ModelNode* a,ModelNode* b);
ModelNode* csgCreateSubtraction  (ModelNode* a,ModelNode* b);

/*
    Копирование узла модели
*/

ModelNode* csgCopy     (ModelNode*);
ModelNode* csgInstance (ModelNode*,size_t add_ref_count=1); //увеличение числа ссылок (без полного копирования)
void       csgRelease  (ModelNode*);

/*
    Проверка класса узла: примитив или булева операция
*/

bool csgIsPrimitive (ModelNode*);
bool csgIsBoolean   (ModelNode*);

/*
    Преобразования над примитивом (для булевых узлов игнорируются)
*/

void csgTranslate (ModelNode*,float dx,float dy,float dz);
void csgRotate    (ModelNode*,float angle,float axis_x,float axis_y,float axis_z);
void csgScale     (ModelNode*,float sx,float sy,float sz);
void csgSetColor  (ModelNode*,float red,float green,float blue);

/*
    Нормализация CSG-дерева
*/

ModelNode* csgNormalize (ModelNode* tree);

/*
    Создание CSG-дерева по текстовому выражению на языке Lua
*/

ModelNode* csgParseExpression (const char* expression,const char* name=0);
ModelNode* csgParseFile       (const char* file_name);

/*
    Рисование дерева
*/

struct RenderModel; //визуализируемая модель, построенная по CSG дереву

RenderModel* csgCreateRenderModel (ModelNode*);
void         csgRelease           (RenderModel*);
void         csgDraw              (RenderModel*,bool draw_csg=true);

/*
    Средства отладки
*/

void   csgDump (ModelNode*);
size_t csgTotalPrimitivesCount (ModelNode*);
size_t csgTotalBooleansCount   (ModelNode*);
