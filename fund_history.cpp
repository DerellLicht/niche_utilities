//**********************************************************************************************
//  fund_history - do some computations on Fidelity Purchase History page
//  
//  build: g++ -Wall -O2 fund_history.cpp -o fund_history.exe
//  test run: fund_history -fFFRHX "Portfolio Positions.htm"
//**********************************************************************************************
//  reference documents:
//  Portfolio Positions.htm - saved page from Fidelity:FFRHX fund
//  PortfolioTable.html - extract from Portfolio Positions.htm, Line 2136, length: 28254,
//     which contains the entire data table that I need to parse.
//**********************************************************************************************
#include <windows.h>
#include <stdio.h>
#ifdef _lint
#include <stdlib.h>
#endif

// typedef unsigned char         u8 ;
// typedef unsigned short        u16 ;
// typedef unsigned long         u32 ;
typedef unsigned int          uint ;
//lint -e10   Expecting '}'

//lint -e641  PSF_SEEK_FUND_CODE, PSF_SEEK_ACQUIRED, PSF_FOUND_ACQUIRED, PSF_SCAN_DATA, PSF_END)
//lint -e818  Pointer parameter could be declared as pointing to const
//lint -esym(40, errno)
//lint -esym(749, PSF_END)  //local enumeration constant not referenced

static char fund_data[MAX_PATH+1] = "" ;
#define  FUND_NAME_LEN  5
static char fund_code[FUND_NAME_LEN+1] = "";

typedef struct fid_fund_info_s {
   fid_fund_info_s *next ;
   uint ymd ;
   uint year ;
   double current_value ;
   double cost_basis_value ;
} fid_fund_info_t, *fid_fund_info_p ;

static fid_fund_info_p ffi_top = NULL ;
static fid_fund_info_p ffi_tail = NULL ;
               
//****************************************************************************
void usage(void)
{
   puts("Usage: fund_history fund_data_filename -f<five_char_fund>");
}

//****************************************************************************
//  state 0: search for <scan>FCODE</scan>
//  state 1: search for 'Acquired' in data table header
//  state 2: search for first data row [pvd-table__row], iterate over these rows
//  state 3: iterate over data elements in this row
static char fund_str[80] ;
static uint parse_state = 0 ;

enum {
   PSF_SEEK_FUND_CODE=0,
   PSF_SEEK_ACQUIRED,
   PSF_FOUND_ACQUIRED,
   PSF_SCAN_DATA,
   PSF_END   
} ;

static fid_fund_info_p merge(fid_fund_info_p a, fid_fund_info_p b);
//************************************************************
//  the following object is a dummy point structure
//  which is used by merge_sort.  
//  The main code must allocate a strucure for this to point to.
//  
//  A global function pointer is also required by the sort routine.  
//  This will point to a function which accepts two structure pointers 
//  as arguments, and returns:
//  
//     >0 if a > b
//    ==0 if a == b
//     <0 if a < b
//  
//************************************************************
static fid_fund_info_p z = NULL ;
static int (*sort_fcn) (fid_fund_info_p a, fid_fund_info_p b) ;

//****************************************************
//  allocate a dummy structure for merge_sort()
//****************************************************
static int init_sort(void) 
{
   // z = (fid_fund_info_p ) malloc(sizeof(ffdata)) ;
   //  new does not return errno on OUT_OF_MEMORY, it just aborts
   z = (fid_fund_info_p ) new fid_fund_info_t ;
   // if (z == NULL)
   //    error_exit(OUT_OF_MEMORY, NULL) ;
   z->next = NULL ;
   return 0 ;
}

//*********************************************************
static int sort_ymd(fid_fund_info_p a, fid_fund_info_p b)
{
   if (a->ymd > b->ymd)  return(1) ;
   else if (b->ymd > a->ymd)  return(-1) ;
   else return(0) ;
}  //lint !e818

//***************************************************************************
//  This routine recursively splits linked lists into two parts, 
//  passing the divided lists to merge() to merge the two sorted lists.
//***************************************************************************
static fid_fund_info_p merge_sort(fid_fund_info_p c)
   {
   fid_fund_info_p a, b, prev ;
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
//  This routine merges two sorted linked lists.
//*********************************************************
static fid_fund_info_p merge(fid_fund_info_p a, fid_fund_info_p b)
   {
   fid_fund_info_p c ;
   c = z ;

   do
      {
      int x = sort_fcn(a, b) ;
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
         b = b->next ;  //lint !e613
         }
      }
   while ((a != NULL) && (b != NULL));

   if (a == NULL)  c->next = b ;  //lint !e613
             else  c->next = a ;  //lint !e613
   return z->next ;
   }

