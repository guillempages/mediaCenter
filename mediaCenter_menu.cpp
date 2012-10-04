#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <string>

#include <sys/time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <signal.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#include "utils.h"

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
using std::vector;
using std::string;

void * font;

int width, height;
int pos=0;
vector<string> menu,menuFiles;

int sock=-1;
struct sockaddr_in remote;
string dir=".";

void finish() {
  if (sock>=0)
    close(sock);
  sock=-1;
}

void term(int result=0) {
  finish();
  exit(result);
}

void writeText(double x, double y, const char* text) {
  if (text==NULL)
    return;

  glRasterPos2f(x,y);

  for (int i=0; text[i]!='\0'; i++) {
    glutBitmapCharacter(font,text[i]);
  }
}

int getTextWidth(const char* text) {
  int result=0;
  
  if (text==NULL)
    return result;

  for (int i=0; text[i]!='\0'; i++) {
    result+=glutBitmapWidth(font,text[i]);
  }
  
  return result;
}

void showMenu(vector<string> menu, int pos=-1) {
  double pos0, inc;
  int i,start;

  inc=0.1;

  pos0=0.9;
  
  start=pos-9+menu.size();

  if (menu.size()<19) {
    for (i=0; i<menu.size(); i++) {
      if (i!=pos%menu.size())
        glColor3f(0.6,0.9,0.7);
      else
        glColor3f(0.9,0.3,0.3);
      writeText(-0.1,pos0-(i)*inc,menu[i].c_str());
    }
    
  } else {

    for (i=start; (i<menu.size()||menu.size()>=19) && i<start+19; i++) {
      if (i%menu.size()!=pos%menu.size()) {
        glColor3f(0.6,0.9,0.7);
        font=GLUT_BITMAP_HELVETICA_18;
      } else {
        glColor3f(0.9,0.15,0.2);
        font=GLUT_BITMAP_TIMES_ROMAN_24;
      }
      writeText(-0.06 - 0.0013*getTextWidth(menu[i%menu.size()].c_str()),pos0-(i-start)*inc,menu[i%menu.size()].c_str());
    }
  }
}

void displayFunction(void) {

  glClear(GL_COLOR_BUFFER_BIT);

  glPushMatrix();

  showMenu(menu,pos);  

  glPopMatrix();

  glutSwapBuffers();
}

void
my_reshape(int w, int h)
{
  width = w;
  height = h;
}

void
my_handle_key(unsigned char key, int x, int y)
{

   switch (key) {

   case 27:    // Esc - Quits the program.
      exit(0);
      break;

   case 'u': //Up
      pos=(pos+menu.size()-1)%(menu.size()*2);
      break;
   case 'd': //Down
      pos=(pos+1)%(menu.size()*2);
      break;
   case '\r':
   case '\n': {//Enter
      //send file to server
      string fileName="Movie ";
      fileName = fileName + menuFiles[pos%menuFiles.size()]; 
      DBG(cout << "Sending " << fileName << endl);
      sendto(sock,fileName.c_str(),fileName.length()+1,0,(struct sockaddr*)&remote,sizeof(remote));
      term(0);
      break;
     }
   default:
      break;
   }
   glutPostRedisplay();
}

void 
my_idle()
{
  fd_set fdSock;
  struct timeval timeout;
  int error;

  timeout.tv_sec=0;
  timeout.tv_usec=100000;
  error=0;

  FD_ZERO(&fdSock);
  FD_SET(sock,&fdSock);

  error=select(sock+1,&fdSock,NULL,&fdSock,&timeout);
  if (error>0) {
    char buf[513];

    if (error=recv(sock,buf,512,0)) {
       buf[error]=0;
       if ((buf[error-1]=='\n') || (buf[error-1]=='\r'))
         buf[error-1]=0;
       if ((error>1) && ((buf[error-2]=='\r') || (buf[error-2]=='\n'))) 
         buf[error-2]=0;
       if (!strcmp(buf,"Up")) {
         my_handle_key('u',0,0);
       } else if (!strcmp(buf,"Down")) {
         my_handle_key('d',0,0);
       } else if (!strcmp(buf,"Enter")) {
         my_handle_key('\n',0,0);
       } else {
         DBG(cout << "mediaCenter_menu received " << buf << endl);
       }
    }
  } else if (error==0) {
  } else if (error<0) {
    perror("select");
  }
}

