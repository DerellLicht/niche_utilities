//**********************************************************************************
//  read_wb_skins.cpp 
//  read all fonts installed in WindowBlinds, 
//  display number of skins from each author
//  
//  Written by:  Derell Licht
//  build: g++ -Wall -O2 -s read_wb_skins.cpp -o read_wb_skins.exe
//  build: g++ -Wall -O2 -s -DUNICODE -D_UNICODE read_wb_skins.cpp -o read_wb_skins.exe
//**********************************************************************************

//  the elevated __MSVCRT_VERSION__ declaration is required in order to
//  enable _O_U16TEXT in fcntl.h
#define __MSVCRT_VERSION__    0x0800
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>   //  _O_U16TEXT
#include <shlobj.h>  //  CSIDL_*
// #include <tchar.h>

typedef  unsigned int   uint ;

#define  MAX_LINE_LEN   128
#define  MAX_PATH_LEN   1024


//lint -e10   Expecting '}'
//lint -esym(40, errno)
//lint -e818  Pointer parameter could be declared as pointing to const

//  per Jason Hood, this turns off MinGW's command-line expansion, 
//  so we can handle wildcards like we want to.                    
//lint -e765  external '_CRT_glob' could be made static
//lint -e714  Symbol '_CRT_glob' not referenced           
int _CRT_glob = 0 ;

//lint -esym(843, show_all)
bool show_all = true ;
static bool show_skin_names = false ;

// Windowblinds installed-theme folder
// C:\Users\Public\Documents\Stardock\WindowBlinds
static char file_spec[MAX_PATH_LEN+1] = "C:\\Users\\Public\\Documents\\Stardock\\WindowBlinds\\skins.nbd" ;

//**********************************************************************
static void strip_newlines(char *rstr)
{
   int slen = (int) strlen(rstr) ;
   while (1) {
      if (slen == 0)
         break;
      if (*(rstr+slen-1) == '\n'  ||  *(rstr+slen-1) == '\r') {
         slen-- ;
         *(rstr+slen) = 0 ;
      } else {
         break;
      }
   }
}

//**********************************************************************************
#define  MAX_SKIN_LEN   60   
#define  MAX_AUTH_LEN   80

typedef struct skin_info_s {
   skin_info_s *next ;
   char name[MAX_SKIN_LEN+1] ;
   char author[MAX_AUTH_LEN+1] ;
} skin_info_t, *skin_info_p ;

static skin_info_p si_head = NULL ;
static skin_info_p si_tail = NULL ;

typedef struct skin_author_s {
   skin_author_s *next ;
   char author[MAX_AUTH_LEN+1] ;
   uint skin_count ;   
} skin_author_t, *skin_author_p ;

static skin_author_p sa_head = NULL ;
static skin_author_p sa_tail = NULL ;

//*********************************************************
static int sort_name(skin_info_p a, skin_info_p b)
{
   return(strcmpi(a->author, b->author)) ;
}

//*********************************************************
static skin_info_p z = NULL ;

//*********************************************************
//  This routine merges two sorted linked lists.
//*********************************************************
static skin_info_p merge(skin_info_p a, skin_info_p b)
   {
   skin_info_p c = z ;

   do {
      int x = sort_name(a, b) ;
      if (x <= 0)
         {
         c->next = a ;
         c = a ;
         a = a->next ;
         }
      else
         {
         c->next = b ;
         c = b ;
         b = b->next ;
         }
      }
   while ((a != NULL) && (b != NULL));

   if (a == NULL)  c->next = b ;
             else  c->next = a ;
   return z->next ;
   }

//*********************************************************
//  This routine recursively splits linked lists
//  into two parts, passing the divided lists to
//  merge() to merge the two sorted lists.
//*********************************************************
static skin_info_p merge_sort(skin_info_p c)
   {
   skin_info_p a ;
   skin_info_p b ;
   skin_info_p prev ;
   int pcount = 0 ;
   int j = 0 ;

   if ((c != NULL) && (c->next != NULL))
      {
      a = c ;
      while (a != NULL)
         {
         pcount++ ;
         a = a->next  ;
         }
      a = c ;
      b = c ;
      prev = b ;
      while (j <  pcount/2)
         {
         j++ ;
         prev = b ;
         b = b->next ;
         }
      prev->next = NULL ;  //lint !e771

      return merge(merge_sort(a), merge_sort(b)) ;
      }
   return c ;
   }