//*********************************************************
//  This intermediate function is used because I want
//  merge_sort() to accept a passed parameter,
//  but in this particular application the initial
//  list is global.  This function sets up the global
//  comparison-function pointer and passes the global
//  list pointer to merge_sort().
//*********************************************************
static void sort_files(int (*current_sort)(fid_fund_info_p a, fid_fund_info_p b))
{
   sort_fcn = current_sort ;
   ffi_top = merge_sort(ffi_top) ;
}

//*****************************************************************
void sort_by_ymd(void)
{
   if (z == 0) {
      init_sort() ;  //lint !e534
   }

   sort_files(sort_ymd) ;
}

//lint +e1013  Symbol 'LowPart' not a member of class '_LARGE_INTEGER'
//lint +e40    Undeclared identifier 'LowPart'
//lint +e63    Expected an lvalue
//****************************************************************************
//  data table header
// <table class="pvd-table__table posweb-purchase-history" tabindex="0">
// <thead class="pvd-table__head">
// <tr class="pvd-table__row posweb-lots-table-row-header">
// <th scope="col" class="pvd-table__column-header-cell posweb-lots-header">
// <span class="pvd-table__column-header-cell-content">Acquired</span></th>
// <th scope="col" class="pvd-table__column-header-cell posweb-lots-header">
// <span class="pvd-table__column-header-cell-content">Term</span></th>
// <th scope="col" class="pvd-table__column-header-cell posweb-lots-header">
// <span class="pvd-table__column-header-cell-content">$ Total Gain/Loss</span></th><th scope="col" class="pvd-table__column-header-cell posweb-lots-header"><span class="pvd-table__column-header-cell-content">% Total Gain/Loss</span></th><th scope="col" class="pvd-table__column-header-cell posweb-lots-header"><span class="pvd-table__column-header-cell-content">Current Value</span></th><th scope="col" class="pvd-table__column-header-cell posweb-lots-header">
// <span class="pvd-table__column-header-cell-content">Quantity</span></th><th scope="col" class="pvd-table__column-header-cell posweb-lots-header">
// <span class="pvd-table__column-header-cell-content">Average Cost Basis</span></th><th scope="col" class="pvd-table__column-header-cell posweb-lots-header"><span class="pvd-table__column-header-cell-content">Cost Basis Total</span></th>
// </tr></thead>
// <tbody class="pvd-table__body">
//  
//  Data rows
// <tr class="pvd-table__row posweb-lots-table-row">
// <td class="pvd-table__data-cell pvd-table__data-cell--right">Nov-29-2024</td>
// <td class="pvd-table__data-cell">Short</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-$1.71</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-0.41%</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$417.15</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">44.663</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$9.38</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$418.86</td>
// </tr>
// 
// <tr class="pvd-table__row posweb-lots-table-row">
// <td class="pvd-table__data-cell pvd-table__data-cell--right">Oct-31-2024</td>
// <td class="pvd-table__data-cell">Short</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-$1.81</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-0.41%</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$444.43</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">47.583</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$9.38</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$446.24</td>
// </tr>
//  last table row
//  $55,027.22</td></tr></tbody></table>

//****************************************************************************
static char *seek_next_tr_tag(char *hd)
{
   char *tl = strstr(hd, "posweb-lots-table-row") ;
   if (tl != NULL) {
      tl = strchr(tl, '>');
   }
   return tl ;
}

//****************************************************************************
//  $55,027.22</td
static double convert_to_double(char *td)
{
   char tbuf[30] = "" ;
   uint outidx = 0 ;
   while (*td != 0) {
      switch (*td) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '.':
         tbuf[outidx++] = *td ;
         tbuf[outidx] = 0 ;
         break ;
         
      default:
         break ;
      }
      
      td++ ;
   }
   double tdbl = strtod(tbuf, NULL);   //lint !e119
   return tdbl ;
}

