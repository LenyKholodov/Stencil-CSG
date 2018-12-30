#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <io.h>

#include "gl/glew.h"
#include "gl/glui.h"

#include "model.h"

#pragma warning (disable : 4996) //'function' was declared deprecated

//�������������� ��������� ����������
enum ControlId
{
  CONTROL_FILE_BROWSER, //�������� �������
};

//���������� ���������� �����������
GLUI_FileBrowser* file_browser    = 0;     //��������� �� �������� �������
GLUI_StaticText*  primitives_text = 0;     //������ ������������� � ���������� ���������� � ����������� ������
GLUI_StaticText*  booleans_text   = 0;     //������ ������������� � ���������� ������� �������� � ����������� ������
bool              motion_enable   = false; //���� ������ �������� ������
int               mouse_x         = 0,     //������� ���������� ����
                  mouse_y         = 0;                  

//���������� ���������� ���������� �������
float model_rotate [16]  = {1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1}; //������� �������� ������
float model_position [3] = {0,0,0};                           //������ ����������� ������

//���������� ������������ ������
RenderModel* render_model  = 0;    //��������������� CSG ������
int          visialize_csg = 1;    //��������������� �� ��������� CSG-��������

//��������� �������� ���� ������
void resize (int,int)
{
  int x, y, w, h;
  
  GLUI_Master.get_viewport_area (&x,&y,&w,&h);  
  
    //������ ������� ���� ������

  glViewport (x,y,w,h);
  
    //�������� ������� �������������
    
  float ky = (float)w / (float)h; //����������� ��������������� �� ��� Oy    
    
  glMatrixMode   (GL_PROJECTION);
  glLoadIdentity ();
  glOrtho        (-40,40,-40/ky,40/ky,-1000,1000);
  glMatrixMode   (GL_MODELVIEW);
  
    //�������� ������� ���������� GLUT �� �����������
  
  glutPostRedisplay ();
}

//������� ������������ �����
void redraw ()
{
    //������� ���������� �������
  glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
  
    //��������� ������������ ����� ��������� � ��������� CSG-������
  
  glPushMatrix   ();  
  glLoadIdentity ();
  glTranslatef   (model_position [0],model_position [1],model_position [2]);
  glMultMatrixf  (model_rotate);
  csgDraw        (render_model,visialize_csg != 0);
  glPopMatrix    ();    

  glFinish        (); //��������� �������� ������ OpenGL
  glutSwapBuffers (); //����� ��������������� ����������� � ���� ������
}

//��������� ������������ CSG ������ ������������ ������� ���������
void motion (int x,int y)
{
  if (!motion_enable) //��������: ��������� �� ��������� � ������ �������� ��� ������ ����
    return;

    //��������, ��������������� ����������� ��������� ����

  glPushMatrix  ();
  glLoadMatrixf (model_rotate);  
  glRotatef     (x-mouse_x,0,1,0);
  glRotatef     (y-mouse_y,1,0,0);
  glGetFloatv   (GL_MODELVIEW_MATRIX,model_rotate);
  glPopMatrix   ();

    //���������� ������� ��������� ��������� ����
  
  mouse_x = x;
  mouse_y = y;
  
    //������ �� ���������� ���� ������

  glutPostRedisplay ();
}

//���������� ������� ������ ����
void mouse (int button,int state,int x,int y)
{
  if (button != GLUT_LEFT_BUTTON)
    return;    

  motion_enable = !state;
  mouse_x       = x;
  mouse_y       = y;
}

//���������� ������� ����������
void glui_callback (int control)
{
  switch (control)
  {
    case CONTROL_FILE_BROWSER: //�������� ������ ��������� �������������
    {      
        //������������ ���������� ��������������� ������      
      
      csgRelease (render_model);
      
      render_model = 0;
      
        //������� �������������� ���������
        
      primitives_text->set_text ("");
      booleans_text->set_text ("");
      
        //�������� ����� ������
      
      ModelNode* model = csgParseFile (file_browser->get_file ());

      if (!model)
      {
        printf ("������ ������� CSG-������\n");        
        break;
      }

        //�������� ��������������� ������ �� �����������      
      
      render_model = csgCreateRenderModel (model);            
      
        //����� ���������� � ������
      
      char text_buf [512] = {0};
      
      _snprintf (text_buf,sizeof (text_buf),"Total primitives count: %d",csgTotalPrimitivesCount (model));
      
      primitives_text->set_text (text_buf);
      
      _snprintf (text_buf,sizeof (text_buf),"Total booleans count: %d",csgTotalBooleansCount (model));
      
      booleans_text->set_text (text_buf);
      
        //������������ ����������� ������

      csgRelease (model);      

      break;      
    }
    default:
      break;
  }

  glutPostRedisplay ();
}