//*********************************************************
//  This intermediate function is used because I want
//  merge_sort() to accept a passed parameter,
//  but in this particular application the initial
//  list is global.  This function sets up the global
//  comparison-function pointer and passes the global
//  list pointer to merge_sort().
//*********************************************************
static void sort_skins_by_author(void)
{
   if (z == 0) {
      // z = new ffdata ;
      // z = (struct ffdata *) malloc(sizeof(ffdata)) ;
      z = (skin_info_p) new skin_info_t ;
      if (z == NULL)
         return ;
      memset((char *) z, 0, sizeof(skin_info_t)) ;
   }
   si_head = merge_sort(si_head) ;
}

//**********************************************************************************
static void save_skin_entry(char *skin_name_int, char *skin_author)
{
   skin_info_p p = (skin_info_p) new skin_info_t ;
   ZeroMemory(p, sizeof(skin_info_t));
   strcpy(p->name, skin_name_int);
   strcpy(p->author, skin_author);
   
   if (si_head == NULL) {
      si_head = p ;
   }
   else {
      si_tail->next = p ;
   }
   si_tail = p ;
}

//*********************************************************
static skin_author_p y = NULL ;

//*********************************************************
// #define  USE_FWD  1
// #undef  USE_FWD
static int sort_skin_count(skin_author_p a, skin_author_p b)
{
// #ifdef USE_FWD
//    if (a->skin_count > b->skin_count)  return(1) ;
//    else if (b->skin_count > a->skin_count)  return(-1) ;
//    else return(0) ;
// #else   
   if (b->skin_count > a->skin_count)  return(1) ;
   else if (a->skin_count > b->skin_count)  return(-1) ;
   else return(0) ;
// #endif   
}

//*********************************************************
//  This routine merges two sorted linked lists.
//*********************************************************
static skin_author_p merge(skin_author_p a, skin_author_p b)
   {
   skin_author_p c = y ;

   do {
      int x = sort_skin_count(a, b) ;
      if (x <= 0)
         {
         c->next = a ;
         c = a ;
         a = a->next ;
         }
      else
         {
         c->next = b ;
         c = b ;
         b = b->next ;
         }
      }
   while ((a != NULL) && (b != NULL));

   if (a == NULL)  c->next = b ;
             else  c->next = a ;
   return y->next ;
   }

//*********************************************************
//  This routine recursively splits linked lists
//  into two parts, passing the divided lists to
//  merge() to merge the two sorted lists.
//*********************************************************
static skin_author_p merge_sort(skin_author_p c)
   {
   skin_author_p a ;
   skin_author_p b ;
   skin_author_p prev ;
   int pcount = 0 ;
   int j = 0 ;

   if ((c != NULL) && (c->next != NULL))
      {
      a = c ;
      while (a != NULL)
         {
         pcount++ ;
         a = a->next  ;
         }
      a = c ;
      b = c ;
      prev = b ;
      while (j <  pcount/2)
         {
         j++ ;
         prev = b ;
         b = b->next ;
         }
      prev->next = NULL ;  //lint !e771

      return merge(merge_sort(a), merge_sort(b)) ;
      }
   return c ;
   }

//*********************************************************
//  This intermediate function is used because I want
//  merge_sort() to accept a passed parameter,
//  but in this particular application the initial
//  list is global.  This function sets up the global
//  comparison-function pointer and passes the global
//  list pointer to merge_sort().
//*********************************************************
static void sort_skins_by_count(void)
{
   if (y == 0) {
      // y = new ffdata ;
      // y = (struct ffdata *) malloc(sizeof(ffdata)) ;
      y = (skin_author_p) new skin_author_t ;
      if (y == NULL)
         return ;
      memset((char *) y, 0, sizeof(skin_author_t)) ;
   }
   sa_head = merge_sort(sa_head) ;
}