//****************************************************************************
static char const monstr[13][4] = {
   "Jan", "Feb", "Mar", "Apr", "May", "Jun", 
   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec", ""};
   
static uint extract_month(char *inpstr)
{
   uint idx;
   for (idx=0; monstr[idx][0] != 0; idx++) {
      if (strncmp(monstr[idx], inpstr, 3) == 0) {
         return idx ;
      }
   }
   return idx ;
}

//****************************************************************************
//    0: Dec-29-2023
//    0: Nov-29-2024
//    0: Oct-31-2024
//    0: Sep-30-2024
//    0: Aug-30-2024
//    0: Jul-31-2024
//    0: Jun-28-2024
//    0: May-31-2024
//    0: Apr-30-2024
//    0: Mar-28-2024
//    0: Feb-29-2024
//    0: Jan-31-2024
//****************************************************************************
static void extract_date(char *inpstr, fid_fund_info_p ffi_new)
{
   uint month = extract_month(inpstr);
   uint date  = (uint) atoi(inpstr+4);
   ffi_new->year  = (uint) atoi(inpstr+7);
   ffi_new->ymd = (ffi_new->year * 10000) + ((month+1) * 100) + date;
   // printf("   [%04u %02u %02u, %08u]\n", year, month+1, date, (uint) ymd);
   // return ffi_new->ymd ;
}

//****************************************************************************
//  "Acquired" has been Acquired...
//  All required data is in the string that is passed to this function.
//  Now, find the required elements and process them.
//****************************************************************************
static int parse_fund_data(char *inpstr)
{
   char *srch = NULL;

   // row4sum = 0.0 ;
   // row7sum = 0.0 ;
   uint data_column = 0 ;
   
   //******************************************************
   //  parse entire table
   //******************************************************
   bool outer_done = false ;  
   while (!outer_done) {
      switch (parse_state) {
      case PSF_FOUND_ACQUIRED:
         srch = strstr(inpstr, "pvd-table__body");
         //  seek end of data header row
         if (srch == NULL) {
            printf("end of table header line not found, aborting!!\n");
            return 1 ;
         }
         printf("found start of data table\n");
         parse_state = PSF_SCAN_DATA ; //  go to iterate-over-table-row state
         break ;
         
      //  iterate over table-row entries
// <tr class="pvd-table__row posweb-lots-table-row">
// <td class="pvd-table__data-cell pvd-table__data-cell--right">Oct-31-2024</td>
// <td class="pvd-table__data-cell">Short</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-$1.81</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-0.41%</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$444.43</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">47.583</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$9.38</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$446.24</td>
// </tr>

      case PSF_SCAN_DATA:
         {  //  begin local context
         printf("ready to start scanning data elements\n");
         uint data_rows = 0 ;
         char *tr = srch ;
         //*********************************************************         
         //  iterate over table row with all table data in it
         //*********************************************************         
         bool done = false ;
         while (!done) {   //  parse table data
            //  this also counts the terminating row...
            data_rows++ ;
            // printf("data row: %u\n", data_rows);
            data_column = 0 ;

            //  allocate struct for new data row
            fid_fund_info_p ffi_new = new fid_fund_info_t ;
            ZeroMemory(ffi_new, sizeof(fid_fund_info_t));
            
            //  seek to <tr> tag for next row
            tr = seek_next_tr_tag(tr);
            if (tr == NULL) {
               // printf("data rows: %u, end of table found\n", data_rows);
               parse_state = PSF_SEEK_FUND_CODE ;
               done = true ;
               outer_done = true ;
               continue ;
            }
            //  temp null-term row tag so TD string searches will be bounded
            char *trcl = strstr(tr, "</tr>");
            if (trcl == NULL) {
               printf("DR: %u, row term not found\n", data_rows);
               parse_state = PSF_SEEK_FUND_CODE ;
               done = true ;
               outer_done = true ;
               continue ;
            }
            *trcl = 0 ; //  NULL-term the row close tag
            
            //*****************************************************
            //  iterate over TD elements in row
            //*****************************************************
            char *td = tr ;
            bool data_srch_done = false ;
            while (!data_srch_done) {
               // printf("### TR len left: %u\n", strlen(td));
               td = strstr(td, "<td") ;
               //  if this fails, we likely are at end of row
               if (td == NULL) {
                  // printf("DR: %u, TD open not found\n", data_rows);
                  // parse_state = PSF_SEEK_FUND_CODE ;
                  data_srch_done = true ;
                  // done = true ;
                  // outer_done = true ;
                  continue ;
               }
               td = strchr(td, '>');
               if (td == NULL) {
                  printf("DR: %u, TD open tail not found\n", data_rows);
                  parse_state = PSF_SEEK_FUND_CODE ;
                  data_srch_done = true ;
                  done = true ;
                  outer_done = true ;
                  continue ;
               }
               td++ ;   //  point to data element
               //  debug: temp null-term TD close tag for debug output
               char *tdcl = strstr(td, "</td>");
               if (tdcl == NULL) {
                  printf("DR: %u, TD close not found\n", data_rows);
                  parse_state = PSF_SEEK_FUND_CODE ;
                  data_srch_done = true ;
                  done = true ;
                  outer_done = true ;
                  continue ;
               }
               *tdcl = 0 ;

               //  show extracted value               
               // data row: 35
               //   0: Apr-21-2022       Acquired date
               //   1: Long              Term: short/long
               //   2: -$223.63          Total gain/loss, dollars
               //   3: -0.41%            Total gain/loss, percent
               //   4: $54,803.59        Current Value
               //   5: 5,867.622         Quantity
               //   6: $9.38             Average Cost Basis
               //   7: $55,027.22        Cost Basis Total
               
               // printf("   %u: %s\n", data_column, td);
               switch (data_column) {
               case 0:
                  extract_date(td, ffi_new);
                  break ;
               case 4:
                  ffi_new->current_value = convert_to_double(td) ;
                  // row4sum += convert_to_double(td) ;
                  break ;
               case 7:
                  ffi_new->cost_basis_value = convert_to_double(td) ;
                  // row7sum += convert_to_double(td) ;
                  break ;
                  
               default:
                  break ;
               }
               *tdcl = '<' ;  //  restore TD close tag
               
               data_column++ ;
            }  //  end !data_srch_done
            
            *trcl = '<' ;  //  restore row term after parsing data elements
            tr = trcl ;   //  slide tr ptr to next element
            //  now, iterate over <td> tags until </tr> is found

            //*****************************************************            
            //  attach new element onto linked list
            //*****************************************************            
            if (ffi_top == NULL) {
               ffi_top = ffi_new ;
            }
            else {
               ffi_tail->next = ffi_new ;
            }
            ffi_tail = ffi_new ;
         }
         }  //  end local context
         break ;
         
      default: 
         printf("unused deep state, resetting...\n");
         parse_state = PSF_SEEK_FUND_CODE ;
         outer_done = true ;
         break ;
      }
   }
   return 0 ;
}
         
//****************************************************************************
static int parse_input_line(char *inpstr, uint lcount, uint slen)
{
   //  states 1, 2, 3 are all in one common line of text
// L2135  slen: 251
// L2136  slen: 28254
// L2137  slen: 95
//    if (lcount > 2130  &&  lcount < 2150) {
//       printf("L%u  slen: %u\n", lcount, slen);
//    }
   int result = 0 ;
   switch (parse_state) {
   case PSF_SEEK_FUND_CODE:  //  search for <scan>FCODE</scan>
      if (strstr(inpstr, fund_str) != NULL) {
         printf("Found %s\n", fund_code);
         parse_state = PSF_SEEK_ACQUIRED ;
      }
      break ;
      
   case PSF_SEEK_ACQUIRED:
      if (strstr(inpstr, "Acquired") != NULL) {
         //  all table data will be in this one line
         printf("Line %u, length: %u: Found [Acquired]\n", lcount, slen);
         parse_state = PSF_FOUND_ACQUIRED ;
         result = parse_fund_data(inpstr);
      }
      break ;
      
   default:
      printf("unused state, resetting...\n");
      parse_state = PSF_SEEK_FUND_CODE ;
      break ;
   }
   return result;
}

//****************************************************************************
int main(int argc, char** argv)
{
   int idx ;
   for (idx=1; idx<argc; idx++) {
      char *p = argv[idx];
      if (*p == '-') {
         p++ ;
         switch (*p) {
         case 'f':
            p++ ;
            strncpy(fund_code, p, FUND_NAME_LEN);
            fund_code[FUND_NAME_LEN] = 0 ;
            break ;
            
         default:
            usage() ;
            return 1 ;
         }
      }
      else {
         strncpy(fund_data, p, MAX_PATH);
         fund_data[MAX_PATH] = 0 ;
      }
   }
   
   if (fund_data[0] == 0  ||  fund_code[0] == 0) {
      usage() ;
      return 1 ;
   }
   
   printf("seek %s in %s\n", fund_code, fund_data);
   sprintf(fund_str, "<span>%s</span>", fund_code);

   FILE* infd = fopen(fund_data, "rt");
   if (infd == NULL) {
      printf("%s: %s\n", fund_data, strerror(errno));
      return errno;
   }

//  longest line in file was 795,642 chars   
//  The line with our target table data, was Line 2136, length: 28254 chars
#define  MAX_LINE_LEN   (1024*1024)
#define  MAX_SAFE_LEN   (MAX_LINE_LEN - 2000)
   char inpstr[MAX_LINE_LEN+1];
   int result = 0 ;
   uint lcount = 0 ;
   uint max_line_len = 0 ;
   while (fgets(inpstr, MAX_LINE_LEN, infd) != NULL) {
      lcount++ ;
      //  the purpose of this test, is to ensure that we don't read a partial text line.
      //  I would rather just have enough space for the longest line in the file,
      //  and not bother with partial-line merging.
      uint slen = strlen(inpstr);
      if (slen > MAX_SAFE_LEN) {
         printf("line %u: excessive line length [%u]\n", lcount, slen);
         printf("test file with maxlen, then update this program");
         result = 1 ;
         break ;
      }
      if (max_line_len < slen) {
         max_line_len = slen ;
      }
      
      result = parse_input_line(inpstr, lcount, slen);
      if (result != 0) {
         printf("line %u: error %d\n", lcount, result);
         break ;
      }
   }
   fclose(infd);
   
   //  sort list by ymd
   sort_by_ymd();
   
   //  then iterate over list to build desired totals
   double row4sum = 0.0 ;
   double row7sum = 0.0 ;

   // double current_value ;
   // double cost_basis_value ;
   uint baseline_year = 0 ;
   double baseline_value = 1.0 ;
   double baseline_thold = 10000.0 ;   //  if line change > thold, reset baseline
   double current_value = 0.0 ;
   double value_change = 0.0 ;
   double pct_change = 0.0 ;
   fid_fund_info_p ffi_temp ;
   uint fcount = 0 ;
   puts("");
   for (ffi_temp = ffi_top; ffi_temp != NULL; ffi_temp = ffi_temp->next) {
      fcount++ ;
      //  if new value is much larger than current value,
      //  assume this reflects a new deposit; update current and baseline values
      if (ffi_temp->current_value > (baseline_value + baseline_thold)) {
         baseline_value = ffi_temp->current_value ;
         current_value = ffi_temp->current_value ;
         baseline_year = ffi_temp->year ;
      }
      else if (ffi_temp->year != baseline_year) {
         current_value += ffi_temp->current_value ;
         value_change = (current_value - baseline_value) ;
         pct_change = (value_change / baseline_value) * 100.0 ;
         printf("%04u: %.2f %.2f, %.2f, %.2f\n", baseline_year, current_value, baseline_value,
            value_change, pct_change);
            
         //  update baseline values
         current_value += ffi_temp->current_value ;
         baseline_value = current_value ;
         baseline_year = ffi_temp->year ;
         
      }
      else {
         current_value += ffi_temp->current_value ;
      }
      
      //  calculate totals
      row4sum += ffi_temp->current_value ;
      row7sum += ffi_temp->cost_basis_value ;
      printf("%03u: %08u: %9.2f  %9.2f\n", 
         fcount, ffi_temp->ymd, ffi_temp->current_value, ffi_temp->cost_basis_value);
   }
   //  update final calculations
   value_change = (current_value - baseline_value) ;
   pct_change = (value_change / baseline_value) * 100.0 ;
   printf("%04u: %.2f %.2f, %.2f, %.2f\n", baseline_year, current_value, baseline_value,
      value_change, pct_change);
   
   //  output totals
   printf("max line length: %u chars\n", max_line_len);
   printf("%u elements found\n", fcount);
   printf("Current value:    %.2f\n", row4sum);
   printf("Cost basis total: %.2f\n", row7sum);
   
   return result ;
}
