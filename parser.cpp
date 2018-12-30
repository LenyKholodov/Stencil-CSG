#include "model.h"
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <io.h>

extern "C"
{

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

}

/*
    Функция вызываемая в случае ошибок времени выполнения в lua-скрипте
*/

static void parRunTimeError (lua_State* state,const char* format,...)
{
  va_list list;
  
  va_start (list,format);  
  
  char buffer [512];
  
  _vsnprintf (buffer,sizeof (buffer),format,list);
  
  buffer [511] = '\0';
  
  luaL_error (state,"%s\n",buffer);
}

/*
    Работа со стеком lua
*/

static ModelNode* parTryGetModelNode (lua_State* state,int index)
{
  ModelNode** node_ptr = (ModelNode**)luaL_checkudata (state,index,"model_node");

  return node_ptr ? *node_ptr : 0;
}

static ModelNode* parGetModelNode (lua_State* state,int index)
{ 
  ModelNode** node_ptr = (ModelNode**)luaL_checkudata (state,index,"model_node");

  if (!node_ptr)
  {
    parRunTimeError (state,"Неверный тип аргумента %d. Ожидается ModelNode",index);
    return 0;
  }

  ModelNode* node = *node_ptr;  
  
  if (!node)
    parRunTimeError (state,"Попытка обращения к ModelNode, который не был инициализирован");    
    
  return node;
}

static void parPushModelNode (lua_State* state,ModelNode* node,bool need_instance=false)
{
  ModelNode** node_ptr = (ModelNode**)lua_newuserdata (state,sizeof (ModelNode*));
   
  *node_ptr = need_instance ?  csgInstance (node) : node;
  
  luaL_getmetatable (state,"model_node");
  lua_setmetatable  (state,-2);
}

/*
    Обработчик копирования/удаления ModelNode в lua
*/

static int parDestroyModelNode (lua_State* state)
{
  ModelNode** node_ptr = (ModelNode**)lua_touserdata (state,1);
  
  if (!node_ptr) 
    luaL_typerror (state,1,"model_node");    

  csgRelease (*node_ptr);
  
  return 0;
}

/*
    Создание примитивов
*/

static int parCreateBox (lua_State* state)
{
  double width  = luaL_checknumber (state,1),
         height = luaL_checknumber (state,2),
         depth  = luaL_checknumber (state,3);
         
  if (width < 0.0 || height < 0.0 || depth < 0.0)
  {
    parRunTimeError (state,"Некорректный вызов функции box(%g,%g,%g)."
                     "Размеры параллелипиппеда должны быть положительными числами",width,height,depth);

    return 0;
  }
  
  parPushModelNode (state,csgCreateBox (width,height,depth));

  return 1;
}

static int parCreateSphere (lua_State* state)
{
  double radius = luaL_checknumber (state,1),
         slices = luaL_optnumber (state,2,30);

  if (slices < 0.0 || radius < 0.0)
  {
    parRunTimeError (state,"Некорректный вызов функции sphere(%g,%g)."
                     "Количество разбиений и радиус должны быть положительными числами",radius,slices);

    return 0;
  }

  parPushModelNode (state,csgCreateSphere (radius,slices));

  return 1;
}

static int parCreateCylinder (lua_State* state)
{
  double radius = luaL_checknumber (state,1),
         height = luaL_checknumber (state,2),
         slices = luaL_optnumber   (state,3,30);  
  
  if (slices < 0.0 || radius < 0.0 || height < 0.0)
  {
    parRunTimeError (state,"Некорректный вызов функции cylinder(%g,%g,%g)."
                     "Количество разбиений и размеры цилиндра должны быть положительными числами",radius,height,slices);

    return 0;
  }

  parPushModelNode (state,csgCreateCylinder (radius,height,slices));

  return 1;
}

static int parCreateCone (lua_State* state)
{
  double radius = luaL_checknumber (state,1),
         height = luaL_checknumber (state,2),
         slices = luaL_optnumber   (state,3,30);  

  if (slices < 0.0 || radius < 0.0 || height < 0.0)
  {
    parRunTimeError (state,"Некорректный вызов функции cone(%g,%g,%g)."
                     "Количество разбиений и размеры конуса должны быть положительными числами",radius,height,slices);

    return 0;
  }

  parPushModelNode (state,csgCreateCone (radius,height,slices));

  return 1;
}

/*
    Преобразования над примитивом
*/

static int parTranslate (lua_State* state)
{
  ModelNode* node = parGetModelNode (state,1);
  
  csgTranslate     (node,luaL_checknumber (state,2),luaL_checknumber (state,3),luaL_checknumber (state,4));
  parPushModelNode (state,node,true);

  return 1;
}

static int parRotate (lua_State* state)
{
  ModelNode* node = parGetModelNode (state,1);
  
  csgRotate        (node,luaL_checknumber (state,2),luaL_checknumber (state,3),
                    luaL_checknumber (state,4),luaL_checknumber (state,5));
  parPushModelNode (state,node,true);

  return 1;
}