//**********************************************************************************
//lint -esym(714, display_skin_info)
//lint -esym(759, display_skin_info)
//lint -esym(765, display_skin_info)
void display_skin_info(void)
{
   //  do something with the list
   uint si_count = 0 ;
   skin_info_p p ;
   for (p=si_head; p!=NULL; p=p->next) {
      si_count++ ;
      printf("%s, %s\n", p->name, p->author);
   }
   printf("%s: %u skins found\n", file_spec, si_count);
}

//**********************************************************************************
static void put_skin_author_count(char *author, uint count)
{
   // printf("%s: %u skins\n", author, count);
   skin_author_p p = (skin_author_p) new skin_author_t ;
   ZeroMemory((skin_author_p) p, sizeof(skin_author_t));
   strcpy(p->author, author);
   p->skin_count = count ;
   
   if (sa_head == NULL) {
      sa_head = p ;
   }
   else {
      sa_tail->next = p ;
   }
   sa_tail = p ;
}

//**********************************************************************************
static void count_skins_by_author(void)
{
   uint ca_count = 0 ;
   skin_info_p p ;
   skin_info_p curr_auth = NULL ;
   for (p=si_head; p!=NULL; p=p->next) {
      // printf("%s, %s\n", p->name, p->author);
      if (curr_auth == NULL) {
         curr_auth = p ;
         ca_count = 1 ;
      }
      else {
         if (strcmp(p->author, curr_auth->author) == 0) {
            ca_count++ ;
         }
         else {
            put_skin_author_count(curr_auth->author, ca_count);
            curr_auth = p ;
            ca_count = 1 ;
         }
      }
   }
   //  pick up last pending author
   put_skin_author_count(curr_auth->author, ca_count);   //lint !e794
}

//**********************************************************************************
static void show_skins_for_name(char *author)
{
   skin_info_p p ;
   for (p=si_head; p!=NULL; p=p->next) {
      if (strcmp(p->author, author) == 0) {
         printf("          : %s\n", p->name);
      }
   }
}

//**********************************************************************************
static void display_author_info(void)
{
   printf("# skins   author\n");
   printf("=======   ==========================================\n");
   skin_author_p p ;
   uint scount = 0 ;
   for (p=sa_head; p!=NULL; p=p->next) {
      scount++ ;
   }
   printf("%3u       Total number of installed skins\n", scount);
   for (p=sa_head; p!=NULL; p=p->next) {
      printf("%3u       %s\n", p->skin_count, p->author);
      if (show_skin_names) {
         show_skins_for_name(p->author);
      }
   }
}