vector<string> listDirectory(const string& directory) {
  
  vector<string> result;

  DIR * films=opendir(directory.c_str());

  if (films==NULL) {
    perror("opendir failed");
    return result;
  }

  struct dirent *entry=readdir(films);
  string file,strtmp;
  struct stat filestat;

  while (entry!=NULL) {
    file=entry->d_name;

    strtmp=directory+"/"+file;
    stat(strtmp.c_str(),&filestat);
	  
    file=file.substr(0,file.rfind("."));

    if (S_ISREG(filestat.st_mode)) {
      menuFiles.push_back(strtmp);
      result.push_back(file);
    }

    entry=readdir(films);
  }

}

int initGlutWindow(int argc, char* argv[]) {


  glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
  glutInitWindowSize(750,600);
  glutInitWindowPosition(20,0);

  DBG(cout << "glutInit..." << flush);
  glutInit(&argc,argv);
  DBG(cout << "done" << endl);

  glClearColor(0.8,0.8,1,0.5);
  font=GLUT_BITMAP_TIMES_ROMAN_24;

  glutCreateWindow("window");

  glutReshapeWindow(750,600);


  glutReshapeFunc(my_reshape);
  glutKeyboardFunc(my_handle_key);
  glutDisplayFunc(&displayFunction);
  glutIdleFunc(&my_idle);

  menu=listDirectory(dir);

  signal(SIGTERM,term);
  signal(SIGINT,term);
  atexit(finish);

  glutMainLoop();
}


void usage(string progName) {
  std::cerr << "Usage:" << endl
       << "  " << progName << " [-p local port] [-s server] [-r remote port] [-f dir] [-t type]" << endl;
}

int main(int argc, char * argv[]) {

  //signal handlers
  signal(SIGINT, term);
  signal(SIGTERM, term);

  //remove leading path from program name.
  string programName=argv[0];
  int port=10011;
  int remotePort=10010;
  string server="127.0.0.1";
  string mediaType="";
  
  int pos=programName.rfind("/");
  if (pos!=string::npos) {
    programName=programName.substr(pos+1);
  }

  const char * basename=programName.c_str();

  int optc=0;
  while ((optc=getopt(argc,argv,"r:p:s:f:t:")) != -1 ) {
    switch (optc) {
      case 'p': // port
        port=atoi(optarg);
        break;
      case 's': // server
        server=optarg;
        break;
      case 'r': // remotePort
        remotePort=atoi(optarg);
        break;
      case 'f': // dir
        dir=optarg;
        break;
      case 't': // type 
        mediaType=optarg;
        break;
      default:
        usage(basename);
        exit(-1);
    }
  }

  DBG(cout << "Program Name: " << programName << endl);
  DBG(cout << "Port: " << port << endl);
  DBG(cout << "Server: " << server << endl);
  DBG(cout << "Remote Port: " << remotePort << endl);
  DBG(cout << "Directory: " << dir << endl);
  DBG(cout << "Type: " << mediaType << endl);

  sock=socket(PF_INET,SOCK_DGRAM,0);
  if (sock<0) {
    perror(basename);
    term(-2);
  }

  struct sockaddr_in local;
  struct hostent *phe;
  
  local.sin_family=AF_INET;
  local.sin_port=htons(port);
  local.sin_addr.s_addr=INADDR_ANY;

  if (bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0) {
    perror((programName+".bind").c_str());
    term(-2);
  } 

  remote.sin_family=AF_INET;
  remote.sin_port=htons(remotePort);

  if (phe=gethostbyname(server.c_str()))
    memcpy((char*)&remote.sin_addr, phe->h_addr, phe->h_length);
  else if ( ( remote.sin_addr.s_addr = inet_addr(server.c_str())) == INADDR_NONE) {
    perror(basename);
    term(-2);
  }


  initGlutWindow(argc,argv);

}