//������������� ���������� OpenGL
void init_opengl ()
{
    //��������� ���������� ������� ������
  glMatrixMode (GL_PROJECTION);
  glOrtho      (-40,40,-40,40,-1000,1000);
  glMatrixMode (GL_MODELVIEW);
  
    //��������� ����� ����
  glClearColor (1,1,1,0.7);
  
    //������������� ���������� ���������� GLEW (���������� ��� OpenCSG)
  glewInit ();
    
    //��������� ������ ������������  
  glEnable (GL_DEPTH_TEST);
  glEnable (GL_LIGHTING);
  glEnable (GL_LIGHT0);
  glEnable (GL_COLOR_MATERIAL);
  
    //���������� ��������� ���� � ���� ������
  glLightModeli (GL_LIGHT_MODEL_TWO_SIDE,1);
}

//������������� ���������� GLUI
void init_glui (int main_window)
{
    //��������� ������� ��������� �������� ����   
    
  extern void resize (int,int);

  GLUI_Master.set_glutReshapeFunc (resize);

    //�������� �������� ���� � ������� ��������� ����������

  GLUI*       search_window   = GLUI_Master.create_glui_subwindow (main_window,GLUI_SUBWINDOW_RIGHT);
  GLUI*       settings_window = GLUI_Master.create_glui_subwindow (main_window,GLUI_SUBWINDOW_TOP);  
  GLUI_Panel* control_panel   = new GLUI_Panel (settings_window,"",true);  
  GLUI_Panel* search_panel    = new GLUI_Panel (search_window,"",true); 
  
    //�������� ��������� ���������� ������������� ��������
  
  GLUI_Translation* trans_xy = new GLUI_Translation (control_panel,"XY",GLUI_TRANSLATION_XY,model_position);
  
  new GLUI_Column (control_panel,false);
  
  GLUI_Translation* trans_x = new GLUI_Translation (control_panel,"X",GLUI_TRANSLATION_X,model_position);

  new GLUI_Column (control_panel,false);
  
  GLUI_Translation* trans_y = new GLUI_Translation (control_panel,"Y",GLUI_TRANSLATION_Y,&model_position [1]);
  
  new GLUI_Column (control_panel,false);
  
  GLUI_Rotation* view_rot = new GLUI_Rotation (control_panel,"Rotate",model_rotate);    
  
  new GLUI_Column (control_panel,false);    

  trans_xy->set_speed (.05);
  trans_x->set_speed  (.05);  
  trans_y->set_speed  (.05);
  view_rot->set_spin  (1.0);  
  
    //�������� ������� �������� �����

  new GLUI_StaticText (search_panel,"Open Text File:");     
  
  file_browser = new GLUI_FileBrowser (search_panel,"",false,CONTROL_FILE_BROWSER,glui_callback);
  
  file_browser->set_h                (280);
  file_browser->set_allow_change_dir (0);   //��������� ����� ��������
  
    //�������� �������������� ���������
    
  new GLUI_Column (settings_window,false);  
    
  new GLUI_StaticText (settings_window,"");
    
  primitives_text = new GLUI_StaticText (settings_window,"");
  booleans_text   = new GLUI_StaticText (settings_window,"");
  
  new GLUI_StaticText (settings_window,"");  
  new GLUI_Checkbox (settings_window,"Visualize CSG",&visialize_csg);
  
    //��������� �������� ���� ����������

  settings_window->set_main_gfx_window (main_window);
  search_window->set_main_gfx_window (main_window);
}

int main (int argc, char* argv[])
{
    //������� � ������� � ������� ������
    
  chdir ("models");

    //������������� ���������� GLUT

  glutInit            (&argc,argv);
  glutInitWindowSize  (800,600);
  glutInitDisplayMode (GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE|GLUT_STENCIL);
    
  int main_window = glutCreateWindow ("CSG-model visualization");

  glutDisplayFunc (redraw);
  glutReshapeFunc (resize);
  glutMotionFunc  (motion);  
  glutMouseFunc   (mouse);  

    //������������� ���������� GLUI
    
  init_glui (main_window);
  
    //������������� ���������� OpenGL
    
  init_opengl ();  

    //������� ���� ����������
    
  glutMainLoop ();
  
  return 0;
}