static int parScale (lua_State* state)
{
  ModelNode* node = parGetModelNode (state,1);
  
  csgScale         (node,luaL_checknumber (state,2),luaL_checknumber (state,3),luaL_checknumber (state,4));
  parPushModelNode (state,node,true);

  return 1;
}

/*
    Установка цвета примитива
*/

static int parSetColor (lua_State* state)
{
  ModelNode* node = parGetModelNode (state,1);
  
  csgSetColor      (node,luaL_checknumber (state,2),luaL_checknumber (state,3),luaL_checknumber (state,4));
  parPushModelNode (state,node,true);

  return 1;
}

/*
    Булевы операции
*/

static int parUnion (lua_State* state)
{  
  ModelNode *a = parGetModelNode (state,1),
            *b = parGetModelNode (state,2);
    
  parPushModelNode (state,csgCreateUnion (a,b));

  return 1;
}

static int parIntersection (lua_State* state)
{
  ModelNode *a = parGetModelNode (state,1),
            *b = parGetModelNode (state,2);
    
  parPushModelNode (state,csgCreateIntersection (a,b));

  return 1;
}

static int parSubtraction (lua_State* state)
{
  ModelNode *a = parGetModelNode (state,1),
            *b = parGetModelNode (state,2);
    
  parPushModelNode (state,csgCreateSubtraction (a,b));

  return 1;
}

/*
    Обработка исключительных ситуаций
*/

static int parProcessError (lua_State* state)
{
  printf ("Исключительная ситуация при разборе lua-выражения\n");
//  exit (0);
  
  return 0;
}

/*
    Разбор выражения
*/

//мета-таблица для введенного в lua типа данных 
static const luaL_reg model_node_meta_table [] = {
  {"__add",parUnion},
  {"__sub",parSubtraction},
  {"__mul",parIntersection},
  {"__gc",parDestroyModelNode},
  {0,0}
};

//таблица методов для введенного в lua типа данных
static const luaL_reg model_node_methods [] = {
  {"color",parSetColor},
  {"translate",parTranslate},
  {"rotate",parRotate},
  {"scale",parScale},
  {0,0}
};

static void parRegisterModelNode (lua_State* state)
{
    //регистрация мета-таблицы

  luaopen_table     (state);
  luaL_openlib      (state,"model_node",model_node_methods,0);
  luaL_newmetatable (state,"model_node");
  luaL_openlib      (state,0,model_node_meta_table,0);
  lua_pushliteral   (state,"__index");
  lua_pushvalue     (state,-3);
  lua_rawset        (state,-3);
  lua_pushliteral   (state,"__metatable");
  lua_pushvalue     (state,-3);
  lua_rawset        (state,-3);
  lua_pop           (state,1);

    //регистрация функций-расширений языка Lua для создания CSG дерева
  
  lua_register (state,"box",parCreateBox);  
  lua_register (state,"sphere",parCreateSphere);
  lua_register (state,"cylinder",parCreateCylinder);
  lua_register (state,"cone",parCreateCone);
  lua_register (state,"translate",parTranslate);
  lua_register (state,"rotate",parRotate);
  lua_register (state,"scale",parScale);  
  lua_register (state,"color",parSetColor);
  lua_register (state,"union",parUnion);
  lua_register (state,"intersection",parIntersection);
  lua_register (state,"subtraction",parSubtraction);
}

ModelNode* csgParseExpression (const char* expression,const char* name)
{
    //генерация обёртки над переданным выражением

  lua_State* state = lua_open ();  
  
    //регистрация обработчика исключительных ситуаций

  lua_atpanic (state,parProcessError);  
  
  parRegisterModelNode (state);          

    //вычисление значения выражения    
    
  int parse_error = lua_dobuffer (state,expression,strlen (expression),name?name:"expression");

  if (parse_error)
  {
    printf    ("Загрузка невозможна из-за некорретного формата модели '%s'\n",name?name:"expression");
    lua_close (state);

    return 0;
  }  
    
    //конвертация вычисленного значения

  ModelNode* node = csgInstance (parTryGetModelNode (state,-1));  
  
  lua_close (state);  
  
  if (!node)
  {
    printf ("Неверный тип возвращаемого значения в модели '%s'\n",name?name:"expression");
    return 0;
  }  
  
  return node;
}

ModelNode* csgParseFile (const char* file_name)
{
  FILE* file = fopen (file_name,"r");
  
  if (!file)
  {
    printf ("Файл '%s' не найден\n",file_name);
    return 0;
  }
  
  size_t file_size = _filelength (_fileno (file));
  
  char* buffer = (char*)malloc (file_size+1);

  if (!buffer)
  {
    printf ("Недостаточно памяти для разбора файла '%s'\n",file_name);
    return 0;
  }
    
  size_t read_size   = fread (buffer,1,file_size,file);  
  buffer [read_size] = 0;
  
  fclose (file);  
  
  ModelNode* node = csgParseExpression (buffer,file_name);
  
  free (buffer);
  
  return node;
}