//**************************************************************************************
//  If this program were to be changed to Unicode (which it should, for general use),
//  then fopen()/fgets()/fclose() should be replaced with Unicode equivalents.
//**************************************************************************************
static int read_wb_files(void)
{
   FILE* infd = fopen(file_spec, "rt");
   if (infd == NULL) {
      printf("%s: %s\n", file_spec, strerror(errno));
      return errno;
   }
// [Aero 11]
// SkinName=Aero 11
// SkinUISName=Aero 11
// SkinAuthor=SimplexDesigns
// SkinCategory=OS
// SkinDate=1727225360
   uint parse_state = 0 ;
   char skin_name[MAX_SKIN_LEN+1];  //  often lower-case
   char skin_name_int[MAX_SKIN_LEN+1]; //  case-sensitive
   char skin_author[MAX_SKIN_LEN+1];

   char inpstr[MAX_LINE_LEN+1];
   while (fgets(inpstr, MAX_LINE_LEN, infd) != NULL) {
      strip_newlines(inpstr);
      char *tl ;
      char *p = inpstr ;
      switch (parse_state) {
      case 0:  //  look for skin name
         if (*p == '[') {
            p++ ; //  skip head char
            //  if closing bracket not found, skip this entry
            tl = strchr(p, ']');
            if (tl != NULL) {
               *tl = 0 ;
               strcpy(skin_name, p);
               parse_state = 1 ; //  search for internal skin name
            }
         }
         break ;
         
      case 1: //  search for internal skin name
         //  if new WB skin name is found, reset state
         if (*p == '[') {
            printf("%s: error, reset search state\n", skin_name);
            parse_state = 0 ;
            break ;
         }
         
         if (strncmp(p, "SkinName", 8) == 0) {
            p += 9 ; //  skip to name
            strcpy(skin_name_int, p);
            parse_state = 2 ; //  search for internal skin name
         }
         break ;
         
      case 2: //  search for internal skin name
         //  if new WB skin name is found, reset state
         if (*p == '[') {
            printf("%s: error, reset search state\n", skin_name);
            parse_state = 0 ;
            break ;
         }
         
         if (strncmp(p, "SkinAuthor", 10) == 0) {
            p += 11 ; //  skip to name
            strcpy(skin_author, p);
            
            // name diff: darkmarble/Dark Marble, David Knott
            // name diff: darkmarble_II/DarkMarble II, David Knott
            // name diff: dr15m/dr15m /.39, purple
            // name diff: elemental/Elemental Forces, MindlessPuppet
            // name diff: HandOfShinyMetal/Hand of ShinyMetal 2.1, scratch
            // name diff: setiglass/SETI glass, DavidK (lightstream)
            // name diff: setigreen/SETI green, DavidK (lightstream)
            // name diff: W.V.S/W.V.S_7, Lypnjtu
            // if (strcasecmp(skin_name, skin_name_int) != 0) {
            //    printf("name diff: %s/%s, %s\n", skin_name, skin_name_int, skin_author);
            // }
            // else {
            //    printf("%s/%s, %s\n", skin_name, skin_name_int, skin_author);
            // }
            save_skin_entry(skin_name_int, skin_author);
            parse_state = 0 ; //  reset search state
         }
         break ;
         
      default:
         parse_state = 0 ; //  reset search state
         break ;
      }
      
   }
   fclose(infd);
   return 0;
}
   
//**********************************************************************************
// static char file_spec[MAX_PATH_LEN+1] = 
//    "C:\\Users\\Public\\Documents\\Stardock\\WindowBlinds\\skins.nbd" ;
static void get_public_documents_path(void)
{
   TCHAR szPath[MAX_PATH];

   HRESULT result = SHGetFolderPath(NULL, CSIDL_COMMON_DOCUMENTS, NULL, 0, szPath) ;
   if (result == 0) {
      // _tprintf(L"public document folder: [ %s]\n", szPath) ;
      // public document folder: [ C:\Users\Public\Documents]      
      // printf("public document folder: [ %s]\n", szPath) ;
      sprintf(file_spec, "%s\\Stardock\\WindowBlinds\\skins.nbd", szPath);
      printf("source: [%s]\n", file_spec);
   }
}

//**********************************************************************************
static void usage(void)
{
   puts("Usage: read_wb_skins [options]");
   puts("Default: only list skin authors and number of skins for each");
   puts("options:");
   puts("-l - list skin names for each author");
   
}

//**********************************************************************************
int main(int argc, char **argv)
{
   int idx ;
   for (idx=1; idx<argc; idx++) {
      char *p = argv[idx];
      if (*p == '-') {
         p++ ;
         switch (*p) {
         case 'l':
            show_skin_names = true ;
            break ;
            
         default:
            usage() ;
            return 1;
         }
      }
      else {
         usage();
         return 1 ;
      }
   }

   get_public_documents_path();
   int result = read_wb_files();
   if (result != 0) {
      return result ;
   }
   // printf("%s: %u lines read, %u skins found\n", file_spec, lcount, sa_count);
   
   sort_skins_by_author();
   // display_skin_info();
   count_skins_by_author();
   sort_skins_by_count();
   display_author_info();
   
   return 0;
}
